/* -*-C-*-
 *
 * $Revision: 1.8 $
 *   $Author: amerritt $
 *     $Date: 1996/09/05 10:37:44 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Device tables -- just one serial device for now.
 */

#include "devdriv.h"

/*
 * Headers declaring the angel_DeviceEntry struct for each device
 * should be included here:
 */

#include "prof.h"
#include "serial.h"             /* serial device */
#include "devconf.h"

#ifdef JTAG_ADP_SUPPORTED
# include "jtag.h"
#endif


/*
 * The master device table - one entry per device
 * ORDER AND NUMBER OF ENTRIES MUST MATCH enum DeviceIdent IN devconf.h
 */
const struct angel_DeviceEntry * const angel_Device[DI_NUM_DEVICES] = {
  &angel_SerialDevice,
#ifdef JTAG_ADP_SUPPORTED
  &angel_JTAGDevice,
#endif
};


/*
 * The interrupt handler table - one entry per handler.
 *
 * DE_NUM_INT_HANDLERS must be set in devdriv.h to the number of
 * entries in this table.
 */

#if DE_NUM_INT_HANDLERS>0

const struct angel_IntHandlerEntry angel_IntHandler[DE_NUM_INT_HANDLERS] = {
  { angel_SerialIntHandler, SER_DEV_IDENT_0 }
#if PARALLEL_SUPPORTED
  ,{ angel_SerialIntHandler, PAR_DEV_IDENT }
#endif
#if PROFILE_SUPPORTED
  ,{ Angel_TimerIntHandler, 0 }
#endif
};

#endif


/*
 * The poll handler table - one entry per handler
 *
 * DE_NUM_POLL_HANDLERS must be set in devdriv.h to the number of
 * entries in this table.
 */

#if DE_NUM_POLL_HANDLERS

const struct angel_PollHandlerEntry angel_PollHandler[DE_NUM_POLL_HANDLERS] = {
# ifdef JTAG_ADP_SUPPORTED
    { angel_JTAGPollHandler, JTAG_DEV_IDENT_0,
      angel_JTAGPollHandler, JTAG_DEV_IDENT_0 },
# endif
};

#endif

/* EOF devices.c */
