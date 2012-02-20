/* except.h */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1994/10/25 15:40:11 $
 * Revising $Author: mwilliam $
 */

/* exceptions.h -- exceptions as defined in exceptions.s */

#ifndef _exceptions_h
#define _exceptions_h

/* None of these operations are likely to return */

extern void __fp_invalid_operation(void);
extern void __fp_overflow(void);
extern void __fp_underflow(void);
extern void __fp_divide_by_zero(void);
extern void __fp_inexact(void);

#endif
