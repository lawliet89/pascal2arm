/* signal.c: ANSI draft (X3J11 Oct 86) library code, section 4.7 */
/* Copyright (C) Codemist Ltd, 1988                              */
/* Copyright (C) Advanced Risc Machines Ltd., 1991               */
/* version 0.01d */

/* N.B. machine dependent messages (only) below. */

#include "hostsys.h"
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "interns.h"

#define SIGLAST 11  /* one after highest signal number (see <signal.h>) */

static void (*_signalvector[SIGLAST+1])(int);

/* HIDDEN EXPORTS - for compatibility with previous shared C libraries */
extern void __ignore_signal_handler(int sig);
extern void __error_signal_marker(int sig);
extern void __default_signal_handler(int sig);

extern void __ignore_signal_handler(int sig)
{
    /* do this in case called because of SharedCLibrary botch... */
    signal(sig, SIG_IGN);
}

static void _real_default_signal_handler(int sig)
{   const char *s;
    switch (sig)
    {
case SIGABRT:   s = "Abnormal termination (e.g. abort() function)";
                break;
case SIGILL:    s = "Illegal instruction (call to non-function/code corrupted)"
                    "\n[is the floating point emulator installed?]";
                break;
case SIGINT:    s = "Interrupt received from user - program terminated";
                break;
case SIGSEGV:   s = "Illegal address (e.g. wildly outside array bounds)";
                break;
case SIGTERM:   s = "Termination request received";
                break;
case SIGUSR1:
case SIGUSR2:   s = "User-defined signal";
                break;
default:        /* Note: including SIGFP */
                s = _hostos_signal_string(sig);
                break;
    }
    _sys_msg(s, NL_PRE+NL_POST);        /* ensure out even if stderr problem */
    if (sig == SIGINT)
        __rt_exit(1);
    else
        _postmortem();
}

#pragma -s1

static void _default_sigstak_handler()
{
    _init_flags.error_recursion = 1; /* lie to prevent attempt to use stdio */
    _sys_msg("Stack overflow\n", NL_PRE+NL_POST);
    __rt_exit(100);
}

extern void __default_signal_handler(int sig)
{
    if (sig==SIGSTAK)
      _default_sigstak_handler();
    else
      _real_default_signal_handler(sig);
}

extern void __error_signal_marker(int sig)
/* This function should NEVER be called - its value is used as a marker     */
/* return from signal (SIG_ERR).   If someone manages to use pass this      */
/* value back to signal and thence get it invoked we make it behave as      */
/* if signal got SIG_DFL:                                                   */
{
    __default_signal_handler(sig);
}

int raise(int sig)
{
    void (*handler)(int);
    if (sig<=0 || sig>=SIGLAST) return (errno = ESIGNUM);
    handler = _signalvector[sig];
    if (handler==__SIG_DFL)
        (*__default_signal_handler)(sig);
    else if (handler!=__SIG_IGN)
    {   _signalvector[sig] = __SIG_DFL;
        handler(sig);
    }
    return 0;
}

int _signal_real_handler(int sig)
{
    if (sig<=0 || sig>=SIGLAST) return 0;
    return (_signalvector[sig]!=__SIG_DFL);
}

#pragma -s0

void (*signal(int sig, void (*func)(int)))(int)
{
    void (*oldf)(int);
    if (sig<=0 || sig>=SIGLAST) return __SIG_ERR;
    oldf = _signalvector[sig];
    _signalvector[sig] = func;
    return oldf;
}

void _signal_init()
{
    int i;
    /* do the following initialisation explicitly so code restartable */
    for (i=1; i<SIGLAST; i++) signal(i, __SIG_DFL);
}

/* end of signal.c */
