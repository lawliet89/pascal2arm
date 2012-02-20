/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.3.6.5 $
 *   $Author: rivimey $
 *     $Date: 1997/12/22 14:15:33 $
 *
 * olicom.c - device driver for  Intel 82595 controller used in
 * the Olicom EtherCom PCMCIA Ethernet card
 */
#include <stdio.h>
#include <string.h>

#include "pcmcia.h"
#include "pid.h"
#include "olicom.h"
#include "82595.h"
#include "arm.h"
#include "serlock.h"

#include "angel.h"
#include "logging.h"
#include "support.h"

/*
 * Fusion includes
 */
#include "std.h"
#include "enet.h"
#include "flags.h"
#include "netdev.h"
#include "netioc.h"
#include "m.h"

/*
 * I *really* don't know why Pacific Softworks
 * don't have this in a header file
 */
extern a48 host_addr;

/*
 * structure describing the interface, supplied by higher levels of Fusion
 */
static struct netdev *Netdev = NULL;

/*
 * forward reference to ISR routine
 */
static void olicom_isr(const PCMCIASocket *pcs, unsigned int empty_stack);

/*
 * State Machine state
 */
static OlicomState state = eth_ABSENT;

/**********************************************************************/

/*
 * PCMCIA support functions
 */

/*
 *  Function: identify_card
 *   Purpose: Test whether a PCMCIA card is the Olicom Ethernet card
 *
 *    Params:
 *       Input: pcs     PCMCIA Socket containing the card
 *
 *   Returns:
 *          OK: 1 - Yes, it's an Olicom card
 *              0 - No, it's something else
 */
static int identify_card(const PCMCIASocket *const pcs)
{
    unsigned char buffer[256], *cptr, *dptr;/* cptr and dptr are pointers*/ 
    unsigned int offset = 0;                /* for the comparison of the */
    /* Ethernet or Ethercom strings which were changed by Olicom - dawb  */
    
    /*
     * try to find the Level 1 Version tuple for the card
     */
    if ((cptr = pcmcia_find_tuple(pcs, 0x15, &offset,
                                  buffer, sizeof(buffer))) == NULL)
    {
        LogInfo(LOG_OLICOM, ("olicom: can't find Level 1 Version tuple\n"));
        return 0;
    }

    /*
     * OK, we need to check both the manufacturer and product strings,
     * the first of which starts at offset 4 within the tuple.
     */
    cptr = buffer + 4;
    if (__rt_strcmp((char *)cptr, "Olicom") != 0)
    {
        LogInfo(LOG_OLICOM, ("olicom: unknown manufacturer `%s'\n", cptr));
        return 0;
    }

    cptr += __rt_strlen((char *)cptr) + 1;
    dptr = cptr;

    /* This has had to be changed as Olicom changed the identification */
    /* strings without telling anyone !! - dawb 16/6/97 */

    if (__rt_strcmp((char *)cptr, "Ethercom") != 0 && 
        __rt_strcmp((char *)dptr, "Ethernet") != 0)
    {
        LogInfo(LOG_OLICOM, ("olicom: unknown product `%s'\n", cptr));
        return 0;
    }

    return 1;
}

/*
 *  Function: read_card_params
 *   Purpose: Read configuration parameters from Olicom Ethernet card
 *
 *    Params:
 *       Input: pcs     PCMCIA socket generating the event
 *
 *      Output: isr     The ISR to call when a card-generated interrupt is
 *                      received from the socket.
 *
 *   Returns:
 *          OK: 1
 *       Error: 0
 *
 * Post-conditions: All configuration tuples will have been read from
 *                      the card into the hardware description structure,
 *                      and the PCMCIA I/O window will be open.
 *
 *                  The routine will have updated pcs->pcs_cardisr with
 *                      the address of the ISR to call when a card-generated
 *                      interrupt is received from the socket.
 */
