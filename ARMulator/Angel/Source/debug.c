/*> debug.c < */
/*---------------------------------------------------------------------------*/
/* This file provides the main debug agent support for Angel. It deals
 * with certain messages, and with routing certain messages to
 * specific handlers. i.e. certain CPU related messages are sent to
 * the debug-hardware module, whilst O/S specific messages are passed
 * up through Angel to the environment executing on top of Angel.
 *
 * $Revision: 1.6.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:54:36 $
 *
 * Copyright Advanced RISC Machines Limited, 1995.
 * All Rights Reserved.
 */
/*---------------------------------------------------------------------------*/

#include <stdarg.h>             /* Standard varargs processing */
#include "angel.h"              /* Generic Angel definitions */
#include "adp.h"                /* ADP protocol */
#include "arm.h"                /* ARM processor definitions */
#include "debug.h"              /* header for this file */
#include "endian.h"             /* For target independant endianness access */
#include "debughwi.h"           /* Hardware debugging functions interface. */
#include "channels.h"           /* Channels stuff */
#include "msgbuild.h"           /* unpack_message(), etc. */
#include "logging.h"            /* LogError() etc. */
#include "serlock.h"            /* Angel_Yield() */
#include "devconf.h"

/*---------------------------------------------------------------------------*/

/* Global (single-threaded) data. Certain parts of the debug agent are
 * single-threaded, so we can have some globally allocated Angel
 * variables. If multi-threaded code needs to manipulate these
 * variables, then suitable synchronisation should be provided in the
 * code to ensure atomic access.
 */


/* The "debug_Status" variable holds the current debug agent state: */
unsigned32 debug_Status = 0x00000000;

/* The "debug_VectorCatch" value is a bitmask of ARM exception vectors
 * and Angel vectors that will raise pseudo-breakpoints to the host
 * when taken.
 */
unsigned32 debug_VectorCatch = 0x00000000;

int boot_completed = 0;

/*---------------------------------------------------------------------------*/

/*
 * .. Software breakpoints ..
 * 
 * .. Those where the breakpoint is set within 2^26 of the breakpoint entry routine
 * .. a direct branch instruction can be used .. can breakpoint in any mode
 * .. requires run-time support to enter SVC mode
 * .. limits placing of breakpoints and address of handler
 * .. requires each breakpoint to have a stub entry, that can be used to uniquely identify the breakpoint
 * 
 * .. Those where a direct PC instruction can be used to load the breakpoint entry routine address
 * .. direct breakpoint entry, can be entered in any mode
 * .. requires run-time support to enter SVC mode
 * .. requires each breakpoint to have a stub entry, that can be used to uniquely identify the breakpoint
 * .. Handler entry stubs at a fixed low address, but breakpoints can be anywhere
 * MOV     pc,#<address>
 * 
 * .. Those where an undefined instruction is required
 * .. Cannot be used to safely breakpoint code executing in UNDef mode
 * .. Does *NOT* require run-time support to enter SVC mode
 * .. Does *NOT* require a seperate entry stub for each breakpoint
 * .. Breakpoint and breakpoint handling code can live at any address
 */

/* The last of these options is the one chosen for the default Angel
 * implementation. However, if soft-breakpoints are required to debug
 * UNDefined mode code then one of the previous two schemes must be
 * used. These options are left as alternative Angel implementations
 * to be provided as conditional build systems.
 */

/* TODO: .. possible mutex on asynchronous event code ..
 * .. to avoid eating all the free buffers ..
 * .. e.g. wait until current breakpoint message sent before sending another .. */

/*---------------------------------------------------------------------------*/
/* ADP message vectoring
 * ---------------------
 * The indirection tables used by the debug world are defined in
 * assembler. This allows *ALL* of the handler functions to be
 * imported WEAK. If the developer then does not include a sub-section
 * (e.g. breakpoint support), then the table will have zero slots for
 * those functions. The vectoring code can then check for these, and
 * perform suitable error message generation if there is no handler.
 *
 * This is better than having a lot of conditionals in the 'C' source,
 * since now all that is required to remove handler functions is
 * simply to build an Angel with the relevant object files (or AREAs)
 * missing.
 */

