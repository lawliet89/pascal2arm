/* stdio.c: ANSI draft (X3J11 Oct 86) library code, section 4.9 */
/* Copyright (C) Codemist Ltd, 1988                             */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */
/* version 0.17e */

/*
 * RCS $Revision: 1.15 $
 * Checkin $Date: 1995/04/07 14:38:33 $
 * Revising $Author: enevill $
 */

/* Incorporate _sys_tmpnam_ idea from WGD */

/* N.B. I am trying to factor out machine dependence via calls to           */
/* routines like _sys_read_ which can be implemented as _osgbpb or          */
/* NIOP as required.  This file SHOULD therefore be machine independent     */

#include "hostsys.h"   /* _sys_alloc() etc */
#include <stdio.h>
#include <string.h>    /* for memcpy etc */
#include <errno.h>

#include "ioguts.h"
#include "interns.h"
#include "externs.h"

#ifndef _SYS_OPEN
/* Temp feature to cope with upgrade path of compiler */
#define _SYS_OPEN SYS_OPEN
#endif

#define NO_DEBUG


#ifdef DEBUG
#include "riscos.h"
#define dbmsg(m, m2) if (stream > stderr) {int last=0;int iw; char v[128]; \
        sprintf(v, m, m2); \
        for(iw=0;v[iw];__riscos_oswrch(last=v[iw++])); \
        if (last == 10)__riscos_oswrch(13);}
#define dbmsg_noNL(m, m2) if (stream > stderr) {int last;int iw; char v[128]; \
        sprintf(v, m, m2); \
        for(iw=0;v[iw];__riscos_oswrch(last=v[iw++]));}
#else
#define dbmsg(m, m2)
#define dbmsg_noNL(m, m2)
#endif

extern void _seterr(FILE *stream);

extern int _writebuf(unsigned char *buf, int len, FILE *stream);

extern int __filbuf(FILE *);
   /*
    * called by getc to refill buffer and or sort out flags.
    * Returns: first character put into buffer or EOF on error.
    * Note that now file->icnt has *not* been decremented by
    * getc() before __filbuf() is called.
    */

extern int __flsbuf(int, FILE *);
   /*
    * called by putc to flush buffer and or sort out flags.
    * Returns: character put into buffer or EOF on error.
    * Note that now file->ocnt has *not* been decremented by
    * putc() before __flsbuf() is called.
    */

extern void _flushlinebuffered(void);

extern int _fflush(FILE *stream);

extern void _deferredlazyseek(FILE *stream);

#define _ftell(stream) (  (stream)->flag & _IOLAZY \
                        ? (stream)->lspos \
                        : (stream)->pos + (stream)->ptr - (stream)->base)

#define _getc(p) \
  { int ic = (p)->icnt; unsigned char *ip = (p)->ptr; \
    if (--ic < 0) return __filbuf(p); \
    { int r = *ip++; (p)->icnt = ic; (p)->ptr = ip; \
      return r; } }

#define _putc(ch, p) \
  { unsigned char *op = (p)->ptr; int oc = (p)->ocnt; \
    if (--oc < 0) return __flsbuf(ch, p); \
    { int r = (*op++) = (ch); (p)->ocnt = oc; (p)->ptr = op; \
      return r; } }

#define _feof(stream)   ((stream)->flag & _IOEOF)
#define _ferror(stream) ((stream)->flag & _IOERR)

#define EXTENT(stream) ((stream)->extent > (stream)->ptr \
                       ? (stream)->extent : (stream)->ptr)

#if defined iob_c
/* disgusting bodge: initialised to ensure contiguousness */
FILE __stdin = {};
FILE __stdout = {};
FILE __stderr = {};
FILE __iob[_SYS_OPEN-3] = {};
#endif

#if defined stdio_c || defined SHARED_C_LIBRARY

#ifndef SHARED_C_LIBRARY
FILE __stdin;
FILE __stdout;
FILE __stderr;
static FILE iob[_SYS_OPEN-3];
#else
#define iob __iob
extern FILE iob[];
#endif

void _seterr(FILE *stream)
{   stream->flag |= _IOERR;
    stream->icnt = stream->ocnt = 0;
}

int _writebuf(unsigned char *buf, int len, FILE *stream)
{   int w;
    FILEHANDLE fh = stream->file;
    int flag = stream->flag;
    if (flag & _IOSHARED) /* this is really gross */
    /* If a file is shared, any position recorded in this sharer need not be */
    /* correct (because some other sharer may since have written).           */
    {   flag |= _IOSEEK;
        stream->pos = _sys_flen(fh);
dbmsg("_IOSHARED so zoom to end %d\n", (int)stream->pos);
    }
    if (flag & _IOSEEK+_IOBUFREAD)
    {
dbmsg("_writebuf seeking to %d\n", (int)stream->pos);
        if (_sys_seek(fh, stream->pos) < 0)
        {   _seterr(stream);
            return EOF;
        }
        stream->flag = (flag &= ~(_IOSEEK+_IOBUFREAD));
    }
dbmsg_noNL("_writebuf pop goes the stoat %i, ", fh);
dbmsg_noNL("%X, ", (int)buf);
dbmsg_noNL("%d, ", len);
dbmsg("%X\n", flag);
    w = _sys_write(fh, buf, len, flag);
#ifdef HOSTOS_NEEDSENSURE
    stream->flag |= _IONEEDSENSURE;
#endif
dbmsg("_sys_write returned %d\n", w);
    stream->pos += len - (w & 0x7fffffffL);
    if (w!=0)    /* AM: was (w<0) but trap unwritten chars as error too */
    {   _seterr(stream);
        return EOF;
    }
dbmsg("filelen = %ld\n",_sys_flen(fh));           /* length of this file      */
    return 0;
}

