/* -*-C-*-
 *
 * $Revision: 1.1.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:51:15 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial writes to 16c552 serial port B.
 */

#include "channels.h"
#include "pid.h"
#include "st16c552.h"
#include "log552.h"
#include "debug.h"

static char  log552_buf[128];
static char *log552_pos;
static char *log552_end;
static WarnLevel log552_level;
static short log552_firstmsg = TRUE;
static short log552_deferred_prefix = FALSE;
static int   log552_minlevel; /* use dbg to set this to 0 for all msgs up to 3 for errors only */

/*
 * NT's HyperTerminal needs CRLF, not just LF!
 */
#define LOG552_ADD_CR_TO_LF

/*
 * the Interrupt Enable register is write-only, so we use the
 * scratch register to keep track of currently enabled Ints
 * [code from st16c552.c, spr not used otherwise -- RIC ]
 */
#define IER_reset(u)    ((u)->ier = ((u)->spr = 0));
#define FCR_set(u, f)   ((u)->fcr = (f))

#pragma no_check_stack

#if DEFBAUD == 9600
# define BAUDVALUE Baud9600

#elif DEFBAUD == 19200
# define BAUDVALUE Baud19200

#elif DEFBAUD == 38400
# define BAUDVALUE Baud38400

#elif DEFBAUD == 57600
# define BAUDVALUE Baud57600

#elif DEFBAUD == 115200
# define BAUDVALUE Baud115200

#else
# error invalid baud rate
#endif

#include "banner.h"

/* #define LOGONMESSAGE "\n\nAngel Debug Monitor\n\n" */
#define LOGONMESSAGE "\n\n" ANGEL_BANNER "\n\n"

/*
 * prefix for lines not at start of message -- see stuff in putchar too
 */
#define PREFIXLEN   6

/*
 * prototypes
 */

void log552_flushbuf(void);


bool log552_PreWarn(WarnLevel level)
{
    ST16C552Reg *const serchip = (LOG552_PORT == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;
    char *p;

    /*
     * Interrupt Enable Register - all interrupts off
     */
    IER_reset(serchip);

    /*
     * wait for current transmits to finish
     */
    while ((serchip->lsr & (1 << 6)) == 0)
        /* do nothing */
        ;

    /*
     * Modem Control Register - DTR* and RTS* outputs high; INT
     * pin tri-stated; Rx and Tx "disabled" by setting chip in
     * loopback mode.
     */
    serchip->mcr = 0x10;

    /*
     * FIFO Control Register - FIFO enabled, but Rx and Tx
     * FIFOs empty; RxRDY and TxRDY mode 1; FIFO interrupt
     * trigger level = 8.
     */
    serchip->fcr = 0x8f;

    /*
     * Enable divisor latch via the Line Control Register, and
     * set the baud rates
     */
    serchip->lcr = 0x80;
    serchip->dll = (unsigned char)(BAUDVALUE & 0xff);
    serchip->dlm = (unsigned char)((BAUDVALUE >> 8) & 0xff);

    /*
     * OK, set the operational values for the LCR: 8-bit words,
     * 1 stop bit, no parity
     */
    serchip->lcr = 0x03;

    /*
     * take the chip out of loopback, and enable interrupts; DTR* and
     * RTS* outputs are forced low.
     */
    serchip->mcr = 0x0b;

    /*
     * set up the buffer pointers... reset in flushbuf
     */
    log552_pos = log552_buf;
    log552_end = (log552_buf + sizeof(log552_buf) - 1);

    if (log552_firstmsg)
    {
        /*
         * print a logon banner to say we're here!
         */
        p = LOGONMESSAGE;
        while(*p)
            log552_PutChar(*p++);

        log552_flushbuf();

        log552_firstmsg = FALSE;
        log552_deferred_prefix = TRUE;
        log552_minlevel = 0;
    }
    else if (log552_level != level && !log552_deferred_prefix)
    {
        log552_PutChar('\n');
    }
    log552_level = level;

    /*if (level >= log552_minlevel)
        return FALSE; */
    return TRUE;
}

void log552_flushbuf(void)
{
    char *p;
    ST16C552Reg *const serchip = (LOG552_PORT == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;
    
    p = log552_buf;
    while(p < log552_pos)
    {
        while ((serchip->lsr & (1 << 5)) == 0)
            /* fifo full -- wait */;

        serchip->thr = *p;  /* putchar */
        p++;
    }

    log552_pos = log552_buf;
    log552_end = log552_buf + sizeof(log552_buf) - 1;
}


int log552_PutChar(char c)
{
    if (log552_deferred_prefix == TRUE && log552_firstmsg == FALSE)
    {
        char *p;
        if ((log552_pos + PREFIXLEN) >= log552_end)
            log552_flushbuf();

        switch(log552_level)
        {
        case WL_TRACE:
            p = "Trace:";
            while(*p != '\0')
                *log552_pos++ = *p++;
            break;
        case WL_INFO:
            p = "Info :";
            while(*p != '\0')
                *log552_pos++ = *p++;
            break;
        case WL_WARN:
            p = "Warn :";
            while(*p != '\0')
                *log552_pos++ = *p++;
            break;
        case WL_ERROR:
            p = "Error:";
            while(*p != '\0')
                *log552_pos++ = *p++;
            break;
        }

        log552_deferred_prefix = FALSE;
    }
    else
    {
        if (log552_pos >= log552_end)
            log552_flushbuf();
    }

    if (c == '\n')
    {
#ifdef LOG552_ADD_CR_TO_LF
        *log552_pos++ = '\r';
        
        if (log552_pos >= log552_end)
            log552_flushbuf();
#endif
        log552_deferred_prefix = TRUE;
    }
    
    *log552_pos++ = c;

    return 0;
}

void log552_PostWarn(unsigned int len)
{
    log552_flushbuf();
}

#pragma check_stack

/* EOF log552.c */
