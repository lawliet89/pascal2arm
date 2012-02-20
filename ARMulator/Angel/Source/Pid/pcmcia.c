/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.4.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:55:44 $
 *
 * pcmcia.c - routines for driving Vadem VG-468 PCMCIA Interface Controller
 */

#include <stdio.h>

#include "pcmcia.h"
#include "pid.h"
#include "devconf.h"

/*
 * required Angel headers
 */
#include "angel.h"
#include "logging.h"
#include "support.h"
#include "devconf.h"

/*
 * headers with prototypes for the card handler functions
 */
#include "olicom.h"

/*
 * types of PCMCIA memory to map
 */
typedef enum
{
    Common,
    Attribute
} MemType;

/*
 * array of Card Detect Handlers
 */
static CardHandler cards[] =
{
#if ETHERNET_SUPPORTED
    olicom_card_handler,
#endif
    NULL
};

/*
 * structures describing the two PCMCIA sockets
 */
static PCMCIASocket sockets[2];

/**********************************************************************/

/*
 * Routines for accessing VG-468 controller.
 *
 * The Vadem chip will always shift data down to D[7:0] for reads,
 * but the data register is located at a byte-1 offset, so this shifting
 * of data conflicts with the ARMs behaviour during non-word aligned reads
 * (the ARM will try to shift byte-1 to D[7:0] itself).
 */
#pragma no_check_stack
static unsigned int vadem_read(const unsigned int index)
{
    *PCIndex = index;

    return ((*PCDataR >> 24) & 0xff);
}
#pragma check_stack

static void vadem_write(const unsigned int index, const unsigned int value)
{
    *PCIndex = index;

    *PCDataW = value;
}

/*
 * unlock the unique registers in the VG-468 controller
 */
static void vadem_unlock(void)
{
    /*
     * XXX
     *
     * Black Magic Alert
     */
    *PCIndex = 0x0e;
    *PCIndex = 0x37;
}

/*
 *  Function: map_memory
 *   Purpose: program a memory window
 *
 *  Pre-conditions: Caller is responsible for ensuring that no windows
 *                      overlap.
 *
 *    Params:
 *       Input: socket    The PCMCIA socket the window is for
 *
 *              window    Window number
 *
 *              base      Base address of window in NISA memory space
 *
 *              range     Width of window
 *
 *              offset    Offset of window in PCMCIA card's memory space
 *
 *              type      The type of memory being mapped
 *
 *   Returns: Nothing
 *
 * Post-conditions: Window is mapped, but not enabled
 */
static void map_memory(const unsigned int socket, const unsigned int window,
                       const unsigned int base, const unsigned int range,
                       const unsigned int offset, MemType type)
{
    unsigned int reg;
    unsigned int stop, cardoff;

    /*
     * registers for each window are 8 bytes apart
     */
    reg = socket + (window << 3);

    /*
     * set start address, 16-bit data path, and no Zero Wait State
     */
    vadem_write(PCSysA0MStartL + reg, (base >> 12) & 0xff);
    vadem_write(PCSysA0MStartH + reg, ((base >> 20) & 0x0f) | 0x80);

    /*
     * set stop address, and 3 wait states for 16-bit access
     */
    stop = base + (range - 1);
    vadem_write(PCSysA0MStopL + reg, (stop >> 12) & 0xff);
    vadem_write(PCSysA0MStopH + reg, ((stop >> 20) & 0x0f) | 0xc0);

    /*
     * set card offset, no Write Protect, and attribute bit
     */
    cardoff = offset - base;
    vadem_write(PCCMemA0OffL + reg, (cardoff >> 12) & 0xff);
    vadem_write(PCCMemA0OffH + reg, (((cardoff >> 20) & 0x3f) |
                                     ((type == Attribute) ? 0x40 : 0x00)));
}

/*
 *  Function: initialise_slot
 *   Purpose: Set up the PCMCIA controller for a newly inserted card
 *
 *    Params:
 *       Input: socket  Where the card has been inserted (passed
 *                      for convenience).
 *
 *      In/Out: pcs     Complete description of the socket the newly
 *                      inserted card is in.
 *
 *   Returns: Nothing
 */
