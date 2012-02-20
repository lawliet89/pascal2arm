/* atan.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.16.6.1 $
 * Checkin $Date: 1997/08/13 13:26:10 $
 * Revising $Author: hmeeking $
 */

#include "mathlib.h"
#include "constant.h"

extern double _atnpol(ip_number,int);

#ifdef _atnpol_c

static const ip_number two_minus_sqrt3 = { 0x00003FFD,0x8930A2F4,0xF66AB18A };
static const ip_number sqrt3_minus_one = { 0x00003ffe,0xbb67ae85,0x84caa73c };
static const ip_number sqrt3           = { 0x00003fff,0xddb3d742,0xc265539e };

static const ip_number tan_poly_p[] = {
  0x80003FFE,0xD66BD6CD,0x8C3DE934, /* p3 = -0.83758299368150059274e0 */
  0x80004002,0x87E9FAE4,0x6B531A29, /* p2 = -0.84946240351320683534e1 */
  0x80004003,0xA40BFDCF,0x15E65691, /* p1 = -0.20505855195861651981e2 */
  0x80004002,0xDB053288,0x30E70EB4  /* p0 = -0.13688768894191926929e2 */
  };

static const ip_number tan_poly_q[] = {
  0x00004002,0xF0624F0A,0x56388310, /* q3 = 0.15024001160028576121e2 */
  0x00004004,0xEE505190,0x6D1EB4E8, /* q2 = 0.59578436142597344465e2 */
  0x00004005,0xAC509020,0x5B6D243B, /* q1 = 0.86157349597130242515e2 */
  0x00004004,0xA443E5E6,0x24AD4B90  /* q0 = 0.41066306682575781263e2 */
  }; 

static const ip_number tan_table[] = {
  0x00003FFE,0x860A91C1,0x6B9B2C23, /* pi/6 */
  0x00003FFF,0x860A91C1,0x6B9B2C23, /* pi/3 */
  0x00003FFF,0xC90FDAA2,0x2168C234, /* pi/2 */
  0x00003FFF,0xC90FDAA2,0x2168C234, /* pi/2 */
  0x00004000,0x860A91C1,0x6B9B2C23, /* 2pi/3 */
  0x00004000,0xA78D3631,0xC681F72B, /* 5pi/6 */
  0x00004000,0xC90FDAA2,0x2168C234  /* pi */
  }; 


double _atnpol(ip_number x,int sign)
{
  ip_number x2;

  /* Combined atan and atan2 (pol) code. */
  /*  The sign information is put into a word which will eventually contain
      significant information in bits 31 and 2:0, as follows:
      Bit 31 is 1 if the result must eventually be negated, after all other
        transformations have been done to it;
      Bits 2:0 indicate what change must be done to the raw result, before the
        bit 31 negation:
          Bits 2:0 = 000: no change;
          Bits 2:0 = 001: result <- pi/6 + result;
          Bits 2:0 = 010: result <- pi/3 - result;
          Bits 2:0 = 011: result <- pi/2 - result;
          Bits 2:0 = 100: result <- pi/2 + result;
          Bits 2:0 = 101: result <- 2pi/3 + result;
          Bits 2:0 = 110: result <- 5pi/6 - result;
          Bits 2:0 = 111: result <- pi - result;          */

  /* if (x>1.0) { x=1.0/x; sign^=3; } */
  /* x>=1.0 is signified by having an exponent >= EIExp_bias */

  ip_number r;

  /*printf("atnpol(%f,%08x) ",_e2d(x),sign);*/

  if (fp_grpow(x,0)) {
    x=_erdv(x,&one);
    sign^=3;
  }

  if (_egeq(x2=x,&two_minus_sqrt3)) {
    ip_number t;
    t=_eadd(x2,&sqrt3);
    /* ((A*x-1.0)+x)/(B+f) where A=sqrt(3)-1 and B=sqrt(3) */
    x2=_ediv(_eadd(_esub(_emul(x2,&sqrt3_minus_one),&one),&x2),&t);

    sign^=1;
  }

  if (fp_geqpow(x2,-32)) {
    ip_number g,r1,r2;
    g=_esquare(x2);
    r1=__fp_poly1(g,4,tan_poly_p);
    r2=__fp_poly0(g,4,tan_poly_q);
    r2=_erdv(r2,&r1);
    r=__fp_spike(r2,&x2);       /* r=r2*x+x */
  } else {
    r=x2;
  }

  /*printf("%f (PI*%f) %08x\n",_e2d(r),_e2d(_ediv(r,&tan_table[6])),sign);*/

  if (sign & (1<<1)) fp_negate(r);
  
  /* Add a multiple of pi/6 */
  if (sign & 7) r=_eadd(r,&tan_table[(sign & 7)-1]);

  r.word.hi^=(sign & sign_bit);

  return _e2d(r);
}

