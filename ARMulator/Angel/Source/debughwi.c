/* -*-C-*-
 *
 * $Revision: 1.9.4.3 $
 *   $Author: rivimey $
 *     $Date: 1998/01/15 11:43:30 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * debughwi.c: This file contains the target independant debug
 * handler functions for Angel.
 */
#include "angel.h"
#include "adp.h"
#include "channels.h"
#include "debug.h"
#include "debugos.h"
#include "endian.h"
#include "debughwi.h"
#include "logging.h"
#include "msgbuild.h"
#include "devconf.h"
#include "buffers.h"

int 
gendbg_unimplemented(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);

    return -1;
}

static int 
gendbg_unrecognised(p_Buffer * buffer, void *stateptr)
{
    int reason, debugID, OSinfo1, OSinfo2;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_unrecognised\n"));

    unpack_message(BUFFERDATA(*buffer), "%w%w%w%w", &reason, &debugID, &OSinfo1,
                   &OSinfo2);

    return msgsend(CI_HADP, "%w%w%w%w%w", (ADP_HADPUnrecognised | TtoH),
                   debugID, OSinfo1, OSinfo2, reason);
}

/**********************************************************************/

/*
 * Information message handler functions
 */
static int 
gendbg_info(p_Buffer * buffer, void *stateptr)
{
    int subreason, err;

    subreason = PREAD(LE, BUFFERDATA(*buffer) + 16);
    subreason &= 0xFFFF;        /* Mask out channel information. */

    LogInfo(LOG_DEBUGHWI, ( "gendbg_info(subreason = %d)\n", subreason));


    /* Range check subreason code. */
    if (subreason < info_hfptr_max)
        err = info_hfptr[subreason] (buffer, stateptr);
    else
        err = -1;

    return err;
}

/**********************************************************************/

/*
 * Control message handler functions
 */
static int 
gendbg_ctrl(p_Buffer * buffer, void *stateptr)
{
    int subreason, err;

    subreason = PREAD(LE, (unsigned int *)(BUFFERDATA(*buffer) + 16));
    subreason &= 0xFFFF;        /* Mask out channel information. */
    
    LogInfo(LOG_DEBUGHWI, ( "gendbg_ctrl(subreason %d)\n", subreason));

    /* Range check subreason code. */
    if (subreason < ctrl_hfptr_max)
        err = ctrl_hfptr[subreason] (buffer, stateptr);
    else
        err = -1;

    return err;
}

/**********************************************************************/

/*
 * General debugging message handler functions
 */
static int 
gendbg_read(p_Buffer * buffer, void *stateptr)
{
    /* Reads an area of memory. */
    p_Buffer rbuff;
    int debugID, OSinfo1, OSinfo2;
    word address, nbytes;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &address, &nbytes);
    LogInfo(LOG_DEBUGHWI, ( "gendbg_read(address = 0x%x, nbytes = %d)\n", address, nbytes));

    rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (rbuff != NULL)
    {
        int count;
        word bytesread;
        word status = RDIError_Error;

        count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w%w", (ADP_Read | TtoH),
                         debugID, OSinfo1, OSinfo2, status, 0);

        status = angelOS_MemRead(OSinfo1, OSinfo2, address, nbytes,
                                 BUFFERDATA(rbuff) + count, &bytesread);

        /* Write status field to buffer. */
        PUT32LE(BUFFERDATA(rbuff) + 16, status);

        /* Write number of bytes not read into buffer. */
        PUT32LE(BUFFERDATA(rbuff) + 20, nbytes - bytesread);

        count += bytesread;
        
        angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);
    }
    else
        LogError(LOG_DEBUGHWI, ( "ERROR: Couldn't allocate buffer in 'gendbg_read'.\n"));

    return 0;
}

static int 
gendbg_write(p_Buffer * buffer, void *stateptr)
{
    /* Writes an area of memory. */
    int reason, debugID, OSinfo1, OSinfo2, count;
    word address, nbytes;
    word byteswritten, status;

    IGNORE(stateptr);

    count = unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%w%w", &reason,
                           &debugID, &OSinfo1, &OSinfo2, &address, &nbytes);
    
    LogInfo(LOG_DEBUGHWI, ( "gendbg_write(address = 0x%x, nbytes = %d)\n", address, nbytes));

    status = angelOS_MemWrite(OSinfo1, OSinfo2, address, nbytes,
                              BUFFERDATA(*buffer) + count, &byteswritten);

    /* We make sure we release the buffer here as it may be the long one */
    angel_ChannelReleaseBuffer(*buffer);
    *buffer = NULL;

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Write | TtoH),
                   debugID, OSinfo1, OSinfo2, status,
                   nbytes - byteswritten);
}