static void 
debugHandler(DeviceID devid, ChannelID chanid,
             p_Buffer buffer, unsigned int length,
             void *stateptr)
{
    unsigned int reason;
    int err;

    IGNORE(devid);
    
    if (length < 4)
    {
        LogWarning(LOG_DEBUG, ("debugHandler: buffer 0x%08X too small (%d bytes)\n",
                     (unsigned int)buffer, length));
        angel_ChannelReleaseBuffer(buffer);
        return;
    }
    unpack_message(BUFFERDATA(buffer), "%w", &reason);

    LogInfo(LOG_BOOT, ("debugHandler: message received: reason 0x%08x (%s), chanid %d\n",
                         reason, log_adpname(reason), chanid));

    /* Check to make sure booting is finished. */
    if (boot_completed == 0)
    {
        p_Buffer rbuff;

        LogError(LOG_DEBUG, ( "Entered debugHandler before boot has completed."));
        angel_ChannelReleaseBuffer(buffer);
        rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
        if (rbuff != NULL)
        {
            int count;

            count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w", reason | TtoH,
                             0, ADP_HandleUnknown, ADP_HandleUnknown,
                             RDIError_NotInitialised);
            angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);
        }
        else
            LogError(LOG_DEBUG, ( "Couldn't allocate buffer in debugHandler"));
        return;
    }

    if (reason & TtoH)
    {
        LogWarning(LOG_DEBUG, ( "Warning: debugHandler has received TtoH msg - ignoring \n"));
    }
    else
    {
        if (chanid == CI_HADP)
        {   
            /* Range check reason code. */
            if ((reason & 0xFFFF) < hfptr_max)
            {
                LogInfo(LOG_DEBUG, ( "Jumping to vector table entry (reason code): 0x%x\n", reason & 0xFFFF));
                err = hfptr[reason & 0xFFFF] (&buffer, stateptr);
            }
            else
                err = -1;

            if (err == -1)
            {
                /* Message isn't implemented so return a ADP_Unrecognised message. */
                p_Buffer rbuff;

                LogWarning(LOG_DEBUG, ( "debugHandler: Message not implemented, "
                                          "reason 0x%x\n", reason & 0xffff));
                rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
                if (rbuff != NULL)
                {
                    int count;

                    count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w",
                                     ADP_HADPUnrecognised | TtoH, 0, ADP_HandleUnknown,
                                     ADP_HandleUnknown, reason);
                    angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);
                }
                else
                {
                    LogError(LOG_DEBUG, ( "ERROR: Couldn't allocate buffer in function debugH..."));
                }
            }
        }
        else
        {
            
            LogWarning(LOG_DEBUG, ( "Warning: debugHandler received message for channel %d\n", chanid));
        }
    }

    /* if the buffer is passed to a vector table entry, it may get freed there */
    if (buffer != NULL)
    {
        LogInfo(LOG_DEBUG, ("debugHandler: freeing request buffer\n"));
        angel_ChannelReleaseBuffer(buffer);
    }
    
    /* Always return OK, buffer excepted even if contents are ignored. */
    return;
}

/*
 * .. Message handling
 * 
 * .. Direct action (immediate bounded effect)
 * .. Angel debug must be in control in single-threaded world
 * .. Angel multi-threaded debug must exist in multi-threaded world
 * .. Host-to-Target message
 * .. Target performs action
 * .. Target returns ACK (with any required data)
 * .. Debug agent still has control of the target
 * 
 * .. In-direct action (unbounded effect)
 * .. Angel debug must be in control in single-threaded world
 * .. Angel multi-threaded debug must exist in multi-threaded world
 * .. Host-to-Target message
 * .. Target schedules some action
 * .. Target returns ACK
 * .. Target performs action (e.g. Execute)
 * .. Debug agent no longer has control of the target
 * 
 */

/*
 * .. single-threaded debugging
 * .. for debugging OSs, and the multi-threaded debugging system
 * 
 * .. multi-threaded debugging
 * .. when the OS is active, and we are debugging threads within it
 * 
 * .. interaction (through a defined API) with the hardware debugging features (e.g. Breakpoints, Watchpoints, etc.)
 */


/*---------------------------------------------------------------------------*/
/* debug_ThreadStopped
 * ------------------_
 * This routine deals with notifying the host of asynchronous target
 * events. It raises an "ADP_Stopped" message with the relevant
 * parameters.
 *
 * NOTE: This is a varargs function, to cope with the fact that some
 * messages have arguments to be passed. This code does not perform
 * any argument checking or verification, and it is assumed that the
 * caller of this function has passed the correct number and type of
 * arguments.
 */

static volatile int stopping;

typedef struct
{
    ChanRx_CB_Fn fn;
    void *state_ptr;
}
debug_stophandlerargs;

