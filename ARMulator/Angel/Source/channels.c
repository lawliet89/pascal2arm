/* -*-C-*-
 *
 * $Revision: 1.13.2.11 $
 *   $Author: rivimey $
 *     $Date: 1998/03/11 10:38:37 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Implementation of channels layer
 */

#include <stdlib.h>             /* for NULL macro */

#include "channels.h"
#include "chandefs.h"
#include "chanpriv.h"
#include "devclnt.h"
#include "devconf.h"
#include "logging.h"
#include "serlock.h"
#include "adp.h"
#include "adperr.h"
#include "endian.h"
#include "msgbuild.h"

/* General purpose constants, macros, enums, typedefs */

/*
 * The number of packets which can be outstanding at once -- normally
 * called the window size. In this case, though, the window size is
 * in a sense 1 as each channel is stop-and-wait. However, theoretically
 * all channels could have sent a packet and none received an ack, so
 * the number of outstanding packets possible is the number of configured
 * channels.
 */
#define MAX_UNACKED   CI_NUM_CHANNELS

/**********************************************************************
 *                      T Y P E    D E F I N I T I O N S
 **********************************************************************/

enum ChanPrivateErrors
{
    CE_BLOCKING = CE_PRIVATE,   /* used for blocking reads and sends */
    CE_UNBLOCK_BASE,
    CE_UNBLOCK_OKAY = CE_UNBLOCK_BASE + CE_OKAY
};

/* state per channel */
struct ChanState
{
    ChanRx_CB_Fn reader;               /* the read callback, or NULL */
    void        *read_cb_data;         /* the read callback data */
    p_Buffer     read_buffer;          /* for blocking reads */
    unsigned int read_len;             /* for blocking reads */
    bool         read_blocking;        /* for blocking reads */
    ChanTx_CB_Fn writer;               /* the write callback, or NULL */
    void        *write_cb_data;        /* the write callback data */
    bool         write_blocking;       /* for blocking sends */
    p_Buffer     write_buff;           /* buffer being written */
};

/* state per device */

/*
 * ackExpected and currentFrame form the lower and upper edges of the sender's
 * window.
 *
 * frameExpected is the lower and upper edges of the receiver's
 * window.
 */
struct DevState
{
    SequenceNR currentFrame;      /* seq number of last frame sent */
    SequenceNR ackExpected;       /* seq number of successfully-received frame */
    SequenceNR frameExpected;     /* seq number of last received frame from host */

    SequenceNR lastReceived;      /* distinguish heartbeat(8) from data(8) */
    
    /*
     * The following is used to implement a list of buffers which have yet to
     * be acknowledged.
     */
    struct _unacked
    {
        p_Buffer        buf;    /* the list of un-acked packets */
        unsigned short  len;    /* the length of data in this packet */
        int             packno; /* packet sequence number for this packet */
        short           next;   /* next buffer in this list, LISTEND => none */
    } unackedBufs[MAX_UNACKED];
    short firstFree;            /* first free buffer, LISTEND => none */
    short lastUsed;             /* last (most recently pushed) buffer, LISTEND => none */
    short firstUsed;            /* first (oldest) buffer, LISTEND => none */
};

#define LISTEND  (-1)

extern int booted_flow_controlled;

/**********************************************************************
 *                      M O D U L E   D A T A
 **********************************************************************/

/* Publically-accessible globals */

/* do we acknowledge heartbeats; TRUE by default. */
unsigned Angel_AckedHeartbeats = 1;

/* the default size of a channel buffer, for global use */
unsigned Angel_ChanBuffSize = 0;

/* the size of a long buffer, for global use */
unsigned Angel_ChanLongSize = 0;


/* Private globals */

/* the state of each channel */
static volatile struct ChanState chan_state[CI_NUM_CHANNELS];

/* the state of each device */
static struct DevState dev_state[DI_NUM_DEVICES];

/* the active device */
static DeviceID active_device = DE_DEFAULT_ACTIVE;

/* the reader-of-all-devices */
static ChannelID read_all_channel;
static ChanRx_CB_Fn read_all_callback = NULL;
static void *read_all_cb_data = NULL;


/**********************************************************************
 *          P R O T O T Y P E   D E C L A R A T I O N S
 **********************************************************************/

/* forward declarations */
static p_Buffer angel_ChannelAllocDevBuffer(unsigned req_size, void *cb_data);
static AdpErrs resend_packet_from(DeviceID devid, SequenceNR packet_no);
static void angel_ChannelQuerySizes(unsigned *default_size, unsigned *max_size);

/**********************************************************************
 *              I N T E R N A L   F U N C T I O N S
 **********************************************************************/

/*
 *  Function: adp_between
 *   Purpose: To return TRUE if a <= b < c circularly.
 *
 *    Params: 
 *       Input: none
 *
 *   Returns: TRUE/FALSE
 */
static int
adp_between(SequenceNR a, SequenceNR b, SequenceNR c)
{
    return (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)));
}


/*
 *  Function: angel_InitResendList
 *   Purpose: To initialise the resend list - link all elements onto
 *            the free list, and setting the used list to empty
 *
 *    Params: 
 *       Input: dev - the device to initialise.
 *
 *   Returns: none
 */
static void
angel_InitResendList(register struct DevState *dev)
{
    register int i;
    
    dev->firstFree = 0;
    dev->firstUsed = LISTEND;
    dev->lastUsed = LISTEND;
    
    for(i = 0; i < MAX_UNACKED; i++)
    {
        dev->unackedBufs[i].buf = NULL;
        dev->unackedBufs[i].len = 0;
        dev->unackedBufs[i].packno = 0;
        dev->unackedBufs[i].next = i + 1;
    }
    dev->unackedBufs[i - 1].next = LISTEND;
    
    return;
}


/*
 *  Function: Angel_InitChannels
 *   Purpose: To Initialise the sequence numbers, and resend list.
 *            This should be called only when the state of memory
 *            is unknown (i.e. the data structures are random).
 *
 *    Params: 
 *       Input: none
 *
 *   Returns: adp_ok
 */
AdpErrs
Angel_InitChannels(void)
{
    int j;

    LogInfo(LOG_CHANNEL, ( "Angel_InitChannels: resetting...\n"));
    Angel_EnterCriticalSection();
    for (j = 0; j < DI_NUM_DEVICES; j++)
    {
        register struct DevState *dev = &dev_state[j];

        dev->currentFrame = 0;
        dev->ackExpected = 0;
        dev->lastReceived = 0;
        dev->frameExpected = 1;
        
        angel_InitResendList(dev);
    }
    Angel_LeaveCriticalSection();
    
    LogInfo(LOG_CHANNEL, ( "Angel_InitChannels: reset channels.\n"));
    return adp_ok;
}


/*
 *  Function: Angel_ReinitChannels
 *   Purpose: To re initialise the device; very similar to the
 *            initialise, but assumes the device state is "sensible",
 *            freeing any buffers allocated on the resend list.
 *
 *    Params: 
 *       Input: none
 *
 *   Returns: adp_ok
 */

AdpErrs
Angel_ReinitChannels(void)
{
    int i, j;
    
    LogInfo(LOG_CHANNEL, ( "Angel_ReinitChannels: resetting...\n"));
    Angel_EnterCriticalSection();
    for (j = 0; j < DI_NUM_DEVICES; j++)
    {
        register struct DevState *dev = &dev_state[j];

        dev->currentFrame = 0;
        dev->ackExpected = 0;
        dev->lastReceived = 0;
        dev->frameExpected = 1;

        /* free all packets on the used list */
        i = dev->firstUsed;
        while ( i != LISTEND )
        {
            if (dev->unackedBufs[i].buf)
            {
                Angel_BufferRelease(dev->unackedBufs[i].buf);
            }
            
            i = dev->unackedBufs[i].next;
        }

        /* then reinitialise the free list. */
        angel_InitResendList(dev);
    }
    Angel_LeaveCriticalSection();    

    LogInfo(LOG_CHANNEL, ( "Angel_ReinitChannels: reset channels.\n"));
    
    return adp_ok;
}


