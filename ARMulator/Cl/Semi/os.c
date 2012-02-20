/*
; Purpose: ARMulator-hosted C-library Monitor.
;
; Copyright (C) Advanced RISC Machines Ltd., 1991.
;
; Conditions of use:
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; Advanced RISC Machines.
;
;----------------------------------------------------------------------------*/

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1995/06/09 13:20:07 $
 * Revising $Author: hmeekings $
 */

#include "hostsys.h"
#include "ioguts.h"

int _sys_istty(FILE *f)
{return __sys_istty(f->file) ;}

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

