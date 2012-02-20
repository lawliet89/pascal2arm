/* -*-C-*-
 *
 * $Revision: 1.8.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:49:55 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * ctrl.c: Implementation of ADP_Control subreasons.
 */

#include "angel.h"
#include "adp.h"
#include "debug.h"
#include "debughwi.h"
#include "debugos.h"
#include "logging.h"
#include "msgbuild.h"
#include "sys.h"


static int 
ctrl_nop(p_Buffer * buffer, void *stateptr)
{
    /* Return an RDIError_NoError to indicate ctrl calls are available. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;

    IGNORE(stateptr);

    LogInfo(LOG_CTRL, ( "ctrl_nop()\n"));
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Control | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Ctrl_NOP, status);
}

static int 
ctrl_vectorcatch(p_Buffer * buffer, void *stateptr)
{
    /* Specify which hardware exceptions should be reported to the debugger. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word status = RDIError_NoError;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &debug_VectorCatch);

    LogInfo(LOG_CTRL, ( "ctrl_vectorcatch(vectorcatch = 0x%x)\n", debug_VectorCatch));

    status = angelOS_VectorCatch(OSinfo1, OSinfo2, debug_VectorCatch);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Control | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Ctrl_VectorCatch, status);
}

static int 
ctrl_pointstatus_watch(p_Buffer * buffer, void *stateptr)
{
    /* Return information about a watchpoint given a handle. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word handle;
    word status, hwresource, type;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &handle);
    LogInfo(LOG_CTRL, ( "ctrl_pointstatus_watch(handle = 0x%x)\n", handle));

    status = angelOS_WatchPointStatus(OSinfo1, OSinfo2,
                                      handle, &hwresource, &type);
    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_PointStatus_Watch, status, hwresource, type);
}

static int 
ctrl_pointstatus_break(p_Buffer * buffer, void *stateptr)
{
    /* Return information about a breakpoint given a handle. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word status, hwresource, type;
    word handle;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &handle);
    LogInfo(LOG_CTRL, ( "ctrl_pointstatus_break(handle = 0x%x)\n", handle));

    status = angelOS_BreakPointStatus(OSinfo1, OSinfo2,
                                      handle, &hwresource, &type);
    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_PointStatus_Break, status, hwresource, type);
}

static int 
ctrl_semihosting_setstate(p_Buffer * buffer, void *stateptr)
{
    /* Set whether or not semi-hosting is enabled. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_state, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &semihosting_state);

    LogInfo(LOG_CTRL, ("ctrl_semihosting_setstate(state = %d)\n", semihosting_state));

    status = angelOS_SemiHosting_SetState(OSinfo1, OSinfo2, semihosting_state);

    return msgsend(CI_HADP, "%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_SetState, status);
}

static int 
ctrl_semihosting_getstate(p_Buffer * buffer, void *stateptr)
{
    /* Reads whether or not semi-hosting is enabled. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_state, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason);

    status = angelOS_SemiHosting_GetState(OSinfo1, OSinfo2,
                                          &semihosting_state);

    LogInfo(LOG_CTRL, ( "ctrl_semihosting_getstate -> %d\n", semihosting_state));
    
    return msgsend(CI_HADP, "%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_GetState, status, semihosting_state);
}

static int 
ctrl_semihosting_setvector(p_Buffer * buffer, void *stateptr)
{
    /* Set the semi-hosting vector. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_vector, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &semihosting_vector);
    LogInfo(LOG_CTRL, ( "ctrl_semihosting_setvector(vec = 0x%x)\n", semihosting_vector));

    status = angelOS_SemiHosting_SetVector(OSinfo1, OSinfo2,
                                           semihosting_vector);

    return msgsend(CI_HADP, "%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_SetVector, status);
}

static int 
ctrl_semihosting_getvector(p_Buffer * buffer, void *stateptr)
{
    /* Reads the value of the semi-hosting vector. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_vector, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason);

    status = angelOS_SemiHosting_GetVector(OSinfo1, OSinfo2,
                                           &semihosting_vector);

    LogInfo(LOG_CTRL, ( "ctrl_semihosting_getvector -> %08x\n", semihosting_vector));

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_GetVector, status, semihosting_vector);
}

static int 
ctrl_log(p_Buffer * buffer, void *stateptr)
{
    /* Read logging level. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    word logsetting;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);
    LogInfo(LOG_CTRL, ( "ctrl_log -> 0x%x\n", logsetting));

    status = angelOS_Ctrl_Log(OSinfo1, OSinfo2, &logsetting);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_Log, status, logsetting);
}

static int 
ctrl_setlog(p_Buffer * buffer, void *stateptr)
{
    /* Set logging level. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word logsetting;
    word status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &logsetting);
    LogInfo(LOG_CTRL, ( "ctrl_setlog(logsetting = 0x%x)\n", logsetting));

    /* Range check logsetting. */
    if ((logsetting & 0x7) == logsetting)
        status = angelOS_Ctrl_SetLog(OSinfo1, OSinfo2, logsetting);
    else
        status = RDIError_Error;

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Control | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Ctrl_SetLog, status);
}


