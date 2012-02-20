/* -*-C-*-
 *
 * $Revision: 1.9.6.3 $
 *   $Author: rivimey $
 *     $Date: 1998/03/11 10:37:10 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *     Title: Devide driver for the Thumb Direct Comms Channel
 */

#include "devdriv.h"            /* device driver support */
#include "logging.h"            /* LogInfo() */
#include "support.h"            /* __rt_[en/dis]ableIRQ() */
#include "serlock.h"            /* serialisation, etc. */
#include "params.h"             /* parameter structures and utilities */
#include "serring.h"            /* interface to high-level driver */

#include "dccdrv.h"


/* General purpose constants, macros, enums, typedefs */
/* none */

/*
 * forward declarations of necessary functions
 */
#if !defined(RAW_DCC) || (RAW_DCC == 0)  /* Angel/shared */
static void dcc_flow_control(char fc_char, void *cb_data);

#endif

static DevError dcc_Control(DeviceID devid, DeviceControl op, void *arg);
static void dcc_ControlTx(DeviceID devid);
static void dcc_ControlRx(DeviceID devid);
static void dcc_KickStartFn(DeviceID devid);

/* in dccsup.s */
extern unsigned int dcc_GetFlags(void);
extern unsigned int dcc_GetWord(void);
extern void dcc_PutWord(unsigned int);

/* the state shared between the IRQ handler and the deferred processor */
static RingBuffer dcc_rx_ring;
static RingBuffer dcc_tx_ring;

#if !defined(RAW_DCC) || (RAW_DCC == 0)  /* Angel/shared */

/* the configuration needed by the TX and RX engines */

#define DCC_FC_SET  ((1 << serial_XON) | (1 << serial_XOFF))
#define DCC_CTL_SET ((1 << serial_STX) | (1 << serial_ETX) |\
                        (1 << serial_ESC))
#define DCC_ESC_SET (DCC_FC_SET | DCC_CTL_SET)

static const struct re_config engine_config =
{
    serial_STX, serial_ETX, serial_ESC,  /* self-explanatory?               */
    DCC_FC_SET,                 /* set of flow-control characters  */
    DCC_ESC_SET,                /* set of characters to be escaped */
    dcc_flow_control, (void *)DI_DCC,  /* what to do with FC chars */
    angel_DD_RxEng_BufferAlloc, (void *)DI_DCC  /* how to get a buffer */
};

/* the state of the rx engine */
static struct re_state rx_engine_state;

/* the state of the tx engine */
static struct te_state tx_engine_state;

/* packet for actual rx in progress */
static struct data_packet rx_packet;

/* the current write packet */
static struct data_packet tx_packet;

/* collected TX and RX engine state */
static RxTxState dcc_rx_tx_state =
{
    &engine_config,
    &rx_engine_state,
    &tx_engine_state,
    &rx_packet,
    &tx_packet,
    1 /* rx_flow */
};

#else /* raw */

static RawState dcc_raw_state;

static DevError dcc_RawWrite(DeviceID devid, p_Buffer buff, unsigned length);
static DevError dcc_RawRead(DeviceID devid, p_Buffer buff, unsigned length);

#endif /* ... else raw ... */

/*
 * The control functions and interface
 */
static const SerialControl dcc_ctrl =
{
#if !defined(RAW_DCC) || (RAW_DCC == 0)
    &dcc_rx_tx_state,
    NULL,
    serpkt_int_tx_processing,
    serpkt_int_rx_processing,
#else
    NULL,
    &dcc_raw_state,
    serraw_int_tx_processing,
    serraw_int_rx_processing,
#endif
    0,                          /* only one DCC device, ident not used */
    &dcc_tx_ring,
    &dcc_rx_ring,
    dcc_ControlTx,
    dcc_ControlRx,
    dcc_Control,
    dcc_KickStartFn
};

/* Publically-accessible globals */

/*
 * The device table entry for this device
 */
