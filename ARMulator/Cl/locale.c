/* locale.c: ANSI draft (X3J11 Oct 86) library header, section 4.3 */
/* Copyright (C) Codemist Ltd., 1988                  */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */
/* version 0.01 */

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1994/07/07 17:03:41 $
 * Revising $Author: irickard $
 */

#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>  /* multibyte characters & strings */
#include <limits.h>  /* for CHAR_MAX */

#include "interns.h"

/* #define LC_COLLATE  1
   #define LC_CTYPE    2
   #define LC_MONETARY 4
   #define LC_NUMERIC  8
   #define LC_TIME    16
   #define LC_ALL     31
*/

#ifdef _lc_c
/* character array as required by setlocale to set _lc to the C locale.
   Having it here allows static initialisation of _lc to save space.
 */
const char _lc_C[] =
{'.', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
 CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX
};

/* Do static initialisation of _lc (if not shared-lib)
   Why isn't a struct lconv defined with const char * fields?
 */
struct lconv _lc =
{(char *)&_lc_C[0], /* . */
 (char *)&_lc_C[2], (char *)&_lc_C[3], (char *)&_lc_C[4],
 (char *)&_lc_C[5], (char *)&_lc_C[6], (char *)&_lc_C[7],
 (char *)&_lc_C[8], (char *)&_lc_C[9], (char *)&_lc_C[10],
 CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX
};
#endif

#ifdef SHARED_C_LIBRARY
/* IJR: _lc is now initialised dynamically by set_lconv
 */
struct lconv _lc;
#endif

#if defined localeconv_c || defined SHARED_C_LIBRARY

/* A client may reasonably want this without doing any locale_setting itself */

struct lconv *localeconv(void)
{
  return &_lc;
}

#endif

#if defined locale_c || defined SHARED_C_LIBRARY

static char Clocale[] = "C";
static char iso_8859_1[] = "ISO8859-1";
static char *locales[5] = 
#ifdef SHARED_C_LIBRARY
  { NULL }; /* IJR: set by _locale_init */
#else
  { Clocale, Clocale, Clocale, Clocale, Clocale };
#endif
static char *lc_all =
#ifdef SHARED_C_LIBRARY
  NULL; /* IJR: set by _locale_init */
#else
  Clocale;
#endif
/*
static struct lconv lc_native =
{".", ",", "\3", "STG ", "\243", ".", ",", "\3", "", "-",
 2, 2, 1, 0, 1, 0, 1, 2};
*/
static const char lc_native[] =
{'.', '\0', ',', '\0', '\3', '\0', 'S', 'T', 'G', ' ', '\0', '\243', '\0',
 '.', '\0', ',', '\0', '\3', '\0', '\0', '-', '\0',
 2, 2, 1, 0, 1, 0, 1, 2};

static void set_lconv(int category, const char *from)
{   int j;
    int *to = (int *)&_lc;
/*
 * This function is full of horrid magic numbers - BEWARE!
 * The first loop fills in the (char *) fields of the lconv struct;
 * the second loop fills in the char fields (beware packing!).
 */
    for (j = 0;  j < 10;  ++j)
    {   if (category & LC_NUMERIC  && j < 3 ||
            category & LC_MONETARY && j >= 3) *to = (int)from;
        while (*from != 0) ++from;
        ++from;
        ++to;
    }
    if (category & LC_MONETARY)
    {   char *t = (char *)to;
        for (; j < 18;  ++j) *t++ = *from++;
    }
}

static int index_of(int bitset)
{
  int j = 0;
  /* return index of ls bit */
  bitset &= (-bitset);
  for (;;) {
    bitset >>= 1;
    if (bitset == 0) return j;
    ++j;
  }
}

