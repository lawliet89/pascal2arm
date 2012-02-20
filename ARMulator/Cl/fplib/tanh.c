/* tanh.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.8 $
 * Checkin $Date: 1997/01/07 15:42:57 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"

static const ip_number log3_by_2 = { 0x00003ffe, 0x8c9f53d5, 0x68184f62 };

static const ip_number tanhp_poly[] = {
  0x80003ffe, 0xf6e14677, 0xdd3bf408, /* p2 = -0.96437492777225469787e0 */
  0x80004005, 0xc673ad0d, 0xd2e66361, /* p1 = -0.99225929672236083313e2 */
  0x80004009, 0xc9ad2e4d, 0xf0f0c390  /* p0 = -0.16134119023996228053e4 */
  };

static const ip_number tanhq_poly[] = {
  0x00004005, 0xe17d4f0f, 0x5fbfd3ed, /* q2 =  0.11274474380534949335e3 */
  0x0000400a, 0x8b9c5a68, 0x0f8795f3, /* q1 =  0.22337720718962312926e4 */
  0x0000400b, 0x9741e2ba, 0x74b496a6  /* q0 =  0.48402357071988688686e4 */
  };

#include "constant.h"

double tanh(double x)
{
  /* The first two exits avoid premature overflow as well as needless use  */
  /* of the exp() function.                                                */
  int sign;
  dp_number fx;

  fx.d=x;

  sign=fp_sign(fx); fp_abs(fx);

  if (fx.d<=27.0) {
    ip_number ix=_d2e(fx.d);
    if (fp_uncommon(ix)) goto uncommon;

    if (_egr(ix,&log3_by_2)) {
      ix=_emul(ix,&two);
      ix=_exp(ix);
      if (fp_error(ix)) goto error;
      ix=_erdv(_eadd(ix,&one),&two); fp_negate(ix); ix=_eadd(ix,&one);
    } else {
      if (fp_grpow(ix,-32)) {
        ip_number g,t,r1,r2,ix2;
        ix2=ix;
        g=_esquare(ix2);
        r1=__fp_poly1(g,3,tanhp_poly);
        r2=__fp_poly0(g,3,tanhq_poly);
        t=_erdv(r2,&r1);
        ix=__fp_spike(t,&ix2);   /* ix=g*ix+ix */
      }
    }
    ix.word.hi|=sign;
    if (fp_error(ix)) goto error;
    return _e2d(ix);
  } else {
    dp_number result;
    result.word.hi=0x3ff00000 | sign;
    result.word.lo=0;
    return result.d;
  }

 error: return __fp_erange(sign, TRUE);
 uncommon: return __fp_edom(0, TRUE);
}
