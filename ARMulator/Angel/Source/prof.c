
/* -*-C-*-
 *
 * $Revision: 1.6.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:12 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * prof.c: Implementation of ADP_Profile subreasons.
 */

#ifdef ICEMAN2
#include <stdlib.h>             /* malloc() and free() */
#endif

#include "angel.h"
#include "adp.h"
#include "channels.h"
#include "debug.h"
#include "debugos.h"
#include "debughwi.h"
#include "logging.h"
#include "msgbuild.h"
#include "support.h"
#include "devconf.h"
#include "prof.h"

AngelProfileState angel_profilestate;

#if PROFILE_SUPPORTED

static void 
free_buffer(p_Buffer * buffer)
{
    angel_ChannelReleaseBuffer(*buffer);
    *buffer = NULL;
}

static int 
profile_supported(p_Buffer * buffer, void *stateptr)
{
    /* Check whether profiling is available. */
    IGNORE(stateptr);
    if (angelOS_ReturnInfo()->infoBitset & ADP_Info_Target_Profiling)
    {
        int debugID, OSinfo1, OSinfo2;

        unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                       &OSinfo2);

        free_buffer(buffer);

        return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                       debugID, OSinfo1, OSinfo2, ADP_Profile_Supported,
                       RDIError_NoError);
    }
    else
        return -1;
}

static int 
profile_stop(p_Buffer * buffer, void *stateptr)
{
    int debugID, OSinfo1, OSinfo2;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    free_buffer(buffer);

    angel_profilestate.enabled = NO;
    Angel_ProfileTimerStop();

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Profile_Stop,
                   RDIError_NoError);
}

static int 
profile_start(p_Buffer * buffer, void *stateptr)
{
    int debugID, OSinfo1, OSinfo2, subreason;
    int interval;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &interval);

    free_buffer(buffer);

    Angel_ProfileTimerStart(interval);
    angel_profilestate.enabled = YES;

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Profile_Start,
                   RDIError_NoError);
}

static int 
profile_writemap(p_Buffer * buffer, void *stateptr)
{
    int debugID, OSinfo1, OSinfo2;
    word subreason, totalcount, wordoffset, wordcount;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &totalcount, &wordcount, &wordoffset);

    if (angel_profilestate.numentries != totalcount)
    {
        unsigned *counts = NULL;
        int mapsize = sizeof(int) * (totalcount + 1);
        int i;

#if defined(ICEMAN2)
        /* use malloc - this is only possible for ICEman, since ICEman IS the
         * application, as well as the thing being profiled !!! */
        if (angel_profilestate.counts != NULL)
            free(angel_profilestate.counts);
        counts = (unsigned *)malloc(2 * mapsize);
#elif defined(Angel_ProfileAreaSize) && (Angel_ProfileAreaSize > 0)
        /* If there's a separate area configured for profiling to use, use it */
        if (2 * mapsize <= Angel_ProfileAreaSize)
        {
#if Angel_ProfileAreaIsRelativeToTopOfMemory
            counts = (unsigned *)(Angel_TopOfMemory +
                                  Angel_ProfileAreaBaseOffsetFromTopOfMemory);
#else
            counts = (unsigned *)Angel_FixedProfileAreaBase;
#endif
        }
        else
            counts = NULL;
#else
        /* Otherwise, grab a block of heap, from the top of the heap */
        if (angel_heapstackdesc.heapbase == 0 ||
            ((2 * mapsize)
             < (angel_heapstackdesc.heaplimit - angel_heapstackdesc.heapbase)))
        {
            counts = (unsigned *)(angel_heapstackdesc.heaplimit - (2 * mapsize));
            angel_heapstackdesc.heaplimit = counts;
        }
        else
            counts = NULL;
#endif
        if (counts == NULL)
        {
            free_buffer(buffer);
            return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                           debugID, OSinfo1, OSinfo2, ADP_Profile_WriteMap,
                           RDIError_OutOfStore);
        }
        angel_profilestate.counts = counts;
        angel_profilestate.map = (unsigned *)((char *)counts + mapsize);
        for (i = totalcount; i >= 0; i--)
            counts[i] = 0;
        angel_profilestate.numentries = totalcount;
    }

    __rt_memcpy(&angel_profilestate.map[wordoffset],
                BUFFERDATA(*buffer) + ADP_ProfileWriteHeaderSize,
                wordcount * sizeof(int));

    free_buffer(buffer);

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Profile_WriteMap,
                   RDIError_NoError);
}

static int 
profile_readmap(p_Buffer * buffer, void *stateptr)
{
    /* Reads a number of words from the profiling map. */
    p_Buffer rbuff;
    int debugID, OSinfo1, OSinfo2;
    word subreason, wordoffset, wordcount;

    IGNORE(stateptr);

    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2, &subreason, &wordoffset, &wordcount);

    free_buffer(buffer);

    rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (rbuff != NULL)
    {
        int count = msgbuild(BUFFERDATA(rbuff), "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                             debugID, OSinfo1, OSinfo2, ADP_Profile_ReadMap,
                             RDIError_NoError);

        __rt_memcpy(BUFFERDATA(rbuff) + ADP_ProfileReadHeaderSize,
                    &angel_profilestate.counts[wordoffset],
                    wordcount * sizeof(int));

        count += wordcount * sizeof(int);

        angel_ChannelSend(CH_DEFAULT_DEV, CI_HADP, rbuff, count);

    }
    else
        LogError(LOG_PROF, ( "Couldn't allocate buffer in 'profile_readmap'.\n"));

    return 0;
}

static int 
profile_clearcounts(p_Buffer * buffer, void *stateptr)
{
    int debugID, OSinfo1, OSinfo2;

    IGNORE(stateptr);
    unpack_message(BUFFERDATA(*buffer) + 4, "%w%w%w", &debugID, &OSinfo1,
                   &OSinfo2);

    free_buffer(buffer);

    {
        int i;
        unsigned *map = angel_profilestate.counts;

        for (i = angel_profilestate.numentries; i >= 0; i--)
            map[i] = 0;
    }

    return msgsend(CI_HADP, "%w%w%w%w%w%w", (ADP_Profile | TtoH),
                   debugID, OSinfo1, OSinfo2, ADP_Profile_ClearCounts,
                   RDIError_NoError);
}

#else /* PROFILE_SUPPORTED */

static int 
profile_supported(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
}

static int 
profile_stop(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
}

static int 
profile_start(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
}

static int 
profile_writemap(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
}

static int 
profile_readmap(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
}

static int 
profile_clearcounts(p_Buffer * buffer, void *stateptr)
{
    IGNORE(buffer);
    IGNORE(stateptr);
    return -1;
}

#endif /* PROFILE_SUPPORTED */

const handler_function_pointer profile_hfptr[] =
{
    profile_supported,
    profile_stop,
    profile_start,
    profile_writemap,
    profile_readmap,
    profile_clearcounts
};

const int profile_hfptr_max =
sizeof(profile_hfptr) / sizeof(handler_function_pointer);

/**********************************************************************/

/* EOF prof.c */
