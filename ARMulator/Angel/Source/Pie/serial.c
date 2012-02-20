/* -*-C-*-
 *
 * $Revision: 1.27.2.6 $
 *   $Author: rivimey $
 *     $Date: 1998/01/23 18:41:02 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Serial device driver for PIE board
 */

/*
 * Provide a single serial device.  Since we're only providing a single
 * device we don't need to worry about device id's and idents.  But these
 * could be used to provide for two or more devices via the one driver.
 */

#include "serial.h"             /* header for this file */

#include "devconf.h"
#include "devdriv.h"            /* device driver support */
#include "hw.h"                 /* description of serial hardware */
#include "params.h"             /* parameter structures and utilities */
#include "rxtx.h"               /* rx/tx packet engines */
#include "logging.h"
#include "serlock.h"            /* serialisation, etc. */
#include "serring.h"            /* interface to high-level driver */
#include "prof.h"

/* General purpose constants, macros, enums, typedefs */

#define DEFBAUD               9600           /* default baud rate */

/* Forward declarations */

static void serial_ControlTx(DeviceID devid);
static void serial_ControlRx(DeviceID devid);
static DevError serial_Control(DeviceID devid,
                               DeviceControl op, void *arg);
static void serial_KickStartFn(DeviceID devid);

static DevError serial_set_params(DeviceID devid, const ParameterConfig *conf);

/*
 * The set of parameter options supported by the device
 */
static unsigned int baud_options[] = { 1200, 2400, 4800, 9600, 19200, 38400 };

static ParameterList param_list[] = {
    {
        AP_BAUD_RATE,
        sizeof(baud_options) / sizeof(baud_options[0]),
        baud_options
    }
};

/*
 *The default parameter config for the device
 */
static Parameter param_default[] = {
    { AP_BAUD_RATE, 9600 }
};

/* the state shared between the interrupt handler and the deferred processor */
static struct RingBuffer serial_rx_ring;
static struct RingBuffer serial_tx_ring;

#if !defined(RAW_PIE_SERIAL) || (RAW_PIE_SERIAL == 0) /* Angel/shared */

/* the configuration needed by the TX and RX engines */

#define SERIAL_FC_SET  ((1<<serial_XON)|(1<<serial_XOFF))
#define SERIAL_CTL_SET ((1<<serial_STX)|(1<<serial_ETX)|(1<<serial_ESC))
#define SERIAL_ESC_SET (SERIAL_FC_SET|SERIAL_CTL_SET)

static const struct re_config engine_config = {
    serial_STX, serial_ETX, serial_ESC, /* self-explanatory?               */
    SERIAL_FC_SET,                      /* set of flow-control characters  */
    SERIAL_ESC_SET,                     /* set of characters to be escaped */
    serpkt_flow_control, (void *)DI_SERIAL,    /* what to do with FC chars */
    angel_DD_RxEng_BufferAlloc, (void *)DI_SERIAL   /* how to get a buffer */
};

/* the state of the rx engine */
static struct re_state rx_engine_state;

/* the state of the tx engine */
static struct te_state tx_engine_state;

/* packet for actual rx in progress */
static struct data_packet  rx_packet;

/* the current write packet */
static struct data_packet  tx_packet;

/* collected TX and RX engine state */
static RxTxState serial_rx_tx_state =
{
    &engine_config,
    &rx_engine_state,
    &tx_engine_state,
    &rx_packet,
    &tx_packet,
    1
};

#else  /* raw */

static RawState serial_raw_state;

#endif /* ... else raw ... */


/*
 * The control functions and interface
 */
static const SerialControl serial_ctrl =
{
#if !defined(RAW_PIE_SERIAL) || (RAW_PIE_SERIAL == 0)
    &serial_rx_tx_state,
    NULL,
    serpkt_int_tx_processing,
    serpkt_int_rx_processing,
#else
    NULL,
    &serial_raw_state,
    serraw_int_tx_processing,
    serraw_int_rx_processing,
#endif
    SER_DEV_IDENT_0,
    &serial_tx_ring,
    &serial_rx_ring,
    serial_ControlTx,
    serial_ControlRx,
    serial_Control,
    serial_KickStartFn
};

/* Publically-accessible globals */

/*
 * This is the device table entry for this device
 */
