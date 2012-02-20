/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: cgood $
 *     $Date: 1996/06/18 09:37:33 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Thumb comms channel driver
 */

#ifndef angel_dcc_h
#define angel_dcc_h

#include "devdriv.h"

extern const struct angel_DeviceEntry angel_DccDevice;
DevError dcc_PollWrite(DeviceID devid);
DevError dcc_Write(DeviceID devid, p_Buffer buffer,
                               unsigned int length, DevChanID devchan);
DevError dcc_PollRead(DeviceID devid);

#endif /* ndef angel_dcc_h */

/* EOF dcc.h */
