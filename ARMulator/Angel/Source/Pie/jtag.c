/* -*-C-*-
 *
 * $Revision: 1.5.2.5 $
 *   $Author: rivimey $
 *     $Date: 1998/03/11 10:36:02 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: JTAG device driver for EICE (PIE) board
 */

/*
 * Provide a single serial device.  Since we're only providing a single
 * device we don't need to worry about device id's and idents.  But these
 * could be used to provide for two or more devices via the one driver.
 */

#include "jtag.h"               /* header for this file */

#include "channels.h"           /* for getting buffers */
#include "icegenop.h"           /* description of iceman interface */
#include "armiceop.h"           /* Needed for DCC polling, read and write */
#include "params.h"             /* parameter structures and utilities */
#include "rxtx.h"               /* rx/tx packet engines */
#include "logging.h"
#include "serlock.h"            /* serialisation, etc. */
#include "logging.h"
#include "ringbuff.h"

/* General purpose constants, macros, enums, typedefs */

/* Flags stored in serial_status */

#define JTAG_RX_DISABLED       (1<<9)  /* rx disabled and XOFF sent */

#define JTAG_TX_DATA           (1<<10) /* true if tx'ing packet */
#define JTAG_TX_FLOW_CONTROL   (1<<11) /* true if need to tx flow char */
#define JTAG_TX_CONTROLLED_OFF (1<<12) /* true if disabled by XOFF */
#define JTAG_TX_EOD            (1<<14) /* end of data pkt is in ring buffer */

#define JTAG_RX_QUEUED         (1<<15) /* deferred rx processing is queued */
#define JTAG_TX_QUEUED         (1<<16) /* deferred tx processing is queued */
#define JTAG_TX_KICKSTART      (1<<17) /* need to kickstart packet Tx      */

/* Forward declarations */

static DevError jtag_Write(DeviceID, p_Buffer, unsigned, DevChanID);
static DevError jtag_RegisterRead(DeviceID, DevChanID );
static DevError jtag_Control(DeviceID, DeviceControl, void *);
static void     jtag_flow_control(char fc_char, void *cb_data); 

/* Publically-accessible globals */
static unsigned long jtag_rx_numpackets = 0;
static unsigned long jtag_rx_numbadpackets = 0;
static unsigned long jtag_rx_numdrops = 0;
static unsigned long jtag_rx_numbytes = 0;

static unsigned long jtag_tx_numpackets = 0;
static unsigned long jtag_tx_numdrops = 0;
static unsigned long jtag_tx_numbytes = 0;

struct StatInfo jtag_stat_info[] =
{
    "JTAG Stats:\n"
    " Transmit\n"
    "  NumPackets          %4d\n", &jtag_tx_numpackets,
    "  NumPackets Dropped  %4d\n", &jtag_tx_numdrops,
    "  Number of Bytes     %4d\n", &jtag_tx_numbytes,
    "\n"
    " Receive\n"
    "  NumPackets          %4d\n", &jtag_rx_numpackets,
    "  NumPackets Bad      %4d\n", &jtag_rx_numbadpackets,
    "  NumPackets Dropped  %4d\n", &jtag_rx_numdrops,
    "  Number of Bytes     %4d\n", &jtag_rx_numbytes,
    NULL, NULL
};

/* Publically-accessible globals */

/*
 * This is the device table entry for this device
 */
const struct angel_DeviceEntry angel_JTAGDevice =
{
    DT_ANGEL,
    {
        jtag_Write,
        jtag_RegisterRead
    },
    jtag_Control, 
    (void *)JTAG_DEV_IDENT_0,   /* not used - only one device */
    NULL,
    NULL
};

/* Private globals */

/*
 * The state of the jtag driver.
 *
 * As we only have one device it makes sense to use a single local
 * global repository for device state flags.
 */
static unsigned jtag_status;
static int rx_flow = 1; /* see poll()/init() functions */

/* the configuration needed by the TX and RX engines */

#define SERIAL_FC_SET  0
#define SERIAL_CTL_SET ((1<<serial_STX)|(1<<serial_ETX)|(1<<serial_ESC))
#define SERIAL_ESC_SET (SERIAL_FC_SET|SERIAL_CTL_SET)

static const struct re_config engine_config =
{
    serial_STX, serial_ETX, serial_ESC, /* self-explanatory?               */
    SERIAL_FC_SET,                      /* set of flow-control characters  */
    SERIAL_ESC_SET,                     /* set of characters to be escaped */
    jtag_flow_control, (void *)DI_JTAG,        /* what to do with FC chars */
    angel_DD_RxEng_BufferAlloc, (void *)DI_JTAG     /* how to get a buffer */
};

