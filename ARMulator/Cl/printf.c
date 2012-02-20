/* printf.c: ANSI draft (X3J11 Oct 86) part of section 4.9 code */
/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

/*
 * RCS $Revision: 1.23 $
 * Checkin $Date: 1997/04/07 12:22:05 $ 0
 * Revising $Author: hmeeking $
 */

/* printf and its friends return the number of characters planted. In    */
/* the case of sprintf this does not include the terminating '\0'.       */
/* Consider using ftell instead of charcount in printf (see scanf).      */

#include "hostsys.h"       /* which requires a FILEHANDLE to be defined  */

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
#include "externs.h"
#include "interns.h"
/* #include "ioguts.h" */

#if defined __SOFTFP__ && (defined __arm || defined __thumb)
/* use extended precision routines to obtain full accuracy */
#  include "mathlib.h"
#endif

typedef struct {
  int flags;
  char *prefix;
  int precision;
  int before_dot;
  int after_dot;
  double d;
} fp_print_rec;

typedef int fp_print(int ch, char buff[], fp_print_rec *p);

extern int _no_fp_display(int ch, char buff[], fp_print_rec *p);

extern int _fp_display(int ch, char buff[], fp_print_rec *p);

typedef struct {
  fp_print_rec fpr;
  fp_print *fn;
  int (*putc)(int, FILE *);
  int (*ferror)(FILE *);
  char *hextab;
  int width;
  unsigned long long llval;
} printf_display_rec;

int __vfprintf(FILE *p, const char *fmt, va_list args, printf_display_rec *dr);

#define _LJUSTIFY         0x1
#define _SIGNED           0x2
#define _BLANKER          0x4
#define _VARIANT          0x8
#define _PRECGIVEN       0x10
#define _LONGSPEC        0x20
#define _LLSPEC          0x40
#define _SHORTSPEC       0x80
#define _PADZERO        0x100    /* *** DEPRECATED FEATURE *** */
#define _FPCONV         0x200

#if defined __vfprintf_c || defined SHARED_C_LIBRARY

#define intofdigit(x) ((x)-'0')
#define xputc(dr, ch, f) (dr->putc(ch, f))

#define pr_padding(dr, ch, n, f)  while (--n>=0) charcount++, xputc(dr, ch, f);

#define pre_padding(dr, f)                                                \
        if (!(flags&_LJUSTIFY))                                           \
        {   char padchar = flags & _PADZERO ? '0' : ' ';                  \
            pr_padding(dr, padchar, width, f); }

#define post_padding(dr, f)                                               \
        if (flags&_LJUSTIFY)                                              \
        {   pr_padding(dr, ' ', width, f); }

#ifdef never /* do it this way? */
static int pr_num(unsigned int v, int flags, char *prefix,
                   int width, int precision, FILE *p)
{}
#endif

