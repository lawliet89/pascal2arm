/* -*-C-*-
 *
 *    CVS $Revision: 1.1.2.5 $
 * Revising $Author: rivimey $
 *            $Date: 1998/01/27 15:08:32 $
 *
 * Copyright (c) 1996-1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * logging.h - methods for logging warnings, errors and trace info
 */

#ifndef angel_logpriv_h
#define angel_logpriv_h

#if DEBUG == 1

/*
 * Some macros for debugging and warning messages
 */

struct LogSaveBuffer
{
    unsigned long *start;
    unsigned long *current;
    unsigned long *end;
    unsigned long size;
    unsigned long message;
};

#define WL_SAVEMSG      0x01
#define WL_PRINTMSG     0x02

/*---------------------------------------------------------------------------*/

extern unsigned long __rt_logging_mask;

extern void log_set_begin(void);
extern void log_set_log_id(log_id id, int onoff);
extern int  log_get_log_id(log_id id);
extern int  log_get_num_ids(void);

extern void log_getmsginfo(char **file, int *line, log_id *id);

extern int log_set_logging_options(int opts);
extern int log_get_logging_options(void);

extern char *log_get_format_string(void);
extern void log_set_format_string(char * str);

extern int log_logprint(long msgno, unsigned long *ptr);
extern int log_printf(char *format, ...);

extern void log_setupsave(struct LogSaveBuffer *sb, unsigned long *b, long sz);
extern void log_saveitem(struct LogSaveBuffer *sb, unsigned long item);

extern int __rt_bitnumber(unsigned long x);
extern log_id log_tracebit(char *name);
extern char *log_tracename(log_id bit);
extern int log_formatinfo(char *str, int lineno, char *file,
                          log_id id, WarnLevel level, long msgno);

extern void log_set_log_minlevel(WarnLevel level);
extern WarnLevel log_get_log_minlevel(void);

#endif /* DEBUG */

#endif /* ndef angel_logging_h */

/* EOF logging.h */
