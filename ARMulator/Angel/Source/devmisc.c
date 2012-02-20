/* -*-C-*-
 *
 * $Revision: 1.3.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:01 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Miscellaneous device stuff required in full AND minimal systems.
 */

#include "devdriv.h"
#include "logging.h"

/* The semi-public status of each device */
volatile unsigned int angel_DeviceStatus[DI_NUM_DEVICES];

/**********************************************************************/

void 
angel_DeviceYield(void)
{
#if (DE_NUM_POLL_HANDLERS > 0)

    int i;
    const struct angel_PollHandlerEntry *entry;

    TRACE("dye");

    for (i = 0; i < DE_NUM_POLL_HANDLERS; ++i)
    {
        entry = &angel_PollHandler[i];

        entry->read_handler(entry->read_data);
        entry->write_handler(entry->write_data);
    }

#endif
}

/*
 *  Function: Nulldev
 *   Purpose: Do nothing function that returns an OK condition
 *
 *    Params: None
 *
 *   Returns: DE_OKAY
 */
static DevError 
Nulldev(void)
{
    return DE_OKAY;
}

/*
 *  Function: angel_NodevIntHandler
 *   Purpose: Place-holding interrupt function that reports an error
 *
 *    Params: As described in devdriv.h
 *
 *   Returns: This routine does not return
 */
void 
angel_NodevIntHandler(unsigned int ident, unsigned int data,
                      unsigned int empty_stack)
{
    IGNORE(data);
    IGNORE(empty_stack);

    LogError(LOG_DEVMISC, ( "NodevIntHandler called: ident %d\n", ident));
}

/**********************************************************************/

const struct angel_DeviceEntry angel_NullDevice =
{
    DT_ANGEL,
    {
        (angel_DeviceWriteFn) Nulldev,
        (angel_DeviceRegisterReadFn) Nulldev
    },
    (angel_DeviceControlFn) Nulldev,
    0,
    {
        0, NULL
    },
    {
        0, NULL
    }
};

/* EOF devmisc.c */
