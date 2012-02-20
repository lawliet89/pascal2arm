/*> sys.c <*/
/*---------------------------------------------------------------------------*/
/*
 * Target _sys_ function definitions
 *
 * $Revision: 1.13.6.5 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:54:55 $
 *
 * Copyright Advanced RISC Machines Limited, 1995.
 * All Rights Reserved
 */

/****************************************************************************
  These are functions needed by the C Library.  They are implemented by
  sending messages to the host.  The message format is defined in sys.h, see
  also channels.h.  They are not linked into an application but
  called indirectly via a veneer and an undefined instruction trap. This
  calls SysLibraryHandler which in turn calls the relevant routine.
  The veneers package up their arguments and pass them to the handler.
  Strings are passed along with their lengths. This aoids needed a strlen
  function inside the Angel system. It does use its own __rt_memcpy routine
  though.
****************************************************************************/

/****************************************************************************

     BE VERY CAREFUL IN THIS MODULE TO RESPECT THE ENDIANNESS OF VALUES

     In particular:
     
      - all values passed to _sys_ functions, excepting sys_writec and
        sys_write0, are in little endian form.

      - values passed to SysLibraryHandler are raw target registers, and
        are thus in target endianness. Embedded ICE therefore can't access
        these locations directly.
        
        sys_writec and sys_write0 use target-endianness parameters.

      - Values palaced in packets (e.g. via sys_build_transaction) MUST be
        in little endian form.

      - Embedded ICE can be assumed to be running in little endian mode.

 ****************************************************************************/


#include <stdarg.h>

#include "angel.h"
#include "adp.h"
#include "arm.h"
#include "logging.h"
#include "msgbuild.h"    /* General support functions for message handling */
#include "support.h"
#include "time.h"
#include "sys.h"
#include "endian.h"
#include "debug.h" 
#include "stacks.h"

#ifdef ICEMAN_LEVEL_3
#  include "debugos.h"          /* angelOS_MemRead()/Write() */
#endif

#define FILEHANDLE int

/*
 * External function prototypes not in .h files
 */
/* These should be from bytesex.h but that uses host.h! */
int32 bytesex_hostval(int32 v);


#ifdef ICEMAN2
static unsigned target_top_of_memory = (512 * 1024);
static word bytes_transferred;

/* we need to use MemRead and MemWrite to access target memory */
# define MEMCPY_FROM_TARGET( d, s, l )                                  \
         angelOS_MemRead( -1, -1, (word)(s), (word)(l), (byte *)(d),        \
                         &bytes_transferred )
# define MEMCPY_TO_TARGET( d, s, l )                                    \
         angelOS_MemWrite( -1, -1, (word)(d), (word)(l), (byte *)(s),       \
                         &bytes_transferred )

#else

/* we can just use __rt_memcpy to access target memory */

# define MEMCPY_FROM_TARGET( d, s, l )  __rt_memcpy( (d), (s), (l) )
# define MEMCPY_TO_TARGET( d, s, l )    __rt_memcpy( (d), (s), (l) )

/* and __rt_strlen to implement strlen() */

#endif

static int sys_handler_running=0;


static int _sys_do_transaction(p_Buffer request, unsigned req_len,
                               p_Buffer *reply,
                               word expectedReason, word *data)
{
    p_Buffer rbuff = request;
    word reasoncode, DebugID, OSInfo1, OSInfo2;
    ChanError err;

    err = angel_ChannelSendThenRead(CH_DEFAULT_DEV, CI_CLIB, &rbuff, &req_len);
    if (err == CE_OKAY)
    {
        unpack_message(BUFFERDATA(rbuff), "%w%w%w%w%w",  &reasoncode,
                       &DebugID, &OSInfo1, &OSInfo2, data);

        if (reasoncode == expectedReason)
        {
            if (reply==NULL)
                angel_ChannelReleaseBuffer(rbuff);
            else
                *(p_Buffer *)reply=rbuff;
            return 0;
        }
        else
        {
            LogWarning(LOG_SYS, ("_sys_do_transaction: unexpected reason %d\n", reasoncode));
            angel_ChannelReleaseBuffer(rbuff);
            return -1;
        }
    }
    LogWarning(LOG_SYS, ("SendThenRead failed %d\n", err));
    return -1;
}

