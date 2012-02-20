/*
 * scanf.c: ANSI draft (X3J11 Oct 86) part of section 4.9 code
 * Copyright (C) Codemist Ltd., 1988
 * Copyright: 1991-1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.11.2.3 $ Codemist 4a
 * Checkin $Date: 1998/03/02 17:43:00 $ 0
 * Revising $Author: ijohnson $
 */

/* BEWARE: there are quite a few ambiguities/oddities in the Oct 86 ANSI   */
/* draft definition of scanf().                                            */
/* Memo: consider using ftell() (or rather fgetpos() for big file worries) */
/* one day instead of charcount below.  See also 'countgetc()'.            */
/* Memo 2: the code below always reads one char beyond the end of the      */
/* item to be converted.  The exception is '%c' (q.v.).                    */
/* The last char is then __backspace()'d.  This is done to avoid using up  */
/* the 1 char ungetc() guaranteed at all other times.                      */

#include "hostsys.h"       /* which requires a FILEHANDLE to be defined  */

#include <stdio.h>       /* we define scanf for this        */
#include <stdlib.h>      /* and strtol/strtoul etc for this */
#include <math.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#include "config.h"     /* _fp_normalise */
#include "interns.h"
#include "scanf.h"

#ifdef EMBEDDED_CLIB
extern ___weak volatile int *__rt_errno_addr(void);
#define set_errno(e) (__rt_errno_addr ? (*__rt_errno_addr() = e) : 0)
#define read_errno() (__rt_errno_addr ? *__rt_errno_addr() : 0)
#else
#define set_errno(e) (errno = (e))
#define read_errno() errno
#endif

#if defined _chval_c || defined SHARED_C_LIBRARY

int _chval(int ch, int radix)
{
    int val;
    /* Memo: beware that ANSI currently say a-z act as 10-35 for strtol() */
    /* etc.  The test below is isalpha() etc.  This means that this test  */
    /* may not work in a non-C locale where isalpha('{') may be true      */
    /* (e.g. Swedish ASCII).                                              */
    if ('A' == 193)  /* ebcdic */
        val = (isdigit(ch) ? (ch) - '0' :
               isalpha(ch) ? (ch |= 0x40,         /* quick ebcdic toupper */
                   ch <= 'I' ? ch - 'A' + 10 :
                   ch <= 'R' ? ch - 'J' + 19 :
                               ch - 'S' + 28) :
               -1);
    else
        val = (isdigit(ch) ? (ch) - '0' :
               islower(ch) ? (ch) - 'a' + 10 :
               isupper(ch) ? (ch) - 'A' + 10 :
               -1);
    return (val < radix ? val : -1);
}

#endif

typedef struct {
  va_list argv;
  int flag;
  int field;
  int (*getch)(FILE *);
  int (*backspace)(FILE *);
} ScanfReadRec;

int __vfscanf(FILE *p, char const *sfmt, ScanfReadRec *sr);

#if defined _scanf_c || defined SHARED_C_LIBRARY

#define countgetc(sr, p) (charcount++, sr->getch(p))

/* The next macros, with the help of the compiler, ensures that we can */
/* test for LONG and SHORT properly, but not general extra code.       */
#define isLONGLONG_(flag) ((flag) & LONGLONG)
#define isLONG_(flag) ((flag) & LONG && sizeof(int) != sizeof(long))
#define isSHORT_(flag) ((flag) & SHORT)
#define isLONGDOUBLE_(flag) \
    ((flag) & LONGDOUBLE && sizeof(double) != sizeof(long double))

#define CVTEOF     (-1)   /* used for eof return (!= any number of chars)   */
#define CVTFAIL    (-2)   /* used for error return (!= any number of chars) */

#define scanf_intofdigit(c) ((c) - '0')

