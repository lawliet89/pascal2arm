/* ctype.c: ANSI draft (X3J11 May 88) library code, section 4.3 */
/* Copyright (C) Codemist Ltd., 1988                  */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */
/* Version 3a */

#include <ctype.h>
#include "interns.h"

/* IL (illegal) is 0, but enhances readability.  */

#define IL 0
#define _UX (__U+__X)
#define _LX (__L+__X)
#define _CS (__C+__S)           /* control and space (e.g. tab) */

#if defined __ctype_c
/* for a shared library, prodice a separate module containing just
   the array __ctype, allowing it to be independently positioned to
   aid foreverness.
 */
static unsigned char eof[4] = {IL, IL, IL, IL};
unsigned char __ctype[256] = {};
#endif

#if defined ctype_c || defined SHARED_C_LIBRARY

#ifdef SHARED_C_LIBRARY
/*
 * MUST arrange that an array of ILs gets put into memory just before __ctype.
 * 4 copies for both byte sexes and to fill out a 32 bit word. This is
 * done elsewhere, in cl_data.
 */
extern unsigned char __ctype[256];
#else
static unsigned char eof[4] = {IL, IL, IL, IL};
unsigned char __ctype[256] = {};
#endif

#if 'A' == 193       /* ebcdic -- this test relies on NorCroft __C */
static const unsigned char ctype[256] = {
    __C,__C,__C,__C, IL,_CS, IL,__C, IL, IL, IL,_CS,_CS,_CS,__C,__C,
    __C,__C,__C,__C, IL, IL,__C, IL,__C,__C, IL, IL,__C,__C,__C,__C,
     IL, IL, IL, IL, IL,_CS,__C,__C, IL, IL, IL, IL, IL,__C,__C,__C,
     IL, IL,__C, IL, IL, IL, IL,__C, IL, IL, IL, IL,__C,__C, IL, IL,
__B+__S, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,
    __P, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,__P,
    __P,__P, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,
     IL,__P, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,__P,__P,
     IL,_LX,_LX,_LX,_LX,_LX,_LX,__L,__L,__L, IL, IL, IL, IL, IL, IL,
     IL,__L,__L,__L,__L,__L,__L,__L,__L,__L, IL, IL, IL, IL, IL, IL,
     IL, IL,__L,__L,__L,__L,__L,__L,__L,__L, IL, IL, IL,__P, IL, IL,
     IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P, IL, IL,
    __P,_UX,_UX,_UX,_UX,_UX,_UX,__U,__U,__U, IL, IL, IL, IL, IL, IL,
    __P,__U,__U,__U,__U,__U,__U,__U,__U,__U, IL, IL, IL, IL, IL, IL,
    __P, IL,__U,__U,__U,__U,__U,__U,__U,__U, IL, IL, IL, IL, IL, IL,
    __N,__N,__N,__N,__N,__N,__N,__N,__N,__N, IL, IL, IL, IL, IL, IL
};
#else                /* ascii  */
static const unsigned char ctype[128] = {
        __C,                     /*   nul        */
        __C,                     /*   \001       */
        __C,                     /*   \002       */
        __C,                     /*   \003       */
        __C,                     /*   \004       */
        __C,                     /*   \005       */
        __C,                     /*   \006       */
        __C,                     /*   bell       */
        __C,                     /*   backspace  */
        __C+__S,                 /*   tab        */
        __C+__S,                 /*   newline    */
        __C+__S,                 /*   vtab       */
        __C+__S,                 /*   formfeed   */
        __C+__S,                 /*   return     */
        __C,                     /*   \016       */
        __C,                     /*   \017       */
        __C,                     /*   \020       */
        __C,                     /*   \021       */
        __C,                     /*   \022       */
        __C,                     /*   \023       */
        __C,                     /*   \024       */
        __C,                     /*   \025       */
        __C,                     /*   \026       */
        __C,                     /*   \027       */
        __C,                     /*   \030       */
        __C,                     /*   \031       */
        __C,                     /*   \032       */
        __C,                     /*   \033       */
        __C,                     /*   \034       */
        __C,                     /*   \035       */
        __C,                     /*   \036       */
        __C,                     /*   \037       */
        __B+__S,                /*   space      */
        __P,                    /*   !          */
        __P,                    /*   "          */
        __P,                    /*   #          */
        __P,                    /*   $          */
        __P,                    /*   %          */
        __P,                    /*   &          */
        __P,                    /*   '          */
        __P,                    /*   (          */
        __P,                    /*   )          */
        __P,                    /*   *          */
        __P,                    /*   +          */
        __P,                    /*   ,          */
        __P,                    /*   -          */
        __P,                    /*   .          */
        __P,                    /*   /          */
        __N,                    /*   0          */
        __N,                    /*   1          */
        __N,                    /*   2          */
        __N,                    /*   3          */
        __N,                    /*   4          */
        __N,                    /*   5          */
        __N,                    /*   6          */
        __N,                    /*   7          */
        __N,                    /*   8          */
        __N,                    /*   9          */
        __P,                    /*   :          */
        __P,                    /*   ;          */
        __P,                    /*   <          */
        __P,                    /*   =          */
        __P,                    /*   >          */
        __P,                    /*   ?          */
        __P,                    /*   @          */
        __U+__X,                /*   A          */
        __U+__X,                /*   B          */
        __U+__X,                /*   C          */
        __U+__X,                /*   D          */
        __U+__X,                /*   E          */
        __U+__X,                /*   F          */
        __U,                    /*   G          */
        __U,                    /*   H          */
        __U,                    /*   I          */
        __U,                    /*   J          */
        __U,                    /*   K          */
        __U,                    /*   L          */
        __U,                    /*   M          */
        __U,                    /*   N          */
        __U,                    /*   O          */
        __U,                    /*   P          */
        __U,                    /*   Q          */
        __U,                    /*   R          */
        __U,                    /*   S          */
        __U,                    /*   T          */
        __U,                    /*   U          */
        __U,                    /*   V          */
        __U,                    /*   W          */
        __U,                    /*   X          */
        __U,                    /*   Y          */
        __U,                    /*   Z          */
        __P,                    /*   [          */
        __P,                    /*   \          */
        __P,                    /*   ]          */
        __P,                    /*   ^          */
        __P,                    /*   _          */
        __P,                    /*   `          */
        __L+__X,                /*   a          */
        __L+__X,                /*   b          */
        __L+__X,                /*   c          */
        __L+__X,                /*   d          */
        __L+__X,                /*   e          */
        __L+__X,                /*   f          */
        __L,                    /*   g          */
        __L,                    /*   h          */
        __L,                    /*   i          */
        __L,                    /*   j          */
        __L,                    /*   k          */
        __L,                    /*   l          */
        __L,                    /*   m          */
        __L,                    /*   n          */
        __L,                    /*   o          */
        __L,                    /*   p          */
        __L,                    /*   q          */
        __L,                    /*   r          */
        __L,                    /*   s          */
        __L,                    /*   t          */
        __L,                    /*   u          */
        __L,                    /*   v          */
        __L,                    /*   w          */
        __L,                    /*   x          */
        __L,                    /*   y          */
        __L,                    /*   z          */
        __P,                    /*   {          */
        __P,                    /*   |          */
        __P,                    /*   }          */
        __P,                    /*   ~          */
        __C                     /*   \177       */
};
#endif

