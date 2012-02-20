/* -*-C-*-
 *
 * $Revision: 1.7.6.3 $
 *   $Author: rivimey $
 *     $Date: 1998/03/03 13:48:37 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Buffer management
 */

#include <stdio.h>
#include "adp.h"
#include "buffers.h"
#include "chandefs.h"
#include "devices.h"
#include "devconf.h"
#include "logging.h"

#include "serlock.h" /* for Enter/LeaveCriticalSection */

/*
 * bitfield indicating which buffers have been allocated
 * this is hidden behind the api calls so could be changed
 * in the unlikely case of needing more than 32 buffers
 */
static unsigned long buffers_used = 0L;
static unsigned long long_buffers_used = 0L;

#define MAX_NUMBUFFERS 32

/* Theoretically it is possible for the worst case situation to
 * be when every channel has a buffer allocated, and is processing
 * it and also every device other device is attempting to reboot.
 * This results in a worst case of CI_NUM_CHANNELS+DI_NUM_DEVICES
 * being needed, which is alot of buffers.  Thus in practice a
 * much smaller number are allocated.  Be aware that it is possible
 * to run out of buffers through!
 */
#define NUMBUFFERS 10
#define NUMLONGBUFFERS 1

/* size of a default buffer */
#define BUFFERDEFSIZE ADP_BUFFER_MIN_SIZE

/* This may have been specified in devconf.h, but if it hasn't
 * then use a default value here.  Choosing a value above the
 * default will speed up download.  Between 2kb and 16kb is best
 */
#if !defined(BUFFERLONGSIZE)
#define BUFFERLONGSIZE 2048
#endif

/*
 * It would be nice to have this compile-time check but sadly
 * CI_NUM_CHANNELS and DI_NUM_DEVICES are enums, unavailable to
 * the preprocessor.  We do it in the init routine instead.
 */
#if 0
#if (NUMBUFFERS > MAX_NUMBUFFERS)
#error Too many devices/channels for the buffer manager
#endif
#if (NUMLONGBUFFERS > MAX_NUMBUFFERS)
#error Too many devices/channels for the buffer manager
#endif
#endif

/* allocate the buffers list */
static unsigned char buffers[NUMBUFFERS][BUFFERDEFSIZE];

/* allocate the long buffer */
static unsigned char buffers_long[NUMLONGBUFFERS][BUFFERLONGSIZE];


/*
 *  Function: showUsedBuffers
 *   Purpose: To display the used buffers.
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: none
 */
#if DEBUG == 1
static void 
showUsedBuffers()
{
    register int c;

    LogInfo(LOG_BUFFER, ( "Buffers used: STD (%d max) ", NUMBUFFERS));
    for (c = 0; c < NUMBUFFERS; c++)
    {
        if ((buffers_used & (0x1 << c)) != 0)
            LogInfo(LOG_BUFFER, ( "%d, ", c));
    }

    LogInfo(LOG_BUFFER, ( "Long (%d max) ", NUMLONGBUFFERS));
    for (c = 0; c < NUMLONGBUFFERS; c++)
    {
        if ((long_buffers_used & (0x1 << c)) != 0)
            LogInfo(LOG_BUFFER, ( "%d, ", c));
    }
    LogInfo(LOG_BUFFER, ( "\n"));
}
#else
#define showUsedBuffers()
#endif


#ifdef STAT_BUFFERS
#define collateStats(s)  angel_collateStats(s)
#define initBufferStats()      angel_initBufferStats()

short numAllocs = 0;
unsigned char largeStats = 0;
unsigned char bufferStats[BUFFERDEFSIZE];
static void angel_collateStats(int size)
{
    if (size < BUFFERDEFSIZE)
        bufferStats[size]++;
    else
        largeStats++;
    numAllocs ++;
}

static char *nChars(int n)  /* return a pointer to a string of 'n' asterisks */
{
    static char b[60];
    int j = 0;
    while(n-- > 0)
        b[j++] = '*';
    b[j] = '\0';
    return b;
}

static void angel_initBufferStats()
{
    int i;
    largeStats = 0;
    numAllocs = 0;
    for (i = 0; i < BUFFERDEFSIZE; i++)
    {
        bufferStats[i] = 0;
    }
}

void angel_printBufferStats()
{
    int i, max = 0;
    LogInfo(LOG_BUFFER, ("%4d allocations\n%4d large buffers\nSmall buffer distribution:\n",
                           numAllocs, largeStats));
    for (i = 0; i < BUFFERDEFSIZE; i++)
    {
        if (max < bufferStats[i])
            max = bufferStats[i];
    }
    for (i = 0; i < BUFFERDEFSIZE; i++)
    {
        LogInfo(LOG_BUFFER, ("%8d : %-5d  %s\n",
                               i, bufferStats[i],
                               nChars((bufferStats[i] * 50) / max)));
    }
}
#else
#define collateStats(s)
#define initBufferStats(s)
#endif


