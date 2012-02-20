/* -*-C-*-
 *
 * $Revision: 1.13.6.4 $
 *   $Author: rivimey $
 *     $Date: 1998/02/11 13:15:01 $
 *
 * Copyright (c) 1995, 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Boot agent support for Angel. Deals with
 *            the bootstrap related messages.
 */

#include "angel.h"              /* Generic Angel definitions. */
#include "adp.h"
#include "channels.h"
#include "debugos.h"
#include "devclnt.h"            /* device control */
#include "endian.h"             /* Target independant endian access. */
#include "params.h"
#include "devdriv.h"
#include "logging.h"
#include "msgbuild.h"
#include "support.h"
#include "serlock.h"
#include "banner.h"             /* ANGEL_BANNER */

/* called from assembler, so not in a .h file */
void angel_BootInit(void);
void angel_LateBootInit(unsigned int swi_code, angel_LateStartType type);

/* boot_completed only TRUE once fully booted and talking to debugger */
extern int boot_completed;

/* cold reboot entry point */
extern void __rom(void);

typedef enum BootState
{
    BS_STARTUP,                 /* just started OR rebooted */
    BS_AVAILABLE,               /* ok for debugger to connect */
    BS_RESETTING,               /* reset received, awaiting TBOOT ack */
    BS_CONNECTED                /* connected to debugger */
}
BootState;

/*
 * waiting for an ADP_Reset message; needed so channels code can take proper action
 * if a bad packet or heartbeat arrives.
 */
int booted_flow_controlled = 0;

#if (LATE_STARTUP > 0)
static BootState boot_state = BS_STARTUP;

#define REBOOT_ACK AB_LATE_ACK
#else
static BootState boot_state = BS_AVAILABLE;

#define REBOOT_ACK AB_NORMAL_ACK
#endif

static const char angel_banner[] = ANGEL_BANNER;
static const unsigned int angel_banner_size = sizeof(angel_banner);

static char angel_ratemsg[24];

static void
send_booted_message()
{

    /* Send the ADP_Booted message: */
    /* NOTE: Boot-strapping is a special case, distinct from the rest of
     * the communications in that we may *NOT* get a reply to the
     * ADP_Booted message. If we transmit the message to a host that is
     * not yet ready, we will not get a reply. Sometime later (before
     * any other messages are transmitted by the host however) we should
     * receive an ADP_Reboot or ADP_Reset message from the host.
     *
     * NOTE: We do not block, continually re-sending the ADP_Booted
     * message, until we get an ACK from the host. If we did it would
     * mean that application based Angel systems would not start
     * executing until a host is attached. This does not tie cleanly
     * with the Angel model, of letting the application execute, with
     * the host being attached and removed as required.
     */
    DeviceID device;
    unsigned int count, ratelen;
    p_Buffer buffer;

    LogInfo(LOG_BOOT, ( "Enter: send_booted_message\n"));

    buffer = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
    if (buffer == NULL)
        return;

    ratelen =  __rt_strlen(angel_ratemsg);
    count = msgbuild(BUFFERDATA(buffer),
                     "%w%w%w%w%w%w%w%w%w%w%w%w",
                     (ADP_Booted | TtoH),
                     ADP_HandleUnknown,
                     ADP_HandleUnknown,
                     ADP_HandleUnknown,
                     Angel_ChanBuffSize,  /* Angel message buffer size. */
                     Angel_ChanLongSize,  /* Angel long buffer size */
                     ANGELVSN,  /* Angel version number. */
                     ADPVSN,    /* ADP version number. */
#if defined(THUMB_SUPPORT) && THUMB_SUPPORT!=0
                     ADP_ARM_ARCH_THUMB,  /* ARM architecture info. */
#else
                     0,         /* ARM architecture info. */
#endif
                     angel_Endianess[0],  /* CPU feature set. */
                     angel_Endianess[1],  /* Target hardware status. */
                     angel_banner_size + ratelen /* No. of bytes in banner. */
        );

    __rt_memcpy(BUFFERDATA(buffer) + count, angel_banner, angel_banner_size);
    count += angel_banner_size - 1;  /* banner size includes trailing null */
    __rt_memcpy(BUFFERDATA(buffer) + count, angel_ratemsg, ratelen);
    count += ratelen;                /* rate len doesn't include null */

    /* ensure there is a null in there somewhere... */
    (BUFFERDATA(buffer) + count)[0] = 0;
    count++;

    if (count >= Angel_ChanBuffSize)
    {
        LogWarning(LOG_BOOT, ("Boot messgage too long (%d >= %d)\n",
                              count, Angel_ChanBuffSize));
        count = Angel_ChanBuffSize;
        msgbuild(BUFFERDATA(buffer) + (11 * 4), "%w", count - (12*4));
    }
    
    angel_ChannelReadActiveDevice(&device);
    booted_flow_controlled = 0;
    if (boot_state == BS_RESETTING)
    {
        /*
         * we must only accept one packet between now and the processing of the
         * boot ack -- the boot ack itself. If this is not enforced, there is a
         * race between the recieve code asking for a buffer for the next packet
         * and the reinitialise code which is busy clearing them.
         *
         * Sadly this has a horrible implication that we can't recover from a bad
         * boot ack packet or a heartbeat arriving at just the wrong time without
         * changes to the channels code... hence the 'booted_flow_controlled'
         * variable.
         *
         * The -1 in the call below says 'only let one packet through'.
         */
        LogWarning(LOG_BOOT, ( "Restricting packet flow\n"));
        booted_flow_controlled = 1;
        angel_DeviceControl(device, DC_RX_PACKET_FLOW, (void*)-1);
    }
    
    /*
     * send the boot out and wait for a reply.
     */
    angel_ChannelSend(device, CI_TBOOT, buffer, count);

    LogInfo(LOG_BOOT, ( "Leave: send_booted_message\n"));

    return;
}