static int read_card_params(PCMCIASocket *const pcs)
{
    unsigned char buffer[256], *cptr;
    unsigned int offset = 0;
    unsigned int iobase;
    a48 hwaddr;
    ethercom *const oc = &(Netdev->nd_olicom);

    /*
     * There are four tuples that we need to read from the card:
     *
     * FUNCID (0x21) - Should identify the card as an Network Adaptor
     *
     * FUNCE (0x22) - Function extension tuple detailing the Ethernet
     * hardware address
     *
     * CONFIG (0x1a) - Describes card configuration registers
     *
     * CFTABLE_ENTRY (0x1b) - There are several different copies of this
     * one tuple, which describe the different I/O addresses accepted by
     * the Intel 82595 Controller.
     *
     * These tuples should be in the order listed, so we start searching
     * for the next tuple from where the previous one finished, but we
     * also try again from the beginning if the search failed.
     */

    /*
     * tuple 1 - FUNCID
     */
    if ((cptr = pcmcia_find_tuple(pcs, 0x21, &offset,
                                  buffer, sizeof(buffer))) == NULL)
    {
        LogInfo(LOG_OLICOM, ("olicom: can't find FUNCID tuple\n"));
        return 0;
    }

    if (*(cptr + 2) != 0x06)
    {
        LogInfo(LOG_OLICOM, ("olicom: bad FUNCID 0x%02x\n", *(cptr + 2)));
        return 0;
    }

    /*
     * tuple 2 - FUNCE
     */
    if ((cptr = pcmcia_find_tuple(pcs, 0x22, &offset,
                                  buffer, sizeof(buffer))) == NULL)
    {
        /*
         * try again, from the beginning
         */
        offset = 0;

        if ((cptr = pcmcia_find_tuple(pcs, 0x22, &offset,
                                      buffer, sizeof(buffer))) == NULL)
        {
            LogInfo(LOG_OLICOM, ("olicom: can't find FUNCE tuple\n"));
            return 0;
        }
    }

    /*
     * this should be flagged as a node id tuple (byte 2 = 0x04), with
     * a length (byte 3) of 6 bytes
     */
    if (*(cptr + 2) != 0x04 || *(cptr + 3) != 0x06)
    {
        LogInfo(LOG_OLICOM, ("olicom: Bad FUNCE\n"));
        return 0;
    }

    /*
     * OK, copy hardware address and pass it to higher Fusion layers
     */
    (void)__rt_memcpy(hwaddr, cptr + 4, 6);
    brd_host_addr(&hwaddr);

    /*
     * tuple 3 - CONFIG
     */
    if ((cptr = pcmcia_find_tuple(pcs, 0x1a, &offset,
                                  buffer, sizeof(buffer))) == NULL)
    {
        /*
         * try again, from the beginning
         */
        offset = 0;

        if ((cptr = pcmcia_find_tuple(pcs, 0x1a, &offset,
                                      buffer, sizeof(buffer))) == NULL)
        {
            LogInfo(LOG_OLICOM, ("olicom: can't find CONFIG tuple\n"));
            return 0;
        }
    }

    /*
     * OK, bytes 4 and 5 of the tuple contain the address (in attribute
     * memory) of the card configuration registers (lsb first), byte 6
     * contains the bitmap of which registers are present
     */
    if (*(cptr + 6) != 0x03)
    {
        LogInfo(LOG_OLICOM, ("olicom: unexpected number of card registers (0x%02x)\n",
                *(cptr + 6)));
        return 0;
    }

    oc->oc_cardbase = (*(cptr + 5) << 8) | *(cptr + 4);

    /*
     * tuple 4 - CFTABLE_ENTRY
     *
     * I must confess to a certain level of ignorance here: using the
     * version of the PCMCIA Metaformat Specification that we have here
     * (November 1995) I cannot fully decode the default Configuration
     * Table Entry supplied by the Olicom Ethernet Card.  However, I
     * *can* understand the I/O Space Descriptions, of which the Olicom
     * card supplies several (because it does not fully decode the I/O
     * address); fortuitously, for all non-default CFTABLE_ENTRY tuples,
     * these I/O Space Descriptions are the first descriptions within
     * the tuple, and therefore easy to find.
     */
    if ((cptr = pcmcia_find_tuple(pcs, 0x1b, &offset,
                                  buffer, sizeof(buffer))) == NULL)
    {
        /*
         * try again, from the beginning
         */
        offset = 0;

        if ((cptr = pcmcia_find_tuple(pcs, 0x1b, &offset,
                                      buffer, sizeof(buffer))) == NULL)
        {
            LogInfo(LOG_OLICOM, ("olicom: can't find any CFTABLE_ENTRY tuples\n"));
            return 0;
        }
    }

    /*
     * keep going until we get some I/O space mapped
     */
    for (;;)
    {
        /*
         * don't bother trying if this is the default configuration
         * tuple (identified by bit 6 of byte 2); can't cope with the
         * Interface Description field (flagged by bit 7, byte 2) being
         * present either.
         */
        if (((*(cptr + 2)) & 0xc0) == 0)
        {
            /*
             * Paranoia (i.e. limited implementation) checks:
             *
             * Bit 3 of byte 3 flags an I/O description structure;
             * this should be the lsb set in the byte.
             *
             * Byte 5 describes the I/O range - we can only
             * cope with a specific case (the general case
             * is neither hard, nor worth implementing).
             */
            if ((*(cptr + 3) & 0x0f) == 0x08 && *(cptr + 5) == 0x60)
            {
                /*
                 * OK, we can cope with this tuple: bytes 6 and 7
                 * give the I/O base address, and byte 8 gives the
                 * range (which should be 0x0f)
                 */
                if (*(cptr + 8) != 0x0f)
                {
                    LogInfo(LOG_OLICOM, ("olicom: bad number of I/O registers (%d)\n",
                                *(cptr + 8)));
                    return 0;
                }

                /*
                 * pull out the base address, and try to get this
                 * mapped into the PCMCIA controller's I/O space;
                 * note that we also ask for Interrupts to be "enabled"
                 * by the PCMCIA controller, but no interrupt sources
                 * on the card itself are enabled yet.
                 */
                iobase = (*(cptr + 7) << 8) | *(cptr + 6);
                if (pcmcia_mapio(pcs, iobase, 0x0f, 1))
                {
                    /*
                     * calculate the actual base address of the
                     * chip in I/O space
                     */
                    oc->oc_82595base = PCMCIA_CALC_ADDR(iobase) + NISA_IO_BASE;

                    LogInfo(LOG_OLICOM, ("olicom: I/0 mapped at 0x%04x (%08x)\n",
                            iobase, oc->oc_82595base));

                    /*
                     * mark the card as present, and keep a record of
                     * the PCMCIA socket
                     */
                    Netdev->nd_olicom.oc_pcs = pcs;

                    /*
                     * everything's done, apart from "registering"
                     * an ISR handler
                     */
                    pcs->pcs_cardisr = olicom_isr;

                    return 1;
                }
            }
        }

        if ((cptr = pcmcia_find_tuple(pcs, 0x1b, &offset,
                                      buffer, sizeof(buffer))) == NULL)
        {
            LogInfo(LOG_OLICOM, ("olicom: can't find any non-default"
                    " CFTABLE_ENTRY tuples\n"));
            return 0;
        }
    }
}