static int 
ctrl_semihosting_setarmswi(p_Buffer * buffer, void *stateptr)
{
#ifdef ICEMAN2

    /* Set the semi-hosting ARM SWI. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_armswi, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &semihosting_armswi);

    LogInfo(LOG_CTRL, ( "ctrl_semihosting_setarmswi(swi 0x%x)\n", semihosting_armswi));

    status = angelOS_SemiHosting_SetARMSWI(OSinfo1, OSinfo2,
                                           semihosting_armswi);

    return msgsend(CI_HADP, "%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_SetARMSWI, status);

#else
    IGNORE(buffer);
    IGNORE(stateptr);
    LogInfo(LOG_CTRL, ( "ctrl_semihosting_setarmswi -> unsupported\n"));
    return -1;
#endif
}

static int 
ctrl_semihosting_getarmswi(p_Buffer * buffer, void *stateptr)
{
#ifdef ICEMAN2

    /* Reads the value of the semi-hosting vector. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_armswi, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason);

    status = angelOS_SemiHosting_GetARMSWI(OSinfo1, OSinfo2,
                                           &semihosting_armswi);

    LogInfo(LOG_CTRL, ( "ctrl_semihosting_getarmswi -> %08x\n", semihosting_armswi));

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_GetARMSWI, status, semihosting_armswi);

#else
    IGNORE(buffer);
    IGNORE(stateptr);
    LogInfo(LOG_CTRL, ( "ctrl_semihosting_getarmswi - unsupported\n"));
    return -1;
#endif
}

static int 
ctrl_semihosting_setthumbswi(p_Buffer * buffer, void *stateptr)
{
#ifdef ICEMAN2

    /* Set the semi-hosting ARM SWI. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_thumbswi, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &semihosting_thumbswi);
    LogInfo(LOG_CTRL, ( "ctrl_semihosting_setthumbswi(swi = 0x%x)\n",
                          semihosting_thumbswi));

    status = angelOS_SemiHosting_SetThumbSWI(OSinfo1, OSinfo2,
                                             semihosting_thumbswi);

    return msgsend(CI_HADP, "%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_SetThumbSWI, status);

#else
    IGNORE(buffer);
    IGNORE(stateptr);
    LogInfo(LOG_CTRL, ( "ctrl_semihosting_setthumbswi -> unsupported\n"));
    return -1;
#endif
}

static int 
ctrl_semihosting_getthumbswi(p_Buffer * buffer, void *stateptr)
{
#ifdef ICEMAN2

    /* Reads the value of the semi-hosting vector. */
    int debugID, OSinfo1, OSinfo2, subreason;
    word semihosting_thumbswi, status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason);

    status = angelOS_SemiHosting_GetThumbSWI(OSinfo1, OSinfo2,
                                             &semihosting_thumbswi);

    LogInfo(LOG_CTRL, ("ctrl_semihosting_getthumbswi -> %08x\n",
                         semihosting_thumbswi));

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SemiHosting_GetThumbSWI,
                   status, semihosting_thumbswi);

#else
    IGNORE(buffer);
    IGNORE(stateptr);
    LogInfo(LOG_CTRL, ( "ctrl_semihosting_getthumbswi - unsupported\n"));
    return -1;
#endif
}

static int 
ctrl_download_supported(p_Buffer * buffer, void *stateptr)
{
    int debugID, OSinfo1, OSinfo2;

    /* For Angel return CantLoadConfig - we can't do that
     * for EmbeddedICE (ICEman) say we can
     * for EICEADP - the ADP over JTAG software say we can't
     */

#if defined(ICEMAN2) && !defined(JTAG_ADP_SUPPORTED)
    word status = RDIError_NoError;

#else
    word status = RDIError_CantLoadConfig;

#endif

    IGNORE(stateptr);

    LogInfo(LOG_CTRL, ( "ctrl_download_supported -> %d\n", status));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Control | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Ctrl_Download_Supported,
                   status);
}

