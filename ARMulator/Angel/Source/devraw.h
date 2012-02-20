/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: mgray $
 *     $Date: 1996/07/12 16:46:09 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: User Application interface to devices
 */

#ifndef angel_devraw_h
#define angel_devraw_h

/*
 * This header exports the actual raw interface available to 
 * the User Application, used either directly in a minimal system 
 * or via SWIs and devshare.h in a non-minmal system.
 *
 * This header is NOT intended for direct inclusion by user applications -
 * see instead devappl.h.
 */

#include "devices.h"

/* General purpose constants, macros, enums, typedefs
 * These are defined in devices.h
 */

/* Publically-accessible globals */
/* none */

/* Public functions */

/*
 * Function: Angel_RawDeviceWrite
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

DevError Angel_RawDeviceWrite(DeviceID devID, p_Buffer buff, unsigned length);


/*
 * Function: Angel_RawDeviceRead
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

DevError Angel_RawDeviceRead(DeviceID devID, p_Buffer buff, unsigned length);


/*
 * Function: Angel_RawDeviceControl
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

DevError Angel_RawDeviceControl(DeviceID devID, DeviceControl op, void *arg);


#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0

/*
 * Function: Angel_RawDeviceYield
 *  Purpose: allow any polled devices to do a poll
 *
 *   Params:    None
 *
 * Should be called in any spinloop, and often during app processing.
 */

void Angel_RawDeviceYield(void);


/*
 * Function: Angel_RawReceiveMode
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

void Angel_RawReceiveMode(DevRecvMode mode);


/*
 * Function: Angel_RawResetDevices
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

void Angel_RawResetDevices(void);


/*
 * Function: Angel_RawInitialiseDevices
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

void Angel_RawInitialiseDevices(void);

#endif /* def MINIMAL_ANGEL */


#endif /* ndef angel_devraw_h */

/* EOF devraw.h */