static void initialise_slot(PCMCIASocket *const pcs,
                            const unsigned int socket)
{
    /*
     * socket now has a card present
     */
    pcs->pcs_present = 1;

    /*
     * turn on power (Vpp{1,2} both get Vcc), and then enable
     * output.  Note that the controller requires this to be
     * a 2-stage process.
     */
    vadem_write(PCPwrRstCtl + socket, 0x30);
    vadem_write(PCPwrRstCtl + socket, 0xb0);

    delay(10000);

    /*
     * reset the card by asserting the reset line, waiting a bit,
     * then taking reset off again
     */
    vadem_write(PCIntGCtl + socket, 0x00);

    delay(10000);

    vadem_write(PCIntGCtl + socket, 0x40);

    /*
     * wait until card is ready
     */
    while ((vadem_read(PCIfStatus + socket) & 0x20) == 0)
        /* do nothing */
        ;

    /*
     * OK - enable window 0, which is the attribute memory window
     */
    vadem_write(PCAddWinEn + socket, 0x01);
}

/*
 *  Function: shutdown_slot
 *   Purpose: Turn off an unwanted PCMCIA slot
 *
 *    Params:
 *       Input: socket  The slot to be closed down
 *
 *   Returns: Nothing
 */
static void shutdown_slot(const unsigned int socket)
{
    /*
     * disable all I/O interrupts, and reset the card
     */
    vadem_write(PCIntGCtl + socket, 0x00);

    /*
     * disable all memory windows
     */
    vadem_write(PCAddWinEn + socket, 0x00);

    /*
     * remove the card's power
     */
    vadem_write(PCPwrRstCtl + socket, 0x20);
}

/*
 *  Function: finalise_slot
 *   Purpose: Reset the PCMCIA controller after a card has been removed
 *
 *    Params:
 *       Input: socket  Where the card has been removed from (passed
 *                      for convenience).
 *
 *      In/Out: pcs     Complete description of the socket the newly
 *                      removed card was in.
 *
 *   Returns: Nothing
 */
static void finalise_slot(PCMCIASocket *pcs, const unsigned int socket)
{
    shutdown_slot(socket);
    pcs->pcs_present = 0;
}

/*
 *  Function: initialise_card
 *   Purpose: Try to find a driver for a newly inserted PCMCIA card
 *
 *    Params:
 *      In/Out: pcs     Complete description of the socket the newly
 *                      inserted card is in.
 *
 *   Returns: Nothing
 *
 * Post-conditions: The handler (if any) that claimed the card will be
 *                      recorded in the socket description, along with
 *                      its ISR.
 */
static void initialise_card(struct PCMCIASocket *pcs)
{
    CardHandler *nexth;

    for( nexth = cards; *nexth != NULL; ++nexth)
        if ((*nexth)(pcs, Insert))
            break;
 
    if (*nexth == NULL)
    {
        /*
         * nobody wanted this card
         */
        LogInfo(LOG_PCMCIA, ("Unrecognised card inserted in PCMCIA socket %s\n",
                             (pcs->pcs_socket == PCSocketA) ? "A" : "B"));

        /*
         * disable power to the card
         */
        shutdown_slot(pcs->pcs_socket);
    }

    /*
     * OK, record the handler that claimed the card
     */
    pcs->pcs_cardhandler = *nexth;
}

/*
 *  Function: finalise_card
 *   Purpose: Tell installed driver that it's precious card has gone away
 *
 *    Params:
 *      In/Out: pcs     Complete description of the socket the newly
 *                      removed card was in.
 *
 *   Returns: Nothing
 */
static void finalise_card(PCMCIASocket *const pcs)
{
    /*
     * if a handler was registered for this card,
     * let it know that it has has gone away
     */
    if (pcs->pcs_cardhandler != NULL)
        (void)(pcs->pcs_cardhandler)(pcs, Removal);

    /*
     * the socket no longer has registered handlers, or I/O space
     * allocated to it
     */
    pcs->pcs_iobase = 0;
    pcs->pcs_iorange = 0;
    pcs->pcs_cardhandler = NULL;
    pcs->pcs_cardisr = NULL;
}

