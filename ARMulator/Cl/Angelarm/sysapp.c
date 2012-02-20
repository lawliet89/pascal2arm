/*> sys.c <*/
/*---------------------------------------------------------------------------*/
/*
 * Target _sys_ veneer function definitions
 *
 * $Revision: 1.5 $
 *   $Author: amerritt $
 *     $Date: 1996/11/25 15:02:10 $
 *
 * Copyright (C) 1995 Advanced RISC Machines Limited. All rights reserved.
*/
/****************************************************************************
  These are veneers for low level potentially OS/host specific functions
  needed by the C Library.  The functions are called indirectly via the
  _syscall function. The reason for this is to avoid having to link each
  application with Angel.
****************************************************************************/ 

#undef FILEHANDLE
#include <string.h>
#include "hostsys.h"
#include "ioguts.h"
#include "errno.h"
#include "arm.h"


#if defined(__thumb)
#define SYSCALL _syscall16
#else
#define SYSCALL _syscall
#endif
extern word SYSCALL(word ,word *);

/* Open file 'name' in mode 'openmode'. */
extern FILEHANDLE _sys_open(const char *name, int openmode)
{
    word args[3];
    FILEHANDLE ret;
    args[0]=(word)name;
    args[1]=(word)openmode;
    args[2]=(word)strlen(name);
    ret= (FILEHANDLE)SYSCALL(SYS_OPEN,args);
    if (ret == NONHANDLE) __errno = (int) SYSCALL(SYS_ERRNO,(word *)0);
    return ret;
}

/* Close file associated with 'fh'. */
extern int _sys_close(FILEHANDLE fh)
{
        word arg;
        int ret;
        arg=(word)fh;
        ret = SYSCALL(SYS_CLOSE,&arg);
        if (ret) __errno = (int) SYSCALL(SYS_ERRNO,(word *)0);
        return ret;
}

/* Write 'len' characters of 'buf' to file associated with 'fh' in 'mode'
   Returns the number of characters NOT written, i.e. 0==NoError.           */ 
/* assumes len is no of bytes rather than words, doesn't check that its 
   right */
extern int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len,
                      int flag)
{
        word args[4];
        int ret;

        IGNORE(flag);
        args[0]=(word)fh;
        args[1]=(word)buf;
        args[2]=(word)len;
        ret = SYSCALL(SYS_WRITE,args);
        if (ret != 0)  __errno = (int) SYSCALL(SYS_ERRNO,(word *)0);
        return ret;
}

/* Read 'len' characters of 'buf' from file associated with 'fh' in 'mode'. 
   Returns the number of characters NOT read, i.e. 0==NoError.              */ 
/* Note: Assumes that buf is large enough to hold the number of bytes read */
extern int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode) 
{
        word args[4];
        int ret;
        args[0]=(word)fh;
        args[1]=(word)buf;
        args[2]=(word)len;
        args[3]=(word)mode;
        ret = SYSCALL(SYS_READ,args);
        if (ret != 0)  __errno = (int) SYSCALL(SYS_ERRNO,(word *)0); 
        return ret;
}

/* Return TRUE if status value indicates an error. */
extern int _sys_iserror(int status)
{
        word arg;
        arg=(word)status;
        return SYSCALL(SYS_ISERROR,&arg);
}

/* Returns non-zero if the file is connected to an interactive device. */
extern int _sys_istty(FILE *f)
{
        word arg;
        arg=(word)(f->file);
        return SYSCALL(SYS_ISTTY,&arg);
}

/* Seeks to position 'pos' in the file associated with 'fh'.  Returns a
   negative value if there is an error, otherwise >=0.                  */ 
extern int _sys_seek(FILEHANDLE fh, long pos)
{
        word args[2];
        int ret;
        args[0]=(word)fh;
        args[1]=(word)pos;
        ret = SYSCALL(SYS_SEEK,args);
        if (ret< 0) __errno = (int) SYSCALL(SYS_ERRNO,(word *)0);  
        return ret;
}

/* Flushes any buffers associated with fh and ensures that the file is 
   up to date on the backing store medium.  The result is >=0 if OK, 
   negative for an error. */
extern int _sys_ensure(FILEHANDLE fh)
{
        word arg;
        int ret;
        arg=(word)fh;
        ret = SYSCALL(SYS_ENSURE,&arg);
        if (ret<0) __errno = (int) SYSCALL(SYS_ERRNO,(word *)0);
        return ret;
}

/* Returns length of the file fh ( or a negative error indicator). */
extern long _sys_flen(FILEHANDLE fh)
{
        word arg;
        arg=(word)fh;
        return SYSCALL(SYS_FLEN,&arg);
}

/* Returns the name for a temporary file number fileno in the buffer name */
/* NOTE: header unclear leave atm */

extern int _sys_tmpnam(char *name, int sig, unsigned maxlen)
{
        word args[3];
        args[0]=(word)name;
        args[1]=(word)sig;
        args[2]=(word)maxlen;
        return SYSCALL(SYS_TMPNAM,args);
}

extern void _ttywrch(int ch)
{
        
        (void)SYSCALL(SYS_WRITEC,(word *)&ch);
}

extern int system(const char *string)
{
        word args[2];
        int ret;
        args[0]=(word)string;
        args[1]=(word)strlen(string);
        ret = (int)SYSCALL(SYS_SYSTEM,args);
        if (ret) __errno = SYSCALL(SYS_ERRNO,(word *)0);
        return ret;
}


extern int remove(const char *pathname)
{
        word args[2];
        int ret;
        args[0]=(word)pathname;
        args[1]=(word)strlen(pathname);
        ret = (int)SYSCALL(SYS_REMOVE,args);
        if (ret) __errno = SYSCALL(SYS_ERRNO,(word *)0); 
        return ret;
}

extern int rename(const char *oldname,const char *newname)
{
        word args[4];
        int ret;
        args[0]=(word)oldname;
        args[1]=(word)strlen(oldname);
        args[2]=(word)newname;
        args[3]=(word)strlen(newname);
        ret = (int)SYSCALL(SYS_RENAME,args);
        if (ret) __errno = SYSCALL(SYS_ERRNO,(word *)0);
        return ret;
}

static char command_string[256];
extern char *__rt_command_string(void)
{
        word args[2];
        args[0]=(word)&command_string[0];
        args[1]=(word)sizeof(command_string);
        if (SYSCALL(SYS_GET_CMDLINE,args)) return NULL;
        return command_string;
}

static clock_t _clock_init_value;

extern clock_t clock(void)
{
        return (clock_t) SYSCALL(SYS_CLOCK,(word *)0)-_clock_init_value;
}

extern void _clock_init(void)
{
        _clock_init_value = (clock_t) SYSCALL(SYS_CLOCK,(word *)0);
}

extern time_t time(time_t *timer)
{
        time_t retval;
        retval = (time_t ) SYSCALL(SYS_TIME,(word *)0);
        if (timer != (time_t *)0) *timer = retval;
        return retval;
}

extern char *getenv(const char *name)
{
        IGNORE(name);
        return NULL;
}
