/* -*-C-*-
 *
 * $Revision: 1.5.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/22 14:14:25 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: User Application interface to shared (Angel) devices
 */

#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0

/* For the minimal version of Angel we just call the devraw.h
 * functions directly, since they are linked in.  We can just define
 * some macros to do this for us.
 *
 * This is done in the header file, devappl.h.
 */

#error devshare.c not required in minimal Angel system.

#else

#include "arm.h"
#include "devclnt.h"
#include "devdriv.h"
#include "devraw.h"
#include "serlock.h"
#include "logging.h"
#include "support.h"

/* forward declarations */
static DevError angel_AngelDeviceWrite(DeviceID devID,
                                       p_Buffer buff, unsigned length);
static DevError angel_AngelDeviceRead(DeviceID devID,
                                      p_Buffer buff, unsigned length);


/* There may be Angel devices and raw devices in a non-minimal system.
 * This module switches APPL requests to the corresponding code
 * according to the type of device.
 */

static DevError 
angel_SwitchDeviceWrite(DeviceID devID,
                        p_Buffer buff,
                        unsigned length)
{
    ASSERT(angel_Device != NULL, ("not initialised"));

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (angel_Device[devID]->type == DT_ANGEL)
        return angel_AngelDeviceWrite(devID, buff, length);
    else
        return Angel_RawDeviceWrite(devID, buff, length);
}


static DevError 
angel_SwitchDeviceRead(DeviceID devID,
                       p_Buffer buff,
                       unsigned length)
{
    ASSERT(angel_Device != NULL, ("not initialised"));

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (angel_Device[devID]->type == DT_ANGEL)
        return angel_AngelDeviceRead(devID, buff, length);
    else
        return Angel_RawDeviceRead(devID, buff, length);
}


static DevError 
angel_SwitchDeviceControl(DeviceID devID,
                          DeviceControl op,
                          void *arg)
{
    ASSERT(angel_Device != NULL, ("not initialised"));

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (angel_Device[devID]->type == DT_ANGEL)
        return DE_OKAY;         /* ignored - Angel is in charge */
    else
        return Angel_RawDeviceControl(devID, op, arg);
}





#define APPL_BLOCKING ((unsigned int)(-1))

static void 
angel_ApplWriteCallback(void *v_buff,
                        void *v_length,
                        void *v_status,
                        void *cb_data)
{
    DevStatus status = (DevStatus) (unsigned int)v_status;
    unsigned int *write_error = (unsigned int *)cb_data;

    IGNORE(v_buff);
    IGNORE(v_length);

    ASSERT(*write_error == APPL_BLOCKING, ("not blocking"));

    if (status == DS_DONE)
        *write_error = DE_OKAY;
    else
        *write_error = DE_FAILED;
}


static DevError 
angel_AngelDeviceWrite(DeviceID devID,
                       p_Buffer buff, unsigned length)
{
    volatile unsigned int write_error = APPL_BLOCKING;
    DevError dev_error;

    /* start an async write */
    dev_error = angel_DeviceWrite(devID,
                                  buff, length,
                                  angel_ApplWriteCallback,
                                  (void *)(&write_error),
                                  DC_APPL);
    if (dev_error != DE_OKAY)
        return dev_error;

    /* now wait for the write to complete */
    while (write_error == APPL_BLOCKING)
        Angel_Yield();

    return (DevError) write_error;
}


typedef struct appl_ReadDetails
{
    p_Buffer dest;              /* destination for read in progress */
    unsigned int read_size;     /* size of read in progress */
    unsigned int n_done;        /* how many done */
    DevStatus status;           /* callback status */
    p_Buffer buffer;            /* if we have one */
    unsigned int buff_size;     /* size of buffer */
    unsigned int n_buff;        /* num bytes remaining in buffer */
    bool buff_locked;           /* buff in use by device driver */
    bool registered;            /* read registered? */
}
appl_ReadDetails;

