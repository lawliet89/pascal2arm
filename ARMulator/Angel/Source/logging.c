/* -*-C-*-
 *
 * $Revision: 1.1.6.19 $
 *   $Author: rivimey $
 *     $Date: 1998/02/24 15:56:14 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * logging.c - methods for logging warnings, errors and trace info
 *
 */

#include <stdarg.h>             /* ANSI varargs support */

#ifdef TARGET

#include "angel.h"
#include "devconf.h"

#else 

#include "host.h"
#include "dbg_hif.h"
/* there are no other options for this: */
#define DEBUG_METHOD logarmdbg

#ifdef __unix
#include <sys/time.h>
#if defined(sun) && !defined(__svr4__)
int gettimeofday(struct timeval *tp, struct timezone *tzp);
#endif
#endif

#endif

#include "logging.h"            /* Header file for this source code */
#include "logpriv.h"            /* Private Header info */
#include "adp.h"
#include "sys.h"

#if DEBUG == 1

/****************************************************************************/

#ifdef TARGET
#include "support.h"

#define strcpy(s, t)   __rt_strcpy(s, t)
#define strcmp(s, t)   __rt_strcmp(s, t)
#define strcat(s, t)   __rt_strcat(s, t)
#else
#include <string.h>
#endif

#ifdef DEBUG_METHOD

#  define  DEBUG_METHOD_HEADER        STRINGIFY(DEBUG_METHOD##.h)
#  include DEBUG_METHOD_HEADER

#  define  METHOD_EXPAND_2(m, p, c) m##p(c)
#  define  METHOD_EXPAND(m, p, c)   METHOD_EXPAND_2(m, p, c)

#  define  CHAROUT(c)    METHOD_EXPAND(DEBUG_METHOD, _PutChar,  (c))
#  define  PRE_DEBUG(l)  METHOD_EXPAND(DEBUG_METHOD, _PreWarn,  (l))
#  define  POST_DEBUG(n) METHOD_EXPAND(DEBUG_METHOD, _PostWarn, (n))

#else
#error Must define DEBUG_METHOD
#endif /* DEBUG_METHOD */

#pragma no_check_stack

/****************************************************************************/
/*                      P R O T O T Y P E S                                 */
/****************************************************************************/

static int log_vprintf(char *format, va_list args);
static void log_log(WarnLevel level, char *format, ...);
/* static int log_printf(char *format, ...);*/
static int buf_itoh(char *buf, unsigned long uval, int width, int padzero);

/****************************************************************************/
/*                      L O C A L   D A T A                                 */
/****************************************************************************/

static const char hextab1[] = "0123456789ABCDEF";
static const char hextab2[] = "0123456789abcdef";

/****************************************************************************/
/*                        F U N C T I O N S                                 */
/****************************************************************************/

#ifdef TARGET
static int log_logging_options = WL_PRINTMSG | WL_SAVEMSG;
#else
static int log_logging_options = WL_PRINTMSG;
#endif
unsigned char log_logging_flags[(int)LOG_ALWAYS];
static char log_formatstring[48] = "%w: %m %l: ";
static bool log_fasttrace = FALSE;   /* must be set true in debugger if req'd */

/* details of the file and line .. set by Log_logmsginfo, which should
 * be called before every log_log(info,warning,error) to indicate the source of
 * the message.
 * Recorded so we can print it only at the start of a line.
 */ 
static char *log_filename = 0; /* remember file & line info; printed only at start of lines */
static int log_line = 0;
static log_id log_module = LOG_NEVER;
static WarnLevel log_min_level = WL_INFO;

static
char *log_tracenames[] =
{
    "(none)",
    "buffer",
    "channel",
    "debug",
    "debughwi",
    "debugos",
    "boot",
    "ctrl",
    "devclnt",
    "serlock",
    "devmisc",
    "devraw",
    "rx",
    "tx",
    "params",
    "sys",
    "pcmcia",
    "st16c552",
    "serpkt",
    "serraw",
    "dccdrv",
    "devshare",
    "info",
    "param",
    "prof",
    "ether",
    "rdi",
    "devsw",
    "comm",
    "serdrv",
    "olicom",
    "fusion",
    "serial",
    "ardi",
    "eice",
    "82595",
    "appl",

    "chanhb",
    "chanto",
    "chanasp",

    "wire",
    "jtag",

    "common",
    NULL /* always */
};