const struct angel_DeviceEntry angel_SerialDevice = {
#if !defined(RAW_PIE_SERIAL) || (RAW_PIE_SERIAL == 0)
    DT_ANGEL,
    {
        serpkt_AsyncWrite,
        serpkt_RegisterRead
    },
    serpkt_Control,
#else
    DT_RAW,
    {
        /* nasty but necessary casts, as can only statically initialise
         * the first member of a union
         */
        (angel_DeviceWriteFn)       serraw_Write,
        (angel_DeviceRegisterReadFn)serraw_Read
    },
    serraw_Control,
#endif
    &serial_ctrl,
    { sizeof(param_list) / sizeof(param_list[0]), param_list },
    { sizeof(param_default) / sizeof(param_default[0]), param_default }
};

/* Private globals */

/* a copy of the Interrupt Mask Register (since it is Write-Only) */
static word IMR_copy;

static int driver_has_sent_character = 0;

/* macros to manipulate and keep track of Interrupt Mask Register */
#define IMR_set(f) ( SerialChip->ISR_IMR = (IMR_copy |= (f)) )
#define IMR_clr(f) ( SerialChip->ISR_IMR = (IMR_copy &= (~(f))) )

/* macros to control IMR in various sensible ways */
#define serdriv_EnableTxInterrupt()  (IMR_set(ISRTxReady))
#define serdriv_DisableTxInterrupt() (IMR_clr(ISRTxReady))
#define serdriv_EnableRxInterrupt()  (IMR_set(ISRRxReady))
#define serdriv_DisableRxInterrupt() (IMR_clr(ISRRxReady))

/* macros to get and put serial characters */
#define serdriv_GetChar()  (SerialChip->RHR_THR)
#define serdriv_PutChar(c) (SerialChip->RHR_THR = (c))

/* macros to get serial register status and interrupt status */
#define serdriv_GetIRQStatus() (SerialChip->ISR_IMR)
#define serdriv_GetRegStatus() (SerialChip->SR_CSR)

/* macros to control serial chip */
#define serdriv_ResetError() (SerialChip->CR = ResetError)

#pragma no_check_stack
/*
 *  Function: serial_KickStartFn
 *   Purpose: Kick-start tx by sending first character
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: Nothing
 */
static void serial_KickStartFn(DeviceID devid)
{
    char c;

    IGNORE(devid);

    c = ringBufGetChar( &serial_tx_ring );
    
    serdriv_PutChar(c);
    driver_has_sent_character = 1;
}
#pragma check_stack

/*
 * enable async transmission if not already enabled
 * NB caller should ensure protection against interrupt
 */
#pragma no_check_stack

static void serial_ControlTx(DeviceID devid)
{
    unsigned tx_status = angel_DeviceStatus[DI_SERIAL] & SER_TX_MASK;

    IGNORE(devid);
    /*
     * We enable tx whenever we want to send a flow control char,
     * OR when we are sending a packet and we have not been XOFF'ed
     */

    if (    ( tx_status & SER_TX_FLOW_CONTROL )
         || ( tx_status == SER_TX_DATA )        )
    {
        /* tx interrupt should be enabled */
        if ( angel_DeviceStatus[DI_SERIAL] & SER_TX_IRQ_EN )
        {
            /* already enabled -- do nothing */
        }
        else
        {
            serdriv_EnableTxInterrupt(); /* this should raise an interrupt */
            angel_DeviceStatus[DI_SERIAL] |= SER_TX_IRQ_EN;
        }
    }
    else
    {
        /* tx interrupt should be disabled */
        if ( angel_DeviceStatus[DI_SERIAL] & SER_TX_IRQ_EN )
        {
            serdriv_DisableTxInterrupt();
            angel_DeviceStatus[DI_SERIAL] &= ~SER_TX_IRQ_EN;
        }
        else
        {
            /* already disabled -- do nothing */
        }
    }
}

#pragma check_stack


/*
 * control rx interrupt mask according to flow control and read interest
 * NB caller should ensure protection against interrupt
 */
