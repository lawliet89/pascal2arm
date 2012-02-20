/* -*-C-*-
 *
 * $Revision: 1.4.2.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/23 09:41:37 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Hardware description for PIE card
 */

#ifndef angel_hw_h
#define angel_hw_h

#include "devconf.h"

/* --- Define the base address of the driver hardware --- */

#define SerialChipBase  IOBase
#define PIObase         (ExpBase + (ExpSize / 2))

/*
 * Serial hardware constants for the SCC2691 on PIE60, PIE7 and PID7 boards.
 */

/* --- Values of the fields in MR1 --- */

#define BitsPerChar     3         /* 0 for 5, 1 for 6, 2 for 7, 3 for 8 */
#define ParityType      4         /* 0 for Even, 4 for Odd              */
#define ParityMode      16        /* 0 for on, 8 for force, 16 for off  */
#define ErrorMode       0         /* 0 for char, 32 for block           */

/* It seems odd, but buffered mode is needed for unbuffered operation
 * due to a bug in unbuffered mode which causes spuripus interrupts to
 * occur.
 */
#if BUFFERED_SERIAL_INPUT
# define RxInterrupt     0         /* 0 for char ready, 64 for FIFO full */
#else
# define RxInterrupt     64         /* 0 for char ready, 64 for FIFO full */
#endif
#define RxAutoRTS       0         /* 0 for on, 128 for off              */

#define MR1Value        (BitsPerChar + ParityType + ParityMode  \
                         + ErrorMode + RxInterrupt + RxAutoRTS)


/* --- Values of the fields in MR2 --- */

#define StopBits        7         /* 7 for 1, 15 for 2 (see data sheet) */
#define AutoCTS         0         /* 0 for off, 16 for on               */
#define TxAutoRTS       0         /* 0 for off, 32 for on               */

#define MR2Value        (StopBits + AutoCTS + TxAutoRTS)


/* --- Values for the fields in the CSR --- */

/*
 * The baud rates are defined here and in the top bit of ACR
 *  called the BRG bit. Most useful of the 18 values provided are
 *         BRG = 0 ; CSR = 06       =>  1k2
 *                   CSR = 08       =>  2k4
 *                   CSR = 09       =>  4k8
 *                   CSR = 11       =>  9k6
 *                   CSR = 12       => 38k4
 *         BRG = 1 ; CSR = 12       => 19k2
 *
 * The BRG bit is stored in the top bit of this value; the basic
 * baud value in the bottom 4 bits -- this will be duplicated (as
 * Tx and RX speeds are indicated independently).
 *
 * The Chip's max speed is 38k4, min speed is 50.
 */

#define Baud1200        0x00000006
#define Baud2400        0x00000008
#define Baud4800        0x00000009
#define Baud9600        0x0000000b
#define Baud19200       0x8000000c
#define Baud38400       0x0000000c


/* --- Values for CR --- */

#define ResetMR1        0x10       /* Reset MR1                          */
#define ResetRx         0x20       /* Reset Receiver                     */
#define ResetTx         0x30       /* Reset Transmitter                  */
#define ResetError      0x40       /* Reset Error Status                 */
#define ResetBreak      0x50       /* Reset Break Status                 */
#define StartTimer      0x80       /* Start the timer                    */
#define ResetTimer      0x90       /* Reset the timer interrupt          */
#define AssertMPO       0xa0       /* MPO goes low                       */
#define NegateMPO       0xb0       /* MPO goes high                      */
#define ResetMPI        0xc0       /* Reset MPI                          */
#define DisableRxTx     0x0a       /* Disable the receiver and transmit. */
#define EnableRxTx      0x05       /* Enable the receiver and transmit.  */


/* --- Values for SR --- */

#define SRRxReady         1        /* A character has been received      */
#define SRRxFull          2        /* Rx in FIFO is full                 */
#define SRTxReady         4        /* Space in the Tx FIFO               */
#define SRTxEmpty         8        /* Tx FIFO is empty                   */
#define SROverrunError   16
#define SRParityError    32
#define SRFrameError     64
#define SRReceivedBreak 128

#define SRNastyError    (SROverrunError + SRParityError + SRFrameError \
                         + SRReceivedBreak)


/* --- Values for ACR --- */

/*
 * currently the baud rate is set at runtime (BRG bit)
 */

#define MPOOutput       0          /* See data sheet                     */
#define PowerDown       8          /* 0 for on, 8 for off                */
#define TimerSource     0x70       /* See data sheet                     */

#define ACRValue        (MPOOutput + PowerDown + TimerSource)


/* --- Values for ISR and IMR --- */

#define ISRTxReady        1        /* Space for the Tx FIFO              */
#define ISRTxEmpty        2        /* Tx FIFO is empty                   */
#define ISRRxReady        4        /* A character has been received      */
#define ISRRxFull         4        /* Rx FIFO is full                    */
#define ISRBreak          8        /* Change in break                    */
#define ISRTimerTicked   16        /* Timer has reached zero twice       */
#define MPIState         64        /* Latched state of MPI               */
#define MPIChanged      128        /* MPI has changed state              */

#define IMRValue        (ISRRxReady + ISRTimerTicked + ISRBreak)
#define NoTimerIMRValue (ISRRxReady + ISRBreak)
#define IMRTimer        (ISRTimerTicked)
#define SerialIntSource (ISRTxReady + ISRTxEmpty + ISRRxReady + ISRBreak)


/* --- Values for CTUR and CTLR (Timer) --- */

#define ClockFreq     3686400      /* 3.6864 MHz external clock freq     */
#define TimerFreq  (ClockFreq/32)  /* ClockFreq/(16*2)                   */
#define TimerTick (TimerFreq/100)  /* TimerFreq/100 - ie 100 times a sec */


/***************************************************************************\
* Parallel hardware constants                                               *
\***************************************************************************/


/* --- Interface control constants --- */

#define RxDataMask      0x01       /* Rx Data Reg. contains data         */
#define RxIntEnable     0x01       /* Enable Rx interrupts               */
#define RxIntDisable    0x00       /* Disable Rx interrupts              */

typedef struct
{
  word RDR;                    /* Rx Data Reg.          */
  word dummy;                  /* not used              */
  word RSR;                    /* Rx Status Reg.        */
  word IMR;                    /* Interrup Mask Reg.    */
} PIOstruct;

/* offsets into PIOstruct, assuming 32 bit word */
#define PIO_RDR         (0*4)
#define PIO_RSR         (2*4)
#define PIO_IMR         (3*4)


typedef struct
{
  word MR;
  word SR_CSR;
  word CR;
  word RHR_THR;
  word ACR;
  word ISR_IMR;
  word CTUR;
  word CTLR;
} SerialChipStruct;

/* offsets into SerialChipStruct, assuming 32 bit word */
#define SER_MR          (0*4)
#define SER_MR1         (SER_MR)
#define SER_MR2         (SER_MR)
#define SER_SR          (1*4)
#define SER_CSR         (SER_SR)
#define SER_CR          (2*4)
#define SER_RHR         (3*4)
#define SER_THR         (SER_RHR)
#define SER_ACR         (4*4)
#define SER_ISR         (5*4)
#define SER_IMR         (SER_ISR)
#define SER_CTUR        (6*4)
#define SER_CTLR        (7*4)

/* --- Driver hardware base addresses for C functions --- */

#define SerialChip ((volatile SerialChipStruct *) SerialChipBase)
#define PIO        ((volatile PIOstruct *) PIObase)

#endif /* ndef angel_hw_h */

/* EOF hw.h */
