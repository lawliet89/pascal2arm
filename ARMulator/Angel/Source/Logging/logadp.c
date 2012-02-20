/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/08/22 15:42:32 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via ADP to host
 */

#include "channels.h"
#include "logadp.h"
#include "debug.h"

#define LOGADP_PREFIX_STR "TLOG: "
#define LOGADP_PREFIX_LEN 6        /* set by hand, to avoid strlen */

static p_Buffer logadp_buf = NULL;
char           *logadp_pos;
char           *logadp_end;

extern bool logadp_inUSRmode( void );

#pragma no_check_stack

bool logadp_PreWarn(WarnLevel level)
{
    IGNORE(level);

    if ( ! boot_completed )
       return FALSE;

    if ( logadp_buf != NULL )
       return FALSE;            /* busy */

    if ( ! logadp_inUSRmode() )
       return FALSE;

    logadp_buf = angel_ChannelAllocBuffer( Angel_ChanBuffSize );
    if (logadp_buf != NULL)
    {
        char *prefix = LOGADP_PREFIX_STR;

        logadp_pos = (char *)logadp_buf;
        logadp_end = (char *)(logadp_buf + Angel_ChanBuffSize - 1);

        while ( (*logadp_pos++ = *prefix++) != '\0' )
           /* classic strcpy, no body required */ ;

        return TRUE;
    }
    else
       return FALSE;
}

void logadp_PostWarn(unsigned int len)
{
    ChanError err;

    len += LOGADP_PREFIX_LEN;
    *logadp_pos = '\0';
    ++len;

    err = angel_ChannelSendThenRead( CH_DEFAULT_DEV, CI_TLOG,
                                     &logadp_buf, &len);
    if ( err == CE_OKAY )
       angel_ChannelReleaseBuffer( logadp_buf );

    logadp_buf = NULL;
}

#pragma check_stack

/* EOF logadp.c */