/* the state of the rx engine */
static struct re_state rx_engine_state;

/* the state of the tx engine */
static struct te_state tx_engine_state;

/* packet for actual rx in progress */
static struct data_packet  rx_packet;

/* the current write packet */
static struct data_packet  tx_packet;

/* a static to hold the state of the icegenop module */
static ICE_GenericOperationsState *ICE_state;

/* a static to hold the state of the armiceop module */
static ARMIceOp_State *ARMIceOp_state;

/* the state shared between the interrupt handler and the deferred processor */
static struct RingBuffer jtag_rx_ring;
static struct RingBuffer jtag_tx_ring;

static void     jtag_fill_tx_ring( void );

/* 
 * Implementation of asynchronous write for jtag driver
 *
 * on entry, devid and devchan are guaranteed in range, and the device
 * DEV_WRITE_BUSY flag is guaranteed to have been clear (but now set).
 */
static DevError jtag_Write(DeviceID devid,
                           p_Buffer buffer, unsigned length,
                           DevChanID devchan)
{
    IGNORE( devid );

    LogInfo(LOG_JTAG, ( "jtag_Write\n" ));

    tx_packet.len     = length;
    tx_packet.type    = devchan;
    tx_packet.data    = buffer;
    
    Angel_TxEngineInit(&engine_config, &tx_packet, &tx_engine_state);
    
    jtag_status |= JTAG_TX_KICKSTART;
    jtag_fill_tx_ring();  /* which will send 1st 4 chars */

    return DE_OKAY;
}


/* 
 * implementation of asynchronous read registration for serial driver
 *
 * on entry, devid and devchan are guaranteed in range, and the device
 * DEV_READ_BUSY(devchan) flag is guaranteed to have been clear (but now set).
 */
static DevError jtag_RegisterRead(DeviceID devid, DevChanID devchan )
{
    IGNORE( devid );
    IGNORE( devchan );

    LogInfo(LOG_JTAG, ( "jtag_RegisterRead\n" ));

    rx_packet.data = NULL;       /* gets set in callback later on */
    rx_packet.type = DC_NUM_CHANNELS; /* invalid */

    return DE_OKAY;
}

/* This is brought in from arm7dt.c, and is the config data for Thumb */
extern ProcessorDesc ARM7DT_procdesc;


/*
 * Reset control operation
 */
static DevError jtag_reset(DeviceID devid)
{
    IGNORE(devid);

    /* reset flags */
    angel_DeviceStatus[devid] &= ~ (DEV_READ_BUSY_MASK | DEV_WRITE_BUSY);
    jtag_status = 0;
   
    Angel_RxEngineInit(&engine_config, &rx_engine_state);

    return DE_OKAY;
}


/*
 * Initialisation control operation
 */
static DevError jtag_init(DeviceID devid)
{
    ICE_Error err;
    IGNORE(devid);

    /*
     * do one-time start-up initialisation
     * (for this device, just a device reset)
     */

    jtag_reset(devid);

    err = ICE_GenericOperationsInitialise(NULL, &ARM7DT_procdesc,
                                          NULL, 0, &ICE_state);

    ICE_ReturnARMIceOpState(ICE_state, &ARMIceOp_state);

    if (err != IErr_NoError) return DE_BAD_OP;
    return DE_OKAY;
}




/*
 * Receive Mode control operation
 */
static DevError jtag_recv_mode(DeviceID devid, DevRecvMode mode)
{
    DevError ret_code = DE_OKAY;

    IGNORE( devid );

    if ( mode == DR_DISABLE )
    {
        if ( ! (jtag_status & JTAG_RX_DISABLED) )
        {
            /* disable reception and schedule send of XOFF */
            jtag_status |= (JTAG_RX_DISABLED | JTAG_TX_FLOW_CONTROL);
        }
    }
    else if ( mode == DR_ENABLE )
    {
        if ( jtag_status & JTAG_RX_DISABLED )
        {
            /* enable reception and schedule send of XON */
            jtag_status &= ~JTAG_RX_DISABLED;
            jtag_status |= JTAG_TX_FLOW_CONTROL;
        }
    }
    else
    {
        ret_code = DE_INVAL;
    }
    
    return ret_code;
}


