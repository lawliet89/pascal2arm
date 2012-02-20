/* -*-C-*-
 *
 * $Revision: 1.1.2.4 $
 *   $Author: rivimey $
 *     $Date: 1998/01/12 21:22:34 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial writes to 16c552 serial port B.
 */

#if DEBUG == 1 && (!defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0)

#include <string.h>

#include "channels.h"
#include "pid.h"
#include "st16c552.h"
#include "logging.h"
#include "logging/logterm.h"
#include "debug.h"
#include "support.h"

#define SAVEBUFSIZE     2048    /* max #words in save buffer -- min 6 words/message */
#define MAXARGS         32      /* max number of distinct args on cmd line */
#define CMDBUFSIZE      128     /* max number of characters on command line */
#define OUTPUTBUFSIZE   64      /* number of buffered characters from message before flush */

#ifndef UNUSED
#define UNUSED(x)       (0 ? (x) = (x) : 0)
#endif

static unsigned long msgsavebuf[SAVEBUFSIZE];
static struct LogSaveBuffer savebuf;

static char log_commandbuf[CMDBUFSIZE];
static int log_cursor = 0;

static WarnLevel logterm_level;
static char  logterm_buf[OUTPUTBUFSIZE];
static char *logterm_pos;
static char *logterm_end;
static int log_tracing = TRUE;
static int log_deferredprompt = FALSE;
static int log_buflock = FALSE;
static int log_cmdlock = FALSE;

/* static void setupsave(void); */
void log_emitch(char ch);
void log_emitstr(char *str);
static void log_output(int enable);
static void log_processchar(char ch, unsigned int empty_stack);

static int log_dump(int argc, char **argv);
static int log_echo(int argc, char **argv);
static int log_ver(int argc, char **argv);
static int log_help(int argc, char **argv);
static int log_trace(int argc, char **argv);
static int log_go(int argc, char **argv);
static int log_pause(int argc, char **argv);
static int log_stat(int argc, char **argv);
static int log_level(int argc, char **argv);
static int log_format(int argc, char **argv);

static struct 
{
    char *str;
    int (*pfn)(int argc, char **argv);
    char *desc, *helpstr;
} log_cmds[] =
{   /* These must be kept in sorted order by name */
    { "dump",  log_dump,  "Dump\n",
                          "syntax: dump <start addr> [ <end addr> | +<len> ]\n"
    },
    { "echo",  log_echo,  "Echo\n",
                          "syntax: echo <words>\n"
    },
    { "format",log_format,"Show / set the per-line format string\n",
                          "syntax: format [<new string>]\n"
    },
    { "go",    log_go,    "Undo pause; reenable ints\n",
                          "syntax: go\n"
    },
    { "help",  log_help,  "Command help\n",
                          "syntax: help [command]\n"
    },
    { "pause", log_pause, "Pause Angel; disable ints\n",
                          "syntax: pause\n"
    },
    { "stat",  log_stat,  "Display internal statistics info\n",
                          "syntax: stat\n"
    },
    { "level", log_level,  "Set the minimum log level displayed.\n",
                          "syntax: level [info|warn|err]\n"
    },
    { "trace", log_trace, "Enable/disable tracing or display trace buffer\n",
                          "syntax: trace           -- display current settings\n"
                          "        trace <n>       -- show <n> lines of buffer\n"
                          "        trace <n>       -- show <n> lines of buffer\n"
                          "        trace on | off  -- enable/disable run-time trace\n"
    },
    { "ver",   log_ver,   "Display Angel version info\n",
                          "syntax: ver\n"
    },
};
static const int log_ncmds = sizeof(log_cmds)/sizeof(log_cmds[0]);

/*
 * NT's HyperTerminal needs CRLF, not just LF!
 */
#define LOGTERM_ADD_CR_TO_LF

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
#define LOGONMESSAGE "\n\n" ANGEL_BANNER "Type 'help' for more info.\n\n"
#define PROMPT       "% "