/*
 *  Function: adp_get_unack_bufno
 *   Purpose: To search the resend list for a given packet and return the buffer
 *            number in which it is located. Additionally, return the previous
 *            buffer number, if any, for the purposes of list pointer changes.
 *
 *    Params: 
 *       Input: devid    - the device ID of the driver 
 *              pkt_no   - the packet number required
 *              startbuf - the initial buffer index (use to select btw. free
 *                         and used buffer lists)
 *
 *      Output: prev     - if non-null, a pointer to a value holding the previous
 *                         buffer, whose next pointer points at the wanted one, or
 *                         LISTEND signifying this was the first buffer.
 *
 *   Returns: OK:    the buffer index of the required buffer
 *            Error: LISTEND if packet number not found
 */
static int adp_get_unack_bufno(DeviceID devid, SequenceNR pkt_no,
                               short startbuf, short *prev)
{
    register struct DevState *dev = &dev_state[devid];
    register short i, j = LISTEND;

    for (i = startbuf; i != LISTEND; i = dev->unackedBufs[i].next)
    {
        if (dev->unackedBufs[i].packno == pkt_no)
        {
            if (prev != NULL)
                *prev = j;
            return i;
        }
        j = i;
    }

    return LISTEND;
}


/*
 *  Function: Angel_show_unacked_list
 *   Purpose: To display the used and free resend lists for a given device.
 *
 *    Params: 
 *       Input: devid - the device ID of the driver 
 *
 *   Returns: none
 */
#if DEBUG == 1
static void Angel_show_unacked_list(DeviceID devid)
{
    register struct DevState *dev = &dev_state[devid];
    register int i;

    LogInfo(LOG_CHANNEL, ("Resend list for Device %d\n", devid));
    for (i = dev->firstUsed; i != LISTEND; i = dev->unackedBufs[i].next)
    {
        LogInfo(LOG_CHANNEL, ("    Buffer %d: Packet %d unacked (@%x, len %d)\n", i,
                              dev->unackedBufs[i].packno, dev->unackedBufs[i].buf,
                              dev->unackedBufs[i].len));
    }
}
#else
#define Angel_show_unacked_list(d)
#endif


/*
 *  Function: adp_add_pkt
 *   Purpose: To add the given packet to the resend list.
 *
 *    Params: 
 *       Input: devid - the device ID of the driver 
 *              pkt_no - the packet number required
 *              buffer - the buffer pointer for this packet
 *              len - the length of this packet in bytes
 *
 *   Returns: adp_ok
 */
static AdpErrs adp_add_pkt(DeviceID devid, SequenceNR pkt_no, p_Buffer buffer, int len)
{
    int b; /* the list reference number of the packet to add */
    register struct DevState *dev = &dev_state[devid];

    Angel_EnterCriticalSection();
    
    if (dev->firstFree != LISTEND)
    {
        /* remove this buffer from the free list */
        b = dev->firstFree;
        dev->firstFree = dev->unackedBufs[b].next;

        /* set it up... */
        dev->unackedBufs[b].buf = buffer;
        dev->unackedBufs[b].len = len;
        dev->unackedBufs[b].packno = pkt_no;

        /* now add it onto the unacked buffer list. firstUsed is the head -- set
         * this if the list was initially empty. lastUsed is the tail; set this
         * to the current buffer after modifying the old 'next' pointer.
         */
        dev->unackedBufs[b].next = LISTEND;
        if (dev->lastUsed != LISTEND)
            dev->unackedBufs[dev->lastUsed].next = b;
        else
            dev->firstUsed = b;
        dev->lastUsed = b;

        Angel_LeaveCriticalSection();
        
        LogInfo(LOG_CHANNEL, ("adp_add_pkt: Dev: %d stored packet %d, buffer 0x%x\n",
                              devid, pkt_no, buffer));
        Angel_show_unacked_list(devid);

        return adp_ok;
    }
    
    Angel_LeaveCriticalSection();    

    LogWarning(LOG_CHANNEL, ("adp_add_pkt: Dev %d: No free buffers to add"
                             " packet %d to resend list\n", devid, pkt_no));
    Angel_show_unacked_list(devid);
    return adp_ok;
}


/*
 *  Function: adp_free_pkt
 *   Purpose: To remove the packet with a given buffer ref from the resend list.
 *
 *  Pre-Conditions: 'cur' must point to a buffer in the used list.
 *
 *    Params: 
 *       Input: devid - the device ID of the driver 
 *              cur - the buffer ref of the buffer to delete
 *              last - the buffer ref of the previous buffer, or LISTEND if cur is
 *                     first buffer
 *
 *   Returns: adp_ok
 */
static void adp_free_pkt(register struct DevState *dev, int cur, int last)
{
    /* free up the actual buffer */
    if (dev->unackedBufs[cur].buf)
        Angel_BufferRelease(dev->unackedBufs[cur].buf);
    else
        LogWarning(LOG_CHANNEL, ("adp_free_pkt: buffer NULL at index %d\n", cur));

        /* clear buffer contents for tidyness */
    dev->unackedBufs[cur].len = 0;
    dev->unackedBufs[cur].packno = 0;
    dev->unackedBufs[cur].buf = NULL;

    /* remove buffer from used list */
    if (cur == dev->firstUsed)
    {
        /* remove first, possibly only item */
        dev->firstUsed = dev->unackedBufs[cur].next;
        if (dev->firstUsed == LISTEND)
            dev->lastUsed = LISTEND;
    }
    else if (cur == dev->lastUsed)
    {
        /* remove last item -- cannot be only one */
        dev->lastUsed = last;
        dev->unackedBufs[last].next = LISTEND;
    }
    else
    {
        /* remove any other -- must be at least 3 to begin with. */
        dev->unackedBufs[last].next = dev->unackedBufs[cur].next;       
    }

    /* and add back to free list */
    dev->unackedBufs[cur].next = dev->firstFree;
    dev->firstFree = cur;
}


/*
 *  Function: adp_remove_pkt
 *   Purpose: To remove all packets which are acknowledged by
 *            the given sequence number. Normally, this is just
 *            the packet which has sequence number 'pkt_no', but
 *            if two (or more) packets are sent out simultaneously,
 *            then both will be ack'd by the next reply from the
 *            host. Two or more packets can be sent without an
 *            intervening reply if the packets are on different
 *            channels.
 *
 *    Params: 
 *       Input: devid - the device the packet is using
 *              pkt_no - the packet sequence number
 *
 *   Returns: adp_err - an adp error code equivalent to dev_err
 */

static AdpErrs adp_remove_pkt(DeviceID devid, SequenceNR pkt_no)
{
    struct DevState *dev = &dev_state[devid];
    short i, j = LISTEND;

    LogInfo(LOG_CHANNEL, ("adp_remove_pkt: Dev: %d ack packet %d (ackExpected %d to max %d)\n",
                          devid, pkt_no, dev->ackExpected, nextSEQ(dev->currentFrame)));
    
    Angel_EnterCriticalSection();

    while (adp_between(dev->ackExpected, pkt_no, nextSEQ(dev->currentFrame)))
    {
        /* if we've got a buffered packet for this sequence number,
         * free it now.
         */
        i = adp_get_unack_bufno(devid, dev->ackExpected, dev->firstUsed, &j);
        if (i != LISTEND)
        {
            adp_free_pkt(dev, i, j);
        }
        
        INC(dev->ackExpected);
    }

    Angel_LeaveCriticalSection();
        
    Angel_show_unacked_list(devid);

    return adp_ok;
}


/*
 *  Function: adp_translate_dev_error
 *   Purpose: To translate an error code from the underlying device
 *            system to the ADP error code set.
 *
 *    Params: 
 *       Input: dev_err  - a device error code (DE_xxx)
 *
 *   Returns: adp_err - an adp error code equivalent to dev_err
 */