/*
 *  Function: handle_insertion
 *   Purpose: See whether the newly inserted PCMCIA card is of any interest
 *              to this driver.
 *
 *    Params: (Contained in args)
 *      In/Out: pcs     PCMCIA socket generating the event
 *
 *   Returns:
 *          OK: 1 - This is an Olicom card, and we are interested in it.
 *              0 - We are not interested in this card for some reason
 *
 * Post-conditions: All configuration tuples will have been read from
 *                      the card into the hardware description structure,
 *                      and the PCMCIA I/O window will be open.
 */
static int handle_insertion(void *const args)
{
    PCMCIASocket *const pcs = (PCMCIASocket *)args;

    /*
     * test whether this is in Olicom card
     */
    if (!identify_card(pcs))
    {
        LogInfo(LOG_OLICOM, ("handle_insertion: not an Olicom card\n"));

        return 0;
    }

    /*
     * this *is* an Olicom card - see whether we can
     * cope with it yet
     */
    if (Netdev == NULL)
    {
        LogError(LOG_OLICOM, ("Olicom card present, but system not yet initialised!\n"));
        return 0;
    }

    /*
     * looks like it is, try to read the required parameters
     */
    return read_card_params(pcs);
}

/*
 *  Function: handle_removal
 *   Purpose: Deal with one of our cards being removed from its socket
 *
 *    Params: (via args)
 *       Input: pcs     PCMCIA socket generating the event
 *
 *   Returns: Nothing
 */
