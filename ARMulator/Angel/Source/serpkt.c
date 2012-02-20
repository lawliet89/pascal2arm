/* -*-C-*-
 *
 * $Revision: 1.6.6.7 $
 *   $Author: rivimey $
 *     $Date: 1998/03/21 22:54:10 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: High-level packet-based device driver for serial (or similar)
 *              devices.  Interfaces to low-level drivers via serring.h.
 */

#include "devdriv.h"            /* device driver support */
#include "rxtx.h"               /* rx/tx packet engines */
#include "logging.h"

#include "serring.h"

/* General purpose constants, macros, enums, typedefs */

/* Publically-accessible globals */
static unsigned long spk_rx_numpackets = 0;
static unsigned long spk_rx_numbadpackets = 0;
static unsigned long spk_rx_numdrops = 0;
static unsigned long spk_rx_numbytes = 0;

static unsigned long spk_tx_numpackets = 0;
static unsigned long spk_tx_numdrops = 0;
static unsigned long spk_tx_numbytes = 0;

struct StatInfo spk_stat_info[] =
{
    "Serpkt Stats:\n"
    " Transmit\n"
    "  NumPackets          %4d\n", &spk_tx_numpackets,
    "  NumPackets Dropped  %4d\n", &spk_tx_numdrops,
    "  Number of Bytes     %4d\n", &spk_tx_numbytes,
    "\n"
    " Receive\n"
    "  NumPackets          %4d\n", &spk_rx_numpackets,
    "  NumPackets Bad      %4d\n", &spk_rx_numbadpackets,
    "  NumPackets Dropped  %4d\n", &spk_rx_numdrops,
    "  Number of Bytes     %4d\n", &spk_rx_numbytes,
    NULL, NULL
};

/* Private globals */
/* none */

/*
 *  Function: serpkt_fill_tx_ring
 *   Purpose: Fill ups a transmit ring buffer from the Tx engine
 *
 *  Pre-conditions: Interrupts should be disabled
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *      In/Out: ring    the ring buffer to be filled
 *
 *   Returns: Nothing
 */
static void 
serpkt_fill_tx_ring(const DeviceID devid,
                    RingBuffer * const ring)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    unsigned int eod = *status & SER_TX_EOD; /* not bool -- it's a bitmask */

    /*
     * loop while data left to send and the buffer has space left
     */
    while ((eod == 0) && !ringBufFull(ring))
    {
        te_status tx_status;
        unsigned char new_ch;
        unsigned int s;

        /*
         * This task is running as AngelCallback priority so it can
         * be interrupted. If a heartbeat arrives which needs to write
         * an acknowledge, the interrupting heartbeat task will never
         * terminate (because we're busy); the result is deadlock.
         *
         * Interim Solution: Don't let the interrupt happen!
         */
        /* s = Angel_EnableInterruptsFromSVC(); */
        tx_status = Angel_TxEngine(SerEngine(devid)->tx_packet,
                                   SerEngine(devid)->te_state,
                                   &new_ch);
        /* Angel_RestoreInterruptsFromSVC(s); */

        switch (tx_status)
        {
            case TS_DONE_PKT:
                eod = TRUE;
                spk_tx_numpackets++;
                /* and FALL THROUGH TO */

            case TS_IN_PKT:
#if DEBUG == 1
                {
                    static int count = 0;
                    LogInfo(LOG_WIRE, (">%02X ", new_ch));
                    if (++count > 16)
                    {
                        LogInfo(LOG_WIRE, ("\n"));
                        count = 0;
                    }
                }
#endif
                ringBufPutChar(ring, new_ch);
                spk_tx_numbytes++;
                break;

            case TS_IDLE:
                LogWarning(LOG_SERPKT, ( "TS_IDLE in fill_ring\n"));
                break;
                
            default:
                LogError(LOG_SERPKT, ( "Unknown status in fill_ring\n"));
                break;
        }
    }

    if (eod)
        *status |= SER_TX_EOD;

    /*
     * if ring buffer was empty when we started filling it, then
     * we need to kick start the first character
     */
    if ((*status & SER_TX_KICKSTART) != 0)
    {
        *status |= SER_TX_DATA;
        *status &= ~SER_TX_KICKSTART;
        SerCtrl(devid)->control_tx(devid);
        SerCtrl(devid)->kick_start(devid);
    }
}

/*
 *  Function: serpkt_AsyncWrite
 *   Purpose: Entry point for asynchronous writes to the serial device.
 *            See documentation for angel_DeviceWrite in devdriv.h
 */