AdpErrs
adp_translate_dev_error(DevError dev_err)
{
    switch (dev_err)
    {
        case DE_OKAY:
            return adp_ok;
        case DE_NO_DEV:
        case DE_BAD_DEV:
            return adp_device_not_found;
        case DE_BAD_CHAN:
            return adp_bad_channel_id;
        case DE_BUSY:
            return adp_write_busy;
        case DE_INVAL:
            return adp_bad_packet;
        default:
            return adp_failed;
    }
}


/*
 *  Function: release_callback
 *   Purpose: To release the buffer used by various send functions, if the
 *            packet being sent is not on a reliable packet stream (i.e. there
 *            will be no acknowledging packet).
 *
 *    Params: 
 *       Input: v_chan_pkt -- pointer to packet's data
 *              v_chan_len -- length of the packet.
 *              v_status   -- status
 *              cb_data    -- the device ID
 *
 *   Returns: none
 */
static void release_callback(void *v_chan_pkt,
                            void *v_chan_len,
                            void *v_status,
                            void *cb_data)
{
    DevStatus status = (DevStatus) (int)v_status;
    p_Buffer  chan_pkt = (p_Buffer)v_chan_pkt;
    p_Buffer  buffer = chan_pkt - CB_CHAN_DATA_BYTE_POS;

    IGNORE(cb_data);
    IGNORE(v_chan_len);

    if (status != DS_DONE)
        LogWarning(LOG_CHANNEL, ("release_callback: status not OK (%d)\n", status));

    LogInfo(LOG_CHANNEL, ( "release callback: freeing packet\n"));
    Angel_BufferRelease( buffer );
}


/*
 *  Function: send_resend_msg
 *   Purpose: To format and send a resend message on device devid.
 *            The packet sequence numbers are taken from the device 
 *            state, and the last good frame (i.e. one before the
 *            packet being requested) is passed in.
 *
 *    Params: 
 *       Input: devid - the device ID of the driver
 *              lastGoodFrame - the sequence number of the last good
 *                      frame. The other side should send LGF+1.
 *
 *   Returns: AdpErrs - adp_ok if ok
 *            adp_failed if buffer not available
 *            translation of device error (see adp_translate_dev_error())
 *            otherwise.
 */
static AdpErrs 
send_resend_msg(DeviceID devid, SequenceNR lastGoodFrame)
{
    DevError dev_err;
    p_Buffer packet;

    packet = angel_ChannelAllocBuffer(CF_DATA_BYTE_POS);
    if (packet == NULL)
    {
        LogError(LOG_CHANNEL, ("send_resend_msg: Could not allocate a buffer\n"));
        return adp_failed;
    }
    packet[CF_CHANNEL_BYTE_POS] = CI_PRIVATE;
    packet[CF_PACKET_SEQ_BYTE_POS] = dev_state[devid].currentFrame;
    packet[CF_ACK_SEQ_BYTE_POS] = lastGoodFrame;
    packet[CF_FLAGS_BYTE_POS] = CF_RESEND;

    LogWarning(LOG_CHANNEL, ( "send_resend_msg: resend for frame %d\n",
                              lastGoodFrame));
    do
    {        
        dev_err = angel_DeviceWrite(devid, packet,
                                    CF_DATA_BYTE_POS, release_callback,
                                    (void *)CI_PRIVATE, DC_DBUG);
        if (dev_err == DE_BUSY);
        {
            Angel_Yield();
        }
    }
    while (dev_err == DE_BUSY);
    
    if (dev_err == DE_OKAY)
    {
        LogInfo(LOG_CHANNEL, ( "sent resend msg... ok\n", dev_err));
    }
    else
    {
        LogError(LOG_CHANNEL, ( "sent resend msg, FAILED ret code 0x%x\n", dev_err));
        angel_ChannelReleaseBuffer(packet);
    }
    return adp_translate_dev_error(dev_err);
}

/*
 *  Function: send_heartbeat_ack_msg
 *   Purpose: Send the acknowledgement for a heartbeat msg
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: AdpErrs - should be adp_ok
 */
static AdpErrs 
send_heartbeat_ack_msg(DeviceID devid, int timestamp)
{
    p_Buffer packet;
    DevError dev_err;

    /*
     * if not ack'ing then just return; we dont want to mess our
     * debugger up!
     */
    if (!Angel_AckedHeartbeats)
    {
        LogInfo(LOG_CHANNEL, ( "not acking heartbeats.\n"));
        return adp_ok;
    }

    /*
     * Send a resend message, usually in response to a bad packet or
     * a resend request
     */
    packet = angel_ChannelAllocBuffer(CF_DATA_BYTE_POS);
    if (packet == NULL)
    {
        return adp_failed;
    }
    packet[CF_CHANNEL_BYTE_POS] = CI_PRIVATE;
    packet[CF_PACKET_SEQ_BYTE_POS] = dev_state[devid].currentFrame;
    packet[CF_ACK_SEQ_BYTE_POS] = lastSEQ(dev_state[devid].frameExpected);
    packet[CF_FLAGS_BYTE_POS] = CF_HEARTBEAT;
    PUT32LE(packet + CF_DATA_BYTE_POS, timestamp);

    LogInfo(LOG_CHANNEL, ( "set up heartbeat packet, trying to write\n"));

    do
    {        
        dev_err = angel_DeviceWrite(devid, packet,
                                    CF_DATA_BYTE_POS + 4, release_callback,
                                    (void *)CI_PRIVATE, DC_DBUG);
        if (dev_err == DE_BUSY);
        {
            Angel_Yield();
        }
    }
    while (dev_err == DE_BUSY);
    
    if (dev_err == DE_OKAY)
    {
        LogInfo(LOG_CHANNEL, ( "sent heartbeat msg... ok\n", dev_err));
    }
    else
    {
        LogError(LOG_CHANNEL, ( "sent heartbeat msg, FAILED ret code 0x%x\n", dev_err));
        angel_ChannelReleaseBuffer(packet);
    }
    return adp_translate_dev_error(dev_err);
}

/*
 *  Function: read_callback
 *   Purpose: callback for packet rx on active device.
 *            This code handles all packet input for the DC_DBUG
 *            device channel, implementing it's own channel split
 *            on top of this to implement the various debugging
 *            channels.
 *
 *            The code must perform most of the reliability functions
 *            required by the debug channels, checking for lost or
 *            out of sequence packets, requesting resends etc. This
 *            version of the code is very different to that in previous
 *            Angel systems, as it is here that many of the improvements
 *            to Angel have been made for SDT 2.11a.
 *
 *            For a description of the code, see inline comments, and
 *            the forthcoming Guide "Angel SDT2.11a: An Introduction"
 *
 *    Params:
 *       Input: v_chan_pkt -- pointer to packet's data
 *              v_chan_len -- length of packet
 *              v_status   -- status of packet -- good or bad data
 *              cb_data    -- the device ID
 *
 *   Returns: Nothing. May call specific or general read callback,
 *            or trigger a message to be sent in reply.
 */

/* this macro is called when it is possible that a bad packet delivery
 * has been counted in the packet flow control; if this weren't done
 * the code would hang if waiting for the recovery packet when a bad
 * packet was delivered.
 */
#define check_flow_control()   if (booted_flow_controlled)  \
                                   angel_DeviceControl(devid, DC_RX_PACKET_FLOW, (void*)-1);