static long int rd_longlong(FILE *p, int radix, ScanfReadRec *sr)
{   long int charcount = -1;                   /* allow for always ungetc */
    unsigned long long n = 0;
    int ch;
    int flag = sr->flag;
    int field = sr->field;
    while (isspace(ch = countgetc(sr, p)))
        continue;                                   /* leading whitespace */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+NUMNEG);
    if (field > 0 && flag & ALLOWSIGN) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(sr, p);
            field--;
            break;
    }
    if (field > 0 && ch=='0')
    {   flag |= NUMOK, field--;  /* starts with 0 - maybe octal or hex mode */
        ch = countgetc(sr, p);
        if (field > 0 && (ch=='x' || ch=='X') && (radix==0 || radix==16))
        {   flag &= ~NUMOK, field--;
            ch = countgetc(sr, p);
            radix = 16;
        }
        else if (radix == 0) radix = 8;
    }
    if (radix == 0) radix = 10;
    {   int digit;
        while (field > 0 && (digit = _chval(ch, radix)) >= 0)
        {   flag |= NUMOK, field--;
            n = n*radix + digit;
            ch = countgetc(sr, p);
        }
    }
    sr->backspace(p);
    if (!(flag & NUMOK)) return CVTFAIL;
    if (!(flag & NOSTORE))
    { /* This code is pretty specious on a 2's complement machine         */
      if (flag & ALLOWSIGN)
      { long long m = flag & NUMNEG ? -n : n;
        long long *p = va_arg(sr->argv, long long *);
        *p = m;
      }
      else {
        unsigned long long *p = va_arg(sr->argv, unsigned long long *);
        *p = n;
      }
    }
    return charcount;
}

static long int rd_int(FILE *p, int radix, ScanfReadRec *sr) {
  int flag = sr->flag;
  if (flag & LONGLONG)
    return rd_longlong(p, radix, sr);
  else {
    long int charcount = -1;                   /* allow for always ungetc */
    unsigned long int n = 0;
    int ch;
    int field = sr->field;
    while (isspace(ch = countgetc(sr, p)))
        continue;                                   /* leading whitespace */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+NUMNEG);
    if (field > 0 && flag & ALLOWSIGN) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(sr, p);
            field--;
            break;
    }
    if (field > 0 && ch=='0')
    {   flag |= NUMOK, field--;  /* starts with 0 - maybe octal or hex mode */
        ch = countgetc(sr, p);
        if (field > 0 && (ch=='x' || ch=='X') && (radix==0 || radix==16))
        {   flag &= ~NUMOK, field--;
            ch = countgetc(sr, p);
            radix = 16;
        }
        else if (radix == 0) radix = 8;
    }
    if (radix == 0) radix = 10;
    {   int digit;
        while (field > 0 && (digit = _chval(ch, radix)) >= 0)
        {   flag |= NUMOK, field--;
            n = n*radix + digit;
            ch = countgetc(sr, p);
        }
    }
    sr->backspace(p);
    if (!(flag & NUMOK)) return CVTFAIL;
    if (!(flag & NOSTORE))
    { /* This code is pretty specious on a 2's complement machine         */
      if (flag & ALLOWSIGN)
      { long int m = flag & NUMNEG ? -n : n;
        void *p = va_arg(sr->argv, void *);
        if isSHORT_(flag)
            *(short *)p = (short)m;
        else if isLONG_(flag)
            *(long *)p = m;
        else
            *(int *)p = (int)m;
      }
      else  /* pointer case comes here too - with quite some type pun!  */
      { void *p = va_arg(sr->argv, void *);
            /* rely on sizeof(unsigned int *)==sizeof(unsigned short *) */
        if isSHORT_(flag)
            *(unsigned short *)p = (unsigned short)n;
        else if isLONG_(flag)
            *(unsigned long *)p = n;
        else
            *(unsigned int *)p = (unsigned int)n;
      }
    }
    return charcount;
  }
}

#ifndef NO_FLOATING_POINT

static float carefully_narrow(double l)
/* All the trouble I went to to detect potential overflow has to be re-  */
/* done here so that underflow and overflow do not occur during the      */
/* narrowing operation that is about to be done.                         */
/* *********** machine dependent code ************* (should not go here) */
    {   int w;
        (void)frexp(l, &w);     /* extract exponent */
        w = w + 0x7e;           /* exponent for single precision version */
        if (w<=0 && l!=0.0)
        {   set_errno(ERANGE);
            return 0.0f;
        }
        else if (w >= 0xff)
/* Overflow of single-precision values - fudge single precision infinity */
/* *********** machine dependent code ************* (should not go here) */
        {   static int posinf = 0x7f800000, neginf = 0xff800000;
            set_errno(ERANGE);
            return *(float *)(l >= 0.0 ? &posinf : &neginf);
        }
        else return (float)l;  /* what we really wanted to do */
    }

#ifdef HOST_HAS_BCD_FLT

