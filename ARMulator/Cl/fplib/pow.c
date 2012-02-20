/* pow.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.10 $
 * Checkin $Date: 1997/01/07 15:42:50 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"

#ifdef EMBEDDED_CLIB
extern ___weak volatile int *__rt_errno_addr(void);
#endif

double pow(double x, double y)
{
  ip_number ix,ix2,iy,iy2;
  int ny;
  unsigned int sign=0;          /* Prospective result sign */

  ix=_d2e(x); iy=_d2e(y);

  /* Check both arguments for problems */
  if ((ix.word.hi | iy.word.hi) & uncommon_bit) goto dom_error;
  
  ny=_efix(iy);

  if ((ix.word.lo & iy.word.lo) & units_bit) {
    /*
; F1 is an integer. We now have the following cases:
;
; * if F0 is positive and not equal to 1.0, and F1 is non-zero, the result
;   is calculated in the normal way;
;
; * if F0 is negative and not equal to -1.0, and F1 is non-zero, we
;   calculate (-F0)^F1 in the normal way, then negate if F1 is odd;
;
; In all cases, we have a sign for the prospective result which is positive
; if F1 is even or F0 is positive, negative if F1 is odd and F0 is negative.
; Generate this sign in the bottom bit of Rtmp.
*/
    if ((double)ny==y) {        /* Integer power */
      /* Pick out a few special cases */
      if (x==1.0 || ny==1) return x;
      if (-x==1.0) { if (!(ny & 1)) x=-x; return x; }
      if (ny==2) return x*x;    /* Special case these */

      if (fp_sign(ix)) {
        sign=ny<<31; /* sign=1<<31 iff ny is odd */
        fp_abs(ix);
      }

      if ((ny<0 ? -ny : ny) < (1<<16)) { /* abs(ny)<2^16 */
        ip_number acc;
        unsigned int bit=1<<16;

        /* Use a repeated multiplication/division for iy in [-2^16..2^16] */
        if (ny>0) {
          while (!(ny & bit)) bit>>=1;
          acc=ix; bit>>=1;
          while (bit) { /* Repeated multiplication */
            acc=_esquare(acc);
            if (ny & bit) acc=_emul(acc,&ix);
            bit>>=1;
          }
        } else {
          ny=-ny;
          while (!(ny & bit)) bit>>=1;
          acc.word.hi=0x3fff; acc.word.lo=units_bit; acc.word.lo2=0; /* load 1.0 */
          acc=_ediv(acc,&ix); bit>>=1;
          while (bit) {    /* Repeated division */
            acc=_esquare(acc);
            if (ny & bit) acc=_ediv(acc,&ix);
            bit>>=1;
          }
        }
        acc.word.hi^=sign;
        return _e2d(acc);
      }
    } else if (fp_sign(ix)) {
      /* -x^non-int-power is a domain error */
      goto dom_error;
    } else {
      /* A number to a regular power... implement using log and exp */
      ip_number ix2;
      ix2=_emul(_log(x),&iy);
      if (fp_error(ix2)) goto range_error;
      ix2=_exp(ix2);
      if (fp_error(ix2)) goto range_error;
      return _e2d(ix2);
    }
  } else {
    dp_number r;

    r.word.lo=0;
    /* One or t'other arg is a zero. If iy=0.0 the result is one */
    if (fp_units(iy)) {         /* iy is non-zero */
      /* ix MUST be zero. If y<0 EDOM otherwise 0.0 */
      if (fp_sign(iy)) goto dom_error; /* iy<0.0 */
      r.word.hi=0;
      return r.d;
    }
    if (!fp_units(ix)) {
#ifdef ZeroToTheZeroIsOne
#ifdef EMBEDDED_CLIB
    if (__rt_errno_addr)
        *__rt_errno_addr() = EDOM;
#else
      errno=EDOM; /* Set EDOM for 0.0^0.0 */
#endif
#else
      goto dom_error;
#endif
    }
    r.word.hi=0x3ff00000;
    return r.d;
  }

 dom_error: 
  return __fp_edom(sign, TRUE);
 range_error:
  return __fp_erange(sign, TRUE);
}
