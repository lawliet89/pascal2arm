/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.3.6.3 $
 *   $Author: rivimey $
 *     $Date: 1997/12/22 14:15:31 $
 *
 * 82595.c - low-level device driver routines for Intel 82595 controller
 * used in the Olicom EtherCom PCMCIA Ethernet card
 */
#include <stdio.h>

#include "angel.h"
#include "olicom.h"
#include "82595.h"
#include "pid.h"

/*
 * required Angel headers
 */
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
 * I still don't know why Pacific Softworks
 * don't have this in a header file
 */
extern a48 host_addr;

/**********************************************************************/

/*
 *  Function: load_packet
 *   Purpose: Copy a packet into the transmit ring buffer
 *
 *    Params:
 *       Input: mp      The packet to transmit
 *
 *              chain   Flag whether this packet is being chained into
 *                      the buffer (1) or not (0).
 *
 *      In/Out: oc      Structure describing driver environment
 *
 *   Returns:
 *          OK: 1
 *       Error: 0
 */
static int load_packet(const m *const mp, ethercom *const oc, const int chain)
{
    int nbytes, length;
    unsigned char *cptr;
    unsigned int chip = oc->oc_82595base;

    /*
     * nbytes is the actual number of bytes in the packet; and length
     * is the amount of space consumed in the transmit buffer, i.e.
     * the packet data padded to a half-word boundary, and 8 bytes of
     * transmit header
     */
    nbytes = m_dsize(mp);
    length = COUNT_TO_LEN(nbytes);

    /*
     * see whether there is enough space for this packet.
     */
    if (oc->oc_txstart < oc->oc_txstop)
    {
        /*
         * free space is from txstart to the byte before txstop
         */
        if ((oc->oc_txstart + length) >= oc->oc_txstop)
        {
            LogInfo(LOG_82595, ("load_packet: not enough space for packet\n"));
            return 0;
        }
    }
    else
    {
        /*
         * free space is from txstart to the top
         * of the transmit buffer.
         */
        if ((oc->oc_txstart + length) > RAM_TX_LIMIT)
        {
            /*
             * packet won't fit in the top of the ring buffer,
             * maybe it will go in the free space at the bottom
             */
            if ((RAM_TX_BASE + length) >= oc->oc_txstop)
            {
                LogInfo(LOG_82595, ("load_packet: not enough space for packet (even when wrapped)\n"));
                return 0;
            }

            /*
             * to get this far, the packet must fit when wrapped
             */
            oc->oc_txstart = RAM_TX_BASE;
        }
    }

    if (chain)
    {
        /*
         * save the start address of this packet in the transmit
         * chain field of the previous frame
         */
        PCMCIA_IO_WRITE(chip + I595_R12, oc->oc_txchain & 0xff);
        PCMCIA_IO_WRITE(chip + I595_R13, (oc->oc_txchain >> 8) & 0xff);

        PCMCIA_IO_WRITE(chip + I595_R14, oc->oc_txstart & 0xff);
        PCMCIA_IO_WRITE(chip + I595_R15, (oc->oc_txstart >> 8) & 0xff);
    }

    /*
     * the transmit chain field is 4 bytes into the transmit
     * structure - record its position for later use by the
     * chaining code
     */
    oc->oc_txchain = oc->oc_txstart + 4;

    /*
     * OK - setup host address (r12, r13), and load up the preamble
     */
    PCMCIA_IO_WRITE(chip + I595_R12, oc->oc_txstart & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R13, (oc->oc_txstart >> 8) & 0xff);

    /*
     * Opcode, and reserved field
     */
    PCMCIA_IO_WRITE(chip + I595_R14, CMD_TRANSMIT);
    PCMCIA_IO_WRITE(chip + I595_R15, 0x00);

    /*
     * Status 0,1
     */
    PCMCIA_IO_WRITE(chip + I595_R14, 0x00);
    PCMCIA_IO_WRITE(chip + I595_R15, 0x00);

    /*
     * Chain Pointer Lo, Hi.  Use top of Tx buffer as
     * end of chain marker.
     */
    PCMCIA_IO_WRITE(chip + I595_R14, RAM_TX_LIMIT & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R15, (RAM_TX_LIMIT >> 8) & 0xff);

    /*
     * Byte Count Lo, Hi
     */
    PCMCIA_IO_WRITE(chip + I595_R14, nbytes & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R15, (nbytes >> 8) & 0x7f);

    /*
     * now copy the packet in
     */
    for (cptr = (unsigned char *)(mp->m_hp); nbytes > 0; nbytes -= 2)
    {
        PCMCIA_IO_WRITE(chip + I595_R14, *cptr++);
        PCMCIA_IO_WRITE(chip + I595_R15, *cptr++);
    }

    /*
     * finished - update ring buffer start pointer and go away
     */
    oc->oc_txstart += length;
    return 1;
}