static int printf_display(FILE *p, int ch, printf_display_rec *dr, unsigned int v)
{
    int len = 0, charcount = 0;
    char buff[32];       /* used to accumulate value to print    */
    int flags = dr->fpr.flags;
    dr->fpr.before_dot = -1, dr->fpr.after_dot = -1;
    if (!(dr->fpr.flags & _FPCONV+_PRECGIVEN)) dr->fpr.precision = 1;
    switch (ch)
    {
    case 'p':
    case 'X':
    case 'x':   if (flags & _LLSPEC) {
                    unsigned long long w = dr->llval;
                    while (w != 0)
                    {   buff[len++] = dr->hextab[(int)w & 0xf];
                        w = w >> 4;
                    }
                } else {
                    while(v != 0)
                    {   buff[len++] = dr->hextab[v & 0xf];
                        v = v >> 4;
                    }
                }
                break;
    case 'o':   if (flags & _LLSPEC) {
                    unsigned long long w = dr->llval;
                    while (w != 0)
                    {   buff[len++] = dr->hextab[(int)w & 7];
                        w = w >> 3;
                    }
                } else {
                    while(v != 0)
                    {   buff[len++] = '0' + (v & 7);
                        v = v >> 3;
                    }
                }
/* Fix for X3J11 Interpretation #21 [printf("%#.4o,345) => "0531"]       */
/* The precision is increased only if necessary to obtain 0 as 1st digit */
                if (flags & _VARIANT) dr->fpr.precision--;
                break;
    case 'u':
    case 'i':
    case 'd':   if (flags & _LLSPEC) {
                    unsigned long long w = dr->llval;
                    while (w != 0)
                    {   buff[len++] = '0' + (w % 10);
                        w = w / 10;
                    }
                } else {
                    while(v != 0)
                    {   buff[len++] = '0' + (v % 10);
                        v = v / 10;
                    }
                }
                break;

#ifndef NO_FLOATING_POINT
    case 'f':
    case 'g':   case 'G':
    case 'e':   case 'E':
                len = dr->fn(ch, buff, &dr->fpr);
                break;

#else
/* If floating point is not supported I display ALL %e, %f and %g        */
/* items as 0.0                                                          */
    default:    buff[0] = '0';
                buff[1] = DecimalPoint;
                buff[2] = '0';
                len = 3;
                break;
#endif
    }
/* now work out how many leading '0's are needed for precision specifier */
/* _FPCONV is the case of FP printing in which case extra digits to make */
/* up the precision come within the number as marked by characters '<'   */
/* and '>' in the buffer.                                                */
    {   int precision;
        int width = dr->width;
        if (flags & _FPCONV)
        {   precision = 0;
            if (dr->fpr.before_dot>0) precision = dr->fpr.before_dot-1;
            if (dr->fpr.after_dot>0) precision += dr->fpr.after_dot-1;
        } else {
            precision = dr->fpr.precision;
            if ((precision -= len)<0) precision = 0;
        }
/* and how much padding is needed */
        width -= (precision + len + strlen(dr->fpr.prefix));

/* AM: ANSI appear (Oct 86) to suggest that the padding (even if with '0') */
/*     occurs before the possible sign!  Treat this as fatuous for now.    */
        if (!(flags & _PADZERO)) pre_padding(dr, p);

        {   int c;                                      /* prefix    */
            char *prefix = dr->fpr.prefix;
            for (; (c=*prefix++)!=0; charcount++) xputc(dr, c, p);
        }

        pre_padding(dr, p);

/* floating point numbers are in buff[] the normal way around, while     */
/* integers have been pushed in with the digits in reverse order.        */
        if (flags & _FPCONV)
        {   int i, c;
            for (i = 0; i<len; i++)
            {   switch (c = buff[i])
                {
        case '<':   pr_padding(dr, '0', dr->fpr.before_dot, p);
                    break;
        case '>':   pr_padding(dr, '0', dr->fpr.after_dot, p);
                    break;
        default:    xputc(dr, c, p);
                    charcount++;
                    break;
                }
            }
        }
        else
        {   pr_padding(dr, '0', precision, p);
            charcount += len;
            while (len-- > 0) xputc(dr, buff[len], p);
        }
/* By here if the padding has already been printed width will be zero    */
        post_padding(dr, p);
        return charcount;
    }
}