DevError 
serpkt_AsyncWrite(DeviceID devid, p_Buffer buffer,
                  unsigned int length, DevChanID devchan)
{
    struct data_packet *const txp = SerEngine(devid)->tx_packet;
    RingBuffer *ring = SerCtrl(devid)->tx_ring;

    txp->len = length;
    txp->type = devchan;
    txp->data = buffer;

    Angel_TxEngineInit(SerEngine(devid)->config, txp,
                       SerEngine(devid)->te_state);

    /* need to protect this bit */
    Angel_EnterSVC();
    angel_DeviceStatus[devid] |= SER_TX_KICKSTART;
    serpkt_fill_tx_ring(devid, ring);
    Angel_ExitToUSR();

    LogInfo(LOG_SERPKT, ( "serpkt_AsyncWrite OK\n"));
    return DE_OKAY;
}

/*
 *  Function: serpkt_RegisterRead
 *   Purpose: Entry point for asynchronous reads from the serial device.
 *            See documentation for angel_RegisterRead in devdriv.h
 */
DevError 
serpkt_RegisterRead(DeviceID devid, DevChanID devchan)
{
    struct data_packet *const rxp = SerEngine(devid)->rx_packet;

    IGNORE(devchan);

    LogInfo(LOG_SERPKT, ( "serpkt_RegisterRead\n"));

    Angel_EnterSVC();

    rxp->data = NULL;           /* gets set in callback later on */
    rxp->type = DC_NUM_CHANNELS;  /* invalid */
    SerCtrl(devid)->control_rx(devid);

    Angel_ExitToUSR();

    return DE_OKAY;
}

/*
 *  Function: serpkt_reset
 *   Purpose: Reset a serial port
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: DE_OKAY
 */
static DevError 
serpkt_reset(const DeviceID devid)
{
    Angel_RxEngineInit(SerEngine(devid)->config, SerEngine(devid)->re_state);

    /* get low-level control to do the rest */
    return SerCtrl(devid)->control(devid, DC_RESET, NULL);
}

/*
 *  Function: serpkt_rxflow
 *   Purpose: 
 *            
 */
static DevError 
serpkt_rxflow(DeviceID devid, void *v_arg)
{
    int oldarg;
    int arg = (int)v_arg;
    
    LogWarning(LOG_SERPKT, ( "serpkt_rxflow: dev %d arg %x\n", devid, arg));
    
    /*
     * arg == 0: no packets allowed;
     * arg > 0: normal processing
     * arg < 0: allow (-arg) packets (delivered) before disabling flow
     * Note: delivered packets exclude bad packets!
     */

    if (SerCtrl(devid)->rx_tx_state != NULL)
    {
        oldarg = SerCtrl(devid)->rx_tx_state->rx_pkt_flow;
        SerCtrl(devid)->rx_tx_state->rx_pkt_flow = (int)arg;

        /* This code is (I think ) only relevant in a system which can
         * handle multiple tasks... if in minimal angel, therefore,
         * just ignore it.
         *
         * ... I may be wrong!  RIC 1/98
         */
#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
        
        /* restart the rx engine if it was stopped */
        if (arg > 0 && oldarg == 0)
        {
            volatile unsigned int *const status = angel_DeviceStatus + devid;

            if ((*status & SER_RX_QUEUED) == 0)
            {
                int s;
                
                *status |= SER_RX_QUEUED;
                
                /* allow pending ints to be processed... */
                s = Angel_EnableInterruptsFromSVC();
                Angel_RestoreInterruptsFromSVC(s);
                
                LogWarning(LOG_SERPKT, ( "serpkt_rxflow: Queue rx_processing.\n"));
                /*
                 * This is what is wanted, but it can't be done here and now;
                 * if a packet were delivered as a result, we would end up
                 * reentering code which isn't reentrant!
                 *
                 * SerCtrl(devid)->rx_processing((void*)devid);
                 */                
                Angel_QueueCallback((angel_CallbackFn)SerCtrl(devid)->rx_processing,
                                    TP_AngelWantLock,
                                    (void *)devid, 0, 0, 0);
            }
            else
                LogWarning(LOG_SERPKT, ( "serpkt_rxflow: still running\n"));
        }
#endif
    }
    
    /* get low-level control to do the rest */
    return SerCtrl(devid)->control(devid, DC_RX_PACKET_FLOW, v_arg);
}

/*
 *  Function: serpkt_Control
 *   Purpose: Entry point for device control functions
 *            See documentation for angel_DeviceControl in devdriv.h
 */