/*
 *  Function: start_tx
 *   Purpose: Start transmitting a packet
 *
 *  Pre-conditions: Controller has Bank 0 selected
 *
 *    Params:
 *       Input: oc      Structure describing the environment
 *
 *              addr    Address of the packet to be transmitted
 *
 *   Returns: Nothing
 */
static void start_tx(const ethercom *oc, const unsigned int addr)
{
    unsigned int chip = oc->oc_82595base;

    /*
     * this really is *very* simple - program the base address
     * register, and tell the chip to get on with things
     */
    PCMCIA_IO_WRITE(chip + I595_R10, addr & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R11, (addr >> 8) & 0xff);

    RunCmd(chip, CMD_TRANSMIT);
}

/**********************************************************************/

/*
 * Externally visible functions.  These functions
 * are documented in olicom.h
 */
int i595_bringup(void *args)
{
    const netdev *nd = (netdev *)args;
    unsigned int attrmem = nd->nd_olicom.oc_pcs->pcs_attrbase;
    unsigned int chip = nd->nd_olicom.oc_82595base;
    unsigned char *cptr;

    /*
     * Step 1 - Olicom card configuration register manipulation:
     * issue a reset (r0, bit 7); then remove it and set up a
     * working configuration.
     */
    PCMCIA_MEM_WRITE(attrmem + CardConfReg0, 0xc0);
    delay(200);

    /*
     * configuration is:
     *
     * I/O Enable (r0, bit 0) - On
     * Level Mode Intr (r0, bit 6) - On
     * Reset (r0, bit 7) - Off
     *
     * 8-bit I/O only (r1, bit 5) - On
     *
     * All other bits are reserved, and set to 0.  To be safe,
     * we give the card 200us to recover from reset being
     * removed before enabling I/O
     */
    PCMCIA_MEM_WRITE(attrmem + CardConfReg0, 0x40);
    delay(200);

    PCMCIA_MEM_WRITE(attrmem + CardConfReg0, 0x41);
    PCMCIA_MEM_WRITE(attrmem + CardConfReg1, 0x20);

    /*
     * Step 2 - check that the Exec interrupt is set, and that the
     * state is Init Done (interrupt bits are in bank 0, r1)
     */
    SelectBank(chip, 0);
    if ((PCMCIA_IO_READ(chip + I595_R1) & ALL_INT_BITS) != EXEC_INT ||
        (PCMCIA_IO_READ(chip + I595_R0) & OPCODE_MASK) != RESULT_INIT_DONE)
    {
        LogInfo(LOG_82595, ("olicom: bringup: bad post-reset state (%x, %x)\n",
                            PCMCIA_IO_READ(chip + I595_R1),
                            PCMCIA_IO_READ(chip + I595_R0)));

        return 0;
    }

    /*
     * acknowledge the `interrupt'
     */
    PCMCIA_IO_WRITE(chip + I595_R1, EXEC_INT);

    /*
     * Step 3 - program transmit and receive ring buffer parameters.  This
     * process starts in bank 0 with registers 4 & 5 (RCV CAR/BAR), which
     * are initialised to the first half-word above the base of RCV memory
     */
    PCMCIA_IO_WRITE(chip + I595_R4, (RAM_RX_BASE + 2) & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R5, ((RAM_RX_BASE + 2) >> 8) & 0xff);

    /*
     * registers 6 & 7 - RCV STOP.  These are set to the base of RCV memory
     */
    PCMCIA_IO_WRITE(chip + I595_R6, RAM_RX_BASE & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R7, (RAM_RX_BASE >> 8) & 0xff);

    /*
     * registers 10 & 11 - XMT CAR/BAR
     */
    PCMCIA_IO_WRITE(chip + I595_R10, RAM_TX_BASE & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R11, (RAM_TX_BASE >> 8) & 0xff);

    /*
     * now move along to bank 1, registers 8 & 9 - RCV Lower
     * and Upper Limits
     */
    SelectBank(chip, 1);
    PCMCIA_IO_WRITE(chip + I595_R8, (RAM_RX_BASE >> 8) & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R9, (RAM_RX_LIMIT >> 8) & 0xff);

    /*
     * registers 10 & 11 - XMT Lower and Upper Limits
     */
    PCMCIA_IO_WRITE(chip + I595_R10, (RAM_TX_BASE >> 8) & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R11, (RAM_TX_LIMIT >> 8) & 0xff);

    /*
     * Step 4 - feature selection.  We start off by disabling the
     * receive concurrent processing feature that the chip supports
     * by setting both the RCV BOF Threshold (bank 1, register 7)
     * and the RCV Copy Threshold (bank 0, register 8) to 0
     */
    PCMCIA_IO_WRITE(chip + I595_R7, 0);
    SelectBank(chip, 0);
    PCMCIA_IO_WRITE(chip + I595_R8, 0);

    /*
     * Program Individual Address registers (bank 2, registers 4-9)
     * These must be programmed before other feature selection
     * registers in bank 2, and the EXE-DMA must be idle before
     * the IA-SETUP process is started by writing to register 9.
     */
    if (EXEC_STATE(PCMCIA_IO_READ(chip + I595_R1)) != EXEC_STATE_IDLE)
    {
        LogInfo(LOG_82595, ("olicom: bringup: Chip not idle, cannot initialise!\n"));
        return 0;
    }

    SelectBank(chip, 2);
    cptr = host_addr;

    PCMCIA_IO_WRITE(chip + I595_R4, *cptr);
    ++cptr;
    PCMCIA_IO_WRITE(chip + I595_R5, *cptr);
    ++cptr;
    PCMCIA_IO_WRITE(chip + I595_R6, *cptr);
    ++cptr;
    PCMCIA_IO_WRITE(chip + I595_R7, *cptr);
    ++cptr;
    PCMCIA_IO_WRITE(chip + I595_R8, *cptr);
    ++cptr;
    PCMCIA_IO_WRITE(chip + I595_R9, *cptr);

    /*
     * now wait for the IA-SETUP command to finish
     */
    SelectBank(chip, 0);
    while (EXEC_STATE(PCMCIA_IO_READ(chip + I595_R1)) != EXEC_STATE_IDLE)
        /* do nothing */
        ;

    /*
     * bank 2, register 1 features - we do not use Transmit Concurrent
     * Processing, but we do discard bad frames
     */
    SelectBank(chip, 2);
    PCMCIA_IO_WRITE(chip + I595_R1, 0x80);

    /*
     * bank 2, register 2 features - not promiscuous, receive broadcast,
     * do not save CRC, Length Enable is on, Source Address insertion is
     * on, only one individual address, loopback off.
     */
#if 1
    PCMCIA_IO_WRITE(chip + I595_R2, 0x14);
#else
    /* TRACE - DO NOT RECOGNISE BROADCAST PACKETS */
    PCMCIA_IO_WRITE(chip + I595_R2, 0x16);
#endif

    /*
     * register 3 is a completely unknown quantity, because it
     * depends entirely upon the undocumented innards of the
     * Olicom card - all we know is that we must clear the top
     * two bits, since these are unwanted test-mode bits
     */
    PCMCIA_IO_WRITE(chip + I595_R3, PCMCIA_IO_READ(chip + I595_R3) & 0x3f);

    /*
     * Step 5 - enable interrupts by clearing the masks in bank 0,
     * register 3, and disabling the Tri-State bit in bank 1,
     * register 1.
     */
    SelectBank(chip, 1);
    PCMCIA_IO_WRITE(chip + I595_R1, 0x80);
    SelectBank(chip, 0);
    PCMCIA_IO_WRITE(chip + I595_R3,
                  PCMCIA_IO_READ(chip + I595_R3) & ~ALL_INT_BITS);

    /*
     * Step 6 - Reset Tristate, and enable packet reception
     */
    RunCmd(chip, CMD_RESET_TRISTATE);
    RunCmd(chip, CMD_RCV_ENABLE);

    /*
     * Step 7 - I can't see this mentioned anywhere in the documentation,
     * but it seems we need to give the chip some time to settle down.
     */
    delay(50000);

    /* finished */
    return 1;
}