/*
 *  Function: card_insertion
 *   Purpose: Inform potential clients about a newly inserted card
 *
 *    Params:
 *       Input: socket  Where the card has been inserted (passed
 *                      for convenience).
 *
 *      In/Out: pcs     Complete description of the socket the newly
 *                      inserted card is in.
 *
 *   Returns: Nothing
 *
 * Post-conditions: The card is fully powered up, and the appropriate
 *                      handler has been informed of its presence.
 */
static void card_insertion(PCMCIASocket *const pcs,
                           const unsigned int socket)
{
    /*
     * first set up the PCMCIA slot, then try
     * to set up the card itself
     */
    initialise_slot(pcs, socket);

    initialise_card(pcs);
}

/*
 *  Function: card_removal
 *   Purpose: Inform client about a freshly removed card
 *
 *    Params:
 *       Input: socket  Where the card has been removed from (passed
 *                      for convenience).
 *
 *      In/Out: pcs     Complete description of the socket the newly
 *                      removed card was in.
 *
 *   Returns: Nothing
 */
static void card_removal(PCMCIASocket *const pcs, const unsigned int socket)
{
    /*
     * the card driver gets told about this first, then
     * we close down the PCMCIA slot
     */
    finalise_card(pcs);

    finalise_slot(pcs, socket);
}

/*
 *  Function: pcmcia_read_tuple
 *   Purpose: Read a single tuple from a PCMCIA card
 *
 *    Params:
 *       Input: pcs     Structure describing the slot the card is in.
 *
 *              offset  Offset in attribute memory for the start of the tuple.
 *
 *              buflen  Size of the buffer provided.
 *
 *      Output: buffer  Buffer for holding the tuple.
 *
 *   Returns:
 *          OK: Offset of next tuple, or 0 if no more tuples.
 */
static unsigned int pcmcia_read_tuple(const PCMCIASocket *const pcs,
                                      unsigned int offset,
                                      unsigned char *const buffer,
                                      unsigned int buflen)
{
    unsigned int tuple_type, tuple_length;
    unsigned int base = pcs->pcs_attrbase;
    unsigned char *cptr = buffer;

    /*
     * test 1 - load the tuple type
     */
    if ((tuple_type = PCMCIA_MEM_READ(base + offset)) == 0xff)
        /*
         * end of list, or not a tuple
         */
        return 0;

    /*
     * we are going to be returning at least a type and a link,
     * so get these into the buffer.
     */
    *(cptr++) = tuple_type;
    offset += 2;

    tuple_length = PCMCIA_MEM_READ(base + offset);
    *cptr = tuple_length;
    offset += 2;

    /*
     * test 2 - tuple link marking end of list
     */
    if (tuple_length == 0xff)
        /*
         * yup, it's the end of the list
         */
        return 0;

    /*
     * OK, keep copying until we reach the end of the
     * tuple, or run out of buffer space
     */
    buflen -= 2;
    if (buflen > tuple_length)
        buflen = tuple_length;

    while (buflen-- > 0)
    {
        *++cptr = PCMCIA_MEM_READ(base + offset);
        offset += 2;
    }

    /*
     * finished
     */
    return offset;
}

/**********************************************************************/

/*
 * Externally visible functions.  These functions are documented in
 * pcmcia.h
 */