static int _sys_build_and_transact(p_Buffer *reply, word expectedReason,
                                   word *data,
                                   char *format, ...)
{
    va_list    args;
    p_Buffer   request;
    unsigned   length;

    request = angel_ChannelAllocBuffer( Angel_ChanBuffSize );
    if (request != NULL)
    {
        va_start( args, format );
        length = vmsgbuild( BUFFERDATA(request), format, args );
        va_end( args );

        return _sys_do_transaction( request, length, reply,
                                    expectedReason, data );
    }
    else
    {
        LogWarning(LOG_SYS, ("_sys_build_and_transact: no build buffer\n"));
        return -1;
    }
}

/* Open file 'name' in mode 'openmode'. */
static FILEHANDLE _sys_open(const char *name, int openmode, int namelen)
{
    /* note openmode translation from char to int must have taken place
     * earlier on */
    int count;
    p_Buffer pbuff;

    FILEHANDLE handle;

    pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (pbuff!=NULL) {
        count = msgbuild(BUFFERDATA(pbuff),"%w%w%w%w%w", CL_Open|TtoH,
                         0, ADP_HandleUnknown, ADP_HandleUnknown, namelen);
        MEMCPY_FROM_TARGET((char *)BUFFERDATA(pbuff)+count, name,namelen+1);
        count += namelen+1;  /* allow for trailing NULL */
        PUT32LE((BUFFERDATA(pbuff)+count), openmode);
        count+=4;

        if ( _sys_do_transaction(pbuff, count,
                                 NULL,
                                 CL_Open|HtoT, (word *)&handle) )
            return -1;
        else
            return handle;
    }
    return -1;
}

/* Close file associated with 'fh'. */
static int _sys_close(FILEHANDLE fh)
{
    int status;

    if (_sys_build_and_transact(NULL, CL_Close|HtoT, (word *)&status,
                                "%w%w%w%w%w", CL_Close|TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown, fh))
        return -1;
    else
        return status;
}

/*
 * this routine makes the CL_WriteC Clib request; 'data' is a TARGET MEMORY
 * pointer to the character to send (i.e. data[0] is the character required)
 */
static int _sys_writeC(unsigned char *data)
{
    int      status;
    unsigned char ch = 0;

    if (data != NULL)
    {
        /* copy the character to local variable ch */
        MEMCPY_FROM_TARGET((char *)&ch, (char *)data, 1);

        /* send that byte as the argument to CL_WriteC */
        if (_sys_build_and_transact(NULL, CL_WriteC|HtoT, (word *)&status,
                                    "%w%w%w%w%b",
                                    CL_WriteC|TtoH, 0,
                                    ADP_HandleUnknown, ADP_HandleUnknown,
                                    ch))
            return -1;
        else
            return status;
    }
    else
        return -1;
}

/*
 * this routine makes the CL_Write0 Clib request; 'data' is a TARGET MEMORY
 * pointer to the string to send (i.e. data[0] is the first character of the
 * string), and the string is null terminated.
 *
 * NOTE: Various pieces of documentation say SWI write0 takes a pointer
 * to a pointer to a string... this *was* correct but is no longer so.
 *
 * NOTE2: The documentation does not state what the behavoiour for long
 * (>240ish) byte strings is. prior to this change, they would have corrupted
 * someother buffer, with possibly bad consequences; now, multiple CL_Write0
 * packets are sent.
 */
