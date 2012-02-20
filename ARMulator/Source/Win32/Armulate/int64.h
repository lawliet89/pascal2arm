/*
 * int64.h
 * Copyright (C) Advanced RISC Machines Limited 1996. All rights reserved.
 */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 1997/05/06 15:58:34 $
 * Revising $Author: hmeeking $
 */

#ifndef int64_h
#define int64_h

#include "host.h"
#include "ieeeflt.h"

typedef struct { uint32 lo; int32 hi; } int64;
typedef struct { uint32 lo, hi; } uint64;

typedef enum {
  i64_ok,
  i64_overflow,
  i64_negative,
  i64_divide_by_zero,
  i64_overlarge_shift
} I64_Status;

I64_Status I64_SAdd(int64 *res, int64 const *a, int64 const *b);
I64_Status I64_SSub(int64 *res, int64 const *a, int64 const *b);
I64_Status I64_SMul(int64 *res, int64 const *a, int64 const *b);
I64_Status I64_SDiv(int64 *quot, int64 *rem, int64 const *a, int64 const *b);

/* These are the same as the signed set above, except that overflow is set differently */
I64_Status I64_UAdd(uint64 *res, uint64 const *a, uint64 const *b);
I64_Status I64_USub(uint64 *res, uint64 const *a, uint64 const *b);
I64_Status I64_UMul(uint64 *res, uint64 const *a, uint64 const *b);
I64_Status I64_UDiv(uint64 *quot, uint64 *rem, uint64 const *a, uint64 const *b);

I64_Status I64_Neg(int64 *res, int64 const *a);

I64_Status I64_SRsh(int64 *res, int64 const *a, unsigned b);
I64_Status I64_URsh(uint64 *res, uint64 const *a, unsigned b);
I64_Status I64_Lsh(int64 *res, int64 const *a, unsigned b);
I64_Status I64_Ror(int64 *res, int64 const *a, unsigned b);

void I64_And(int64 *res, int64 const *a, int64 const *b);
void I64_Or(int64 *res, int64 const *a, int64 const *b);
void I64_Eor(int64 *res, int64 const *a, int64 const *b);
void I64_Not(int64 *res, int64 const *a);

I64_Status I64_SToI(int32 *res, int64 const *a);
I64_Status I64_UToI(uint32 *res, uint64 const *a);
I64_Status I64_IToS(int64 *res, int32 a);
I64_Status I64_IToU(uint64 *res, uint32 a);

I64_Status I64_DToI(int64 *res, DbleBin const *a);
I64_Status I64_DToU(uint64 *res, DbleBin const *a);
I64_Status I64_IToD(DbleBin *res, int64 const *a);
I64_Status I64_UToD(DbleBin *res, uint64 const *a);

int I64_SComp(int64 const *a, int64 const *b);
int I64_UComp(uint64 const *a, uint64 const *b);

int I64_sprintf(char *b, char const *fmt, int64 const *p);

#endif /* int64_h */

/* End of int64.h */