static void
param_negotiate(DeviceID devid, p_Buffer buffer)
{
    unsigned int option[AP_NUM_PARAMS][AP_MAX_OPTIONS];
    ParameterList req_list[AP_NUM_PARAMS];
    ParameterOptions request;
    unsigned int i;

    LogInfo(LOG_BOOT, ( "Enter: param_negotiate\n"));

    /* set up empty request */
    request.num_param_lists = AP_NUM_PARAMS;
    request.param_list = req_list;
    for (i = 0; i < AP_NUM_PARAMS; ++i)
    {
        request.param_list[i].num_options = AP_MAX_OPTIONS;
        request.param_list[i].option = option[i];
    }

    if (Angel_ReadParamOptionsMessage(buffer, &request))
    {
        const ParameterConfig *config;
        unsigned int count, speed;
        p_Buffer rbuff;

        rbuff = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
        if (rbuff == NULL)
        {
            LogError(LOG_BOOT, ( "param_negotiate: no buffer, exit\n"));
            return;
        }

        count = msgbuild(rbuff, "%w%w%w%w",
                         (ADP_ParamNegotiate | TtoH), 0,
                         ADP_HandleUnknown, ADP_HandleUnknown);

        config = Angel_MatchParams(&request, &angel_Device[devid]->options);
        if (config != NULL)
        {
            LogInfo(LOG_BOOT, ( "param_negotiate: found param match\n"));

            PUT32LE(rbuff + count, RDIError_NoError);
            count += sizeof(word);
            count += Angel_BuildParamConfigMessage(rbuff + count, config);
            angel_ChannelSend(devid, CI_HBOOT, rbuff, count);

            LogInfo(LOG_BOOT, ( "param_negotiate: sent negotiate ack, setting new params\n"));

            /* set the new config */
            angel_DeviceControl(devid, DC_SET_PARAMS, (void *)config);
            
            if (Angel_FindParam(AP_BAUD_RATE, config, &speed))
            {
                int i;
                __rt_strcpy(angel_ratemsg, "Serial Rate:      0\n");
                for(i = 18; i > 12 && speed != 0; i--)
                {
                    angel_ratemsg[i] = (speed % 10) + '0';
                    speed /= 10;
                }
            }
            else
                angel_ratemsg[0] = '\0';
        }
        else
        {
            LogWarning(LOG_BOOT, ( "param_negotiate: no param match\n"));

            PUT32LE(rbuff + count, RDIError_Error);
            count += sizeof(word);
            angel_ChannelSend(devid, CI_HBOOT, rbuff, count);
        }
    }
    else
        LogWarning(LOG_BOOT, ( "param_negotiate: reading param options\n"));
    
    LogInfo(LOG_BOOT, ( "param_negotiate: done.\n"));
}