int i595_takedown(void *args)
{
    unsigned int attrmem = ((netdev *)args)->nd_olicom.oc_pcs->pcs_attrbase;

    /*
     * This is *very* crude - issue a reset to the chip
     */
    PCMCIA_MEM_WRITE(attrmem + CardConfReg0, 0x80);
    delay(200);

    return 1;
}

int i595_txstart(void *args)
{
    m *mp = (m *)args;
    ethercom *oc = &mp->m_ndp->nd_olicom;

    /*
     * since we are starting a transmit, we know that the
     * transmit buffer is currently empty, so reset the
     * ring buffer control pointers
     */
    oc->oc_txstart = RAM_TX_BASE;
    oc->oc_txstop = RAM_TX_LIMIT + 1;

    /*
     * copy the packet into the transmit buffer
     */
    if (!load_packet(mp, oc, 0))
        /* failed */
        return 0;

    /*
     * everything's ready, start the transmit
     */
    start_tx(oc, RAM_TX_BASE);

    return 1;
}

void i595_txchain(void *args)
{
    m *mp = (m *)args;
    ethercom *oc = &mp->m_ndp->nd_olicom;

    /*
     * all we need to do is to load the packet, asking
     * for it to be chained
     */
    (void)load_packet(mp, oc, 1);
}