#define LineBufferedOutput(f) (((f).flag & (_IOLBF | _IOWRITE)) == (_IOLBF | _IOWRITE))

void _flushlinebuffered(void)
{ int i;

  if (LineBufferedOutput(__stdin)) fflush(&__stdin);
  if (LineBufferedOutput(__stdout)) fflush(&__stdout);
  if (LineBufferedOutput(__stderr)) fflush(&__stderr);
  for (i = 0 ; i < _SYS_OPEN-3 ; i++)
    if (LineBufferedOutput(iob[i])) fflush(&iob[i]);
}

int _fflush(FILE *stream)
{
    unsigned char *buff = stream->base;
    unsigned char *extent = EXTENT(stream);
dbmsg("%s\n", "_fflush");
    stream->flag &= ~_IOUNGET;
    if ((stream->flag & _IOREAD+_IOWRITE) == _IOREAD) return 0;
    if ((stream->flag & _IOERR+_IOWRITE) != _IOWRITE) return EOF;
    /* N.B. really more to do here for ANSI input stream */
    if (stream->flag & _IODIRTIED)
    {   /* only write if dirty buffer - this avoids problems with
           writing to a file opened in append (or read+write) mode
           when only input has been done since the last fflush/fseek.
        */
dbmsg("%s\n", "dirty buffer");
        if (extent != buff)       /* something to do */
        {
dbmsg("%s\n", "call _writebuf");
          if (_writebuf(buff, extent - buff, stream)) return EOF;
        }
        stream->ptr = stream->extent = extent = buff;
/* the next line forces a call to __filbuf/__flsbuf on next putc/getc -  */
/* this is necessary since change of direction may happen.               */
        stream->ocnt = 0;
        stream->flag &= ~_IODIRTIED;
dbmsg("%s\n", "clear IODIRTIED");
    }
else dbmsg("%s\n", "not a dirty buffer");

/* now do quick frig to fix I/O streams: essentially fseek(s, 0, SEEK_CUR).
    if ((stream->flag & (_IOREAD+_IOWRITE)) == (_IOREAD+_IOWRITE))
    {
      stream->flag = stream->flag & ~(_IOEOF|_IONOWRITES|_IOREADS|_IOPEOF)
                                                                   |_IOSEEK;
      stream->icnt = stream->ocnt = 0;
      stream->pos += (extent - buff);
      stream->ptr = buff;
    }
*/

    return 0;
}

void _deferredlazyseek(FILE *stream)
{
dbmsg("deferredlazyseek to %d\n", (int)stream->lspos);
    /* only here because of a seek */
    stream->flag &= ~_IOLAZY;
    if (stream->pos != stream->lspos) {
      _fflush(stream);
      /* clear EOF condition */
dbmsg("%s\n", "clear IODIRTIED");
      stream->flag = stream->flag & ~(_IONOWRITES | _IONOREADS) | _IOSEEK;
      stream->pos = stream->lspos;
      stream->ptr = stream->extent = stream->base;
    }
else dbmsg("%s\n", ".....already there");
    stream->flag &= ~(_IOEOF | _IOPEOF);
}

int fclose(FILE *stream)
{   /* MUST be callable on a closed file - if stream clr then no-op. */
    FILEHANDLE fh = stream->file;
    unsigned char *buff = stream->base;
    int flag = stream->flag;
    int res = 0;

    if (!(flag & _IOREAD+_IOWRITE)) return EOF;   /* already closed    */
    if (!(flag & _IOSTRG))                    /* from _fopen_string    */
    {   /* @@@                                                         */
        /* It is assumed here that a file may be shared at most once.  */
        /* This strikes me like an accident waiting to happen: the     */
        /* justification involves __dup() (the only way for sharing to */
        /* arise) not being available to clients, and use by the       */
        /* library being restricted to std{in/out/err}, and only the   */
        /* latter two being shareable because of open mode conflict.   */
        int fd = _SYS_OPEN;
        res = fflush(stream);
        if (flag & _IOSHARED)
        {   for (fd =  0;  fd < _SYS_OPEN;  ++fd)
            {   FILE *f = &_iob[fd];
                if (f != stream &&
                    (f->flag & _IOREAD+_IOWRITE) &&     /* f is open */
                    f->file == fh && (f->flag & _IOSHARED))
                {   f->flag &= ~_IOSHARED;       /* no longer shared */
                    stream->flag &= ~_IOSHARED;
                    break;
                }
            }
        }
        /* Assert: fd != SYS_OPEN => fh is shared and mustn't be closed */
        if (fd == _SYS_OPEN &&
            _sys_close(fh) < 0) res = EOF;          /* close real file */
        if (flag & _IOSBF) __rt_free(buff);   /* free buffer if system */
        if ((flag & _IODELMSK) == _IODEL
#ifndef SHARED_C_LIBRARY
            && _tmpfile_delete != 0
#endif
           )
            res = _tmpfile_delete(stream, res);
    }
    memset(stream, 0, sizeof(FILE));
    return res;
}