/*
 * the Interrupt Enable register is write-only, so we use the
 * scratch register to keep track of currently enabled Ints
 */
#define IER_set(u, f)   ((u)->ier = ((u)->spr |= (f)))
#define IER_clr(u, f)   ((u)->ier = ((u)->spr &= (~(f))))
#define IER_reset(u)    ((u)->ier = ((u)->spr = 0));

/*
 * macros to control Interrupt Enable Register in various sensible ways
 */
#define st16c552_EnableTxInterrupt(u)  (IER_set((u), TxReadyInt))
#define st16c552_DisableTxInterrupt(u) (IER_clr((u), TxReadyInt))
#define st16c552_EnableRxInterrupt(u)  (IER_set((u), RxReadyInt))
#define st16c552_DisableRxInterrupt(u) (IER_clr((u), RxReadyInt))

/*
 * macros to get and put serial characters
 */
#define st16c552_GetChar(u)     ((u)->rhr)
#define st16c552_PutChar(u, c)  ((u)->thr = (c))

/*
 * macro to test "fullness" of transmit buffer
 */
#define st16c552_TxBuffFull(u)   (((u)->lsr & (1 << 5)) == 0)
#define st16c552_TxBuffEmpty(u)  (((u)->lsr & (1 << 6)) != 0)

#define st16c552_RxBuffEmpty(u)  (((u)->lsr & (1 << 0)) == 0)


/*
 * prefix for lines not at start of message -- see stuff in putchar too
 */
#define PREFIXLEN   6

/*
 * prototypes
 */

/*
 *  Function: stoi
 *   Purpose: string to integer conversion; like atoi but doesn't pull in
 *            the rest of the c library!
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: s - pointer to (base-10) number in ASCII
 *
 *       Output: e - pointer to last character converted
 *              
 *   Returns: number converted
 */