int i595_packetgone(void *args)
{
    ethercom *oc = &(((struct netdev *)args)->nd_olicom);
    unsigned int chip = oc->oc_82595base;
    unsigned int base, link, length;

    /*
     * the address of the packet just sent will be in the Base
     * Address Register - use this to read the Tx chain pointer
     * out of the packet
     */
    base = PCMCIA_IO_READ(chip + I595_R10);
    base |= (PCMCIA_IO_READ(chip + I595_R11) << 8);

    PCMCIA_IO_WRITE(chip + I595_R12, (base + 4) & 0xff);
    PCMCIA_IO_WRITE(chip + I595_R13, ((base + 4) >> 8) & 0xff);
    link = PCMCIA_IO_READ(chip + I595_R14);
    link |= PCMCIA_IO_READ(chip + I595_R15) << 8;

    /*
     * the next two bytes in the buffer give the length of the packet
     */
    length = PCMCIA_IO_READ(chip + I595_R14);
    length |= (PCMCIA_IO_READ(chip + I595_R15) << 8);
    length = COUNT_TO_LEN(length);

    /*
     * stop pointer for Tx ring buffer is the first byte beyond
     * the end of the packet
     */
    oc->oc_txstop = base + length;

    /*
     * check for another packet; remember that RAM_TX_LIMIT is used
     * as an end of chain marker
     */
    if (link != RAM_TX_LIMIT)
    {
        start_tx(oc, link);
        return 1;
    }
    else
        return 0;
}

