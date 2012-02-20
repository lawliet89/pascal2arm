/* Copyright (C) Acorn Computers Ltd., 1988           */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */

/*
 * RCS $Revision: 1.11 $
 * Checkin $Date: 1995/04/20 17:08:34 $
 * Revising $Author: lsmith $
 */

#include <stddef.h>

#include "hostsys.h"
#include "interns.h"

#if defined _sysdie_c || defined SHARED_C_LIBRARY

void _sysdie(const char *s1, const char *s2)
{   _sys_msg("*** fatal error in run time system: ", NL_PRE);
    _sys_msg(s1, 0);
    _sys_msg(s2, NL_POST);
    _exit(1);
}

#endif

#if defined _sys_alloc_c || defined SHARED_C_LIBRARY

void *_sys_alloc(size_t n)
{ void *a = __rt_malloc(n);
  if (a == NULL)
    _sysdie("No store left for I/O buffer or the like", "");
  return a;
}

#endif

#if defined lib_init_c || defined SHARED_C_LIBRARY

struct _init_flags _init_flags;

static PROC *cpp_finalise;

void __rt_lib_init(void *topofstack, void *codebase, void *codelimit,
        PROC *cpp_init_final[2])
{   char *stdinfile  = TTYFILENAME,
         *stdoutfile = TTYFILENAME,
         *stderrfile = TTYFILENAME;
    PROC *cpp_initialise = cpp_init_final[0];
    cpp_finalise = cpp_init_final[1];
    /* Provide names for stdfile opens in _initio.  (Questionable host            */
    /* independence - wouldn't be the right thing for a UNIX-style os, where      */
    /* open stdfiles are inherited).                                              */

    _init_flags.error_recursion = 0;
    _init_flags.alloc_finalised = 0;
    _init_flags.io_finalised = 0;
#ifndef SHARED_C_LIBRARY
    /* IJR: these are RISCOS relics */
    CallIfPresent(_backtrace_init, (topofstack, codebase, codelimit));
    CallIfPresent(_getenv_init, ());
#endif
    CallIfPresent(_locale_init, ()); /* init locale data structures */
    CallIfPresent(_ctype_init, ());  /* init to C locale      */
    CallIfPresent(__rt_exit_init, ());/* must happen before exit() can be called   */
    CallIfPresent(_signal_init, ()); /* had better be done pretty early           */
    CallIfPresent(_clock_init, ());  /* set Cpu time zero point  */
    CallIfPresent(_init_alloc, ());  /* as had the allocator     */
/* SIGINT events are not safe until about now.                           */
    _raise_stacked_interrupts();     /* enable SIGINT                  */
    CallIfPresent(_initio, (stdinfile, stdoutfile, stderrfile));

    if (cpp_initialise) cpp_initialise();
}

void __rt_lib_shutdown(int callexitfns) {
    /* ensure no recursion if finalisation fails */
    if (callexitfns)
        CallIfPresent(__rt_call_exit_fns, ());

    if (cpp_finalise) {cpp_finalise();  cpp_finalise = 0;}

    if (!_init_flags.alloc_finalised) {
        _init_flags.alloc_finalised = 1;
        CallIfPresent(_terminate_user_alloc, ());
    }
    if (!_init_flags.io_finalised) {
        _init_flags.io_finalised = 1;
        CallIfPresent(_terminateio, ());
    }
}

#endif

#if defined _main_c || defined SHARED_C_LIBRARY

/* Not particularly host-OS specific: the STDFILE_REDIRECTION stuff      */
/* isn't useful under Unix, but it isn't harmful either.                 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>                              /* isprint(), isspace()  */
/*#include <string.h>*/                             /* for strlen()          */
#include "ioguts.h"                         /* private flag bits, _iob   */
#include "externs.h"

static void __arg_error(const char *s1, const char *s2, const char *s3) {
    _sys_msg(s1, NL_PRE);
    _sys_msg(s2, 0);
    _sys_msg(s3, NL_POST);
    _exit(EXIT_FAILURE);
}

#ifdef INTERWORK
extern int __call_r2(int, char **, int (*main)(int, char**));
#endif