static void
read_callback(void *v_chan_pkt,
              void *v_chan_len,
              void *v_status,
              void *cb_data)
{
    p_Buffer chan_pkt = (p_Buffer) v_chan_pkt;
    unsigned chan_len = (unsigned)v_chan_len;
    DevStatus status = (DevStatus) (int)v_status;
    DeviceID devid = (DeviceID) cb_data;
    ChannelID chanid;
    SequenceNR rx_packetseq;
    SequenceNR rx_ackseq;
    unsigned char rx_flags;
    register struct DevState *dev = NULL;
    
    /*
     * clear buffers mode tells buffers.c to put 0xDEAD in
     * free buffers. check we haven't been handed one of these.
     */
    if (*chan_pkt == 0xAD && *(chan_pkt+1) == 0xDE &&
        *(chan_pkt+2) == 0xAD && *(chan_pkt+3) == 0xDE )
    {
        LogError(LOG_CHANNEL, ("read callback: called with free buffer %x (status %d)\n",
                               chan_pkt, status));
        return;
    }
           
    dev = &dev_state[devid];
    switch (status)
    {

        case DS_DONE:
            /* extract and check channel */
            chanid = chan_pkt[CF_CHANNEL_BYTE_POS];

            rx_packetseq = chan_pkt[CF_PACKET_SEQ_BYTE_POS];
            rx_ackseq = chan_pkt[CF_ACK_SEQ_BYTE_POS];
            rx_flags = chan_pkt[CF_FLAGS_BYTE_POS];
            LogInfo(LOG_CHANNEL, ("read_callback: rx msg buffer %p chan %d with "
                                  "packetseq %d, ackseq %d, rxflags 0x%x\n", chan_pkt,
                                  chanid, rx_packetseq, rx_ackseq, rx_flags));

            if (chanid >= CI_NUM_CHANNELS)
            {
                LogError(LOG_CHANNEL, ("Bad channel ID %d\n", chanid));
                angel_ChannelReleaseBuffer(chan_pkt + CF_DATA_BYTE_POS);
                check_flow_control();
                return;
            }

            if (rx_flags & CF_RESEND)
            {
                LogInfo(LOG_CHANNEL, ("*** Resend packet received, release buffer 0x%x\n",
                                        chan_pkt + CF_DATA_BYTE_POS));

                /* free this buffer as we don't need it any more */
                angel_ChannelReleaseBuffer(chan_pkt + CF_DATA_BYTE_POS);

                /*
                 * resend packets contain a rx_ackseq value which is the last packet the
                 * requester got successfully. Hence, the packet we send is rx_ackseq + 1:
                 */
                resend_packet_from(devid, nextSEQ(rx_ackseq));
                check_flow_control();
                return;
            }


            if (rx_flags & CF_HEARTBEAT)
            {
                int timestamp = GET32LE(chan_pkt + CF_DATA_BYTE_POS);

                LogInfo(LOG_CHANNEL, ("*** Heartbeat packet received, timestamp %d\n",
                                     timestamp));

                angel_ChannelReleaseBuffer(chan_pkt + CF_DATA_BYTE_POS);
                
                check_flow_control();
                
                if (rx_packetseq == dev->frameExpected)
                {
                    /* this is not good enough; although it's good that this isn't lower,
                     * we haven't made sure yet that we actually received the real
                     * packet with this sequence number (we should have...)
                     */
                    if (rx_packetseq == dev->lastReceived)
                    {
                        /* all's well */
                        LogInfo(LOG_CHANNEL, ("+++ seq ok: (%d), returning ack...\n",
                                              rx_packetseq));
                        send_heartbeat_ack_msg(devid, timestamp);
                    }
                    else
                    {
                        LogInfo(LOG_CHANNEL, ("+++ seq low: (%d < %d), requesting resend of %d+1\n",
                                              dev->lastReceived, rx_packetseq, lastSEQ(dev->frameExpected)));
                        send_resend_msg(devid, lastSEQ(dev->frameExpected));
                    }
                }
                else
                {
                    /*
                     * The sequence number isn't the one we want. If it's already been
                     * received, then that's fine... just throw it away. If it's too
                     * high, we've missed something, so must request a resend.
                     * When dealing with circular sequence numbers, we cannot just use
                     * the < and > operators; define the midpoints in the sequence.
                     */
                    int low = dev->frameExpected - (MAX_SEQ/2);
                    if (low < 0)
                        low += MAX_SEQ;

                    if (adp_between(low, rx_packetseq, dev->frameExpected))
                    {
                        /* sequence low;. */
                        LogInfo(LOG_CHANNEL, ("+++ seq low: (%d <= %d < %d), acking anyway.\n",
                                              low, rx_packetseq, dev->frameExpected));
                        send_heartbeat_ack_msg(devid, timestamp);
                    }
                    else
                    {
                        /* sequence high; missed something. */
                        LogInfo(LOG_CHANNEL, ("+++ seq high: (%d), requesting resend of %d+1.\n",
                                              rx_packetseq, lastSEQ(dev->frameExpected)));
                        
                        send_resend_msg(devid, lastSEQ(dev->frameExpected));
                    }
                }
                return;
            }

            if (rx_flags & CF_RELIABLE)
            {
                LogInfo(LOG_CHANNEL, ("*** Reliable packet received, rx seq %d, ack: %d\n",
                                        rx_packetseq, rx_ackseq));

                /* record that we received this packet -- needed to disambiguate
                 * subsequent heartbeats.
                 */
                if (dev->lastReceived != lastSEQ(rx_packetseq))
                    LogInfo(LOG_CHANNEL, ("@@@ This packet doesn't follow on from "
                                          "last received (%d)?\n", dev->lastReceived));
                    
                dev->lastReceived = rx_packetseq;

                /* This code allows the initial reset through on Ethernet, where a
                 * previous session will have left the sequence numbers in an
                 * indeterminate state (on serial/serpar, the final LinkCheck will
                 * reset them)
                 */
#if ETHERNET_SUPPORTED
                if (chanid == CI_HBOOT && devid == DI_ETHERNET)
                {
                    int reason;
                    p_Buffer data_pkt = chan_pkt + CF_DATA_BYTE_POS;
                    unpack_message(BUFFERDATA(data_pkt), "%w", &reason);
                    
                    if ((reason & 0xFFFF) == (ADP_Reset & 0xFFFF))
                    {
                        LogInfo(LOG_CHANNEL, ("*** Reset/Reboot packet received, "
                                              "reinit sequence numbers...\n"));

                        Angel_ReinitChannels();
                        
                        /* let this packet through, whatever it's sequence numbers... */
                        rx_packetseq = dev->frameExpected;
                    }
                }
#endif

                if (rx_packetseq == dev->frameExpected) /* seq ok */
                {
                    LogInfo(LOG_CHANNEL, ("seq ok (%d), channel %d\n", rx_packetseq, chanid));
                        
                    INC(dev->frameExpected);
                        
                    /* remove the buffered packet which should have been kept */
                    adp_remove_pkt(devid, rx_ackseq);
                }
                else
                {
                    /*
                     * The sequence number isn't the one we want. If it's already been
                     * received, then that's fine... just throw it away. If it's too
                     * high, we've missed something, so must request a resend.
                     * When dealing with circular sequence numbers, we cannot just use
                     * the < and > operators; define the midpoints in the sequence.
                     */
                    int low = dev->frameExpected - (MAX_SEQ/2);
                    if (low < 0)
                        low += MAX_SEQ;
                    
                    check_flow_control();
                    if (adp_between(low, rx_packetseq, dev->frameExpected))  /* seq low */
                    {
                        LogWarning(LOG_CHANNEL, ("+++ seq low (%d <= %d < %d), discarding packet\n",
                                                 low, rx_packetseq, dev->frameExpected));
                        angel_ChannelReleaseBuffer(chan_pkt + CF_DATA_BYTE_POS);
                        
                        /* we have already received this packet so discard */
                        return;
                    }
                    else
                    {
                        /* sequence high; missed something. */
                        LogWarning(LOG_CHANNEL, ("+++ seq high (%d < %d > %d), asking host to resend\n",
                                                 low, rx_packetseq, dev->frameExpected));
                        angel_ChannelReleaseBuffer(chan_pkt + CF_DATA_BYTE_POS);

                        /* protocol sends last frame got ok, not the frame expected... */
                        send_resend_msg(devid, lastSEQ(dev->frameExpected));
                        return;
                    }
                }
            }
            else
            {
                LogInfo(LOG_CHANNEL, ("*** Uneliable packet received: rx seq: %d, frameExpected %d\n",
                                        rx_packetseq, dev->frameExpected));
            }

                /* packet accepted ; send it off ... */
            {
                p_Buffer data_pkt = chan_pkt + CF_DATA_BYTE_POS;
                unsigned data_len = chan_len - CF_DATA_BYTE_POS;

                if (chan_state[chanid].reader != NULL)
                {
                    chan_state[chanid].reader(devid, chanid, data_pkt, data_len,
                                              chan_state[chanid].read_cb_data);
                }
                else if (chanid == read_all_channel && read_all_callback != NULL)
                {
                    read_all_callback(devid, chanid, data_pkt, data_len,
                                      read_all_cb_data);
                }
                else
                {
                    LogWarning(LOG_CHANNEL, ("NO READER for channel %d, dropped!\n",
                                               chanid));
                    angel_ChannelReleaseBuffer(chan_pkt);
                }
            }
            break;

        default:
            LogWarning(LOG_CHANNEL, ("\n**** Receive bad msg (0x%x) freeing it.\n", chan_pkt));

            check_flow_control();
            /* free this buffer as we don't need it any more */

            if (chan_pkt != 0)
                angel_ChannelReleaseBuffer(chan_pkt + CF_DATA_BYTE_POS);
            break;

    }
}


