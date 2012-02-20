/* -*-C-*-
 *
 * $Revision: 1.9.2.3 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:03 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * info.c: Implementation of ADP_Info subreasons
 */

#include "angel.h"
#include "adp.h"
#include "channels.h"
#include "debughwi.h"
#include "debugos.h"
#include "endian.h"
#include "logging.h"
#include "msgbuild.h"
#include "devconf.h"
#include "dbg_cp.h"

/* Limit on number of copro description sections per message */
#define DCP_HDR_LEN        (5*4)  /* five words of overhead */
#define DCP_FIXED_OVERHEAD 2    /* cpno + 0xFF */
#define DCP_VAR_MAX        (ADP_BUFFER_MIN_DATASIZE - DCP_HDR_LEN         \
                                                    - DCP_FIXED_OVERHEAD)
#define DCP_VAR_LEN        8
#define MAX_CPRS           (DCP_VAR_MAX / DCP_VAR_LEN)


static void
info_free_buffer(p_Buffer * buffer)
{
    angel_ChannelReleaseBuffer(*buffer);
    *buffer = NULL;
}


static int
info_nop(p_Buffer * buffer, void *stateptr)
{
    /*
     * Return an RDIError_NoError to indicate info calls are available.
     */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_nop.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_NOP, status);
}

static int
info_target(p_Buffer * buffer, void *stateptr)
{
    /* Return target specific information. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    struct AngelOSInfo *info = angelOS_ReturnInfo();

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_target.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_Target, status,
                   info->infoBitset, info->infoModel);
}

static int
info_points(p_Buffer * buffer, void *stateptr)
{
    /* Return target specific break/watchpoint information. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    word breakinfo;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_points.\n"));

    breakinfo = angelOS_ReturnInfo()->infoPoints;

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w", (ADP_Info | TtoH), debugID,
                   OSinfo1, OSinfo2, ADP_Info_Points, status, breakinfo);
}

static int
info_step(p_Buffer * buffer, void *stateptr)
{
    /* Return target specific stepping information. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    word stepinfo;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_step.\n"));

    stepinfo = angelOS_ReturnInfo()->infoStep;

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_Step, status, stepinfo);
}

static int
info_mmu(p_Buffer * buffer, void *stateptr)
{
    /* Return target specific information about memory management system. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    word meminfo;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_mmu.\n"));

    status = angelOS_MemInfo(&meminfo);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_MMU, status, meminfo);
}

static int
info_semihosting(p_Buffer * buffer, void *stateptr)
{
    /* Check whether semi-hosting calls are available. */
    int debugID, OSinfo1, OSinfo2;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_semihosting.\n"));

    /* TODO: Change this as required. */
    status = RDIError_NoError;

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_SemiHosting, status);
}

static int
info_copro(p_Buffer * buffer, void *stateptr)
{
    /* Check whether co-processor calls are available. */
    int debugID, OSinfo1, OSinfo2;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_copro.\n"));

    status = RDIError_NoError;

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_CoPro, status);
}

static int
info_cycles(p_Buffer * buffer, void *stateptr)
{
    /*
     * Return the number of instructions and cycles executed since
     * the target was initialised.
     */
    int debugID, OSinfo1, OSinfo2;
    word status;
    word ninstr, Scycles, Ncycles, Icycles, Ccycles, Fcycles;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_cycles.\n"));

    /* TODO: Change these as required. */
    status = RDIError_Error;
    ninstr = 0;
    Scycles = 0;
    Ncycles = 0;
    Icycles = 0;
    Ccycles = 0;
    Fcycles = 0;

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w%w%w%w%w",
                   (ADP_Info | TtoH), debugID, OSinfo1, OSinfo2,
                   ADP_Info_Cycles, status, ninstr, Scycles, Ncycles,
                   Icycles, Ccycles, Fcycles);
}

static int
info_describecopro(p_Buffer * buffer, void *stateptr)
{
    /* Describe the registers of a coprocessor. */
    int debugID, OSinfo1, OSinfo2, subreason;
    byte *bp = NULL;            /* pointer into buffer */
    byte cpno;
    byte cpd_buffer[Dbg_CoProDesc_Size(MAX_CPRS)];
    struct Dbg_CoProDesc *cpd = (struct Dbg_CoProDesc *)cpd_buffer;
    int i, count;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_describecopro.\n"));

    count = unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%b",
                           &debugID, &OSinfo1, &OSinfo2, &subreason, &cpno);

    bp = BUFFERDATA(*buffer) + 4 + count;

    for (i = 0; *bp != 0xFF && i < MAX_CPRS; ++i)
    {
        cpd->regdesc[i].rmin = *bp++;
        cpd->regdesc[i].rmax = *bp++;
        cpd->regdesc[i].nbytes = *bp++;
        cpd->regdesc[i].access = *bp++;
        cpd->regdesc[i].accessinst.cprt.read_b0 = *bp++;
        cpd->regdesc[i].accessinst.cprt.read_b1 = *bp++;
        cpd->regdesc[i].accessinst.cprt.write_b0 = *bp++;
        cpd->regdesc[i].accessinst.cprt.write_b1 = *bp++;
    }                           /* endfor */
    cpd->entries = i;

    if (*bp == 0xFF)
        status = angelOS_DescribeCoPro(cpno, cpd);
    else
        status = RDIError_Error;

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, subreason, status);
}

