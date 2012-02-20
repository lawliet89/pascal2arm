/* riscos/hostsignal.c                                         */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

#include <stddef.h>
#include <signal.h>
#include "riscos.h"
#include "hostsys.h"

char *_hostos_signal_string(int sig) {
    if (sig == SIGFPE || sig == SIGOSERROR) {
        __riscos_error *e = __riscos_last_oserror();
        if (e != NULL) return &(e->errmess[0]);
    }
    return "unknown signal";
}
