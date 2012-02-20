/* tan.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.7 $
 * Checkin $Date: 1997/01/07 15:42:56 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"

static const ip_number tan_range[] = {
  0x00003FFE,0xA2F9836E,0x4E44152A, /* 2/pi to extended precision */
  0x00003FFF,0xC9100000,0x00000000, /* M1 = 1.57080078125 exactly */
  0x80003FED,0x95777A5C,0xF72CECE6  /* 2 = pi/2-M1 to ext. prec. */
  };

static const ip_number tanp_poly[] = {
  0x80003FEF,0x95D5B975,0x16391DA8, /* p3 = -0.17861707342254426711e-4 */
  0x00003FF6,0xE0741531,0xDD56F650, /* p2 =  0.34248878235890589960e-2 */
  0x80003FFC,0x8895AF2A,0x6847FCD5, /* p1 = -0.13338350006421960681e+0 */
  0x00003FFF,0x80000000,0x00000000  /* p0 =  1.0 */
  };

static const ip_number tanq_poly[] = {
  0x00003FEA,0x85BBA783,0xB3C748A9, /* q4 =  0.49819433993786512270e-6 */
  0x80003FF3,0xA37B24C8,0x4A42092E, /* q3 = -0.31181531907010027307e-3 */
  0x00003FF9,0xD23CF50B,0xF10ACA84, /* q2 =  0.25663832289440112864e-1 */
  0x80003FFD,0xEEF5823F,0xDECEA969, /* q1 = -0.46671683339755294240e+0 */
  0x00003FFF,0x80000000,0x00000000  /* q0 =  1.0 */
  };

double tan(double x)
{
  dp_number fx;
  range_red range;
  ip_number ix,f;
  int n;

  fx.d=x;
  if (dp_iszero(fx)) return fx.d; /* Exact case */
  
  ix=_d2e(fx.d);
  if (fp_uncommon(ix)) goto dom_error;
  
  range=__fp_range_red_by_mod(ix,tan_range);
  if (fp_error(range.x)) goto error;
  f=range.x; n=range.n;

  /* Spot a small number by examining the exponent. If f < 2^32 then its
     exponent will be < EIExp_bias - 32 */
  if (fp_geqpow(f,-32)) {
    ip_number g,g2,f2,xnum,xden;
    f2=f;
    g=_esquare(f2);
    xnum=_emul(__fp_poly(g2=g,4,tanp_poly),&f2);
    xden=__fp_poly(g2,5,tanq_poly);
    /* If odd, negate result by splatting xden */
    xden.word.hi^=(n<<31);
    return _e2d((n & 1) ? _ediv(xden,&xnum) : _erdv(xden,&xnum));
  } else if (n & 1) {
    ip_number one;    

    /* the argument is close to an odd multiple of pi/2.
     * Return -1/(range-reduced-number) */
    one.word.hi=0x80003fff; one.word.lo=units_bit; one.word.lo2=0;
    return _e2d(_erdv(f,&one));
  } else {
    /* the argument is close to an even multiple of pi/2
     * (i.e. a multiple of pi) - just return the range reduced argument. */
    return _e2d(f);
  }


 dom_error:
  /* An uncommon argument: return x */
  if (!fp_infinity(ix)) return __fp_edom(ix.word.hi, TRUE);
  /* Drop through to... */

 error:
  /* Input argument is out of range. ANSI only allows us to return some
   * random value. */
  fx.word.hi=fx.word.lo=0;
  return fx.d;
}