static int _sys_write0(unsigned char *data)
{
    int totlen;
    /* Max number of chars which will fit in a buffer, excluding
     * the zero terminator. 5 * 4 is the number of bytes in the
     * header, '-1' is to allow for null termination.
     */
    const int maxlen = (Angel_ChanBuffSize - (5 * 4)) - 1;
    int count, status;
    int len;
    int left;

    if (data != NULL)
    {        
#ifdef ICEMAN2
        int atend = FALSE;
        char buf[16], *p;
        
        /* implement strlen() for strings in TARGET memory. We need to know the
         * length of the string before we copy any of it to a packet buffer,
         * hence this code (otherwise we could have copied the whole string
         * only once).
         */
        totlen = 0;
        while(!atend)
        {
            MEMCPY_FROM_TARGET(buf, (char *)data + totlen, sizeof(buf));
            for(p = buf; p < (buf + sizeof(buf)); p++)
            {
                if (*p == '\0')
                {
                    atend = TRUE;
                    break;
                }
                totlen++;
            }
        }
#else
        /*
         * For direct angel, we can just do it the traditional way...
         */
        totlen = __rt_strlen((char*)data);
#endif
        
        left = totlen;
        status = 0;
        
        while (left > 0)
        {
            char *string;
            p_Buffer pbuff;

            /* make len the number of chars we'll print with this write;
             * subtracting it from 'left' gives the number still to go.
             */
            len = (left >= maxlen) ? maxlen : left;
            left = left - len;

            pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
            if (pbuff == NULL)
                return -1;

            /* construct the basic message. the return value count should
             * always be 20 (5 words at 4 bytes/word)
             */
            count = msgbuild(BUFFERDATA(pbuff), "%w%w%w%w%w", CL_Write0|TtoH,
                             0, ADP_HandleUnknown, ADP_HandleUnknown, len);

            /* this is the start address of the string space in the packet.
             * copy from target memory at 'data' to this packet, zero
             * terminating the result.
             */
            string = (char*)(BUFFERDATA(pbuff)+count);
            MEMCPY_FROM_TARGET(string, (char *)data, len);            
            string[len] = '\0';

            count += len+1;
            
            /* adjust 'data' to point to the next 'chunk' of string (if any) */
            data += len;            
            
            /* if this call returns 0, we're ok... carry on. If non-zero,
             * there's been a comms error, and we return without further ado.
             */
            if (_sys_do_transaction(pbuff, count, NULL,
                                    CL_Write0|HtoT, (word *)&status))
                return -1;
            
        }
        return status;
    }
    return -1;
}


/* Write 'len' characters of 'buf' to file associated with 'fh' in 'mode'
   Returns the number of characters NOT written, i.e. 0==NoError.           */
/* Mode:  not passed on. It was not passed on in the Demon SWI World either.
   So it looks like remnant of some earlier OS */
/* assumes len is no of bytes rather than words, doesn't check that its
   right */
static int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len)
{
    unsigned int msglen;
    unsigned int nbytes;
    p_Buffer pbuff, rbuff;
    unsigned int bufp=0;
    word status;
    int expectedReason;
    unsigned int lenToSend;

#define SYS_PROTOCOL_OVERHEAD (4*4)
#define MAX_DATA_IN_WRITE  (Angel_ChanBuffSize - SYS_PROTOCOL_OVERHEAD - 3*4)
#define MAX_DATA_IN_WRITEX (Angel_ChanBuffSize - SYS_PROTOCOL_OVERHEAD - 4)

    pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (pbuff!=NULL) {

        lenToSend = len > MAX_DATA_IN_WRITE ? MAX_DATA_IN_WRITE : len;
        expectedReason =  CL_Write | HtoT;

        msglen = msgbuild(BUFFERDATA(pbuff), "%w%w%w%w%w%w%w", CL_Write |TtoH,
                          0, ADP_HandleUnknown, ADP_HandleUnknown, fh, len,
                          lenToSend);
        LogInfo(LOG_SYS, ("CL_Write: total %d this %d\n", len, lenToSend));

        do {
            MEMCPY_FROM_TARGET(BUFFERDATA(pbuff)+msglen, &buf[bufp], lenToSend);

            bufp += lenToSend;
            len  -= lenToSend;

            if (_sys_do_transaction(pbuff, msglen+lenToSend,
                                    &rbuff, expectedReason, &status) )
                return -1;

            nbytes = GET32LE(BUFFERDATA(rbuff)+20);
            angel_ChannelReleaseBuffer(rbuff);

            if (status)
                return status;

            if (len > 0) {
                pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
                if (pbuff == NULL)
                    return -1;

                lenToSend = len > MAX_DATA_IN_WRITEX ? MAX_DATA_IN_WRITEX : len;
                expectedReason =  CL_WriteX | HtoT;

                msglen = msgbuild(BUFFERDATA(pbuff), "%w%w%w%w%w",
                                  CL_WriteX |TtoH, 0, ADP_HandleUnknown,
                                  ADP_HandleUnknown,  lenToSend);
                LogInfo(LOG_SYS, ("CL_WriteX: %d of %d\n", lenToSend, len));
            }
        } while (len > 0);

        return nbytes;
    }
    else return -1;
}

