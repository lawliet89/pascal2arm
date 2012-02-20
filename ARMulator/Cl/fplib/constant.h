/* constant.h - floating-point constants */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1995/11/15 13:17:46 $
 * Revising $Author: mwilliam $
 */

#ifndef constant_h
#define constant_h

extern const ip_number __fp_constants[];
#define one __fp_constants[0]
#define half __fp_constants[1]
#define two __fp_constants[2]
#define pi_by_2 __fp_constants[3]
#define sinh_lnv __fp_constants[4]
#define sinh_vm2 __fp_constants[5]
#define sinh_v2m1 __fp_constants[6]

#endif