static struct 
{
    ChannelID chan;
    char *str;
    int reason;
} log_adpnames[] =
{
    { CI_TBOOT, "ADP_Booted", ADP_Booted },
    /* { "ADP_TargetResetIndication", ADP_TargetResetIndication }, */
    /* { "ADP_HostResetIndication", ADP_HostResetIndication }, */
    { CI_HBOOT, "ADP_Reboot", ADP_Reboot },
    { CI_HBOOT, "ADP_Reset", ADP_Reset },
    { CI_HBOOT, "ADP_ParamNegotiate", ADP_ParamNegotiate },
    { CI_HBOOT, "ADP_LinkCheck", ADP_LinkCheck },
    { CI_HADP,  "ADP_HADPUnrecognised", ADP_HADPUnrecognised },
    { CI_HADP,  "ADP_Info", ADP_Info },
    { CI_HADP,  "ADP_Control", ADP_Control },
    { CI_HADP,  "ADP_Read", ADP_Read },
    { CI_HADP,  "ADP_Write", ADP_Write },
    { CI_HADP,  "ADP_CPUread", ADP_CPUread },
    { CI_HADP,  "ADP_CPUwrite", ADP_CPUwrite },
    { CI_HADP,  "ADP_CPread", ADP_CPread },
    { CI_HADP,  "ADP_CPwrite", ADP_CPwrite },
    { CI_HADP,  "ADP_SetBreak", ADP_SetBreak },
    { CI_HADP,  "ADP_ClearBreak", ADP_ClearBreak },
    { CI_HADP,  "ADP_SetWatch", ADP_SetWatch },
    { CI_HADP,  "ADP_ClearWatch", ADP_ClearWatch },
    { CI_HADP,  "ADP_Execute", ADP_Execute },
    { CI_HADP,  "ADP_Step", ADP_Step },
    { CI_HADP,  "ADP_InterruptRequest", ADP_InterruptRequest },
    { CI_HADP,  "ADP_HW_Emulation", ADP_HW_Emulation },
    { CI_HADP,  "ADP_ICEbreakerHADP", ADP_ICEbreakerHADP },
    { CI_HADP,  "ADP_ICEman", ADP_ICEman },
    { CI_HADP,  "ADP_Profile", ADP_Profile },
    { CI_HADP,  "ADP_InitialiseApplication", ADP_InitialiseApplication },
    { CI_HADP,  "ADP_End", ADP_End },
    { CI_TADP,  "ADP_TADPUnrecognised", ADP_TADPUnrecognised },
    { CI_TADP,  "ADP_Stopped", ADP_Stopped },
    { CI_TTDCC, "ADP_TDCC_ToHost", ADP_TDCC_ToHost },
    { CI_TTDCC, "ADP_TDCC_FromHost", ADP_TDCC_FromHost },
    { CI_CLIB,  "CL_Unrecognised", CL_Unrecognised },
    { CI_CLIB,  "CL_WriteC", CL_WriteC },
    { CI_CLIB,  "CL_Write0", CL_Write0 },
    { CI_CLIB,  "CL_ReadC", CL_ReadC },
    { CI_CLIB,  "CL_System", CL_System },
    { CI_CLIB,  "CL_GetCmdLine", CL_GetCmdLine },
    { CI_CLIB,  "CL_Clock", CL_Clock },
    { CI_CLIB,  "CL_Time", CL_Time },
    { CI_CLIB,  "CL_Remove", CL_Remove },
    { CI_CLIB,  "CL_Rename", CL_Rename },
    { CI_CLIB,  "CL_Open", CL_Open },
    { CI_CLIB,  "CL_Close", CL_Close },
    { CI_CLIB,  "CL_Write", CL_Write },
    { CI_CLIB,  "CL_WriteX", CL_WriteX },
    { CI_CLIB,  "CL_Read", CL_Read },
    { CI_CLIB,  "CL_ReadX", CL_ReadX },
    { CI_CLIB,  "CL_Seek", CL_Seek },
    { CI_CLIB,  "CL_Flen", CL_Flen },
    { CI_CLIB,  "CL_IsTTY", CL_IsTTY },
    { CI_CLIB,  "CL_TmpNam", CL_TmpNam },
    { 0,  NULL, 0 }
};

static
char *swicalls[] =
{
    "(Unknown)"
    "SYS_OPEN",
    "SYS_CLOSE",
    "SYS_WRITEC",
    "SYS_WRITE0",
    "SYS_WRITE",
    "SYS_READ",
    "SYS_READC",
    "SYS_ISERROR",
    "SYS_ISTTY",
    "SYS_SEEK",
    "SYS_ENSURE",
    "SYS_FLEN",
    "SYS_TMPNAM",
    "SYS_REMOVE",
    "SYS_RENAME",
    "SYS_CLOCK",
    "SYS_TIME",
    "SYS_SYSTEM",
    "SYS_ERRNO",
    "SYS_INSTALL_RAISE",
    "SYS_GET",
    "SYS_HEAPINFO",
};

#if !defined(TARGET) && !defined(COMPILING_ON_WINDOWS)
static struct timeval start_time = {0,0};
#endif

/* Flag: True if log_vprintf etc should print the recorded file/line info before
 * the first line, False if printf is still in middle of a line...
 *
 * The flag is kept up-to-date even if no file/line has been set (and so will
 * not be printed, either).
 * 
 * Initially true, so first line gets printed too..
 */
static int log_perline = TRUE;   


void log_set_begin(void)
{
#if !defined(TARGET) && !defined(COMPILING_ON_WINDOWS)
    gettimeofday(&start_time, NULL);
#endif
}


/*
 * set the log mask; this defines the set of log messages
 * which will appear in the recorded trace and/or the
 * console.
 *
 * Mask values should be taken from the LOG_xxx values).
 */
void log_set_log_id(log_id id, int onoff)
{
    int i;
    
    if (id == LOG_ALWAYS)
    {
        for(i = 0; i < LOG_ALWAYS; i++)
        {
            log_logging_flags[i] = onoff;
        }        
    }
    else if (id == LOG_COMMON)
    {
        for(i = 0; i < LOG_ALWAYS; i++)
        {
            if (i != LOG_CHANTO && i != LOG_CHANHB && i != LOG_CHANASP && i != LOG_SERLOCK)
                log_logging_flags[i] = onoff;
        }        
    }
    else
        log_logging_flags[(int)id] = onoff;
}

/*
 * get the log id.
 */
int log_get_log_id(log_id id)
{
    return log_logging_flags[(int)id];
}

/*
 */
