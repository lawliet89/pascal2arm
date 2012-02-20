/* interns.h */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */

/*
 * RCS $Revision: 1.8 $
 * Checkin $Date: 1996/11/05 01:04:54 $
 * Revising $Author: enevill $
 */

#pragma force_top_level
#pragma include_only_once

#ifndef __internals_h
#define __internals_h 1

/* internals.h                                                  */
/* Functions defined within the library, solely for the use of  */
/* other parts of the library (but not those defined and used   */
/* local to one package).                                       */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include "hostsys.h"   /* FILEHANDLE */

#ifdef SHARED_C_LIBRARY

#define CallIfPresent(f, arg) f arg

#else

#define CallIfPresent(f, arg) if (f) f arg

#endif

extern char *_strerror(int n, char *v);
 /* The same as strerror, except that if an error message must be constructed
 * this is done into the array v.
 */

extern const unsigned char _monlen[12];
/* for localtime() and mktime() */

extern void _set_ctype_8859(int yes);

___weak void _ctype_init(void);

extern void __set_strcoll_table(const unsigned char *table);

extern struct _init_flags {
    char error_recursion;
    char alloc_finalised, io_finalised;
} _init_flags;

extern int __backspace(FILE *stream);  /* a strict R-inverse of getc() */

___weak int _sys_msg_via_stderr(const char *s, int flag);

extern int __dup(int newf, int oldf);

extern int _vfscanf(FILE *p, const char *sfmt, va_list argv);

typedef void PROC(void);
extern void __rt_lib_init(void *topofstack, void *codebase, void *codelimit,
    PROC *cpp_init_final[2]);

extern void __rt_lib_shutdown(int callexitfns);
/* The argument is non-0 if functions registered by atexit() are to be called */

extern void _main(int (*main)(int, char **));

extern void _exit(int);
/* return to host os, calling atexit() handlers */

___weak void __rt_exit_init(void);

___weak void __rt_call_exit_fns(void);

extern void _sysdie(const char *s1, const char *s2);

#define NL_PRE 1
#define NL_POST 2
extern void _sys_msg(const char *s, int flag);

#include "rt.h"

___weak void _backtrace_init(void *topofstack, void *codebase, void *codelimit);

extern void _backtrace(int why, int *address, __rt_unwindblock *uwb);

___weak void _clock_init(void);

extern void *(*__rt_malloc)(size_t);
/* used by _sys_alloc to acquire heap for the library's own purposes.  Normally,
   this will be set to malloc early in a program's life (but not if it doesn't
   use malloc itself).
 */

extern void (*__rt_free)(void *);

/* cf _malloc */
extern int _interrupts_off;
extern void _raise_stacked_interrupts(void);
extern void _postmortem(void);
___weak void _init_alloc(void);
___weak void _initio(char *,char *,char *);
___weak void __cpp_initialise(void);
___weak void __cpp_finalise(void);
___weak void _terminateio(void);
___weak void _signal_init(void);
___weak void _locale_init(void);

extern int _signal_real_handler(int sig);

extern void *_sys_alloc(size_t n);
___weak void _init_user_alloc(void);
___weak void _terminate_user_alloc(void);

extern double _ldfp(void *x);
extern void _stfp(double d, void *p);

extern struct lconv _lc;
extern const char _lc_C[];

#ifdef EMBEDDED_CLIB
#define DecimalPoint '.'
#else
#define DecimalPoint (_lc.decimal_point[0])
#endif

#endif
