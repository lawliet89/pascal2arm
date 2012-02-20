/* fmod.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1997/01/07 15:42:43 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"


double fmod(double x,double y)
{
  /* floating point remainder of (x/y) for integral quotient. Remainder    */
  /* has same sign as x.                                                   */
  dp_number fx,fy,fr;
  double q;

  fx.d=x; fy.d=y;

  if (!dp_uncommon(fx) && !dp_uncommon(fy) && !dp_iszero(fy)) {
    if (dp_iszero(fx)) return x;

    fp_abs(fy); y=fy.d;

    fr.d=modf(fx.d/y,&q);
    fr.d=fx.d-q*y;

    /* The next few lines are an ultra-cautious scheme to ensure that the    */
    /* result is less than fabs(y) in value and that it has the sign of x.   */
    if (!fp_sign(fx)) {
      while (fr.d>=y) fr.d-=y;
      while (fp_sign(fr) && !dp_iszero(fr)) fr.d+=y;
    } else {
      y=-y;
      while (fr.d<=y) fr.d-=y;
      while (!fp_sign(fr) && !dp_iszero(fr)) fr.d+=y;
    }
    return fr.d;
  }
  
  return __fp_edom(sign_bit, TRUE);
}