void log_set_log_minlevel(WarnLevel level)
{
    log_min_level = level;
}

/*
 */
WarnLevel log_get_log_minlevel()
{
    return log_min_level;
}


int log_get_num_ids(void)
{
    return (int)LOG_ALWAYS;
}


/*
 * get logging options
 *
 */
int log_get_logging_options()
{
    return log_logging_options;
}

/*
 * set logging options (from one of the flags WL_xx) and return the
 * old options. New options take immediate effect.
 */
int log_set_logging_options(int opts)
{
    int oldopt = log_logging_options;
    log_logging_options = opts;
    return oldopt;
}

/*
 * get format string
 */
char *log_get_format_string()
{
    return log_formatstring;
}

/*
 * set format string
 */
void log_set_format_string(char * str)
{
    strcpy(log_formatstring, str);
}

/*
 *  Function: log_tracebit
 *   Purpose: Return the bit number of the module which has trace name 'name';
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: name   - a string from the tracenames[] array above
 *
 *   Returns: the corresponding bit value.
 */

log_id log_tracebit(char *name)
{
    int j;

    if (name == NULL || name[0] == 0)
        return (log_id)0;
    if (strcmp(name, "all") == 0)
        return LOG_ALWAYS;
    if (strcmp(name, "common") == 0)
        return LOG_COMMON;
    
    for(j = 0; log_tracenames[j] != NULL; j++)
    {
        if (strcmp(name, log_tracenames[j]) == 0)
        {
            return (log_id)j;
        }
    }
    return (log_id)0;
}

/*
 *  Function: log_tracename
 *   Purpose: Return the name of the module which has trace id 'id';
 *            note that this must be a LOG_xxx value.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: id - a value from the log_id enum.
 *
 *   Returns: char * name - the enum name as a string.
 */

char *log_tracename(log_id id)
{
    if (id < LOG_ALWAYS)
        return log_tracenames[(int)id];
    return "(none)";
}

/*
 *  Function: log_adpname
 *   Purpose: Return the name of the module which has adp reason 'reason';
 *            note that this is the LOG_xxx value.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: bit   - a 32 bit word with one bit set corresponding to
 *                      a LOG_xxx value from logging.h
 *
 *   Returns: char * name
 */

char *log_adpname(int reason)
{
    int j;
    long r;
    
    reason &= 0xffffff;
    for(j = 0; log_adpnames[j].str != NULL; j++)
    {
        r = log_adpnames[j].reason;
        if (reason == r)
        {
            return log_adpnames[j].str;
        }
    }
    return "(none)";
}

/*
 *  Function: log_swiname
 *   Purpose: Return the name of the module which has adp reason 'reason';
 *            note that this is the LOG_xxx value.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: bit   - a 32 bit word with one bit set corresponding to
 *                      a LOG_xxx value from logging.h
 *
 *   Returns: char * name
 */

char *log_swiname(int reason)
{
    if (reason < (int)((sizeof(swicalls) / sizeof(swicalls[0]))))
        return swicalls[reason];
    else
        return "(unknown)";
}

#ifdef NO_LOG_INFO
void 
log_dump_buffer(WarnLevel lvl, log_id mod, unsigned int bpl, char *buffer, unsigned int length)
{}
#else
void 
log_dump_buffer(WarnLevel lvl, log_id mod, unsigned int bpl, char *buffer, unsigned int length)
{
    unsigned int i;
    unsigned int j;
    char b[128];

    Log_logmsginfo( log_file, __LINE__, mod);
    if (length > 256)
    {
        log_log(lvl, "Buffer %x, length %d (truncated to 256)\n", buffer, length);
        length = 256;
    }
    else
    {
        log_log(lvl, "Buffer %x, length %d\n", buffer, length);
    }
    
    for (i = 0; i < length; i += bpl)
    {
        char *p = b;
        
        p += buf_itoh(p, i, 3, 1);
        *p++ = ':';
        *p++ = ' ';
        
        for (j = 0; j < 16 && (i + j) < length; j++)
        {
            p += buf_itoh(p, buffer[i + j] & 0xff, 2, 1);
            *p++ = ' ';
        }
        for (; j <= bpl; j++)
        {
            *p++ = ' ';
            *p++ = ' ';
            *p++ = ' ';
        }
        for (j = 0; j < bpl && (i + j) < length; j++)
        {
            unsigned char c = buffer[i + j];

            *p++ = (c >= 0x20 && c < 0x7F) ? c : '.';
        }
        *p++ = '\0';
        Log_logmsginfo( log_file, __LINE__, mod);
        log_log(lvl, "%s\n", b);
    }
}
#endif

/*
 * Parse the initial segment of a printf % specifier, returning the
 * base code (d, x, s etc) to the caller and updating format to point
 * to that character in the source string. *format should point to the
 * initial %.
 *
 * width is the field width (the precision is not supported), padzero is
 * TRUE if field width began with a 0, longval is TRUE if (ld, lx etc)
 * was seen.
 *
 * A leading '-' on the field width (%-8s etc) is ignored.
 */
