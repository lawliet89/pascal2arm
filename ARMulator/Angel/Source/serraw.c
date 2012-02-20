/* -*-C-*-
 *
 * $Revision: 1.7.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:54:51 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: High-level raw device driver for serial (or similar) devices.
 *              Interfaces to low-level drivers via serring.h.
 */

#include "devappl.h"            /* application device support */
#include "devdriv.h"            /* device driver support */
#include "logging.h"

#include "serring.h"


/* General purpose constants, macros, enums, typedefs */

/* Publically-accessible globals */
/* none */

/* Private globals */
/* none */

/*
 *  Function: serraw_fill_tx_ring
 *   Purpose: Fill up a transmit ring buffer from the Tx engine
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
serraw_fill_tx_ring(const DeviceID devid,
                    RingBuffer * const ring)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    RawState *const raw_state = SerCtrl(devid)->raw_state;

    bool eod = *status & SER_TX_EOD;

    while ((eod == 0) && !ringBufFull(ring))
    {
        ringBufPutChar(ring, raw_state->tx_data[raw_state->tx_n_done++]);
        eod = (raw_state->tx_n_done == raw_state->tx_length);
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
 *  Function: serraw_Write
 *   Purpose: Entry point for blocking writes to the serial device.
 *            See documentation for angel_DeviceWrite in devdriv.h
 */
DevError 
serraw_Write(DeviceID devid, p_Buffer buffer, unsigned int length)
{
    RingBuffer *const ring = SerCtrl(devid)->tx_ring;
    RawState *const raw_state = SerCtrl(devid)->raw_state;

    LogInfo(LOG_SERRAW, ( "serraw_Write\n"));

    if (raw_state->tx_data != NULL)
        return DE_BUSY;

    raw_state->tx_data = buffer;
    raw_state->tx_length = length;
    raw_state->tx_n_done = 0;

    /* need to protect this bit */
    Angel_EnterSVC();
    angel_DeviceStatus[devid] |= SER_TX_KICKSTART;
    serraw_fill_tx_ring(devid, ring);
    Angel_ExitToUSR();

    while (raw_state->tx_data != NULL)
        Angel_ApplDeviceYield();

    /* clear busy flag used by higher levels */
    Angel_EnterSVC();
    angel_DeviceStatus[devid] &= ~DEV_WRITE_BUSY;
    Angel_ExitToUSR();

    return DE_OKAY;
}

/*
 *  Function: serraw_Read
 *   Purpose: Entry point for asynchronous reads from the serial device.
 *            See documentation for angel_Read in devdriv.h
 */
DevError 
serraw_Read(DeviceID devid, p_Buffer buff, unsigned length)
{
    RawState *const raw_state = SerCtrl(devid)->raw_state;
    RingBuffer *rx_ring = SerCtrl(devid)->rx_ring;

    LogInfo(LOG_SERRAW, ( "serraw_Read\n"));

    Angel_EnterSVC();

    raw_state->rx_data = buff;
    raw_state->rx_to_go = length;
    raw_state->rx_copied = length;

    /* see if there is anything left over in the ring buffer */
    while (ringBufNotEmpty(rx_ring) && raw_state->rx_copied > 0)
    {
        *raw_state->rx_data = ringBufGetChar(rx_ring);
        ++raw_state->rx_data;
        --raw_state->rx_to_go;
        --raw_state->rx_copied;
    }

    if (raw_state->rx_copied == 0)
        raw_state->rx_data = NULL;

    /* enable reception */
    SerCtrl(devid)->control_rx(devid);

    Angel_ExitToUSR();

    /* block */
    while (raw_state->rx_data != NULL)
        Angel_ApplDeviceYield();

    /* clear busy flag used by higher levels */
    Angel_EnterSVC();
    angel_DeviceStatus[devid] &= ~DEV_READ_APPL_BUSY;
    Angel_ExitToUSR();

    return DE_OKAY;
}

/*
 *  Function: serraw_Control
 *   Purpose: Entry point for device control functions
 *            See documentation for angel_DeviceControl in devdriv.h
 */
DevError 
serraw_Control(DeviceID devid,
               DeviceControl op,
               void *arg)
{
    DevError ret_code;

#if DEBUG == 1
    LogInfo(LOG_SERRAW, ( "serraw_Control: op %d arg %x\n", op, arg));
#endif

    Angel_EnterSVC();

    /* all passed through */
    ret_code = SerCtrl(devid)->control(devid, op, arg);

    Angel_ExitToUSR();

    return ret_code;
}

/*
 *  Function: serraw_int_rx_processing
 *   Purpose: Deferred Rx processing for interrupt handler
 *
 *    Params: (via args)
 *       Input: devid           device ID of the driver
 *
 *   Returns: Nothing
 */
void 
serraw_int_rx_processing(void *args)
{
    const DeviceID devid = (DeviceID) args;
    RingBuffer *ring = SerCtrl(devid)->rx_ring;
    RawState *const raw_state = SerCtrl(devid)->raw_state;
    unsigned int s;

    ASSERT(devid < DI_NUM_DEVICES, ("devid"));

    /* 
     * We need to disable IRQ & FIQ whenever we modify critical data
     * shared with the interrupt handler
     */
    s = Angel_DisableInterruptsFromSVC();

    /*
     * If there are characters in the rx ring buffer, copy them
     * to the recipient.
     */
    while (ringBufNotEmpty(ring) && raw_state->rx_copied > 0)
    {
        *raw_state->rx_data = ringBufGetChar(ring);
        ++raw_state->rx_data;
        --raw_state->rx_copied;

        if (raw_state->rx_copied == 0)
        {
            raw_state->rx_data = NULL;  /* done */
            SerCtrl(devid)->control_rx(devid);
        }
    }

    /*
     * call rx control to reenable interrupts in case they were disabled
     */
    SerCtrl(devid)->control_rx(devid);

    /*
     * need to clear the flag to say we have been here
     */
    angel_DeviceStatus[devid] &= ~SER_RX_QUEUED;

    /*
     * remember to reenable interrupts before we leave
     */
    Angel_RestoreInterruptsFromSVC(s);
}

/*
 *  Function: serraw_int_tx_processing
 *   Purpose: Deferred Tx processing for interrupt handler
 *
 *    Params: (via args)
 *       Input: devid           device ID of the driver
 *
 *   Returns: Nothing
 */
void 
serraw_int_tx_processing(void *args)
{
    const DeviceID devid = (DeviceID) args;
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    RingBuffer *ring = SerCtrl(devid)->tx_ring;
    unsigned int s;

    ASSERT(devid < DI_NUM_DEVICES, ("devid"));

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
        SerCtrl(devid)->raw_state->tx_data = NULL;  /* done */
    }

    /* need to clear the flag to say we have been here */
    *status &= ~SER_TX_QUEUED;

    /*
     * If there is space in the tx ring buffer, fill it up
     * as far as possible.
     */
    if ((*status & SER_TX_DATA) != 0)
        serraw_fill_tx_ring(devid, ring);

    /* remember to reenable interrupts before we leave */
    Angel_RestoreInterruptsFromSVC(s);
}


/* EOF serraw.c */
