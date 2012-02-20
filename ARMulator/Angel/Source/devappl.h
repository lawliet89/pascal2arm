/* -*-C-*-
 *
 * $Revision: 1.4 $
 *   $Author: mgray $
 *     $Date: 1996/07/12 16:45:58 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: User Application interface to devices
 */

#ifndef angel_devappl_h
#define angel_devappl_h

/*
 * This header exports the interface available to the User Application
 * to Angel-compliant device drivers.  These routines are not used by
 * Angel itself - see devclnt.h for that interface.
 *
 * The way the functions declared in this header are implemented
 * depends on whether a full Angel system or a minimal Angel system is
 * being used.  In the full system a SWI gets called which gets
 * trapped by Angel and then the device driver request is processed,
 * wheras in the minimal system the is a very thin veneer layer onto
 * the functions in devclnt.h
 */

#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0

/* For the minimal version of Angel we just call the devraw.h
 * functions directly, since they are linked in.  We can just define
 * some macros to do this for us.
 */

#include "devraw.h"

#define Angel_ApplDeviceWrite(devID, buff, length) \
        Angel_RawDeviceWrite((devID), (buff), (length))

#define Angel_ApplDeviceRead(devID, buff, length) \
        Angel_RawDeviceRead((devID), (buff), (length))
       
#define Angel_ApplDeviceControl(devID, op, arg) \
        Angel_RawDeviceControl((devID), (op), (arg))

#define Angel_ApplDeviceYield() \
        angel_DeviceYield()

#define Angel_ApplReceiveMode(mode) \
        Angel_RawReceiveMode((mode))

#define Angel_ApplResetDevices() \
        Angel_RawResetDevices()

#define Angel_ApplInitialiseDevices() \
        Angel_RawInitialiseDevices()

#else

#include "devices.h"

/* General purpose constants, macros, enums, typedefs
 * These are defined in devices.h
 */

/* Publically-accessible globals */
/* none */

/* Public functions */

/*
 * Function: Angel_ApplDeviceWrite
 *  Purpose: The main entry point for blocking writes to a device.
 *
 *   Params:
 *              Input: devID     index of the device to write to
 *                     buff      data to write
 *                     length    how much data to write
 *             Output: -
 *             In/Out: -
 *
 *            Returns: DE_OKAY     write request completed
 *                     DE_NO_DEV   no such device                      
 *                     DE_BUSY     device busy with another write      
 *                     DE_INVAL    silly length                        
 *
 *      Reads globals: -
 *   Modifies globals: -
 *
 * Other side effects: -
 *
 * Write length bytes from buffer to the device, and block until complete.
 */

DevError Angel_ApplDeviceWrite(DeviceID devID, p_Buffer buff, unsigned length);


/*
 * Function: Angel_ApplDeviceRead
 *  Purpose: The main entry point for blocking reads from a device.
 *
 *   Params:
 *              Input: devID     index of the device to read from
 *                     buff      where to read the data to
 *                     length    how much data to read
 *             Output: -
 *             In/Out: -
 *
 *            Returns: DE_OKAY     read request completed
 *                     DE_NO_DEV   no such device                      
 *                     DE_BUSY     device busy with another read      
 *                     DE_INVAL    silly length                        
 *
 *      Reads globals: -
 *   Modifies globals: -
 *
 * Other side effects: -
 *
 * Read length bytes from the device to the buffer, blocking until complete.
 */

DevError Angel_ApplDeviceRead(DeviceID devID, p_Buffer buff, unsigned length);


/*
 * Function: Angel_ApplDeviceControl
 *  Purpose: Call a control function for a device
 *
 *   Params:
 *              Input: devID     index of the device to control to
 *                     op        operation to perform
 *                     arg       parameter depending on op
 *
 *            Returns: DE_OKAY     control request is underway           
 *                     DE_NO_DEV   no such device                      
 *                     DE_BAD_OP   device does not support operation
 *
 *      Reads globals: -
 *   Modifies globals: -
 *
 * Other side effects: -
 *
 * Have a device perform a control operation.  Extra parameters vary 
 * according to the operation requested.
 *
 * Important Note: When used in a full Angel system, this function
 * will not actually cause anything to happen, since Angel will have
 * control of the devices.  However, it will need to be called to set
 * up the device in the minimal Angel system since Angel will not own
 * the devices then.
 */

DevError Angel_ApplDeviceControl(DeviceID devID, DeviceControl op, void *arg);


/*
 * Function: Angel_ApplDeviceYield
 *  Purpose: allow any polled devices to do a poll
 *
 *   Params:    None
 *
 * Should be called in any spinloop, and often during app processing.
 */

void Angel_ApplDeviceYield(void);


/*
 * Function: Angel_ApplReceiveMode
 *  Purpose: enable or disable reception across all devices
 *
 *   Params:
 *              Input: mode   choose enable or disable
 *
 * Pass the mode parameter to the receive_mode control method of each
 * device
 *
 * Important Note: When used in a full Angel system, this function
 * will not actually cause anything to happen, since Angel will have
 * control of the devices.  However, it will need to be called to
 * enable / disable recption in the minimal Angel system since Angel
 * will not own the devices then.
 */

void Angel_ApplReceiveMode(DevRecvMode mode);


/*
 * Function: Angel_ApplResetDevices
 *  Purpose: reset all devices
 *
 *   Params: none
 *
 * Call the reset control method for each device
 *
 * Important Note: When used in a full Angel system, this function
 * will not actually cause anything to happen, since Angel will have
 * control of the devices.  However, it will need to be called to
 * reset the devices in the minimal Angel system since Angel will not
 * own the devices then.
 */

void Angel_ApplResetDevices(void);


/*
 * Function: Angel_ApplInitialiseDevices
 *  Purpose: initialise the device driver layer
 *
 *   Params: none
 *
 * Set up the device driver layer and call the init method for each device
 *
 * Important Note: When used in a full Angel system, this function
 * will not actually cause anything to happen, since Angel will have
 * control of the devices.  However, it will need to be called to set
 * up the devices in the minimal Angel system since Angel will not own
 * the devices then.
 */

void Angel_ApplInitialiseDevices(void);


#endif /* else ndef MINIMAL_ANGEL */

#endif /* ndef angel_devappl_h */

/* EOF devappl.h */