/*
 * Function: angel_InitialiseChannels
 *  Purpose: initialise the channels layer
 *
 * see channels.h for full description
 */
void 
angel_InitialiseChannels(void)
{
    int i;

    LogInfo(LOG_CHANNEL, ( "angel_InitialiseChannels: start\n"));

    /* set up channel states */
    for (i = 0; i < CI_NUM_CHANNELS; ++i)
    {
        chan_state[i].reader = NULL;
        chan_state[i].read_cb_data = NULL;
        chan_state[i].read_buffer = NULL;
        chan_state[i].read_blocking = FALSE;
        chan_state[i].writer = NULL;
        chan_state[i].write_buff = NULL;
        chan_state[i].write_cb_data = NULL;
        chan_state[i].write_blocking = FALSE;
    }

    /* set up device states... */
    Angel_InitChannels();

    for (i = 0; i < DI_NUM_DEVICES; ++i)
    {
        if (angel_IsAngelDevice(i))
        {
            /* ...and register callback */
            if (angel_DeviceRegisterRead(i, read_callback, (void *)i,
                                         angel_ChannelAllocDevBuffer, NULL,
                                         DC_DBUG)
                != DE_OKAY)
            {
                LogError(LOG_CHANNEL, ( "cannot DeviceRegisterRead\n"));
            }
        }
    }
    active_device = DE_DEFAULT_ACTIVE;

    read_all_callback = NULL;
    read_all_cb_data = NULL;

    angel_ChannelQuerySizes(&Angel_ChanBuffSize, &Angel_ChanLongSize);

    LogInfo(LOG_CHANNEL, ( "angel_InitialiseChannels: complete\n"));

    return;
}

/*
 * Function: angel_ChannelQuerySizes
 *  Purpose: return the default and maximum buffer sizes available to user
 *
 *   Params:
 *              Input: -
 *             Output: default_size    the usual size of buffer
 *                     max_size        the largest size of buffer
 *             In/Out: -
 *
 *            Returns: -
 *
 *      Reads globals: -
 *   Modifies globals: -
 *
 * Other side effects: -
 */
static void 
angel_ChannelQuerySizes(unsigned *default_size,
                        unsigned *max_size)
{
    ASSERT(default_size != NULL, ("Nowhere to write default buffer size\n"));
    ASSERT(max_size != NULL, ("Nowhere to write max buffer size\n"));

    Angel_BufferQuerySizes(default_size, max_size);

    /* remove space used for channel overheads */
    *default_size -= CB_CHAN_DATA_BYTE_POS;
    *max_size -= CB_CHAN_DATA_BYTE_POS;
}

/*
 * Function: angel_CanAckHeartbeat
 *  Purpose: To return whether we can or can't acknowledge heartbeat
 *           packets. We set an internal variable TRUE letting us know
 *           that we should do so....
 *
 *   Params:  none.
 *
 *            Returns: - always TRUE
 *
 *      Reads globals: -
 *   Modifies globals: - Angel_AckedHeartbeats; read by code which
 *                       receives hbs to determine what to do.
 *
 * Other side effects: - none
 */

word 
angel_CanAckHeartbeat()
{
#if 1
    /* this is what was intended ... */
    Angel_AckedHeartbeats = TRUE;
    LogInfo(LOG_CHANNEL, ( "Will Acknowledge Heartbeats\n"));
    return TRUE;
#else
    /* this is to disable heartbeat acks for SDT211 (RIC 6.8.97) ... */
    /* as they cause problems elsewhere in the ROM code */
    Angel_AckedHeartbeats = FALSE;
    LogInfo(LOG_CHANNEL, ( "Not Acknowledging Heartbeats\n"));
    return FALSE;
#endif
}


/*
 * Function: angel_ChannelAllocBuffer
 *  Purpose: allocate a buffer that is at least req_size bytes long
 *
 * see channels.h for full description
 */

p_Buffer 
angel_ChannelAllocBuffer(unsigned req_size)
{
    p_Buffer ret_buffer;

    /* commented out as the buffer code also has v. similar info calls:*/
    /* LogInfo(LOG_CHANNEL, "angel_ChannelAllocBuffer: req_size %d\n", req_size); */

    /* make sure we get space for our overheads */
    req_size += CB_CHAN_DATA_BYTE_POS;

    ret_buffer = Angel_BufferAlloc(req_size);

    if (ret_buffer != NULL)
    {
        /* null the link pointer */
        *CB_LINK(ret_buffer) = NULL;

        /* shift the returned value to the data area */
        ret_buffer += CB_CHAN_DATA_BYTE_POS;
    }

    /* LogInfo(LOG_CHANNEL, "angel_ChannelAllocBuffer: buffer 0x%x\n", ret_buffer); */
    return ret_buffer;
}


/*
 * Function: angel_ChannelReleaseBuffer
 *  Purpose: release a buffer back to the free pool
 *
 * see channels.h for full description
 */

void 
angel_ChannelReleaseBuffer(p_Buffer buffer)
{
    /* commented out as the buffer code also has v. similar info calls: */
    /* LogInfo(LOG_CHANNEL, ("angel_ChannelReleaseBuffer: buffer 0x%x\n", buffer)); */

    /* adjust the pointer back to where it ought to be */
    if (buffer != NULL)
    {
        buffer -= CB_CHAN_DATA_BYTE_POS;
    }

    /* pass it on to the buffer module */
    Angel_BufferRelease(buffer);
}


/*
 * Function: angel_ChannelAllocDevBuffer
 *  Purpose: allocate a buffer that is at least req_size bytes long,
 *           alligned for use by the device driver. This routine is
 *           used to allocate buffers when receiving new packets.
 *
 *   Params:
 *              Input: req_size        the minimum size required
 *                     cb_data         ignored
 *             Output: -
 *             In/Out: -
 *
 *            Returns: pointer to allocated buffer, or
 *                     NULL if unable to allocate suitable buffer
 *
 *      Reads globals: -
 *   Modifies globals: -
 *
 * Other side effects: MUST ONLY be used by device drivers
 */