static void handle_removal(void *args)
{
    IGNORE(args);

    LogInfo(LOG_OLICOM, ("olicom: Card has been removed\n"));
}

/**********************************************************************/

/*
 * The Olicom State Machine
 */

/*
 *  Function: st_absent
 *   Purpose: Handle events received when in eth_ABSENT state
 *
 *  Pre-conditions: All further events are disabled
 *
 *    Params:
 *       Input: event   The event to be processed
 *      In/Out: args    Pointer to event-specific arguments
 *      Output: state   New state
 *
 *   Returns: Nothing
 */
static void st_absent(OlicomState *const state, const OlicomEvent event,
                      void *const args)
{
    /*
     * the only event that means anything to us is a card insertion
     */
    if (event == ev_INSERTION)
    {
        if (handle_insertion(args))
            *state = eth_DOWN;
    }
    else if (event != ev_TAKEDOWN)
        LogInfo(LOG_OLICOM, ("st_absent: unexpected event %d\n", event));
}

/*
 *  Function: st_down
 *   Purpose: Handle events received when in eth_DOWN state
 *
 *  Pre-conditions: All further events are disabled
 *
 *    Params:
 *       Input: event   The event to be processed
 *      In/Out: args    Pointer to event-specific arguments
 *      Output: state   New state
 *
 *   Returns: Nothing
 */
static void st_down(OlicomState *const state, const OlicomEvent event,
                    void *const args)
{
    switch (event)
    {
      case ev_REMOVAL:
          /*
           * this is the only state where removal is not an error
           */
          (void)handle_removal(args);
          *state = eth_ABSENT;

          break;

      case ev_BRINGUP:
          if (i595_bringup(args))
              *state = eth_IDLE;

          break;

      case ev_TAKEDOWN:
          break;

      default:
          LogInfo(LOG_OLICOM, ("st_down: unexpected event %d\n", event));
          break;
    }
}

/*
 *  Function: st_idle
 *   Purpose: Handle events received when in eth_IDLE state
 *
 *  Pre-conditions: All further events are disabled
 *
 *    Params:
 *       Input: event   The event to be processed
 *      In/Out: args    Pointer to event-specific arguments
 *      Output: state   New state
 *
 *   Returns: Nothing
 */
static void st_idle(OlicomState *const state, const OlicomEvent event,
                    void *const args)
{
    switch (event)
    {
      case ev_REMOVAL:
          /*
           * this is unfortunate
           */
          (void)handle_removal(args);
          *state = eth_ABSENT;
          
          LogInfo(LOG_OLICOM, ("st_idle: unexpected ev_REMOVAL\n"));

          break;

      case ev_BRINGUP:
          /*
           * don't consider this an error
           */
          break;

      case ev_TAKEDOWN:
          if (i595_takedown(args))
              *state = eth_DOWN;

          break;

      case ev_TxPACKET:
          if (i595_txstart(args))
              *state = eth_TxACTIVE;

          break;

      case ev_RxPACKET:
      case ev_RxOVERFLOW:
          i595_rxpackets(args);
          break;

      default:
          LogInfo(LOG_OLICOM, ("st_idle: unexpected event %d\n", event));
          break;
    }
}