/*
 *  Function: jtag_Control
 *   Purpose: To implement the Angel_DeviceControl function for JTAG;
 *     this is called to initialise, reset , etc, the device drivers Angel is using.
 *     Each operation uses it's own function.
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
static DevError jtag_Control(DeviceID devid, 
                             DeviceControl op, void *arg)
{
    DevError ret_code;

    LogInfo(LOG_JTAG, ( "jtag_Control: op %d arg %x\n", op, arg ));

    /* we only implement the standard ones here */
    switch( op )
    {
        case DC_INIT:          
            ret_code = jtag_init(devid);
            break;

        case DC_RESET:         
            ret_code = jtag_reset(devid);
            break;

        case DC_RECEIVE_MODE:  
            ret_code = jtag_recv_mode(devid, (DevRecvMode)((int)arg));
            break;

        case DC_RX_PACKET_FLOW:
            LogInfo(LOG_JTAG, ( "jtag_Control: set rx_flow to %d (was %d)\n",
                                (int) arg, rx_flow));
            rx_flow = (int)arg;
            ret_code = DE_OKAY;
            break;
            
        case DC_SET_PARAMS:     
            ret_code = DE_OKAY; /* not implemented */
            break;

        default:               
            ret_code = DE_BAD_OP;
            break;
    }

    return ret_code;
}


/*
 *  Function: jtag_fill_tx_ring
 *   Purpose: Fill up the transmit ring buffer from the tx engine
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
static void jtag_fill_tx_ring( void )
{
    bool eod = jtag_status & JTAG_TX_EOD;
    static int count = 0;

    TRACE( "sft" );

    while ( ! eod && ! ringBufFull( &jtag_tx_ring ) )
    {
        te_status      tx_status;
        unsigned char  new_ch;

        tx_status = Angel_TxEngine( &tx_packet, &tx_engine_state, &new_ch );

        switch ( tx_status )
        {
            case TS_DONE_PKT:
                jtag_tx_numpackets++;
                eod = TRUE;
                /* and FALL THROUGH TO */
                    
            case TS_IN_PKT:
                LogInfo(LOG_WIRE, (">%02X ", new_ch));
                if (++count > 16)
                {
                    LogInfo(LOG_WIRE, ("\n"));
                    count = 0;
                }
                
                ringBufPutChar( &jtag_tx_ring, new_ch );
                jtag_tx_numbytes++;
                break;
          
            case TS_IDLE:
                LogWarning(LOG_JTAG, ( "TS_IDLE in fill_ring\n" ));
                break;
        }
    }

    if ( eod )
        jtag_status |= JTAG_TX_EOD;

    /*
     * if ring buffer was empty when we started filling it, then
     * we need to kick start the first character
     */
    if ( (jtag_status & JTAG_TX_KICKSTART) != 0 )
    {
        int totgt_ready, fromtgt_ready;
      
        ARMIceOp_DCCPoll(ARMIceOp_state, &fromtgt_ready, &totgt_ready);
        if (totgt_ready)
        {
            int i;
            unsigned data_word;
            
            LogInfo(LOG_JTAG, ("Performing JTAG Write Kickstart\n"));

            jtag_status |= JTAG_TX_DATA;
            jtag_status &= ~JTAG_TX_KICKSTART;

            for (data_word=0,i=0; i<32; i+=8)
            {
                data_word |= ((ringBufGetChar( &jtag_tx_ring ) & 0xff) << i);
                if (ringBufFull(&jtag_tx_ring))
                    break;
            }
            if (i!=32)
            {
                LogError(LOG_JTAG, ("jtag_fill_tx_ring - KICKSTART failed - not enough data !"));
            }
            ARMIceOp_DCCWriteData(ARMIceOp_state, data_word);
        }
    }
}


/*
 *  Function: angel_JTAG_deferred_rx
 *   Purpose: deferred receive processing for JTAG device
 *
 *    Params:
 *       Input: none
 *
 *   Returns: none.
 */
