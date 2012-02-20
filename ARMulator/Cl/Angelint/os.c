/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1996/05/10 15:28:50 $
 * Revising $Author: amerritt $
 *
 * Purpose: ARMulator-hosted C-library Monitor.
 *
 * Copyright (C) Advanced RISC Machines Ltd., 1991.
 *
 * Conditions of use:
 *
 * The terms and conditions under which this software is supplied to you and
 * under which you may use it are described in your licence agreement with
 * Advanced RISC Machines.
 *
 *---------------------------------------------------------------------
 */

#include "hostsys.h"
#include "ioguts.h"

extern struct {
   int valid ;
   __rt_error error ;
   } __rt_errorBuffer ;

static char *ntox(char *p, int n) {
  if (n > 15) { p = ntox(p, n >> 4); n &= 15; }
  *p++ = n > 9 ? n+'0' : n+'a'-10;
  return p;
}

extern void _memcpy(void *, void *, int);
char *_hostos_signal_string(int number)
{static char mess[30] ;

 if (!__rt_errorBuffer.valid) {
    char *p;
    _memcpy(mess,"Unknown Signal (0x", 20);
    p = ntox(&mess[18], number);
    *p++ = ')'; *p = 0;
    return(mess) ;
    }
 else
    return(__rt_errorBuffer.error.errmess) ;
 }