const struct angel_DeviceEntry angel_DccDevice =
{
#if !defined(RAW_DCC) || (RAW_DCC == 0)
    DT_ANGEL,
    {
        serpkt_AsyncWrite,
        serpkt_RegisterRead
    },
    serpkt_Control,
#else
    DT_RAW,
    {
            /*
             * We need our own veneer before the serraw versions, to
             * ensure transfers are multiples of four bytes.
             *
             * Nasty but necessary casts, as can only statically initialise
             * the first member of a union
             */
        (angel_DeviceWriteFn) dcc_RawWrite,
        (angel_DeviceRegisterReadFn) dcc_RawRead
    },
    serraw_Control,
#endif
    &dcc_ctrl,
    {
        0, NULL
    },
    {
        0, NULL
    }
};

/* Private globals */

/* status of DCC registers */
static unsigned int dccflags = 0;


static unsigned int 
ringBufGetWord(RingBuffer * ring)
{
    int c;
    unsigned int res = 0;

    for (c = 0; c < 32; c += 8)
    {
        if (ring->count != 0)
            res |= (ringBufGetChar(ring) << c);
    }
    return res;
}


#pragma no_check_stack
/*
 *  Function: dcc_KickStartFn
 *   Purpose: Kick-start tx by sending first character
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: Nothing
 */
static void 
dcc_KickStartFn(DeviceID devid)
{
    RingBuffer *ring = &dcc_tx_ring;

    IGNORE(devid);

    dccflags = dcc_GetFlags();
    if ( /* dcc_CanWrite() */ ((dccflags & 0x2) == 0))
    {
        unsigned int dcw = ringBufGetWord(ring);

        LogInfo(LOG_DCCDRV, ("\nkickstarting } 0x%8x, ", dcw));
        dcc_PutWord(dcw);
    }
}


DevError 
dcc_PollWrite(DeviceID devid)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    RingBuffer *ring = &dcc_tx_ring;

    if ((ringBufCount(ring) >= 4) ||
        ((ringBufCount(ring) > 0) && ((*status) & SER_TX_EOD)))
    {
        dccflags = dcc_GetFlags();

        if ((dccflags & 0x2) == 0)
        {
            unsigned int dcw;
            bool do_tx_processing = FALSE;

            dcw = ringBufGetWord(ring);
            LogInfo(LOG_WIRE, ("> 0x%x, ", dcw));
            dcc_PutWord(dcw);

            /*
             * if we have reached to low-water mark in the
             * ring buffer then it is time to ask for a refill,
             * as long as there is more of the packet to come
             */
            if ((ring->count < RING_LOW_WATER_MARK)
                && !((*status) & SER_TX_EOD))
                do_tx_processing = TRUE;

            /*
             * is it time to give up yet?
             */
            if (ringBufEmpty(ring))
            {
                /*
                 * if the end of the packet has been sent,
                 * then queue deferred processing of this fact
                 */
                if ((*status) & SER_TX_EOD)
                {
                    do_tx_processing = TRUE;

                    /* disable Tx if necessary */
                    (*status) &= ~SER_TX_DATA;
                }
            }

            if (do_tx_processing)
                SerCtrl(devid)->tx_processing((void *)devid);
        }
        else
        {
            /* waiting for previous tx word to complete */
            return DE_OKAY;
        }
    }

    return DE_OKAY;
}


