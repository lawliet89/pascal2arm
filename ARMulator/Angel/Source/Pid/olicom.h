/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.1 $
 *   $Author: kwelton $
 *     $Date: 1996/07/22 16:52:48 $
 *
 * olicom.h - function prototypes for Olicom EtherCom PCMCIA Ethernet card
 * controller
 */

#ifndef etherd_olicom_h
#define etherd_olicom_h

#include "angel.h"
#include "pcmcia.h"

/*
 *  Function: olicom_card_handler
 *   Purpose: Called when PCMCIA controller has detected a card insertion
 *              event, this routine looks to see whether it is an Olicom
 *              Ethernet Card, and registers the card handler if it is.
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
 *                      interrupt is received from the socket.
 *
 *                  The card's I/O space will be mapped and the card enabled,
 *                      but card interrupts will be disabled.
 */
int olicom_card_handler(PCMCIASocket *const pcs, const CardEvent event);

/**********************************************************************/

/*
 * Assembler support functions in etherser.s
 */

/*
 *  Function: olicom_irqson
 *   Purpose: Re-enable interrupts in CPSR if in non-USR mode
 *
 *    Params: None
 *
 *   Returns: Interrupt state upon entry
 */
unsigned int olicom_irqson(void);

/*
 *  Function: olicom_irqsoff
 *   Purpose: Restore interrupt state (turn them off)
 *
 *    Params:
 *       Input: state   State returned from olicom_irqson
 *
 *   Returns: Nothing
 */
void olicom_irqsoff(const unsigned int state);

/**********************************************************************/

/*
 * low-level driver functions from 82595.c
 */

/*
 *  Function: i595_bringup
 *   Purpose: bring the 82595 up live onto the network
 *
 *    Params: (as args)
 *       Input: nd      Structure describing the network interface
 *
 *   Returns:
 *          OK: 1
 *       Error: 0
 */
int i595_bringup(void *args);

/*
 *  Function: i595_takedown
 *   Purpose: take the 82595 down off the network
 *
 *    Params: (via args)
 *       Input: nd      Structure describing the network interface
 *
 *   Returns:
 *          OK: 1
 *       Error: 0
 */
int i595_takedown(void *args);

/*
 *  Function: i595_txstart
 *   Purpose: Start packet transmission
 *
 *    Params: (via args)
 *       Input: m0      The packet to transmit, and control information
 *
 *   Returns:
 *          OK: 1
 *       Error: 0
 */
int i595_txstart(void *args);

/*
 *  Function: i595_txchain
 *   Purpose: Add another packet to the transmit chain
 *
 *    Params: (via args)
 *       Input: m0      The packet to transmit, and control information
 *
 *   Returns: Nothing
 */
void i595_txchain(void *args);

/*
 *  Function: i595_packetgone
 *   Purpose: "Remove" a transmitted packet from the Tx ring buffer
 *
 *    Params: (via args)
 *      In/Out: nd      Structure describing the driver
 *
 *   Returns:
 *          OK: 0 - No further packets in the ring buffer
 *              1 - More packets were found in the ring buffer
 *
 * Post-conditions: If more packets were found in the ring buffer, then
 *                      the next packet will be transmitted.
 */
int i595_packetgone(void *args);

/*
 *  Function: i595_rxpackets
 *   Purpose: Read received frames from the Rx ring buffer, and pass them to
 *              Fusion.
 *
 *    Params: (via args)
 *       Input: nd      Structure describing the network interface
 *
 *   Returns: Nothing
 */
void i595_rxpackets(void *args);

/*
 *  Function: i595_promiscmode
 *   Purpose: {En,Dis}able promiscuous reception
 * 
 *  Pre-conditions: Interrupts from the controller are disabled
 *
 *    Params: (via args)
 *       Input: oc      Structure describing the driver
 *              enable  On or Off?
 *
 *   Returns: Nothing
 */
void i595_promiscmode(void *args, bool enable);

/*
 *  Function: i595_badpacketrx
 *   Purpose: {En,Dis}able bad packet reception
 *
 *  Pre-conditions: Interrupts from the controller are disabled
 *
 *    Params: (via args)
 *       Input: oc      Structure describing the driver 
 *              enable  On or Off?
 *
 *   Returns: Nothing
 */
void i595_badpacketrx(void *args, bool enable);

/**********************************************************************/

#endif /* ndef etherd_olicom_h */

/* EOF olicom.h */
