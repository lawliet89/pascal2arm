/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.1 $
 *   $Author: kwelton $
 *     $Date: 1996/07/22 16:52:45 $
 *
 * 82595.h - device driver headers for Intel 82595 controller used in
 * the Olicom EtherCom PCMCIA Ethernet card
 */

#ifndef etherd_82595_h
#define etherd_82595_h

/*
 * State Machine definitions - the states.  Note that the values of
 * these states are important, because they are used to index a
 * jump table in the routine olicom_state().
 */
typedef enum
{
    eth_ABSENT = 0,                     /* No card present */
    eth_DOWN = 1,                       /* Card present, not configured */
    eth_IDLE = 2,                       /* Card configured but inactive */
    eth_TxACTIVE = 3,                   /* Card transmitting a packet */
    eth_CMDACTIVE = 4                   /* Card running a command */
} OlicomState;

/*
 * Events
 */
typedef enum
{
    ev_INSERTION = 0,                   /* 0 Card Insertion */
    ev_REMOVAL,                         /* 1 Card Removal */
    ev_BRINGUP,                         /* 2 Request to configure the card */
    ev_TAKEDOWN,                        /* 3 Request to disable the card */
    ev_TxPACKET,                        /* 4 Request to transmit a packet */
    ev_RUNCMD,                          /* 5 Request to run a command */
    ev_RxPACKET,                        /* 6 Received packet interrupt */
    ev_RxOVERFLOW,                      /* 7 Ring buffer overflow interrupt */
    ev_TxGONE,                          /* 8 Transmit complete interrupt */
    ev_EXECFINISHED                     /* 9 Exec complete interrupt */
} OlicomEvent;

/*
 * Olicom Card Configuration registers
 */
#define CardConfReg0    0x3f8
#define CardConfReg1    0x3fa

/*
 * register layout of the Intel 82595 chip
 *
 * those ever imaginative people at Intel have no other
 * names for the registers on their chip than r1, r2
 * etc..
 *
 * To try and optimise I/O to the chip, this structure represents
 * the *real* appearance of the chip in the PID7T NISA I/O space;
 * the wierd wiring arrangement described in pid.h means that,
 * although the chip has a sequential collection of registers, they
 * appear to be grouped as two-register pairs, aligned on a word
 * boundary
 */
#define I595_R0         0x00
#define I595_R1         0x01
#define I595_R2         0x04
#define I595_R3         0x05
#define I595_R4         0x08
#define I595_R5         0x09
#define I595_R6         0x0c
#define I595_R7         0x0d
#define I595_R8         0x10
#define I595_R9         0x11
#define I595_R10        0x14
#define I595_R11        0x15
#define I595_R12        0x18
#define I595_R13        0x19
#define I595_R14        0x1c
#define I595_R15        0x1d

/*
 * register 0 is special, as it's the same in all banks
 */
#define OPCODE_MASK             (0x1f)
#define SelectBank(c, b)        (PCMCIA_IO_WRITE((c) + I595_R0, (b) << 6))
#define RunCmd(c, o)            (PCMCIA_IO_WRITE((c) + I595_R0, \
                                                 ((o) & OPCODE_MASK)))
#define AbortCmd(c)             (PCMCIA_IO_WRITE((c) + I595_R0, (1 << 5)))

/*
 * 82595 Command Set
 */
#define CMD_MCSETUP             0x03
#define CMD_TRANSMIT            0x04
#define CMD_TDR                 0x05
#define CMD_DUMP                0x06
#define CMD_DIAGNOSE            0x07
#define CMD_ABORT               0x0d
#define CMD_XMIT_RAW            0x14
#define CMD_RESUME_XMIT_LIST    0x1c
#define CMD_CONT_XMIT_LIST      0x15
#define CMD_SET_TRISTATE        0x16
#define CMD_RESET_TRISTATE      0x17
#define CMD_POWER_DOWN          0x18
#define CMD_RESET               0x0e
#define CMD_SEL_RESET           0x1e
#define CMD_RCV_ENABLE          0x08
#define CMD_RCV_DISABLE         0x0a
#define CMD_RCV_STOP            0x0b

/*
 * 82595 result codes
 */
#define RESULT_INIT_DONE        0x0e

/*
 * States for Execution and Receive Units (bank 0, register 1)
 */
#define EXEC_STATE(r)           (((r) >> 4) & 0x03)
#define EXEC_STATE_IDLE         (0)
#define EXEC_STATE_BUSY         (2)
#define EXEC_STATE_ABORTING     (3)

#define RCV_STATE(r)            (((r) >> 6) & 0x03)
#define RCV_STATE_DISABLED      (0)
#define RCV_STATE_READY         (1)
#define RCV_STATE_ACTIVE        (2)
#define RCV_STATE_STOPPING      (3)

/*
 * Interrupt bit masks
 */
#define RXSTP_INT               (1 << 0)
#define RX_INT                  (1 << 1)
#define TX_INT                  (1 << 2)
#define EXEC_INT                (1 << 3)
#define ALL_INT_BITS            (0x0f)

/*
 * The card has 64K of RAM for frame transmission and reception.  This
 * is split up into 12K of transmit buffer, and 52K of receive buffer.
 *
 * We use 12K of transmit buffer because the Fusion code does not allow
 * the driver to queue the packets that are passed down for transmission:
 * they must be copied immediately because they are destroyed when the
 * start routine returns. =8(
 */
#define TOTAL_RAM_SIZE          (64 * 1024)
#define RAM_TX_BASE             (0)
#define RAM_TX_SIZE             (12 * 1024)
#define RAM_TX_LIMIT            (RAM_TX_BASE + (RAM_TX_SIZE - 1))

#define RAM_RX_SIZE             (TOTAL_RAM_SIZE - RAM_TX_SIZE)
#define RAM_RX_BASE             (RAM_TX_SIZE)
#define RAM_RX_LIMIT            (RAM_RX_BASE + (RAM_RX_SIZE - 1))

/*
 * macro for converting from packet byte count to length in Tx ring
 * buffer which needs to include 8 bytes of header and half-word
 * alignment
 */
#define COUNT_TO_LEN(x)    ((x) + ((((x) & 0x01) == 0) ? 8 : 9))

#endif /* ndef etherd_82595_h */

/* EOF 82595.h */
