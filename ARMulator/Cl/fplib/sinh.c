/* sinh.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.7 $
 * Checkin $Date: 1997/01/07 15:42:54 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"

static const ip_number sinhp_poly[] = {
  0x80003ffe, 0xca273dc3, 0x7e40d68d, /* p3 = -0.78966127417357099479e0 */
  0x80004006, 0xa3c20b1b, 0x2df25346, /* p2 = -0.16375798202630751372e3 */
  0x8000400c, 0xb4ae15b4, 0xa0a47dd4, /* p1 = -0.11563521196851768270e5 */
  0x80004011, 0xabc89ab2, 0x99a0c6a6  /* p0 = -0.35181283430177117881e6 */
  };

static const ip_number sinhq_poly[] = {
  0x80004007, 0x8ade1c0e, 0x4bff9025, /* q2 = -0.27773523119650701667e3 */
  0x0000400e, 0x8d42b91d, 0xb2f63794, /* q1 =  0.36162723109421836460e5 */
  0x80004014, 0x80d67405, 0xf33895e0  /* q0 = -0.21108770058106271242e7 */
  };

#include "constant.h"

double sinh(double x)
{
  int sign;
  ip_number ix,r;

  ix=_d2e(x);
  if (fp_uncommon(ix)) goto dom_error;
  sign=fp_sign(ix);
  fp_abs(ix);

  if (fp_grpow(ix,0)) {
    /* _sinh_lnv is REQUIRED to read in as a number with the lower part of   */
    /* its floating point representation zero.                               */
    ip_number w,z,z2;

    w=_esub(ix,&sinh_lnv);
    z=_exp(w);
    if (fp_error(z)) goto range_error;
    if (!fp_grpow(z,32)) {
      ip_number t;
      z2=z;
      t=_erdv(z,&sinh_vm2); fp_negate(t); z=_eadd(t,&z2);
    }
    z2=z;
    r=_eadd(_emul(z,&sinh_v2m1),&z2);
    fp_abs(r); r.word.hi|=sign;
  } else if (!fp_grpow(ix,-32)) return x;
  else {
    ip_number g,t,ix2;
    ix2=ix;
    g=_esquare(ix);
    /* Use a (minimax) rational approximation. See Cody & Waite.     */
    t=__fp_poly1(g,4,sinhp_poly);
    r=_erdv(__fp_poly0(g,3,sinhq_poly),&t);
            
    ix2.word.hi|=sign;           /* put sign back */
    r=__fp_spike(r,&ix2);        /* r=r*ix+ix */
  }
  if (fp_error(r)) goto range_error;
  return _e2d(r);

 dom_error:
  return __fp_edom(sign_bit, TRUE);
 range_error:
  return __fp_erange(ix.word.hi, TRUE);
}
