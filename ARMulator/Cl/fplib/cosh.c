/* cosh.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.8 $
 * Checkin $Date: 1997/01/07 15:42:37 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"
#include "constant.h"

double cosh(double x)
{
  ip_number ix=_d2e(x);

  if (fp_uncommon(ix)) goto uncommon;
  fp_abs(ix);

  if (fp_grpow(ix,0)) {
    ip_number ix2;

    ix=_esub(ix,&sinh_lnv);
    ix=_exp(ix);
    if (fp_error(ix)) goto error;
    if (!fp_grpow(ix,32)) {
      ix2=ix;
      ix=_eadd(_erdv(ix,&sinh_vm2),&ix2);
    }
    ix2=ix;
    ix=_eadd(_emul(ix,&sinh_v2m1),&ix2);
  } else {
    ip_number ix2;
    ix2=_exp(ix);
    ix=_emul(_eadd(_erdv(ix2,&one),&ix2),&half);
  }
  if (fp_error(ix)) return __fp_erange(ix.word.hi, TRUE);

  return _e2d(ix);

 uncommon:
  if (!fp_infinity(ix)) return __fp_edom(ix.word.hi, TRUE);
  /* else drop through */

 error:
  return x;
}