/*
 *  Function: st_txactive
 *   Purpose: Handle events received when in eth_TxACTIVE state
 *
 *  Pre-conditions: All further events are disabled
 *
 *    Params:
 *       Input: event   The event to be processed
 *      In/Out: args    Pointer to event-specific arguments
 *      Output: state   New state
 *
 *   Returns: Nothing
 */
static void st_txactive(OlicomState *const state, const OlicomEvent event,
                        void *const args)
{
    switch (event)
    {
      case ev_REMOVAL:
          /*
           * this is unfortunate
           */
          (void)handle_removal(args);
          *state = eth_ABSENT;

          LogInfo(LOG_OLICOM, ("st_txactive: unexpected ev_REMOVAL\n"));

          break;

      case ev_BRINGUP:
          /*
           * don't consider this an error
           */
          break;

      case ev_TAKEDOWN:
          if (i595_takedown(args))
              *state = eth_DOWN;

          break;

      case ev_TxPACKET:
          i595_txchain(args);
          break;

      case ev_RxPACKET:
      case ev_RxOVERFLOW:
          i595_rxpackets(args);
          break;

      case ev_TxGONE:
          if (!i595_packetgone(args))
              /*
               * all packets have been transmitted - chip is now idle
               */
              *state = eth_IDLE;

          break;

      default:
          LogInfo(LOG_OLICOM, ("st_txactive: unexpected event %d\n", event));
          break;
    }
}

/*
 *  Function: st_cmdactive
 *   Purpose: Handle events received when in eth_CMDACTIVE state
 *
 *  Pre-conditions: All further events are disabled
 *
 *    Params:
 *       Input: event   The event to be processed
 *      In/Out: args    Pointer to event-specific arguments
 *      Output: state   New state
 *
 *   Returns: Nothing
 */
static void st_cmdactive(OlicomState *const state, const OlicomEvent event,
                         void *const args)
{
    switch (event)
    {
      case ev_REMOVAL:
          /*
           * this is unfortunate
           */
          (void)handle_removal(args);
          *state = eth_ABSENT;

          LogInfo(LOG_OLICOM, ("st_cmdactive: unexpected ev_REMOVAL\n"));

          break;

      default:
          LogInfo(LOG_OLICOM, ("st_cmdactive: unexpected event %d\n", event));
          break;
    }
}

/*
 *  Function: olicom_state
 *   Purpose: Dispatch State Machine Events to appropriate handler
 *
 *  Pre-conditions: This function is called under the Angel serialiser,
 *                      so it is automatically prevented from re-entrancy.
 *
 *    Params:
 *       Input: event   The event to be processed
 *      In/Out: args    Pointer to event-specific arguments
 *
 *   Returns: Nothing
 *
 * Post-conditions: State will have been updated to reflect the received
 *                      event.
 */
static void olicom_state(const unsigned int event, void *const args)
{
    /*
     * disable interrupts from the Ethernet Controller
     */
    unsigned int s1 = os_critical();

#if DEBUG == 1
    OlicomState oldstate = state;
#endif

    /*
     * create a local jump table for the various states
     */
    static void (*jumptab[])(OlicomState *const state,
                             const OlicomEvent event,
                             void *const args) =
    {
        st_absent, st_down, st_idle, st_txactive, st_cmdactive
    };

    (void)(jumptab[state])(&state, (const OlicomEvent)event, args);

    /*
     * re-enable interrupts from the Ethernet Controller
     */
    os_normal(s1);

    LogInfo(LOG_OLICOM, ("olicom_state: event %d, state %d -> %d\n",
                  event, oldstate, state));
}

/*
 *  Function: olicom_process
 *   Purpose: Serialised ISR processing for Olicom card
 *
 *    Params:
 *       Input: s1      Previous mode value, as set in ISR (via args)
 *
 *   Returns: Nothing
 *
 *   Preconditions: Must only be called from olicom_isr via Serialiser.
 */