void i595_rxpackets(void *args)
{
    struct netdev *nd = (struct netdev *)args;
    ethercom *oc = &nd->nd_olicom;
    unsigned int chip = oc->oc_82595base;
    unsigned int start;

    /*
     * the stop register points to the last pair of bytes
     * *before* the start of the packet
     */
    start = PCMCIA_IO_READ(chip + I595_R6);
    start |= (PCMCIA_IO_READ(chip + I595_R7) << 8);
    start += 2;

    /*
     * loop until the first byte of the next packet indicates
     * that reception of that packet is not yet complete
     */
    for (;;)
    {
        unsigned int event, status, length;
        m *mp;

        /*
         * program host address register, then read
         * the event field, and the NULL field which
         * follows it
         */
        PCMCIA_IO_WRITE(chip + I595_R12, (start & 0xff));
        PCMCIA_IO_WRITE(chip + I595_R13, ((start >> 8) & 0xff));

        event = PCMCIA_IO_READ(chip + I595_R14);
        event |= (PCMCIA_IO_READ(chip + I595_R15) << 8);

        /*
         * is this another complete frame?
         */
        if ((event & 0x08) == 0)
            /* no */
            return;

        /*
         * OK, read status, start of next packet, and byte count
         */
        status = PCMCIA_IO_READ(chip + I595_R14);
        status |= (PCMCIA_IO_READ(chip + I595_R15) << 8);

        start = PCMCIA_IO_READ(chip + I595_R14);
        start |= (PCMCIA_IO_READ(chip + I595_R15) << 8);

        length = PCMCIA_IO_READ(chip + I595_R14);
        length |= (PCMCIA_IO_READ(chip + I595_R15) << 8);

        /*
         * we always read an even number of bytes from the controller
         */
        if ((length & 0x01) != 0)
            ++length;

        /*
         * try to get a new receive buffer
         */
        if ((mp = m_new(length, NULL, F_NOBS)) != NULL)
        {
            char *cptr;

            /*
             * prepare the new buffer
             */
            mp->m_hp -= length;
            mp->m_ndp = nd;

            /*
             * read the data from the Rx ring buffer
             */
            for (cptr = mp->m_hp; length != 0; length -=2)
            {
                *cptr++ = PCMCIA_IO_READ(chip + I595_R14);
                *cptr++ = PCMCIA_IO_READ(chip + I595_R15);
            }

            /*
             * TRACE - CRUDE HANDLING OF PACKET STATUS
             */
            if ((status & 0x2000) != 0)
            {
                /* packet is OK - pass it along */
                LogInfo(LOG_82595, ("passing packet to msm"));
                twq_promise();
                msm(mp, en_up);
                twq_run();
            }
            else
                LogInfo(LOG_82595, ("RxPacket failed: status 0x%04x\n", status));
        }
        else
            LogWarning(LOG_82595, ("RxPacket couldn't get rx buffer" ));

        /*
         * update the stop register from the next packet pointer
         */
        PCMCIA_IO_WRITE(chip + I595_R6, ((start - 2) & 0xff));
        PCMCIA_IO_WRITE(chip + I595_R7, (((start - 2) >> 8) & 0xff));
    }
}

void i595_promiscmode(void *args, bool enable)
{
    unsigned int chip = ((ethercom *)args)->oc_82595base;
    unsigned char reg;

    SelectBank(chip, 2);
    reg = PCMCIA_IO_READ(chip + I595_R2);

    /*
     * Promiscuous mode is controlled by bit 0
     */
    if (enable)
        reg |= 0x01;
    else
        reg &= ~0x01;

    PCMCIA_IO_WRITE(chip + I595_R2, reg);
    SelectBank(chip, 0);
}

void i595_badpacketrx(void *args, bool enable)
{
    unsigned int chip = ((ethercom *)args)->oc_82595base;
    unsigned char reg;

    SelectBank(chip, 2);
    reg = PCMCIA_IO_READ(chip + I595_R1);

    /*
     * bad packet reception is controlled by bit 7
     */
    if (enable)
        reg &= ~0x80;
    else
        reg |= 0x80;

    PCMCIA_IO_WRITE(chip + I595_R1, reg);
    SelectBank(chip, 0);
}

/* EOF 82595.c */
