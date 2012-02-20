/* -*-C-*-
 *
 * $Revision: 1.13.6.6 $
 *   $Author: rivimey $
 *     $Date: 1998/02/09 22:32:32 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Public client interface to devices
 */

#include "devclnt.h"
#include "devdriv.h"

#include "logging.h"

/* private globals */

/* all the read callback info for a device */
typedef struct ReadCallbacks
{
    DevRead_CB_Fn callback;
    void *cb_data;
    DevGetBuff_Fn get_buff;
    void *getb_data;
} ReadCallbacks;

/* all the callback info for a device */
typedef struct DevCallbacks
{
    DevWrite_CB_Fn write_callback;      /* current write */
    void *write_cb_data;                /* current write */
    
    ReadCallbacks read[DC_NUM_CHANNELS];
} DevCallbacks;

/* stored callbacks and callback data for each device */
static DevCallbacks dev_cb[DI_NUM_DEVICES];


/* useful for debugging packets */
#if DEBUG == 1
static void debug_packet(log_id mod, p_Buffer buffer, unsigned int length)
{
    log_dump_buffer(WL_INFO, mod, 16, (char*)buffer, length);
}
#else
#define debug_packet(mod, buff, length)
#endif

DevError 
angel_DeviceWrite(DeviceID devID, p_Buffer buff,
                  unsigned length, DevWrite_CB_Fn callback,
                  void *cb_data, DevChanID chanID)
{
#ifdef CHECK_PARAMS
    if (devID >= DI_NUM_DEVICES)
    {
        return DE_NO_DEV;
    }
    
    if (chanID >= DC_NUM_CHANNELS)
    {
        return DE_BAD_CHAN;
    }

    if (angel_Device[devID]->type != DT_ANGEL)
    {
        return DE_BAD_DEV;
    }
#endif
    
    Angel_EnterSVC();
    if (angel_DeviceStatus[devID] & DEV_WRITE_BUSY)
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }
    angel_DeviceStatus[devID] |= DEV_WRITE_BUSY;  /* cleared by device */
    Angel_ExitToUSR();
    
    LogInfo(LOG_DEVCLNT, ("angel_DeviceWrite: devid %d, cb %x, devchan %d\n",
                          devID, callback, chanID));
    
    debug_packet(LOG_DEVCLNT, buff, length);

    /* Okay, we've passed the checks, let's get on with it */
    dev_cb[devID].write_callback = callback;
    dev_cb[devID].write_cb_data = cb_data;

    LogInfo(LOG_DEVCLNT, ("angel_DeviceWrite: chaining to device async write.\n"));
    
    return angel_Device[devID]->rw.angel.async_write(devID,
                                                     buff, length, chanID);
}


DevError 
angel_DeviceRegisterRead(DeviceID devID,
                         DevRead_CB_Fn callback, void *cb_data,
                         DevGetBuff_Fn get_buff, void *getb_data,
                         DevChanID devchanID)
{
    LogInfo(LOG_DEVCLNT, ("angel_DeviceRegisterRead: devID %d, cb %x, devchan %d\n",
                            devID, callback, devchanID));

#ifdef CHECK_PARAMS
    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (devchanID >= DC_NUM_CHANNELS)
        return DE_BAD_CHAN;

    if (angel_Device[devID]->type != DT_ANGEL)
        return DE_BAD_DEV;
#endif
    
    Angel_EnterSVC();
    if (angel_DeviceStatus[devID] & DEV_READ_BUSY(devchanID))
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }
    angel_DeviceStatus[devID] |= DEV_READ_BUSY(devchanID);
    Angel_ExitToUSR();

    /* Okay, we've passed the checks, let's get on with it */
    dev_cb[devID].read[devchanID].callback = callback;
    dev_cb[devID].read[devchanID].cb_data = cb_data;
    dev_cb[devID].read[devchanID].get_buff = get_buff;
    dev_cb[devID].read[devchanID].getb_data = getb_data;

    LogInfo(LOG_DEVCLNT, ("angel_DeviceRegisterRead: calling device register.\n"));
    
    return angel_Device[devID]->rw.angel.register_read(devID, devchanID);
}


/*
 *  Function: angel_DeviceControl
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
DevError 
angel_DeviceControl(DeviceID devID, DeviceControl op, void *arg)
{
    LogInfo(LOG_DEVCLNT, ("angel_DeviceControl: devID %d, op %d, arg %x\n",
                            devID, op, arg));
#ifdef CHECK_PARAMS

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;
#endif
    
    return angel_Device[devID]->control(devID, op, arg);
}


/*
 *  Function: angel_ControlAll
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
static void 
angel_ControlAll(DeviceControl op, void *arg)
{
    int i;

    LogInfo(LOG_DEVCLNT, ( "angel_ControlAll: op %d, arg %x\n", op, arg));

    for (i = 0; i < DI_NUM_DEVICES; ++i)
        angel_Device[i]->control(i, op, arg);
}


/*
 *  Function: angel_ReceiveMode
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
void 
angel_ReceiveMode(DevRecvMode mode)
{
    angel_ControlAll(DC_RECEIVE_MODE, (void *)mode);
}


/*
 *  Function: angel_ResetDevices
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
void 
angel_ResetDevices(void)
{
    angel_ControlAll(DC_RESET, NULL);
}


/*
 *  Function: angel_InitialiseDevices
 *   Purpose: To call the device initialise code of all devices
 *            configured under Angel.
 *
 *    Params:
 *       Input: none
 *
 *   Returns: none
 */