/* ReadC reads a byte from the debugger console */
static int _sys_readc(void){
    int status;
    p_Buffer rbuff;
    int ret_code;

    if (_sys_build_and_transact(&rbuff, CL_ReadC|HtoT, (word *)&status,
                                "%w%w%w%w", CL_ReadC|TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown))
        return -1;
    else
    {
        if (status)
            ret_code = status;
        else
            ret_code = GET8(BUFFERDATA(rbuff)+20);

        angel_ChannelReleaseBuffer(rbuff);
        return ret_code;
    }
}

/* Read 'len' characters of 'buf' from file associated with 'fh' in 'mode'.
   Returns the number of characters NOT read, i.e. 0==NoError.              */
/* Note: Assumes that buf is large enough to hold the number of bytes read */
static int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len,
                     int mode)
{
    unsigned int nread , nbtotal,nbmore, nbytes, status, notread;
    p_Buffer rbuff;

    IGNORE(mode);

    if (_sys_build_and_transact(&rbuff, CL_Read|HtoT, &status,
                                "%w%w%w%w%w%w", CL_Read|TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown,
                                fh, len))
    {
        LogInfo(LOG_SYS, ("_sys_read: Error on CL_Read: status %d\n", status));
        return -1;
    }
    else {
        unpack_message(BUFFERDATA(rbuff)+20, "%w%w", &nbytes, &nbmore);
        LogInfo(LOG_SYS, ("_sys_read: CL_Read: status %d, nbytes %d. nbmore %d\n",
                          status, nbytes, nbmore));
        nbtotal = nbytes+nbmore;
        notread = len - nbytes;
        nread = nbytes;
        if (status == 0)
            MEMCPY_TO_TARGET(buf, BUFFERDATA(rbuff)+28, nbytes);
        angel_ChannelReleaseBuffer(rbuff);

        if (status)
            return (notread|0x80000000);

        while(nread<nbtotal) {
            LogInfo(LOG_SYS, ("_sys_read: CL_ReadX\n"));
            if (_sys_build_and_transact(&rbuff, CL_ReadX|HtoT, &status,
                                        "%w%w%w%w", CL_ReadX|TtoH,
                                        0, ADP_HandleUnknown, ADP_HandleUnknown))
                return -1;
            else {
                unpack_message(BUFFERDATA(rbuff)+20, "%w%w",  &nbytes, &nbmore);
                notread = nbmore;
                if (status == 0)
                    MEMCPY_TO_TARGET(buf+nread, BUFFERDATA(rbuff)+28, nbytes);
                nread += nbytes;
                angel_ChannelReleaseBuffer(rbuff);

                if (status)
                    return (notread|0x80000000);
            }
        }
        LogInfo(LOG_SYS, ("_sys_read: returning notread: %d\n", notread));
        /* Return number of bytes unread */
        return notread;
    }
}

/* Return TRUE if status value indicates an error. */
static int _sys_iserror(int status)
{
    if (status == -1)
        return TRUE;
    else
        return FALSE;
}

/* Returns non-zero if the file is connected to an interactive device. */
static int _sys_istty(FILEHANDLE fh)
{
    int status;

    if (_sys_build_and_transact(NULL, CL_IsTTY|HtoT, (word *)&status,
                                "%w%w%w%w%w", CL_IsTTY|TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown, fh))
        return -1;
    else
        return status;
}