DevError 
serpkt_Control(DeviceID devid,
               DeviceControl op,
               void *arg)
{
    DevError ret_code;

    LogInfo(LOG_SERPKT, ( "serpkt_Control: dev %d, op %d arg %x\n", devid, op, arg));

    Angel_EnterSVC();

    /* we only implement the special cases here */
    switch (op)
    {
        case DC_INIT:
            if (SerCtrl(devid)->rx_tx_state)
                SerCtrl(devid)->rx_tx_state->rx_pkt_flow = 1;
            ret_code = SerCtrl(devid)->control(devid, op, arg);
            break;
            
        case DC_RESET:
            ret_code = serpkt_reset(devid);
            break;

        case DC_RX_PACKET_FLOW:
            ret_code = serpkt_rxflow(devid, arg);
            break;
            
        default:
            /* all others passed through */
            ret_code = SerCtrl(devid)->control(devid, op, arg);
            break;
    }

    Angel_ExitToUSR();

    return ret_code;
}

/*
 *  Function: serpkt_int_rx_processing
 *   Purpose: Deferred Rx processing for interrupt handler
 *
 *    Params: (via args)
 *       Input: devid           device ID of the driver
 *
 *   Returns: Nothing
 */
void 
serpkt_int_rx_processing(void *args)
{
    const DeviceID devid = (DeviceID) args;
    RingBuffer *ring = SerCtrl(devid)->rx_ring;
    volatile int *rx_flow = &(SerCtrl(devid)->rx_tx_state->rx_pkt_flow);
    volatile unsigned int *status = angel_DeviceStatus + devid;
    struct data_packet *rxp = SerEngine(devid)->rx_packet;
    unsigned int s;
    
    ASSERT(devid < DI_NUM_DEVICES, ("devid"));

    LogInfo(LOG_RX, ("Entry: count = %d\n", ringBufCount(ring)));
    
    /* 
     * We need to disable IRQ & FIQ whenever we modify critical data 
     * shared with the interrupt handler
     */
    s = Angel_DisableInterruptsFromSVC();

    /*
     * If there are characters in the rx ring buffer, process them
     * through the rx engine and take appropriate action.
     */
    while (ringBufNotEmpty(ring))
    {
        re_status rx_eng_status;
        unsigned char new_ch;
        unsigned char overrun;

        /* if we've been stopped, get out*/
        if (*rx_flow == 0)
        {
            LogWarning(LOG_RX, ("Packet processing postponed; rx_flow == 0, ring->count = %d\n",
                                ringBufCount(ring)));
            break;
        }
        
        overrun = ringBufIsOvr(ring);
        if (overrun)
        {
            LogWarning(LOG_RX, ("Character Overflow detected, ring->count = %d\n",
                                ringBufCount(ring)));
            ringBufResetOvr(ring);
        }
        new_ch = ringBufGetChar(ring);

        Angel_RestoreInterruptsFromSVC(s);

#if DEBUG == 1  /* don't want 'count' maintained in release versions */
        {
            static int count = 0;
            LogInfo(LOG_WIRE, ("<%02X ", new_ch));
            if (++count > 16)
            {
                LogInfo(LOG_WIRE, ("\n"));
                count = 0;
            }
        }
#endif
        
        rx_eng_status = Angel_RxEngine(new_ch, overrun, rxp,
                                       SerEngine(devid)->re_state);

        switch (rx_eng_status)
        {
            case RS_WAIT_PKT:
            case RS_IN_PKT:
                /* we've done all we need within the engine */
                break;

            case RS_BAD_PKT:
                /* tell the channels layer about a bad packet */
                spk_rx_numbadpackets++;
                
                if (rxp->type < DC_NUM_CHANNELS)
                {
                    LogWarning(LOG_SERPKT, ( "Bad packet: Chan %d; delivering...\n", rxp->type));
        
                    angel_DD_GotPacket(devid, rxp->data,
                                       rxp->len, DS_BAD_PACKET,
                                       rxp->type);
                }
                else
                {
                    LogWarning(LOG_SERPKT, ( "Bad packet: can't deliver...\n"));

                    /*
                     * we assume that if type is bad, no storage has been
                     * allocated in the engine, so just do this to be safe:
                     */
                    if (rxp->data != NULL)
                    {
                        LogInfo(LOG_SERPKT, ("Bad packet's data non-null, freeing..."));
                        Angel_BufferRelease(rxp->data);
                    }
                    spk_rx_numdrops++;
                }
                /* I think now bad packets shouldn't count towards incrementing the
                 * flow; when the flow is set negative, we are really saying "let this
                 * many _good_ packets through.
                 * 
                 * if (*rx_flow < 0)
                 *  (*rx_flow)++;
                 */
                rxp->data = NULL;
                rxp->type = DC_NUM_CHANNELS;
                break;
                
            case RS_NO_BUF:
                LogWarning(LOG_SERPKT, ( "No buffer available: can't deliver...\n"));
                spk_rx_numdrops++;
                
                /*
                 * See msg above. 
                 * if (*rx_flow < 0)
                 *  (*rx_flow)++;
                 */
                /*
                 * We've done all we can do within the engine. Don't
                 * try to deliver this; it can cause havoc with task
                 * priorities and anyway there's little point... ric
                 */
                rxp->data = NULL;
                rxp->type = DC_NUM_CHANNELS;  /* invalid */
                break;

            case RS_GOOD_PKT:
                spk_rx_numbytes += rxp->len;
                spk_rx_numpackets ++;
                
                if (*rx_flow < 0)
                    (*rx_flow)++;
                
                LogInfo(LOG_SERPKT, ( "Delivering good packet:\n"));
                
                angel_DD_GotPacket(devid, rxp->data, rxp->len,
                                   DS_DONE, rxp->type);
                rxp->data = NULL;
                rxp->type = DC_NUM_CHANNELS;  /* invalid */
                break;
        }

        s = Angel_DisableInterruptsFromSVC();  /* for next test */
    }

    /*
     * need to clear the flag to say we have been here
     */
    *status &= ~SER_RX_QUEUED;

    /*
     * call rx control to reenable interrupts in case they were disabled
     */
    SerCtrl(devid)->control_rx(devid);

    /*
     * remember to reenable interrupts before we leave
     */
    Angel_RestoreInterruptsFromSVC(s);
    
    LogInfo(LOG_RX, ("Exit: count = %d\n", ringBufCount(ring)));
}