static void angel_JTAG_deferred_rx(void)
{
    static int count = 0;
    
    /*
     * If there are characters in the rx ring buffer, process them
     * through the rx engine and take appropriate action.
     */
    while (ringBufNotEmpty( &jtag_rx_ring ))
    {
        re_status rx_eng_status;
        unsigned char new_ch;
        unsigned char overrun;
        
        /* if we've been stopped, get out*/
        if (rx_flow == 0)
        {
            LogWarning(LOG_JTAG, ("Packet processing postponed; rx_flow == 0, ring->count = %d\n",
                                  ringBufCount(&jtag_rx_ring)));
            break;
        }

        overrun = ringBufIsOvr(&jtag_rx_ring);
        if (overrun)
        {
            LogWarning(LOG_JTAG, ("Character Overflow detected, ring->count = %d\n",
                                  ringBufCount(&jtag_rx_ring)));
            ringBufResetOvr(&jtag_rx_ring);
        }
        new_ch = ringBufGetChar( &jtag_rx_ring );

        LogInfo(LOG_WIRE, ("<%02X ", new_ch));
        if (++count > 16)
        {
            LogInfo(LOG_WIRE, ("\n"));
            count = 0;
        }
        
        rx_eng_status = Angel_RxEngine( new_ch, overrun, &rx_packet, &rx_engine_state );

        switch( rx_eng_status )
        {
            case RS_WAIT_PKT:
            case RS_IN_PKT:
                /* we've done all we need within the engine */
                break;

            case RS_BAD_PKT:
                jtag_rx_numbadpackets++;
                /* tell the channels layer about a bad packet */

                if (rx_packet.type < DC_NUM_CHANNELS)
                {
                    LogWarning(LOG_JTAG, ( "Bad packet: Chan %d; delivering...\n", rx_packet.type));
                    angel_DD_GotPacket(DI_JTAG, rx_packet.data,
                                       rx_packet.len, DS_BAD_PACKET,
                                       rx_packet.type);
                }
                else
                {
                    LogWarning(LOG_JTAG, ( "Bad packet: can't deliver...\n"));

                    /*
                     * we assume that if type is bad, no storage has been
                     * allocated in the engine, so just do this to be safe:
                     */
                    if (rx_packet.data != NULL)
                    {
                        LogInfo(LOG_JTAG, ("Bad packet's data non-null, freeing..."));
                        Angel_BufferRelease(rx_packet.data);
                    }
                    jtag_rx_numdrops++;
                }
                /* I think now bad packets shouldn't count towards incrementing the
                 * flow; when the flow is set negative, we are really saying "let this
                 * many _good_ packets through.
                 * 
                 * if (rx_flow < 0)
                 *  (rx_flow)++;
                 */
                rx_packet.data = NULL;
                rx_packet.type = DC_NUM_CHANNELS;
                break;

            case RS_NO_BUF:
                LogWarning(LOG_JTAG, ( "No buffer available: can't deliver...\n"));
                jtag_rx_numdrops++;
                
                /*
                 * See msg above. 
                 * if (rx_flow < 0)
                 *  (rx_flow)++;
                 */
                /*
                 * We've done all we can do within the engine. Don't
                 * try to deliver this; it can cause havoc with task
                 * priorities and anyway there's little point... ric
                 */
                rx_packet.data = NULL;
                rx_packet.type = DC_NUM_CHANNELS;
                break;

            case RS_GOOD_PKT:
                jtag_rx_numbytes += rx_packet.len;
                jtag_rx_numpackets ++;
                
                if (rx_flow < 0)
                    (rx_flow)++;
                
                LogInfo(LOG_JTAG, ( "Delivering good packet:\n"));
                
                angel_DD_GotPacket(DI_JTAG, rx_packet.data,
                                   rx_packet.len, DS_DONE,
                                   rx_packet.type);
                rx_packet.data = NULL;
                rx_packet.type = DC_NUM_CHANNELS;
                break;
        }
    }
    /* need to clear the flag to say we have been here */
    jtag_status &= ~JTAG_RX_QUEUED;
}


/*
 * deferred transmit processing for JTAG driver
 */
static void angel_JTAG_deferred_tx(void)
{
    TRACE( "stp" );

    /*
     * If we are called with JTAG_TX_EOD set then we need to handle
     * TX completion callback 
     */

    if ((jtag_status & JTAG_TX_EOD) && ringBufEmpty(&jtag_tx_ring))
    {
        jtag_status &= ~JTAG_TX_EOD;

        angel_DD_SentPacket(DI_JTAG, tx_packet.data, tx_packet.len,
                            DS_DONE, tx_packet.type);
    }

    /* need to clear the flag to say we have been here */
    jtag_status &= ~JTAG_TX_QUEUED;
}


/* 
 * read and write poll handler for jtag driver
 */