/* Seeks to position 'pos' in the file associated with 'fh'.  Returns a
   negative value if there is an error, otherwise >=0.                  */
static int _sys_seek(FILEHANDLE fh, long pos)
{
    int status;

    if (_sys_build_and_transact(NULL, CL_Seek|HtoT, (word *)&status,
                                "%w%w%w%w%w%w", CL_Seek |TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown,
                                fh, pos))
        return -1;
    else
        return status;
}

/* Flushes any buffers associated with fh and ensures that the file is
   up to date on the backing store medium.  The result is >=0 if OK,
   negative for an error. */
static int _sys_ensure(FILEHANDLE fh)
{
    IGNORE(fh);
    return -1;
}

/* Returns length of the file fh ( or a negative error indicator). */
static long _sys_flen(FILEHANDLE fh)
{
    long length;

    if (_sys_build_and_transact(NULL, CL_Flen|HtoT, (word *)&length,
                                "%w%w%w%w%w", CL_Flen |TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown, fh))
        return -1;
    else
        return length;
}

/* Returns the name for a temporary file number fileno in the buffer name */
/* NOTE: header unclear leave atm */

static int _sys_tmpnam(char *name, int sig, unsigned maxlen)
{
    p_Buffer rbuff;
    int status, namlen;

    if (_sys_build_and_transact(&rbuff, CL_TmpNam|HtoT, (word *)&status,
                                "%w%w%w%w%w%w", CL_TmpNam |TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown,
                                maxlen, sig))
        return -1;

    if (status == 0) {
        namlen = PREAD(LE,(word *)(BUFFERDATA(rbuff)+20));
        MEMCPY_TO_TARGET(name, BUFFERDATA(rbuff)+24, namlen);
    }
    angel_ChannelReleaseBuffer(rbuff);
    return status;
}

static int _sys_clock(void)
{
    p_Buffer rbuff;
    int status, clks;

    if (_sys_build_and_transact(&rbuff, CL_Clock|HtoT, (word *)&status,
                                "%w%w%w%w", CL_Clock |TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown))
        return -1;

    if (status)
        clks = -1;
    else
        clks = PREAD(LE,(word *)(BUFFERDATA(rbuff)+20));

    angel_ChannelReleaseBuffer(rbuff);
    return clks;
}

static time_t _sys_time(void)
{
    p_Buffer rbuff;
    int status;
    time_t time;

    if (_sys_build_and_transact(&rbuff, CL_Time|HtoT, (word *)&status,
                                "%w%w%w%w", CL_Time |TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown))
        return -1;

    if (status)
        time = -1;
    else
        time = PREAD(LE,(word *)(BUFFERDATA(rbuff)+20));

    angel_ChannelReleaseBuffer(rbuff);
    return time;
}

static int _sys_system(unsigned char *name, int namelen)
{
    p_Buffer pbuff, rbuff;
    int status, data, count;

    if (name == NULL) return 0; /* NULL line do nothing */
    pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (pbuff!=NULL) {
        count = msgbuild(BUFFERDATA(pbuff),"%w%w%w%w%w", CL_System|TtoH,
                         0, ADP_HandleUnknown, ADP_HandleUnknown, namelen);
        MEMCPY_FROM_TARGET((char *)BUFFERDATA(pbuff)+count, name, namelen+1);
        count += (namelen+1);  /* allow for trailing NULL */

        if (_sys_do_transaction(pbuff, count,
                                &rbuff, CL_System|HtoT, (word *)&status))
            return -1;

        data = GET32LE(BUFFERDATA(rbuff)+20);
        angel_ChannelReleaseBuffer(rbuff);

        if (status)
            return status;
        else
            return data;
    }
    return -1;
}

static int _sys_remove(unsigned char *name, int namelen)
{
    p_Buffer pbuff;
    int status,count;

    pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (pbuff!=NULL) {
        count = msgbuild(BUFFERDATA(pbuff),"%w%w%w%w%w", CL_Remove|TtoH,
                         0, ADP_HandleUnknown, ADP_HandleUnknown, namelen);
        MEMCPY_FROM_TARGET((char *)BUFFERDATA(pbuff)+count, name, namelen+1);
        count += (namelen+1);  /* allow for trailing NULL */

        if ( _sys_do_transaction(pbuff, count,
                                 NULL, CL_Remove|HtoT, (word *)&status))
            return -1;
        else
            return status;
    }
    return -1;
}

