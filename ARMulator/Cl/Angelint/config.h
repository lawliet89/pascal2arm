#pragma force_top_level
#pragma include_only_once

/* config.h: hardware details of host system for CLIB */
/* Copyright (C) Codemist Ltd, 1988                */
/* Copyright (C) Advanced RISC Machines Ltd., 1991 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1996/05/10 15:28:44 $ 0
 * Revising $Author: amerritt $
 */

#ifndef __config_h
#define __config_h

#ifdef HOSTSEX_b /* ig__ENDIAN */
#  define BYTESEX_ODD 1
#else
#  ifdef HOSTSEX_l /* itle_ENDIAN */
#  else
      #error byte sex not specified - assume little endian
#  endif
#  define BYTESEX_EVEN 1
#endif

#define MACHINE "ARM"

/* this SOFTWARE_FLOATING_POINT must refer to some very old non-trig vsn */
#ifndef SOFTWARE_FLOATING_POINT
#  define HOST_HAS_TRIG 1         /* and ieee trig functions     */
#  define IEEE 1
/* IEEE floating point format assumed.                                  */
/* standard ARM double values are mixed-endian (the first               */
/* word of a double value is the one containing the exponent).          */
#  undef OTHER_WORD_ORDER_FOR_FP_NUMBERS
#  define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1
#endif

/* The following code is NOT PORTABLE but can stand as a prototype for   */
/* whatever makes sense on other machines.                               */

/* Structure of a double value                                           */

#ifndef OTHER_WORD_ORDER_FOR_FP_NUMBERS

#  ifdef BYTESEX_EVEN
typedef union {struct {int mhi:20, x:11, s:1; unsigned mlo; } i;
               double d; } fp_number;
#  else
typedef union {struct {int s:1, x:11, mhi:20; unsigned mlo; } i;
               double d; } fp_number;
#  endif

#else   /* OTHER_WORD_ORDER_FOR_FP_NUMBERS */

#  ifdef BYTESEX_EVEN
typedef union {struct {unsigned mlo; int mhi:20, x:11, s:1; } i;
               double d; } fp_number;
#  else
typedef union {struct {unsigned mlo; int s:1, x:11, mhi:20; } i;
               double d; } fp_number;
#  endif
#endif  /* OTHER_WORD_ORDER_FOR_FP_NUMBERS */

/* the object of the following macro is to adjust the floating point     */
/* variables concerned so that the more significant one can be squared   */
/* with NO LOSS OF PRECISION. It is only used when there is no danger    */
/* of over- or under-flow.                                               */

/* This code is NOT PORTABLE but can be modified for use elsewhere       */
/* It should, however, serve for IEEE and IBM FP formats.                */

#define _fp_normalize(high, low)                                          \
    {   fp_number temp;        /* access to representation     */         \
        double temp1;                                                     \
        temp.d = high;         /* take original number         */         \
        temp.i.mlo = 0;        /* make low part of mantissa 0  */         \
        temp1 = high - temp.d; /* the bit that was thrown away */         \
        low += temp1;          /* add into low-order result    */         \
        high = temp.d;                                                    \
    }

#endif

/* end of config.h */
