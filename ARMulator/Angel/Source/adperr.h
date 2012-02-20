/* -*-C-*-
 *
 * $Revision: 1.2.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/03/25 18:03:40 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Definitions of ADP error codes
 */

#ifndef angel_adperrs_h
#define angel_adperrs_h
/*
 * ADP failure codes start at 256 to distinguish them for debug purposes
 */
enum AdpErrs
{
    adp_ok = 0,
    adp_failed = 256,
    adp_malloc_failure,
    adp_illegal_args,
    adp_device_not_found,
    adp_device_open_failed, /* 260 */
    adp_device_already_open,
    adp_device_not_open,
    adp_bad_channel_id,
    adp_callback_already_registered,
    adp_write_busy,         /* 265 */
    adp_bad_packet,
    adp_seq_high,
    adp_seq_low,
    adp_timeout_on_open,
    adp_abandon_boot_wait,  /* 270 */
    adp_late_startup,
    adp_new_agent_starting,
    adp_resend_failed,
    adp_timeout,                       /* general timeout */
    adp_bootmessage,        /* 275 */  /* boot message at inappropriate time */
        adp_dev_io_err                     /* device I/O error, e.g. framing err on serial */
};

#ifndef __cplusplus
typedef enum AdpErrs AdpErrs;
#endif

#define AdpMess_Failed             "ADP Error - unspecific failure"
#define AdpMess_MallocFailed       "ADP Error - malloc failed"
#define AdpMess_IllegalArgs        "ADP Error - illegal arguments"
#define AdpMess_DeviceNotFound     "ADP Error - invalid device specified"
#define AdpMess_DeviceOpenFailed   "ADP Error - specified device failed to open"
#define AdpMess_DeviceAlreadyOpen  "ADP Error - device already open"
#define AdpMess_DeviceNotOpen      "ADP Error - device not open"
#define AdpMess_BadChannelId       "ADP Error - bad channel Id"
#define AdpMess_CBAlreadyRegd      "ADP Error - callback already registered"
#define AdpMess_WriteBusy          "ADP Error - write busy"
#define AdpMess_BadPacket          "ADP Error - bad packet"
#define AdpMess_SeqHigh            "ADP Error - sequence number too high"
#define AdpMess_SeqLow             "ADP Error - sequence number too low"
#define AdpMess_TimeoutOnOpen      "ADP Error - target did not respond"
#define AdpMess_AbandonBootWait    "abandoned wait for late startup"
#define AdpMess_LateStartup        "Target compiled with LATE_STARTUP set.\n" \
                                   "Waiting for target...\n"                  \
                                   "Press <Ctrl-C> to abort.\n"
#define AdpMessLen_LateStartup    (3*80)
#define AdpMess_NewAgentStarting   \
     "New Debug Agent now loaded - restart may not be automatic.\n"
#define AdpMess_ResendFailed       "ADP Error - Attempt to request message resend failed"
#define AdpMess_Timeout            "ADP Error - Packet Timeout on data link"
#define AdpMess_BootMessage        "ADP Error - Unexpected boot message seen"
#define AdpMess_DevIOErr           "ADP Error - I/O error"

#endif /* ndef angel_adperr_h */

/* EOF adperr.h */
