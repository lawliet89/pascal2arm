/* exp.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.9 $
 * Checkin $Date: 1997/01/13 11:17:33 $
 * Revising $Author: mwilliam $
 */

#include <stdarg.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include <math.h>

#include "mathlib.h"
#include "constant.h"

#ifdef _exp_c

static const ip_number exp_range[] = {
  0x00003fff, 0xb8aa3b29, 0x5c17f0bc, /* 1/ln(2) */
  0x00003ffe, 0xb1800000, 0x00000000, /* M1 */
  0x80003ff2, 0xde8082e3, 0x08654362  /* M2 (ln(2)-M1) */
  };

static const ip_number exp_P[] = {
  0x00003ff1, 0x845a2157, 0x3490f106, /* 2*p2 (p1=0.31555192765684646356e-4) */
  0x00003ff8, 0xf83a5f91, 0x50952c99, /* 2*p1 (p2=0.75753180159422776666e-2) */
  0x00003ffe, 0x80000000, 0x00000000  /* 2*p0 (p0=0.25000000000000000000e+0) */
  };

static const ip_number exp_Q[] = {
  0x00003feb, 0xc99b1867, 0x3490f106, /* 2*q3 (q3=0.75104028399870046114e-6) */
  0x00003ff5, 0xa57862e1, 0x46a6fb39, /* 2*q2 (q2=0.63121894374398503557e-3) */
  0x00003ffb, 0xe8b9428e, 0xfecff592, /* 2*q1 (q1=0.56817302698551221787e-1) */
  0x00003fff, 0x80000000, 0x00000000  /* 2*q0 (q0=0.50000000000000000000e-0) */
  };

/* static const ip_number error_word =  { error_bit, 0x0, 0x0 }; */

#ifdef EMBEDDED_CLIB
extern ___weak volatile int *__rt_errno_addr(void);
#endif

__value_in_regs ip_number _exp(ip_number ix)
{
  if (fp_geqpow(ix,-64)) {
    range_red g,g2;
    ip_number z,gpz,qz,r;

    g=__fp_range_red_by_mod(ix,exp_range);
    if (fp_error(g.x)) goto error;
    g2=g;
    z=_esquare(g.x);
    
    gpz=_emul(__fp_poly(z,3,exp_P),&g2.x);
    qz=__fp_poly(z,4,exp_Q);
    
    r=_eadd(_erdv(_esub(qz,&gpz),&gpz),&half);
    
    r.word.hi+=g.n+1;
    return r;
  } else {
    ip_number r;

    r.word.hi=0x3fff; r.word.lo=units_bit; r.word.lo2=0;
    return r;
  }

 error:
{
  ip_number r;
#ifdef EMBEDDED_CLIB
  if (!fp_sign(ix)) {
    if (__rt_errno_addr)
        *__rt_errno_addr() = ERANGE;
  }
#else
  if (!fp_sign(ix)) errno=ERANGE;
#endif
  r.word.hi=(fp_sign(ix)) ? 0 : error_bit;
  r.word.lo=r.word.lo2=0;

  return r;
}
}

#else
/* declared in mathlib.h */
#endif

#ifdef exp_c

double exp(double x)
{
  ip_number ix;

  ix=_d2e(x);

  if (!fp_uncommon(ix)) {
    ix=_exp(ix);
    if (!fp_error(ix)) return _e2d(ix);
    
  {
    dp_number f;
    /* return fp_sign(ix) ? -HUGE_VAL : HUGE_VAL; */
    f.d=HUGE_VAL;
    f.word.hi|=fp_sign(ix);
    return f.d;
  }
  }

  if (fp_infinity(ix)) return __fp_erange(ix.word.hi, TRUE);
  return __fp_edom(ix.word.hi, TRUE);
}

#endif
