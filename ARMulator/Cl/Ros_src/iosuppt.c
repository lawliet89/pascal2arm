/* riscos/iosuppt.c: bulk of target-dependent code for riscos   */
/*                   targetting of C library                    */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

/* Avoid pulling in clock() just because _ttyread is used.
   (If clock() is present, _ttyread() wants to adjust its value
    to avoid counting time waiting for tty input).
   Having to do it this way is a wart, which a better implementation
   of ___weak would cure.
 */

___weak unsigned clock(void);

#include <errno.h>
#include "hostsys.h"
#include "riscos.h"

#if defined remove_c || defined SHARED_C_LIBRARY

int remove(const char *pathname)
{   __riscos_osfile_block fb;
    if (__riscos_osfile(6, pathname, &fb) <= 0) return 1;
    return 0;
}

#endif

#if defined rename_c || defined SHARED_C_LIBRARY

#define FSControl 0x29

int rename(const char *old, const char *new)
{   __riscos_swi_regs r;
    r.r[0] = 25;        /* rename file */
    r.r[1] = (int)old;
    r.r[2] = (int)new;
    return __riscos_swi(FSControl, &r, &r) != 0;
}

#endif

/* Riscos has a second distinguished FILEHANDLE value, to indicate that  */
/* a file is a keyboard and/or vdu, which can't be read or written using */
/* Riscos file operations (or at any rate, couldn't when the library was */
/* originally implemented).                                              */
#define TTYHANDLE 0

#define istty(fh) ((fh) == TTYHANDLE)

extern int _ttywrite(const unsigned char *buf, unsigned int len, int flag);
extern int _ttyread(unsigned char *buff, int size, int flag);

#if defined sys_open_c || defined SHARED_C_LIBRARY

static int isttyname(const char *s)
{   if (s[0] == ':' && (s[1]|0x20) == 't' && (s[2]|0x20) == 't' && s[3] == 0)
        return 1;   /* string specification (:tt) of terminal stream */
    return 0;
}

FILEHANDLE _sys_open(const char *filename, int openmode)
{ /* nasty magic number interface for openmode */
  static const int modtab[6] = { /* r = */ 0x04c, /* r+ = */ 0x0cc,
                                 /* w = */ 0x4cc, /* w+ = */ 0x4cc,
                                 /* a = */ 0x3cc, /* a+ = */ 0x3cc };
  if (isttyname(filename)) return TTYHANDLE;
  else {
    char *name = (char *)filename;                 /*  yuk yuk yuk yuk yuk  */
    FILEHANDLE fh;
    int size = 16 * 1024;                    /* first try for created files */
    int osmode = modtab[(openmode >> 1) & 7];    /* forget the 'b'inary bit */
    __riscos_osfile_block fb;

    /* maybe stamp file with current datestamp */
    if ((openmode & OPEN_T) ||                /* update timestamp requested */
        (openmode & OPEN_W) ||
        (openmode & ~OPEN_B) == OPEN_A)            /* or mode = w, w+, or a */
    {   if (__riscos_osfile(9, name, &fb) == __riscos_ERROR)
        {   if (__riscos_last_oserror()->errnum == 0x108c9)
            {   (void)__riscos_osfile(9, name, &fb);/* to restore the error */
                errno = -1;
                return NONHANDLE;                       /* (Protected disc) */
            }
        }
    }
retry_open:
    fh = __riscos_osfind_open(osmode & 0xff, name);
    if (osmode <= 0xcc) {                                        /* r or r+ */
      if (fh == __riscos_ERROR) errno = -1;
      return (fh <= 0) ? NONHANDLE :                           /* not found */
                         fh;
    } else if (fh > 0) {
      if ((osmode == 0x4cc) || (size == 0))
          if (__riscos_osargs(3, fh, 0) == __riscos_ERROR)
          {   (void)__riscos_osfind_close(0, fh);
              errno = -1;
              return NONHANDLE;
          }
      return fh;
    } else if (fh <= 0) {
        /* __riscos_osfile(11) creates an empty file of size 'size', of     */
        /* type given by fb.load, stamped with the current date & time      */
      fb.load = (openmode & 1) ? 0xffd : 0xfff;              /* data : text */
      fb.start = 0;
      for (; ; size >>= 1) {
        if (size < 512) { errno = -1; return NONHANDLE; }
        fb.end = size;
        if (__riscos_osfile(11, name, &fb) > 0) break;
      }
      size = 0;
      goto retry_open;
    }
    if (fh == __riscos_ERROR) errno = -1;
    return NONHANDLE;
  }
}

#endif

#if defined sys_istty_c || defined SHARED_C_LIBRARY

#include "ioguts.h"

int _sys_istty(FILE *stream)
{
    return istty(stream->file);
}

#endif