FILE *freopen(const char *name, const char *mode, FILE *iob)
{
/* The use of modes "r+", "w+" and "a+" is not fully thought out   */
/* yet, in that calls to __flsbuf may write back stuff that was    */
/* loaded by __filbuf and thereby corrupt the file.                */
/* This is now just about fixed given the ANSI restriction that    */
/* calls to getc/putc must be fflush/fseek separated.              */
    FILEHANDLE fh;
    int flag, openmode;        /* nasty magic numbers for openmode */
    fclose(iob);
    switch (*mode++)
    {   default:  return(NULL);               /* mode is incorrect */
        case 'r': flag = _IOREAD;  openmode = OPEN_R; break;
        case 'w': flag = _IOWRITE; openmode = OPEN_W; break;
        case 'a': flag = _IOWRITE | _IOAPPEND;
                                   openmode = OPEN_A; break;
    }
    for (;;)
    {   switch (*mode++)
        {
    case '+':   flag |= _IOREAD+_IOWRITE, openmode |= OPEN_PLUS;
                continue;
    case 'b':   flag |= _IOBIN, openmode |= OPEN_B;
                continue;
        }
        if (*(mode-1) == 't') openmode |= OPEN_T;
        break;
    }
    if ((fh = _sys_open(name, openmode)) == NONHANDLE) return NULL;
    iob->ptr = iob->base = NULL; iob->bufsiz = BUFSIZ;
    iob->flag = flag;
    iob->file = fh;
    if (openmode & OPEN_A) fseek(iob, 0L, SEEK_END);  /* a or a+             */
    return iob;
}

FILE *fopen(const char *name, const char *mode)
{   int i;
    for (i=3; i<_SYS_OPEN; i++)
    {   FILE *stream = &_iob[i];
        if (!(stream->flag & _IOREAD+_IOWRITE))  /* if not open then try it */
            return (freopen(name, mode, stream));
    }
    return 0;   /* no more i/o channels allowed for */
}

/* initialisation/termination code... */

int _sys_msg_via_stderr(const char *s, int flag) {
    if (stderr->flag & _IOWRITE) {
    /* error_recursion already checked and found false */
        _init_flags.error_recursion = 1;
        if (flag & NL_PRE) fputc('\n', stderr);
        fputs(s, stderr);
        if (flag & NL_POST) fputc('\n', stderr);
        _init_flags.error_recursion = 0;
        return 1;
    } else
        return 0;
}

static void __open_fail(const char *f) {
    _sys_msg("Couldn't write ", NL_PRE);
    _sys_msg(f, NL_POST);
    _exit(1);
}

void _initio(char *f1,char *f2,char *f3)
{
    memset(_iob, 0, _SYS_OPEN*sizeof(FILE));
    /* In the next lines DO NOT use standard I/O for error msgs (not open yet)
       Moreover, open in this order so we do not create/overwrite output if
       input does not exist. */
    if (freopen(f3, "w", stderr) == 0) __open_fail(f3);
    if (freopen(f1, "r", stdin) == 0) __open_fail(f1);
    if (freopen(f2, "w", stdout) == 0) __open_fail(f2);
    if (setvbuf(stdin,NULL,_IOLBF,BUFSIZ) != 0) __open_fail(f1);
    if (setvbuf(stdout,NULL,_IOLBF,BUFSIZ) != 0) __open_fail(f2);
}

void _terminateio()
{   int i;
    for (i=3; i<_SYS_OPEN; i++) {
        fclose(&_iob[i]);
    }
    /* for cowardice do stdin, stdout, stderr last (in that order) */
    for (i=0; i<3; i++) {
        fclose(&_iob[i]);
    }
}

#endif /* stdio_c */

#if defined flsbuf_c || defined SHARED_C_LIBRARY