#pragma no_check_stack
void angel_PCMCIAIntHandler(unsigned int ident, unsigned int devid,
                            unsigned int empty_stack)
{
    unsigned int socket;
    unsigned int irqtype;
    PCMCIASocket *pcs;

    IGNORE(devid);

    /*
     * check both IRQ source lines
     */
    if (ident == IH_PCMCIA_A)
    {
        pcs = &sockets[0];
        socket = PCSocketA;
    }
    else if (ident == IH_PCMCIA_B)
    {
        pcs = &sockets[1];
        socket = PCSocketB;
    }
    else
        /*
         * not for us
         */
        return;

    /*
     * first check whether this is a Card Detect Change interrupt
     */
    irqtype = vadem_read(PCCStatChng + socket);

    if (irqtype & 0x08)
    {
#ifdef OldCode
        /*
         * it is a card detect change
         */
        unsigned int cdbits, debounce;
        int dbloops = -1;

        /*
         * XXX
         *
         * We need to debounce the CD[0:1] signals.  The
         * VG-468 supposedly does this for us automatically,
         * but I can't get it to work properly yet (it does
         * debounce, but there seems to be some delay before
         * the card can be accessed after the interrupt comes
         * in), so we need to debounce in software instead
         */
        cdbits = vadem_read(PCIfStatus + socket) & 0x0c;
        do
        {
            int i;

            /*
             * the 100us delay time has been obtained by observation
             */
            delay(100);

            debounce = cdbits;
            ++dbloops;
        } while ((cdbits = vadem_read(PCIfStatus + socket) & 0x0c)
                 != debounce);

        LogInfo(LOG_PCMCIA, "Card change on socket %s, CD[1:0] = %s%s (%d loop%s)\n",
                (socket == PCSocketA) ? "A" : "B",
                ((cdbits & 0x08) == 0) ? "0" : "1",
                ((cdbits & 0x04) == 0) ? "0" : "1",
                dbloops, (dbloops == 1) ? "" : "s");

        /*
         * now see whether this is a card insertion, or a removal
         */
        if (cdbits == 0x0c)
        {
            /*
             * An insertion.  We need to be careful, because it is
             * possible to get two interrupts for the one insertion
             * (presumably because there is one IRQ for each transition
             * of CD[1:0] from 0->1, but by the time we read the debounced
             * status for the first IRQ, we already get both bits set).
             */
            if (pcs->pcs_present == 0)
                card_insertion(pcs, socket);
        }
        else if (cdbits == 0x00)
        {
            /*
             * A removal.
             */
            if (pcs->pcs_present != 0)
                card_removal(pcs, socket);
            else
                LogInfo(LOG_PCMCIA, ("Removal of unknown PCMCIA card from socket %s\n",
                                     (socket == PCSocketA) ? "A" : "B"));
        }
#else /* def OldCode */

        /*
         * it is a card detect change
         */
        unsigned int cdbits;

        /*
         * XXX
         *
         * It looks as though the original code mis-interpreted the
         * Vadem documentation: CD[0:1] *are* debounced (for an enormous
         * 65ms) upon card insertion, and are not debounced on card
         * removal.  If either of the bits are clear, then assume we
         * have just seen a card removal, otherwise a card insertion.
         */
        cdbits = vadem_read(PCIfStatus + socket) & 0x0c;

        LogInfo(LOG_PCMCIA, ("Card change on socket %s, CD[1:0] = %s%s\n",
                             (socket == PCSocketA) ? "A" : "B",
                             ((cdbits & 0x08) == 0) ? "0" : "1",
                             ((cdbits & 0x04) == 0) ? "0" : "1"));

        /*
         * now see whether this is a card insertion, or a removal
         */
        if (cdbits == 0x0c)
        {
            /*
             * An insertion.  We need to be careful, because it is
             * possible to get two interrupts for the one insertion
             * (presumably because there is one IRQ for each transition
             * of CD[1:0] from 0->1, but by the time we read the debounced
             * status for the first IRQ, we already get both bits set).
             */
            if (pcs->pcs_present == 0)
                card_insertion(pcs, socket);
        }
        else
        {
            /*
             * A removal.
             */
            if (pcs->pcs_present != 0)
                card_removal(pcs, socket);
            else
                LogInfo(LOG_PCMCIA, ("Removal of unknown PCMCIA card from socket %s\n",
                                     (socket == PCSocketA) ? "A" : "B"));
        }
#endif /* def OldCode */
    }
    else
    {
        /*
         * it is an I/O interrupt
         */
        LogInfo(LOG_PCMCIA, ("I/O IRQ from PCMCIA socket %s\n",
                             (socket == PCSocketA) ? "A" : "B"));

        if (pcs->pcs_cardisr != NULL)
            pcs->pcs_cardisr(pcs, empty_stack);
    }
}
#pragma check_stack