void _main(int (*main)(int, char **))
#define LINE_SIZE 256
#define BAD_REDIRECTION {goto bad_redirection;}
#define NO_REDIRECTION goto no_redirection
{   char ch;
    static char **argv;
    static char *args;
    int curarg = 0, in_quotes = 0, was_quoted = 0;
    int argc = 1, i = 0;
#ifdef STDFILE_REDIRECTION
    int after_file_name = 0, redirection = 0;
    int pre_digit = 0, dup_arg_1 = 0;
    char mode[2];
#endif
    char *s = __rt_command_string();

    while (s[i] != 0)
    {  while (isspace(s[i])) i++;
       while ((!isspace(s[i])) && s[i] != 0) i++;
       argc++;
    }
    argv = (char **)_sys_alloc(argc*sizeof(char *));
    args = (char *)_sys_alloc(++i);
    CallIfPresent(_init_user_alloc, ());

    i = 0; argc = 0;
    do
    {   ch = *s++;
        if (!in_quotes)
        {   if (ch == '"' || ch == '\'')
            {   was_quoted = in_quotes = ch;
                ch = *s++;
            }
#ifdef STDFILE_REDIRECTION
            else if ((i == curarg) && (!after_file_name))
            {   char *next = s - 1;
                pre_digit = -1; dup_arg_1 = -1;
                mode[0] = 0; mode[1] = 0;
                if (*next >= '0' && *next <= '9') pre_digit = *next++ - '0';
                if ((*next == '>') || (*next == '<'))
                {   if (*next == '>') /* stdout or stderr */
                    {   mode[0] = 'w';
                        if (pre_digit == 0 || pre_digit > 2) BAD_REDIRECTION
                        else if (*++next == '>') { mode[0] = 'a'; next++; }
                    } else
                    {   char *p;
                        next++;
                        for (p = next; (*p != 0) && (*p != ' '); p++)
                            if (*p == '>') NO_REDIRECTION;
                        if (pre_digit > 0) BAD_REDIRECTION
                        mode[0] = 'r';
                    }
                    if (*next == '&')
                    {   if (pre_digit != -1) /* was a preceeding digit */
                        {   if ((pre_digit > 0) &&
                                ((*++next >= '0') && (*next <= '2')) &&
                                (*next++ == (pre_digit % 2 + 1) + '0'))
                            {   /* 2>&1 or 1>&2 */
                                mode[0] = 0; /* no fopen required */
                                dup_arg_1 = pre_digit;
                            } else BAD_REDIRECTION
                        } else /* no preceeding digit */
                        {   next++;
                            dup_arg_1 = 2;
                            pre_digit = (mode[0] != 'r'); /* default = 0 or 1 */
                        }
                    }
                    else if (pre_digit == -1)
                        pre_digit = (mode[0] != 'r'); /* default = 0 or 1 */

                    if (mode[0] != 0)
                    {   after_file_name = 1;
                        while (isspace(*next)) next++;
                        if (*next == '"' || *next == '\'') in_quotes = *next++;
                    }
                    else if ((*next != 0) && (!isspace(*next)))
                        BAD_REDIRECTION
                    redirection = 1;
                    s = next; ch = *s++;
                }
            }
#endif /*STDFILE_REDIRECTION*/
        }
        if (in_quotes)
        {   if (ch == '\\' && (*s == '"' || *s == '\\' || *s == '\''))
                ch = *s++;
            else {
                int q = in_quotes;
                while (ch == q) { in_quotes ^= ch; ch = *s++; }
            }
        }

no_redirection:
        if (ch != 0 && (in_quotes || !isspace(ch)))
        {   args[i++] = ch;
            continue;
        }
        /* Assert: ((ch == 0) || (isspace(ch) && !in_quotes)) */
        /* ------- possible end of arg ---------------------- */
        if (i != curarg || was_quoted
#ifdef STDFILE_REDIRECTION
            || (redirection && !after_file_name)
#endif
           )
        {   /* end of arg */
            args[i++] = 0;
#ifdef STDFILE_REDIRECTION
            if (redirection)
            {   if (after_file_name &&
                    freopen(&args[curarg], mode, &_iob[pre_digit]) == 0)
                {   __arg_error("can't open '", &args[curarg],
                                "' for I/O redirection\n");
                }
                if (dup_arg_1 > -1 &&
                    !_sys_istty(&_iob[__dup(dup_arg_1, dup_arg_1 % 2 + 1)]))
                {   /* data to go to file */
                    FILE *s_new = &_iob[dup_arg_1];
                    FILE *s_old = &_iob[dup_arg_1 % 2 + 1];
                    setvbuf(s_new, (char *)_sys_alloc(LINE_SIZE), _IOLBF, LINE_SIZE);
                    s_new->flag |= _IOSBF;
                    setvbuf(s_old, (char *)_sys_alloc(LINE_SIZE), _IOLBF, LINE_SIZE);
                    s_old->flag |= _IOSBF;
                }
                redirection = 0; after_file_name = 0; i = curarg;
            }
            else
#endif /*STDFILE_REDIRECTION*/
            {   argv[argc++] = &args[curarg];
                curarg = i;
            }
        }
        if (ch != 0) { in_quotes = was_quoted = 0; }
    }
    while (ch != 0);

    if (in_quotes)
    {   char a[2]; a[0] = in_quotes; a[1] = 0;
        __arg_error("missing closing ", a, "");
    }

    argv[argc] = 0;      /* for ANSI spec */
    if (argc > 0 && (*(int *)argv[0] & ~0x20202020) == *(int *)"RUN") argc--, argv++;
    /* hmm, relies on lots of things, but fast! */
#ifdef INTERWORK
    _exit(__call_r2(argc, argv, main));
#else
    _exit(main(argc, argv));
#endif

#ifdef STDFILE_REDIRECTION
bad_redirection:
    __arg_error("unsupported or illegal I/O redirection '", --s, "'\n");
#endif
}

#endif

/* end of armsys.c */