extern int __flsbuf(int ch, FILE *stream)
{   int flag;
dbmsg_noNL("%s ", "__flsbuf");
dbmsg("%c\n", ch);
    stream->flag = (stream->flag & ~_IOUNGET) | _IOACTIVE;

    if (stream->flag & _IOLAZY) _deferredlazyseek(stream);

    if ((stream->ocnt < 0) && !(stream->flag & _IOLBF)) {
      /* buffer ~empty, sequence of events that lead to here are :          */
      /* put seek get seek put                                              */
dbmsg("flushbuf negative ocnt = %d\n", stream->ocnt);
      stream->ocnt = (-stream->ocnt) - 1;
      stream->flag |= (_IODIRTIED+_IONOREADS);           /* we are writing */
      return *(stream->ptr)++ = (ch);
    }

    flag = stream->flag;
    if ((flag & _IOERR+_IOSTRG+_IOWRITE+_IONOWRITES) != _IOWRITE)
    {   _seterr(stream);
dbmsg("%s\n", "!= _IOWRITE");
        return EOF;
    }
/* the next conditional code is ACN's view of that APPEND means seek to     */
/* EOF after EVERY fflush, not just initially.  Hmm, ANSI really should     */
/* clarify - the problem is perhaps that we wish to seek to EOF after       */
/* fflush after read, but not after fflush after write?                     */
/* The line "if ((flag & (_IOSEEK+_IOAPPEND)) == (_IOSEEK+_IOAPPEND))"      */
/* which PH10 suggested does not work, try the sequence:                    */
/* create file with something in it; close file; open the file (append mode)*/
/* seek to start; write a char; THE CHAR WRITTEN IS NOT WRITTEN TO EOF      */
/* The following line has been reinstated cos it works.                     */
    if ((flag & (_IONOREADS+_IOSEEK+_IOAPPEND)) == _IOAPPEND)
/* Will somebody please help ACN remember/understand what was going on here!*/
    {   /* first write to APPEND file after FFLUSH, but not FSEEK nor       */
        /* fopen (does its own FSEEK)                                       */
        fseek(stream, 0L, SEEK_END);
        if (stream->flag & _IOLAZY) _deferredlazyseek(stream);
        flag = stream->flag;
    }
dbmsg("%s\n", "set IODIRTIED");
    stream->flag = (flag |= (_IODIRTIED+_IONOREADS));    /* we are writing */
    if (stream->base == NULL)
    {   if (_sys_istty(stream)) {         /* terminal - unbuffered  */
            if (stream->flag & (_IOLBF + _IOFBF)) { /* system buffer */
               stream->ptr = stream->base = (unsigned char *)_sys_alloc(stream->bufsiz);
               stream->flag |= (flag |= _IOSBF);
            } else {
                stream->ptr = stream->base = stream->lilbuf;
                stream->bufsiz = 1;
                stream->flag = (flag |= _IONBF);
            }
        } else {
            /* allocate default system buffer */
            stream->ptr = stream->base = (unsigned char *)_sys_alloc(stream->bufsiz);
            stream->flag |= (flag |= _IOSBF);
            if ((flag & _IOLBF+_IOFBF) == 0) stream->flag |= (flag |= _IOFBF);
        }
    }
    if (flag & _IOFBF)               /* system or user buffer */
    {   unsigned char *buff = stream->base;
        int count = EXTENT(stream) - buff;
        if (count != 0) {
            if (_writebuf(buff, count, stream)) return EOF;
        }
        stream->ptr = stream->extent = buff+1;
        stream->ocnt = stream->bufsiz - 1;
        stream->buflim = stream->bufsiz;
        return (*buff = ch);
    }
    else     /* no buffer (i.e. 1 char private one) or line buffer */
    {   unsigned char *buff = stream->base;
        int count;
        *stream->ptr++ = ch;   /* always room */
        count = EXTENT(stream) - buff;
        stream->buflim = stream->bufsiz;
        if ((flag & _IONBF) ||
               (unsigned char)ch == '\n' || count >= stream->bufsiz)
        {   stream->ptr = stream->extent = buff;
            stream->ocnt = 0;                 /* 2^31 is big but finite */
            return _writebuf(buff, count, stream) ? EOF : (unsigned char)ch;
        }
        return (unsigned char)ch;
    }
}

#endif

#if defined filbuf_c || defined SHARED_C_LIBRARY

