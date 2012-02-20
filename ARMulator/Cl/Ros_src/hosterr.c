/* riscos/hosterror.c                                          */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

#include <stddef.h>

#include "riscos.h"
#include "hostsys.h"

extern char *_hostos_error_string(int no, char *buf) {
    buf = buf; /* unused */
    if (no == -1) {
        __riscos_error *e = __riscos_last_oserror();
        return (e == NULL) ? "unspecified error" : e->errmess;
    } else {
        return "unknown error";
    }
}