void pcmcia_setup(void)
{
    /*
     * Make sure that interrupts from PCMCIA cards are disabled
     */
    *IRQEnableClear = IRQ_CARDA | IRQ_CARDB;

    /*
     * VG-468 IRQs 4 & 3 are tied to AMBA interrupt lines
     * IRQ_CARDA and IRQ_CARDB respectively, set IRQ
     * steering as appropriate, and enable Card Detect
     * interrupts on both sockets
     */
    vadem_write(PCStatChngInt + PCSocketA, 0x48);
    vadem_write(PCStatChngInt + PCSocketB, 0x38);

#ifdef OldCode
    /*
     * Enable Auto Power switching
     */
    vadem_write(PCPwrRstCtl + PCSocketA, 0x20);
    vadem_write(PCPwrRstCtl + PCSocketB, 0x20);
#endif

    /*
     * clear status registers
     */
    (void)vadem_read(PCCStatChng + PCSocketA);
    (void)vadem_read(PCCStatChng + PCSocketB);

    /*
     * initialise socket descriptions, program standard
     * attribute windows, and shut the socket down
     */
    sockets[0].pcs_present = sockets[1].pcs_present = 0;

    sockets[0].pcs_socket = PCSocketA;
    sockets[0].pcs_attrbase = AttribMemory_A;
    sockets[0].pcs_attrrange = AttributeMemorySlot;
    map_memory(PCSocketA, 0, AttribMemory_A, AttributeMemorySlot,
               0, Attribute);
    shutdown_slot(PCSocketA);

    sockets[1].pcs_socket = PCSocketB;
    sockets[1].pcs_attrbase = AttribMemory_B;
    sockets[1].pcs_attrrange = AttributeMemorySlot;
    map_memory(PCSocketB, 0, AttribMemory_B, AttributeMemorySlot,
               0, Attribute);
    shutdown_slot(PCSocketB);

    /*
     * Enable card detect debouncing, and Tri-state
     * unused pins. We need to unlock the VG-468
     * unique registers before this will work
     */
    vadem_unlock();
#if 0
    vadem_write(PCControl + PCSocketA, 0x18);
    vadem_write(PCControl + PCSocketB, 0x18);
#else
    /*
     * TRACE - DISABLE DEBOUNCE AND POWER SEQUENCING
     */
    vadem_write(PCControl + PCSocketA, 0x00);
    vadem_write(PCControl + PCSocketB, 0x00);
#endif

    /*
     * Enable Interrupts from the cards
     */
    *IRQEnable = IRQ_CARDA | IRQ_CARDB;

    /*
     * test whether cards are already present or not
     */
    if ((vadem_read(PCIfStatus + PCSocketA) & 0x0c) == 0x0c)
    {
        LogInfo(LOG_PCMCIA, ("PCMCIA card already present in slot A\n"));
        card_insertion(&sockets[0], PCSocketA);
    }

    if ((vadem_read(PCIfStatus + PCSocketB) & 0x0c) == 0x0c)
    {
        LogInfo(LOG_PCMCIA, ("PCMCIA card already present in slot B\n"));
        card_insertion(&sockets[1], PCSocketB);
    }
}

unsigned char *pcmcia_find_tuple(const PCMCIASocket *const pcs,
                                 const unsigned int tuple,
                                 unsigned int *offset,
                                 unsigned char *buffer,
                                 const unsigned int buflen)
{
    unsigned int loff = *offset;

    /*
     * guard against potential false positives
     */
    *buffer = ~tuple;

    /*
     * keep going until the tuple is found, or proven not to exist
     */
    do
    {
        loff = pcmcia_read_tuple(pcs, loff, buffer, buflen);
    } while (loff != 0 && *buffer != tuple);

    *offset = loff;
    return (*buffer == tuple) ? buffer : NULL;
}