static long int rd_real(FILE *p, ScanfReadRec *sr)
{
    long int charcount = -1;                     /* allow for always ungetc */
    int ch, x = 0;
    unsigned int a[3];            /* IEEE 'packed' format as per ACORN FPE2 */
    double l = 0.0;
    int flag = sr->flag;
    int field = sr->field;
    a[0] = a[1] = a[2] = 0;
    while (isspace(ch = countgetc(sr, p)))
        continue;                        /* not counted towards field width */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+DOTSEEN+NUMNEG);
    if (field > 0) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(sr, p);
            field--;
            break;
    }
    while (field > 0)
    {   if (ch==DecimalPoint && !(flag & DOTSEEN))
            flag |= DOTSEEN, field--;
        else if (isdigit(ch))
        {   flag |= NUMOK, field--;
            if ((a[0] & 0xf00) == 0)
            {   a[0] = (a[0]<<4) | (a[1]>>28);
                a[1] = (a[1]<<4) | (a[2]>>28);
                a[2] = (a[2]<<4) | scanf_intofdigit(ch);
                if (flag & DOTSEEN) x -= 1;
            }
            else if (!(flag & DOTSEEN)) x += 1;
        }
        else break;
        ch = countgetc(sr, p);
    }
    /* we must unread the 'e' in (say) "+.e" as cannot be valid */
    if (field > 0 && (ch == 'e' || ch == 'E') && (flag & NUMOK))
    {   int x2 = 0;
        flag &= ~(NUMOK+NEGEXP), field--;
        switch (ch = countgetc(sr, p))
        {
    case '-':   flag |= NEGEXP;
    case '+':   ch = countgetc(sr, p);
                field--;
    default:    break;
        }
        while (field > 0 && isdigit(ch))
        {   flag |= NUMOK, field--;
            x2 = 10*x2 + scanf_intofdigit(ch);
            ch = countgetc(sr, p);
        }
        if (flag & NEGEXP) x -= x2; else x += x2;
    }
    sr->backspace(p);
    if (a[0]==0 && a[1]==0 && a[2]==0) l = 0.0;
    else
    {   if (a[0]==0 && a[1]==0)
        {   a[1] = a[2];
            a[2] = 0;
            x -= 8;
        }
        while ((a[0] & 0xf00)==0)
        {   a[0] = (a[0]<<4) | (a[1]>>28);
            a[1] = (a[1]<<4) | (a[2]>>28);
            a[2] = a[2]<<4;
            x -= 1;
        }
        x += 18;    /* allow for position of decimal point in packed format */
        if (x < -999)
        {
            l = 0.0;
            set_errno(ERANGE);
        } else if (x > 999)
        {   l = (flag & NUMNEG) ? -HUGE_VAL : HUGE_VAL;
            set_errno(ERANGE);
        }
        else
        {
            if (x < 0) a[0] |= 0x40000000, x = -x;
            a[0] |= (x % 10) << 12;
            x = x / 10;
            a[0] |= (x % 10) << 16;
            x = x / 10;
            a[0] |= (x % 10) << 20;
            if (flag & NUMNEG) a[0] |= 0x80000000;
            l = _ldfp(a);       /* sets errno if necessary */
        }
    }
    if (!(flag & NUMOK)) return CVTFAIL;
    if (flag & LONG)
    {   if (!(flag & NOSTORE))
        {   if (isLONGDOUBLE_(flag))
                *va_arg(sr->argv, long double *) = l;  /* not fully done */
            else
                *va_arg(sr->argv, double *) = l;
        }
    }
    else
    {   float f = carefully_narrow(l);
        /* treat overflow consistently whether or not stored */
        if (!(flag & NOSTORE)) *va_arg(sr->argv, float *) = f;
    }
    return charcount;
}

#else /* HOST_HAS_BCD_FLT */

static long int rd_real(FILE *p, ScanfReadRec *sr)
{
    long int charcount = -1;                     /* allow for always ungetc */
    int ch, x = 0, w;
    int i = 0;
    double l = 0.0, pten = 0.1;
    int flag = sr->flag;
    int field = sr->field;
    while (isspace(ch = countgetc(sr, p)))
        continue;                        /* not counted towards field width */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+DOTSEEN+NUMNEG);
    if (field > 0) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(sr, p);
            field--;
            break;
    }
