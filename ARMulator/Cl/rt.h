#pragma force_top_level
#pragma include_only_once

/* rt.h: Interface to library kernel                            */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

#ifndef __rt_h
#define __rt_h

#ifndef __size_t
#  define __size_t 1
   typedef unsigned int size_t;   /* from <stddef.h> */
#endif

#ifndef __rt_error_and_registers
#define __rt_error_and_registers 1
typedef struct {
   unsigned errnum;      /* error number */
   char errmess[252];    /* error message (zero terminated) */
} __rt_error;

typedef struct {
  int r[16];
} __rt_registers;
#endif

extern void __rt_trap(__rt_error *, __rt_registers *);

extern void __rt_exittraphandler(void);

extern unsigned __rt_alloc(unsigned minwords, void **block);
/*
 *  Tries to allocate a block of sensible size >= minwords.  Failing that,
 *  it allocates the largest possible block (may be size zero).
 *  Sensible size means at least 2K words.
 *  *block is returned a pointer to the start of the allocated block
 *  (NULL if 'a block of size zero' has been allocated).
 */

typedef void __rt_freeproc(void *);
typedef void *__rt_allocproc(unsigned);

extern void __rt_register_allocs(__rt_allocproc *malloc, __rt_freeproc *free);
/*
 *  Registers procedures to be used by the kernel when it requires to
 *  free or allocate storage.  The allocproc may be called during stack
 *  extension, so may not check for stack overflow (nor may any procedure
 *  called from it), and must guarantee to require no more than 41 words
 *  of stack.
 */

typedef struct stack_chunk {
   unsigned long sc_mark;       /* == 0xf60690ff */
   struct stack_chunk *sc_next, *sc_prev;
   unsigned long sc_size;
   int (*sc_deallocate)();
} __rt_stack_chunk;

extern __rt_stack_chunk *__rt_current_stack_chunk(void);

/*  divide and remainder functions.
 *  The signed functions round towards zero.
 *  The result returned is a quotient in a1, and a remainder in a2.
 */

extern unsigned __rt_udiv(unsigned divisor, unsigned dividend);
extern unsigned __rt_udiv10(unsigned dividend);

extern int __rt_sdiv(int divisor, int dividend);
extern int __rt_sdiv10(int dividend);

typedef union {
    struct {int s:1, u:16, x: 15; unsigned mhi, mlo; } i;
    int w[3];
} __rt_extended_fp_number;

typedef struct {
   int r4, r5, r6, r7, r8, r9;
   int fp, sp, pc, sl;
   __rt_extended_fp_number f4, f5, f6, f7;
} __rt_unwindblock;

extern int __rt_unwind(__rt_unwindblock *inout);
/*
 *  Unwinds the call stack one level.
 *  Returns >0 if it succeeds
 *          0 if it fails because it has reached the stack end
 *          <0 if it fails for any other reason (eg stack corrupt)
 *  Input values for fp, sl and pc  must be correct.
 *  r4-r9 and f4-f7 are updated if the frame addressed by the input value
 *  of fp contains saved values for the corresponding registers.
 *  fp, sp, sl and pc are always updated
 */

#endif
/* end of rt.h */