extern int __filbuf(FILE *stream)
{   int w;
    unsigned char *buff;
    FILEHANDLE fh;
    int request;
dbmsg("%s\n", "__filbuf");
    stream->flag |= _IOACTIVE;
    if (stream->flag & _IOUNGET) {
      stream->icnt = stream->icnt_save;
      stream->ocnt = stream->ocnt_save;
      stream->flag &= ~_IOUNGET;
      return stream->lilbuf[1];
    }

    if (stream->flag & _IOLAZY) _deferredlazyseek(stream);

    /* note that sscanf (q.v.) requires this next line to yield EOF */
    if ((stream->flag & (_IOEOF+_IOERR+_IOSTRG+_IOREAD+_IOPEOF+_IONOREADS))
                                                                    != _IOREAD)
    {   stream->icnt = 0;                      /* 2^31 is big but finite */
        if (stream->flag & _IOEOF+_IOPEOF)
            /* writing ok after EOF read according to ansi */
            stream->flag = stream->flag & ~(_IONOWRITES+_IOPEOF) | _IOEOF;
        else _seterr(stream);
        return EOF;
    }

/*  if ((stream->flag & _IOFBF+_IOSBF+_IONBF+_IOLBF+_IODIRTIED) == 0)
    DVJ + HCM don't understand what this is meant to be/do */
    if ((stream->base == NULL) && ((stream->flag & _IODIRTIED) == 0))
    {
            /* allocate default system buffer */
            stream->ptr = stream->base = (unsigned char *)_sys_alloc(stream->bufsiz);
            stream->flag |= _IOSBF;
            if ((stream->flag & _IOLBF+_IOFBF) == 0) stream->flag |= _IOFBF;
    }

    if (stream->icnt < 0) {
      /* buffer ~empty, came here cos seek wanted to to tidy up flags etc */
dbmsg("fillbuf negative icnt = %d\n", stream->icnt);
      stream->icnt = (-stream->icnt) - 1;
      stream->flag |= _IONOWRITES;           /* we are reading */
      return *(stream->ptr)++;
    }

    fh = stream->file;
    if (stream->flag & _IOSEEK) {
      if (stream->flag & _IODIRTIED) _fflush(stream);
      else {
dbmsg("fillbuf seeking to %d\n", (int)stream->pos);
#ifdef HOSTOS_NEEDSENSURE
        if (stream->flag & _IONEEDSENSURE) {
          _sys_ensure(fh);
          stream->flag &= ~_IONEEDSENSURE;
        }
#endif
        if (_sys_seek(fh, stream->pos) < 0)
        {   _seterr(stream);
            return EOF;
        }
      }
    }

    stream->flag |= _IONOWRITES;           /* we are reading */

    if (stream->flag & _IODIRTIED) {
      int extent = (int) (EXTENT(stream) - stream->base);
      request = stream->bufsiz - extent;
      if (request == 0) {
        _fflush(stream);
        request = stream->bufsiz;
      } else {
dbmsg("fillbuf flag %X\n", stream->flag);
        if ((stream->flag & (_IOBUFREAD+_IOSEEK)) == 0) {
dbmsg("fillbuf dirty buffer, read into end,seeking to %d\n",(int) stream->pos+extent);
          _fflush(stream);    /* LDS 20-Aug-92 To fix _sys_seek past end
                                 of underlying file, faulted on the Mac. */
#ifdef HOSTOS_NEEDSENSURE
          if (stream->flag & _IONEEDSENSURE) {
            _sys_ensure(fh);
            stream->flag &= ~_IONEEDSENSURE;
          }
#endif
          if (_sys_seek(fh, stream->pos + extent) < 0)
          {   _seterr(stream);
              return EOF;
          } else stream->flag |= _IOBUFREAD;
        }
      }
      buff = stream->base + (stream->bufsiz - request);
dbmsg_noNL("fillbuf not splatting out writ part of file request %d ", request);
dbmsg("at buff %X\n", (int)buff);
    } else {
      request = stream->bufsiz;
      buff = stream->base;
      stream->pos += stream->ptr - buff;     /* add buf size for ftell() */
    }
    stream->flag &= ~_IOSEEK;

    /* On reading from an unbuffered or line-buffered streams, flush all */
    /* line-buffered output streams. This is an ANSI requirement for     */
    /* unbuffered inputs; doing it for line-buffered inputs allows the   */
    /* standard streams to be line-buffered without requiring explicit   */
    /* flushes to make prompts appear.                                   */
    if (stream->flag & (_IONBF | _IOLBF))
       _flushlinebuffered();

dbmsg("READING FROM FILE REQUEST = %d\n", request);
dbmsg("filelen = %ld\n",_sys_flen(fh));             /* length of this file */
    w = _sys_read(fh, buff, request, stream->flag);
#ifdef HOSTOS_NEEDSENSURE
    stream->flag &= ~_IONEEDSENSURE;
#endif
dbmsg("_sys_read returned %d\n", w);
    if (w<0) {
      /* either error or EOF */
      if (_sys_iserror(w)) { _seterr(stream); return EOF; }
      /* this deals with operating systems with 'early' eof */
      stream->flag |= _IOPEOF;
      w = w & 0x7fffffff;
    }
    w = request - w;
dbmsg("read %d bytes\n", w);
    stream->extent = buff + w;
    stream->buflim = stream->bufsiz;
    if (w==0)    /* this deals with operating systems with 'late' eof  */
    {   stream->flag |= _IOEOF;                /* is this case independent? */
        stream->flag &= ~_IONOWRITES;          /* writing OK after EOF read */
        stream->icnt = 0;
        stream->ptr = buff;  /* just for fun - NB affects ftell() - check */
        return EOF;
    }
    else
    {   stream->icnt = w-1;
        stream->ptr = buff+1;
        stream->flag |= _IOBUFREAD;
        return(buff[0]);
    }
}

#endif

#if defined backspace_c || defined SHARED_C_LIBRARY

int __backspace(FILE *stream)
{
    if (!(stream->flag & _IOEOF) &&
         (stream->ptr > stream->base))
    {   ++(stream->icnt);
        --(stream->ptr);
        return 0;
    }
    return EOF;
}

#endif

#if defined clearerr_c || defined SHARED_C_LIBRARY

void clearerr(FILE *stream)
{   /* we should do more in 'clearerr' resetting __pos __ptr and _cnt      */
    stream->flag &= ~(_IOEOF+_IOERR+_IOPEOF);
}

#endif

#if defined feof_c || defined SHARED_C_LIBRARY

int (feof)(FILE *stream) { return _feof(stream); }

#endif

#if defined ferror_c || defined SHARED_C_LIBRARY

int (ferror)(FILE *stream) { return _ferror(stream); }

#endif

#if defined fgetc_c || defined SHARED_C_LIBRARY

int (fgetc)(FILE *stream) { _getc(stream); }

#endif

#if defined getc_c || defined SHARED_C_LIBRARY

int (getc)(FILE *stream) { _getc(stream); }

#endif

#if defined getchar_c || defined SHARED_C_LIBRARY

int (getchar)() { _getc(stdin); }

#endif

#if defined fputc_c || defined SHARED_C_LIBRARY

int (fputc)(int ch, FILE *stream) { _putc(ch, stream); }

#endif

#if defined putc_c || defined SHARED_C_LIBRARY