/*
 *  Function: serpkt_int_tx_processing
 *   Purpose: Deferred Tx processing for interrupt handler
 *
 *    Params: (via args)
 *       Input: devid           device ID of the driver
 *
 *   Returns: Nothing
 */
void 
serpkt_int_tx_processing(void *args)
{
    const DeviceID devid = (DeviceID) args;
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    RingBuffer *ring = SerCtrl(devid)->tx_ring;
    struct data_packet *txp = SerEngine(devid)->tx_packet;
    unsigned int s;

    ASSERT(devid < DI_NUM_DEVICES, ("serpkt_int_tx_processing: bad devid %d", devid));

    /* 
     * We need to disable IRQ & FIQ whenever we modify critical data
     * shared with the interrupt handler
     */
    s = Angel_DisableInterruptsFromSVC();

    /*
     * If we are called with SER_TX_EOD set then we need to handle
     * TX completion callback 
     */
    if (((*status & SER_TX_EOD) != 0) && ringBufEmpty(ring))
    {
        *status &= ~SER_TX_EOD;

        Angel_RestoreInterruptsFromSVC(s);

        angel_DD_SentPacket(devid, txp->data, txp->len, DS_DONE, txp->type);

        s = Angel_DisableInterruptsFromSVC();
    }

    /* need to clear the flag to say we have been here */
    *status &= ~SER_TX_QUEUED;

    /*
     * If there is space in the tx ring buffer, fill it up
     * as far as possible.
     */
    if ((*status & SER_TX_DATA) != 0)
    {
        serpkt_fill_tx_ring(devid, ring);
    }
    
    /* remember to reenable interrupts before we leave */
    Angel_RestoreInterruptsFromSVC(s);
}

/**********************************************************************/

/*
 *  Function: serpkt_flow_control
 *   Purpose: Flow control handler called by Rx engine when it gets an
 *              XON or XOFF
 *
 *  Pre-conditions:
 *
 *    Params:
 *       Input: fc_char         the Rx character
 *
 *              cb_data         device ID of the driver
 *
 *   Returns: Nothing
 */
void 
serpkt_flow_control(char fc_char, void *cb_data)
{
    const DeviceID devid = (DeviceID) cb_data;
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    unsigned int s = Angel_DisableInterruptsFromSVC();

    ASSERT(devid < DI_NUM_DEVICES, ("serpkt_flow_control: bad devid %d", devid));

    if (fc_char == serial_XON)
    {
        *status &= ~SER_TX_CONTROLLED_OFF;
        SerCtrl(devid)->control_tx(devid);
    }
    else if (fc_char == serial_XOFF)
    {
        *status |= SER_TX_CONTROLLED_OFF;
        SerCtrl(devid)->control_tx(devid);
    }

    Angel_RestoreInterruptsFromSVC(s);
}

/* EOF serpkt.c */