void 
angel_InitialiseDevices(void)
{
    int i;

    LogInfo(LOG_DEVCLNT, ( "angel_InitialiseDevices\n"));

    Angel_InitChannels();
    Angel_InitBuffers();

    for (i = 0; i < DI_NUM_DEVICES; ++i)
    {
        angel_DeviceStatus[i] = 0;
#if 0
        Pq_InitQueue(&dev_cb[i].writeq, &dev_cb[i].queuebufs, 4);
#endif
        angel_Device[i]->control(i, DC_INIT, NULL);
    }

    /* err, thats all? */
    LogInfo(LOG_DEVCLNT, ( "angel_InitialiseDevices complete.\n"));
}


/*
 *  Function: 
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */bool 
angel_IsAngelDevice(DeviceID devID)
{
    ASSERT(devID < DI_NUM_DEVICES, ("illegal device"));
    return (angel_Device[devID]->type == DT_ANGEL);
}


/*
 *  Function: 
 *   Purpose: handle receipt of a packet callback
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
void 
angel_DD_GotPacket(DeviceID devID,  /* which device */
                   p_Buffer buff,  /* pointer to data */
                   unsigned length,  /* how much done */
                   DevStatus status,  /* success code */
                   DevChanID devchanID)  /* appl or chan */
{
    angel_TaskPriority taskpri;

    LogInfo(LOG_DEVCLNT, ("angel_DD_GotPacket: dev %d, buff %x, len %d, stat %s, "
                            "chan %d, [reason %s]\n",
                            devID, buff, length, status == DS_DONE ? "ok" : "*BAD*",
                            devchanID, log_adpname(*(long*)(buff+4))));

    ASSERT(devID < DI_NUM_DEVICES, ("devID bad"));
    ASSERT(devchanID < DC_NUM_CHANNELS, ("devchanID bad"));
    ASSERT(dev_cb[devID].read[devchanID].callback != NULL,
           ("no read callback"));

    if (buff != NULL && length > 0)
        debug_packet(LOG_DEVCLNT, buff, length);

#if defined(JTAG_ADP_SUPPORTED)
    taskpri = TP_AngelCallBack;      /* no appl thread in ADP-over-JTAG relay */
#else
    taskpri = (devchanID == DC_DBUG ? TP_AngelCallBack : TP_ApplCallBack);
#endif

    Angel_QueueCallback(
                           dev_cb[devID].read[devchanID].callback,
                           taskpri,
                           (void *)buff,
                           (void *)length,
                           (void *)status,
                           (void *)dev_cb[devID].read[devchanID].cb_data
        );
}


/*
 *  Function: 
 *   Purpose: handle transmission completion
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
void 
angel_DD_SentPacket(DeviceID devID,  /* which device */
                    p_Buffer buff,  /* pointer to data */
                    unsigned length,  /* how much done */
                    DevStatus status,  /* success code */
                    DevChanID devchanID)  /* appl or chan */
{
    unsigned int s;
    angel_TaskPriority tp;

    LogInfo(LOG_DEVCLNT, ("angel_DD_SentPacket: dev %d, buff %x, len %d, stat %d, "
                          "chan %d, [reason %s]\n",
                          devID, buff, length, status,
                          devchanID, log_adpname(*(long*)(buff+4))));
    
    ASSERT(devID < DI_NUM_DEVICES, ("devID bad"));
    ASSERT(devchanID < DC_NUM_CHANNELS, ("devchanID bad"));
    ASSERT(dev_cb[devID].write_callback != NULL, ("no write callback"));

    /*
     * We need to protect this
     */
    s = Angel_DisableInterruptsFromSVC();
    angel_DeviceStatus[devID] &= ~DEV_WRITE_BUSY;
    Angel_RestoreInterruptsFromSVC(s);

#if defined(JTAG_ADP_SUPPORTED)
    tp = TP_AngelCallBack;      /* no appl thread in ADP-over-JTAG relay */
#else
    tp = (devchanID == DC_DBUG ? TP_AngelCallBack : TP_ApplCallBack);
#endif
    
    Angel_QueueCallback(
        dev_cb[devID].write_callback,
        tp,
        (void *)buff,
        (void *)length,
        (void *)status,
        (void *)dev_cb[devID].write_cb_data
        );
}


/* grab a buffer for receiving a packet into */
p_Buffer 
angel_DD_GetBuffer(DeviceID devID, DevChanID devchanID,
                   unsigned req_size)
{
    p_Buffer ret_buffer;

    LogInfo(LOG_DEVCLNT, ("angel_DD_GetBuffer: dev %d, chanID %d, size %d\n",
                            devID, devchanID, req_size));

    ASSERT(devID < DI_NUM_DEVICES, ("devID bad"));
    ASSERT(devchanID < DC_NUM_CHANNELS, ("devchanID bad"));

    {
        struct ReadCallbacks *rcb = &dev_cb[devID].read[devchanID];

        if (rcb->get_buff == NULL)
        {
            LogError(LOG_DEVCLNT, ( "angel_DD_GetBuffer: No alloc fn! (rcb = %p)\n", rcb));
            return NULL;
        }

        ret_buffer = rcb->get_buff(req_size, rcb->getb_data);
    }

    if (ret_buffer == NULL)
        LogError(LOG_DEVCLNT, ( "angel_DD_GetBuffer: get_buff returned NULL\n"));
    else
        LogInfo(LOG_DEVCLNT, ( "angel_DD_GetBuffer: returning buffer 0x%x\n", ret_buffer));

    return ret_buffer;
}


/* EOF devclnt.c */