static word loadagent_address = (word) - 1;
static word loadagent_size;
static word loadagent_sofar;

static int 
ctrl_download_data(p_Buffer * buffer, void *stateptr)
{
    /* Decode an addconfig message */
    int debugID, OSinfo1, OSinfo2, reason, subreason, count, err;
    word nbytes;
    word status;

    IGNORE(stateptr);

    count = unpack_message(BUFFERDATA(*buffer), "%w%w%w%w%w%w", &reason,
                           &debugID, &OSinfo1, &OSinfo2, &subreason, &nbytes);
    LogInfo(LOG_CTRL, ( "ctrl_download_data(nbytes = %d)\n", nbytes));

    status = angelOS_LoadConfigData(OSinfo1, OSinfo2,
                                    nbytes, BUFFERDATA(*buffer) + count);

    if (status == RDIError_NoError && loadagent_address != -1)
        loadagent_sofar += nbytes;

    /* We make sure we release the buffer here as it may be the long one */
    angel_ChannelReleaseBuffer(*buffer);
    *buffer = NULL;

    err = msgsend(CI_HADP, "%w%w%w%w%w%w",
                  (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                  ADP_Ctrl_Download_Data, status);

    return err;
}

static int 
ctrl_download_agent(p_Buffer * buffer, void *stateptr)
{
    /* Decode an loadagent message */
    int debugID, OSinfo1, OSinfo2, subreason;
    word loadaddress, nbytes;
    word status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &loadaddress, &nbytes);
    
    LogInfo(LOG_CTRL, ( "ctrl_download_agent(loadadr = 0x%x, nbytes = %d)\n",
                          loadaddress, nbytes));

    status = angelOS_LoadAgent(OSinfo1, OSinfo2, loadaddress, nbytes);

    if (status == RDIError_NoError)
    {
        loadagent_address = loadaddress;
        loadagent_size = nbytes;
        loadagent_sofar = 0;
    }

    return msgsend(CI_HADP, "%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_Download_Agent, status);
}


static int 
ctrl_start_agent(p_Buffer * buffer, void *stateptr)
{
    /* Decode an loadagent message */
    int debugID, OSinfo1, OSinfo2, subreason, err;
    word startaddress;
    word status;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &startaddress);
    LogInfo(LOG_CTRL, ( "ctrl_start_agent(startaddress = %x)\n", startaddress));

    if (loadagent_sofar == loadagent_size
        && startaddress >= loadagent_address
        && startaddress < (loadagent_address + loadagent_size))
        status = RDIError_NoError;
    else
        status = RDIError_BadConfigData;

    err = msgsend(CI_HADP, "%w%w%w%w%w%w",
                  (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                  ADP_Ctrl_Start_Agent, status);

    if (err == RDIError_NoError && status == RDIError_NoError)
        angelOS_ExecuteNewAgent(startaddress);

    return err;
}


static int 
ctrl_settopmem(p_Buffer * buffer, void *stateptr)
{
#ifdef ICEMAN2

    /* Set the top of memory we report on a HEAPINFO SWI */
    int debugID, OSinfo1, OSinfo2, subreason;
    word topmem;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &topmem);
    LogInfo(LOG_CTRL, ( "ctrl_settopmem(topmem = 0x%x)\n", topmem));

    angel_SetTopMem((unsigned)topmem);

    return msgsend(CI_HADP, "%w%w%w%w%w%w",
                   (ADP_Control | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Ctrl_SetTopMem, RDIError_NoError);

#else
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
#endif
}


const handler_function_pointer ctrl_hfptr[] =
{
    ctrl_nop,
    ctrl_vectorcatch,
    ctrl_pointstatus_watch,
    ctrl_pointstatus_break,
    ctrl_semihosting_setstate,
    ctrl_semihosting_getstate,
    ctrl_semihosting_setvector,
    ctrl_semihosting_getvector,
    ctrl_log,
    ctrl_setlog,
    ctrl_semihosting_setarmswi,
    ctrl_semihosting_getarmswi,
    ctrl_semihosting_setthumbswi,
    ctrl_semihosting_getthumbswi,
    ctrl_download_supported,
    ctrl_download_data,
    ctrl_download_agent,
    ctrl_start_agent,
    ctrl_settopmem
};

const int ctrl_hfptr_max =
sizeof(ctrl_hfptr) / sizeof(handler_function_pointer);

/* end of file */