static char log_readformat(char **format, int *width, int *padzero, int *longval)
{
    char fch = *(++(*format));

    /*
     * Check if the format has a width specified. NOTE: We do
     * not use the "isdigit" function here, since it will
     * require run-time support. The current ARM Ltd header
     * defines "isdigit" as a macro, that uses a fixed
     * character description table.
     */
    if (fch == '-')
        fch = *(++(*format));   /* ignore right-adjust flag */

    if ((fch >= '0') && (fch <= '9'))
    {
        if (fch == '0')
        {
            /* Leading zeroes padding */
            *padzero = TRUE;
            fch = *(++(*format));
        }

        while ((fch >= '0') && (fch <= '9'))
        {
            *width = (((*width) * 10) + (fch - '0'));
            fch = *(++(*format));
        }
    }

    if (fch == 'l')
    {
        /* skip 'l' in "%lx", etc. */
        *longval = TRUE;
        fch = *(++(*format));
    }
    return fch;
}

/*
 * Convert the long value ival to text, writing the result character
 * by character using the 'CHAROUT' macro in hexadecimal. 
 *
 * fch is used to specify one of the three variants used: 'p' for
 * the pointer type (0x00224AFDB etc) 'X' for hex numbers with leading
 * 0x and capital letter A-F, 'x' for hex numbers with no leading 0x
 * and lower-case a-f.
 *
 * width is the field width, which may be exceeded if representation
 * of the value demands, pad is TRUE if the value should be padded to
 * to the left up to the width
 *
 */

static int log_itoh(char fch, unsigned long uval, int width, int padzero)
{
    int count, loop;
    int len = 0, mark = FALSE;
    const char *hextab;
    char buffer[12];  /* stack space used to hold number */

    if (fch == 'X' || fch == 'p')
    {
        hextab = hextab1;
        if (fch == 'p')
            mark = TRUE;
    }
    else
    {
        hextab = hextab2;
    }

    /*
     * Read each nibble from high to low; unless it's zero copy the
     * hex character equivalent into the buffer. Note we start from
     * bit (n-4) where (n) is the number of bits in the word, as
     * loop is the base bit number (i.e. the nibble is from the bit
     * at 'loop' to the bit at 'loop+3'.
     */
    count = 0;
    for (loop = (sizeof(long) * 8) - 4; loop >= 0; loop -= 4)
    {
        int nibble = (uval >> loop) & 0xF;
        if (nibble != 0 || count != 0)
        {
            buffer[count++] = hextab[nibble];
        }
    }

    if (count == 0)
        buffer[count++] = '0';

    if (width != 0)
      {
        width -= count + (mark? 2: 0);

        if (padzero)
          {
            if (mark)
              {
                CHAROUT('0');
                CHAROUT('x');
                len += 2;
              }
            for (; (width > 0); width--)
              {
                CHAROUT('0');
                len++;
              }
          }
        else
          {
            for (; (width > 0); width--)
              {
                CHAROUT(' ');
                len++;
              }
            if (mark)
              {
                CHAROUT('0');
                CHAROUT('x');
                len += 2;
              }
          }
      }
    else if (mark)
      {
        CHAROUT('0');
        CHAROUT('x');
        len += 2;
      }
    
    for (loop = 0; loop < count; loop++)
    {
        CHAROUT(buffer[loop]);
        len++;
    }
    return len;
}

/*
 * Convert the long value ival to text, writing the result character
 * by character using the 'CHAROUT' macro in hexadecimal. 
 *
 * fch is used to specify one of the three variants used: 'p' for
 * the pointer type (0x00224AFDB etc) 'X' for hex numbers with leading
 * 0x and capital letter A-F, 'x' for hex numbers with no leading 0x
 * and lower-case a-f.
 *
 * width is the field width, which may be exceeded if representation
 * of the value demands, pad is TRUE if the value should be padded to
 * to the left up to the width
 *
 */

static int buf_itoh(char *buf, unsigned long uval, int width, int padzero)
{
    int count, loop;
    int len = 0;
    char buffer[12];  /* stack space used to hold number */

    count = 0;
    for (loop = (sizeof(long) * 8) - 4; loop >= 0; loop -= 4)
    {
        int nibble = (uval >> loop) & 0xF;
        if (nibble != 0 || count != 0)
        {
            buffer[count++] = hextab2[nibble];
        }
    }

    if (count == 0)
        buffer[count++] = '0';

    if (width != 0)
    {
        width -= count;
        
        for (; (width > 0); width--)
        {
            *buf++ = (padzero)?'0':' ';
            len++;
        }
    }
    
    for (loop = 0; loop < count; loop++)
    {
        *buf++ = (buffer[loop]);
        len++;
    }
    return len;
}

/*
 * Convert the long value ival to text, writing the result character
 * by character using the 'CHAROUT' macro in decimal. 
 *
 * width is the field width, which may be exceeded if representation
 * of the value demands, padzero is TRUE if the value should be padded to
 * to the left up to the width, sign is TRUE if the value should be
 * considered to be a signed number.
 *
 * note: the sign char is not counted in the field width [BUG]
 */

static int log_itod(long ival, int width, int padzero, int sign)
{
    int count;
    int len = 0;
    int writeminus = FALSE;
    char buffer[12];  /* stack space used to hold number */

    if (sign && (ival < 0))
    {
        ival = -ival;
        writeminus = TRUE;
    }

    /*
     * The simplest method of displaying numbers is to
     * provide a small recursive routine. However, to reduce
     * stack usage the following non-recursive solution is
     * used.
     */

    /*
     * Place the conversion into the buffer in
     * reverse order:
     */
    count = 0;
    while (ival != 0)
    {
        buffer[count++] = (char)('0' + ((unsigned long)ival % 10));
        ival = ((unsigned long)ival / 10);
    }

    if (count == 0)
        buffer[count++] = '0';

    /*
     * Check if we are placing the data in a fixed 
     * width field, and write the minus in the right
     * place relative to the padding.
     */
    if (width != 0)
    {
        width -= count + (writeminus ? 1 : 0);

        if (padzero)
        {
            if (writeminus)
            {    
                CHAROUT('-');
                len++;
            }
            for (; (width > 0); width--)
            {
                CHAROUT('0');
                len++;
            }
        }
        else
        {
            for (; (width > 0); width--)
            {
                CHAROUT(' ');
                len++;
            }                
            if (writeminus)
            {    
                CHAROUT('-');
                len++;
            }
        }
    }
    else if (writeminus)
    {    
        CHAROUT('-');
        len++;
    }

    /* then display the buffer in reverse order */
    for (; (count != 0); count--)
    {
        CHAROUT(buffer[count - 1]);
        len++;
    }
    return len;
}