static void serial_ControlRx(DeviceID devid)
{
    IGNORE(devid);

    if (    ! (angel_DeviceStatus[DI_SERIAL] & SER_RX_DISABLED)
         && ( angel_DeviceStatus[DI_SERIAL] & DEV_READ_BUSY_MASK ) )
    {
        /* rx interrupt should be enabled */
        if ( angel_DeviceStatus[DI_SERIAL] & SER_RX_IRQ_EN )
        {
            /* already enabled -- do nothing */
        }
        else
        {
#if PARALLEL_SUPPORTED
            PIO->IMR = RxIntEnable;
#endif
            serdriv_EnableRxInterrupt();
            angel_DeviceStatus[DI_SERIAL] |= SER_RX_IRQ_EN;
        }
    }
    else
    {
        /* rx interrupt should be disabled */
        if ( angel_DeviceStatus[DI_SERIAL] & SER_RX_IRQ_EN )
        {
#if PARALLEL_SUPPORTED
            PIO->IMR = RxIntDisable;
#endif
            serdriv_DisableRxInterrupt();
            angel_DeviceStatus[DI_SERIAL] &= ~SER_RX_IRQ_EN;
        }
        else
        {
            /* already disabled -- do nothing */
        }
    }
}


/*
 * wait for tx to drain completely
 * 
 * Note that we only wait to see if the Tx buffer is empty if
 * we are sure that some characters have been sent.  This is
 * because until a character has been sent it always claims to
 * be non empty (a feature of the serial chip).
 */
#define DRAIN_DELAY_COUNT 800

static void serial_tx_drain( void )
{
    int i;                      /* delay loop counter */
    volatile int z;             /* delay loop dummy   */
    
    if (driver_has_sent_character) {
      while( ! (serdriv_GetRegStatus() & SRTxEmpty) )
        /* just wait */ ;
    }

    /*
     * Kick our heals for a wee while.  This is ugly but it seems to work.
     */
    for ( i = 0; i < DRAIN_DELAY_COUNT; ++i ) {
      z = (123 * z << 23) ^ (7 * z >> 13);
    }
}


/*
 * Harware reset of UART
 */
static DevError serdriv_ResetDriver( DeviceID devid )
{
    serial_tx_drain();

    SerialChip->CR        = DisableRxTx;
    SerialChip->CR        = ResetMR1;
    SerialChip->MR        = MR1Value;
    SerialChip->MR        = MR2Value;
    SerialChip->CR        = ResetError;
    SerialChip->CR        = ResetBreak;
    SerialChip->CR        = ResetMPI;

    IMR_set( 0 );        /* rx and tx interrupts enabled as needed */

#if PARALLEL_SUPPORTED
    PIO->IMR = RxIntDisable;
#endif
    
    return ( serial_set_params( devid, &angel_SerialDevice.default_config ) );
}


/*
 * Initialisation control operation
 */
static DevError serial_init(DeviceID devid)
{
   DevError err;

   /*
    * do one-time start-up initialisation
    * (for this device, just a device reset)
    */
   Angel_ExitToUSR();           /* because DeviceControl requires it */

#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
   err=Angel_RawDeviceControl(devid, DC_RESET, NULL);
#else
   err=angel_DeviceControl(devid, DC_RESET, NULL);
#endif

   Angel_EnterSVC();

   return err;
}


/*
 * Reset control operation
 */
static DevError serial_reset(DeviceID devid)
{
   (void)serdriv_ResetDriver(devid);            /* low_level reset */

   /* reset private flags (LEAVE lowest 8 bits alone) */
   angel_DeviceStatus[devid] &= 0xFF;

   serial_ControlRx(devid);

   return DE_OKAY;
}


/*
 * Receive Mode control operation
 */
static DevError serial_recv_mode(DeviceID devid, DevRecvMode mode)
{
    DevError ret_code = DE_OKAY;

    if ( mode == DR_DISABLE )
    {
        if ( ! (angel_DeviceStatus[DI_SERIAL] & SER_RX_DISABLED) )
        {
            /* disable reception and schedule send of XOFF */
            angel_DeviceStatus[DI_SERIAL] |=
                (SER_RX_DISABLED | SER_TX_FLOW_CONTROL);
            serial_ControlRx(devid);
            serial_ControlTx(devid);
        }
    }
    else if ( mode == DR_ENABLE )
    {
        if ( angel_DeviceStatus[DI_SERIAL] & SER_RX_DISABLED )
        {
            /* enable reception and schedule send of XON */
            angel_DeviceStatus[DI_SERIAL] &= ~SER_RX_DISABLED;
            angel_DeviceStatus[DI_SERIAL] |= SER_TX_FLOW_CONTROL;
            serial_ControlRx(devid);
            serial_ControlTx(devid);
        }
    }
    else
    {
        ret_code = DE_INVAL;
    }

    return ret_code;
}


/*
 * Set Speed control operation
 */
