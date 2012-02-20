/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.1 $
 *   $Author: kwelton $
 *     $Date: 1996/07/22 16:52:51 $
 *
 * pcmcia.h - function prototypes for Vadem VG-468 PCMCIA Interface Controller
 *
 */

#ifndef etherd_pcmcia_h
#define etherd_pcmcia_h

/*
 * Events for CardHandler
 */
typedef enum
{
    Insert,
    Removal
} CardEvent;

struct PCMCIASocket;                                    /* XXX */

/*
 *  Function: CardISR
 *   Purpose: Routine registered as the ISR for interrupts from a PCMCIA
 *              card.
 *
 *    Params:
 *       Input: pcs             The PCMCIA socket which generated the interrupt
 *
 *              empty_stack     Top of the current mode's stack.  Passed
 *                              along to serialiser functions.
 *
 *   Returns: Nothing
 */
typedef void (*CardISR)(const struct PCMCIASocket *pcs,
                        unsigned int empty_stack);

/*
 *  Function: CardHandler
 *   Purpose: Routine called when a card insertion/removal event is
 *              detected by the PCMCIA controller.
 *
 *    Params:
 *       Input: event   The event being notified
 *
 *      In/Out: pcs     PCMCIA socket generating the event
 *
 *   Returns:
 *          OK: 1 - Handler has recognised the card
 *              0 - Handler does not recognise the card
 *
 * Post-conditions: The routine will have updated pcs->pcs_cardisr with
 *                      the address of the ISR to call when a card-generated
 *                      interrupt is received from the socket; this can be
 *                      NULL if there is no ISR to register.
 */
typedef int (*CardHandler)(struct PCMCIASocket *const pcs,
                           const CardEvent event);

/*
 * structure describing a PCMCIA socket.
 */
typedef struct PCMCIASocket
{
    unsigned char pcs_present;
    unsigned char pcs_socket;
    unsigned int  pcs_attrbase;
    unsigned int  pcs_attrrange;
    unsigned int  pcs_iobase;
    unsigned int  pcs_iorange;
    CardHandler   pcs_cardhandler;
    CardISR       pcs_cardisr;
} PCMCIASocket;

/*
 *  Function: angel_PCMCIAIntHandler
 *   Purpose: Entry point for interrupts from the PCMCIA Controller.
 *            See documentation for angel_IRQHandlerFn in devdriv.h
 */
void angel_PCMCIAIntHandler(unsigned int ident, unsigned int devid,
                            unsigned int empty_stack);

/*
 *  Function: pcmcia_setup
 *   Purpose: Initialise PCMCIA controller
 *
 *  Pre-conditions: Handlers have been installed for the two PCMCIA
 *                      interrupt sources.
 *
 *    Params: None
 *
 *   Returns: Nothing
 *
 * Post-conditions: Registered card handlers will have been called if
 *                      a card already found to be inserted in either
 *                      slot.
 */
void pcmcia_setup(void);

/*
 *  Function: pcmcia_find_tuple
 *   Purpose: Read a specific tuple for a PCMCIA card
 *
 *    Params:
 *       Input: pcs     Structure describing the slot the card is in
 *
 *              tuple   The Tuple Code to look for, with 0xff interpreted
 *                      as a wildcard.
 *
 *              buflen  Size of buffer provided.
 *
 *      In/Out: offset  Where to start searching for the tuple; will be
 *                      returned marking the first tuple after the one
 *                      being returned.  Passing 0 interpreted as "start
 *                      of tuple chain", returned as 0 when end of chain
 *                      reached.
 *
 *              buffer  Space to hold the tuple
 *
 *   Returns:
 *          OK: Pointer to buffer containing the requested tuple
 *       Error: NULL
 */
unsigned char *pcmcia_find_tuple(const PCMCIASocket *pcs,
                                 const unsigned int tuple,
                                 unsigned int *offset,
                                 unsigned char *buffer,
                                 const unsigned int buflen);

/*
 *  Function: pcmcia_mapio
 *   Purpose: Try to map some I/O space for a card driver
 *
 *    Params:
 *       Input: base    I/O base address
 *
 *              range   I/O address range
 *
 *              mapirq  Flag whether to enable (1) or disable (0) I/O
 *                      interrupts for the card.
 *
 *      In/Out: pcs     PCMCIA socket the card is in.
 *
 *   Returns:
 *          OK: 1 - I/O Mapped
 *       Error: 0 - I/O range clash
 */
int pcmcia_mapio(PCMCIASocket *pcs, const unsigned int base,
                 const unsigned int range, const int mapirq);


#endif /* ndef etherd_pcmcia_h */

/* EOF pcmcia.h */