DevError 
dcc_PollRead(DeviceID devid)
{
    RingBuffer *ring = &dcc_rx_ring;

    dccflags = dcc_GetFlags();

    /*  if (dcc_CanRead()) { */
    if (dccflags & 0x1)
    {
        bool etx = FALSE;
        int c;
        unsigned int new_word;
        RawState *const raw_state = dcc_ctrl.raw_state;

        new_word = dcc_GetWord();

        /* dcc is word based, we need to process byte-wise though */
        for (c = 0; c < 32; c += 8)
        {
            char new_ch = (new_word >> c) & 0xFF;

            LogInfo(LOG_WIRE, ("< 0x%2x, ", new_ch));
            ringBufPutChar(ring, new_ch);
            if (raw_state != NULL && raw_state->rx_to_go > 0)
                --(raw_state->rx_to_go);
            else if (new_ch == serial_ETX)
                etx = TRUE;
        }
        LogInfo(LOG_WIRE, ("\n"));

        if ((raw_state == NULL && etx)
            || (raw_state != NULL && raw_state->rx_to_go == 0
                && raw_state->rx_data != NULL)
            || (ringBufCount(ring) >= RING_HIGH_WATER_MARK))
        {
            SerCtrl(devid)->rx_processing((void *)devid);
        }
    }

    return DE_OKAY;
}


/*
 *  Function: dcc_init
 *   Purpose: Initialise the dcc port
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: status from device reset
 */
static DevError 
dcc_init(const DeviceID devid)
{
    DevError err;
    volatile unsigned int *const status = angel_DeviceStatus + devid;

    LogInfo(LOG_DCCDRV, ("dcc_init\n"));

    /* do one-time start-up initialisation */

    /* reset private flags (LEAVE lowest 8 bits alone) */
    (*status) &= 0xFF;

    /* do a reset, via the framework */
    Angel_ExitToUSR();          /* because DeviceControl requires it */
#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
    err = Angel_RawDeviceControl(devid, DC_RESET, NULL);
#else
    err = angel_DeviceControl(devid, DC_RESET, NULL);
#endif
    Angel_EnterSVC();

    return err;
}

/*
 *  Function: dcc_reset
 *   Purpose: Reset the dcc port
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 
 *
 *   Returns: DE_OKAY
 */
static DevError 
dcc_reset(DeviceID devid)
{

    IGNORE(devid);

    LogInfo(LOG_DCCDRV, ("dcc_reset\n"));

    return DE_OKAY;
}

static DevError 
dcc_Control(DeviceID devid, DeviceControl op, void *arg)
{
    DevError ret_code;

    IGNORE(arg);
    IGNORE(devid);

    LogInfo(LOG_DCCDRV, ("dcc_Control: op %d arg %x\n", op, arg));

    /* we only implement the standard ones here */
    switch (op)
    {
        case DC_RESET:
            ret_code = dcc_reset(devid);
            break;
        case DC_INIT:
            ret_code = dcc_init(devid);
            break;

        default:
            LogInfo(LOG_DCCDRV, ("dcc_Control defualt case taken - op=0x%x\n", op));
            ret_code = DE_BAD_OP;
            break;
    }

    return ret_code;
}


#if !defined(RAW_DCC) || (RAW_DCC == 0)  /* Angel/shared */

static void 
dcc_flow_control(char fc_char, void *cb_data)
{
    IGNORE(fc_char);
    IGNORE(cb_data);

    LogInfo(LOG_DCCDRV, ("dcc_flowcontrol\n"));

    /* shouldn't have any flow control on this device */
    return;
}

#endif


/* dummies - no Interrupt control required, fully polled device */

static void 
dcc_ControlTx(DeviceID devid)
{
    IGNORE(devid);
}


static void 
dcc_ControlRx(DeviceID devid)
{
    IGNORE(devid);
}

#if defined(RAW_DCC) && (RAW_DCC > 0)  /* raw */

static DevError 
dcc_RawWrite(DeviceID devid, p_Buffer buff, unsigned length)
{
    if ((length % 4) != 0)
        return DE_INVAL;
    else
        return serraw_Write(devid, buff, length);
}


static DevError 
dcc_RawRead(DeviceID devid, p_Buffer buff, unsigned length)
{
    if ((length % 4) != 0)
        return DE_INVAL;
    else
        return serraw_Read(devid, buff, length);
}


#endif /* raw */

/* EOF dccdrv.c */