/*
 *  Function: angel_clearBuffer
 *   Purpose: to clear the buffer to either 0 or 0xDEAD to indicate 'just
 *            allocated' or 'free' respectively. This mainly to help when
 *            debugging Angel, hence the #ifdef.
 *
 *    Params:
 *       Input: buff       -- pointer to packet's data
 *              size       -- the size of the buffer in bytes
 *              nowfree    -- FALSE for a just-allocated packet,
 *                            TRUE for a just-freed one
 *
 *   Returns: Nothing. 
 */

#ifdef CLEAR_BUFFERS
#define clearBuffer(b, s, f)  angel_clearBuffer(b, s, f)

static void 
angel_clearBuffer(unsigned char *buff, int size, int nowfree)
{
    int i;
    int v1 = (nowfree) ? 0xAD : 0x00;
    int v2 = (nowfree) ? 0xDE : 0x00;

    /*
     * clear out the buffer, writing 0xDEAD to mark it unused,
     * and 0x0000 unwritten.
     */
    for (i = 0; i < size; i += 2)
    {
        buff[i] = v1;
        buff[i + 1] = v2;
    }
}
#else
#define clearBuffer(b, s, f)
#endif


/*
 *  Function: Angel_BufferQuerySizes
 *   Purpose: To return the available sizes of buffers. Two sizes are
 *            supported, 'default' which is for simple packets, and 'long'
 *            which is for high-volume data transport, e.g. downloading
 *            images.
 *
 *    Params:
 *
 *   Returns: 
 *              default_size   -- pointer to locn to write to for default
 *              max_size       -- pointer to locn to write to for long size
 */
void 
Angel_BufferQuerySizes(unsigned int *default_size,
                       unsigned int *max_size)
{
    *default_size = BUFFERDEFSIZE;
    *max_size = BUFFERLONGSIZE;
}


/*
 *  Function: Angel_BuffersLeft
 *   Purpose: To return the number of data buffers available for use,
 *            assuming normal size packets. There is in the current
 *            implementation only one long (max-size) packet available.
 *
 *    Params:
 *
 *   Returns: int - the number of default-size packet buffers left
 */
unsigned int 
Angel_BuffersLeft(void)
{
    unsigned int c, num_used = 0;

    for (c = 0; c < NUMBUFFERS; c++)
    {
        if ((buffers_used & (0x1 << c)) != 0x0)  /* is the buffer used? */
            num_used++;
    }

    for (c = 0; c < NUMLONGBUFFERS; c++)
    {
        if ((long_buffers_used & (0x1 << c)) != 0x0)  /* is the buffer used? */
            num_used++;
    }

    return (NUMBUFFERS + NUMLONGBUFFERS) - num_used;
}


/*
 *  Function: Angel_BufferAlloc
 *   Purpose: To return a pointer to a packet buffer of sufficient size,
 *            or NULL if no buffer available (well, currently, never NULL
 *            as the LogError() calls don't return).
 *
 *            If the requested size is greater than the default size, the
 *            long buffer is considered for use automatically. If there are
 *            no default size buffers free for a default size request, the
 *            long buffer will be allocated as a last resort.
 *
 *    Params:
 *       Input: size -- the number of bytes needed in the buffer
 *              
 *
 *   Returns: p_Buffer - a pointer to the start of the data buffer.
 */
p_Buffer 
Angel_BufferAlloc(unsigned int req_size)
{
    unsigned int c = 0;

    collateStats(req_size);

    /* 
     * simple, common case; short buffer wanted.
     */
    if (req_size <= BUFFERDEFSIZE)
    {
        /* we want a default size (or smaller) */
        for (c = 0; c < NUMBUFFERS; c++)
        {
            if ((buffers_used & (0x1 << c)) == 0)
            {
                buffers_used |= (0x1 << c);
                LogInfo(LOG_BUFFER, ("Angel_BufferAlloc: STD %d (%08x) size %d\n",
                                       c, buffers[c], req_size));
                showUsedBuffers();
                clearBuffer(buffers[c], BUFFERDEFSIZE, FALSE);
                return (p_Buffer) buffers[c];
            }
        }
    }

    /*
     * now, either it is too long for a short buffer, or we have run out of short
     * buffers. See if a long one is available...
     */
    if (req_size <= BUFFERLONGSIZE)
    {
        /* we want a default size (or smaller) */
        for (c = 0; c < NUMLONGBUFFERS; c++)
        {
            if ((long_buffers_used & (0x1 << c)) == 0)
            {
                long_buffers_used |= (0x1 << c);
                LogInfo(LOG_BUFFER, ("Angel_BufferAlloc: LONG %s%d (%08x) size %d\n",
                                       (req_size <= BUFFERDEFSIZE) ? "AS DEFAULT " : "",
                                       c, buffers[c], req_size));
                showUsedBuffers();
                clearBuffer(buffers_long[c], BUFFERLONGSIZE, FALSE);
                
                return (p_Buffer) buffers_long[c];
            }
        }
    }
    else
    {
        LogError(LOG_BUFFER, ("Angel_BufferAlloc: size %d too big!\n", req_size));
        showUsedBuffers();
        return NULL;
    }

    LogError(LOG_BUFFER, ("Angel_BufferAlloc: out of buffers"));
    showUsedBuffers();
    return NULL;                /* no buffers free */
}