/* I accumulate up to 6 (decimal) significant figures in the integer     */
/* variable i, and remaining digits in the floating point variable l.    */
    while (field > 0)
    {   if (ch==DecimalPoint && !(flag & DOTSEEN))
            flag |= DOTSEEN, field--;
        else if (isdigit(ch))
        {   flag |= NUMOK, field--;
            if (i < 100000)
            {   i = 10*i + scanf_intofdigit(ch);
                if (flag & DOTSEEN) x -= 1;
            }
            else
            {  
                l += pten * (int)scanf_intofdigit(ch);
                pten /= 10.0;
                if (!(flag & DOTSEEN)) x += 1;
            }
        }
        else break;
        ch = countgetc(sr, p);
    }
    /* we must unread the 'e' in (say) "+.e" as cannot be valid */
    if (field > 0 && (ch == 'e' || ch == 'E') && (flag & NUMOK))
    {   int x2 = 0;
        flag &= ~(NUMOK+NEGEXP), field--;
        switch (ch = countgetc(sr, p))
        {
    case '-':   flag |= NEGEXP;
    case '+':   ch = countgetc(sr, p);
                field--;
    default:    break;
        }

/* If the exponent starts to get very large, then we give up trying     */
/* to convert it. This might be in contravention of the ANSI standard;  */
/* for example strtod("0.000 ... 0001e1000000") where ... represents a  */
/* million zeros - we will return the wrong value. However in such      */
/* cases "the wrong value" includes setting errno, so all is not lost.  */
        while (x2 < 1000000 && field > 0 && isdigit(ch))
        {   flag |= NUMOK, field--;
            x2 = 10*x2 + scanf_intofdigit(ch);
            ch = countgetc(sr, p);
        }
        if (x2 < 1000000) {
          if (flag & NEGEXP) x -= x2; else x += x2;
        } else {
          /* the exponent is massive - ensure that ERANGE is set. [MLS 766] */
          if (flag & NEGEXP) x = -x2; else x = x2;
        }
    }
    sr->backspace(p);


    /* catch massive underflow/overflow now, to prevent d3 from
     * overflowing later */
    if (x>=512) {
      if (i || l) {             /* zero mantissa gives zero result [MLS 765] */
        l = (flag & NUMNEG) ? -HUGE_VAL : HUGE_VAL;
        set_errno(ERANGE);
      }
    } else if (x<=-512) {
      if (i || l) {
        l = 0.0;
        set_errno(ERANGE);
      }
    } else {
/* The code that follows multiplies (i.l) by 10^x using one-and-a-half   */
/* precision arithmetic, with relevant scaling so that any over or under */
/* flow is deferred to the very last minute.                             */
        double d, dlow, d3, d3low;
        int bx = (10*x)/3, w1;
        l = ldexp(l, x-bx);
        d = ldexp((double)i, x-bx);
        dlow = 0.0;
        if (x < 0)
        {   w1 = -x;
            d3 = 0.2;
            d3low = 0.0;
            _fp_normalize(d3, d3low);
            d3low = (1.0 - 5.0*d3)/5.0;
        }
        else
        {   w1 = x;
            d3 = 5.0;
            d3low = 0.0;
        }
        if (w1!=0) for(;;)
        {   if ((w1&1)!=0)
            {   l *= (d3 + d3low);
                dlow = d*d3low + dlow*(d3 + d3low);
                d *= d3;
                _fp_normalize(d, dlow);
                if (w1==1) break;
            }
            d3low *= (2.0*d3 + d3low);
            d3 *= d3;
            _fp_normalize(d3, d3low);
            w1 = w1 >> 1;
        }
        l = l + dlow;
        l = l + d;
        l = frexp(l, &w);
        w += bx;
/* Now I check to see if the number would give a floating point overflow */
/* and if so I return HUGE_VAL, and set errno to ERANGE.                 */
/* ********* machine dependent integers ***********                      */
        if (w >= 0x7ff-0x3fe)
        {
          if (l!=0.0) { /* zero mantissa gives zero result [MLS 765] */
            l = (flag & NUMNEG) ? -HUGE_VAL : HUGE_VAL;
            set_errno(ERANGE);
          }
          /* else l=0, which is correct */
        }
/* Underflows yield a zero result but set errno to ERANGE                */
        else if (w <= -0x3fe && l!=0.0)
        {   l = 0.0;
            set_errno(ERANGE);
        }
        else
        {   l = (flag & NUMNEG) ? -ldexp(l, w) : ldexp(l, w);
        }
    }
    if (!(flag & NUMOK)) return CVTFAIL;
    if (flag & LONG)
    {   if (!(flag & NOSTORE))
        {   if (isLONGDOUBLE_(flag))
                *va_arg(sr->argv, long double *) = l;  /* not fully done */
            else
                *va_arg(sr->argv, double *) = l;
        }
    }
    else
    {   float f = carefully_narrow(l);
        /* treat overflow consistently whether or not stored */
        if (!(flag & NOSTORE))
            *va_arg(sr->argv, float *) = f;
    }
    return charcount;
}



#endif /* HOST_HAS_BCD_FLT */

