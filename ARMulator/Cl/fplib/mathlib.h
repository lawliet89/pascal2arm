/* mathlib.h */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.9 $
 * Checkin $Date: 1997/01/07 15:42:48 $
 * Revising $Author: mwilliam $
 */

/* mathlib.h -- types/functions used/defined by the low-level floating
   point code. */

#ifndef _mathlib_h
#define _mathlib_h

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include <math.h>

typedef union {
  struct { unsigned int hi,lo; } word; /* unsigned int word[2]; */
  double d;
} dp_number;

/* Some floating point constants */
#define _pi_       3.14159265358979323846
#define _pi_3      1.04719755119659774615
#define _pi_2      1.57079632679489661923
#define _pi_4      0.78539816339744830962
#define _pi_6      0.52359877559829887038
#define _sqrt_half 0.70710678118654752440

typedef struct {
  struct { unsigned int hi,lo,lo2; } word; /* unsigned int word[3]; */
} ip_number;

extern __value_in_regs ip_number _d2e(double);
extern __value_in_regs ip_number _f2e(float);
extern  double _e2d(ip_number);
extern float __fp_e2f(ip_number,unsigned);
#define _e2f(ip) __fp_e2f((ip),(ip).word.hi)
extern float _d2f(double);
extern double _f2d(float);

extern __value_in_regs ip_number _eflt(int);
extern __value_in_regs ip_number _efltu(unsigned);
extern __value_in_regs int _efix(ip_number);
extern __value_in_regs unsigned int _efixu(ip_number);
extern __value_in_regs ip_number _eadd(ip_number,const ip_number *);
extern __value_in_regs ip_number _esub(ip_number,const ip_number *);
extern __value_in_regs ip_number _emul(ip_number,const ip_number *);
extern __value_in_regs ip_number _esquare(ip_number);
extern __value_in_regs ip_number _ediv(ip_number,const ip_number *);
extern __value_in_regs ip_number _erdv(ip_number,const ip_number *);
extern __value_in_regs ip_number _esqrt(ip_number);
extern __value_in_regs int _eeq(ip_number,const ip_number *);
extern __value_in_regs int _eneq(ip_number,const ip_number *);
extern __value_in_regs int _els(ip_number,const ip_number *);
extern __value_in_regs int _eleq(ip_number,const ip_number *);
extern __value_in_regs int _egr(ip_number,const ip_number *);
extern __value_in_regs int _egeq(ip_number,const ip_number *);

/* Set errno=???? and return a (signed) HUGE_VAL/zero */
extern double __fp_edom(unsigned long sign, unsigned huge_val);
extern double __fp_erange(unsigned long sign, unsigned huge_val);
#ifndef TRUE
#  define TRUE 1
#  define FALSE 0
#endif

/* These basic functions should probably be reimplemented in assembler */

#define sign_bit      0x80000000 /* In word.hi */
#define units_bit     0x80000000 /* In word.lo */
#define uncommon_bit  0x40000000
#define error_bit     0x20000000
#define fp_negate(var) ((var).word.hi ^= sign_bit)
#define fp_abs(var)    ((var).word.hi &= ~sign_bit)
#define fp_exponent(var)  ((var).word.hi & ~(sign_bit|uncommon_bit|error_bit))
#define fp_error(var) ((var).word.hi & error_bit)
#define fp_uncommon(var) ((var).word.hi & uncommon_bit)
#define fp_sign(var) ((var).word.hi & sign_bit)
#define fp_units(var) ((var).word.lo & units_bit)
#define fp_infinity(var) (((var.word.lo<<1) | var.word.lo2)==0) /* Assumes uncommon */

#define assign_abs(v1,v2) (((v1).word.hi=(v2).word.hi&~sign_bit),   \
                           ((v1).word.lo=(v2).word.lo),             \
                           ((v1).word.lo2=(v2).word.lo2))

#define EIExp_bias 0x3fff       /* Bias of exponent in internal precision */
#define DExp_bias 0x3ff         /* Bias of exponent in double precision */
#define DExp_pos 20             /* Position of exponent in a double */

/* Macro to determine whether an ip_number is greater than a given power of 
   two. Works by checking the exponent (assumes the number is normalised). */
#define fp_grpow(var,pow) \
  (/*!fp_uncommon(var) && */ (fp_exponent(var)>EIExp_bias+(pow) ||              \
                         (fp_exponent(var)==EIExp_bias+(pow) &&            \
                          ((var).word.lo!=units_bit || (var).word.lo2!=0))))
/* Macro to determine whether an ip_number is greater than or equal to a given
   power of two. Works by checking the exponent (assumes the number is
   normalised). */
#define fp_geqpow(var,pow) \
  (/*!fp_uncommon(var) &&*/ fp_exponent(var)>=EIExp_bias+(pow))
/* Macro to determine whether an ip_number is equal to a given
   power of two. Works by checking the exponent (assumes the number is
   normalised). */
#define fp_eqpow(var,pow) \
  (/*!fp_uncommon(var) &&*/ fp_exponent(var)==EIExp_bias+(pow) && \
   (var).word.lo==units_bit && (var).word.lo2==0)

/* Explicit comparison for a d.p. zero (-ve/+ve) */
#define dp_iszero(var) ((((var).word.hi<<1) | (var).word.lo)==0)
#define dp_ispos(var) (((var).word.hi & sign_bit)==0)

#define dfhi_len 20
#define dexp_len 11

/* Nasty piece of code designed to make two ARM instructions (I hope!) */
#define dp_uncommon(var) (~(((int)(var.word.hi<<1))>>(dfhi_len+1))==0)
#define dp_infinity(var) (((var.word.hi<<(dexp_len+1))|var.word.lo)==0)

#define fp_show(var) \
  fprintf(stderr,"0x%08x, 0x%08x, 0x%08x\n", \
          (var).word.hi,(var).word.lo,(var).word.lo2)

typedef struct {
  ip_number x;
  int n;
} range_red;

typedef struct {
  double f;
  int x;
} frexp_str;

extern __value_in_regs range_red __fp_range_red_by_mod(ip_number x,const ip_number *parms);
#ifdef __thumb
#define __fp_frexp(d) __16__fp_frexp(d)
extern __value_in_regs frexp_str __16__fp_frexp(double);
#else
extern __value_in_regs frexp_str __fp_frexp(double);
#endif
extern __value_in_regs ip_number __fp_poly(ip_number x,int n,const ip_number *);
extern __value_in_regs ip_number __fp_poly0(ip_number x,int n,const ip_number *);
extern __value_in_regs ip_number __fp_poly1(ip_number x,int n,const ip_number *);
#define __fp_spike(X,Y) _eadd(_emul(X,Y),Y)

/* Functions exported between the math.h functions */
extern __value_in_regs ip_number _exp(ip_number);
extern __value_in_regs ip_number _log(double);

#endif
