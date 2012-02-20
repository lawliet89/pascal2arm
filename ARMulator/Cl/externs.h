#pragma force_top_level
#pragma include_only_once

/*                                                              */
/* externals.h                                                  */
/* Non-ANSI exports from the C library, intended for client use */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

#include <stdio.h>
#include <stdarg.h>

/* these are for direct client use */

extern void _mapstore(void);
extern void _fmapstore(char *);
extern void _write_profile(char *);

extern const char *_clib_version(void);

extern void __heap_checking_on_all_deallocates(int /*on*/);
extern void __heap_checking_on_all_allocates(int /*on*/);
/*
 * If on = TRUE, the structure of the heap is checked on every deallocate
 * or allocate, giving a very expensive way of localising heap corruption.
 */

extern int _fisatty(FILE *stream);  /* not in ANSI, but related needed for ML */

extern FILE *_fopen_string_file(const char *data, int length);

/* was xmath.h - experimental extra maths library for use with Norcroft C. */

/* sqrt(x*x+y*y) but calculated with care */
extern double hypot(double x, double y);

/* the following are for compiler use. */

extern void __rt_count(void), __rt_count1(void);

/* printf functions guaranteed not to need fp formatting */
int _fprintf(FILE *fp, const char *fmt, ...);
int _printf(const char *fmt, ...);
int _sprintf(char *buff, const char *fmt, ...);
int _vfprintf(FILE *p, const char *fmt, va_list args);
int _vsprintf(char *buff, const char *fmt, va_list a);

/* and some names not expressible in ANSI C, implemented in assembler so not
   strictly required to be here.

   extern int __rt_div10(int);
   extern int __rt_udiv10(int);

   extern int x$divide(int divisor, int dividend);
   extern int x$udivide(int divisor, int dividend);

   extern int x$divtest(int divisor);
 */