static int 
gendbg_cpread(p_Buffer * buffer, void *stateptr)
{
    /* Reads values from a co-processor. */
    p_Buffer rbuff;
    int debugID, OSinfo1, OSinfo2;
    byte cpnum;
    word mask;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%b%w", &debugID, &OSinfo1,
                   &OSinfo2, &cpnum, &mask);
    LogInfo(LOG_DEBUGHWI, ( "gendbg_cpread(cpnum %d, mask 0x%x)\n", cpnum, mask));

    rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (rbuff != NULL)
    {
        int count;
        word status = RDIError_Error;
        word nbytes;

        count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w", (ADP_CPread | TtoH),
                         debugID, OSinfo1, OSinfo2, status);
        status = angelOS_CPRead(OSinfo1, OSinfo2, cpnum, mask,
                                (word *) (BUFFERDATA(rbuff) + count), &nbytes);

        /* Write status field to buffer. */
        PUT32LE(BUFFERDATA(rbuff) + 16, status);

        count += nbytes;
        angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);
    }
    else
        LogError(LOG_DEBUGHWI, ( "ERROR: Couldn't allocate buffer in 'gendbg_cpread'.\n"));

    return 0;
}

static int 
gendbg_cpwrite(p_Buffer * buffer, void *stateptr)
{
    /* Writes values to a co-processor. */
    int reason, debugID, OSinfo1, OSinfo2, count;
    byte cpnum;
    word mask;
    word nbytes, status;

    IGNORE(stateptr);

    count = unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%b%w", &reason,
                           &debugID, &OSinfo1, &OSinfo2, &cpnum, &mask);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_cpwrite(cpnum %d, mask 0x%x)\n", cpnum, mask));

    status = angelOS_CPWrite(OSinfo1, OSinfo2, cpnum, mask,
                             (word *) (BUFFERDATA(*buffer) + count), &nbytes);

    return msgsend(CI_HADP, "%w%w%w%w%w", (ADP_CPwrite | TtoH),
                   debugID, OSinfo1, OSinfo2, status);
}

static int 
gendbg_cpuread(p_Buffer * buffer, void *stateptr)
{
    /* Reads values from the CPU. */
    p_Buffer rbuff;
    int debugID, OSinfo1, OSinfo2;
    word mask;
    byte mode;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_cpuread:\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%b%w", &debugID, &OSinfo1,
                   &OSinfo2, &mode, &mask);

    rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (rbuff != NULL)
    {
        int count, i;
        word status = RDIError_Error;

        count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w", (ADP_CPUread | TtoH),
                         debugID, OSinfo1, OSinfo2, status);
        status = angelOS_CPURead(OSinfo1, OSinfo2, mode, mask,
                                 BUFFERDATA(rbuff) + count);

        LogInfo(LOG_DEBUGHWI, ( "CPU read mask %x mode %x status = %x\n", mask, mode, status));

        /*
         * Increment count by the number of registers we have.
         */
        for (i = 0; i < 20; i++)
            if (mask & (1L << i))
                count += 4;

        /* Write status to buffer. */
        PUT32LE(BUFFERDATA(rbuff) + 16, status);

        angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);
    }
    else
        LogError(LOG_DEBUGHWI, ( "ERROR: Couldn't allocate buffer in 'gendbg_cpuread'.\n"));

    return 0;
}

static int 
gendbg_cpuwrite(p_Buffer * buffer, void *stateptr)
{
    /* Writes values to the CPU. */
    int reason, debugID, OSinfo1, OSinfo2;
    int count;
    byte mode;
    word mask;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_cpuwrite:\n"));
    count = unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%b%w", &reason,
                           &debugID, &OSinfo1, &OSinfo2, &mode, &mask);

    LogInfo(LOG_DEBUGHWI, ( "CPU write mask %x mode %x\n", mask, mode));

    status = angelOS_CPUWrite(OSinfo1, OSinfo2, mode, mask,
                              BUFFERDATA(*buffer) + count);

    LogInfo(LOG_DEBUGHWI, ( "status = %x\n", status));

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_CPUwrite | TtoH),
                   debugID, OSinfo1, OSinfo2, status);
}

static int 
gendbg_setbreak(p_Buffer * buffer, void *stateptr)
{
    /* Sets a breakpoint. */
    int reason, debugID, OSinfo1, OSinfo2;
    int count;
    struct SetPointValue spv;
    struct PointValueReturn pvr;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "DEBUG: Entered function gendbg_setbreak.\n"));
    count = unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%w%b", &reason,
                           &debugID, &OSinfo1, &OSinfo2, &spv.pointAddress,
                           &spv.pointType);

    if ((spv.pointType & 0x7) > 4)
        unpack_message(BUFFERDATA(*buffer) + count, "%w", &spv.pointBound);

    status = angelOS_SetBreak(OSinfo1, OSinfo2, &spv, &pvr);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w",
                   (ADP_SetBreak | TtoH), debugID, OSinfo1, OSinfo2, status,
                   pvr.pointHandle, pvr.pointRaddress, pvr.pointRbound);
}

