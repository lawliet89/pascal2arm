#ifndef __ioguts_h
#define __ioguts_h 1

/* ioguts.h: internals of the I/O subsystem                     */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 1995/04/04 15:12:16 $
 * Revising $Author: hmeekings $
 */
#define _iob stdin

struct __FILE
{ int icnt;      /* two separate _cnt fields so we can police ...        */
  unsigned char *ptr;
  int ocnt;      /* ... restrictions that read/write are fseek separated */
  int flag;
  unsigned char *base; /* buffer base */
  FILEHANDLE file;     /* RISCOS/Arthur/Brazil file handle */
  long pos;            /* position in file */
  int bufsiz;          /* maximum buffer size */
  int signature;       /* used with temporary files */

  unsigned char lilbuf[2]; /* single byte buffer for them that want it  */
                            /* plus an unget char is put in __lilbuf[1]   */
  long lspos;              /* what __pos should be (set after lazy seek) */
  unsigned char *extent;   /* extent of writes into the current buffer  */
  int buflim;              /* used size of buffer                       */
  int icnt_save;           /* after unget contains old icnt             */
  int ocnt_save;           /* after unget contains old ocnt             */
  int (*putc)(int, FILE *);
};

/* Explanation of the _IOxxx bits:                                       */
/* IONOWRITES says that the file is positioned at the end of the current */
/* buffer (as when performing sequential reads), while IODIRTIED         */
/* indicates that its position is at the start of the current buffer (as */
/* in the usual case when writing a file). When the relevant bit is not  */
/* set and a transfer is needed _sys_seek() is used to reposition the    */
/* file. Now extra bit _IOSEEK indicating that repositioning of the file */
/* is required before performing I/O                                     */
/* Extra bit _IOLAZY added to indicate that a seek has been requested    */
/* but has been deferred until the next fill or flush buffer             */
/* I use IONOWRITES and IONOREADS to check the restriction               */
/* that read and write operations must be separated by fseek/fflush.     */

/* N.B. bits up to (and including 0xfff) in <stdio.h>, as below:         */
#define _IOREAD           0x01     /* open for input */
#define _IOWRITE          0x02     /* open for output */
#define _IOBIN            0x04     /* binary stream */
#define _IOSTRG           0x08     /* string stream */
#define _IOSEEK           0x10     /* physical seek required before IO */
#define _IOLAZY           0x20     /* possible seek pending */
#define _IOEOF            0x40     /* end-of-file reached */
#define _IOERR            0x80     /* error occurred on stream */
/* The next three macros are also defined in stdio.h                     */
/* It would be nice to leave them here, as a check that the values in    */
/* stdio.h are the same, but this causes a warning about the identical   */
/* redefinition                                                          */
#if 0
#define _IOFBF           0x100     /* fully buffered IO */
#define _IOLBF           0x200     /* line buffered IO */
#define _IONBF           0x400     /* unbuffered IO */
#endif
#define _IOSBF           0x800     /* system allocated buffer */
#define _IOAPPEND      0x08000     /* must seek to eof before write */

#define _IONOWRITES 0x00001000     /* last op was a read                 */
#define _IONOREADS  0x00002000     /* last op was a write                */
#define _IOPEOF     0x00004000     /* 'pending' EOF                      */
#define _IODIRTIED  0x00010000     /* buffer has been written to         */
#define _IOBUFREAD  0x00020000     /* indicates that the buffer being    */
                                   /* written was read from file, so a   */
                                   /* seek to start of buffer is required*/
#define _IONEEDSENSURE 0x00040000  /* indicates that the underlying      */
                                   /* system buffer was last written to  */
                                   /* and an ensure should be done if    */
                                   /* a seek is about to happen before   */
                                   /* the next read is performed         */
                                   /* (Only if HOSTOS_NEEDSENSURE).      */
#define _IOUNGET    0x00080000     /* last op was an unget               */
#define _IOSHARED   0x00100000     /* 2 streams share the same file      */
#define _IOACTIVE   0x00400000     /* some IO operation already performed*/
#define _IODEL      0xad800000     /* for safety check 9 bits            */
#define _IODELMSK   0xff800000

___weak int _tmpfile_delete(FILE *stream, int res);

#endif
