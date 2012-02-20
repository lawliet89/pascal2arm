/* -*-C-*-
 *
 *    CVS $Revision: 1.1.6.11 $
 * Revising $Author: rivimey $
 *            $Date: 1998/02/24 15:56:15 $
 *
 * Copyright (c) 1996-1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * logging.h - methods for logging warnings, errors and trace info
 */

#ifndef angel_logging_h
#define angel_logging_h

#include <stdarg.h>

/*
 * values for 'bits' below. So far, they've all corresponded to specific files.
 * To conserve bits, however, it is suggested that where modules are specific
 * to a port, they reuse bits where possible.
 *
 * Further expansion could be implemented by having subsidiary bits, or changing
 * it to be an index into a global array.
 */

typedef enum _log_id
{
    LOG_NEVER = 0,
    LOG_BUFFER,
    LOG_CHANNEL,
    LOG_DEBUG,
    LOG_DEBUGHWI,
    LOG_DEBUGOS,
    LOG_BOOT,
    LOG_CTRL,
    LOG_DEVCLNT,
    LOG_SERLOCK,
    LOG_DEVMISC,
    LOG_DEVRAW,
    LOG_RX,
    LOG_TX,
    LOG_PARAMS,
    LOG_SYS,      /* sys.c / hsys.c */
    LOG_PCMCIA,   /* pid/pcmcia */
    LOG_ST16C552, /* pid/st16c552 */
    LOG_SERPKT,
    LOG_SERRAW,
    LOG_DCCDRV,
    LOG_DEVSHARE,
    LOG_INFO,
    LOG_PARAM,
    LOG_PROF,     /* profiling */
    LOG_ETHER,    /* pid/ethernet */
    LOG_RDI,      /* host RDI, comms level */
    LOG_DEVSW,
    LOG_COMM,     /* target comms I/F */
    LOG_SERDRV,   /* */
    LOG_OLICOM,   /* pid/olicom.c */
    LOG_FUSION,   /* fusion IP stack in general */
    LOG_SERIAL,   /* pie/serial.c */
    LOG_ARDI,     /* host RDI, ADP level */
    LOG_ICE,      /* Embedded ICE */
    LOG_82595,    /* olicom ethernet driver */
    LOG_APPL,     /* hostappl etc */

    LOG_CHANHB,   /* heartbeat - these three are here because they produce */
    LOG_CHANTO,   /* timeout   - so much output that you rarely want them! */
    LOG_CHANASP,  /* async processing -                                    */
    
    LOG_WIRE,     /* logging msgs showing bytes on serial/serpar cable */
    
    LOG_JTAG,     /* Jtag device, for eiceadp */

    /* pseudo-codes */
    LOG_COMMON,
    LOG_ALWAYS   /* MUST BE LAST */
} log_id;

/*---------------------------------------------------------------------------*/

typedef enum WarnLevel {
    WL_TRACE,
    WL_INFO,
    WL_WARN,
    WL_ERROR
} WarnLevel;

struct StatInfo
{
    char *format;
    unsigned long *param;
};

/*---------------------------------------------------------------------------*/

/*
 * The logging mask is used to skip messages from modules at run-time;
 * a bit is allocated to each message, and the message only output if
 * the mask has that bit set. LogTerm does this internally; other debug
 * schemes do it in-place.
 */
extern unsigned char log_logging_flags[];

/*
 * LogInfo
 * ----------
 * Provides a standard method of generating run-time system
 * informational messages. The actual action taken by this code can be
 * board or target application specific, e.g. internal logging, debug
 * message, etc.
 * 
 */
void log_loginfo(char *format, ...);

/*
 * LogWarning
 * ------------
 * Provides a standard method of generating run-time system
 * warnings. The actual action taken by this code can be board or
 * target application specific, e.g. internal logging, debug message,
 * etc.
 */
void log_logwarning(char *format, ...);


/*
 * LogError
 * ----------
 * Raise an internal Angel error. The parameters are passed directly
 * to "LogWarning" for display, and the code then raises a debugger
 * event and stops the target processing.
 */
void log_logerror(char *format, ...);