#endif /* NO_FLOATING_POINT */

/* Amalgamate the next two routines? */
static long int rd_string(FILE *p, ScanfReadRec *sr)
{   long int charcount = -1;                     /* allow for always ungetc */
    int ch; char *s = NULL;
    int flag = sr->flag;
    int field = sr->field;
    while (isspace(ch = countgetc(sr, p)))
        continue;                        /* not counted towards field width */
    if (ch == EOF) return CVTEOF;                /* fail if EOF occurs here */
    if (!(flag & NOSTORE))
        s = va_arg(sr->argv, char *);
    while (field > 0 && ch != EOF && !isspace(ch))
    {   field--;
        if (!(flag & NOSTORE)) *s++ = ch;
        ch = countgetc(sr, p);
    }
    sr->backspace(p);                               /* OK if ch == EOF         */
    if (!(flag & NOSTORE)) *s = 0;
    return charcount;
}

/* Ambiguity in Oct 86 ANSI draft: can "%[x]" match a zero-length string?  */
/* p119 line 19 suggests no, p121 example suggests yes.  Treat as yes here */
static long int rd_string_map(FILE *p, int charmap[], ScanfReadRec *sr)
{   long int charcount = -1;                     /* allow for always ungetc */
    int ch; char *s = NULL;
    int flag = sr->flag;
    int field = sr->field;
    if (!(flag & NOSTORE))
        s = va_arg(sr->argv, char *);
    ch = countgetc(sr, p);
    if (ch == EOF) return CVTEOF;
    while (field > 0 && ch != EOF && (charmap[ch/32] & (1<<(ch%32))) != 0)
    {   field--;
        if (!(flag & NOSTORE)) *s++ = ch;
        ch = countgetc(sr, p);
    }
    sr->backspace(p);                               /* OK if ch == EOF         */
    if (!(flag & NOSTORE)) *s = 0;
    return charcount;
}