static void
boot_handler(DeviceID devid, ChannelID chanid,
             p_Buffer buffer, unsigned length,
             void *stateptr)
{
    unsigned int reason;
    unsigned int debugID;
    unsigned int OSinfo1;
    unsigned int OSinfo2;
    unsigned int param;

    IGNORE(chanid);
    IGNORE(stateptr);
    IGNORE(length);

    LogInfo(LOG_BOOT, ( "Enter: boot_handler\n"));

    unpack_message(BUFFERDATA(buffer), "%w%w%w%w%w", &reason, &debugID, &OSinfo1,
                   &OSinfo2, &param);

    LogInfo(LOG_BOOT, ("boot_handler: message received: reason 0x%08x (%s), "
                         "debugID 0x%08x, OSinfo1 0x%08x, OSinfo2 0x%08x, param 0x%8x\n",
                         reason, log_adpname(reason), debugID, OSinfo1, OSinfo2, param));

    if (!(reason & TtoH))
    {

        /* We always take the precaution of setting the default dev
         * when we get a HBOOT message.
         */
        angel_ChannelSelectDevice(devid);
        if (reason != ADP_ParamNegotiate)
        {
            LogInfo(LOG_BOOT, ( "boot_handler: release request buffer.\n"));
            angel_ChannelReleaseBuffer(buffer);
        }
        
        switch (reason)
        {
            case ADP_Booted:
                {
                    if (
                           boot_state == BS_RESETTING
#if (LATE_STARTUP > 0) && !defined(ICEMAN_LEVEL_3)
                           || boot_state == BS_AVAILABLE
#endif
                        )
                    {
                        LogInfo(LOG_BOOT, ( "Received ADP_Booted ACK.\n"));
                        Angel_InitChannels();
                        Angel_InitBuffers();
                        boot_state = BS_CONNECTED;
                        boot_completed = 1;
                        /* see commentary in send_booted_message(). This call
                         * reenables normal operation of the packet code.
                         */
                        LogInfo(LOG_BOOT, ( "(Re)Enable Packet FLOW:\n"));
                        booted_flow_controlled = 0;
                        angel_DeviceControl(devid, DC_RX_PACKET_FLOW, (void*)1);
                    }
                    else
                        LogWarning(LOG_BOOT, ( "ADP_Booted ACK received, "
                                                 "state not BS_RESETTING.\n"));

                    break;
                }

            case ADP_Reboot:
                {
                    int err;
                    LogInfo(LOG_BOOT, ( "Received ADP_Reboot request.\n"));
                    if ((err = msgsend(CI_HBOOT, "%w%w%w%w%w", ADP_Reboot | TtoH,
                                      ADP_HandleUnknown, ADP_HandleUnknown,
                                      ADP_HandleUnknown, REBOOT_ACK)) != 0)
                    {
                        LogWarning(LOG_BOOT, ( "Cannot ACK Reboot: Error %d; "
                                                 "aborting request\n", err));
                    }
                    else
                    {
                        LogInfo(LOG_BOOT, ( "----------------------------------\n"
                                              "--- Rebooting...\n\n\n"));
                        Angel_EnterSVC();
                        __rom();  /* should never return */
                    }
                    break;
                }

            case ADP_Reset:
                {
                    int reset_ack, err;
                    bool do_reset = FALSE;

                    LogInfo(LOG_BOOT, ( "Received ADP_Reset request.\n"));

                    switch (boot_state)
                    {
                        case BS_STARTUP:
                            {
                                LogInfo(LOG_BOOT, ( "Deferring ADP_Reset request.\n"));
                                reset_ack = AB_LATE_ACK;
                                boot_state = BS_STARTUP;
                                break;
                            }

                        case BS_RESETTING:
                            {
                                LogWarning(LOG_BOOT, ( "Already resetting.\n"));
                                /* fall through to... */
                            }

                        case BS_AVAILABLE:
                        case BS_CONNECTED:
                            {
                                LogInfo(LOG_BOOT, ( "Actioning ADP_Reset_request.\n"));
                                reset_ack = AB_NORMAL_ACK;
                                do_reset = TRUE;
                                boot_state = BS_RESETTING;
                                boot_completed = FALSE;
                                break;
                            }

                        default:
                            {
                                LogError(LOG_BOOT, ( "ERROR: bad case.\n"));
                                reset_ack = AB_ERROR;
                                break;
                            }
                    }

                    if ((err = msgsend(CI_HBOOT, "%w%w%w%w%w", ADP_Reset | TtoH,
                                ADP_HandleUnknown, ADP_HandleUnknown,
                                ADP_HandleUnknown, reset_ack)) != 0)
                    {
                        LogWarning(LOG_BOOT, ( "Cannot ACK Reset: Error %d; "
                                                 "aborting request\n", err));
                    }

                    if (do_reset)
                    {
                        /* Initialise the the target dependant debug code.
                         * This is needed as the debugger tool box does not
                         * tidy up after itself and expects the target to be
                         * initialised when the debugger starts up.
                         */

                        /* Take note of the endianess of the host debugger */
                        LogInfo(LOG_BOOT, ( "----------------------------------\n"
                                            "--- Resetting...\n\n"));
                        angel_debugger_endianess = param;
#if LATE_STARTUP == 0 || defined(ICEMAN_LEVEL_3)
                        angelOS_Initialise();
#endif
                        /*
                         * Originally, the code reinitialised channels, etc, here.
                         * However, the host code doesn't until after the booted
                         * message is sent. As we already reinit after the booted
                         * msg is received in the target, the initialise here can
                         * just be deleted.
                         */
                        send_booted_message();
                    }
                    break;
                }               /* end case ADP_Reset */

            case ADP_ParamNegotiate:
                {
                    param_negotiate(devid, buffer + (4 * 4));
                    LogInfo(LOG_BOOT, ( "boot_handler: release param request.\n"));
                    angel_ChannelReleaseBuffer(buffer);
                    break;
                }

            case ADP_LinkCheck:
                {
                    LogInfo(LOG_BOOT, ( "LinkCheck Received, disable rx packet flow\n"));

                    /* disable packet input until reinit has completed. */
                    angel_DeviceControl(devid, DC_RX_PACKET_FLOW, (void*)0);
                    
                    msgsend(CI_HBOOT, "%w%w%w%w", ADP_LinkCheck | TtoH, 0,
                            ADP_HandleUnknown, ADP_HandleUnknown);
                    
                    Angel_ReinitChannels();

                    /* reinit ok, continue... */
                    LogInfo(LOG_BOOT, ( "Enable rx packet flow\n"));
                    angel_DeviceControl(devid, DC_RX_PACKET_FLOW, (void*)1);
                    
                    LogInfo(LOG_BOOT, ( "LinkCheck: Reinit Channels complete.\n"));
                    break;
                }

            default:
                LogWarning(LOG_BOOT, ( "boot_handler: Reason code not recognised.\n"));
                break;
        }
    }
    else
        LogWarning(LOG_BOOT, ( "boot_handler got TtoH message!"));

    return;
}

