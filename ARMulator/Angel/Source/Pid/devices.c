/* -*-C-*-
 *
 * $Revision: 1.13.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:55:42 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Device tables for PID card
 */
#include "devdriv.h"
#include "devconf.h"
/*
 * Headers declaring the angel_DeviceEntry struct for each device
 * should be included here:
 */
#include "st16c552.h"                   /* 2-port serial device */
#if DEBUG == 1 && LOGTERM_DEBUGGING
# include "logging/logterm.h"
#endif
#if DCC_SUPPORTED
# include "dccdrv.h"
#endif
#if ETHERNET_SUPPORTED
# include "ethernet.h"                  /* Fusion Ethernet */
#endif
#if PCMCIA_SUPPORTED
# include "pcmcia.h"                    /* PCMCIA Controller */
#endif
#if PROFILE_SUPPORTED
# include "prof.h"
#endif
/*
 * The master device table - one entry per device
 * ORDER AND NUMBER OF ENTRIES MUST MATCH enum DeviceIdent IN target.h
 */
const struct angel_DeviceEntry *const angel_Device[DI_NUM_DEVICES] =
{
    &angel_ST16C552Serial[0],
#if (ST16C552_NUM_PORTS > 1)
    &angel_ST16C552Serial[1],
#else
    &angel_NullDevice,
#endif
#if (ETHERNET_SUPPORTED > 0)
    &angel_EthernetDevice,
#else
    &angel_NullDevice,
#endif
#if DCC_SUPPORTED
    &angel_DccDevice,
#else
    &angel_NullDevice,
#endif
};

/*
 * The interrupt handler table - one entry per handler.
 *
 * DE_NUM_INT_HANDLERS must be set in devconf.h to the number of
 * entries in this table.
 */

#if (DE_NUM_INT_HANDLERS > 0)

const struct angel_IntHandlerEntry angel_IntHandler[DE_NUM_INT_HANDLERS] = {
    { angel_ST16C552IntHandler, DI_ST16C552_A },
# if (ST16C552_NUM_PORTS > 1)
    { angel_ST16C552IntHandler, DI_ST16C552_B },
#else
# if DEBUG == 1 && defined(LOGTERM_DEBUGGING) && (LOGTERM_DEBUGGING != 0)
    { angel_LogtermIntHandler, DI_ST16C552_B },
#else
    { angel_NodevIntHandler, 0},
# endif
# endif
    
# if PARALLEL_SUPPORTED
    { angel_ST16C552IntHandler, DI_PARALLEL },
#else
    { angel_NodevIntHandler, 0},
# endif
# if PCMCIA_SUPPORTED
    { angel_PCMCIAIntHandler, 0 },
    { angel_PCMCIAIntHandler, 0 }
#else
    { angel_NodevIntHandler, 0},
    { angel_NodevIntHandler, 0}
# endif
# if PROFILE_SUPPORTED
  ,{ Angel_TimerIntHandler, 0 }
# endif
};

#endif


/*
 * The poll handler table - one entry per handler
 *
 * DE_NUM_POLL_HANDLERS must be set in devconf.h to the number of
 * entries in this table.
 */
#if (DE_NUM_POLL_HANDLERS > 0)

const struct angel_PollHandlerEntry angel_PollHandler[DE_NUM_POLL_HANDLERS] = {
# if ETHERNET_SUPPORTED
    { angel_EthernetPoll, 0, angel_EthernetNOP, DI_ETHERNET },
# endif
#if DCC_SUPPORTED
    { (angel_PollHandlerFn)dcc_PollRead, DI_DCC,
      (angel_PollHandlerFn)dcc_PollWrite, DI_DCC },
#endif
};

#endif

/* EOF devices.c */