/*
 *  Function: Angel_BufferRelease
 *   Purpose: To release a buffer back to the buffer pool. The
 *            address passed in may be any address within the buffer
 *            to be released (this simplifies things as offsets
 *            are routinely applied to buffer addresses, and is a
 *            change from past behaviour -- RIC).
 *
 *    Params:
 *       Input: buffer - a pointer into the data buffer to be released.
 *              
 *   Returns: Nothing.
 */
void 
Angel_BufferRelease(p_Buffer buffer)
{
    unsigned int buff_num;

    for (buff_num = 0; buff_num < NUMBUFFERS; buff_num++)
    {
        register p_Buffer bufn = (p_Buffer) buffers[buff_num];

        if (buffer >= bufn && buffer < (bufn + BUFFERDEFSIZE))
        {
            LogInfo(LOG_BUFFER, ( "Angel_BufferRelease: STD %d @ %08x (%08x)\n",
                                    buff_num, bufn, buffer));
            
            if ((buffers_used & (0x1 << buff_num)) == 0)
                LogWarning(LOG_BUFFER, ("Angel_BufferRelease: FREE TWICE\n"));
            
            buffers_used = buffers_used & ~(0x1 << buff_num);
            showUsedBuffers();

            clearBuffer(buffers[buff_num], BUFFERDEFSIZE, TRUE);
            return;
        }
    }
    
    for (buff_num = 0; buff_num < NUMLONGBUFFERS; buff_num++)
    {
        register p_Buffer bufn = (p_Buffer) buffers_long[buff_num];

        if (buffer >= bufn && buffer < (bufn + BUFFERLONGSIZE))
        {
            LogInfo(LOG_BUFFER, ( "Angel_BufferRelease: LONG %d @ %08x (%08x)\n",
                                    buff_num, bufn, buffer));
            
            if ((long_buffers_used & (0x1 << buff_num)) == 0)
                LogWarning(LOG_BUFFER, ("Angel_BufferRelease: FREE TWICE\n"));
            
            long_buffers_used = long_buffers_used & ~(0x1 << buff_num);
            showUsedBuffers();

            clearBuffer(buffers[buff_num], BUFFERLONGSIZE, TRUE);
            return;
        }
    }

    LogError(LOG_BUFFER, ( "Trying to free a non-existant buffer %08x\n", buffer));
    showUsedBuffers();
    return;
}

/*
 *  Function: Angel_InitBuffers
 *   Purpose: To initialise the variables used in the buffer management code,
 *            most notably the 'buffer free' flags. If compiled with the
 *            CLEAR_BUFFERS define, also clear the data buffer contents to
 *            the 'free' state.
 *
 *    Params:
 *       Input: Nothing.
 *              
 *   Returns: Nothing.
 */
buf_init_error 
Angel_InitBuffers(void)
{
#ifdef CLEAR_BUFFERS
    int buff_num;
#endif

    Angel_EnterCriticalSection();
    
    initBufferStats();
    
    if (NUMBUFFERS > MAX_NUMBUFFERS)
    {
        LogError(LOG_BUFFER, ("Angel_InitBuffers: NUMBUFFERS config err: (%d > %d)\n",
                                NUMBUFFERS, MAX_NUMBUFFERS));
        return INIT_BUF_FAIL;
    }

    LogInfo(LOG_BUFFER, ("Angel_InitBuffers: entered with "
                           "B-used: %04x, LB-used: %04x\n",
                           buffers_used, long_buffers_used));

    LogInfo(LOG_BUFFER, ("  Buffer addresses: Std: %p to %p, Long: %p to %p\n",
                           &buffers[0], &buffers[NUMBUFFERS],
                           &buffers_long[0], &buffers_long[NUMLONGBUFFERS]));

#ifdef CLEAR_BUFFERS
    for (buff_num = 0; buff_num < NUMBUFFERS; buff_num++)
    {
        clearBuffer(buffers[buff_num], BUFFERDEFSIZE, TRUE);
    }
    for (buff_num = 0; buff_num < NUMLONGBUFFERS; buff_num++)
    {
        clearBuffer(buffers_long[buff_num], BUFFERLONGSIZE, TRUE);
    }
#endif

    buffers_used = 0x0L;
    long_buffers_used = 0x0L;
    
    Angel_LeaveCriticalSection();
    
    return INIT_BUF_OK;
}


/* EOF buffers.c */