/*---------------------------------------------------------------------------*/

void
angel_BootInit(void)
{
    void *stateptr = NULL;
    ChanError err = CE_OKAY;    /* Error value */
    static bool registered = FALSE;

    LogInfo(LOG_BOOT, ( "Enter: angel_BootInit\n"));

    /* Register our message handler for both channels: */
    /* TODO: check return value */
    if (!registered)
    {
        LogInfo(LOG_BOOT, ( "registering boot handlers\n"));
        err = angel_ChannelReadAll(CI_HBOOT, boot_handler, stateptr);
        err = angel_ChannelReadAsync(CH_DEFAULT_DEV, CI_TBOOT, boot_handler,
                                     stateptr);
        registered = TRUE;
    }

    /* Originate the ADP_Booted message to notify the host that the
     * system is alive. This allows the host to start generating
     * standard messages. NOTE: The "debugID" field is set to unknown
     * here, since we have not yet had a communication from the host.
     */
    if (boot_state == BS_AVAILABLE)
    {
        (void)send_booted_message();
    }
    
    return;
}


void
angel_LateBootInit(unsigned int swi_code, angel_LateStartType type)
{
    ASSERT(swi_code == angel_SWIreason_LateStartup, ("bad SWI reason %d", swi_code));

    if (boot_state != BS_STARTUP)
    {
        LogError(LOG_BOOT, ( "late startup already called."));
    }
    else
    {
        LogInfo(LOG_BOOT, ( "angel_LateBootInit\n"));

        switch (type)
        {
            case AL_CONTINUE:
                {
                    /* do nothing other than sending the boot message */
                    break;
                }

            case AL_BLOCK:
                {
                    Angel_BlockApplication(1);
                    break;
                }

            default:
                {
                    LogError(LOG_BOOT, ( "Unknown LateStartType %d.\n", type));
                    break;
                }
        }

        boot_state = BS_AVAILABLE;
        send_booted_message();
        Angel_Yield();          /* to ensure BlockApplication() takes effect */
    }
}


/* EOF boot.c */