/* It seems amazing that vfscanf is not available externally in ANSI */
int __vfscanf(FILE *p, char const *sfmt, ScanfReadRec *sr)
{
/* The next line is essential (see isspace() ANSI doc. and also use of
 * charmap[] below) if char is signed by default.
 * Our char is unsigned, but the following line should
 * just use the same register for fmt/sfmt and so cost nothing!
 */
    unsigned char const *fmt = (unsigned char const *)sfmt;
    int cnt = 0;
    long charcount = 0;
    for (;;)
    {   int fch;
        switch (fch = *fmt++)
        {
case 0:     return cnt;                        /* end of format string   */

default:    {   int ch;
                if (isspace(fch))              /* consolidate whitespace */
                {   int seen = 0;
                    while (isspace(fch = *fmt++))
                        continue;
                    fmt--;
/* N.B. isspace() must return 0 if its arg is '\0' or EOF.               */
                    while (isspace(ch = sr->getch(p))) {
                        charcount++;
                        seen = 1;
                    }
                    sr->backspace(p);
#ifdef never
/* The next line requires non empty whitespace to match format whilespace. */
/* Removed as incompatible with bsd unix (and other prior practice?).      */
                    if (!seen) return cnt;     /* require at least 1     */
#endif
                    continue;
                }
                else if ((ch = sr->getch(p)) == fch)
                {   charcount++;
                    continue;
                }
                sr->backspace(p);  /* offending char is left unread         */
                if (ch == EOF && cnt == 0) return EOF;
                return cnt;     /* unmatched literal                     */
            }

case '%':   {   int field = 0, flag = 0;
                long int worked;
                if (*fmt == '*') fmt++, flag |= NOSTORE;
                while (isdigit(fch = *fmt++))
                {   if (field > INT_MAX/10) return cnt; /* overflow check */
                    field = field*10 + fch - '0';
                    if (field < 0) return cnt;         /* overflow check */
                    flag |= FIELDGIVEN;
                }
                if (!(flag & FIELDGIVEN)) field = INT_MAX;
                if (fch == 'l') {
                    fch = *fmt++;
                    if (fch == 'l') 
                        fch = *fmt++, flag |= LONGLONG;
                    else
                        flag |= LONG;
                } else if (fch == 'L') {
                    fch = *fmt++;
                    if (fch == 'L') 
                        fch = *fmt++, flag |= LONGLONG;
                    else
                        flag |= LONG | LONGDOUBLE;
                } else if (fch == 'h')
                    fch = *fmt++, flag |= SHORT;
                sr->flag = flag;
                sr->field = field;
                switch (fch)
                {
        default:    return cnt;         /* illegal conversion code       */
        case '%':   {   int ch = sr->getch(p);
/* treat as fatuous the omission of '%' from non-skipping white space list */
                        if (ch == '%')
                        {   charcount++;
                            continue;
                        }
                        sr->backspace(p);  /* offending char is left unread */
                        if (ch == EOF && cnt == 0) return EOF;
                        return cnt;     /* unmatched literal '%'         */
                    }
        case 'c':   if (!(flag & FIELDGIVEN)) field = 1;
                    {   char *cp = NULL; int ch;
                        if (!(flag & NOSTORE))
                            cp = va_arg(sr->argv, char *);
/* ANSI say no chars match -> failure.  Hence 0 width must always fail.     */
                        if (field == 0)
                            return cnt;
                        for (; field > 0; field--)
/* The next line reflects the ANSI wording suggesting EXACTLY 'field' chars */
/* should be read.                                                          */
                        {   if ((ch = countgetc(sr, p)) == EOF)
                                return cnt == 0 ? EOF : cnt;
                            if (!(flag & NOSTORE)) *cp++ = ch;
                        }
                    }
                    if (!(flag & NOSTORE)) cnt++; /* If conversion succeeds */
                    continue;
        case 'd':   sr->flag |= ALLOWSIGN;
                    worked = rd_int(p, 10, sr);
                    break;
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
#ifndef NO_FLOATING_POINT
                    worked = rd_real(p, sr);
#else
                    return(cnt);    /* Floating point not implemented    */
#endif
                    break;
        case 'i':   sr->flag |= ALLOWSIGN;
                    worked = rd_int(p, 0, sr);
                    break;
/* %n assigns the number of characters read from the input so far - 0TE */
/* that this assignment is 0T influenced by the * flag and does 0T     */
/* count towards the value returned by scanf.  Note that h and l apply.  */
        case 'n':   {   void *p = va_arg(sr->argv, void *);
                        if isSHORT_(flag)
                            *(short *)p = (short)charcount;
                        else if isLONGLONG_(flag)
                            *(long long *)p = (long long)charcount;
                        else if isLONG_(flag)
                            *(long *)p = charcount;
                        else
                            *(int *)p = (int)charcount;
                    }
                    continue;
        case 'o':   sr->flag |= ALLOWSIGN;
                    worked = rd_int(p, 8, sr);
                    break;
                    /* pointers are displayed in hex, but h,l,L ignored */
        case 'p':   sr->flag &= ~(LONG|SHORT);
                    worked = rd_int(p, 16, sr);
                    break;
        case 's':   worked = rd_string(p, sr);
                    break;
        case 'u':   worked = rd_int(p, 10, sr);
                    break;
        case 'x':
        case 'X':   sr->flag |= ALLOWSIGN;
                    worked = rd_int(p, 16, sr);
                    break;
        case '[':   {   int negated = 0, i, charmap[8];
                        if ((fch = *fmt++) == '^') negated = 1, fch = *fmt++;
                        for (i=0; i<8; i++) charmap[i] = 0;
                        /* the 'do' next allows special treatment of %[]})] */
                        do { if (fch==0) return cnt;  /* %[... unterminated */
                             charmap[fch/32] |= 1<<(fch%32);
                        } while ((fch = *fmt++) != ']');
                        if (negated) for (i=0; i<8; i++)
                            charmap[i] = ~charmap[i];
                        worked = rd_string_map(p, charmap, sr);
                    }
                    break;
                }
                if (worked < 0)                  /* conversion failed       */
                     return worked == CVTEOF && cnt == 0 ? EOF : cnt;
                if (!(flag & NOSTORE)) cnt++;    /* another assignment made */
                charcount += worked;             /* chars were read anyway  */
                continue;
            }
        }
    }
}

#endif

#if defined scanf_c || defined SHARED_C_LIBRARY

int scanf(const char *fmt, ...)
{
    ScanfReadRec sr;
    int n;
    va_start(sr.argv, fmt);
    sr.getch = fgetc; sr.backspace = __backspace;
    n = __vfscanf(stdin, fmt, &sr);
    va_end(sr.argv);
    return n;
}

#endif

#if defined fscanf_c || defined SHARED_C_LIBRARY

int fscanf(FILE *fp, const char *fmt, ...)
{
    ScanfReadRec sr;
    int n;
    va_start(sr.argv, fmt);
    sr.getch = fgetc; sr.backspace = __backspace;
    n = __vfscanf(fp, fmt, &sr);
    va_end(sr.argv);
    return n;
}