#if defined sys_seek_c || defined SHARED_C_LIBRARY

int _sys_seek(FILEHANDLE fh, long pos)
{
    if istty(fh) return 0;
    {   int rc = __riscos_osargs(1, fh, (int)pos);
        if (rc == __riscos_ERROR) errno = -1;
        return rc;
    }
}

#endif

#if defined sys_flen_c || defined SHARED_C_LIBRARY

long _sys_flen(FILEHANDLE fh)
{
    int rc = __riscos_osargs(2, fh, 0);
    if (rc == __riscos_ERROR) errno = -1;
    return rc;
}

#endif

#if defined sys_iserror_c || defined SHARED_C_LIBRARY

int _sys_iserror(int status)
{
    return status == __riscos_ERROR;
}

#endif

#if defined sys_write_c || defined SHARED_C_LIBRARY

int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{   if (istty(fh))
        return _ttywrite(buf, len, mode);
    else {
        __riscos_osgbpb_block b;
        b.dataptr = (void *)buf;
        b.nbytes = (int)len;
        if (__riscos_osgbpb(2, fh, &b) == __riscos_ERROR) {
            errno = -1;
            return __riscos_ERROR;
        } else
            return b.nbytes;
    }
}

#endif

#if defined sys_read_c || defined SHARED_C_LIBRARY

int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode)
{
    if (istty(fh))
        return _ttyread(buf, (int)len, mode);
    else {
        __riscos_osgbpb_block b;
        b.dataptr = (void *)buf;
        b.nbytes = (int)len;
        if (__riscos_osgbpb(4, fh, &b) == __riscos_ERROR) {
            errno = -1;
            return __riscos_ERROR;
        } else
            return b.nbytes;
    }
}

#endif

#if defined sys_ensure_c || defined SHARED_C_LIBRARY

int _sys_ensure(FILEHANDLE fh)
{
    if (istty(fh)) return 0;
    {   int rc = __riscos_osargs(0xFF, fh, 0);
        if (rc == __riscos_ERROR) errno = -1;
        return rc;
    }
}

#endif

#if defined sys_close_c || defined SHARED_C_LIBRARY

int _sys_close(FILEHANDLE fh)
{   if (istty(fh)) return 0;
    {   int rc = __riscos_osfind_close(0, fh);
        if (rc == __riscos_ERROR) errno = -1;
        return rc;
    }
}


#endif

#define LF '\n'

___weak void _clock_ignore(clock_t);

#if defined ttyread_c || defined SHARED_C_LIBRARY

#include <stdio.h>
#include "ioguts.h"   /* _IOBIN */
#include "interns.h"

int _ttyread(unsigned char *buff, int size, int flag)
{
/* behaviour similar to Kgbpb but reads from keyboard, performing local  */
/* edits as necessary.  Control chars echo as ^<upper case>.             */
/* AM: ignore clock ticks while waiting for keyboard                     */
/* If _IOBIN is set return 1st char read with no echo.
 * If _IONBF is set return 1st char read with echo.
 * Else read till CR LF ^D or EOF and return line.  Refuse echo if buffer full.
 */
    int count=0;
    clock_t t = (clock == NULL) ? 0 : clock();
    do
    {   int ch;
        do {ch = __riscos_osrdch();} while (ch == -27) /* ESCAPE */;
        if (flag & _IOBIN && ch != EOF && ch != __riscos_ERROR)
            buff[count++] = ch;             /* binary - no echo */
        else switch (ch)
        {
case __riscos_ERROR:
case EOF:                                   /* see _osrdch for EOF */
case 0x04:  CallIfPresent(_clock_ignore, (t));               /* ^D */
            return(0x80000000+size-count);
case '\n':                                  /* treat CR as LF */
case '\r':  if(count>=size) continue;
            __riscos_oswrch('\r'); __riscos_oswrch(LF);
            buff[count++] = '\n';
            CallIfPresent(_clock_ignore, (t));
            return(size-count);
case 0x08:                                  /* BS     */
case 0x7f:  if(count!=0)                    /* rubout */
            {   __riscos_oswrch(0x7f);
                if (buff[--count] < ' ') __riscos_oswrch(0x7f);
            }
            break;
case 0x15:  while(count>0)                  /* ctrl-U kills line */
            {   __riscos_oswrch(0x7f);
                if (buff[--count] < ' ') __riscos_oswrch(0x7f);
            }
            break;
default:    if(count>=size) continue;
            if (ch < ' ' && ch != 0x07)
               __riscos_oswrch('|'), __riscos_oswrch(ch | '@');
            else
               __riscos_oswrch(ch);
            buff[count++] = ch;
            break;
        }
    } while (!(flag & _IOBIN+_IONBF));
    CallIfPresent(_clock_ignore, (t));
    return(size-count);
}

