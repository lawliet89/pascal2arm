#pragma force_top_level
#pragma include_only_once

/*
 * math.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.5
 * Copyright (C) Codemist Ltd., 1988
 * Copyright 1991-1997 Advanced Risc Machines Limited. All rights reserved
 */

/*
 * RCS $Revision: 1.4.22.2 $ Codemist 0.03
 * Checkin $Date: 1997/12/10 16:23:16 $
 * Revising $Author: ijohnson $
 */

#ifndef __math_h
#define __math_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HUGE_VAL
#  define HUGE_VAL __huge_val
extern const double HUGE_VAL;
#endif

/* Be careful with no_side_effects.
   Nothing taking a pointer argument can safely be declared to be so
   (how can CSE know whether the thing pointed to has been updated, or
    whether indeed the argument is not an output).
   Nor may things which set errno - a side effect -.
 */

extern double acos(double /*x*/);
   /* computes the principal value of the arc cosine of x */
   /* a domain error occurs for arguments not in the range -1 to 1 */
   /* Returns: the arc cosine in the range 0 to Pi. */
extern double asin(double /*x*/);
   /* computes the principal value of the arc sine of x */
   /* a domain error occurs for arguments not in the range -1 to 1 */
   /* and -HUGE_VAL is returned. */
   /* Returns: the arc sine in the range -Pi/2 to Pi/2. */

#pragma no_side_effects

extern double atan(double /*x*/);
   /* computes the principal value of the arc tangent of x */
   /* Returns: the arc tangent in the range -Pi/2 to Pi/2. */

extern double __d_atan(double);

#pragma side_effects

extern double atan2(double /*y*/, double /*x*/);
   /* computes the principal value of the arc tangent of y/x, using the */
   /* signs of both arguments to determine the quadrant of the return value */
   /* a domain error occurs if both args are zero, and -HUGE_VAL returned. */
   /* Returns: the arc tangent of y/x, in the range -Pi to Pi. */


#pragma no_side_effects

extern double cos(double /*x*/);
   /* computes the cosine of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the cosine value. */
extern double sin(double /*x*/);
   /* computes the sine of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the sine value. */

#pragma side_effects

extern double tan(double /*x*/);
   /* computes the tangent of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the tangent value. */
   /*          if range error; returns HUGE_VAL. */

extern double cosh(double /*x*/);
   /* computes the hyperbolic cosine of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the hyperbolic cosine value. */
   /*          if range error; returns HUGE_VAL. */
extern double sinh(double /*x*/);
   /* computes the hyperbolic sine of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the hyperbolic sine value. */
   /*          if range error; returns -HUGE_VAL or HUGE_VAL depending */
   /*          on the sign of the argument */

#pragma no_side_effects

extern double tanh(double /*x*/);
   /* computes the hyperbolic tangent of x. */
   /* Returns: the hyperbolic tangent value. */

#pragma side_effects

extern double exp(double /*x*/);
   /* computes the exponential function of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the exponential value. */
   /*          if underflow range error; 0 is returned. */
   /*          if overflow range error; HUGE_VAL is returned. */

extern double frexp(double /*value*/, int * /*exp*/);
   /* breaks a floating-point number into a normalised fraction and an */
   /* integral power of 2. It stores the integer in the int object pointed */
   /* to by exp. */
   /* Returns: the value x, such that x is a double with magnitude in the */
   /* interval 0.5 to 1.0 or zero, and value equals x times 2 raised to the */
   /* power *exp. If value is zero, both parts of the result are zero. */

extern double ldexp(double /*x*/, int /*exp*/);
   /* multiplies a floating-point number by an integral power of 2. */
   /* A range error may occur. */
   /* Returns: the value of x times 2 raised to the power of exp. */
   /*          if range error; HUGE_VAL is returned. */
extern double log(double /*x*/);
   /* computes the natural logarithm of x. A domain error occurs if the */
   /* argument is negative, and -HUGE_VAL is returned. A range error occurs */
   /* if the argument is zero. */
   /* Returns: the natural logarithm. */
   /*          if range error; -HUGE_VAL is returned. */
extern double log10(double /*x*/);
   /* computes the base-ten logarithm of x. A domain error occurs if the */
   /* argument is negative. A range error occurs if the argument is zero. */
   /* Returns: the base-ten logarithm. */
extern double modf(double /*value*/, double * /*iptr*/);
   /* breaks the argument value into integral and fraction parts, each of */
   /* which has the same sign as the argument. It stores the integral part */
   /* as a double in the object pointed to by iptr. */
   /* Returns: the signed fractional part of value. */

extern double pow(double /*x*/, double /*y*/);
   /* computes x raised to the power of y. A domain error occurs if x is */
   /* zero and y is less than or equal to zero, or if x is negative and y */
   /* is not an integer, and -HUGE_VAL returned. A range error may occur. */
   /* Returns: the value of x raised to the power of y. */
   /*          if underflow range error; 0 is returned. */
   /*          if overflow range error; HUGE_VAL is returned. */
extern double sqrt(double /*x*/);
   /* computes the non-negative square root of x. A domain error occurs */
   /* if the argument is negative, and -HUGE_VAL returned. */
   /* Returns: the value of the square root. */

#pragma no_side_effects

extern double ceil(double /*x*/);
   /* computes the smallest integer not less than x. */
   /* Returns: the smallest integer not less than x, expressed as a double. */
extern double fabs(double /*x*/);
   /* computes the absolute value of the floating-point number x. */
   /* Returns: the absolute value of x. */

extern double __d_abs(double);  /* inline expansion of fabs() */

extern double floor(double /*d*/);
   /* computes the largest integer not greater than x. */
   /* Returns: the largest integer not greater than x, expressed as a double */

#pragma side_effects

extern double fmod(double /*x*/, double /*y*/);
   /* computes the floating-point remainder of x/y. */
   /* Returns: the value x - i * y, for some integer i such that, if y is */
   /*          nonzero, the result has the same sign as x and magnitude */
   /*          less than the magnitude of y. If y is zero, a domain error */
   /*          occurs and -HUGE_VAL is returned. */

#ifndef __SOFT_DOUBLES__
#  define fabs(x) __d_abs(x)
#endif

#ifdef __cplusplus
}
#endif

#endif

/* end of math.h */