#endif

#ifdef atan2_c

double atan2(double y,double x)
{
  dp_number fx,fy;
  fx.d=x;
  fy.d=y;

  if (!dp_iszero(fx)) {
    ip_number ix,ix2,iy,ixy;
    unsigned sign_flags=0;

    ix=_d2e(x);
    if (fp_uncommon(ix)) goto ix_uncommon;
    /* I really want to do:
       sign_flags=(fp_sign(ix)) ? 7 : 0;
       Instead we leave 7<<29 or 0<<29 (i.e. 0) and do the rest later. */
    sign_flags=((signed)(fp_sign(ix)))>>2;
    fp_abs(ix);
    ix2=ix;

    iy=_d2e(fy.d);
    if (fp_uncommon(iy)) goto iy_uncommon;
    sign_flags=fp_sign(iy) | (sign_flags>>29);
    fp_abs(iy);

    ixy=_ediv(iy,&ix2);
    return _atnpol(ixy,sign_flags);

    /* Strictly speaking atan2(Inf,?) etc. have results. */
  ix_uncommon:
    if (fp_infinity(ix)) {
      dp_number f;

      /* If y is uncommon, unless infinite, we cannot return a result */
      if (dp_uncommon(fy)) {
        if (dp_infinity(fy)) {  /* both numbers are infinities */
          /* There is no real answer for this. However we cannot return
           * a domain or range error, so we have to fudge a return value.
           * Reluctantly, we return pi/4 in the relevant quadrant */
          if (fp_sign(ix)) {
            f.word.hi=0x4002D97C; /* This is 3*pi/4 */
            f.word.lo=0x7F3321D2;
          } else {
            f.word.hi=0x3FE921FB; /* This is pi/4. */
            f.word.lo=0x54442D18;
          }
        } else
          goto dom_error;
      } else {
        /* x is an infinity, y is not. The result is:
         *     fp_sign(ix) ? fp_sign(iy) ? -_pi_ : pi
         *                 : fp_sign(iy) ? -0.0 : 0.0
         */
        if (fp_sign(ix)) {
          f.word.hi=0x400921FB; /* This is pi. */
          f.word.lo=0x54442D18;
        } else {
          f.word.hi=f.word.lo=0;
        }
      }
      /* all code paths take the sign from fy */
      f.word.hi|=fp_sign(fy);
      return f.d;
    }
    goto dom_error;

  iy_uncommon:
    /* y is uncommon, and x is not */
    if (fp_infinity(iy)) {

      /* y is an infinity, x is not, so the result is pi/2 with the sign
       * of y - this is the same as the code below for x 0, y not. */
      goto return_pi_2;
    }
    goto dom_error;

  } else {
    if (dp_iszero(fy) ||
        (dp_uncommon(fy) && !dp_infinity(fy))) goto dom_error;
    /* x is zero, y is not. return pi/2 with sign of y */
  return_pi_2:
    fy.word.hi=0x3FF921FB | fp_sign(fy);
    fy.word.lo=0x54442D18;     /* load +/-_pi_2 */
    return fy.d;
  }

 dom_error:
  return __fp_edom(sign_bit, TRUE);
}

#endif

#ifdef atan_c

double (atan)(double x)
{
  int sign;
  ip_number ix;

  ix=_d2e(x);
  if (!fp_uncommon(ix)) {
    sign=fp_sign(ix);
    fp_abs(ix);
    return _atnpol(ix,sign);
  }

  if (fp_infinity(ix)) {
    dp_number f;
    /* return fp_sign(ix) ? -_pi_2 : _pi_2; */
    f.word.hi=0x3FF921FB | fp_sign(ix);
    f.word.lo=0x54442D18;       /* load +/-_pi_2 */
    return f.d;
  }
  return __fp_edom(ix.word.hi, TRUE);
}

#endif