static DevError serial_set_params(DeviceID devid, const ParameterConfig *conf)
{
    word speed;
    word baud_word, baud_value;
    word acr;

    IGNORE(devid);

    if ( ! Angel_FindParam( AP_BAUD_RATE, conf, &speed ) )
    {
       LogWarning(LOG_SERIAL, ("serial_set_params: Speed parameter not found in config\n"));
       return DE_OKAY;
    }

    switch (speed)
    {
        case 1200:
            baud_word = Baud1200;
            break;
            
        case 2400:
            baud_word = Baud2400;
            break;
            
        case 4800:
            baud_word = Baud4800;
            break;
            
        case 9600:
            baud_word = Baud9600;
            break;
            
        case 19200:
            baud_word = Baud19200;
            break;
            
        case 38400:
            baud_word = Baud38400;
            break;
            
        default:     return DE_BAD_OP;
    }

    if (baud_word & 0x80000000)
        acr = ACRValue + 0x80;
    else
        acr = ACRValue;
    
    baud_word &= ~0x80000000;
    
    baud_value = (baud_word | (baud_word << 4));     /* baud value for sender and received */

    LogInfo(LOG_SERIAL, ("serial_set_params: Speed %d (csr: 0x%x, acr: 0x%x)\n",
                         speed, baud_value, acr));
    
    /* First wait for sending to finish */
    serial_tx_drain();

    SerialChip->CR     = DisableRxTx;

    SerialChip->SR_CSR = baud_value;
    SerialChip->ACR    = acr;
    SerialChip->CR     = ResetRx;
    SerialChip->CR     = ResetTx;
    SerialChip->CR     = EnableRxTx;

    return DE_OKAY;
}


/*
 * Set the LED on the PIE board - uses serial chip controller (!!!)
 */
static DevError serial_set_led(DeviceID devid, int state)
{
    IGNORE(devid);
    SerialChip->CR = (state & 1) == 0 ? NegateMPO : AssertMPO;
    return DE_OKAY;
}

/*
 * implementation of device control for serial driver
 *
 * devid is guaranteed to be in range but op is not checked as
 * individual devices can extend the range of operations
 */
static DevError serial_Control(DeviceID devid,
                               DeviceControl op, void *arg)
{
    DevError ret_code;

    LogInfo(LOG_SERIAL, ("serial_Control: op %d arg %x\n", op, arg ));

    /* we only implement the standard ones here */
    switch( op )
    {
       case DC_INIT:
           ret_code = serial_init(devid);
           break;

       case DC_RESET:
           ret_code = serial_reset(devid);
           break;

       case DC_RECEIVE_MODE:
           ret_code = serial_recv_mode(devid, (DevRecvMode)((int)arg));
           break;

       case DC_SET_PARAMS:
           ret_code = serial_set_params(devid, (const ParameterConfig *)arg);
           break;

       case DC_SET_LED:
           ret_code = serial_set_led(devid, (int) arg);
           break;

       default:
           ret_code = DE_BAD_OP;
           break;
    }

    return ret_code;
}


/*
 * character processor for serial interrupt handler
 */
#pragma no_check_stack
__inline static bool serial_process_rx_char( char rx_ch )
{
    bool queue_rx_deferred = FALSE;

    if ( ! ringBufFull( &serial_rx_ring ) )
    {
        ringBufPutChar( &serial_rx_ring, rx_ch );

#if !defined(RAW_PIE_SERIAL) || (RAW_PIE_SERIAL == 0) /* Angel/shared */

        if (   rx_ch == serial_ETX
            || (ringBufCount( &serial_rx_ring ) >= RING_HIGH_WATER_MARK) )
        {
            /* note that we need to do deferred rx processing */
            queue_rx_deferred = TRUE;
        }

#else  /* raw */

        {
            RawState *const raw_state = serial_ctrl.raw_state;

            if (raw_state->rx_to_go > 0)
               --(raw_state->rx_to_go);

            if (    ( raw_state->rx_to_go == 0 && raw_state->rx_data != NULL )
                 || (ringBufCount( &serial_rx_ring ) >= RING_HIGH_WATER_MARK) )
            {
                /* note that we need to do deferred rx processing */
                queue_rx_deferred = TRUE;
            }
        }

#endif

    }
    else
    {
        LogError(LOG_SERIAL, ("rx ring buffer overflow\n" ));
    }

    return queue_rx_deferred;
}
#pragma check_stack