static int _sys_rename(unsigned char *oldname, int oldnamelen,
                       unsigned char *newname, int newnamelen)
{
    p_Buffer pbuff;
    int status, count, count2;

    pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (pbuff!=NULL) {
        count = msgbuild(BUFFERDATA(pbuff),"%w%w%w%w%w", CL_Rename|TtoH,
                         0, ADP_HandleUnknown, ADP_HandleUnknown, oldnamelen);
        MEMCPY_FROM_TARGET((char *)BUFFERDATA(pbuff)+count, oldname,++oldnamelen);
        count += oldnamelen;  /* allow for trailing NULL */
        count2 = count;
        PUT32LE((char *)(BUFFERDATA(pbuff)+count), newnamelen);
        count+=4;
        MEMCPY_FROM_TARGET((char *)BUFFERDATA(pbuff)+count, newname,++newnamelen);
        count +=newnamelen;

        if (_sys_do_transaction(pbuff, count,
                                NULL, CL_Rename|HtoT, (word *)&status))
            return -1;
        else
            return status;
    }
    return -1;
}

static int _sys_getcmdline(unsigned char *cmdline)
{
    p_Buffer rbuff;
    int status, cmdlen;

    if (_sys_build_and_transact(&rbuff, CL_GetCmdLine|HtoT, (word *)&status,
                                "%w%w%w%w", CL_GetCmdLine|TtoH,
                                0, ADP_HandleUnknown, ADP_HandleUnknown))
        return -1;

    if (status == 0) {
        cmdlen = PREAD(LE,(word *)(BUFFERDATA(rbuff)+20));
        /* overwrite buffer, but that's okay */
        ((char *)BUFFERDATA(rbuff))[24+cmdlen] = '\0';
        MEMCPY_TO_TARGET(cmdline,(char *)BUFFERDATA(rbuff)+24, cmdlen+1);
    }

    angel_ChannelReleaseBuffer(rbuff);
    return status;
}

/* Put the following data into the datablock passed to us:
 * datablock[0] = heap base
 * datablock[1] = heap limit
 * datablock[2] = stack base
 * datablock[3] = stack limit
 */
static int _sys_heapinfo(unsigned *datablock)
{
#ifdef ICEMAN2
# define TARGET_STACK_SIZE (16*1024)
    unsigned tmp_data[4];
    tmp_data[0] = 0; /* This means use the end of the image as heap base*/
    tmp_data[1] = bytesex_hostval(target_top_of_memory - TARGET_STACK_SIZE);
    tmp_data[2] = bytesex_hostval(target_top_of_memory);
    tmp_data[3] = bytesex_hostval(target_top_of_memory - TARGET_STACK_SIZE);
    MEMCPY_TO_TARGET((char *) datablock,(char *)tmp_data, 16);
#else
    *(AngelHeapStackDesc *)datablock = angel_heapstackdesc;
#endif
    return (int) datablock;
}

/*
 * The main decoder for the C library support routines...
 *
 *  sysCode is the Semihosting Call number for the SH SWI
 *  r1 points at the args of the function. for details see sys.h
 *
 * Remember that r1 is a target address at this point; if running in
 * a memory space other than the target's (e.g. E-ICE), appropriate action
 * will have to be taken to get the data required from the target. 
 *
 */
#define MAX_ARGS 4