int __vfprintf(FILE *p, const char *fmt, va_list args, printf_display_rec *dr)
/* ACN: I apologize for this function - it seems long and ugly. Some of  */
/*      that is dealing with all the jolly flag options available with   */
/*      printf, and rather a lot more is a cautious floating point print */
/*      package that takes great care to avoid the corruption of its     */
/*      input by rounding, and to generate consistent decimal versions   */
/*      of all possible values in all possible formats.                  */
{
    int ch, charcount = 0;
    while ((ch = *fmt++) != 0)
    {   if (ch != '%') { xputc(dr, ch, p); charcount++; }
        else
        {   int flags = 0, width = 0;
            unsigned int v = 0;
            dr->fpr.precision = 0;
/* This decodes all the nasty flags and options associated with an       */
/* entry in the format list. For some entries many of these options      */
/* will be useless, but I parse them all the same.                       */
            for (;;)
            {   switch (ch = *fmt++)
                {
/* '-'  Left justify converted flag. Only relevant if width specified    */
/* explicitly and converted value is too short to fill it.               */
        case '-':   flags = _LJUSTIFY | (flags & ~_PADZERO);
                    continue;

/* '+'  Always print either '+' or '-' at start of numbers.              */
        case '+':   flags |= _SIGNED;
                    continue;

/* ' '  Print either ' ' or '-' at start of numbers.                     */
        case ' ':   flags |= _BLANKER;
                    continue;

/* '#'  Variant on main print routine (effect varies across different    */
/*      styles, but for instance %#x puts 0x on the front of displayed   */
/*      numbers.                                                         */
        case '#':   flags |= _VARIANT;
                    continue;

/* '0'  Leading blanks are printed as zeros                              */
/*        This is a *** DEPRECATED FEATURE *** (precision subsumes)      */
        case '0':   flags |= _PADZERO;
                    continue;

        default:    break;
                }
                break;
            }
            /* now look for 'width' spec */
            {   int t = 0;
                if (ch=='*')
                {   t = va_arg(args, int);
/* If a negative width is passed as an argument I take its absolute      */
/* value and use the negativeness to indicate the presence of the '-'    */
/* flag (left justification). If '-' was already specified I lose it.    */
                    if (t<0)
                    {   t = - t;
                        flags ^= _LJUSTIFY;
                    }
                    ch = *fmt++;
                }
                else
                {   while (isdigit(ch))
                    {   t = t*10 + intofdigit(ch);
                        ch = *fmt++;
                    }
                }
                width = t>=0 ? t : 0;                 /* disallow -ve arg */
            }
            if (ch == '.')                            /* precision spec */
            {   int t = 0;
                ch = *fmt++;
                if (ch=='*')
                {   t = va_arg(args, int);
                    ch = *fmt++;
                }
                else while (isdigit(ch))
                {   t = t*10 + intofdigit(ch);
                    ch = *fmt++;
                }
                if (t >= 0) flags |= _PRECGIVEN, dr->fpr.precision = t;
            }
            if (ch=='l' || ch=='L')
/* 'l'  Indicate that a numeric argument is 'long'. Here int and long    */
/*      are the same (32 bits) and so I can ignore this flag!            */
/* 'L'  Marks floating arguments as being of type long double. Here this */
/*      is the same as just double, and so I can ignore the flag.        */
            {   ch = *fmt++;
                if (ch == 'l' || ch == 'L') {
                    flags |= _LLSPEC;
                    ch = *fmt++;
                } else
                    flags |= _LONGSPEC;
            }
            else if (ch=='h')
/* 'h' Indicates that an integer value is to be treated as short.        */
            {   flags |= _SHORTSPEC;
                ch = *fmt++;
            }

/* Now the options have been decoded - I can process the main dispatch   */
            switch (ch)
            {

/* %c causes a single character to be fetched from the argument list     */
/* and printed. This is subject to padding.                              */
    case 'c':   ch = va_arg(args, int);
                /* drop through */

/* %? where ? is some character not properly defined as a command char   */
/* for printf causes ? to be displayed with padding and field widths     */
/* as specified by the various modifers. %% is handled by this general   */
/* mechanism.                                                            */
    default:    width--;                        /* char width is 1       */
                pre_padding(dr, p);
                xputc(dr, ch, p);
                charcount++;
                post_padding(dr, p);
                continue;

/* If a '%' occurs at the end of a format string (possibly with a few    */
/* width specifiers and qualifiers after it) I end up here with a '\0'   */
/* in my hand. Unless I do something special the fact that the format    */
/* string terminated gets lost...                                        */
    case 0:     fmt--;
                continue;

/* %n assigns the number of chars printed so far to the next arg (which  */
/* is expected to be of type (int *).                                    */
    case 'n':   if (flags & _SHORTSPEC)
                    *va_arg(args, short *) = (short)charcount;
                else if (flags & _LLSPEC)
                    *va_arg(args, long long *) = charcount;
                else if (flags & _LONGSPEC)
                    *va_arg(args, long *) = charcount;
                else
                    *va_arg(args, int *) = charcount;
                continue;

/* %s prints a string. If a precision is given it can limit the number   */
/* of characters taken from the string, and padding and justification    */
/* behave as usual.                                                      */
    case 's':   {   char *str = va_arg(args, char *);
                    int i, n;
                    if (flags & _PRECGIVEN) {
                      int precision = dr->fpr.precision;
                      for (n = 0; n < precision && str[n] != 0; n++) continue;
                    } else
                      n = strlen(str);
                    width -= n;
                    pre_padding(dr, p);
                    for (i=0; i<n; i++) xputc(dr, str[i], p);
                    charcount += n;
                    post_padding(dr, p);
                }
                continue;

/* %x prints in hexadecimal. %X does the same, but uses upper case       */
/* when printing things that are not (decimal) digits.                   */
/* I can share some messy decoding here with the code that deals with    */
/* octal and decimal output via %o and %d.                               */
    case 'X':   if (flags & _LLSPEC)
                    dr->llval = va_arg(args, long long);
                else
                    v = va_arg(args, int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                dr->hextab = "0123456789ABCDEF";
                dr->fpr.prefix = (flags&_VARIANT) ? "0X" : "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'x':   if (flags & _LLSPEC)
                    dr->llval = va_arg(args, long long);
                else
                    v = va_arg(args, int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                dr->hextab = "0123456789abcdef";
                dr->fpr.prefix = (flags&_VARIANT) ? "0x" : "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

/* %p is for printing a pointer - I print it as a hex number with the    */
/* precision always forced to 8.                                         */
    case 'p':   v = (unsigned int)va_arg(args, void *);
                dr->hextab = "0123456789abcdef";
                dr->fpr.prefix = (flags&_VARIANT) ? "@" : "";
                dr->fpr.precision = 8;
                flags |= _PRECGIVEN;
                break;

    case 'o':   if (flags & _LLSPEC)
                    dr->llval = va_arg(args, long long);
                else
                    v = va_arg(args, int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                dr->fpr.prefix = (flags&_VARIANT) ? "0" : "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'u':   if (flags & _LLSPEC)
                    dr->llval = va_arg(args, long long);
                else
                    v = va_arg(args, unsigned int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                dr->fpr.prefix = "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'i':
    case 'd':   {   int w;
                    if (flags & _LLSPEC) {
                        dr->llval = va_arg(args, long long);
                        if ((long long)dr->llval < 0) {
                            dr->llval = -dr->llval;
                            w = -1;
                        } else
                            w = 0;
                    } else {
                        w = va_arg(args, int);
                        if (flags & _SHORTSPEC) w = (signed short)w;
                        v = (w < 0) ? -w : w;
                    }
                    dr->fpr.prefix = (w < 0) ? "-" :
                           (flags & _SIGNED) ? "+" :
                          (flags & _BLANKER) ? " " : "";
                }
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'f':
    case 'e':
    case 'E':
    case 'g':
    case 'G':   flags |= _FPCONV;
                if (!(flags & _PRECGIVEN)) dr->fpr.precision = 6;
#ifndef NO_FLOATING_POINT
                dr->fpr.d = va_arg(args, double);
                /* technically, for the call to printf_display() below to  */
                /* be legal and not reference an undefined variable we     */
                /* need to do the following (overwritten in fp_display_fn) */
                /* (It also stops dataflow analysis (-fa) complaining!)    */
                dr->fpr.prefix = 0, v = 0;
#else  /* NO_FLOATING_POINT */
                {   int w = va_arg(args, int);
                    w = va_arg(args, int);
/* If the pre-processor symbol FLOATING_POINT is not set I assume that   */
/* floating point is not available, and so support %e, %f and %g with    */
/* a fragment of code that skips over the relevant argument.             */
/* I also assume that a double takes two int-sized arg positions.        */
                    dr->fpr.prefix = (flags&_SIGNED) ? "+" :
                                     (flags&_BLANKER) ? " " : "";
                }
#endif /* NO_FLOATING_POINT */
                break;

            }
            dr->width = width;
            dr->fpr.flags = flags;
            charcount += printf_display(p, ch, dr, v);
            continue;
        }
    }
    return dr->ferror(p) ? EOF : charcount;
}


#endif  /* __vfprintf_c */

#if defined _fp_disp_c || defined SHARED_C_LIBRARY

#include <math.h>

#ifdef IEEE
#define FLOATING_WIDTH 17
#else
#define FLOATING_WIDTH 18       /* upper bound for sensible precision    */
#endif

static int fp_round(char buff[], int len)
/* round (char form of) FP number - return 1 if carry, 0 otherwise       */
/* Note that 'len' should be <= 20 - see fp_digits()                     */
/* The caller ensures that buff[0] is always '0' so that carry is simple */
/* However, beware that this routine does not re-ensure this if carry!!  */
{   int ch;
    char *p = &buff[len];
    if ((ch = *p)==0) return 0;                      /* at end of string */
    if (ch < '5') return 0;                          /* round downwards  */
    if (ch == '5')                                   /* the dodgy case   */
    {   char *p1;
        for (p1 = p; (ch = *++p1)=='0';);
        if (ch==0) return 0;                         /* .5 ulp exactly   */
    }
    for (;;)
    {   ch = *--p;
        if (ch=='9') *p = '0';
        else
        {   *p = ch + 1;
            break;
        }
    }
    if (buff[0]!='0')           /* caused by rounding                    */
    {   int w;                  /* renormalize the number                */
        for (w=len; w>=0; w--) buff[w+1] = buff[w];
        return 1;
    }
    return 0;
}

typedef enum { fp_ord, fp_nan, fp_inf } fp_type;

#ifdef HOST_HAS_BCD_FLT

static int fp_digits(char *buff, double d)
/* This routine turns a 'double' into a character string representation of  */
/* its mantissa and returns the exponent after converting to base 10.       */
/* It guarantees that buff[0] = '0' to ease problems connected with         */
/* rounding and the like.  See also comment at first call.                  */
/* Use FPE2 convert-to-packed-decimal feature to do most of the work        */
/* The sign of d is returned in the LSB of x, and x has to be halved to     */
/* obtain the 'proper' value it needs.                                      */
{
    unsigned int a[3], w, d0, d1, d2, d3;
    int x, i;
    _stfp(d, a);
    w = a[0];
/* I allow for a four digit exponent even though sensible values can     */
/* only extend to 3 digits. I call this caution!                         */
    if ((w & 0x0ffff000) == 0x0ffff000)
    {   x = 999;    /* Infinity will print as 1.0e999 here */
                    /* as will NaNs                        */
        for (i = 0; i<20; i++) buff[i] = '0';
        buff[1] = '1';
    }
    else
    {   d0 = (w>>24) & 0xf;
        d1 = (w>>20) & 0xf;
        d2 = (w>>16) & 0xf;
        d3 = (w>>12) & 0xf;
        x = ((d0*10 + d1)*10 + d2)*10 + d3;
        if (w & 0x40000000) x = -x;
        buff[0] = '0';
        for (i = 1; i<4; i++) buff[i] = '0' + ((w>>(12-4*i)) & 0xf);
        w = a[1];
        for (i = 4; i<12; i++) buff[i] = '0' + ((w>>(44-4*i)) & 0xf);
        w = a[2];
        for (i = 12; i<20; i++) buff[i] = '0' + ((w>>(76-4*i)) & 0xf);
    }
    buff[20] = 0;
    return x;
}

#else /* HOST_HAS_BCD_FLT */

static void pr_dec(int d, char *p, int n)
                                /* print d in decimal, field width n     */
{                               /* store result at p. arg small & +ve.   */
    while ((n--)>0)
    {   *p-- = '0' + d % 10;
        d = d / 10;
    }
}

static int fp_digits(char *buff, double d)
/* This routine turns a 'double' into a character string representation of  */
/* its mantissa and returns the exponent after converting to base 10.       */
/* For this we use one-and-a-half precision done by steam                   */
/* It guarantees that buff[0] = '0' to ease problems connected with         */
/* rounding and the like.  See also comment at first call.                  */
{   int hi, mid, lo, dx;
    if (d < 0.0) d = -d;
    if (d==0.0) { hi = mid = lo = 0; dx = -5; }
    else
    {   double d1, d2, d2low, d3, d3low, scale;
        int w, bx;
        d1 = frexp(d, &bx);     /* exponent & mantissa   */
        /* fraction d1 is in range 0.5 to 1.0            */
        /* remember log_10(2) = 0.3010!                  */
        dx = (301*bx - 5500)/1000;   /* decimal exponent */
        scale = ldexp(1.0, dx-bx);
        w = dx;

#if defined __SOFTFP__ && (defined __arm || defined __thumb)
        /* Do this in extended precision for accuracy */
      {
        ip_number d3_e,scale_e;

        scale_e=_d2e(scale);

        if (w<0) {
          w = -w;
          /* 0.2 */
          d3_e.word.hi = 0x3ffc; d3_e.word.lo = 0xcccccccc; d3_e.word.lo2 = 0xcccccccd;
        } else {
          /* 5.0 */
          d3_e.word.hi = 0x4001; d3_e.word.lo = 0xa0000000; d3_e.word.lo2 = 0x0;
        }

        if (w!=0) for (;;) {    /* scale *= 5**dx        */
          if ((w & 1)!=0) {
            scale_e=_emul(d3_e,&scale_e);
            if (w==1) break;
          }
          d3_e=_esquare(d3_e);
          w=w>>1;
        }
        scale=_e2d(scale_e);
      }
#else
        if (w < 0) { w = -w; d3 = 0.2; }
        else d3 = 5.0;

        if (w!=0) for (;;)      /* scale *= 5**dx        */
        {   if((w&1)!=0)
            {   scale *= d3;
                if (w==1) break;
            }
            d3 *= d3;
            w = w >> 1;
        }
#endif
        d2 = d1/scale;

/* the initial value selected for dx was computed on the basis of the    */
/* binary exponent in the argument value - now I refine dx. If the value */
/* produced to start with was accurate enough I will hardly have to do   */
/* any work here.                                                        */
        while (d2 < 100000.0)
        {   d2 *= 10.0;
            dx -= 1;
            scale /= 10.0;
        }
        while (d2 >= 1000000.0)
        {   d2 /= 10.0;
            dx += 1;
            scale *= 10.0;
        }
        hi = (int) d2;
        for (;;)               /* loop to get hi correct                 */
        {   d2 = ldexp((double) hi, dx-bx);
            /* at worst 24 bits in d2 here                               */
            /* even with IBM fp numbers there is no accuracy lost        */
            d2low = 0.0;
            w = dx;
            if (w<0)
            {   w = -w;
/* the code here needs to set (d3, d3low) to a one-and-a-half precision  */
/* version of the constant 0.2.                                          */
                d3 = 0.2;
                d3low = 0.0;
                _fp_normalize(d3, d3low);
                d3low = (1.0 - 5.0*d3)/5.0;
            }
            else
            {   d3 = 5.0;
                d3low = 0.0;
            }
/* Now I want to compute d2 = d2 * d3**dx in extra precision arithmetic  */
            if (w!=0) for (;;)
            {   if ((w&1)!=0)
                {   d2low = d2*d3low + d2low*(d3 + d3low);
                    d2 *= d3;
                    _fp_normalize(d2, d2low);
                    if (w==1) break;
                }
                d3low *= (2.0*d3 + d3low);
                d3 *= d3;
                _fp_normalize(d3, d3low);
                w = w>>1;
            }
            if (d2<=d1) break;
            hi -= 1;          /* hardly ever happens */
        }

        d1 -= d2;
              /* for this to be accurate d2 MUST be less */
              /* than d1 so that d1 does not get shifted */
              /* prior to the subtraction.               */
        d1 -= d2low;
        d1 /= scale;
/* Now d1 is a respectably accurate approximation for (d - (double)hi)   */
/* scaled by 10**dx                                                      */

        d1 *= 1000000.0;
        mid = (int) d1;
        d1 = 1000000.0 * (d1 - (double) mid);
        lo = (int) d1;

/* Now some postnormalization on the integer results                     */
/* If I do things this way the code will work if (int) d rounds or       */
/* truncates.                                                            */
        while (lo<0) { lo += 1000000; mid -= 1; }
        while (lo>=1000000) { lo -= 1000000; mid += 1; }
        while (mid<0) { mid += 1000000; hi -= 1; }
        while (mid>=1000000) { mid -= 1000000; hi += 1; }
        if (hi<100000)
        {
            hi = 10*hi + mid / 100000;
            mid = 10*(mid % 100000) + lo / 100000;
            lo = 10*(lo % 100000);
            dx -= 1;
        }
        else if (hi >= 1000000)
        {   mid += 1000000*(hi % 10);
            hi = hi / 10;
            lo += 1000000*(mid % 10);
            mid = mid / 10;
            lo = (lo + 5)/10;    /* pretence at rounding */
            dx += 1;
        }
    }

/* Now my result is in three 6-digit chunks (hi, mid, lo)                */
/* The number of characters put in the buffer here MUST agree with       */
/* FLOATING_PRECISION. This version is for FLOATING_PRECISION = 18.      */
    buff[0] = '0';
    pr_dec(hi,  &buff[6], 6);
    pr_dec(mid, &buff[12], 6);
    pr_dec(lo,  &buff[18], 6);
    buff[19] = '0';
    buff[20] = 0;
    return dx+5;
}

#endif /* HOST_HAS_BCD_FLT */

static fp_type fp_classify(double *d, int *sign) {
    unsigned *w = (unsigned *)d;
    *sign = (w[0] & 0x80000000) ? 1 : 0;
    return (w[0] & 0x7ff00000) != 0x7ff00000 ? fp_ord :
           (w[0] & 0x000fffff) || w[1] ? fp_nan : fp_inf;
}

static int fp_addexp(char *buff, int len, int dx, int ch)
{
    buff[len++] = ch;
    if (dx<0) { dx = -dx; buff[len++] = '-'; }
    else buff[len++] = '+';
    if (dx >= 1000)
    {
        buff[len++] = '0' + dx / 1000;
        dx = dx % 1000;
    }
    if (dx >= 100)
    {
        buff[len++] = '0' + dx / 100;
        dx = dx % 100;
    }
    buff[len++] = '0' + dx / 10;
    buff[len++] = '0' + dx % 10;
    return len;
}

#define fp_insert_(buff, pos, c)                    \
    {   int w;                                      \
        for (w=0; w<=pos; w++) buff[w] = buff[w+1]; \
        buff[pos+1] = c; }

int _fp_display(int ch, char buff[], fp_print_rec *p)
{   int flags = p->flags;
    {   int sign;
        fp_type valtype = fp_classify(&p->d, &sign);
        p->prefix = sign ? "-" :
                    (flags&_SIGNED) ? "+" :
                    (flags&_BLANKER) ? " " : "";
        if (valtype != fp_ord) {
            char *s = valtype == fp_nan ? "NaN" : "Inf";
            buff[0] = s[0]; buff[1] = s[1]; buff[2] = s[2]; buff[3] = 0;
            p->flags = flags & ~_PADZERO;
            return 3;
        }
    }
    {   int len = 0;
        double d = p->d;
        switch (ch)
        {
/* The following code places characters in the buffer buff[]             */
/* to print the floating point number given as d.                        */
/* It is given flags that indicate what format is required and how       */
/* many digits precision are needed.                                     */
/* Floating point values are ALWAYS converted into 18 decimal digits     */
/* (the largest number possible reasonable) to start with, and rounding  */
/* is then performed on this character representation. This is intended  */
/* to avoid all possibility of boundary effects when numbers like .9999  */
/* are being displayed.                                                  */
    case 'f':
                {   int dx = fp_digits(buff, d);
                    if (dx<0)
                    /* insert leading zeros */
                    {   dx = -dx;
                        if (dx>p->precision+1)
                        {   len = 0;       /* prints as zero */
                            buff[len++] = '0';
                            buff[len++] = DecimalPoint;
                            p->after_dot = p->precision;
                        }
                        else
                        {   len = p->precision - dx + 2;
                            if (len > FLOATING_WIDTH + 1)
                            {   p->after_dot = len - (FLOATING_WIDTH + 2);
                                len = FLOATING_WIDTH+2;
                            }
                            if (fp_round(buff, len))
                                dx--, len++; /* dx-- because of negation */
/* unfortunately we may have dx=0 now because of the rounding            */
                            if (dx==0)
                            {   buff[0] = buff[1];
                                buff[1] = DecimalPoint;
                            }
                            else if (dx==1)
                            {   int w;
                                for(w=len; w>0; w--) buff[w+1] = buff[w];
                                len += 1;
                                buff[0] = '0';
                                buff[1] = DecimalPoint;
                            }
                            else
                            {   int w;
                                for(w=len; w>0; w--) buff[w+2] = buff[w];
                                len += 2;
                                buff[0] = '0';
                                buff[1] = DecimalPoint;
                                buff[2] = '<';
                                p->before_dot = dx - 1;
                            }
                        }
                        if (p->after_dot > 0) buff[len++] = '>';
                    }
                    else /* dx >= 0 */
                    {   len = dx + p->precision + 2;
                        if (len > FLOATING_WIDTH+1)
                        {   len = FLOATING_WIDTH+2;
/* Seemingly endless fun here making sure that the number is printed     */
/* without truncation or loss even if it is very big & hence needs very  */
/* many digits. Only the first few digits will be significant, of course */
/* but the C specification forces me to print lots of insignificant ones */
/* too. Use flag characters '<' and '>' plus variables (before_dot) and  */
/* (after_dot) to keep track of what has happened.                       */
                            if (fp_round(buff, len))
                                dx++, len++;         /* number extended  */
                            if (dx<len-1)
                            {   fp_insert_(buff, dx, DecimalPoint);
                                p->after_dot = dx + p->precision - FLOATING_WIDTH;
                                if (p->after_dot!=0) buff[len++] = '>';
                            }
                            else
                            {   int w;
                                for (w=0; w<len-1; w++) buff[w] = buff[w+1];
                                buff[len-1] = '<';
                                p->before_dot = dx - len + 2;
                                buff[len++] = DecimalPoint;
                                if (p->precision!=0)
                                {   p->after_dot = p->precision;
                                    buff[len++] = '>';
                                }
                            }
                        }
                        else
                        {   if (fp_round(buff, len))
                                dx++, len++;     /* number extended  */
                            fp_insert_(buff, dx, DecimalPoint);
                        }
                    }
                    if ((p->precision==0) && ((flags&_VARIANT)==0)) len -= 1;
                }
                break;
    default:
/*
    case 'g':
    case 'G':
*/
                {   int dx = fp_digits(buff, d);
                    if (p->precision<1) p->precision = 1;
                    len = (p->precision>FLOATING_WIDTH) ? FLOATING_WIDTH+1 :
                                                          p->precision + 1;
                    dx += fp_round(buff, len);
/* now choose either 'e' or 'f' format, depending on which will lead to  */
/* the more compact display of the number.                               */
                    if ((dx>=p->precision) || (dx<-4))
                    {   buff[0] = buff[1];          /* e or E format */
                        buff[1] = DecimalPoint;
                    }
                    else
                    {   ch = 'f';                   /* uses f format */
                        if (dx>=0)
/* Insert a decimal point at the correct place for 'f' format printing   */
                        {   fp_insert_(buff, dx, DecimalPoint)
                        }
                        else
/* If the exponent is negative the required format will be something     */
/* like 0.xxxx, 0.0xxx or 0.00xx and I need to lengthen the buffer       */
                        {   int w;
                            dx = -dx;
                            for (w=len; w>=0; w--) buff[w+dx] = buff[w];
                            len += dx;
                            for(w=0; w<=dx; w++) buff[w] = '0';
                            buff[1] = DecimalPoint;
                        }
                    }
                    if((flags&_VARIANT)==0)         /* trailing 0?   */
                    {   p->after_dot = -1;
                        if (buff[len]!=DecimalPoint) while (buff[len-1]=='0') len--;
                        if (buff[len-1]==DecimalPoint) len--;
                    }
                    else
/* Allow for the fact that the specified precision may be very large in  */
/* which case I put in trailing zeros via the marker character '>' and a */
/* count (after_dot). Not applicable unless the '#' flag has been given  */
/* since without '#' trailing zeros in the fraction are killed.          */
                    {   if (p->precision>FLOATING_WIDTH)
                        {   p->after_dot = p->precision - FLOATING_WIDTH;
                            buff[len++] = '>';
                        }
                    }
                    if (ch!='f')    /* sets 'f' if it prints in f format */
                                    /* and 'e' or 'E' if in e format.    */
                        len = fp_addexp(buff, len, dx, ch + ('e'-'g'));
                }
                break;
    case 'e':
    case 'E':
                {   int dx = fp_digits(buff, d);
                    if (p->precision>FLOATING_WIDTH)
                    {   p->after_dot = p->precision - FLOATING_WIDTH;
                        p->precision = FLOATING_WIDTH;
                    }
                    len = p->precision + 2;
                    dx += fp_round(buff, len);
                    buff[0] = buff[1];
                    if ((p->precision==0) && !(flags&_VARIANT)) len = 1;
                    else buff[1] = DecimalPoint;
/* Deal with trailing zeros for excessive precision requests             */
                    if (p->after_dot>0) buff[len++] = '>';
                    len = fp_addexp(buff, len, dx, ch);
                }
                break;
        }
        return len;
    }
}

#endif /* _fp_display_c */

#if defined _nofp_disp_c || defined SHARED_C_LIBRARY

int _no_fp_display(int ch, char buff[], fp_print_rec *p)
{
    ch = ch;
    buff = buff;
    p = p;
    return 0;
}

#endif

/* There is a curiosity in the functions below: the compiler turns fp-free */
/* calls to {|f|s}printf (only) into calls to _{|f|s}printf, but _vfprintf */
/* and _vsprintf are also defined (not _vprintf)                           */

#if defined _printf_c || defined SHARED_C_LIBRARY

int _printf(const char *fmt, ...)
{
    va_list a;
    int n;
    printf_display_rec dr;
    va_start(a, fmt);
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _no_fp_display;
    n = __vfprintf(stdout, fmt, a, &dr);
    va_end(a);
    return n;
}

#endif

#if defined printf_c || defined SHARED_C_LIBRARY

int printf(const char *fmt, ...)
{
    va_list a;
    int n;
    printf_display_rec dr;
    va_start(a, fmt);
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _fp_display;
    n = __vfprintf(stdout, fmt, a, &dr);
    va_end(a);
    return n;
}

#endif

#if defined _fprintf_c || defined SHARED_C_LIBRARY

int _fprintf(FILE *fp, const char *fmt, ...)
{
    va_list a;
    int n;
    printf_display_rec dr;
    va_start(a, fmt);
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _no_fp_display;
    n = __vfprintf(fp, fmt, a, &dr);
    va_end(a);
    return n;
}

#endif

#if defined fprintf_c || defined SHARED_C_LIBRARY

int fprintf(FILE *fp, const char *fmt, ...)
{
    va_list a;
    int n;
    printf_display_rec dr;
    va_start(a, fmt);
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _fp_display;
    n = __vfprintf(fp, fmt, a, &dr);
    va_end(a);
    return n;
}

#endif

#if defined vprintf_c || defined SHARED_C_LIBRARY

/* apparently no _vprintf ?? */

int vprintf(const char *fmt, va_list a)
{
    printf_display_rec dr;
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _fp_display;
    return __vfprintf(stdout, fmt, a, &dr);
}

#endif

#if defined _vfprintf_c || defined SHARED_C_LIBRARY

int _vfprintf(FILE *fp, const char *fmt, va_list args)
{
    printf_display_rec dr;
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _no_fp_display;
    return __vfprintf(fp, fmt, args, &dr);
}

#endif

#if defined vfprintf_c || defined SHARED_C_LIBRARY

int vfprintf(FILE *fp, const char *fmt, va_list args)
{
    printf_display_rec dr;
    dr.putc = fputc; dr.ferror = ferror;
    dr.fn = _fp_display;
    return __vfprintf(fp, fmt, args, &dr);
}

#endif

typedef struct {
    char *ptr;
} StringFile;

int _sputc(int ch, FILE *f);
int _serror(FILE *f);

#if defined _sputc_c || defined SHARED_C_LIBRARY

int _sputc(int ch, FILE *fp) {
    StringFile *sf = (StringFile *)fp;
    char *op = sf->ptr;
    int r = *op++ = ch;
    sf->ptr = op;
    return r;
}

int _serror(FILE *f) {
    return 0;
}

#endif

#if defined _sprintf_c || defined SHARED_C_LIBRARY

int _sprintf(char *buff, const char *fmt, ...)
{
    StringFile sf;
    va_list a;
    int length;
    printf_display_rec dr;
    va_start(a, fmt);
    sf.ptr = buff;
    dr.putc = _sputc; dr.ferror = _serror;
    dr.fn = _no_fp_display;
    length = __vfprintf((FILE *)&sf, fmt, a, &dr);
    _sputc(0, (FILE *)&sf);
    va_end(a);
    return length;
}

#endif

#if defined sprintf_c || defined SHARED_C_LIBRARY

int sprintf(char *buff, const char *fmt, ...)
{
    StringFile sf;
    va_list a;
    int length;
    printf_display_rec dr;
    va_start(a, fmt);
    sf.ptr = buff;
    dr.putc = _sputc; dr.ferror = _serror;
    dr.fn = _fp_display;
    length = __vfprintf((FILE *)&sf, fmt, a, &dr);
    _sputc(0, (FILE *)&sf);
    va_end(a);
    return length;
}

#endif

#if defined _vsprintf_c || defined SHARED_C_LIBRARY

int _vsprintf(char *buff, const char *fmt, va_list a)
{
    StringFile sf;
    int length;
    printf_display_rec dr;
    dr.putc = _sputc; dr.ferror = _serror;
    dr.fn = _no_fp_display;
    sf.ptr = buff;
    length = __vfprintf((FILE *)&sf, fmt, a, &dr);
    _sputc(0, (FILE *)&sf);
    return length;
}

#endif

#if defined vsprintf_c || defined SHARED_C_LIBRARY

int vsprintf(char *buff, const char *fmt, va_list a)
{
    StringFile sf;
    int length;
    printf_display_rec dr;
    dr.putc = _sputc; dr.ferror = _serror;
    dr.fn = _fp_display;
    sf.ptr = buff;
    length = __vfprintf((FILE *)&sf, fmt, a, &dr);
    _sputc(0, (FILE *)&sf);
    return length;
}

#endif

/* End of printf.c */
