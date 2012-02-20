/* -*-C-*-
 *
 * $Revision: 1.4.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:02 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: User Application interface to raw devices
 */

#include "devraw.h"

#include "devdriv.h"
#include "logging.h"

DevError 
Angel_RawDeviceWrite(DeviceID devID, p_Buffer buff, unsigned length)
{
    LogInfo(LOG_DEVRAW, ("Angel_RawDeviceWrite: devid %d, buff %x, length %d\n",
              devID, buff, length));

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (angel_Device[devID]->type != DT_RAW)
        return DE_BAD_DEV;

    Angel_EnterSVC();
    if (angel_DeviceStatus[devID] & DEV_WRITE_BUSY)
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }
    angel_DeviceStatus[devID] |= DEV_WRITE_BUSY;  /* cleared by device */
    Angel_ExitToUSR();

    return angel_Device[devID]->rw.raw.write(devID, buff, length);
}


DevError 
Angel_RawDeviceRead(DeviceID devID, p_Buffer buff, unsigned length)
{
    LogInfo(LOG_DEVRAW, ("Angel_RawDeviceRead: devid %d, buff %x, length %d\n",
              devID, buff, length));

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (angel_Device[devID]->type != DT_RAW)
        return DE_BAD_DEV;

    Angel_EnterSVC();
    if (angel_DeviceStatus[devID] & DEV_READ_APPL_BUSY)
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }
    angel_DeviceStatus[devID] |= DEV_READ_APPL_BUSY;
    Angel_ExitToUSR();

    /* Okay, we've passed the checks, let's get on with it */
    return angel_Device[devID]->rw.raw.read(devID, buff, length);
}


DevError 
Angel_RawDeviceControl(DeviceID devID, DeviceControl op, void *arg)
{
    LogInfo(LOG_DEVRAW, ("angel_RawDeviceControl: devID %d, op %d, arg %x\n",
              devID, op, arg));

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    return angel_Device[devID]->control(devID, op, arg);
}


#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0

void 
Angel_RawDeviceYield(void)
{
    /* jump straight to the core */
    angel_DeviceYield();
}


static void 
angel_RawControlAll(DeviceControl op, void *arg)
{
    int i;

    LogInfo(LOG_DEVRAW, ("angel_RawControlAll: op %d, arg %x\n", op, arg));

    for (i = 0; i < DI_NUM_DEVICES; ++i)
        angel_Device[i]->control(i, op, arg);
}


void 
Angel_RawReceiveMode(DevRecvMode mode)
{
    angel_RawControlAll(DC_RECEIVE_MODE, (void *)mode);
}


void 
Angel_RawResetDevices(void)
{
    angel_RawControlAll(DC_RESET, NULL);
}


void 
Angel_RawInitialiseDevices(void)
{
    int i;

    LogInfo(LOG_DEVRAW, ( "angel_RawInitialiseDevices\n"));

    for (i = 0; i < DI_NUM_DEVICES; ++i)
    {
        angel_DeviceStatus[i] = 0;
        angel_Device[i]->control(i, DC_INIT, NULL);
    }

    /* err, thats all? */
}


#endif /* def MINIMAL_ANGEL */

/* EOF devraw.c */
