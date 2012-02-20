/* constant.c - floating-point constants */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1995/11/15 13:17:44 $
 * Revising $Author: mwilliam $
 */

#include "mathlib.h"
#include "constant.h"

const ip_number __fp_constants[] = { /* order must be kept in step with .h */
/* one */     { 0x00003fff, 0x80000000, 0x00000000 },
/* half */    { 0x00003ffe, 0x80000000, 0x00000000 },
/* two */     { 0x00004000, 0x80000000, 0x00000000 },
/* pi_by_2 */ { 0x00003fff, 0xc90fdaa2, 0x2168c235 },
/* lnv */     { 0x00003ffe, 0xb1730000, 0x00000000 },
/* vm2 */     { 0x00003ffc, 0xfffe2ff1, 0x483b9d27 },
/* v2m1 */    { 0x00003fee, 0xe8089758, 0x1016b37d }
};