static word RealSysLibraryHandler(unsigned int sysCode, word r1)
{
#ifdef ICEMAN2
    word argarray[MAX_ARGS];
#endif
    int *args;

    LogInfo(LOG_SYS, ("RealSysLibraryHandler: code = 0x%x, r1 = 0x%x.\n", sysCode, r1));

    if (boot_completed == 0)
    {
        LogWarning(LOG_SYS, ("RealSysLibraryHandler: Not booted -- unable to do semihosting!\n" ));
        return -1;
    }

    /* these operations do their own parameter management */
    switch (sysCode)
    {
        case SYS_WRITEC:
            return _sys_writeC((unsigned char *)r1);
                
        case SYS_WRITE0:
            return _sys_write0((unsigned char *)r1);

        default:
            break;
    }

    /* grab the parameter list from the target for the functions below. */
    if ( r1 != NULL )
    {
#ifdef ICEMAN2
        int i;
        word count;

        /* r1 is in target memory, and we need a local memory pointer... copy
         * the args to local memory before passing them on.
         */
        args = (int*)argarray;
        
        /*
         * Really, we ought to only fetch the appropriate number of args,
         * according to sysCode.  But that's a chore and as the maximum is
         * only 4 words we can afford to get them all. There's a very small
         * chance that this will cause an illegal read in the target, but in
         * actual fact this seems pretty unlikely.
         */
        angelOS_MemRead( -1, -1, r1, MAX_ARGS * 4, (byte *)args, &count );
        
        /* If the TARGET is big endian we must byte-reverse these
         * words for little endian transmission to the host.
         */
        for (i=0; i<MAX_ARGS ; i++)
        {
            args[i] = bytesex_hostval(args[i]);
        }
#else
        /* for direct Angel, r1 is already a suitable pointer to the args. */
        args = (int*)r1;
#endif
    }

    switch (sysCode)
    {
        case SYS_OPEN:
            return _sys_open((char *)args[0], args[1], args[2]);
            
        case SYS_CLOSE:
            return _sys_close((FILEHANDLE)args[0]);
            
        case SYS_WRITE:
            return _sys_write((FILEHANDLE)args[0],
                              (unsigned char *)args[1],
                              (unsigned)args[2]);
            
        case SYS_READC:
            return _sys_readc();
            
        case SYS_READ:
            return _sys_read((FILEHANDLE)args[0],
                             (unsigned char *)args[1],
                             (unsigned)args[2],args[3]);
            
        case SYS_ISTTY:
            return _sys_istty((FILEHANDLE )args[0]);
            
        case SYS_ENSURE:
            return _sys_ensure((FILEHANDLE )args[0]);
            
        case SYS_ISERROR:
            return _sys_iserror(args[0]);

        case SYS_SEEK:
            return _sys_seek((FILEHANDLE)args[0],(long)args[1]);

        case SYS_FLEN:
            return (word)_sys_flen((FILEHANDLE)args[0]);

        case SYS_TMPNAM:
            return _sys_tmpnam((char *)args[0],args[1],
                               (unsigned) args[2]);
        case SYS_GET_CMDLINE:
            return _sys_getcmdline((unsigned char *)args[0]);

        case SYS_HEAPINFO:
            return _sys_heapinfo((unsigned *) args[0]);

        case SYS_RENAME:
            return _sys_rename((unsigned char *)args[0],args[1],
                               (unsigned char *)args[2],args[3]);
            
        case SYS_REMOVE:
            return _sys_remove((unsigned char *)args[0],args[1]);

        case SYS_SYSTEM:
            return _sys_system((unsigned char *)args[0],args[1]);

        case SYS_TIME:
            return _sys_time();

        case SYS_CLOCK:
            return _sys_clock();

        default:
            LogWarning(LOG_SYS, ("RealSysLibraryHandler: Unknown Angel SWI called\n"));
            break;
    }
    return -1;
}

word SysLibraryHandler(unsigned int sysCode, word r1)
{
    word retval;
    sys_handler_running = 1;
    retval = RealSysLibraryHandler(sysCode, r1);
    sys_handler_running = 0;
    return retval;
}

/*
 * This routine is called by Angel Startup to initialise the C library support
 * code.
 */
void angel_SysLibraryInit(void)
{
    sys_handler_running = 0;
    return;
}

#ifdef ICEMAN2
void angel_SetTopMem(unsigned addr)
{
    target_top_of_memory=addr;
}
#endif

int Angel_IsSysHandlerRunning(void)
{
    return sys_handler_running;
}
