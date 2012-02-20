/* sincos.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.10.6.1 $
 * Checkin $Date: 1997/08/13 13:26:11 $
 * Revising $Author: hmeeking $
 */

#include "mathlib.h"
#include "constant.h"

extern double _sincos(ip_number,int);

#ifdef _sincos_c

static const ip_number sincos_range[] = {
  0x00003FFD,0xA2F9836E,0x4E44152A, /* 1/pi to extended precision */
  0x00004000,0xC9100000,0x00000000, /* M1 = 3.1416015625 exactly */
  0x80003FEE,0x95777A5C,0xF72CECE6  /* M2 = pi-M1 to ext. prec. */
  };

static const ip_number sincos_poly[] = {
  0x00003FCE,0xC407FB4C,0x9EFCA5FE, /* r8 in Cody & Waite */
  0x80003FD6,0xD72106E0,0x424CDF56, /* r7 */
  0x00003FDE,0xB091E343,0x56A17FA8, /* r6 */
  0x80003FE5,0xD7322A5A,0xEE055B44, /* r5 */
  0x00003FEC,0xB8EF1D29,0x27831824, /* r4 */
  0x80003FF2,0xD00D00D0,0x09F0D114, /* r3 */
  0x00003FF8,0x88888888,0x88858061, /* r2 */
  0x80003FFC,0xAAAAAAAA,0xAAAAA603  /* r1 */
  };

double _sincos(ip_number x,int sign)
{
  ip_number f;
  range_red g;
  
  g=__fp_range_red_by_mod(x,sincos_range);
  f=g.x;
  if (fp_error(f)) goto error;
  /* Here I really mean:
   * if ((g.n & 1)!=0) sign^=sign_bit;
   * but I've replaced with following, which compiles to a
   * single ARM instruction */
  sign^=(g.n)<<31;

  if (fp_geqpow(f,-32)) {
    ip_number g,r,f2;
    g=_esquare(f2=f);
    r=__fp_poly1(g,8,sincos_poly);
    f=__fp_spike(r,&f2);
  }

  f.word.hi^=sign;

  return _e2d(f);

 error:
{
  dp_number fx;
  fx.word.hi=fx.word.lo=0;
  return fx.d;
}
}

#endif

#ifdef sin_c

double (sin)(double x)
{
  dp_number fx;
  fx.d=x;
  if (!dp_iszero(fx)) {
    double p;
    ip_number ix=_d2e(fx.d);
    int sign=fp_sign(ix);
    if (!fp_uncommon(ix)) {
      fp_abs(ix);                 /* sin(-x) = -sin(x) */
      p=_sincos(ix,sign);
      return p;
    }
    if (!fp_infinity(ix)) return __fp_edom(0, TRUE);
    fx.word.hi=fx.word.lo=0;    /* return 0.0 */
  } 
  return fx.d;
}

#endif

#ifdef cos_c

double (cos)(double x)
{
  dp_number fx;
  fx.d=x;
  if (!dp_iszero(fx)) {
    ip_number ix=_d2e(fx.d);
    if (!fp_uncommon(ix)) {
      fp_abs(ix);                 /* cos(-x) = cos(x) */
      ix=_eadd(ix,&pi_by_2);      /* cos(x) = sin(x+pi/2) */
      return _sincos(ix,0);       /* prospective sign is zero */
    }
    if (fp_infinity(ix)) {
      fx.word.hi=fx.word.lo=0;
      return fx.d;
    }
    return __fp_edom(0, TRUE);
  }
  fx.word.hi=0x3ff00000;
  return fx.d;
}

#endif