/* Handler to receive ack. for stopped message. */
static void 
stoppedHandler(DeviceID devid, ChannelID chanid,
               p_Buffer buffer, unsigned int length,
               void *stateptr)
{
    word reason, debugID, OSinfo1, OSinfo2, status;

    IGNORE(devid);
    IGNORE(chanid);
    IGNORE(stateptr);

    LogInfo(LOG_DEBUG, ( "DEBUG: Entered stoppedHandler."));
    if (length < 20)
    {
        LogWarning(LOG_DEBUG, ("Message shorter than expected!!!"));
    }

    unpack_message(BUFFERDATA(buffer), "%w%w%w%w%w", &reason, &debugID,
                   &OSinfo1, &OSinfo2, &status);
    angel_ChannelReleaseBuffer(buffer);

    if (reason != (ADP_Stopped | HtoT))
    {
        LogInfo(LOG_DEBUG, ( "Unexpected reason code in stoppedHandler: %x\n", reason));
        return;
    }
    if (status != RDIError_NoError)
    {
        LogWarning(LOG_DEBUG, ( "Bad status in stopped ACK"));
        return;
    }

    LogInfo(LOG_DEBUG, ( "got stopped ACK"));
    stopping = FALSE;
    return;
}

void 
debug_ThreadStopped(word OSinfo1, word OSinfo2, unsigned int event,
                    unsigned int subcode)
{
    /*
     * Called from except.s as a callback,
     * also from debughwi.c:gendbg_interruptrequest()
     */
    IGNORE(subcode);
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    if (debug_Status & debug_Status_Init)
    {
        p_Buffer pbuff;
        word status = 0;

        LogInfo(LOG_DEBUG, ( "Entered function debug_ThreadStopped.\n"));
        LogInfo(LOG_DEBUG, ( "Reason no. for stopping = %x\n", event));

        switch (event)
        {
            case ADP_Stopped_BreakPoint:
                LogInfo(LOG_DEBUG, ( "ADP_Stopped_BreakPoint."));
                break;
        }

        /* Send an ADP_Stopped. */
        pbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
        if (pbuff != NULL)
        {
            int count;
            ChanError err;

            count = msgbuild(BUFFERDATA(pbuff), "%w%w%w%w%w%w",
                             (ADP_Stopped | TtoH), 0, ADP_HandleUnknown,
                             ADP_HandleUnknown, event, status);

            /* Register a special handler to receive the reply. */
            LogInfo(LOG_DEBUG, ( "Registering stoppedHandler."));
            err = angel_ChannelReadAsync(CH_DEFAULT_DEV, CI_TADP,
                                         stoppedHandler,
                                         NULL);
            if (err != RDIError_NoError)
                LogError(LOG_DEBUG, ( "Register failed."));

            LogInfo(LOG_DEBUG, ( "Transmitting stopped message."));
            stopping = TRUE;
            err = angel_ChannelSend(CH_DEFAULT_DEV, CI_TADP, pbuff, count);
            if (err != RDIError_NoError)
                LogWarning(LOG_DEBUG, ( "Transmit failed."));

            LogInfo(LOG_DEBUG, ( "Waiting for stopped ack..."));

            while (stopping)
                Angel_Yield();

            LogInfo(LOG_DEBUG, ( "Unregistering stoppedHandler."));
            err = angel_ChannelReadAsync(CH_DEFAULT_DEV, CI_TADP,
                                         NULL, NULL);
            if (err != RDIError_NoError)
                LogError(LOG_DEBUG, ( "Unregister failed."));

            LogInfo(LOG_DEBUG, ( "Returning..."));

        }
        else
        {
            LogError(LOG_DEBUG, ( "ERROR: Couldn't allocate buffer in debug_ThreadStopped."));
            return;
        }
    }
    /* Else return to the caller. */
    return;
}

/*---------------------------------------------------------------------------*/
/* angel_DebugInit
 * ---------------
 * This is the starting point for attaching the Angel debug agent. At
 * the moment this single entry point must deal with setting up both
 * the single- and multi-threaded debugging handlers.
 */
void 
angel_DebugInit(void)
{
    LogInfo(LOG_BOOT, ( "angel_DebugInit: entered\n"));

    /* Open the channel for single-threaded debugging messages */
    {
        if (angel_ChannelReadAsync(CH_DEFAULT_DEV, CI_HADP, debugHandler,
                                   NULL) < 0)
        {
            LogError(LOG_DEBUG, ( "angel_DebugInit: Failed to attach handler for CI_HADP"));
        }
    }
    /*
     * TODO: Comments about preserving break-points on
     * application initialisation (rom ROM initialisation)
     */
    /*
     * .. NOTE: We will need magic here to deal with automatically
     * .. setting a breakpoint on "main" when building ROM applications.
     * .. Similarly we have the problem of the application being downloaded
     * .. using the boot monitor, the user then setting a breakpoint on
     * .. main, the application being started, which then provides a new
     * .. breakpoint handler (as part of its Angel initialisation). We need
     * .. to ensure that breakpoints are passed through so that the
     * .. application library based Angel will still stop when main is
     * .. reached.
     */

    /* NOTE: We explicitly set the single "Init"ialised flag, to clear
     * the rest of the debug agent state bits:
     */
    debug_Status = (debug_Status_Init);

    return;
}