static int 
gendbg_clearbreak(p_Buffer * buffer, void *stateptr)
{
    /* Clears a breakpoint. */
    int reason, debugID, OSinfo1, OSinfo2;
    word handle;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_clearbreak:\n"));
    unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%w", &reason, &debugID,
                   &OSinfo1, &OSinfo2, &handle);

    status = angelOS_ClearBreak(OSinfo1, OSinfo2, handle);

    return msgsend(CI_HADP, "%w%w%w%w%w", (ADP_ClearBreak | TtoH),
                   debugID, OSinfo1, OSinfo2, status);
}

static int 
gendbg_setwatch(p_Buffer * buffer, void *stateptr)
{
    /* Sets a watch point. */
    int reason, debugID, OSinfo1, OSinfo2;
    int count;
    word status;
    struct SetPointValue spv;
    struct PointValueReturn pvr;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_setwatch:\n"));
    count = unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%w%b%b", &reason,
                           &debugID, &OSinfo1, &OSinfo2, &spv.pointAddress,
                           &spv.pointType, &spv.pointDatatype);

    if ((spv.pointType & 0x7) > 4)
        unpack_message(BUFFERDATA(*buffer) + count, "%w", &spv.pointBound);

    status = angelOS_SetWatch(OSinfo1, OSinfo2, &spv, &pvr);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w",
                   (ADP_SetWatch | TtoH), debugID, OSinfo1, OSinfo2, status,
                   pvr.pointHandle, pvr.pointRaddress, pvr.pointRbound);
}

static int 
gendbg_clearwatch(p_Buffer * buffer, void *stateptr)
{
    /* Clears a watch point. */
    int reason, debugID, OSinfo1, OSinfo2;
    word handle;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_clearwatch:\n"));
    unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%w", &reason, &debugID,
                   &OSinfo1, &OSinfo2, &handle);

    status = angelOS_ClearWatch(OSinfo1, OSinfo2, handle);

    return msgsend(CI_HADP, "%w%w%w%w%w", (ADP_ClearWatch | TtoH),
                   debugID, OSinfo1, OSinfo2, status);
}

static int 
gendbg_execute(p_Buffer * buffer, void *stateptr)
{
    /* Start executing from the stored CPU state. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_execute:\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

#if DEBUG == 1
    angelOS_PrintState(OSinfo1, OSinfo2);
#endif

    status = RDIError_NoError;

    if (!msgsend(CI_HADP, "%w%w%w%w%w", (ADP_Execute | TtoH), debugID,
                 OSinfo1, OSinfo2, status))
    {
        angelOS_Execute(OSinfo1, OSinfo2);
        return 0;
    }
    else
        return -1;
}

static int 
gendbg_step(p_Buffer * buffer, void *stateptr)
{
    /* Step through program. */
    int debugID, OSinfo1, OSinfo2;
    word ninstr, status = RDIError_NoError;
    void *handle;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_step:\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &ninstr);

    status = angelOS_SetupStep(OSinfo1, OSinfo2, ninstr, &handle);

    if (!msgsend(CI_HADP, "%w%w%w%w%w", (ADP_Step | TtoH), debugID, OSinfo1,
                 OSinfo2, status))
    {
        if (status == RDIError_NoError)
        {
            (void)angelOS_DoStep(handle);
            return 0;
        }
    }

    return -1;
}

static int 
gendbg_interruptrequest(p_Buffer * buffer, void *stateptr)
{
    /* Stop program execution. */
    int debugID, OSinfo1, OSinfo2;
    word status, send_status;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_interruptrequest:\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    status = angelOS_InterruptExecution(OSinfo1, OSinfo2);

    /* send the reply */
    send_status = msgsend(CI_HADP, "%w%w%w%w%w", (ADP_InterruptRequest | TtoH),
                          debugID, OSinfo1, OSinfo2, status);

#ifndef ICEMAN2
    if (status == RDIError_NoError && send_status == RDIError_NoError)
    {
        /* send the stopped message and await a reply */
        debug_ThreadStopped(OSinfo1, OSinfo2, ADP_Stopped_UserInterruption, 0);
    }
#endif

    return RDIError_NoError;
}

/**********************************************************************/

/*
 * Hardware emulation message handler functions
 */
static int 
gendbg_hw_emul(p_Buffer * buffer, void *stateptr)
{
    unsigned int subreason;
    int err;

    LogInfo(LOG_DEBUGHWI, ( "gendbg_hw_emul:\n"));
    subreason = PREAD(LE, BUFFERDATA(*buffer) + 16);
    subreason &= 0xFFFF;        /* Mask out channel information. */

    /* Range check subreason code. */
    if (subreason < hwemul_hfptr_max)
        err = hwemul_hfptr[subreason] (buffer, stateptr);
    else
        err = -1;

    return err;
}

