/* -*-C-*-
 *
 * $Revision: 1.4.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:49:59 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: User Application interface to devices
 */

#include "devappl.h"

#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0

/* For the minimal version of Angel we just call the devraw.h
 * functions directly, since they are linked in.  We can just define
 * some macros to do this for us.
 *
 * This is done in the header file, devappl.h.
 */

#error devappl.c not required in minimal Angel system.

#else

#include "arm.h"


unsigned 
__swi(angel_SWI_ARM & 0xFFFFFF)
ApplDeviceSWI(unsigned arg1, unsigned arg2);


     DevError Angel_ApplDeviceWrite(DeviceID devID, p_Buffer buff, unsigned length)
{
    unsigned arg_space[4];

    arg_space[0] = (unsigned)angel_SWIreason_ApplDevice_Write;
    arg_space[1] = (unsigned)devID;
    arg_space[2] = (unsigned)buff;
    arg_space[3] = (unsigned)length;
    return (DevError) ApplDeviceSWI(angel_SWIreason_ApplDevice,
                                    (unsigned)arg_space);
}


DevError 
Angel_ApplDeviceRead(DeviceID devID, p_Buffer buff, unsigned length)
{
    unsigned arg_space[4];

    arg_space[0] = (unsigned)angel_SWIreason_ApplDevice_Read;
    arg_space[1] = (unsigned)devID;
    arg_space[2] = (unsigned)buff;
    arg_space[3] = (unsigned)length;
    return (DevError) ApplDeviceSWI(angel_SWIreason_ApplDevice,
                                    (unsigned)arg_space);
}


DevError 
Angel_ApplDeviceControl(DeviceID devID, DeviceControl op, void *arg)
{
    unsigned arg_space[4];

    arg_space[0] = (unsigned)angel_SWIreason_ApplDevice_Control;
    arg_space[1] = (unsigned)devID;
    arg_space[2] = (unsigned)op;
    arg_space[3] = (unsigned)arg;
    return (DevError) ApplDeviceSWI(angel_SWIreason_ApplDevice,
                                    (unsigned)arg_space);
}


void 
Angel_ApplDeviceYield(void)
{
    unsigned arg_space[1];

    arg_space[0] = (unsigned)angel_SWIreason_ApplDevice_Yield;
    ApplDeviceSWI(angel_SWIreason_ApplDevice, (unsigned)arg_space);
    return;
}


void 
Angel_ApplReceiveMode(DevRecvMode mode)
{
    /* does nothing in non-minimal system */
    IGNORE(mode);
    return;
}


void 
Angel_ApplResetDevices(void)
{
    /* does nothing in non-minimal system */
    return;
}


void 
Angel_ApplInitialiseDevices(void)
{
    unsigned arg_space[1];

    arg_space[0] = (unsigned)angel_SWIreason_ApplDevice_Init;
    ApplDeviceSWI(angel_SWIreason_ApplDevice, (unsigned)arg_space);
    return;
}


#endif /* def MINIMAL_ANGEL ... else ... */

/* EOF devappl.c */