void angel_JTAGPollHandler( unsigned data )
{
    int totgt_ready, fromtgt_ready, i;
    unsigned data_word;

    IGNORE( data );

    /* See if there is anything we can do from the D.C.C.'s point of view */
    ARMIceOp_DCCPoll(ARMIceOp_state, &fromtgt_ready, &totgt_ready);
    
    /* Deal with data arriving */
    if (fromtgt_ready)
    {
        /* if we've been disabled from returning packets, don't ask for them! */
        if (rx_flow != 0)
        {
            bool queue_rx_deferred = FALSE;

            LogInfo(LOG_JTAG, ("jtag device: Data ready from target\n"));

            /* Place the characters into a ring-buffer and queue processing */
            ARMIceOp_DCCReadData(ARMIceOp_state, &data_word);

            for (i = 0; i < 32 && !ringBufFull(&jtag_rx_ring); i += 8)
            {
                char rx_ch = (char) ((data_word>>i) & 0xFF);
                
                if (ringBufCount(&jtag_rx_ring) < RING_SIZE-1)
                {
                    ringBufPutChar(&jtag_rx_ring, rx_ch);

                    /* If this an ETX then ignore the rest of the word */
                    if (rx_ch == serial_ETX)
                    {
                        queue_rx_deferred = TRUE;
                        break;
                    }
                }
                else if (i != 24) /* if there is another character to read in */
                {
                    ringBufSetOvr(&jtag_rx_ring);
                    ringBufPutChar(&jtag_rx_ring, rx_ch);
                    break;
                }
            }
            
            if (ringBufCount(&jtag_rx_ring) >= RING_HIGH_WATER_MARK)
            {
                /* note that we need to do deferred rx processing */
                queue_rx_deferred = TRUE;
            }
    
            /* see if we need to do deferred rx processing */
            if (queue_rx_deferred && !(jtag_status & JTAG_RX_QUEUED) )
            {
                jtag_status |= JTAG_RX_QUEUED;
                angel_JTAG_deferred_rx();
            }
        }
        else
            LogWarning(LOG_JTAG, ("do_poll: rx_flow == 0 with data available\n"));
    }
    
    /* Deal with sending data */
    if (totgt_ready)
    {
        bool queue_tx_deferred = FALSE;
    
        /* If we have reached the low-water mark in the ring buffer then it
         * is time to ask for a refill, as long as there is more of the
         * packet to come
         */
        if ((ringBufCount(&jtag_tx_ring) < RING_LOW_WATER_MARK) &&
            ((jtag_status & JTAG_TX_EOD) == 0) &&
            ((jtag_status & JTAG_TX_DATA) != 0)  )
        {
            jtag_fill_tx_ring();
        }
        
        /* If there is anything left in the ring buffer then send it out */
        if (ringBufNotEmpty(&jtag_tx_ring))
        {
            char tx_ch;
            
            LogInfo(LOG_JTAG, ("jtag device: target ready for waiting data from host\n"));

            /*
             * The fill_tx_ring above should ensure that, at this point, if there are
             * only a few bytes in the ring, the last is in fact an ETX character,
             * signalling end of packet.
             */
            for (data_word = 0, i = 0; i < 32; i += 8)
            {
                tx_ch = ringBufGetChar(&jtag_tx_ring);
                
                data_word |= ((tx_ch & 0xff) << i);
                if (ringBufEmpty(&jtag_tx_ring))
                {
                    if (tx_ch != serial_ETX)
                        LogError(LOG_JTAG, ("ARMIceOp_DCC_Poll: ring buf empty "
                                            "but last char not ETX (%x)\n", tx_ch));
                    break;
                }
            }

            LogInfo(LOG_JTAG, ("ARMIceOp_DCC_Poll: write %8X", data_word));

            ARMIceOp_DCCWriteData(ARMIceOp_state, data_word);

            /* If everything in the buffer has been sent and we have reached
             * the end of the packet then it must be time to make the
             * packet sent callback
             */
            if (ringBufEmpty(&jtag_tx_ring))
            {
                if (jtag_status & JTAG_TX_EOD)
                {
                    queue_tx_deferred = TRUE;
                    jtag_status &= ~JTAG_TX_DATA;
                }
                else
                {
                    /*
                     * queue_tx_deferred will already be set, but we need to set
                     * a flag to tell fill_tx_ring that it needs to restart
                     * character Tx.
                     */
                    jtag_status |= JTAG_TX_KICKSTART;
                }
            }

            /* We may need to do deferred tx processing */
            if ( queue_tx_deferred && !(jtag_status & JTAG_TX_QUEUED) )
            {
                jtag_status |= JTAG_TX_QUEUED;
                angel_JTAG_deferred_tx();
            }
        }
    }
}

/*
 * flow control handler called by rx engine when it gets XON or XOFF
 */
static void jtag_flow_control( char fc_char, void *cb_data )
{
    /* DeviceID devid = (DeviceID)cb_data; */
    IGNORE( cb_data );

    if ( fc_char == serial_XON )
    {
        jtag_status &= ~JTAG_TX_CONTROLLED_OFF;
    }
    else if (fc_char == serial_XOFF )
    {
        jtag_status |= JTAG_TX_CONTROLLED_OFF;
    }
}


/* EOF jtag.c */