int (putc)(int ch, FILE *stream) { _putc(ch, stream); }

#endif

#if defined putchar_c || defined SHARED_C_LIBRARY

int (putchar)(int ch) { _putc(ch, stdout); }

#endif

#if defined ungetc_c || defined SHARED_C_LIBRARY

int ungetc(int c, FILE *stream)
{   /* made into a fn to evaluate each arg once. */
    if (c==EOF || (stream->flag & (_IOUNGET+_IONOREADS))) return EOF;
    /* put char into unget buffer */
    stream->lilbuf[1] = c;
    stream->flag = (stream->flag | _IOUNGET | _IONOWRITES | _IOACTIVE) & ~(_IOEOF);
    stream->ocnt_save = stream->ocnt;
    stream->icnt_save = stream->icnt;
    stream->icnt = stream->ocnt = 0;
    return c;
}

#endif

#if defined fgets_c || defined SHARED_C_LIBRARY

char *fgets(char *s, int n, FILE *stream)
{   char *a = s;
    if (n <= 1) return NULL;                  /* best of a bad deal */
    do { int ch = getc(stream);
         if (ch == EOF)                       /* error or EOF       */
         {   if (s == a) return NULL;         /* no chars -> leave  */
             if (_ferror(stream)) a = NULL;
             break; /* add NULL even if ferror(), spec says 'indeterminate' */
         }
         if ((*s++ = ch) == '\n') break;
       }
       while (--n > 1);
    *s = 0;
    return a;
}

#endif

#if defined gets_c || defined SHARED_C_LIBRARY

char *gets(char *s)
{   char *a = s;
    for (;;)
    {    int ch = getc(stdin);
         if (ch == EOF)                       /* error or EOF       */
         {   if (s == a) return NULL;         /* no chars -> leave  */
             if (_ferror(stdin)) a = NULL;
             break; /* add NULL even if ferror(), spec says 'indeterminate' */
         }
         if (ch == '\n') break;
         *s++ = ch;
    }
    *s = 0;
    return a;
}

#endif

#if defined fputs_c || defined SHARED_C_LIBRARY

int fputs(const char *s, FILE *stream)
{
    char c;
    while((c = *s++) != 0)
        if(putc(c, stream)==EOF) return(EOF);
    return(0);
}

#endif

#if defined puts_c || defined SHARED_C_LIBRARY

int puts(const char *s)
{
    char c;
    while ((c = *s++) != 0)
       if (putchar(c) == EOF) return EOF;
    return putchar('\n');
}

#endif

#if defined fflush_c || defined SHARED_C_LIBRARY

static int _do_fflush(FILE *stream)
{ /* ignore the effect of a previous unget on the file position indicator by
     using _ftell rather than ftell */
  if (stream->flag & _IOREAD+_IOWRITE)   /* not open */
  { long offset = _ftell(stream);
    int res;
dbmsg("%s\n", "fflush");
    if (stream->flag & _IOLAZY) _deferredlazyseek(stream);
    stream->flag &= ~(_IONOREADS+_IONOWRITES);
    res =_fflush(stream);
    fseek(stream, offset, SEEK_SET);
    return res;
  } else return 0;
}

int fflush(FILE *stream)
{ /* new definition (ANSI May 88) says that if stream == NULL, apply fflush to
     all applicable streams */
   int res = 0;
   if (stream != NULL)
     res =_do_fflush(stream);
   else
   { int i;
     for (i=0; i<_SYS_OPEN; i++)
       if (_do_fflush(&_iob[i]) != 0) res = EOF;
   }
   return res;
}

#endif

#if defined fgetpos_c || defined SHARED_C_LIBRARY

int fgetpos(FILE *stream, fpos_t *pos)
{  pos->__lo = ftell(stream);
   return 0;
}

#endif

#if defined fread_c || defined SHARED_C_LIBRARY

static int _fillb2(FILE *stream)
{   if (__filbuf(stream) == EOF) return EOF;
    stream->icnt++;
    stream->ptr--;
    return 0;
}

/* _read improved to use __filbuf and block moves.  Optimisation
   to memcpy too if word move.  Still possible improvments avoiding copy
   but I don't want to do these yet because of interactions
   (e.g. __pos of a file).   N.B.  _read is not far from unix 'read' */
static int _read(char *ptr, int nbytes, FILE *stream)
{   int i = nbytes;
dbmsg("%s\n", "_read");
    if (i == 0) return 0;
    if (stream->flag & _IOUNGET) {
      *ptr++ = __filbuf(stream); i--;
    }
    if (stream->flag & _IOLAZY) _deferredlazyseek(stream);
    do
    {
dbmsg("in _read, icnt = %d\n", stream->icnt);
        if (i <= stream->icnt)
        {   memcpy(ptr, stream->ptr, i);
            stream->icnt -= i; stream->ptr += i;
            return nbytes;
        }
        else if (stream->icnt > 0)
        {   memcpy(ptr, stream->ptr, stream->icnt);
            ptr += stream->icnt, i -= stream->icnt;
            stream->ptr += stream->icnt, stream->icnt = 0; /* for __pos */
        }
    } while (_fillb2(stream) != EOF);
    return nbytes-i;
/*
    for (i=0; i<nbytes; i++)
    {   if ((ch = getc(stream)) == EOF) return i;
        *ptr++ = ch;
    }
    return nbytes;
*/
}