static int
info_requestcoprodesc(p_Buffer * buffer, void *stateptr)
{
    /* Return description of the registers of a co-processor. */
    word debugID, OSinfo1, OSinfo2, subreason;
    byte cpno;
    word status = RDIError_Error;
    byte cpd_buffer[Dbg_CoProDesc_Size(MAX_CPRS)];
    struct Dbg_CoProDesc *cpd = (struct Dbg_CoProDesc *)cpd_buffer;
    int count, i;
    p_Buffer rbuff;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_requestcoprodesc.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%b", &debugID,
                   &OSinfo1, &OSinfo2, &subreason, &cpno);

    rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (rbuff != NULL)
    {
        count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w%w", (ADP_Info | TtoH),
                         debugID, OSinfo1, OSinfo2, subreason, status);

        cpd->entries = MAX_CPRS;
        status = angelOS_RequestCoProDesc(cpno, cpd);

        /* write status field */
        PUT32LE(BUFFERDATA(rbuff) + 20, status);

        if (status == RDIError_NoError || status == RDIError_BufferFull)
        {
            for (i = 0; i < cpd->entries; ++i)
                count += msgbuild(BUFFERDATA(rbuff) + count, "%b%b%b%b",
                                  (int)cpd->regdesc[i].rmin,
                                  (int)cpd->regdesc[i].rmax,
                                  (int)cpd->regdesc[i].nbytes,
                                  (int)cpd->regdesc[i].access
                    );

            BUFFERDATA(rbuff)[count++] = 0xFF;

        }

        info_free_buffer(buffer);

        angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);

    }
    else
    {                           /* some kind of debug output to indicate no buffer space */
    }

    return 0;
}

static int
info_angelbuffersize(p_Buffer * buffer, void *stateptr)
{
    /* Return size of angel buffers. */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    word longsize = Angel_ChanLongSize;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_angelbuffersize.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

#if ETHERNET_SUPPORTED
    /* If we are using ethernet at the moment then make the largest buffer
     * supported one which will fit into a ethernet packet
     */
    {
        DeviceID dev;

        angel_ChannelReadActiveDevice(&dev);
        if (dev == DI_ETHERNET)
        {
            longsize = ETHERNET_BUFFERLONGSIZE;
        }
    }
#endif

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_AngelBufferSize,
                   status, Angel_ChanBuffSize, longsize);
}


static int
info_changeableshswi(p_Buffer * buffer, void *stateptr)
{
    /* Check whether it is possible to change semihosting SWIs */
    int debugID, OSinfo1, OSinfo2;
    word status;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_changeableshswi.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);
    info_free_buffer(buffer);

    status = angelOS_CanChangeSHSWI();

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                 debugID, OSinfo1, OSinfo2, ADP_Info_ChangeableSHSWI, status);
}


static int
info_cantargetexecute(p_Buffer * buffer, void *stateptr)
{
    /* Say whether target is ready to execute or not (on debugger *
     * startup.  We are ready to execute if the Late Debugger Startup
     * support SWI was called - ie. Late debugger startup is on.
     * Note that Iceman2 is a special case - it is a specila sort of
     * Late debugger startup application, but execution should not
     * be allowed immediately.
     */
    int debugID, OSinfo1, OSinfo2;

#if LATE_STARTUP > 0 && !defined(ICEMAN2)
    word status = RDIError_NoError;

#else
    word status = RDIError_Error;

#endif

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_cantargetexecute.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_CanTargetExecute,
                   status);
}

static int
info_agentendianess(p_Buffer * buffer, void *stateptr)
{
    /* Say whether the debug agent (Angel) is big or little endian.
     */
    int debugID, OSinfo1, OSinfo2;

#ifdef BIG_ENDIAN
    word status = RDIError_BigEndian;

#else
    word status = RDIError_LittleEndian;

#endif

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_agentendianess.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    info_free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_AgentEndianess,
                   status);
}

static int
info_canackheartbeat(p_Buffer * buffer, void *stateptr)
{
    /* Check whether it is possible to acknowledge heartbeats */
    int debugID, OSinfo1, OSinfo2;
    word status = RDIError_NoError;
    word canack;

    IGNORE(stateptr);

    LogInfo(LOG_INFO, ( "DEBUG: Entered function info_canackheartbeat.\n"));

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);
    info_free_buffer(buffer);

    /* can we ack h/b's? */
    canack = angel_CanAckHeartbeat();

    return msgsend(CI_HADP, "%w%w%w%w%w%w%w", (ADP_Info | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Info_CanAckHeartbeat,
                   status, canack);
}

/* 
 * vector tables */
const handler_function_pointer info_hfptr[] =
{
    info_nop,
    info_target,
    info_points,
    info_step,
    info_mmu,
    info_semihosting,
    info_copro,
    info_cycles,
    info_describecopro,
    info_requestcoprodesc,
    info_angelbuffersize,
    info_changeableshswi,
    info_cantargetexecute,
    info_agentendianess,
    info_canackheartbeat
};

const int info_hfptr_max =
sizeof(info_hfptr) / sizeof(handler_function_pointer);

/* end of file */