static int stoi(char *s, char **e)
{
    int i, sign = 0, base = 10;
    while(*s == ' ')
        s++;

    switch(*s)
    {
        case '-':
            sign = -1;
            s++;
            break;

        case '0':
            if (s[1] == 'x')
            {
                base = 16;
                s+=2;
            }
            break;

        case '+':
            s++;
            sign = 1;
            break;

        default:
            sign = 1;
            break;
    }
    i = 0;
    if (base == 10)
    {
        while(*s >= '0' && *s <= '9')
        {
            i = (i * 10) + (*s - '0');
            s++;
        }
    }
    else
    {
        while((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f'))
        {
            if (*s >= 'a')
                i = (i * 16) + (*s - 'a' + 10);
            else
                i = (i * 16) + (*s - '0');
            s++;
        }
    }
    i *= sign;
    *e = s;
    
    return i;
}

void logterm_flushbuf(void);

/*
 *  Function: logterm_Initialise
 *   Purpose: 
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: 
 *
 *   Returns: 0.
 */

/* REMEMBER: This routine gets called VERY EARLY!! */
bool logterm_Initialise(void)
{
    ST16C552Reg *const serchip = (LOGTERM_PORT == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

    /*
     * Interrupt Enable Register - all interrupts off
     */
    IER_reset(serchip);

    /*
     * wait for current transmits to finish
     */
    while (!st16c552_TxBuffEmpty(serchip))
        continue;

    /*
     * Modem Control Register - DTR* and RTS* outputs high; INT
     * pin tri-stated; Rx and Tx "disabled" by setting chip in
     * loopback mode.
     */
    serchip->mcr = 0x10;

    /*
     * FIFO Control Register - FIFO enabled, but Rx and Tx
     * FIFOs empty; RxRDY and TxRDY mode 1; FIFO interrupt
     * trigger level = 1.
     */
    serchip->fcr = 0x0f;

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
     * output is polled, but input is interrupt-driven.
     */
    st16c552_DisableTxInterrupt(serchip);
    st16c552_EnableRxInterrupt(serchip);
    
    /*
     * print a logon banner to say we're here!
     */
    log_emitstr(LOGONMESSAGE);
    log_emitstr(PROMPT);
    
    log_deferredprompt = FALSE;
    log_tracing = TRUE;
    
    log_setupsave(&savebuf, msgsavebuf, SAVEBUFSIZE);    
    log_set_logging_options(WL_SAVEMSG|WL_PRINTMSG);
    log_set_log_id(LOG_ALWAYS, 1);

    /* this works because it's polled input... interrupts
     * are disabled in this code
     */
    log_pause(0,0);
    
    return 0;
}

struct LogSaveBuffer *log_getlogtermbuf(void)
{
    return &savebuf;
}

bool logterm_PreWarn(WarnLevel level)
{
    /*
     * set up the buffer pointers... reset in flushbuf
     */
    logterm_pos = logterm_buf;
    logterm_end = (logterm_buf + sizeof(logterm_buf) - 1);
    logterm_level = level;
    return TRUE;
}

void logterm_flushbuf(void)
{
    char *p;
    
    p = logterm_buf;
    while(p < logterm_pos)
    {
        log_emitch(*p++);
    }

    logterm_pos = logterm_buf;
    logterm_end = logterm_buf + sizeof(logterm_buf) - 1;
}


int logterm_PutChar(char c)
{
    if (logterm_pos >= logterm_end)
        logterm_flushbuf();

    *logterm_pos++ = c;

    return 0;
}

void logterm_PostWarn(unsigned int len)
{
    if (len > 0)
        logterm_flushbuf();
}


/*
 * Show a number nlines of messages from the trace buffer, working
 * back from the current insert position.
 *
 * This is done by starting at the insert position and using the
 * count value (which is at 'insert' - 1) to skip backwards through
 * the buffer until either the oldest data is reached or the required
 * number of lines is found.
 *
 * Then work forward, calling log_logprint() to print the text.
 *
 */
static void log_showitems(struct LogSaveBuffer *sb, int nlines)
{
    int count, message_start = sb->message;
    unsigned long *ptr = sb->current;

    if (ptr == 0 || *ptr == 0) /* nothing to do */
        return;

    ptr--;      /* normally, savebufinsert points at next free slot */

    /*
     * while we haven't got to the start point  -- either the start of
     * data, or the item we want to start with, or the oldest item in
     * the buffer is reached, skip back from the insert point.
     *
     * Note: 'start of data' and 'oldest' are differently encoded -- start
     * of data is indicated by a zero count, written when the buffer is
     * set up. 'oldest' is reached when the current pointer is larger
     * than the insert pointer, and (ptr - count) is less.
     */
    count = nlines;
    while(count > 0 && *ptr != 0)
    {
        /*
         * got to oldest; no more (complete) messages available.
         */
        if (ptr >= sb->current && (ptr - (*ptr & 0xff)) < sb->current)
            break;

        /*
         * if this message marks the end of a line (flagged by the top
         * bits of the word being set), decrement the message number too.
         */
        if (*ptr & ~0xff)
        {
            message_start--;
            count--;
        }

        /* skip one more back, wrapping around the beginning. */
        ptr -= (*ptr & 0xff);
        if (ptr < sb->start)
                ptr += sb->size;
    }

    /*
     * now run forward, printing each line from the data in the buffer. Of
     * course, pointer values are printed from memory, and so may be incorrect
     * if the program changed that memory in the meantime....
     */
    count = nlines;
    while(count > 0)
    {
        log_logprint(message_start, ptr);

        if (*ptr & ~0xff)
        {
            message_start++;
            count--;
        }

        /* skip one more forward, wrapping around the beginning. */
        ptr += *ptr;
        if (ptr >= sb->end)
                ptr -= sb->size;
    }
}


void log_emitch(char ch)
{
    ST16C552Reg *const serchip = (LOGTERM_PORT == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

#ifdef LOGTERM_ADD_CR_TO_LF
    if (ch == '\n')
    {
        while (st16c552_TxBuffFull(serchip))
            continue;
        st16c552_PutChar(serchip, '\r');
    }
#endif

    while (st16c552_TxBuffFull(serchip))
        continue;
    st16c552_PutChar(serchip, ch);
}

void log_emitstr(char *str)
{
    while(*str)
    {
        log_emitch(*str++);
    }
}

/**************************************************************************/
/*                Command Interpreter                                     */
/**************************************************************************/

/*
 *  Function: log_stat
 *   Purpose: Print status report; eventually, this should be able to, for
 *            example, print the number of packets sent, or CRC errors under ADP,
 *            or whatever else. Needs more work!
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "stat").
 *
 *   Returns: 0.
 */
extern struct StatInfo spk_stat_info[];

static int log_stat(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    
    if (__rt_strcmp(argv[1], "serpkt") == 0)
    {
        struct StatInfo *p = spk_stat_info;
        while(p->format != NULL)
        {
            int l;
            
            logterm_PreWarn(WL_INFO);
            l = log_printf(p->format, *p->param);
            logterm_PostWarn(l);
            p++;
        }
    }
    else
        log_emitstr("stat: unknown module\n");
    
    return 0;
}

/*
 *  Function: log_level
 *   Purpose: 
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "level").
 *
 *   Returns: 0.
 */
static int log_level(int argc, char **argv)
{
    WarnLevel lvl;
    
    if (argc == 1)
    {
        lvl = log_get_log_minlevel();
        
        switch(lvl)
        {
            case WL_INFO:
                log_emitstr("level: info\n");
                break;
                
            case WL_WARN:
                log_emitstr("level: warn\n");
                break;
                
            case WL_ERROR:
                log_emitstr("level: error\n");
                break;
        }
    }
    else if (argc == 2)
    {
        
        if (argv[1][0] == 'i')
        {
            log_set_log_minlevel(WL_INFO);
        }
        else if (argv[1][0] == 'w')
        {
            log_set_log_minlevel(WL_WARN);
        }
        else if (argv[1][0] == 'e')
        {
            log_set_log_minlevel(WL_ERROR);
        }
        else
            log_emitstr("level: unknown level name\n");
    }
    else
    {
        log_emitstr("level: bad syntax\n");
    }
    
    return 0;
}

/*
 *  Function: log_go
 *   Purpose: UnPause Angel; allow it to progress after a pause.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "go").
 *
 *   Returns: 0.
 */
volatile static int log_paused = FALSE;

static int log_go(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    if (log_paused)
        log_paused = FALSE;
    else
        log_emitstr("go: not paused\n");
    log_deferredprompt = TRUE;    
    return 0;
}

/*
 *  Function: log_pause
 *   Purpose: Pause Angel; don't allow it to progress. Terminated by 'go'
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "pause").
 *
 *   Returns: 0.
 */

static int log_pause(int argc, char **argv)
{
    ST16C552Reg *const serchip = (LOGTERM_PORT == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;
    int state;

    UNUSED(argc);
    UNUSED(argv);
    
    if (log_paused)
    {
        log_emitstr("pause: Already paused\n");
        return 1;
    }

    state = Angel_DisableInterruptsFromSVC();
    log_paused = TRUE;
    while(log_paused)
    {
        unsigned char ch;

        while(st16c552_RxBuffEmpty(serchip))
            continue;
        ch = st16c552_GetChar(serchip);
        log_processchar(ch, 0);        
    }
    Angel_RestoreInterruptsFromSVC(state);
    
    return 0;
}

/*
 *  Function: log_echo
 *   Purpose: Debugging aid.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_echo(int argc, char **argv)
{
    int i;
    for(i = 0; i < argc; i++)
    {
        log_emitch('\"');
        log_emitstr(argv[i]);
        log_emitch('\"');
        log_emitch(' ');
    }
    log_emitch('\n');
    return 0;
}

/*
 *  Function: log_dump
 *   Purpose: Print the contents of memory.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_dump(int argc, char **argv)
{
    int addr1 = 0, addr2 = 0, len = 0;
    char *e;

    if (argc == 1 || argc > 3)
    {
        log_emitstr("dump: addr1 [addr2 | +nbytes]\n");
        return 0;
    }

    if (argc == 2)
    {
        addr1 = stoi(argv[1], &e);
        addr2 = addr1 + 16;
        len = 16;
    }
    else if (argc == 3)
    {
        if (argv[2][0] == '+')
        {
            len = stoi(argv[2]+1, &e);
            addr2 = addr1 + len;
        }
        else
        {
            addr2 = stoi(argv[2], &e);
            len = (addr2 - addr1) + 1;
        }
    }

    log_dump_buffer(WL_INFO, LOG_BUFFER, 16, (char*)addr1, len);

    return 0;
}

/*
 *  Function: log_ver
 *   Purpose: Version command routine. Prints the Angel banner again.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_ver(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    log_emitstr(ANGEL_BANNER);
    return 0;
}

/*
 *  Function: log_format
 *   Purpose: Set the per-message info
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_format(int argc, char **argv)
{
    if (argc == 1)
    {
        log_emitstr("format: \"");
        log_emitstr(log_get_format_string());
        log_emitstr("\"\n");
    }
    else
    {
        if (argc == 2)
        {
            log_set_format_string(argv[1]);
        }
        else
        {
            log_emitstr("format: too many args\n");
        }
    }
    return 0;
}

/*
 *  Function: log_help
 *   Purpose: Help command routine. Given a single arg, prints the help
 *            string for that command, if it exists.
 *
 *            With no args, prints all help available.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "help").
 *
 *   Returns: 0 if help found, 1 otherwise.
 */

static int log_help(int argc, char **argv)
{
    int i;

    if (argc > 1)
    {
        for (i = 0; i < log_ncmds; i++)
        {
            switch(__rt_strcmp(argv[1], log_cmds[i].str))
            {
                case 0: /* match ! */
                    log_emitstr(log_cmds[i].desc);
                    log_emitstr(log_cmds[i].helpstr);
                    return 0;

                case -1: /* no match found */
                    log_emitstr("No help found?\n");
                    return 1;
            }
        }
    }
    else
    {
        log_emitstr("Help Available:\n");
        for (i = 0; i < log_ncmds; i++)
        {
            log_emitstr(log_cmds[i].str);
            log_emitstr(" - ");
            log_emitstr(log_cmds[i].desc);
        }
    }
    return 0;
}


/*
 *  Function: log_trace
 *   Purpose: The trace action. trace syntax is:
 *       trace <on>|<off>    switch real-time tracing on or off
 *       trace <lines>       show <lines> worth of previous trace history
 *       trace               show currently traced modules
 *       trace <names>       change traced modules; <names> is space separated
 *                           list, with '-' before items to be removed. 'all'
 *                           can be used to specify all modules (so '-all'
 *                           means none).
 * 
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "trace").
 *
 *   Returns: 0
 */

static int log_trace(int argc, char **argv)
{
    int i, rem, n;
    char *p;

    if (argc == 1) /* no args */
    {
        n = log_get_num_ids();
        log_emitstr("trace: ");
        for(i = 0; i < n; i++)
        {
            if (log_get_log_id((log_id)i))
            {
                log_emitstr(log_tracename((log_id)i));
                log_emitch(' ');
            }
        }
        log_emitch('\n');
    }
    else if (argc >= 2) /* at least one arg */
    {
        if (argv[1][0] >= '0' && argv[1][0] <= '9')
        {
            int lines;
            char *ep;
            lines = stoi(argv[1], &ep);
            
            if (ep > argv[1] && lines > 0 && lines < 1000)
            {
                /*
                 * don't want current and old output mixed together, do we?
                 */
                log_output(FALSE);
                log_showitems(&savebuf, lines); 
                log_output(TRUE);
            } 
            else 
                log_emitstr("trace: Bad number.\n"); 
        } 
        else if (strcmp(argv[1], "on") == 0)
        {
            log_emitstr("trace: on (was: "); 
            log_emitstr(log_tracing ? "on)\n" : "off)\n"); 
            log_tracing = TRUE;
        }
        else if (strcmp(argv[1], "off") == 0)
        {
            log_emitstr("trace: off (was: "); 
            log_emitstr(log_tracing ? "on)\n" : "off)\n"); 
            log_tracing = FALSE;
        }
        else
        {
            for(i = 1; i < argc; i++)
            {
                log_id id;

                if (argv[i][0] == '-')
                    rem = TRUE, p = argv[i]+1;
                else if (argv[i][0] == '+')
                    rem = FALSE, p = argv[i]+1;
                else
                    rem = FALSE, p = argv[i];

                id = log_tracebit(p);
                if (id != 0)
                {
                    log_set_log_id(id, !rem);
                }
                else
                {
                    log_emitstr("trace: unknown module ");
                    log_emitstr(p);
                    log_emitch('\n');
                }
            }
        }
    }
    return 0;
}

/*
 *  Function: log_execute
 *   Purpose: Accept a command string which will be split up by into
 *            overwriting spaces into individual strings indexed by
 *            an array of string pointers, to create a C library like
 *            argc/argv pair. Look up the command (the first such string,
 *            if any) in the list of known commands, and if found call
 *            the execution function. If the command is not found, print
 *            an error message ("Eh?"!!) and return.
 *
 *            Unless the command set log_deferredprompt, print a prompt
 *            for the user once command processing complete.
 *
 *  Pre-conditions: log_Initialise called
 *
 *    Params:
 *       Input: cmd - pointer to command string.
 *              
 *   Returns: nothing.
 */

static void log_execute(char *cmd)
{
    char *s = cmd;
    char *argv[MAXARGS];
    int argc, i;

    for(argc = 0; argc < MAXARGS && *s != '\0';)
    {
        while(*s == ' ' || *s == '\t')
            s++;

        /*
         * if quoted arg, search for end of quote;
         * else search for next whitespace.
         */
        if (*s == '\"')
        {
            argv[argc++] = ++s;

            while(*s != '\"' && *s != '\0')
                s++;
        }
        else
        {
            argv[argc++] = s;
            while(*s != ' ' && *s != '\t' && *s != '\0')
                s++;
        }

        /*
         * replace character which ended search (if not EOS)
         * with EOS (to terminate this arg) and increment s
         * so we can search for more...
         */
        if (*s == ' ' || *s == '\t' || *s == '\"')
        {
            *s = '\0';
            s++;
        }
    }

    argv[argc] = NULL;

    if (argv[0] == NULL)
        goto finish;

    for (i = 0; i < log_ncmds; i++)
    {
        switch(strcmp(argv[0], log_cmds[i].str))
        {
            case 0: /* match ! */
                log_cmds[i].pfn(argc, argv);
                break;

            case -1: /* no match found */
                log_emitstr("Eh?\n");
                goto finish;
            
            case 1: /* still looking; */
                break;
        }
    }
finish:
    if (log_deferredprompt == FALSE)
        log_emitstr(PROMPT);
    
    return;
}


/*
 main_table
   "", do_nothing, 0,
   "help", do_execute, helptable,
   NULL,
   
 helptable
   "", do_general_help, helptable,
   "stat", do_specific_help, "Print Statistics\0syntax: ..."

 */

/*
 *  Function: log_serialexecute
 *   Purpose: Wrapper for log_execute, when that function is being called
 *            from the serializer. It basically maintains various locks,
 *            and resets the command buffer...
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: cmd - the command to execute
 *
 *       Output: none
 *              
 *   Returns: none.
 */

static void log_serialexecute(char *cmd)
{
    static char buf[CMDBUFSIZE];

    strcpy(buf, cmd);
    
    log_commandbuf[0] = '\0';
    log_cursor = 0;
    log_buflock = FALSE;
    
    log_emitstr("Executing: ");
    log_emitstr(buf);
    log_output(TRUE);
    
    log_commandbuf[0] = '\0';
    log_cursor = 0;

    log_execute(buf);
    
    log_cmdlock = FALSE;
}

/*
 *  Function: log_processchar
 *   Purpose: CALLED AS AN INTERRUPT ROUTINE!
 *
 *            Process a character received from the controlling
 *            terminal. Form a command string for execution by
 *            log_execute, using the serialiser to enable the
 *            commands to overcome the 'fast' limitation of int
 *            handlers.
 *            
 *            Handle some simplistic echo/deletion codes too.
 *            
 *            If log_deferredprompt is set, we must print a prompt
 *            before eching the character. We must also bracket
 *            the operation with a request to kill run time trace
 *            ouptut temporarily while a command is typed.
 *
 *  Pre-conditions: log_Initialise, interrupt handler set up
 *
 *    Params:
 *       Input: ch - the character read from the terminal
 *              empty-stack - passed in by the int handler for the
 *                            serialiser.
 *              
 *   Returns: nothing.
 */

static void log_processchar(char ch, unsigned int empty_stack)
{
    /* can't process -- previous command hasn't copied buffer yet */
    if (log_buflock)
        return;
    
    if (log_deferredprompt)
    {
        log_emitstr(PROMPT);
        log_output(FALSE);
        log_deferredprompt = FALSE;
    }
    
    switch(ch)
    {
        case '\b':    /* backspace */
        case '\177':  /* delete */
            if (log_cursor > 0)
            {
                log_emitstr("\b \b"); /* delete */
                log_commandbuf[log_cursor--] = '\0';
                log_commandbuf[log_cursor] = '\0';
            }
            if (log_cursor == 0)
            {
                log_deferredprompt = TRUE;
                log_emitstr("\b\b  \b\b"); /* length of prompt */
                log_output(TRUE);
            }
            break;
        
        case '\n':
        case '\r':
            log_emitch('\n');
            if (empty_stack == 0) /* if called from 'pause' */
            {
                log_output(TRUE);
                log_execute(log_commandbuf);
                
                log_commandbuf[0] = '\0';
                log_cursor = 0;
            }
            else
            {
                /* can't execute; previous command hasn't finished yet */
                if (log_cmdlock)
                    break;
                
                /* lock buffer while we're copying it from log_commandbuf;
                   lock command interp. until command completed */
                log_cmdlock = TRUE;
                log_buflock = TRUE;
            
                Angel_SerialiseTask(0,
                                    (angel_SerialisedFn)log_serialexecute,
                                    (void *)log_commandbuf,
                                    empty_stack);
            }
            break;
                
        default:
            if (ch >= 32) /* note: ch signed, so only 32-126 get through (127 above)! */
            {
                log_commandbuf[log_cursor++] = ch;
                log_commandbuf[log_cursor] = '\0';
                log_emitch(ch);
                log_output(FALSE);
            }
            break;
    }
}

/*
 *  Function: log_output
 *   Purpose: To enable/disable real-time trace output while commands are
 *            being typed. If log_tracing is TRUE, then overall we want tracing,
 *            while if false, we don't. log_ouptut is intended to bracket
 *            regions of code when trace would be a bad idea.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: enable          true if we're leaving a sensitive area,
 *                              false if entering one.
 *
 *   Returns: Nothing
 */

static void log_output(int enable)
{
    if (enable && log_tracing)
    {
        log_set_logging_options(log_get_logging_options() | WL_PRINTMSG);
    }
    else
    {
        log_set_logging_options(log_get_logging_options() & ~WL_PRINTMSG);        
    }
}


/*
 *  Function: int_*
 *   Purpose: Set of handlers for the various prioritised interrupts.
 *              These routines do all the urgent processing required
 *              for the interrupt condition
 *
 *  Pre-conditions: Processor is in IRQ / FIQ mode, with a minimal stack,
 *                      AND NO STACK LIMIT SET UP.
 *
 *    Params:
 *       Input: devid           device ID of the driver
 *
 *              port            serial port identifier
 *
 *              serchip            address of the controller for the given port
 *
 *              empty_stack     top of the stack
 *
 *   Returns: Nothing
 */

static void int_msr(unsigned int devid,
                    unsigned int port,
                    ST16C552Reg *const serchip,
                    unsigned int empty_stack)
{
    unsigned int msr;

    UNUSED(empty_stack);
    UNUSED(port);
    UNUSED(devid);

    /*
     * we really couldn't care less about these signals (in
     * fact, we shouldn't really ever get this interrupt
     * because it is never enabled); read the status to clear
     * the interrupt and go away again
     */
    msr = serchip->msr;
}

static void int_txrdy(unsigned int devid,
                      unsigned int port,
                      ST16C552Reg *const serchip,
                      unsigned int empty_stack)
{
    unsigned int lsr;

    UNUSED(empty_stack);
    UNUSED(port);
    UNUSED(devid);

    /*
     * we really couldn't care less about these signals (in
     * fact, we shouldn't really ever get this interrupt
     * because it is never enabled); read the status to clear
     * the interrupt and go away again
     */
    lsr = serchip->msr;
}

static void int_rxrdy(unsigned int devid,
                      unsigned int port,
                      ST16C552Reg *const serchip,
                      unsigned int empty_stack)
{
    UNUSED(devid);
    UNUSED(port);
    UNUSED(empty_stack);

    /*
     * keep looping for as long as there are characters
     * in the Rx FIFO
     */
    while (!st16c552_RxBuffEmpty(serchip))
    {
        unsigned char ch = st16c552_GetChar(serchip);
        log_processchar(ch, empty_stack);
    }
}

static void int_lsr(unsigned int devid,
                    unsigned int port,
                    ST16C552Reg *const serchip,
                    unsigned int empty_stack)
{
    unsigned int lsr;

    IGNORE(devid);
    IGNORE(empty_stack);
    UNUSED(port);

    lsr = serchip->lsr;
}

/*
 *  Function: angel_LogtermIntHandler
 *   Purpose: Entry point for interrupts from the ST16C552 UART
 *            See documentation for angel_IntHandlerFn in devdriv.h
 */
void angel_LogtermIntHandler(unsigned int ident, unsigned int devid,
                             unsigned int empty_stack)
{
    /*
     * jump table for prioritised interrupts
     */
    static void (*const intjump[4])(const unsigned int devid,
                                    const unsigned int port,
                                    ST16C552Reg *const serchip,
                                    const unsigned int empty_stack) =
    {
        int_msr, int_txrdy, int_rxrdy, int_lsr
    };
    
    unsigned int intsrc;
    const unsigned int port = SerCtrl(devid)->port;
    ST16C552Reg *const serchip = (port == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;
    
    IGNORE(ident);

    /*
     * we are only interested in the first three bits of the
     * ISR: bit 0 signals whether another Int. is pending,
     * bits 1-2 indicate the prioritised interrupt; bit 3 is
     * only used to differentiate between a FIFO triggered
     * RxRDY Int and a character timeout RxRDY Int, and we
     * are not interested in this difference
     */
    while (((intsrc = (serchip->isr & 0x07)) & 0x01) == 0)
        intjump[intsrc >> 1](devid, port, serchip, empty_stack);
}

#endif /* DEBUG && !MINIMAL ANGEL */

/* EOF logterm.c */