#endif

#if defined ttywrite_c || defined SHARED_C_LIBRARY

#include <ctype.h>
#include "ioguts.h"    /* _IOBIN */

#pragma -s1
/* stack checking off to allow calling from default sigstak handler */

void _ttywrch(int ch) {
    if (ch == '\n') {
        __riscos_oswrch('\r'); ch = LF;
    } else if (ch < 32 && ch != 0x07 && !isspace(ch)) {
        __riscos_oswrch('|');
        ch = (ch & 31) | 64;
    }
    __riscos_oswrch(ch);
}

#pragma -s0

int _ttywrite(const unsigned char *buf, unsigned len, int flag)
/* behaves like Kgbpb, but outputs to console.                              */
/* if 'flag' has _IOBIN set then LF's ('\n's) do not have CR prefixed.      */
{   if (flag & _IOBIN)
        while (len-- > 0) __riscos_oswrch(*buf++);
    else
        while (len-- > 0) _ttywrch(*buf++);

    return 0;    /* number of chars unwritten */
}

#endif

/* struct bbctime objects are used to hold bbc/brazil 5-byte timer value -
   conventionally centi-seconds since 1-Jan-1900.
*/

struct bbctime {unsigned int l,h;};

#if defined clock_c || defined SHARED_C_LIBRARY

static clock_t _time0;

static clock_t _clock()
{   struct bbctime bt;
    __riscos_osword(1, (int *)&bt);
    return bt.l;
}

void _clock_init()   /* private - for initialisation */
{   _time0 = _clock();
}

void _clock_ignore(clock_t t)
/* ignores ticks from t to now - used to remove terminal input waits */
{   _time0 += (_clock() - _time0) - t;
}

/* Exported... */

clock_t clock()
{   return _clock() - _time0;     /* clock runs even if date not set */
}

#endif

#if defined time_c || defined SHARED_C_LIBRARY

/* Return the current time in secs since 1-Jan-1970 (as for UNIX) */
/* Completely host-os specific                                    */

time_t time(time_t *timer)
{ time_t result;
  struct bbctime bt, w, w2;
  bt.l = 3;                     /* presumably 'request time' arg */
  __riscos_osword(14, (int *)&bt); /* read timer as 5 byte integer  */
/* to two 3-byte things - for divide */
  w.h = ((bt.h & 255) << 8) | (bt.l >> 24);
  w.l = bt.l & 0xffffff;
/* turn csecs to secs */
  w2.h = w.h / 100;
  w2.l = ((w.h % 100 << 24) | w.l) / 100;
/* back to 8 byte binary */
  bt.h = w2.h >> 8;
  bt.l = (w2.h << 24) | w2.l;
/* normalise to Jan70 instead of Jan00... */
#define secs0070 (((unsigned)86400)*(365*70+17))  /* less than 2^32 */
  if (bt.l < secs0070) bt.h--;
  bt.l -= secs0070;
/* if high word is non-zero then date is unset/out of unix range... */
  result = bt.h ? -1 : bt.l;

  if (timer) *timer = result;
  return result;
}

#endif

#if defined system_c || defined SHARED_C_LIBRARY

#include <stdio.h>
#include <stdlib.h>
#include "interns.h"

int system(const char *string)
{
#define CALL  0
#define CHAIN 1
    int rc;
    if (string==NULL) return 1 /* available */;
    if ((string[0] | 0x20)=='c') {
      char c = string[1] | 0x20;
      if (c=='a') {
        if ((string[2] | 0x20)=='l' && (string[3] | 0x20)=='l' &&
            string[4]==':') string = string+5;
      } else if (c=='h') {
        if ((string[2] | 0x20)=='a' && (string[3] | 0x20)=='i' &&
            (string[4] | 0x20)=='n' && string[5]==':') {
          string = string+6;
          __rt_lib_shutdown(1);
          __riscos_system(string, CHAIN);
          /* which never returns */
        }
      }
    }
    rc = __riscos_system(string, CALL);
    if (rc != 0) return rc;
    else
    { char *env;
      env = getenv("Sys$ReturnCode");
      if (env == NULL) return 0;
      else return atoi(env);
    }
    /* -2 Kernel Fail, 0 = OK. Any other = result code from subprogram exit */
}

#endif

#if defined getenv_c || defined SHARED_C_LIBRARY

#include <stdlib.h>
#include "interns.h"

static char *_getenv_value;

char *getenv(const char *name)
{
    if (_getenv_value == NULL) _getenv_value = __rt_malloc(256);
    if (_getenv_value == NULL ||
        __riscos_getenv(name, _getenv_value, 256) != NULL)
       return NULL;
    else
       return _getenv_value;
}


void _getenv_init(void)
{
    _getenv_value = NULL;
}

#endif