static const unsigned char strcoll_table_8859[256] = {
   /* Things preceding letters have normal ASCII ordering */
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
   0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
   0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
   0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
   0x40,  /* @ */
   0x41,  /* A - then 7 A variants */
   0x49,  /* B */
   0x4a,  /* C - then 1 C variant */
   0x4c,  /* D */
   0x4e,  /* E - then 4 E variants */
   0x53,  /* F */
   0x54,  /* G */
   0x55,  /* H */
   0x56,  /* I - then 4 I variants */
   0x5b,  /* J */
   0x5c,  /* K */
   0x5d,  /* L */
   0x5e,  /* M */
   0x5f,  /* N - then 1 N variant */
   0x61,  /* O - then 6 O variants */
   0x68,  /* P */
   0x69,  /* Q */
   0x6a,  /* R */
   0x6b,  /* S */
   0x6c,  /* T */
   0x6d,  /* U - then 4 U variants */
   0x72,  /* V */
   0x73,  /* W */
   0x74,  /* X */
   0x75,  /* Y - then 1 Y variant */
   0x77,  /* Z - then capital Eth & Thorn */
   0x7a,  /* [ */
   0x7b,  /* \ */
   0x7c,  /* ^ */
   0x7d,  /* _ */
   0x7e,  /* ` */
   0x7f,  /* a - then 7 a variants */
   0x87,  /* b */
   0x88,  /* c - then 1 c variant */
   0x8a,  /* d */
   0x8b,  /* e - then 4 e variants */
   0x90,  /* f */
   0x91,  /* g */
   0x92,  /* h */
   0x93,  /* i - then 4 i variants */
   0x98,  /* j */
   0x99,  /* k */
   0x9a,  /* l */
   0x9b,  /* m */
   0x9c,  /* n - then 1 n variant */
   0x9e,  /* o - then 6 o variants */
   0xa5,  /* p */
   0xa6,  /* q */
   0xa7,  /* r */
   0xa8,  /* s - then 1 s variant */
   0xaa,  /* t */
   0xab,  /* u - then 4 u variants */
   0xb0,  /* v */
   0xb1,  /* w */
   0xb2,  /* x */
   0xb3,  /* y - then 2 y variants */
   0xb6,  /* z - then eth & thorn */
   0xb9,  /* { */
   0xba,  /* | */
   0xbb,  /* } */
   0xbc,  /* ~ */
   0xbd,  /* del */
   /* top bit set control characters */
   0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
   0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd,
   0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5,
   0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd,
   /* other non_alpha */
   0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5,
   0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed,
   0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
   0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd,
   0x42,  /* A grave */
   0x43,  /* A acute */
   0x44,  /* A circumflex */
   0x45,  /* A tilde */
   0x46,  /* A umlaut */
   0x47,  /* A ring */
   0x48,  /* AE */
   0x4b,  /* C cedilla */
   0x4f,  /* E grave */
   0x50,  /* E acute */
   0x51,  /* E circumflex */
   0x52,  /* E umlaut */
   0x57,  /* I grave */
   0x58,  /* I acute */
   0x59,  /* I circumflex */
   0x5a,  /* I umlaut */
   0x78,  /* Eth */
   0x60,  /* N tilde */
   0x62,  /* O grave */
   0x63,  /* O acute */
   0x64,  /* O circumflex */
   0x65,  /* O tilde */
   0x66,  /* O umlaut */
   0xfe,  /* multiply sign */
   0x67,  /* O with line */
   0x6e,  /* U grave */
   0x6f,  /* U acute */
   0x70,  /* U circumflex */
   0x71,  /* U umlaut */
   0x76,  /* Y acute */
   0x79,  /* Thorn */
   0xa8,  /* german sz */
   0x80,  /* a grave */
   0x81,  /* a acute */
   0x82,  /* a circumflex */
   0x83,  /* a tilde */
   0x84,  /* a umlaut */
   0x85,  /* a ring */
   0x86,  /* ae */
   0x89,  /* c cedilla */
   0x8c,  /* e grave */
   0x8d,  /* e acute */
   0x8e,  /* e circumflex */
   0x8f,  /* e umlaut */
   0x94,  /* i grave */
   0x95,  /* i acute */
   0x96,  /* i circumflex */
   0x97,  /* i umlaut */
   0xb7,  /* eth */
   0x9d,  /* n tilde */
   0x9f,  /* o grave */
   0xa0,  /* o acute */
   0xa1,  /* o circumflex */
   0xa2,  /* o tilde */
   0xa3,  /* o umlaut */
   0xff,  /* divide sign */
   0xa4,  /* o with line */
   0xac,  /* u grave */
   0xad,  /* u acute */
   0xae,  /* u circumflex */
   0xaf,  /* u umlaut */
   0xb4,  /* y acute */
   0xb8,  /* thorn */
   0xb5   /* y umlaut */
};