static volatile appl_ReadDetails read_details[DI_NUM_DEVICES];

static unsigned int default_buffer_size = ADP_BUFFER_MIN_SIZE;

static void 
angel_ApplUseBuffer(volatile appl_ReadDetails * const details,
                    unsigned int length)
{
    unsigned int n_buff = details->n_buff;
    unsigned int to_copy = (length > n_buff ? n_buff : length);

    __rt_memcpy(details->dest + details->n_done, details->buffer, to_copy);
    details->n_done += to_copy;

    if (to_copy < n_buff)
    {
        /* there is stuff in the buffer still */
        __rt_memcpy(details->buffer, details->buffer + to_copy,
                    n_buff - to_copy);
        details->n_buff = n_buff - to_copy;
    }
    else
    {
        /* buffer empty -- free it */
        Angel_BufferRelease(details->buffer);
        details->buffer = NULL;
    }
}


static void 
angel_ApplReadCallback(void *v_buff,
                       void *v_length,
                       void *v_status,
                       void *cb_data)
{
    p_Buffer buff = (p_Buffer) v_buff;
    unsigned int length = (unsigned int)v_length;
    DevStatus status = (DevStatus) (unsigned int)v_status;
    DeviceID devID = (DeviceID) cb_data;
    volatile appl_ReadDetails *const details = &read_details[devID];

#if ASSERT_ENABLED == 0
    IGNORE(buff);
#endif

    Angel_EnterSVC();

    /* Error? */
    if (status != DS_DONE)
    {
        details->status = status;
        Angel_ExitToUSR();
        return;
    }

    /* Have we read into a buffer? */
    if (details->buffer != NULL)
    {
        ASSERT(buff == (details->buffer + details->n_buff), ("buff wrong"));

        details->n_buff += length;

        /* can we satisfy the read? */
        if (details->dest != NULL)
            angel_ApplUseBuffer(details,
                                details->read_size - details->n_done);

        /* unlock the buffer for use in angel_AngelDeviceRead() */
        details->buff_locked = FALSE;
    }
    else
    {
        /* must be direct read */
        ASSERT(details->dest != NULL, ("dest"));
        details->n_done += length;
    }
    Angel_ExitToUSR();
}


static DevError 
angel_AngelDeviceRead(DeviceID devID,
                      p_Buffer dest, unsigned length)
{
    volatile appl_ReadDetails *const details = &read_details[devID];

    Angel_EnterSVC();

    /* Are we busy with someone else? */
    if (details->dest != NULL)
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }

    details->dest = dest;
    details->read_size = length;
    details->n_done = 0;
    details->status = DS_DONE;

    /* Have we got anything in a buffer? */
    if (details->buffer != NULL && !details->buff_locked)
        angel_ApplUseBuffer(details, length);

    /* We need to do some real reading */

    Angel_ExitToUSR();

    while (details->n_done < length
           && details->status == DS_DONE)
        Angel_Yield();

    Angel_EnterSVC();
    details->dest = NULL;
    Angel_ExitToUSR();

    if (details->status == DS_DONE)
        return DE_OKAY;
    else
        return DE_FAILED;
}


static p_Buffer 
angel_ApplNeedBuffer(volatile appl_ReadDetails * const details,
                     unsigned int req_size)
{
    if (req_size < default_buffer_size)
        req_size = default_buffer_size;

    details->buffer = Angel_BufferAlloc(req_size);
    details->buff_size = req_size;
    details->n_buff = 0;
    details->buff_locked = TRUE;

    return details->buffer;
}


