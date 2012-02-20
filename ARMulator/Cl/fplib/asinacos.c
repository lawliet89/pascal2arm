/* asinacos.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.9 $
 * Checkin $Date: 1997/01/07 15:42:34 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"
#include "constant.h"           /* shared constants */

extern double _asnacs(double,int);

#ifdef _asnacs_c

static const ip_number asnacsp_poly[] = {
  0x80003FFE,0xB25DEDAF,0x30F3242C, /* p5 = -0.69674573447350646411e0 */
  0x00004002,0xA270BB27,0x61C93957, /* p4 =  0.10152522233806463645e2 */
  0x80004004,0x9EC1654D,0x36D4F820, /* p3 = -0.39688862997504877339e2 */
  0x00004004,0xE4D539B0,0x56A451AD, /* p2 =  0.57208227877891731407e2 */
  0x80004003,0xDAF2AD41,0xD05311C4  /* p1 = -0.27368494524164255994e2 */
  };

static const ip_number asnacsq_poly[] = {
  0x80004003,0xBE974377,0xCC30F9E6, /* q4 = -0.23823859153670238830e2 */
  0x00004006,0x96F3E4B2,0xC8E37CBC, /* q3 =  0.15095270841030604719e3 */
  0x80004007,0xBEEE77E2,0xB5423CF3, /* q2 = -0.38186303361750149284e3 */
  0x00004007,0xD0927880,0xF5C2170B, /* q1 =  0.41714430248260412556e3 */
  0x80004006,0xA43601F1,0x5C3E6196  /* q0 = -0.16421096714498560795e3 */
  };

double _asnacs(double x,int acs_flag)
{
  ip_number ix,g,r,t1;
  int sign,flags;

  ix=_d2e(x);

  if (!fp_uncommon(ix)) {
    sign=fp_sign(ix); fp_abs(ix);
    
    if (fp_geqpow(ix,-1)) {       /* ix>=0.5 */
      /* Filter out domain errors and 1.0 */
      if (fp_geqpow(ix,0)) goto dom_error;        /* ix<1.0 */
      
      /* ix is in the range [0.5,1.0] (acos(1.0) already catered for) */
      flags=acs_flag ? sign ? 0xe : 0x0 : sign ? 0xd : 0xc;
      
      /* Do the range reduction g := (1-Y)/2; Y := 2*SQT(g) as done by the
         FPE. */
      g=ix; fp_negate(g); g=_eadd(g,&one);
      g.word.hi--;              /* /2 - we can use exp arithmetic because */
      ix=_esqrt(g);
      ix.word.hi++;             /* *2 - we know ix is in the range [0.5,1.0] */
    } else {
      flags=acs_flag ? sign ? 0x4 : 0xc : sign ? 0x1 : 0x0;
      if (!fp_geqpow(ix,-32)) goto very_small;
      g=_esquare(ix);
    }
    
    t1=__fp_poly0(g,5,asnacsq_poly);
    r=_ediv(__fp_poly1(g,5,asnacsp_poly),&t1);
    
    ix=__fp_spike(r,&ix);         /* ix=r*ix+ix */

  very_small:
    ix.word.hi^=(flags & 0x8) << 28; /* negate if bit 3 of flags is set */
    if (flags & 0x4) {            /* If bit 2 is set... */
      /* ...add a constant */
      ip_number constant=pi_by_2;
      if (flags & 0x2) constant.word.hi++; /* Turn into pi if bit 1 set */
      ix=_eadd(constant,&ix);
    }
    ix.word.hi^=flags<<31;      /* Negate final answer if bit 0 set */

    return _e2d(ix);

  dom_error:
    /* Either a domain error or an argument of 1.0 */
    if (fp_eqpow(ix,0)) {
      dp_number res;
      
      /* This is an exact result of -pi/2, 0, pi/2 or pi depending... */
      
      res.d=_pi_2;
      if (acs_flag) {
        if (sign) res.word.hi+=1<<DExp_pos; /* change pi/2 to pi */
        else res.d=0.0;
      } else {
        res.word.hi|=sign;
      }
      return res.d;
    }
    /* Fall through to EDOM code */
  }

  /* asin/acos(Inf/QNaN/SNaN) => DOMAIN ERROR */
  return __fp_edom(sign_bit, TRUE);
}

#endif

#ifdef asin_c

double asin(double x)
{
  dp_number fx;
  fx.d=x;
  if (!dp_iszero(fx)) return _asnacs(x,0);
  else return x;                /* Exact case */
}

#endif

#ifdef acos_c

double acos(double x)
{
  return _asnacs(x,1);
}

#endif