size_t fread(void *ptr, size_t itemsize, size_t count, FILE *stream)
{    /* ANSI spec says EOF and ERR treated the same as far as fread
      * is concerned and that the number of WHOLE items read is returned.
      */
dbmsg("fread %d\n", count);
    return itemsize == 0 ? 0   /* slight ansi irrationality */
                         : _read((char *)ptr, itemsize*count, stream) / itemsize;
}

#endif

#if defined fseek_c || defined SHARED_C_LIBRARY

/* The treatment of files that can be written to seems complicated in fseek */

int fseek(FILE *stream, long int offset, int whence)
{
    FILEHANDLE fh = stream->file;
    int flag = stream->flag;
dbmsg_noNL("%s ", "SEEK ENTRY");

    if (!(flag & _IOREAD+_IOWRITE+_IOSHARED) || _sys_istty(stream))
        return(2);                              /* fseek impossible  */

    switch(whence)
    {
case SEEK_SET:
        break;                                  /* relative to file start */
case SEEK_CUR:
        offset += ftell(stream);                /* relative seek */
        break;
case SEEK_END:
        {   long int filelen, filepos;
            filelen = _sys_flen(fh);           /* length of this file      */
dbmsg("filelen in seek = %d\n", (int)filelen);
            if (filelen<0)                      /* failed to read length    */
            {   _seterr(stream);
                return 1;
            }
            filepos = stream->pos + EXTENT(stream) - stream->base;
            if (stream->flag & _IOLAZY && filepos < stream->lspos)
              filepos = stream->lspos;
            if (filepos>filelen)                /* only possible on write   */
                filelen = filepos;              /* allow for stuff buffered */
            offset += filelen;                  /* relative to end of file  */
        }
        break;
default:
        _seterr(stream);
        return(2);                              /* illegal operation code   */
    }

    if (offset < 0) { _seterr(stream); return 2; } /* fseek impossible  */

    if ((flag & _IONOREADS) && stream->extent < stream->ptr)
      stream->extent = stream->ptr;

dbmsg_noNL("%s ", "SEEK");
dbmsg_noNL("__pos %d", (int)stream->pos);
dbmsg_noNL(" offset %d", (int)offset);
dbmsg_noNL(" buflim %d", stream->buflim);
dbmsg_noNL(" __ptr %X", (int)stream->ptr);
dbmsg_noNL(" __icnt %d", stream->icnt);
dbmsg_noNL(" __ocnt %d", stream->ocnt);
dbmsg_noNL(" __base %X", (int)stream->base);

    if (offset < stream->pos ||
      offset > stream->pos + EXTENT(stream) - stream->base ||
      offset >= stream->pos + stream->buflim)

      { /* outside buffer */
dbmsg("%s\n", " outside buffer");
      flag |= _IOLAZY;
      stream->icnt = stream->ocnt = 0;
      stream->lspos = offset;
    } else { /* inside buffer */
dbmsg("%s\n", " inside buffer");
      offset -= stream->pos;
      if (flag & _IOWRITE)
        stream->ocnt = -(stream->buflim - (int)offset);
      if (flag & _IOREAD)
        stream->icnt = -((int)(EXTENT(stream) - stream->base) - (int)offset);
      stream->ptr = stream->base + offset;
      flag &= ~_IOLAZY;
    }
dbmsg_noNL("AFTER SEEK __ptr %X", (int)stream->ptr);
dbmsg_noNL(" __icnt %d", stream->icnt);
dbmsg(" __ocnt %d\n", stream->ocnt);
    stream->flag = flag & ~(_IOEOF+_IONOWRITES+_IONOREADS+_IOUNGET);

    return 0;
}

#endif

#if defined fsetpos_c || defined SHARED_C_LIBRARY

int fsetpos(FILE *stream, const fpos_t *pos)
{  int res = fseek(stream, pos->__lo, SEEK_SET);
   if (res) errno = EDOM;
   return res;
}

#endif

#if defined ftell_c || defined SHARED_C_LIBRARY

long int ftell(FILE *stream)
{  long pos;
   if (!(stream->flag & _IOREAD+_IOWRITE))     /* already closed        */
   { errno = EDOM;
     return -1L;
   }
   pos = _ftell(stream);
   if (stream->flag & _IOUNGET && pos > 0) --pos;
   return pos;
}

#endif

#if defined fwrite_c || defined SHARED_C_LIBRARY

static int _write(const char *ptr, int nbytes, FILE *stream)
{   int i;
    for(i=0; i<nbytes; i++)
        if (putc(*ptr++, stream) == EOF) return 0;
        /* H&S say 0 on error */
    return nbytes;
}

size_t fwrite(const void *ptr, size_t itemsize, size_t count, FILE *stream)
{
/* The comments made about fread apply here too */
dbmsg("fwrite %d\n", count*itemsize);
    return itemsize == 0 ? count
                         : _write((char *)ptr, itemsize*count, stream) / itemsize;
}

#endif

#if defined osf_c || defined SHARED_C_LIBRARY

#include "externs.h"