static p_Buffer 
angel_ChannelAllocDevBuffer(unsigned req_size, void *cb_data)
{
    p_Buffer ret_buffer;

    IGNORE(cb_data);

    /* commented out as the buffer code also has v. similar info calls: */
    LogInfo(LOG_CHANNEL, ("angel_ChannelAllocDevBuffer: req_size %d\n", req_size));

    /* make sure we get space for our overheads */
    req_size += CB_CHAN_HEADER_BYTE_POS;

    /*
     * There must be at least 2 packets left here -- one we will allocate, and
     * one which Angel can use to send a reply back (assuming that the in-buffer
     * isn't freed before the out-buffer is allocated, which is common).
     */
    if (Angel_BuffersLeft() < 2)
    {
         LogWarning(LOG_CHANNEL, ("angel_ChannelAllocDevBuffer: Not enough buffers left!\n"));
         return NULL;
    }
    
    ret_buffer = Angel_BufferAlloc(req_size);

    /*
     * Note this call can still fail, despite the check earlier!
     */
    if (ret_buffer != NULL)
    {
        /* null the link pointer */
        *CB_LINK(ret_buffer) = NULL;

        /* shift the returned value to the overall data area */
        ret_buffer += CB_CHAN_HEADER_BYTE_POS;
    }

    /* LogInfo(LOG_CHANNEL, "angel_ChannelAllocDevBuffer: buffer 0x%x\n", ret_buffer); */

    return ret_buffer;
}


/*
 * Function: write_unblock_callback
 *  Purpose: used to signal that a blocking write has completed, and that the
 *           write code can continue, returning to the caller.
 *
 *   Params:
 *              Input: chanid                the channel on which the write occurred.
 *                     callback_data         pointer to the status return (a ChanError)
 *
 *            Returns: none.
 *
 *      Reads globals: (via pointer) write_error
 *   Modifies globals: -
 *
 * Other side effects: MUST ONLY be used by device drivers
 */

static void 
write_unblock_callback(ChannelID chanid, void *callback_data)
{
    ChanError *write_error = (ChanError *) callback_data;

    ASSERT(chanid < CI_NUM_CHANNELS, ("Channel ID %d invalid in callback\n", chanid));
    ASSERT(chan_state[chanid].write_blocking == TRUE, ("Write not blocked in blocking callback!\n"));
    ASSERT(write_error != NULL, ("Nowhere to write error code in callback\n"));

    *write_error = CE_OKAY;
    chan_state[chanid].write_blocking = FALSE;
    LogInfo(LOG_CHANNEL, ( "write_unblock_callback: channel %d\n", chanid));
}


/*
 * Function: angel_ChannelSend
 *  Purpose: blocking send of a packet via a channel
 *
 * see channels.h for full description
 */

ChanError 
angel_ChannelSend(DeviceID devid, ChannelID chanid,
                  const p_Buffer data_pkt, unsigned data_len)
{
    ChanError chan_error;
    ChanError send_error = CE_PRIVATE;

    ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));

    LogInfo(LOG_CHANNEL, ( "ChannelSend: send buffer 0x%x, len %d\n",
                             data_pkt, data_len));
    
    /* Check that a write is not already in progress */
    Angel_EnterCriticalSection();
    if (chan_state[chanid].write_blocking)
    {
        Angel_LeaveCriticalSection();
        LogWarning(LOG_CHANNEL, ("ChannelSend: already busy on chan %d\n", chanid));
        angel_ChannelReleaseBuffer(data_pkt);
        return CE_BUSY;
    }
    chan_state[chanid].write_blocking = TRUE;

    Angel_LeaveCriticalSection();

    /* Start an async write */
    chan_error = angel_ChannelSendAsync(devid, chanid, data_pkt, data_len,
                                        write_unblock_callback,
                                        (void *)&send_error);

    if (chan_error != CE_OKAY)
    {
        chan_state[chanid].write_blocking = FALSE;
        return chan_error;
    }

    /* now wait for the write to complete */
    while (chan_state[chanid].write_blocking)
    {
        TRACE("csb");
        Angel_Yield();
    }

    if (send_error)
        LogWarning(LOG_CHANNEL, ( "ChannelSend: error code %d\n", send_error));
    else
        LogInfo(LOG_CHANNEL, ( "ChannelSend: packet 0x%x sent\n", data_pkt));

    return send_error;
}


/*
 * Function: write_callback
 *  Purpose: used to clean up after a non-blocking write has completed. The channel
 *           state must be freed up and the buffer used to hold the packet freed up.
 *
 *   Params:
 *              Input: v_chan_pkt            pointer to the packet's data buffer
 *                     v_chan_len            the length of the buffer
 *                     v_status              the send status -- was the write completed
 *                     v_cb_data             callback data: in this case, the channel id
 *
 *            Returns: none.
 *
 *      Reads globals: 
 *   Modifies globals: 
 *
 * Other side effects: 
 */

static void 
write_callback(void *v_chan_pkt, void *v_chan_len,
               void *v_status, void *cb_data)
{
    DevStatus status = (DevStatus) (int)v_status;
    p_Buffer buffer = (p_Buffer) v_chan_pkt;
    ChannelID chanid;

    IGNORE(v_chan_len);

    LogInfo(LOG_CHANNEL, ("write_callback: packet %p, len %d, status %d, channel %d\n",
                          v_chan_pkt, (int)v_chan_len, status, (ChannelID) cb_data));

    if (status == DS_DONE)
    {
        /* channel id is in callback data */
        chanid = (ChannelID) cb_data;
        ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));

        if (chan_state[chanid].writer != NULL)  /* could have been aborted */
        {
            /* do the callback */
            chan_state[chanid].writer(chanid, chan_state[chanid].write_cb_data);
            chan_state[chanid].writer = NULL;
            chan_state[chanid].write_buff = NULL;
        }
    }
    else
    {
        /*
         * don't know which channel was being written, so must look it up in the channel
         * state, then free it up
         */
        LogWarning(LOG_CHANNEL, ( "write_callback: device write error %d\n", status));
        for(chanid = 0; chanid < CI_NUM_CHANNELS; chanid++)
        {
            if (chan_state[chanid].write_buff == buffer)
            {
                LogWarning(LOG_CHANNEL, ( "write_callback: releasing device for chan %d\n", chanid));
                chan_state[chanid].writer = NULL;
                chan_state[chanid].write_buff = NULL;
                chan_state[chanid].write_blocking = FALSE;
            }
        }
    }
}

/*
 * Function: angel_ChannelSendAsync
 *  Purpose: asynchronous send of a packet via a channel
 *
 * see channels.h for full description
 */
ChanError 
angel_ChannelSendAsync(DeviceID devid,
                       ChannelID chanid,
                       const p_Buffer data_pkt,
                       unsigned len,
                       ChanTx_CB_Fn callback,
                       void *callback_data)
{
    DevError dev_error;
    p_Buffer buffer;

    /* adjust the device id, if required */
    if (devid == CH_DEFAULT_DEV)
    {
        devid = active_device;
    }

    ASSERT(devid < DI_NUM_DEVICES, ("Bad device ID %d\n", devid));
    ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));
    ASSERT(data_pkt != NULL, ("No data buffer\n"));
    ASSERT(callback != NULL, ("No callback function\n"));

    /* we can only service one request per channel in this version */

    /* adjust buffer back to start, length to include header */
    buffer = data_pkt - CB_CHAN_DATA_BYTE_POS;  /* includes link ptr  */
    len += CF_DATA_BYTE_POS;    /* excludes link ptr - just header */
    
    Angel_EnterCriticalSection();           /* start critical region */

    if (chan_state[chanid].writer != NULL)
    {
        Angel_LeaveCriticalSection();

        LogWarning(LOG_CHANNEL, ( "ChannelSendAsync: already busy on chan %d\n", chanid));

        angel_ChannelReleaseBuffer(data_pkt);
        return CE_BUSY;
    }

    INC(dev_state[devid].currentFrame);
    
    /* record what we need to know later */
    chan_state[chanid].writer = callback;
    chan_state[chanid].write_cb_data = callback_data;
    chan_state[chanid].write_buff = buffer;

    Angel_LeaveCriticalSection();          /* end critical region */

    buffer[CB_PACKET(CF_CHANNEL_BYTE_POS)] = chanid;
    buffer[CB_PACKET(CF_PACKET_SEQ_BYTE_POS)] = dev_state[devid].currentFrame;
    buffer[CB_PACKET(CF_ACK_SEQ_BYTE_POS)] = lastSEQ(dev_state[devid].frameExpected);
    buffer[CB_PACKET(CF_FLAGS_BYTE_POS)] = CF_RELIABLE;
    
    LogInfo(LOG_CHANNEL, ("ChannelSendAsync: packet packetseq %d ackseq %d\n",
              buffer[CB_PACKET(CF_PACKET_SEQ_BYTE_POS)],
              buffer[CB_PACKET(CF_ACK_SEQ_BYTE_POS)]));

    /* store the buffer in the unacked list */
    adp_add_pkt(devid, dev_state[devid].currentFrame, buffer, len);
    
    do
    {
        /* attempt to queue transmission */
        /* we use the channel id as callback data - simplifies write_callback */
        dev_error = angel_DeviceWrite(devid, buffer + CB_CHAN_HEADER_BYTE_POS,
                                      len, write_callback, (void *)chanid,
                                      DC_DBUG);

        if (dev_error == DE_BUSY)
        {
            Angel_Yield();
        }
    } while (dev_error == DE_BUSY);

    if (dev_error == DE_OKAY)
        return CE_OKAY;
    else
        return CE_DEV_ERROR;
}