static char *setlocales(int category, char *value) {
    int j = 0;
    for (; category != 0; category >>= 1, ++j) {
      if (category & 1) locales[j] = value;
    }
    lc_all = value;
    for (j = 0;  j < sizeof(locales) / sizeof(char *);  ++j) {
      if (locales[j] != value) lc_all = NULL;
    }
    return value;
}

#ifdef SHARED_C_LIBRARY
/* character array as required by setlocale to set _lc to the C locale.
   Having it here puts it in close proximity to where it is used below.
 */
const char _lc_C[] =
{'.', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
 CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX,CHAR_MAX
};
#endif

char *setlocale(int category, const char *locale)
{
    /* I expect the category to be a bit-map - complain if out of range  */
    if (((unsigned)category > LC_ALL) || (category == 0))
      /* no can do... */
      return NULL;
    if (locale == 0) {
      /* get locale */
      if (category == LC_ALL)
        return lc_all;
      else
        return locales[index_of(category)];
    } else {
      /* set locale */
      if (strcmp(locale, Clocale) == 0) {
        /* Clocale - also initial state */
        if (category & LC_CTYPE) _set_ctype_8859(0);
        if (category & LC_COLLATE) __set_strcoll_table(0);
        set_lconv(category, _lc_C);
        return setlocales(category, Clocale);
      } else if (locale[0] == 0 || strcmp(locale, iso_8859_1) == 0) {
        /* ISO-8859 - the default locale */
        if (category & LC_CTYPE) _set_ctype_8859(1);
        if (category & LC_COLLATE) __set_strcoll_table(strcoll_table_8859);
        set_lconv(category, lc_native);
        return setlocales(category, iso_8859_1);
      }
    }
    return NULL;
}

#ifdef SHARED_C_LIBRARY
void _locale_init(void)
{ 
  int category;
  char **plocale = &locales[0];

  for (category = LC_ALL; category != 0; category >>= 1)
    *plocale++ = Clocale;
  lc_all = Clocale;
  set_lconv(LC_NUMERIC | LC_MONETARY, _lc_C);
}
#endif

#endif /* locale_c */

#define STATE_DEPENDENT_ENCODINGS 0

#if defined mblen_c || defined SHARED_C_LIBRARY

int mblen(const char *s, size_t n)
{   if (s == 0) return STATE_DEPENDENT_ENCODINGS;
/* @@@ ANSI ambiguity: if n=0 and *s=0 then return 0 or -1?                 */
/* @@@ LDS: for consistency with mbtowc, return -1                          */
    if (n == 0) return -1;
    if (*s == 0) return 0;
    return 1;
}

#endif

#if defined mbtowc_c || defined SHARED_C_LIBRARY

int mbtowc(wchar_t *pwc, const char *s, size_t n)
{   if (s == 0) return STATE_DEPENDENT_ENCODINGS;
/* @@@ ANSI ambiguity: if n=0 and *s=0 then return 0 or -1?                 */
/* @@@ LDS At most n chars of s are examined, ergo must return -1.          */
    if (n == 0) return -1;
    else
    {   wchar_t wc = *(unsigned char *)s;
        if (pwc) *pwc = wc;
        return (wc != 0);
    }
}

#endif

#if defined wctomb_c || defined SHARED_C_LIBRARY

int wctomb(char *s, wchar_t w)
{   if (s == 0) return STATE_DEPENDENT_ENCODINGS;
/* @@@ ANSI ambiguity: what return (and setting for s) if w == 0?           */
/* @@@ LDS The CVS suggests return #chars stored; I agree this is rational. */
    if ((unsigned)w > (unsigned char)-1) return -1;
    *s = w;
    return 1;
}

#endif

#if defined mbstowcs_c || defined SHARED_C_LIBRARY

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
/* @@@ ANSI ambiguity: if n=0 then is *s read?                              */
    size_t r = 0;
    for (; n != 0; n--)
    {   if ((pwcs[r] = ((unsigned char *)s)[r]) == 0) return r;
        r++;
    }
    return r;
}

#endif

#if defined wcstombs_c || defined SHARED_C_LIBRARY

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
/* @@@ ANSI ambiguity: if n=0 then is *pwcs read?  Also invalidity check?   */
    size_t r = 0;
    for (; n != 0; n--)
    {   wchar_t w = pwcs[r];
        if ((unsigned)w > (unsigned char)-1) return (size_t)-1;
        if ((s[r] = w) == 0) return r;
        r++;
    }
    return r;
}

#endif

/* end of locale.c */
