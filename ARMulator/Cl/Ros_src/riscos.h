#pragma force_top_level
#pragma include_only_once

/*
 *      Interface to host OS.
 *      Copyright (C) Acorn Computers Ltd., 1988
 */

#ifndef __riscos_h
#define __riscos_h

#ifndef __size_t
#  define __size_t 1
   typedef unsigned int size_t;   /* from <stddef.h> */
#endif

typedef struct {
   int r[10];            /* only r0 - r9 matter for swi's */
} __riscos_swi_regs;

typedef struct {
   int load, exec;       /* load, exec addresses */
   int start, end;       /* start address/length, end address/attributes */
} __riscos_osfile_block;

typedef struct {
   void * dataptr;       /* memory address of data */
   int nbytes, fileptr;
   int buf_len;          /* these fields for Arthur gpbp extensions */
   char * wild_fld;      /* points to wildcarded filename to match */
} __riscos_osgbpb_block;

typedef struct {
   unsigned errnum;      /* error number */
   char errmess[252];    /* error message (zero terminated) */
} __riscos_error;

extern void __riscos_raise_error(__riscos_error *);
/* return the specified error to the parent */

#define __riscos_NONX 0x80000000
extern __riscos_error *__riscos_swi(int no, __riscos_swi_regs *in,
                                    __riscos_swi_regs *out);
/*
 *  Generic SWI interface.  Returns NULL if there was no error.
 *  The SWI called normally has the X bit set.  To call a non-X bit set SWI,
 *  __riscos_NONX must be orred into no (in which case, if an error occurs,
 *  __riscos_swi does not return).
 */

extern __riscos_error *__riscos_swi_c(int no, __riscos_swi_regs *in,
                                      __riscos_swi_regs *out, int *carry);
/*
 *  As _kernel_swi, but for use with SWIs which return status in the C flag.
 *  The int to which carry points is set to reflect the state of the C flag on
 *  exit from the SWI.
 */

#define __riscos_ERROR (-2)

extern int __riscos_osbyte(int op, int x, int y);
/*
 *  Performs an OSByte operation.
 *  If there is no error, the result contains
 *     the return value of R1 (X) in its bottom byte
 *     the return value of R2 (Y) in its second byte
 *     1 in the third byte if carry is set on return, otherwise 0
 *     0 in its top byte
 *  (Not all of these values will be significant, depending on the
 *   particular OSByte operation).
 */

extern int __riscos_osrdch(void);
/*
 *  Returns a character read from the currently selected OS input stream
 */

extern int __riscos_oswrch(int ch);
/*
 *  Writes a byte to all currently selected OS output streams
 *  The return value just indicates success or failure.
 */

extern int __riscos_osbget(unsigned handle);
/*
 *  Returns the next byte from the file identified by 'handle'.
 *  (-1 => EOF).
 */

extern int __riscos_osbput(int ch, unsigned handle);
/*
 *  Writes a byte to the file identified by 'handle'.
 *  The return value just indicates success or failure.
 */

extern int __riscos_osgbpb(int op, unsigned handle, __riscos_osgbpb_block *inout);
/*
 *  Reads or writes a number of bytes from a filing system.
 *  The return value just indicates success or failure.
 *  Note that for some operations, the return value of C is significant,
 *  and for others it isn't.  In all cases, therefore, a return value of -1
 *  is possible, but for some operations it should be ignored.
 */

extern int __riscos_osword(int op, int *data);
/*
 *  Performs an OSWord operation.
 *  The size and format of the block *data depends on the particular OSWord
 *  being used; it may be updated.
 */

extern int __riscos_osfind_open(int op, char *name);
extern int __riscos_osfind_close(int op, int handle);
/*
 *  Opens or closes a file.
 *    Open returns a file handle (0 => open failed without error)
 *    Close the return value just indicates success or failure
 */

extern int __riscos_osfile(int op, const char *name, __riscos_osfile_block *inout);
/*  Performs an OSFile operation, with values of r2 - r5 taken from the osfile
 *  block.  The block is updated with the return values of these registers,
 *  and the result is the return value of r0 (or an error indication)
 */

extern int __riscos_osargs(int op, unsigned handle, int arg);
/*
 *  Performs an OSArgs operation.
 *  The result is an error indication, or
 *    the current filing system number (if op = handle = 0)
 *    the value returned in R2 by the OSArgs operation otherwise
 */

extern int __riscos_oscli(const char *s);
/*
 *  Hands the argument string to the OS command line interpreter to execute
 *  as a command.  This should not be used to invoke other applications:
 *  _kernel_system exists for that.  Even using it to run a replacement
 *  application is somewhat dubious (abort handlers are left as those of the
 *  current application).
 *  The return value just indicates error or no error
 */

extern __riscos_error *__riscos_last_oserror(void);
/*
 *  Returns a pointer to an error block describing the last os error since
 *  _kernel_last_oserror was last called (or since there program started
 *  if there has been no such call).  If there has been no os error, returns
 *  a null pointer.  Note that occurrence of a further error may overwrite the
 *  contents of the block.
 *  If _kernel_swi caused the last os error, the error already returned by that
 *  call gets returned by this too.
 */

extern int __riscos_system(const char *string, int chain);
/*
 *  Hands the argument string to the OS command line interpreter to execute.
 *  If chain is 0, the calling application is copied to the top of memory first,
 *    then handlers are installed so that if the command string causes an
 *    application to be invoked, control returns to _kernel_system, which then
 *    copies the calling application back into its proper place - the command
 *    is executed as a sub-program.  Of course, since the sub-program executes
 *    in the same address space, there is no protection against errant writes
 *    by it to the code or data of the caller.  And if there is insufficient
 *    space to load the sub-program, the manner of the subsequent death is
 *    unlikely to be pretty.
 *  If chain is 1, all handlers are removed before calling the CLI, and if it
 *    returns (the command is built-in) _kernel_system Exits.
 *  The return value just indicates error or no error
 */

extern __riscos_error *__riscos_getenv(const char *name, char *buffer, unsigned size);
/*
 *  Reads the value of a system variable, placing the value string in the
 *  buffer (of size size).
 *  (This just gives access to  OS_ReadVarVal).
 */

extern __riscos_error *__riscos_setenv(const char *name, const char *value);
/*
 *  Updates the value of a system variable to be string valued, with the
 *  given value (value = NULL deletes the variable)
 */

typedef int __riscos_ExtendProc(int /*n*/, void** /*p*/);
extern __riscos_ExtendProc *__riscos_register_slotextend(__riscos_ExtendProc *proc);
/* When the initial heap is full, the kernel is normally capable of extending
   it (if the OS will allow).  However, if the heap limit is not the same as
   the OS limit, it is assumed that someone else has acquired the space between,
   and the procedure registered here is called to request n bytes from it.
   Its return value is expected to be >= n or = 0 (failure); if success,
   *p is set to point to the space returned.
  */

extern int _kernel_escape_seen(void);
/*
 * Returns 1 if there has been an escape since the previous call of
 * _kernel_escape_seen (or since program start, if there has been no
 * previous call).  Escapes are never ignored with this mechanism,
 * whereas they may be with the language EventProc mechanism since there
 * may be no stack to call it on.
 */

#endif
/* end of riscos.h */