#endif

typedef struct {
    char const *ptr;
    size_t n;
    char const *start;
    int hadeof;
} StringInputFile;

int _sgetc(FILE *f);
int _sbackspace(FILE *f);

#if defined _sgetc_c || defined SHARED_C_LIBRARY

int _sgetc(FILE *fp) {
    StringInputFile *sf = (StringInputFile *)fp;
    size_t n = sf->n;
    if (n != 0) {
        char const *ip = sf->ptr;
        int r = *ip++;
        if (r != 0) {
            sf->ptr = ip;
            sf->n = n-1;
            return r;
        }
    }
    sf->hadeof = 1;
    return EOF;
}

int _sbackspace(FILE *fp) {
    StringInputFile *sf = (StringInputFile *)fp;
    size_t n = sf->n;
    if (n != 0) {
        char const *ip = sf->ptr;
        if (!sf->hadeof && ip != sf->start) {
          /* EOF is sticky; can't backspace past start of string */
          sf->ptr = ip-1;
          sf->n = n+1;
          return 0;
        }
    }
    return EOF;
}

#endif

#if defined sscanf_c || defined SHARED_C_LIBRARY

int sscanf(const char *buff, const char *fmt, ...)
{
    ScanfReadRec sr;
    StringInputFile sf;
    int n;
    va_start(sr.argv, fmt);
    sf.start = sf.ptr = buff;
    sf.n = (size_t)-1;
    sf.hadeof = 0;
    sr.getch = _sgetc; sr.backspace = _sbackspace;
    n = __vfscanf((FILE *)&sf, fmt, &sr);
    va_end(sr.argv);
    return n;
}

#endif

#if defined _strtoul_c || defined SHARED_C_LIBRARY

unsigned long int _strtoul(const char *nsptr, char **endptr, int base)
{
    const unsigned char *nptr = (const unsigned char *)nsptr;  /* see scanf */
    int c, ok = 0, overflowed = 0;
    while ((c = *nptr++)!=0 && isspace(c));
    if (c=='0')
    {   ok = 1;
        c = *nptr++;
        if (c=='x' || c=='X')
        {   if (base==0 || base==16)
            {   ok = 0;
                base = 16;
                c = *nptr++;
            }
        }
        else if (base==0) base = 8;
    }
    if (base==0) base = 10;
    {   unsigned long dhigh = 0, dlow = 0;
        int digit;
        while ((digit = _chval(c,base)) >= 0)
        {   ok = 1;
            dlow = base * dlow + digit;
            dhigh = base * dhigh + (dlow >> 16);
            dlow &= 0xffff;
            if (dhigh >= 0x10000) overflowed = 1;
            c = *nptr++;
        }
        if (endptr) *endptr = ok ? (char *)nptr-1 : (char *)nsptr;
                                                /* extra result */
        return overflowed ? (set_errno(ERANGE), ULONG_MAX)
                          : (dhigh << 16) | dlow;
    }
}

#endif

#if defined strtol_c || defined SHARED_C_LIBRARY

long int strtol(const char *nsptr, char **endptr, int base)
{
/* The specification in the ANSI information bulletin upsets me here:    */
/* strtol is of type long int, and 'if the correct value would cause     */
/* overflow LONG_MAX or LONG_MIN is returned'. Thus for hex input the    */
/* string 0x80000000 will be considered to have overflowed, and so will  */
/* be returned as LONG_MAX.                                              */
/* These days one should use strtoul for unsigned values, so some of     */
/* my worries go away.                                                   */

/* This code is NOT shared with the %i conversion in scanf for several   */
/* reasons: (a) here I deal with overflow in a silly way as noted above, */
/* (b) in scanf I have to deal with field width limitations, which does  */
/* not fit in neatly here (c) this functions scans an array of char,     */
/* while scanf reads from a stream - fudging these together seems too    */
/* much work, (d) here I have the option of specifying the radix, while  */
/* in scanf there seems to be no provision for that. Ah well!            */

    const unsigned char *nptr = (const unsigned char *)nsptr;  /* see scanf */
    int flag = 0, c;
    while ((c = *nptr++)!=0 && isspace(c))
        continue;
    switch (c)
    {
case '-': flag |= NUMNEG;
          /* drop through */
case '+': break;
default:  nptr--;
          break;
    }
    {   char *endp;
        unsigned long ud = _strtoul((char *)nptr, &endp, base);
        if (endptr) *endptr = endp==(char *)nptr ? (char *)nsptr : endp;
/* The following lines depend on the fact that unsigned->int casts and   */
/* unary '-' cannot cause arithmetic traps.  Recode to avoid this?       */
        if (flag & NUMNEG)
            return (-(long)ud <= 0) ? -(long)ud : (set_errno(ERANGE), LONG_MIN);
        else
            return (+(long)ud >= 0) ? +(long)ud : (set_errno(ERANGE), LONG_MAX);
    }
}