static AdpErrs 
resend_packet_from(DeviceID devid, SequenceNR startpacket)
{
    DevError chan_err = DE_OKAY;
    ChannelID chanid;
    short len;
    ChanError send_error = CE_PRIVATE;
    p_Buffer buf;
    register struct DevState *dev = &dev_state[devid];
    register short i, j = LISTEND;
    SequenceNR thispacket = startpacket;
    
    Angel_EnterCriticalSection();

    while (chan_err == DE_OKAY &&
           adp_between(startpacket, thispacket, nextSEQ(dev->currentFrame)))
    {
        /* search for the requested packet in the resend list */
        for (i = dev->firstUsed; i != LISTEND; i = dev->unackedBufs[i].next)
        {        
            if (thispacket == dev->unackedBufs[i].packno)
                break;
        }
        
        if (i == LISTEND)
        {
            LogError(LOG_CHANNEL, ("resend_packet: packet %d not available (from %d, max %d)\n",
                                   thispacket, startpacket, dev->currentFrame));
            break;
        }        

        /* get the buffer details */
        buf = dev_state[devid].unackedBufs[i].buf;
        len = dev_state[devid].unackedBufs[i].len;        
        chanid = (ChannelID) * buf;
        
        Angel_LeaveCriticalSection();
        
        /*
         * set the current frameExpected, not the value when this frame was
         * originally sent
         */
        buf[CB_PACKET(CF_ACK_SEQ_BYTE_POS)] = lastSEQ(dev_state[devid].frameExpected);
        
        LogWarning(LOG_CHANNEL, ( "resend_packet: writing pkt 0x%x seq %d, chan %d, len %d\n",
                                  buf, thispacket, chanid, len));
        
        Angel_EnterCriticalSection();           /* start critical region */
        
        /* Check that a write is not already in progress; if so, wait... */
        /* now wait for the write to complete */
        while (chan_state[chanid].write_blocking && chan_state[chanid].writer != NULL)
        {
            Angel_Yield();
        }
        chan_state[chanid].write_blocking = TRUE;
        chan_state[chanid].write_buff = buf;
        chan_state[chanid].writer = write_unblock_callback;
        chan_state[chanid].write_cb_data = (void *)&send_error;
        
        Angel_LeaveCriticalSection();          /* end critical region */
        
        LogInfo(LOG_CHANNEL, ( "resend msg: writing...\n"));
        do
        {        
            chan_err = angel_DeviceWrite(devid, buf + CB_CHAN_HEADER_BYTE_POS, len,
                                         write_callback, (void *)chanid, DC_DBUG);
            if (chan_err == DE_BUSY);
            {
                Angel_Yield();
            }
        } while (chan_err == DE_BUSY);
        
        if (chan_err == DE_OKAY)
        {
            LogInfo(LOG_CHANNEL, ( "resend msg: %d ok\n", thispacket));
        }
        else
        {
            LogError(LOG_CHANNEL, ( "resend msg: packet %d FAILED ret code 0x%x\n",
                                    thispacket, chan_err));
        }
        /* now wait for the write to complete */
        LogInfo(LOG_CHANNEL, ( "resend msg: waiting for send to complete...\n"));
        
        while (chan_state[chanid].write_blocking)
        {
            Angel_Yield();
        }
        
        LogInfo(LOG_CHANNEL, ( "resend msg: send complete\n"));

        INC(thispacket);
        
        j = i;
        Angel_EnterCriticalSection();
    }
    Angel_LeaveCriticalSection();

    return adp_translate_dev_error(chan_err);
}


/*
 * Function: angel_ChannelRead
 *  Purpose: blocking read of a packet from a channel
 *
 * see channels.h for full description
 */

/* callback for this routine */
static void 
read_unblock_callback(DeviceID devid, ChannelID chanid,
                      p_Buffer buffer, unsigned len,
                      void *callback_data)
{
    ChanError chan_error;
    ChanError *read_error = (ChanError *) callback_data;

    ASSERT(chanid < CI_NUM_CHANNELS, ("Channel ID %d invalid in callback\n", chanid));
    ASSERT(chan_state[chanid].read_blocking == TRUE, ("Read not blocking in callback\n"));
    ASSERT(read_error != NULL, ("Nowhere to write error code in callback\n"));

    /* unregister the async read */
    chan_error = angel_ChannelReadAsync(devid, chanid, NULL, NULL);
    ASSERT(chan_error == CE_OKAY, ("Error unregistering async read in callback\n"));

    LogInfo(LOG_CHANNEL, ("read_unblock_callback: channel %d buffer %d len %d\n",
                          chanid, buffer, len));
    
    *read_error = CE_OKAY;
    chan_state[chanid].read_buffer = buffer;
    chan_state[chanid].read_len = len;
    chan_state[chanid].read_blocking = FALSE;
}

#ifdef NEED_CHANNEL_READ

ChanError 
angel_ChannelRead(DeviceID devid,
                  ChannelID chanid,
                  p_Buffer * buffer,
                  unsigned *len)
{
    ChanError chan_error;
    ChanError read_error = CE_PRIVATE;

    ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));

    /* Check that a read is not already in progress */
    Angel_EnterCriticalSection();
    if (chan_state[chanid].read_blocking)
    {
        Angel_LeaveCriticalSection();
        LogWarning(LOG_CHANNEL, ( "ChannelRead: already busy on chan %d\n", chanid));
        return CE_BUSY;
    }
    chan_state[chanid].read_blocking = TRUE;

    Angel_LeaveCriticalSection();

    /* Register an async read */
    chan_error = angel_ChannelReadAsync(devid, chanid,
                                        read_unblock_callback,
                                        (void *)&read_error);

    if (chan_error != CE_OKAY)
    {
        chan_state[chanid].read_blocking = FALSE;
        return chan_error;
    }

    /* now wait for the read to complete */
    while (chan_state[chanid].read_blocking)
    {
        TRACE("crb");
        Angel_Yield();
    }

    /* get at the results of the async read */
    if (read_error == CE_OKAY)
    {
        *buffer = chan_state[chanid].read_buffer;
        *len = chan_state[chanid].read_len;
    }

    if (read_error)
        LogWarning(LOG_CHANNEL, ( "ChannelRead: error code %d\n", read_error));

    return read_error;
}

#endif /* def NEED_CHANNEL_READ */

/*
 * Function: angel_ChannelReadAsync
 *  Purpose: asynchronous read of a packet via a channel
 *
 * see channels.h for full details
 */