FILE *_fopen_string_file(const char *data, int length)
{
/* open a file that will read data from the given string argument        */
    int i;
    for (i=3; i<_SYS_OPEN; i++)
    {   FILE *stream = &_iob[i];
        if (!(stream->flag & _IOREAD+_IOWRITE))  /* if not open then try it */
        {
            fclose(stream);
            stream->flag = _IOSTRG+_IOREAD+_IOACTIVE;
            stream->ptr = stream->base = (unsigned char *)data;
            stream->icnt = length;
            return stream;
        }
    }
    return 0;   /* no more i/o channels allowed for */
}

#endif

#if defined rewind_c || defined SHARED_C_LIBRARY

void rewind(FILE *stream)
{
   fseek(stream, 0L, SEEK_SET);
   clearerr(stream);
}

#endif

#if defined setvbuf_c || defined SHARED_C_LIBRARY

int setvbuf(FILE *stream, char *buf, int type, size_t size)
{   int flags = stream ->flag;
    unsigned char *ubuf = (unsigned char *) buf;   /* costs nothing */
    if ((!(flags & _IOREAD+_IOWRITE)) || (flags & _IOACTIVE))
        return 1;             /* failure - not open || already active */
    switch (type)
    {   default: return 1;    /* failure */
        case _IONBF:
            ubuf = stream->lilbuf;
            size = 1;
            break;
        case _IOLBF:
        case _IOFBF:
            if (size-1 >= 0xffffff) return 1;  /* unsigned! */
            break;
    }
    stream->ptr = stream->base = ubuf;
    stream->bufsiz = size;
    stream->flag = (stream->flag & ~(_IOSBF+_IOFBF+_IOLBF+_IONBF)) | type;
    return 0;                 /* success */
}

#endif

#if defined setbuf_c || defined SHARED_C_LIBRARY

void setbuf(FILE *stream, char *buf)
{
   (void)setvbuf(stream, buf, (buf!=0 ? _IOFBF : _IONBF), BUFSIZ);
}

#endif

#if defined tmpnam_c || defined SHARED_C_LIBRARY

#include <time.h>

static char _tmp_file_name[L_tmpnam] = "";
static int _tmp_file_ser = 0;

int _tmpfile_delete(FILE *stream, int res)
{
    char name[L_tmpnam];
    _sys_tmpnam(name, stream->signature, L_tmpnam);
    return (remove(name) != 0) ? EOF : res; /* delete the file if pos */
}

char *tmpnam(char *a)
{
/* Obtaining an unique name is tolerably nasty - what I do here is       */
/* derive the name (via _sys_tmpnam_())                                  */
/* from an integer that is constructed out of a serial number combined   */
/* with the current clock setting. An effect of this is that the file    */
/* name can be reconstructed from a 32-bit integer for when I want to    */
/* delete the file.                                                      */
    int signature = ((int)time(NULL) << 8) | (_tmp_file_ser++ & 0xff);
    if (a==NULL) a = _tmp_file_name;
    _sys_tmpnam(a, signature, L_tmpnam);
    return a;
}

FILE *tmpfile()
{
    char name[L_tmpnam];
    int signature = ((int)time(NULL) << 8) | (_tmp_file_ser++ & 0xff);
    FILE *f;
    _sys_tmpnam(name, signature, L_tmpnam);
    f = fopen(name, "w+b");
    if (f)
    {   f->flag |= _IODEL;
        f->signature = signature;
    }
    return f;
}

#endif

#if defined __dup_c || defined SHARED_C_LIBRARY

int __dup(int newi, int oldi)
{   FILE *s_new, *s_old;
    (void) fclose(&_iob[newi]);
    s_new = &_iob[newi];
    s_old = &_iob[oldi];
    s_old->flag |= _IOSHARED;
    s_new->flag = s_old->flag & (_IOREAD+_IOWRITE+_IOAPPEND+_IOSHARED+
                                 _IOSBF+_IOFBF+_IOLBF+_IONBF);
    s_new->file = s_old->file;
    return /*s_new->file*/ oldi;
}

#endif

#if defined _sys_msg_c || defined SHARED_C_LIBRARY

#pragma -s1
/* stack checking off to allow use from SIGSTAK handler */

void _sys_msg(const char *s, int flag)
{   /* write out s carefully for intimate system use.                      */
    if (
#ifndef SHARED_C_LIBRARY
        /* check for inclusion of weakly-referenced fn in image */
        _sys_msg_via_stderr == NULL ||
#endif
        _init_flags.error_recursion ||
        !_sys_msg_via_stderr(s, flag) /* attempt to use stderr failed */)

    {   if (flag & NL_PRE) _ttywrch('\n');
        while (*s != 0) _ttywrch(*s++);
        if (flag & NL_POST) _ttywrch('\n');
    }
}

#pragma -s0

#endif

#if defined perror_c || defined SHARED_C_LIBRARY

void perror(const char *s)
{   char b[80];
    if (s != 0 && *s != 0) fprintf(stderr, "%s: ", s);
    fprintf(stderr, "%s\n", _strerror(errno, b));
}

#endif

#if defined fisatty_c || defined SHARED_C_LIBRARY

#include "externs.h"

int _fisatty(FILE *stream)  /* not in ANSI, but related needed for ML */
{   if ((stream->flag & _IOREAD) && _sys_istty(stream)) return 1;
    return 0;
}

#endif

/* end of stdio.c */