#endif

#if defined strtoul_c || defined SHARED_C_LIBRARY

unsigned long int strtoul(const char *nsptr, char **endptr, int base)
/*
 * I don't think the way negation is treated in this is right, but its closer
 *  than before
 */
{
    const unsigned char *nptr = (const unsigned char *)nsptr;  /* see scanf */
    int flag = 0, c;
    int errno_saved = read_errno();
    while ((c = *nptr++)!=0 && isspace(c))
        continue;
    switch (c)
    {
case '-': flag |= NUMNEG;
          /* drop through */
case '+': break;
default:  nptr--;
          break;
    }
    set_errno(0);
    {   char *endp;
        unsigned long ud = _strtoul((char *)nptr, &endp, base);
        if (endptr) *endptr = endp==(char *)nptr ? (char *)nsptr : endp;
/* ??? The following lines depend on the fact that unsigned->int casts and   */
/* ??? unary '-' cannot cause arithmetic traps.  Recode to avoid this?       */
        if (read_errno() == ERANGE) return ud;
        set_errno(errno_saved);
        if (flag & NUMNEG)
            return (ud <= LONG_MAX) ? -(unsigned long)ud
                                    : (set_errno(ERANGE), ULONG_MAX);
        else
            return +(unsigned long)ud;
    }
}

#endif

#if defined strtod_c || defined SHARED_C_LIBRARY

int _sscanf(StringInputFile *sf, char const *fmt, ...)
{
    ScanfReadRec sr;
    int n;
    va_start(sr.argv, fmt);
    sf->hadeof = 0;
    sr.getch = _sgetc; sr.backspace = _sbackspace;
    n = __vfscanf((FILE *)sf, fmt, &sr);
    va_end(sr.argv);
    return n;
}

double strtod(const char *nptr, char **endptr)
{
    int ch;
    int flag = 0;
    const char *sptr = nptr;

/* Parse the string to determine if it is an fp number (allow "100exx"). */
    while (isspace(ch = *sptr++));
    switch (ch)
    {
case '-':
case '+':   ch = *sptr++;
            break;
    }
    while (1)
    {   if (ch==DecimalPoint && !(flag & DOTSEEN))
            flag |= DOTSEEN;
        else if (isdigit(ch))
            flag |= NUMOK;
        else break;
        ch = *sptr++;
    }
    if ((ch == 'e' || ch == 'E') && (flag & NUMOK))
    {
        flag &= ~NUMOK;
        switch (ch = *sptr++)
        {
    case '-':
    case '+':   ch = *sptr++;
                break;
        }
        while (isdigit(ch))
        {   flag |= NUMOK;
            ch = *sptr++;
        }
/* Here I handle the case of "100exx" by backspacing 2 chars.            */
        if (!(flag & NUMOK))
        {   sptr -= 2;
            ch = *sptr++;  /* reload ch to be clean (currently not used) */
            flag |= NUMOK;
        }
    }

    sptr--; /* now points to 1st invalid character */

    if (endptr != NULL)
        *endptr = (flag & NUMOK) ? (char *)sptr : (char *)nptr;

    if (flag & NUMOK)
    {
        double d;
        StringInputFile sf;
        sf.start = sf.ptr = nptr;
        sf.n = (size_t)(sptr - nptr);
        _sscanf(&sf, "%lf", &d);
        return d;
    }
    return 0.0;
}

#endif

#if defined atoi_c || defined SHARED_C_LIBRARY

int atoi(const char *nptr)
{
    int save_errno = read_errno();
    long int res = strtol(nptr, (char **)NULL, 10);
    set_errno(save_errno);
    return (int)res;
}

#endif

#if defined atol_c || defined SHARED_C_LIBRARY

long int atol(const char *nptr)
{
    int save_errno = read_errno();
    long int res = strtol(nptr, (char **)NULL, 10);
    set_errno(save_errno);
    return res;
}

#endif

#if defined atof_c || defined SHARED_C_LIBRARY

double atof(const char *nptr)
{
    int save_errno = errno;
    double res = strtod(nptr, (char **)NULL);
    errno = save_errno;
    return res;
}

#endif

/* end of scanf.c */