static p_Buffer 
angel_ApplAllocBuffer(unsigned int req_size, void *cb_data)
{
    p_Buffer ret_buff;
    volatile appl_ReadDetails *details;
    unsigned int s;

    ASSERT(cb_data != NULL, ("cb_data"));
    details = (appl_ReadDetails *) cb_data;

    s = Angel_DisableInterruptsFromSVC();

    if (details->buffer == NULL)
    {
        /* no temporary buffer */
        if (details->dest == NULL)
        {
            /* no read-in-progress, either.
             * grab a buffer if possible, just to be kind.
             */
            ret_buff = angel_ApplNeedBuffer(details, req_size);
        }
        else
        {
            /* read in progress */
            unsigned int direct_space = details->read_size - details->n_done;

            if (req_size > direct_space)
            {
                /* not enough room to do it direct.
                 * get a buffer, if possible.
                 */
                ret_buff = angel_ApplNeedBuffer(details, req_size);
            }
            else
            {
                /* room to do it directly */
                ret_buff = details->dest + details->n_done;
            }
        }
    }
    else
    {
        /* already have a temporary buffer */
        unsigned int buffer_space = details->buff_size - details->n_buff;

        if (req_size > buffer_space)
        {
            /* we'll just have to drop it */
            ret_buff = NULL;
        }
        else
        {
            /* room in buffer */
            ret_buff = details->buffer + details->n_buff;
            details->buff_locked = TRUE;
        }
    }
    Angel_RestoreInterruptsFromSVC(s);
    return ret_buff;
}


/* This is called during initialisation */
static void 
angel_InitialiseSharedDevices(void)
{
    unsigned int i;
    unsigned int dummy;

    Angel_BufferQuerySizes(&default_buffer_size, &dummy);

    for (i = 0; i < DI_NUM_DEVICES; ++i)
    {
        if (angel_Device[i]->type == DT_ANGEL)
        {
            if (!read_details[i].registered)
            {
                if (angel_DeviceRegisterRead(
                                                i,
                                            angel_ApplReadCallback, (void *)i,
                            angel_ApplAllocBuffer, (void *)(&read_details[i]),
                                                DC_APPL
                    )
                    != DE_OKAY)
                {
                    LogError(LOG_DEVSHARE, ( "cannot DeviceRegisterRead\n"));
                }
                read_details[i].registered = TRUE;
            }
        }
        else                    /* DT_RAW */
            angel_DeviceControl(i, DC_INIT, NULL);

        read_details[i].dest = NULL;
        read_details[i].read_size = 0;
        read_details[i].n_done = 0;
        read_details[i].buffer = NULL;
        read_details[i].buff_size = 0;
        read_details[i].n_buff = 0;
        read_details[i].buff_locked = FALSE;

        angel_DeviceStatus[i] &= ~(DEV_WRITE_BUSY | DEV_READ_APPL_BUSY);
    }
}


DevError 
angel_ApplDeviceHandler(unsigned swi_r0, unsigned *arg_blk)
{
    IGNORE(swi_r0);

    switch (arg_blk[0])
    {
        case angel_SWIreason_ApplDevice_Read:
            {
                return angel_SwitchDeviceRead((DeviceID) arg_blk[1],
                                              (p_Buffer) arg_blk[2],
                                              (unsigned int)arg_blk[3]);
            }

        case angel_SWIreason_ApplDevice_Write:
            {
                return angel_SwitchDeviceWrite((DeviceID) arg_blk[1],
                                               (p_Buffer) arg_blk[2],
                                               (unsigned int)arg_blk[3]);
            }

        case angel_SWIreason_ApplDevice_Control:
            {
                return angel_SwitchDeviceControl(
                                                    (DeviceID) arg_blk[1],
                                     (DeviceControl) (unsigned int)arg_blk[2],
                                                    (void *)arg_blk[3]
                    );
            }

        case angel_SWIreason_ApplDevice_Yield:
            {
                Angel_Yield();
                return DE_OKAY;
            }

        case angel_SWIreason_ApplDevice_Init:
            {
                angel_InitialiseSharedDevices();
                return DE_OKAY;
            }

            /*
             * No other values than these are expected - just return an error
             */
        default:
            {
                return DE_BAD_OP;
            }
    }
}

#endif /* def MINIMAL_ANGEL ... else ... */

/* EOF devshare.c */