/*
 * log_vlog
 * ----------
 * Generic interface to other Log* functions. Use Deprecated.
 */
void log_vlog(WarnLevel level, char *format, va_list args);

/*
 * LogTrace
 * ----------
 * Provides a standard method of generating short messages. The point of
 * trace is that it is FAST; generally therefore messages must be short,
 * sweet and to the point... preferably less than 4 characters!
 *
 * The output method is the same as for other message types.
 */
void LogTrace(char *format, ...);

/*
 * log_adpname
 * ---------------
 * return a string e.g. "ADP_Info" when given an ADP Reason code
 */
char *log_adpname(int reason);

/*
 * log_swiname
 * ---------------
 * return a string e.g. "SYS_FLEN" when given an Angel SWI function code
 */
char *log_swiname(int reason);

/*
 * Log_logmsginfo
 * ---------------
 * Allows supplementary info to be passed to the logging functions; the
 * macros don't allow them to be passed 'normally'.
 */
void Log_logmsginfo(char *file, int line, log_id id);

void log_dump_buffer(WarnLevel lvl, log_id mod, unsigned int bpl, char *buffer, unsigned int length);

/*---------------------------------------------------------------------------*/

#define  STRINGIFY2(x) #x
#define  STRINGIFY(x)  STRINGIFY2(x)


/*
 * Defining DEBUG_FILE_TOO allows the filename to be included in messages,
 * but at a cost in memory usage. Replace __FILE__ with an array ref, define
 * a macro (e.g. <<#define FILEINFO  static char __filename[] = __FILE__ >>
 * and insert that macro at the top of every .c file if this becomes a
 * problem.
 *
 * Note: many compilers, including armcc, will collapse multiple references
 * to the same string, but not all do, and not always.
 */
#ifdef DEBUG_FILE_TOO
#define log_file   __FILE__
#else
#define log_file   NULL
#endif

/*
 * The next section defines macros intended for use in the rest of Angel.
 * The action performed depends on the compile options as well as runtime
 * options, and for some debug methods (e.g. logterm) can be controlled
 * interactively.
 *
 * Use of the maros is consistent:
 *    <macroname>(<module>, (<printf-type-args>));
 *
 * where:
 *   macroname         is one of the log_xxx macros below
 *   module            is exactly one of the LOG_xxx flags above
 *   printf-type-args  is a list of args, printf style, starting
 *                     with a format string.
 *
 * There are four main macros:
 *   LogFatalError   for occasions when Angel cannot possibly
 *                     proceed further; the world has died.
 *                     Release builds will reset Angel at this point.
 *
 *   LogError        Signals a real error; something has gone
 *                     wrong, but at least for the moment Angel can
 *                     attempt to continue.
 *
 *   LogWarning      Signals a problem; something unexpected has
 *                     happened, but it can be handled. This may,
 *                     for example, indicate a dropped packet, or
 *                     an interrupt for an unhandled source.
 *
 *   LogInfo         For other, probably general debug messages.
 *
 *
 * There is also another macro:
 *   LogTrace        This is called with a single arg; it is
 *                     intended for use on occasions when an absolute
 *                     minimum time must be spent; e.g. during the
 *                     angel_Yield function. Output is not stored.
 */

/* logterm does the checks internally; other log methods do it inline */
#ifdef LOGTERM_DEBUGGING
#define log_cond(id) (1)
#else
#define log_cond(id) (log_logging_flags[(int)id])
#endif

#ifdef TARGET
#include "logtarget.h"
#else
#include "loghost.h"
#endif

#if defined(DEBUG) && (DEBUG == 1) && defined(DO_TRACE)
#   define TRACE(w)         LogTrace("%s ", w)
#   define LogTrace1(s,d)   LogTrace(s,d)
#   define LogTrace2(s,d,e) LogTrace(s,d,e)
#else
#   define TRACE(w)
#   define LogTrace(s)
#   define LogTrace1(s,d)
#   define LogTrace2(s,d,e)
#endif

/*---------------------------------------------------------------------------*/

#endif /* ndef angel_logging_h */

/* EOF logging.h */