int pcmcia_mapio(PCMCIASocket *pcs, const unsigned int base,
                 const unsigned int range, const int mapirq)
{
    PCMCIASocket *altpcs;
    unsigned int socket = pcs->pcs_socket;
    unsigned int irqline;

    /*
     * As with the Card Detect Interrupts in pcmcia_setup(), we
     * need to route IRQs from sockets A and B to IRQ4 and IRQ3
     * repcetively
     */
    if (socket == PCSocketA)
    {
        irqline = 0x04;
        altpcs = &sockets[1];
    }
    else
    {
        irqline = 0x03;
        altpcs = &sockets[0];
    }

    /*
     * check that I/O ranges do not overlap
     */
    if ((base >= altpcs->pcs_iobase &&
         base <= altpcs->pcs_iobase + altpcs->pcs_iorange) ||
        (base + range >= altpcs->pcs_iobase &&
         base + range <= altpcs->pcs_iobase + altpcs->pcs_iorange))
        /*
         * looks like we've got ourselves an overlap here boy
         */
        return 0;

    /*
     * OK - program I/O range for window 0
     */
    vadem_write(PCIOA0StartL + socket, base & 0xff);
    vadem_write(PCIOA0StartH + socket, (base >> 8) & 0xff);
    vadem_write(PCIOA0StopL + socket, (base + range) & 0xff);
    vadem_write(PCIOA0StopH + socket, ((base + range) >> 8) & 0xff);

    /*
     * it is probably safe to assume that this socket contains
     * an I/O type card, so set the appropriate bit in the
     * control register
     */
    vadem_write(PCIntGCtl + socket, 0x60 | ((mapirq) ? irqline : 0));

    /*
     * XXX
     *
     * TODO - FIX THIS CRAP
     *
     * This is by no means general, but at the moment I don't know how
     * to extract the relevant information, and can't be bothered to
     * implement a general scheme for it anyway, so for the time being
     * it stays as a hard wired number.
     */
    vadem_write(PCIOCtl + socket, 0x00);

    /*
     * enable I/O window 0
     */
    vadem_write(PCAddWinEn + socket, vadem_read(PCAddWinEn + socket) | 0x60);

    return 1;
}

#if DEBUG == 1
extern void vadem_dump(unsigned int socket);

void vadem_dump(unsigned int socket)
{
    LogInfo(LOG_PCMCIA, ("\nRegister dump of Vadem VG-468 for socket %s:\n",
              (socket == PCSocketA) ? "A" : "B"));

    LogInfo(LOG_PCMCIA, ("%02x  %02x  %02x  %02x  %02x\n",
              vadem_read(socket + 0x00),
              vadem_read(socket + 0x01),
              vadem_read(socket + 0x02),
              vadem_read(socket + 0x04),
              vadem_read(socket + 0x06)));

    LogInfo(LOG_PCMCIA, ("%02x  %02x  %02x  %02x  %02x\n",
              vadem_read(socket + 0x16),
              vadem_read(socket + 0x1e),
              vadem_read(socket + 0x03),
              vadem_read(socket + 0x05),
              vadem_read(socket + 0x07)));

    LogInfo(LOG_PCMCIA, ("%02x  %02x  %02x  %02x\n",
              vadem_read(socket + 0x08),
              vadem_read(socket + 0x09),
              vadem_read(socket + 0x0a),
              vadem_read(socket + 0x0b)));

    LogInfo(LOG_PCMCIA, ("%02x  %02x  %02x  %02x\n",
              vadem_read(socket + 0x0c),
              vadem_read(socket + 0x0d),
              vadem_read(socket + 0x0e),
              vadem_read(socket + 0x0f)));

    LogInfo(LOG_PCMCIA, ("%02x  %02x  %02x  %02x  %02x %02x\n\n",
              vadem_read(socket + 0x10),
              vadem_read(socket + 0x11),
              vadem_read(socket + 0x12),
              vadem_read(socket + 0x13),
              vadem_read(socket + 0x14),
              vadem_read(socket + 0x15)));
}
#endif /* def DEBUG */

/* EOF pcmcia.c */