ChanError 
angel_ChannelReadAsync(DeviceID devid,
                       ChannelID chanid,
                       ChanRx_CB_Fn callback,
                       void *callback_data)
{
    /* adjust the device id, if required */
    if (devid == CH_DEFAULT_DEV)
    {
        devid = active_device;
    }
    ASSERT(devid < DI_NUM_DEVICES, ("Bad device ID %d\n", devid));

    /* we can only read from the active device in this version */
    ASSERT(devid == active_device, ("ReadAsync device %d not active device %d\n",
                                    devid, active_device));

    ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));

    /*
     * we can only service one request per channel in this version,
     * OR unregister by specifying a NULL callback
     */

    Angel_EnterCriticalSection();           /* start critical region */

    if (chan_state[chanid].reader != NULL && callback != NULL)
    {
        Angel_LeaveCriticalSection();
        LogWarning(LOG_CHANNEL, ( "ChannelReadAsync: already reader on chan %d\n",
                                  chanid));
        return CE_BUSY;
    }

    /* record what we need to know later */
    chan_state[chanid].reader = callback;
    chan_state[chanid].read_cb_data = callback_data;

    Angel_LeaveCriticalSection();          /* end critical region */

    /* that's all we need to do */
    return CE_OKAY;
}


/*
 * Function: angel_ChannelReadAll
 *  Purpose: register an asynchronous read across all devices
 *
 * see channels.h for full details
 */

ChanError 
angel_ChannelReadAll(ChannelID chanid,
                     ChanRx_CB_Fn callback,
                     void *callback_data)
{
    ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));
    ASSERT(callback != NULL, ("No callback function in Read All\n"));

    if (read_all_callback != NULL || chan_state[chanid].reader != NULL)
    {
        LogWarning(LOG_CHANNEL, ( "ChannelReadAll: already reader on chan %d\n", chanid));
        return CE_BUSY;
    }

    /* just register the info */
    read_all_channel = chanid;

    read_all_callback = callback;
    read_all_cb_data = callback_data;

    return CE_OKAY;
}


/*
 * Function: angel_ChannelSendThenRead
 *  Purpose: blocking read of a packet from a channel
 *
 * see channels.h for full description
 */
ChanError 
angel_ChannelSendThenRead(DeviceID devid,
                          ChannelID chanid,
                          p_Buffer * buffer,
                          unsigned *len)
{
    ChanError chan_error;
    ChanError send_error = CE_PRIVATE;
    ChanError read_error = CE_PRIVATE;

#if DEBUG == 1
    unsigned int reason;
    unpack_message(BUFFERDATA(*buffer), "%w", &reason);
#endif
    
    ASSERT(chanid < CI_NUM_CHANNELS, ("Bad channel ID %d\n", chanid));    
    
    LogInfo(LOG_CHANNEL, ( "angel_ChannelSendThenRead: chan %d buffer %p, len %d [reason: %s]\n",
                           chanid, *buffer, *len, log_adpname(reason)));

    /* Check that nothing is already in progress */
    Angel_EnterCriticalSection();
    if (chan_state[chanid].read_blocking || chan_state[chanid].write_blocking)
    {
        Angel_LeaveCriticalSection();
        LogWarning(LOG_CHANNEL, ("ChannelSendThenRead: already busy on chan %d\n",
                                   chanid));
        angel_ChannelReleaseBuffer(*buffer);
        return CE_BUSY;
    }
    chan_state[chanid].read_blocking = TRUE;
    chan_state[chanid].write_blocking = TRUE;
    Angel_LeaveCriticalSection();

    /* Register an async read */
    chan_error = angel_ChannelReadAsync(devid, chanid,
                                        read_unblock_callback,
                                        (void *)&read_error);

    if (chan_error != CE_OKAY)
    {
        LogWarning(LOG_CHANNEL, ("ChannelSendThenRead: Can't register read "
                                   "callback for chan %d\n", chanid));
        angel_ChannelReleaseBuffer(*buffer);
        chan_state[chanid].read_blocking = FALSE;
        chan_state[chanid].write_blocking = FALSE;
        return chan_error;
    }

    /* Then start an async write */
    chan_error = angel_ChannelSendAsync(devid, chanid, *buffer, *len,
                                        write_unblock_callback,
                                        (void *)&send_error);

    if (chan_error != CE_OKAY)
    {
        ChanError unreg_error;

        LogWarning(LOG_CHANNEL, ("ChannelSendThenRead: Send failed, error %d "
                                   "for chan %d\n", chan_error, chanid));
        
        unreg_error = angel_ChannelReadAsync(devid, chanid, NULL, NULL);
        
        if (unreg_error != 0)
            LogWarning(LOG_CHANNEL, ("ChannelSendThenRead: Can't unregister read "
                                       "callback for chan %d, error %d\n", chanid, unreg_error));
        chan_state[chanid].read_blocking = FALSE;
        chan_state[chanid].write_blocking = FALSE;
        return chan_error;
    }

    /* Now wait for both to complete */
    while (chan_state[chanid].write_blocking
           || chan_state[chanid].read_blocking)
    {
        if (!chan_state[chanid].read_blocking)
            LogWarning(LOG_CHANNEL, ( "read packet before write complete\n"));

        TRACE("csr");
        Angel_Yield();
    }

    /* get at the results of the async read */
    if (read_error == CE_OKAY)
    {
        *buffer = chan_state[chanid].read_buffer;
        *len = chan_state[chanid].read_len;
#if DEBUG == 1
        unpack_message(BUFFERDATA(*buffer), "%w", &reason);
        LogInfo(LOG_CHANNEL, ( "angel_ChannelSendThenRead: Return buffer %p, len %d [reason: %s]\n",
                               *buffer, *len, log_adpname(reason)));
#endif
    }

    if (read_error)
        LogWarning(LOG_CHANNEL, ( "SendThenRead: read error code %d\n", read_error));
    if (send_error)
        LogWarning(LOG_CHANNEL, ( "SendThenRead: write error code %d\n", send_error));

    return (send_error ? send_error : read_error);
}


/*
 * Function: angel_ChannelSelectDevice
 *  Purpose: select the device to be used for all channel comms
 *
 * see channels.h for full details
 */

ChanError 
angel_ChannelSelectDevice(DeviceID device)
{
    unsigned int i;

    ASSERT(device < DI_NUM_DEVICES, ("Bad device ID %d\n", device));

    LogInfo(LOG_CHANNEL, ( "ChannelSelectDevice: %d\n", device));

    if (device == active_device)
        return CE_OKAY;         /* already correct */

    /* first we must do an abort on any blocking reads or writes */
    for (i = 0; i < CI_NUM_CHANNELS; ++i)
    {
        if (chan_state[i].read_blocking)
        {
            ChanError *read_error = (ChanError *) chan_state[i].read_cb_data;

            ASSERT(read_error != NULL, ("angel_ChannelSelectDevice: write_error NULL\n"));

            LogWarning(LOG_CHANNEL, ( "ABANDON read on channel %d\n", i));

            *read_error = CE_ABANDONED;
            if (chan_state[i].read_buffer != NULL)
                angel_ChannelReleaseBuffer(chan_state[i].read_buffer);
            chan_state[i].reader = NULL;
            chan_state[i].read_blocking = FALSE;
        }

        if (chan_state[i].write_blocking)
        {
            ChanError *write_error = (ChanError *) chan_state[i].write_cb_data;

            ASSERT(write_error != NULL, ("angel_ChannelSelectDevice: write_error NULL\n"));

            LogWarning(LOG_CHANNEL, ( "ABANDON write on channel %d\n", i));

            *write_error = CE_ABANDONED;
            chan_state[i].writer = NULL;
            chan_state[i].write_buff = NULL;
            chan_state[i].write_blocking = FALSE;
        }
    }

    active_device = device;

    return CE_OKAY;
}


/*
 * Function: angel_ChannelReadActiveDevice
 *  Purpose: reads the device id of the currently active device
 *
 * see channels.h for full details
 */

ChanError 
angel_ChannelReadActiveDevice(DeviceID * device)
{
    *device = active_device;
    return CE_OKAY;
}

/* EOF channels.c */