/*
 * Convert the zero-terminated string stored in memory at address 'string'
 * to characters passed to CHAROUT. If width is non-zero, and is larger
 * than the string length, then pad characters will be used to make up the
 * string to this length, padding on the right. If the pointer value is NULL
 * the characters "(nil)" will be written instead.
 */
static int log_ptos(char *string, int width)
{
    int len = 0;

    if (string != NULL)
    {
        while (*string)
        {
            /*
             * NOTE: We do not use "*string++" as the macro
             * parameter, since we do not know how many times
             * the parameter may be expanded within the macro.
             */
            CHAROUT(*string);
            len++;
            string++;
        }
        if (width != 0)
        {
            width -= len;

            for (; (width > 0); width--)
            {
                CHAROUT(' ');
                len++;
            }
        }
    }
    else
    {
        CHAROUT('(');
        CHAROUT('n');
        CHAROUT('i');
        CHAROUT('l');
        CHAROUT(')');
        len += 5;
    }
    return len;
}

/*
 * Simulate 'printf' using the CHAROUT macro. Read a format string to interpret
 * the n args following (n >= 0) as pointers, integers or characters (as ints).
 * NOTE: Double, float values, precision values (%.8) and variable width fields
 * are not supported.
 * 
 * The number of characters printed is returned.
 */

int 
log_printf(char *format, ...)
{
    va_list args;
    int l;

    va_start(args, format);
    l = log_vprintf(format, args);
    va_end(args);

    return l;
}

static int 
log_vprintf(char *format, va_list args)
{
    int len = 0;

    while ((format != NULL) && (*format != '\0'))
    {
        if (*format == '%')
        {
            char fch;           /* get format character (skipping '%') */
            int width = 0;      /* No field width by default */
            int padzero = FALSE;  /* By default we pad with spaces */
            int longval = FALSE;  /* seen 'ld' etc? */

            fch = log_readformat(&format, &width, &padzero, &longval);
            switch (fch)
            {
                case 'c':
                    /* char => ignore longval */
                    {
                        int ival = va_arg(args, int);

                        CHAROUT((char)ival);
                        len++;
                        break;
                    }

                case 'p':
                    /* hexadecimal pointer => ignore longval */
                    {
                        void *vp = va_arg(args, void *);

                        if (width == 0)
                        {
                            /* default format: 8 digits wide, leading "0x", zero padded */
                            width = 8;
                            padzero = TRUE;
                        }

                        len += log_itoh(fch, (unsigned long)vp, width, padzero);
                        break;
                    }

                case 'X':
                case 'x':
                    /* hexadecimal */
                    /* default format: min width */
                    if (longval)
                    {
                        unsigned long luval = va_arg(args, unsigned long);
                        len += log_itoh(fch, luval, width, padzero);
                    }
                    else
                    {
                        unsigned int uval = va_arg(args, unsigned int);
                        len += log_itoh(fch, (unsigned long)uval, width, padzero);
                    }
                    break;

                case 'i':
                case 'd':
                    /* decimal */
                    /* default format: min width */
                    if (longval)
                    {
                        long lival = va_arg(args, long);
                        len += log_itod(lival, width, padzero, TRUE);
                    }
                    else
                    {
                        int ival = va_arg(args, int);
                        len += log_itod((long)ival, width, padzero, TRUE);
                    }

                    break;

                case 'u':
                    /* unsigned decimal */
                    /* default format: min width */
                    if (longval)
                    {
                        unsigned long lival = va_arg(args, unsigned long);
                        len += log_itod((long)lival, width, padzero, FALSE);
                    }
                    else
                    {
                        unsigned int ival = va_arg(args, unsigned int);
                        len += log_itod((long)ival, width, padzero, FALSE);
                    }

                    break;

                case 's':
                    /* string => ignore longval */
                    {
                        char *string = va_arg(args, char *);
                        len += log_ptos(string, width);
                    }
                    break;

                case '\0':
                    /*
                     * string terminated by '%' character, bodge things
                     * to prepare for default "format++" below
                     */
                    format--;

                    break;

                default:
                    /* just display the character */
                    CHAROUT(*format);
                    len++;

                    break;
            }

        }
        else
        {
            CHAROUT(*format);
            len++;
        }            
        format++;
    }
    return len;
}

/*
 * Routine to return the bit number of the single bit set in 'x'.
 *
 * Multiply the value 'x' by the 'magic number' 0x450FBAF
 * extracting the most significant 6 bits of the 32 bit
 * answer. The result is a unique value.
 * take this unique value and return it's bit postition via a[].
 *
 * Algorithm courtesy D. Seal.
 */
