/* hostsys.h: specify details of host system when compiling CLIB
 * Copyright (C) Codemist Ltd, 1988
 * Copyright (C) Advanced RISC Machines Ltd., 1991
 *
 * RCS $Revision: 1.4 $
 * Checkin $Date: 1996/05/24 14:58:02 $
 * Revising $Author: mgray $
 *
 */

#pragma force_top_level
#pragma include_only_once

#ifndef __hostsys_h
#define __hostsys_h

#include <stdio.h>

/* The host-system-specific things which need to be implemented to       */
/* rehost the library.                                                   */

/* First, things declared in standard ANSI headers whose implementation  */
/* is completely host-system-specific                                    */

/* from time.h   */

#include <time.h>

extern clock_t clock(void);
extern time_t time(time_t *timer);

/* from stdio.h  */

extern int remove(const char *pathname);
extern int rename(const char *oldname, const char *newname);

/* from stdlib.h */

extern char *getenv(const char *name);
extern int system(const char *string);

/* clock() and getenv() may need initialisation: if external functions   */
/* with the following names are defined, they will be called during the  */
/* library's initialisation.  They need not be defined if not needed.    */

___weak void _clock_init(void);
___weak void _getenv_init(void);

/* and now things needed to support the host-system-independent part of  */
/* the library.                                                          */

/* I/O things ... */

#define TTYFILENAME ":tt"
/* The filename to be used in opening a file to terminal.  Only required */
/* (in the host-independent part of the library) to initialise stdin,    */
/* stdout and stderr in __rt_lib_init.                                   */

/* #define HOSTOS_NEEDSENSURE 1 */
/* The host os requires an ensure operation to flush os file buffers to  */
/* disc if an os write is followed by an os read which requires a seek   */
/* (the flush happens before the seek).                                  */
/* Legacy of a RISCOS file-buffering bug, unlikely to be wanted in any   */
/* rehosting.                                                            */

/* the value which identifies an open file to the host system:           */
/* returned by _sys_open, and used in subsequent _sys_ io calls          */
/* There is at least one distinguished value, NONHANDLE, the return of   */
/* which from _sys_open() indicates that the open failed                 */
typedef int FILEHANDLE;

#define NONHANDLE (-1)

extern FILEHANDLE _sys_open(const char *name, int openmode);

/* openmode is a bitmap, whose bits have the following significance ...  */
/* most correspond directly to the ANSI mode specification - the         */
/* exception is OPEN_T, an extension which requests timestamp update.    */
/* This is really a sop to implementation laziness: what is intended is  */
/* that the timestamp should be updated if the file is written to or     */
/* otherwise modified.  But that is known only when the file is closed   */
/* and Riscos has no 'set timestamp given filehandle' operation and the  */
/* name by which the file was opened may no longer be valid at the time  */
/* of its close.                                                         */
#define OPEN_R 0
#define OPEN_W 4
#define OPEN_A 8
#define OPEN_B 1
#define OPEN_PLUS 2
#define OPEN_T 16

extern int _sys_close(FILEHANDLE fh);
/* result is 0 or an error indication                                    */

extern int _sys_write(FILEHANDLE fh, const unsigned char *buf,
                      unsigned len, int mode);
/* result is number of characters not written (ie non-0 denotes a        */
/* failure of some sort) or a negative error indicator.                  */

extern int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode);
/* result is number of characters not read (ie len - result _were_ read),*/
/* or negative to indicate error or EOF.                                 */
/* _sys_iserror(result) distinguishes between the two possibilities.     */
/* (Redundantly, since on EOF it is required that (result & ~0x80000000) */
/* is the number of characters unread).                                  */

extern int _sys_iserror(int status);

extern void _ttywrch(int ch);
/* Write a character to the VDU.  Used (in the host-independent part of  */
/* the library) in the last-ditch error reporter, used when writing to   */
/* stderr is believed to have failed, or to be unsafe (eg in default     */
/* SIGSTK handler).                                                      */

extern int _sys_istty(FILE *f);
/* Return true if the argument file is connected to a terminal.          */
/* Used to:  provide default unbuffered behaviour (in the absence of a   */
/*           setbuf call).                                               */
/*           disallow seek                                               */

extern int _sys_seek(FILEHANDLE fh, long pos);
/* Position the file at offset pos from its beginning.                   */
/* Result is >= 0 if OK, negative for an error.                          */

extern int _sys_ensure(FILEHANDLE fh);
/* Flush any OS buffers associated with fh, ensuring that the file is    */
/* up to date on disc.  (Only required if HOSTOS_NEEDSENSURE; see above) */
/* Result is >= 0 if OK, negative for an error.                          */

extern long _sys_flen(FILEHANDLE fh);
/* Return the current length of the file fh (or a negative error         */
/* indicator).  Required to convert fseek(, SEEK_END) into (, SEEK_START)*/
/* as required by _sys_seek.                                             */

extern int _sys_tmpnam(char *name, int sig, unsigned maxlen);
/* Return the name for temporary file number sig in the buffer name.     */
/* Returns 0 on failure. maxlen is the maximum name length allowed.      */

extern char *_hostos_error_string(int no, char *buf);

extern char *_hostos_signal_string(int no);

extern char *__rt_command_string(void);
/* Returns the address of (maybe a copy of) the string used to invoke    */
/* the program                                                           */

extern int __rt_fpavailable(void);
/* Returns 0 if floating point is not available (no emulator nor         */
/* hardware)                                                             */

extern void __rt_exit(int);
/* finalise library (not including calling atexit() handlers), then      */
/* return to OS with argument as completion code.                        */
/* return to OS with argument as completion code.                        */

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
/* Handle fault (processor detected trap, enabled fp exception, or the   */
/* like).  The argument register set describes the processor state at    */
/* the time of the fault, with the pc value addressing the faulting      */
/* instruction (except perhaps in the case of imprecise fp faults).      */

extern void __rt_exittraphandler(void);
/* Declare no longer within trap handler (reset any scheme to prevent    */
/* recursive trap handler invocation).                                   */

extern int __sys_istty(FILEHANDLE fh);
extern int __sys_tmpnam(char *name, unsigned maxlen);

#endif

/* end of hostsys.h */