static void olicom_process(void *args)
{
    ethercom *oc = &Netdev->nd_olicom;
    unsigned int chip = oc->oc_82595base;
    unsigned int irqset;

    unsigned int s1 = (unsigned int)args;

    LogInfo(LOG_OLICOM, ( "In olicom_process" ));

    /*
     * to reduce latency we only service one interrupt at a time
     */
    if ((irqset = PCMCIA_IO_READ(chip + I595_R1) & ALL_INT_BITS) != 0)
    {
        if ((irqset & RXSTP_INT) != 0)
        {
            PCMCIA_IO_WRITE(chip + I595_R1, RXSTP_INT);
            olicom_state(ev_RxOVERFLOW, Netdev);
        }

        else if ((irqset & RX_INT) != 0)
        {
            PCMCIA_IO_WRITE(chip + I595_R1, RX_INT);
            olicom_state(ev_RxPACKET, Netdev);
        }

        else if ((irqset & TX_INT) != 0)
        {
            PCMCIA_IO_WRITE(chip + I595_R1, TX_INT);
            olicom_state(ev_TxGONE, Netdev);
        }

        else if ((irqset & EXEC_INT) != 0)
        {
            PCMCIA_IO_WRITE(chip + I595_R1, EXEC_INT);
            olicom_state(ev_EXECFINISHED, Netdev);
        }
    }

    /* if there is anything else to do, we re-queue ourselves */
    if ((PCMCIA_IO_READ(chip + I595_R1) & ALL_INT_BITS) != 0)
    {
        Angel_QueueCallback( (angel_CallbackFn)olicom_process, 
                             TP_AngelWantLock,
                             (void *)s1, 0, 0, 0 );
    }
    else
    {
        /* re-enable ethernet interrupts to previous level */
        os_normal( s1 );
    }
}

/*
 *  Function: olicom_isr
 *   Purpose: ISR for Olicom card, registered with PCMCIA controller
 *
 *    Params:
 *       Input: pcs             PCMCIA socket generating the interrupt
 *
 *              empty_stack     Top of the current mode's stack.  Passed
 *                              along to serialiser functions.
 *
 *   Returns: Nothing
 */
#pragma no_check_stack
static void olicom_isr(const PCMCIASocket *const pcs,
                       unsigned int empty_stack)
{
    ethercom *oc = &Netdev->nd_olicom;
    unsigned int s1;

    /*
     *
     * Sanity Checks:
     *
     * 1) See whether the card really *is* interrupting.  This is
     *    flagged in bit 1 of Card Configuration Register 1.
     *
     * 2) Check that the interrupt source is what we expect
     */
    if ((PCMCIA_MEM_READ(pcs->pcs_attrbase + CardConfReg1) & 0x02) == 0 ||
        pcs != oc->oc_pcs)
    {
        /*
         * this really should not happen!
         */
        LogError(LOG_OLICOM, ("olicom: UNEXPECTED INTERRUPT!\n"));

        /*
         * disable interrupt source, this will break
         * the driver, but what else can we do?
         */
        if (pcs->pcs_socket == PCSocketA)
            *IRQEnableClear = IRQ_CARDA;
        else
            *IRQEnableClear = IRQ_CARDB;

        return;
    }

    /* disable interrupts from this source */
    s1 = os_critical();

    /*
     * it really *is* an interrupt - serialise handling of it
     */
    Angel_SerialiseTask(0,
                        olicom_process, (void *)s1,
                        empty_stack);

    /* NEVER GET HERE */
}
#pragma check_stack

/**********************************************************************/

/*
 * These are the driver routines which interface to the Fusion Link
 * Layer.  Their required interfaces are documented in the Fusion
 * Porting Manual
 */

/*
 * INIT (page 6-7)
 *
 * This routine should be called *before* initialising the PCMCIA
 * controller, i.e. before any card already present or subsequently
 * inserted is recognised.  Since we have no hardware, there is *very*
 * little we can do here.
 */
int olicom_init(netdev *nd)
{
    Netdev = nd;

    /*
     * it is now safe to start the PCMCIA controller
     */
    pcmcia_setup();

    return 0;
}