int __rt_bitnumber(unsigned long x)
{
    static unsigned char a[64] =
    {
        0, 0, 1, 12, 2, 6, 0, 13, 3,  0,  7, 0, 0, 0, 0, 14, 10, 4, 0, 0,
        8, 0, 0, 25, 0, 0, 0, 0,  0, 21, 27, 15, 31, 11, 5, 0, 0, 0, 0, 0,
        9,  0, 0, 24, 0, 0, 20, 26, 30, 0, 0, 0, 0, 23, 0, 19, 29, 0, 22,
        18, 28, 17, 16, 0
    };
    if (x == 0)
        return 0xff;
    
    x *= 0x11;   /* multiply by 17 -> total factor 0x11 */
    x *= 0x41;   /* multiply by 65 -> total factor 0x451 */
    x *= 0xffff; /* multiply by 65535 -> total factor 0x450FBAF */
    return (int)(a[x >> 26]);
}

/*
 * Save the current format and args in the message save buffer.
 * The format must be parsed to determine the number and approximate
 * types of the args, then these args must be saved in the buffer.
 * The data:
 *
 *   ulong format, (line, file), flags, <args>, ulong count
 *
 * where (line,file) are compile-time optional.
 *
 * should be appended to the message buffer, starting at savebufinsert, 
 * when this routine exits. Args are all converted to unsigned long
 * type, as this is the type of the buffer. Note that the count
 * value includes itself and the format, so it's minimum value is 2.
 * A count of zero is used in the buffer to indicate start-of-data.
 */
static void 
log_logsave(struct LogSaveBuffer *sb, WarnLevel level,
             char *format, va_list args)
{
    int argcount = 0;
    char *file;
    int line;
    log_id id;
    
    /*
     * This code assumes pointers will fit in an unsigned long value. If
     * this is not true it WILL BREAK!!
     */
    /* ASSERT(sizeof(unsigned long) >= sizeof(void *), "pointer size problem"); */

    sb->message++;
    log_saveitem(sb, (unsigned long)format);
    log_getmsginfo(&file, &line, &id);
    log_saveitem(sb, (unsigned long)line);
    log_saveitem(sb, (unsigned long)file);
    log_saveitem(sb, level | (id & 0xff) << 8);
    
    while ((format != NULL) && (*format != '\0'))
    {
        if (*format == '%')
        {
            char fch;           /* get format character (skipping '%') */
            int ival;           /* holder for integer arguments */
            char *string;       /* holder for string arguments */
            void *vp;
            int longval = FALSE;  /* seen 'ld' etc? */

            /* these aren't needed, except by log_readformat() ... sigh */
            int width = 0; 
            int padzero = FALSE;/* seen '04' etc? */

            fch = log_readformat(&format, &width, &padzero, &longval);
            switch (fch)
            {
                case 'c':
                    /* char */
                    ival = va_arg(args, int);
                    log_saveitem(sb, (unsigned long)ival);
                    argcount++;
                    break;

                case 'p':
                    /* hexadecimal pointer */
                    vp = va_arg(args, void *);
                    log_saveitem(sb, (unsigned long)vp);
                    argcount++;
                    break;

                case 'd':
                case 'u':
                case 'i':
                case 'x':
                case 'X':
                    if (longval)
                    {
                        unsigned long uval = va_arg(args, unsigned long);
                        log_saveitem(sb, uval);
                        argcount++;
                    }
                    else
                    {
                        unsigned int uval = va_arg(args, unsigned int);
                        log_saveitem(sb, (unsigned long)uval);
                        argcount++;
                    }
                    break;

                case 's':
                    /* string */
                    string = va_arg(args, char *);
                    log_saveitem(sb, (unsigned long)string);
                    argcount++;
                    break;

                case '\0':
                    /*
                     * string terminated by '%' character, bodge things
                     * to prepare for default "format++" below
                     */
                    format--;
                    break;

                default:
                    break;
            }
        }
        format++;
    }
    
    /*
     * Save the count of items in this 'entry' -- format + (f,l) + flags + n args + count
     * However, we must also save whether this item completes a line of text.
     * This information is used to manipulate message number counts etc.
     */
    argcount += 5;
    if (format != NULL && *(format-1) == '\n')
        log_saveitem(sb, (unsigned long)argcount | 0xFF00L);
    else
        log_saveitem(sb, (unsigned long)argcount);
    return;
}

/*
 * Interpret strings with %x for various x as format strings
 * for the per-line trace info.
 *
 * x is one of:
 *     w -- warn level -> "I", "W", "E" for Info -> Error
 *     M -- module no
 *     m -- module name, by calling log_tracename() on the bits arg
 *     l -- line number in 4 chars
 *     f -- file name (may be null if not compiled DEBUG_FILE_TOO)
 *     n -- message number in 4 chars
 *     % -- the % character
 *
 * sequences other than these are printed verbatim.
 */

