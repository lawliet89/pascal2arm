/* log.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.10 $
 * Checkin $Date: 1997/01/07 15:42:47 $
 * Revising $Author: mwilliam $
 */

#include <stdarg.h>
#include <stdlib.h>

#include <limits.h>
#include <math.h>

#include "mathlib.h"
#include "constant.h"

#ifdef _log_c

static const ip_number log2[2]=    { 0x00003FFE, 0xB17217F7, 0xD1CF0000,
                                     0x00003FCD, 0xF35793C0, 0x00000000 };

static const ip_number loga_poly[] = {
  0x80003FFE,0xCA20AD9A,0xB5E946E9,   /* a2 = -0.78956112887491257267e0 */
  0x00004003,0x83125100,0xB57F6509,   /* a1 =  0.16383943563021534222e2 */
  0x80004005,0x803FF895,0x9DACD228    /* a0 = -0.64124943423745581147e2 */
  };

static const ip_number logb_poly[] = {
  0x80004004,0x8EAC025B,0x3E7076BB,   /* b2 = -0.35667977739034646171e2 */
  0x00004007,0x9C041FD0,0xA933EF60,   /* b1 =  0.31203222091924532844e3 */
  0x80004008,0xC05FF4E0,0x6C83BB96    /* b0 = -0.76949932108494879777e3 */
  };

__value_in_regs ip_number _log(double x)
{
  frexp_str fr;
  ip_number xn,znum,zden,z,w,r,r2,f,t1,t2;
  int n;

  fr=__fp_frexp(x);
  n=fr.x; f=_d2e(fr.f);

  if (fr.f>_sqrt_half) {
    t2.word.lo=units_bit; t2.word.lo2=0;
    t2.word.hi=0x80003fff;
    znum=_eadd(t2,&f);
    zden=__fp_spike(f,&half);
  } else {
    t2.word.lo=units_bit; t2.word.lo2=0; /* this bit CSE'd with above */
    t2.word.hi=0x80003ffe;
    n--;
    znum=_eadd(t2,&f);
    zden=__fp_spike(znum,&half);
  }
#if 0
  if (fr.f>_sqrt_half) {
    n=fr.x; f=_d2e(fr.f);
    znum=_esub(f,&one);         /* FPE source says that IEEE is accurate enough */
    zden=__fp_spike(f,&half);   /* zden=f*0.5+0.5 */
  } else {
    n=fr.x-1; f=_d2e(fr.f);
    znum=_esub(f,&half);
    zden=__fp_spike(znum,&half); /* zden=znum*0.5+0.5 */
  }
#endif

  z=_erdv(zden,&znum);          /* zden is in registers... */
  w=_esquare(z);
  t1=__fp_poly1(w,3,loga_poly);
  r=_erdv(__fp_poly0(w,3,logb_poly),&t1);
  r=__fp_spike(r,&z);           /* r=r*z+z */

  if (n) {
    ip_number v1,t2;

    r2=r;
    xn=_eflt(n);

    v1=_emul(xn,&log2[0]);
    t1=_eadd(v1,&r2);
    t2=_esub(_esub(t1,&v1),&r2);

    r=_eadd(_eadd(_emul(xn,&log2[1]),&t2),&t1);
  }
    
  return r;
}

#else
/* Declared in mathlib.h */
#endif

#ifdef log_c

double log(double x)
{
  dp_number fx;
  fx.d=x;
  if (dp_uncommon(fx)) goto uncommon;
  if (!dp_iszero(fx)) {
    if (dp_ispos(fx)) return _e2d(_log(fx.d));
    return __fp_edom(fx.word.hi, TRUE); /* guaranteed -ve */
  }
  return __fp_erange(sign_bit, TRUE);

 uncommon: /* Inf/NaN/QNaN */
  if (dp_infinity(fx) && dp_ispos(fx)) return fx.d; /* log(+Inf)=+Inf */
  return __fp_edom(sign_bit, TRUE);
}

#endif

#ifdef log10_c

static const ip_number log10_e =   { 0x00003ffd, 0xde5bd8a9, 0x37287195 };

double log10(double x)
{
  dp_number fx;
  fx.d=x;
  if (dp_uncommon(fx)) goto uncommon;
  if (!dp_iszero(fx)) {
    if (dp_ispos(fx)) return _e2d(_emul(_log(fx.d),&log10_e));
    return __fp_edom(fx.word.hi, TRUE); /* guaranteed -ve */
  }
  return __fp_erange(sign_bit, TRUE);

 uncommon: /* Inf/NaN/QNaN */
  if (dp_infinity(fx) && dp_ispos(fx)) return fx.d; /* log10(+Inf)=+Inf */
  return __fp_edom(sign_bit, TRUE);
}

#endif
