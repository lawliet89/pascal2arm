/* -*-C-*-
 *
 * $Revision: 1.7.6.3 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:55:47 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Serial device driver for PID board, derived from PIE
 *              board serial driver.
 */
#ifndef angel_st16c552_h
#define angel_st16c552_h

#include "devdriv.h"
#include "serring.h"

#define ST16C552_IDENT_A        0
#define ST16C552_IDENT_B        1

extern const struct angel_DeviceEntry angel_ST16C552Serial[];

/*
 * Layout of the Serial controller; from our perspective, each
 * byte-wide register in the serial chip is on a word boundary
 *
 * 960604 KWelton
 *
 * On a big-endian machine, we need to offset I/O addresses by 3
 * (unfortunately, there is no byte-lane steering on APB, so we
 * have to do it here in software =8( ).
 */
typedef struct ST16C552Reg
{
#ifdef BIG_ENDIAN
    volatile unsigned char _BEpad[3];
#endif
    volatile unsigned char rhrthr;
    volatile unsigned char _pad0[3];
    volatile unsigned char ier;
    volatile unsigned char _pad1[3];
    volatile unsigned char isrfcr;
    volatile unsigned char _pad2[3];
    volatile unsigned char lcr;
    volatile unsigned char _pad3[3];
    volatile unsigned char mcr;
    volatile unsigned char _pad4[3];
    volatile unsigned char lsr;
    volatile unsigned char _pad5[3];
    volatile unsigned char msr;
    volatile unsigned char _pad6[3];
    volatile unsigned char spr;
} ST16C552Reg;

#define rhr             rhrthr
#define thr             rhrthr
#define isr             isrfcr
#define fcr             isrfcr

#define dll             rhrthr
#define dlm             ier

/*
 * interrupt bit masks
 */
#define RxReadyInt      (1 << 0)
#define TxReadyInt      (1 << 1)
#define RxLineInt       (1 << 2)
#define ModemInt        (1 << 3)

/*
 * Line Status Register bits
 */
#define LSRRxData       (1 << 0)
#define LSROverrun      (1 << 1)
#define LSRParity       (1 << 2)
#define LSRFraming      (1 << 3)
#define LSRBreak        (1 << 4)
#define LSRTxHoldEmpty  (1 << 5)
#define LSRTxEmpty      (1 << 6)
#define LSRFIFOError    (1 << 7)

/*
 * Clock Divisors for various Baud rates (assuming that
 * the PID is using a 1.843 MHz clock for the UART)
 */
#define Baud1200        0x0060
#define Baud2400        0x0030
#define Baud4800        0x0018
#define Baud9600        0x000c
#define Baud19200       0x0006
#define Baud38400       0x0003
#define Baud56000       0x0002  /* 2.77% error */
#define Baud57600       0x0002  /* 0% error */
#define Baud115200      0x0001

/*
 * Layout of the Parallel controller; once again, the registers
 * appear to be on word boundaries from our perspective
 */
typedef struct ST16C552PP
{
#ifdef BIG_ENDIAN
    volatile unsigned char _BEpad[3];
#endif
    volatile unsigned char port;
    volatile unsigned char _pad0[3];
    volatile unsigned char selstat;
    volatile unsigned char _pad1[3];
    volatile unsigned char ctrlcmd;
} ST16C552PP;

#define ppselect          selstat
#define ppstatus          selstat
#define ppcontrol         ctrlcmd
#define ppcommand         ctrlcmd

/*
 * see devdriv.h for description of Interrupt handler functions
 */
extern void angel_ST16C552IntHandler(unsigned int ident,
                                     unsigned int data,
                                     unsigned int empty_stack);

#endif /* ndef angel_st16c552_h */

/* EOF st16c552.h */