#if PARALLEL_SUPPORTED > 0

#define PP_POLL_TIMEOUT_COUNT 100000

/*
 *  Function: serial_pp_processing
 *   Purpose: Read a stream of characters from the parallel port
 *
 *    Params: (via args)
 *       Input: device ident of the parallel device
 *
 *   Returns: Nothing
 */
static void serial_pp_processing(void *args)
{
    DeviceID devid = (DeviceID)args;
    bool done = TRUE;
    bool timeout;
    unsigned int i;

    /*
     * When we first enter, we know there is a character waiting
     */
    do {
        timeout = TRUE;

        for ( i = 0; i < PP_POLL_TIMEOUT_COUNT; ++i )
        {
            if ( PIO->RSR & RxDataMask )
            {
                /* get and process a character */
                word rx_word = PIO->RDR; /* actually just a byte */

                done = serial_process_rx_char( (char)rx_word );

                if ( done )
                {
                    /*
                     * Since we're already serialised, we just call the
                     * rx_processing stuff anyway!
                     */
                    serial_ctrl.rx_processing((void *)devid);
                }

                timeout = FALSE;
                break;          /* out of the for loop */
            }
        }

    } while ( ! timeout && ! done );

    {
        /* reenable parallel port interrupts as appropriate */
        unsigned int is = Angel_DisableInterruptsFromSVC();
        serial_ControlRx( devid );
        Angel_RestoreInterruptsFromSVC(is);
    }
}
#endif /* PARALLEL_SUPPORTED */


#if !BUFFERED_SERIAL_INPUT

#define SR_POLL_TIMEOUT_COUNT 100000

static char rx_preread_char;

/*
 *  Function: serial_rx_processing
 *   Purpose: Read a stream of characters from the unbuffered
 *            serial port
 *
 *    Params: (via args)
 *       Input: device ident of the serial device
 *
 *   Returns: Nothing
 */
static void serial_rx_processing(void *args)
{
    DeviceID devid = (DeviceID)args;
    bool done = TRUE;
    bool timeout;
    unsigned int i;

    /*
     * When we first enter, we know that a character has been pre-read
     */
    done = serial_process_rx_char(rx_preread_char);
    if ( done ) {
      /*
       * Since we're already serialised, we just call the
       * rx_processing stuff anyway!
       */
      /* reenable serial port interrupts as appropriate */
      serdriv_EnableRxInterrupt();

      serial_ctrl.rx_processing((void *)devid);

      return;
    }

    do {
        timeout = TRUE;

        for ( i = 0; i < SR_POLL_TIMEOUT_COUNT; ++i )
        {   unsigned status = serdriv_GetRegStatus();
            if (status & SRRxReady)
            {
                /* get and process a character */
                char rx_ch = serdriv_GetChar();
                
                done = serial_process_rx_char( rx_ch );

                if ( done )
                {
                    /*
                     * Since we're already serialised, we just call the
                     * rx_processing stuff anyway!
                     */
                    /* reenable serial port interrupts as appropriate */
                    serdriv_EnableRxInterrupt();
                    serial_ctrl.rx_processing((void *)devid);
                    return;
                }

                timeout = FALSE;
                break;          /* out of the for loop */
            }
        }

    } while ( ! timeout && ! done );

    /* reenable serial port interrupts */
    serdriv_EnableRxInterrupt();

}
#endif /* !BUFFERED_SERIAL_INPUT */


/*
 * interrupt handler for serial driver
 */
#pragma no_check_stack