static int 
hwemul_supported(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);

    return -1;
}

static int 
hwemul_memoryaccess(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);

    return -1;
}

static int 
hwemul_memorymap(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);

    return -1;
}

static int 
hwemul_setcpuspeed(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);

    return -1;
}

static int 
hwemul_readclock(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);

    return -1;
}

/**********************************************************************/

/*
 * ICEbreaker message handler functions
 */
static int 
gendbg_iceb(p_Buffer * buffer, void *stateptr)
{
    unsigned int subreason;
    int err;

    LogInfo(LOG_DEBUGHWI, ( "gendbg_iceb:\n"));
    subreason = PREAD(LE, BUFFERDATA(*buffer) + 16);
    subreason &= 0xFFFF;        /* Mask out channel information. */

#ifdef ICEMAN2
    /* Range check subreason code. */
    if (subreason < iceb_hfptr_max)
        err = iceb_hfptr[subreason] (buffer, stateptr);
    else
        err = -1;
#else
    /* No iceb support in Angel */
    IGNORE(stateptr);
    err = -1;
#endif

    return err;
}

/**********************************************************************/

/*
 * ICEman message handler functions
 */
static int 
gendbg_icem(p_Buffer * buffer, void *stateptr)
{
    unsigned int subreason;
    int err;

    LogInfo(LOG_DEBUGHWI, ( "gendbg_icem:\n"));
    subreason = PREAD(LE, BUFFERDATA(*buffer) + 16);
    subreason &= 0xFFFF;        /* Mask out channel information. */

#ifdef ICEMAN2
    /* Range check subreason code. */
    if (subreason < icem_hfptr_max)
        err = icem_hfptr[subreason] (buffer, stateptr);
    else
        err = -1;
#else
    /* No icem support in Angel */
    IGNORE(stateptr);
    err = -1;
#endif

    return err;
}

/**********************************************************************/

/*
 * Profiling message handler functions
 */
static int 
gendbg_profile(p_Buffer * buffer, void *stateptr)
{
    unsigned int subreason;
    int err;

    LogInfo(LOG_DEBUGHWI, ( "DEBUG: Entered function gendbg_profile.\n"));
    subreason = PREAD(LE, BUFFERDATA(*buffer) + 16);
    subreason &= 0xFFFF;        /* Mask out channel information. */

#if PROFILE_SUPPORTED
    /* Range check subreason code. */
    if (subreason < profile_hfptr_max)
        err = profile_hfptr[subreason] (buffer, stateptr);
    else
        err = -1;
#else
    IGNORE(stateptr);
    IGNORE(buffer);
    err = -1;
#endif

    return err;
}

static int 
gendbg_initialiseapp(p_Buffer * buffer, void *stateptr)
{
    /* Initialise Application */
    int debugID, OSinfo1, OSinfo2;
    int status;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_initialiseapp:\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    status = angelOS_InitialiseApplication(OSinfo1, OSinfo2);

    return msgsend(CI_HADP, "%w%w%w%w%w",
                   (ADP_InitialiseApplication | TtoH),
                   debugID, OSinfo1, OSinfo2, status);
}

static int 
gendbg_end(p_Buffer * buffer, void *stateptr)
{
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;

    IGNORE(stateptr);

    LogInfo(LOG_DEBUGHWI, ( "gendbg_end:\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    status = msgsend(CI_HADP, "%w%w%w%w%w", (ADP_End | TtoH),
                     debugID, OSinfo1, OSinfo2, status);

    (void)angelOS_End(debugID);
    
    return status;
}

/* HADP message handlers */
const handler_function_pointer hfptr[] =
{
    gendbg_unrecognised,
    gendbg_info,
    gendbg_ctrl,
    gendbg_read,
    gendbg_write,
    gendbg_cpuread,
    gendbg_cpuwrite,
    gendbg_cpread,
    gendbg_cpwrite,
    gendbg_setbreak,
    gendbg_clearbreak,
    gendbg_setwatch,
    gendbg_clearwatch,
    gendbg_execute,
    gendbg_step,
    gendbg_interruptrequest,
    gendbg_hw_emul,
    gendbg_iceb,
    gendbg_icem,
    gendbg_profile,
    gendbg_initialiseapp,
    gendbg_end
};

const handler_function_pointer hwemul_hfptr[] =
{
    hwemul_supported,
    hwemul_memoryaccess,
    hwemul_memorymap,
    hwemul_setcpuspeed,
    hwemul_readclock
};

const int hfptr_max =
sizeof(hfptr) / sizeof(handler_function_pointer);

const int hwemul_hfptr_max =
sizeof(hwemul_hfptr) / sizeof(handler_function_pointer);

/**********************************************************************/

/* EOF debughwi.c */