/*
 * UPDOWN (page 6-8)
 *
 * This is only the second interface routine, and I am already starting
 * to develop a dislike of the Fusion code:
 *
 * 1)   *WHY* does every driver routine have to copy the hardware address
 *      into the netdev structure?  Why can't the calling routine do it?
 *
 * 2)   The Porting Manual documents this function as having two arguments,
 *      but the function prototypes give it a third (options).  This is
 *      just bollocks.
 */
int olicom_updown(netdev *nd, u16 flags, char *options)
{
    IGNORE(options);

    if (flags != 0)
    {
        /*
         * this *SHOULD NOT* need to be in here, but who
         * am I to argue?
         */
        (void)__rt_memcpy(nd->nd_lladdr.a_ena, host_addr, 6);
        nd->nd_lladdr.a_type = AF_ETHER;
        nd->nd_lladdr.a_len = 6;

        olicom_state(ev_BRINGUP, nd);

        /*
         * Unfortunately, there is no way of getting success/failure
         * results back from the state machine, so we have to bodge
         * the results here. =8(
         */
        return (state == eth_IDLE) ? 0 : -1;
    }
    else
    {
        olicom_state(ev_TAKEDOWN, nd);

        return (state == eth_DOWN) ? 0 : -1;
    }
}

/*
 * START (page 6-9)
 */
int olicom_start(m *mp)
{
    olicom_state(ev_TxPACKET, mp);

    /*
     * Unfortunately, we have no way of passing state back through
     * the serialiser, so we have to pretend that the packet went
     * out OK, even when it may not have done.  This is OK, since
     * the higher levels have to be able to cope with packets lost
     * at a level below our control (e.g. Excess Collisions, CRC
     * failures etc.).
     */
    return 0;
}

/*
 * IOCTL (page 6-10)
 */
int olicom_ioctl(netdev *nd, int cmnd)
{
    int s = os_critical();
    int retc = 0;

    /*
     * the chances of getting this correct are minimal, thanks to
     * the woefully inadequate documentation from Pacific Softworks
     */
    switch (cmnd)
    {
      case ENIOCPROMISC:
          i595_promiscmode(&nd->nd_olicom, TRUE);
          break;

      case ENIOCALL:
          i595_promiscmode(&nd->nd_olicom, FALSE);
          i595_badpacketrx(&nd->nd_olicom, TRUE);
          break;

      case ENIOCNORMAL:
          i595_promiscmode(&nd->nd_olicom, FALSE);
          i595_badpacketrx(&nd->nd_olicom, FALSE);
          break;

      case ENIOCRESET:
          /*
           * this is not supported yet - we need to
           * integrate it into the state machine
           *
           * fall through to...
           */

      default:
          if (cmnd == ENIOCRESET)
              LogWarning(LOG_OLICOM, ("olicom_ioctl: unsupported operation ENIOCRESET\n"));
          else
              LogWarning(LOG_OLICOM, ("olicom_ioctl: bad operation: %#x\n", cmnd));
          retc = EOPNOTSUPP;
          break;
    }

    os_normal(s);
    return retc;
}

/**********************************************************************/

/*
 * Externally visible functions.  These functions are documented in
 * olicom.h
 */
int olicom_card_handler(PCMCIASocket *const pcs, const CardEvent event)
{
    if (event == Insert)
    {
        olicom_state(ev_INSERTION, pcs);

        /*
         * Once again, we are bitten by the lack of results passed
         * back through the Serialiser from the State Machine, so
         * we have to bodge things here.
         */
        return (state == eth_DOWN) ? 1 : 0;
    }
    else
    {
        olicom_state(ev_REMOVAL, pcs);

        return 1;
    }
}

#pragma no_check_stack
int os_critical(void)
{
    const unsigned int s = *IRQEnable & (IRQ_CARDA | IRQ_CARDB);

    *IRQEnableClear = IRQ_CARDA | IRQ_CARDB;

    return s;
}

void os_normal(int s)
{
    *IRQEnableSet = s;
}
#pragma check_stack

/* EOF olicom.c */