void angel_SerialIntHandler( unsigned ident, unsigned data,
                             unsigned empty_stack )
{
    word        ser_status, isr;
    bool        queue_rx_deferred = FALSE;

#if PARALLEL_SUPPORTED == 0
    IGNORE( ident );
#endif
    IGNORE( data );
#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
    IGNORE( empty_stack );
#endif

#if PARALLEL_SUPPORTED
    if (    ident == IH_PARALLEL
         && (angel_DeviceStatus[DI_SERIAL] & SER_RX_IRQ_EN) )
    {
        /* disable interrupts */
        PIO->IMR = RxIntDisable;
        angel_DeviceStatus[DI_SERIAL] &= ~SER_RX_IRQ_EN;

        /* serialise to polled parallel rx */
# if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
        Angel_SerialiseTask( 0,
                             serial_pp_processing, (void *)DI_PARALLEL,
                             empty_stack );
        /* never returns */
# else
        serial_pp_processing((void *)DI_PARALLEL);
        return;
# endif
    }
#endif /* PARALLEL_SUPPORTED */

    /*
     * This could be a rx or tx interrupt, or an error.
     */

    isr = serdriv_GetIRQStatus();
    ser_status = serdriv_GetRegStatus();

    if ( ser_status & SRNastyError )
    {
        LogWarning(LOG_SERIAL, ( "Nasty serial chip error %02x\n", ser_status ));

        serdriv_ResetError();

        return;
    }
#if BUFFERED_SERIAL_INPUT
    else if ( ser_status & SRRxReady )
    {
        /* For an rx interrupt, place the character into a ring-buffer
         * and queue processing for after we have re-enabled
         * interrpts.  This is on the basis that we have a fairly
         * large receive buffer (say 4 bytes) into which bytes get
         * put on arrival, and that an interrupt only occurs when the
         * buffer is pretty much full.  Thus we can afford one
         * interrupt per buffer full, and then process the entire
         * buffer.
         */

        do {
            queue_rx_deferred = serial_process_rx_char( serdriv_GetChar() );

# ifdef USING_ARMPIE
            break;
# endif

            ser_status = serdriv_GetRegStatus();

        } while (    (ser_status & SRRxReady)
                  && (! ringBufFull( &serial_rx_ring )) );
    }
#else   /* Non BUFFERED_SERIAL_INPUT */
    else if ((ser_status & (SRRxReady | SRRxFull) ) == (SRRxReady | SRRxFull))
    {
        /* For non buffered rx, we cannot afford an interrupt and an
         * Angel Serialisation for every byte which arrives, so
         * instead we get the first byte under interrupt and then
         * disable interrupts and poll in the rest after serialisation
         * - in fact much in the same way as we do for the parallel
         * port.
         *
         * Note that for unbuffered we currently require that:
         *  * the first byte of a packet causes an interrupt and
         *    then when we poll in the rest of the packet
         *  * we have a large enough ring buffer for a whole packet.
         *  * we poll for a very long time before timing out when
         *    polling
         */

        /* Read the byte that has just arrived so that we have more
         * time to do the serialisation before overrun occurs.
         * Note that we are not processing it yet though.
         */
        rx_preread_char = serdriv_GetChar();

        /* Although it is not supposed to it seems that the unix ADP
         * driver in Tools 2.10 very occasionally output XON and XOFF
         * characters.  This is not actively supported by this driver,
         * but we do need to cope when receiving these unexpectedly.
         */
        if (rx_preread_char != serial_XON && rx_preread_char != serial_XOFF) {
          /* Disable interrupts */
          serdriv_DisableRxInterrupt();
          angel_DeviceStatus[DI_SERIAL] &= ~SER_RX_IRQ_EN;

          /* serialise to polled serial rx */
# if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
          Angel_SerialiseTask( 0,
                               serial_rx_processing, (void *)DI_SERIAL,
                               empty_stack );
          /* never returns */
# else
          serial_rx_processing((void *)DI_SERIAL);
          return;
# endif
        } else return;
    }
#endif /* BUFFERED_SERIAL_INPUT */
    else if ( ser_status & SRTxReady )
    {
        bool queue_tx_deferred = FALSE;

        /*
         * For a tx interrupt, get the next character to transmit and send
         * it to the device.
         */

        /* do we need to send a flow control character? */
        if ( angel_DeviceStatus[DI_SERIAL] & SER_TX_FLOW_CONTROL )
        {
            bool rx_disabled = angel_DeviceStatus[DI_SERIAL] & SER_RX_DISABLED;

            /* send a flow control char to UART */
            serdriv_PutChar( rx_disabled ? serial_XOFF : serial_XON );

            angel_DeviceStatus[DI_SERIAL] &= ~SER_TX_FLOW_CONTROL;
        }
        else if ( ringBufNotEmpty( &serial_tx_ring ) )
        {
            char ch;
            /* send a packet char to UART */
            serdriv_PutChar( ch=ringBufGetChar( &serial_tx_ring ) );
        }

        /*
         * if we have reached to low-water mark in the
         * ring buffer then it is time to ask for a refill,
         * as long as there is more of the packet to come
         */
        if (   (ringBufCount( &serial_tx_ring ) < RING_LOW_WATER_MARK)
            && ! (angel_DeviceStatus[DI_SERIAL] & SER_TX_EOD)   )
           queue_tx_deferred = TRUE;

        /*
         * is it time to give up yet?
         */
        if ( ringBufEmpty( &serial_tx_ring ) )
        {
            /*
             * if the end of the packet has been sent,
             * then queue deferred processing of this fact
             */
            if ( angel_DeviceStatus[DI_SERIAL] & SER_TX_EOD )
            {
                queue_tx_deferred = TRUE;

                /* disable Tx if necessary */
                angel_DeviceStatus[DI_SERIAL] &= ~SER_TX_DATA;
                serial_ControlTx(DI_SERIAL);
            }
            else
            {
                /*
                 * queue_tx_deferred will already be set, but
                 * we need to set a flag to tell fill_tx_ring
                 * that it needs to restart character Tx.
                 */
                angel_DeviceStatus[DI_SERIAL] |= SER_TX_KICKSTART;
            }
        }

        /* If we got here then we may need to queue deferred tx processing */
        if ( queue_tx_deferred &&
             ! (angel_DeviceStatus[DI_SERIAL] & SER_TX_QUEUED) )
        {
            angel_DeviceStatus[DI_SERIAL] |= SER_TX_QUEUED;
#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
            Angel_SerialiseTask( 0,
                                 serial_ctrl.tx_processing, DI_SERIAL,
                                 empty_stack );
#else
            serial_ctrl.tx_processing((void *)DI_SERIAL);
#endif
        }
        return;                 /* from handling TX interrupt */
    }
    else                        /* Interrupt event not determined */
    {
        LogWarning(LOG_SERIAL, ( "No Interrupt to handle" ));
        return;
    }

#if BUFFERED_SERIAL_INPUT
    /*
     * we drop thru here from a serial rx char see if we need to do
     * deferred rx processing
     */
    if ( queue_rx_deferred &&
         ! (angel_DeviceStatus[DI_SERIAL] & SER_RX_QUEUED) )
    {
        angel_DeviceStatus[DI_SERIAL] |= SER_RX_QUEUED;
# if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
        Angel_SerialiseTask( 0,
                             serial_ctrl.rx_processing, (void *)DI_SERIAL,
                             empty_stack );
# else
        serial_ctrl.rx_processing((void *)DI_SERIAL);
# endif
    }
#endif /* !BUFFERED_SERIAL_INPUT */

}