int
log_formatinfo(char *str, int lineno, char *file,
               log_id id, WarnLevel level, long msgno)
{
    char *s = str;
    int len = 0;

    while(*s)
    {
        if (*s == '%')
        {
            s++;
            switch(*s)
            {
                case '%':
                    CHAROUT('%');
                    len++;
                    break;
                    
                case 'w':
                    switch(level)
                    {
                        case WL_INFO:
                            CHAROUT('I');
                            len++;
                            break;
                            
                        case WL_WARN:
                            CHAROUT('W');
                            len++;
                            break;
                            
                        case WL_ERROR:
                            CHAROUT('E');
                            len++;
                            break;
                            
                        default:
                            CHAROUT('%');
                            CHAROUT('w');
                            len+=2;
                            break;
                    }
                    break;
                case 'm':
                    if (id == 0)
                        len += log_printf("%8s", "(unset)");
                    else
                        len += log_printf("%8s", log_tracename(id));
                    break;
                    
                case 'l':
                    len += log_printf("%4d", lineno);
                    break;
                    
                case 'M':
                    len += log_printf("%2d", id);
                    break;
                    
                case 'f':
                    if (file == NULL)
                        len += log_printf("%15s", "(unset)");
                    else
                        len += log_printf("%15s", file);
                    break;
                    
                case 'n':
                    len += log_printf("%4d", msgno);
                    break;

#if !defined(TARGET) && !defined(COMPILING_ON_WINDOWS)
                case 't':
                {
                    long sdiff, usdiff;
                    struct timeval x;
                    
                    gettimeofday(&x, NULL);
                    usdiff = x.tv_usec - start_time.tv_usec;
                    sdiff = x.tv_sec - start_time.tv_sec;
                    
                    if (usdiff < 0)
                    {
                        /* borrow 1 sec */
                        sdiff--;
                        usdiff = 1000000 + usdiff;
                    }
                    len += log_printf("%2d.%06d", sdiff, usdiff);
                }
                break;
#endif                                      
                default:
                    CHAROUT('%');
                    CHAROUT(*s);
                    len+=2;
                    break;
            }
        }
        else
        {
            CHAROUT(*s);
            len++;
        }
        s++;
    }
    return len;
}

    
/*
 * Simulate 'printf' using log_emitch. Read the buffer to get the
 * format string and any required args; these are stored in a ulong* buffer:
 *
 *   +========+======+======+=============+======+======+==========+=======+===
 *   | format | file | line | flags/level | arg1 | arg2 | ... argn | count |  ...
 *   +========+======+======+=============+======+======+==========+=======+===
 *
 * where:
 *   line is the line number of the statement which started the message
 *   file is a pointer to the file name containing the message
 *   flags/level is split: the LS byte is the level number of the message,
 *                         the next-LS byte is the bit number of the module bitmask
 *                         top 16 bits unused.
 *   format is a pointer to a printf-style format string,
 *   arg1..argn are any arguments implied by 'format' (no args is ok),
 *   count is the number of long int values used, including count itself.
 *
 * So:
 *   printf("hello") has a count of 7,
 *   printf("value %d x %d is %d", a, b, c) has a count of 8.
 *
 * the arg 'ptr' points at 'format' in the buffer.
 *
 * NOTE: Double, float values, precision values (%.8) and variable width fields
 * are not supported.
 * 
 * The number of characters printed is returned.
 */

int 
log_logprint(long msgno, unsigned long *ptr)
{
    int len = 0, lineno, flags;
    log_id id;
    char *format, *file;
    WarnLevel level;
    
#if defined(TARGET) && (!defined(LOGTERM_DEBUGGING) || LOGTERM_DEBUGGING == 0)
    static char firsttime = 1;
    if (firsttime)
    {
        log_set_log_id(LOG_COMMON, 1);
        firsttime = 0;
    }
#endif

    format = (char *)*ptr++;
    lineno = (int)*ptr++;
    file = (char *)*ptr++;
    flags = (int)*ptr++;
    id = (log_id)((flags >> 8) & 0xFF);
    level = (WarnLevel)(flags & 0xFF);

    if ((log_logging_flags[(int)id]) == 0)
        return 0;

    if (level < log_min_level)
        return 0;
    
    /* print the per-line information */
    if (log_perline)
    {
        log_perline = FALSE;
        log_formatinfo(log_formatstring, lineno, file, id, level, msgno);
    }
    
    /*
     * This code assumes pointers will fit in an unsigned long value. If
     * this is not true it WILL BREAK!!
     */
    /* ASSERT(sizeof(unsigned long) >= sizeof(void *), "pointer size problem"); */

    while ((format != NULL) && (*format != '\0'))
    {
        if (*format == '%')
        {
            char fch;           /* get format character (skipping '%') */
            int width = 0;      /* No field width by default */
            int padzero = FALSE;  /* By default we pad with spaces */
            int longval = FALSE;  /* seen 'ld' etc? */

            fch = log_readformat(&format, &width, &padzero, &longval);
            switch (fch)
            {
                case 'c':
                    /* char => ignore longval */
                    {
                        int ival = (int)*ptr++;

                        CHAROUT((char)ival);
                        len++;
                        break;
                    }

                case 'p':
                    /* hexadecimal pointer => ignore longval */
                    {
                        void *vp = (void*)*ptr++;

                        if (width == 0)
                        {
                            /* default format: 8 digits wide, leading "0x", zero padded */
                            width = 8;
                            padzero = TRUE;
                        }

                        len += log_itoh(fch, (unsigned long)vp, width, padzero);
                        break;
                    }

                case 'X':
                case 'x':
                    /* hexadecimal */
                    /* default format: min width */
                    if (longval)
                    {
                        unsigned long luval = (unsigned long)*ptr++;
                        len += log_itoh(fch, luval, width, padzero);
                    }
                    else
                    {
                        unsigned long uval = (unsigned int)*ptr++;
                        len += log_itoh(fch, (unsigned long)uval, width, padzero);
                    }
                    break;

                case 'i':
                case 'd':
                    /* decimal */
                    /* default format: min width */
                    if (longval)
                    {
                        long lival = (long)*ptr++;
                        len += log_itod(lival, width, padzero, TRUE);
                    }
                    else
                    {
                        int ival = (int)*ptr++;
                        len += log_itod((long)ival, width, padzero, TRUE);
                    }
                    break;

                case 'u':
                    /* unsigned decimal */
                    /* default format: min width */
                    if (longval)
                    {
                        unsigned long lival = (unsigned long)*ptr++;
                        len += log_itod((long)lival, width, padzero, FALSE);
                    }
                    else
                    {
                        unsigned int ival = (unsigned int)*ptr++;
                        len += log_itod((long)ival, width, padzero, FALSE);
                    }
                    break;

                case 's':
                    /* string => ignore longval */
                    {
                        char *string = (char *)*ptr++;
                        len += log_ptos(string, width);
                    }
                    break;

                case '\0':
                    /*
                     * string terminated by '%' character, bodge things
                     * to prepare for default "format++" below
                     */
                    format--;

                    break;

                default:
                    /* just display the character */
                    CHAROUT(*format);
                    len++;

                    break;
            }

        }
        else
        {
            CHAROUT(*format);
            len++;
            /* if we come to a newline, set log_perline, so we print out the
             * file and line number when we get the next line.
             *
             * Unless, of course, the next line is in this format, in which
             * case we must print it now...
             *
             * [note: we should check %s args for \n too...]
             */
            if (*format == '\n')
            {
                if (*(format+1) == '\0')
                    log_perline = TRUE;
                else
                {
                    log_perline = FALSE;
                    log_formatinfo(log_formatstring, lineno, file, id, level, msgno);
                }
            }
            else
                log_perline = FALSE;
        }
        format++;           /* step over character */
    }
    return len;
}