extern void _ctype_init()
{
  int j;
  for (j = -4;  j < 0;    ++j) __ctype[j] = IL;     /* for ctype(EOF) */
  for (j =  0;  j < sizeof(ctype);  ++j)
  {
    __ctype[j] = ctype[j];
    if (sizeof(ctype) == 128) __ctype[128+j] = IL;  /* ASCII only */
  }
}

#endif /* ctype_c */

#if defined isalnum_c || defined SHARED_C_LIBRARY
int (isalnum)(int c) { return isalnum(c); }
#endif

#if defined isalpha_c || defined SHARED_C_LIBRARY
int (isalpha)(int c) { return isalpha(c); }
#endif

#if defined iscntrl_c || defined SHARED_C_LIBRARY
int (iscntrl)(int c) { return iscntrl(c); }
#endif

#if defined isdigit_c || defined SHARED_C_LIBRARY
int (isdigit)(int c) { return isdigit(c); }
#endif

#if defined isgraph_c || defined SHARED_C_LIBRARY
int (isgraph)(int c) { return isgraph(c); }
#endif

#if defined islower_c || defined SHARED_C_LIBRARY
int (islower)(int c) { return islower(c); }
#endif

#if defined isprint_c || defined SHARED_C_LIBRARY
int (isprint)(int c) { return isprint(c); }
#endif

#if defined ispunct_c || defined SHARED_C_LIBRARY
int (ispunct)(int c) { return ispunct(c); }
#endif

#if defined isspace_c || defined SHARED_C_LIBRARY
int (isspace)(int c) { return isspace(c); }
#endif

#if defined isupper_c || defined SHARED_C_LIBRARY
int (isupper)(int c) { return isupper(c); }
#endif

#if defined isxdigit_c || defined SHARED_C_LIBRARY
int (isxdigit)(int c) { return isxdigit(c); }
#endif

#if defined tolower_c || defined SHARED_C_LIBRARY
int tolower(int c) { return (isupper(c) ? c + ('a' - 'A') : c); }
#endif

#if defined toupper_c || defined SHARED_C_LIBRARY
int toupper(int c) { return (islower(c) && c != 0xdf ? c + ('A' - 'a') : c); }
#endif

#if defined _set8859_c || defined SHARED_C_LIBRARY

void _set_ctype_8859(int yes)
{
  unsigned int j;               /* unsigned is cue to division */

  if (yes) {
    static const unsigned char t[] = { __C, __P, __U, __L };
    for (j = 128;  j < 256; ++j) __ctype[j] = t[(j-128) / 32];
    __ctype[0xD7] = __P; /* times sign */
    __ctype[0xF7] = __P; /* divide sign */
    __ctype[0xDF] = __L; /* German sz */
  } else {
    for (j = 128;  j < 256;  ++j) __ctype[j] = IL;
  }
}

#endif

/* End of ctype.c */