#if PROFILE_SUPPORTED

void Angel_ProfileTimerStart(int interval)
{

  /* interval is the desired interrupt interval (in microseconds) */
  /* (we assume r0 < 2^16, and measure interval in units of 2^-20 */
  /* seconds for ease of calculation).                            */

  unsigned t1 = interval * (TimerFreq % 0x10000),
           t2 = interval * (TimerFreq / 0x10000);
  t1 = (t1 >> 20) + (t2 >> 4);
  if (t1 == 0) t1 = 1;

  SerialChip->CTUR = t1 >> 8;
  SerialChip->CTLR = t1 & 255;

  SerialChip->CR = StartTimer;

  /* This needs to be atomic wrt serial driver activity */
  Angel_EnterSVC();
  IMR_set(IMRTimer);
  Angel_ExitToUSR();
}

void Angel_ProfileTimerStop(void)
{
  /* This needs to be atomic wrt serial driver activity */
  Angel_EnterSVC();
  IMR_clr(IMRTimer);
  Angel_ExitToUSR();
}

void Angel_TimerIntHandler(unsigned ident, unsigned data, unsigned empty_stack)
{
  unsigned pc = Angel_MutexSharedTempRegBlocks[0].r[15];
  IGNORE(ident);
  IGNORE(data);
  IGNORE(empty_stack);
  if (angel_profilestate.enabled) {
    unsigned *map = angel_profilestate.map;
    int size = angel_profilestate.numentries;
    int low = 0,
        high = size-1;
    if (map[low] <= pc && pc < map[high]) {
      int i;
      for (;;) {
        i = (high + low) / 2;
        if (pc >= map[i]) {
          if (pc < map[i+1]) {
            i++; break;
          } else
            low = i;
        } else if (pc >= map[i-1])
          break;
        else
          high = i;
      }
      angel_profilestate.counts[i-1]++;
    }
  }
  SerialChip->CR = ResetTimer;  /* clear the interrupt */
}

#endif /* PROFILE_SUPPORTED */

#pragma check_stack


/* EOF serial.c */