/*
 * Set up save buffer for a new message. Ensure the insert pointer points
 * into the buffer, and that an intiial empty buffer has a zero in the first
 * location (the count item) as a terminator. Incrememnt a message number
 * variable indicating the current message number (the first message will
 * thus be 1).
 */
void log_setupsave(struct LogSaveBuffer *sb,
                      unsigned long *buffer, long size)
{
    sb->start = buffer;
    sb->current = buffer;
    sb->size = size;
    sb->end = buffer + size;
    sb->message = 0;
    log_saveitem(sb, 0);        /* end condition for backwards search */
}

/*
 * Save a new item (format pointer, arg, count) to the save buffer, wrapping
 * when the buffer limit is reached.
 */
void log_saveitem(struct LogSaveBuffer *sb, unsigned long item)
{
    *(sb->current) = item;
    sb->current++;

    /* implement wrap-around for the permanent save buffer; callers
     * using their own buffer must implement their own safety checks..
     */
    if (sb->current == sb->end)
    {
        sb->current = sb->start;
    }
}

void
Log_logmsginfo(char *file, int line, log_id id)
{
    log_filename = file;
    log_line = line;
    log_module = id;
}

void log_getmsginfo(char **file, int *line, log_id *id)
{
    *file = log_filename;
    *line = log_line;
    *id = log_module;
}

void 
log_vlog(WarnLevel level, char *format, va_list args)
{
    int len;
    unsigned long savebuf[16], *ptr;
    struct LogSaveBuffer tmpbuf;
    struct LogSaveBuffer *buf;
    long msgno = 0;
    
    /*
     * if we want a permanent copy of the message, get a copy of
     * the save location. If not, just use the temporary buffer above.
     */
#if defined(LOGTERM_DEBUGGING) && LOGTERM_DEBUGGING == 1
    if (log_logging_options & WL_SAVEMSG)
    {
        buf = log_getlogtermbuf();
        msgno = buf->message;
    }
    else
    {
        log_setupsave(&tmpbuf, savebuf, 16);
        buf = &tmpbuf;
    }
#else

    log_setupsave(&tmpbuf, savebuf, 16);
    buf = &tmpbuf;

#endif
    
    /*
     * stuff will be saved from here onwards; save this location
     * so we don't have to calculate it...
     */
    ptr = buf->current;

    /*
     * save the info to the save place. We can't call log_vprintf
     * here because we can't parse 'args' twice :-(
     */
    log_logsave(buf, level, format, args);
    
    if (log_logging_options & WL_PRINTMSG)
    {
        if (PRE_DEBUG(level))
        {
            len = log_logprint(msgno, ptr);
            POST_DEBUG(len);
        }
    }
}

/* see logging.h */
void 
log_log(WarnLevel lvl, char *format,...)
{
    va_list args;

    va_start(args, format);
    log_vlog(lvl, format, args);
    va_end(args);

    return;
}

/* see logging.h */
void 
log_logwarning(char *format,...)
{
    va_list args;

    va_start(args, format);
    log_vlog(WL_WARN, format, args);
    va_end(args);

    return;
}

/* see logging.h */
void 
log_loginfo(char *format,...)
{
    va_list args;

    va_start(args, format);
    log_vlog(WL_INFO, format, args);
    va_end(args);

    return;
}

/* see logging.h */
void 
log_logerror(char *format,...)
{
    va_list args;

    va_start(args, format);
    log_vlog(WL_ERROR, format, args);
    va_end(args);
    return;
}

/* see logging.h */
#undef LogTrace
void 
LogTrace(char *format,...)
{
    va_list args;

    if (log_fasttrace)
    {
        va_start(args, format);
        log_vprintf(format, args);
        va_end(args);
    }

    return;
}

#endif /* DEBUG */


/* EOF logging.c */
