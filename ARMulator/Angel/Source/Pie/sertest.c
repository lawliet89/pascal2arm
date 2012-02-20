/* sertest.c
 *
 * Copyright (c) 1995, Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * $Revision: 1.4.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:51:38 $
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "channels.h"
#include "devclnt.h"
#include "devdriv.h"
#include "hw.h"
#include "serial.h"
#include "logging.h"
#include "serlock.h"

#define IN_BUFF_SIZE 256

#define ERROR_CHECK(s, e) printf( "%s returned %s\n", (s), dev_error[(e)] )

volatile SerialChipStruct  SimSerial;
int i;
volatile SerialChipStruct *SerialChip = &SimSerial;

static volatile bool running;
static volatile bool IRQ_enabled = TRUE;

static angel_SerialisedFn  processor_cb;
static void              *processor_cb_data;

static char my_string[] = "This is my string \033 and so is this, "
                          "and on and on!";

static const char *dev_error[] = {
    "okay", "no dev", "bad dev", "bad chan", "bad op", "busy", "inval" 
};

static const char *dev_status[] = {
    "done", "overflow", "bad_packet", "dev_error", "int_error"
};


void Angel_Yield( void )
{
    printf( "AYIE> WHAT ARE WE DOING HERE?\n" );
    exit( 5 );
}


void Angel_SerialiseTask( angel_RegBlock *interrupted_regblock,
                          angel_SerialisedFn processor, void *data,
                          unsigned empty_stack )
{
    IGNORE( interrupted_regblock );
    IGNORE( empty_stack );

    printf( "ASER> Entered\n" );

    if ( processor_cb != NULL )
    {
        printf( "DDQP> ALREADY HAVE QUEUED PROCESSOR\n" );
    }
    else
    {
        processor_cb = processor;
        processor_cb_data = data;
    }
}

static unsigned char static_buff[IN_BUFF_SIZE];

static p_Buffer angel_ChannelAllocDevBuffer(unsigned req_size, void *cb_data)
{
    IGNORE( cb_data );

    printf( "CADB> Entered for %u bytes\n", req_size );
    if ( req_size <= IN_BUFF_SIZE )
        return static_buff;
    else
        return NULL;
}

#if 0
void angel_DD_SentPacket(DeviceID        devID,    /* which device */
                         p_Buffer        buff,     /* pointer to data */
                         unsigned        length,   /* how much done */
                         DevStatus       status,   /* success code */
                         DevWrite_CB_Fn  callback, /* function to call */
                         void           *cb_data,  /* as supplied */
                         DevChanID       devchanID) /* appl or chan */
{
    IGNORE(devchanID);

    printf( "DDSP> Entered\n" );
    running = FALSE;
    angel_DeviceStatus[devID] &= ~DEV_WRITE_BUSY;
    callback( devID, buff, length, status, cb_data );
}

void angel_DD_GotPacket(DeviceID        devID,    /* which device */
                        p_Buffer        buff,     /* pointer to data */
                        unsigned        length,   /* how much done */
                        DevStatus       status,   /* success code */
                        DevRead_CB_Fn   callback, /* function to call */
                        void           *cb_data,  /* as supplied */
                        DevChanID       devchanID) /* appl or chan */
{
    printf( "DDGP> Entered\n" );
    running = FALSE;
    callback( devID, buff, length, status, cb_data );
}
#endif

void Angel_EnableInterruptsFromSVC(void)
{
#ifdef VERBOSE
    printf( "RTEI> Entered\n" );
#endif
    IRQ_enabled = TRUE;
}

void Angel_DisableInterruptsFromSVC(void)
{
#ifdef VERBOSE
    printf( "RTDI> Entered\n" );
#endif
    IRQ_enabled = FALSE;
}


static int inSVC = FALSE;


void Angel_EnterSVC(void)
{
    if ( inSVC )
    {
        printf( "AESV> already in SVC!\n" );
        exit( 5 );
    }
    else
    {
        printf( "AESV> Entering SVC\n" );
        inSVC = TRUE;
    }
}


void Angel_ExitToUSR( void )
{
    if ( ! inSVC )
    {
        printf( "AETU> not in SVC!\n" );
        exit( 5 );
    }
    else
    {
        printf( "AETU> Exiting to USR\n" );
        inSVC = FALSE;
    }
}


void Angel_QueueCallback(angel_CallbackFn fn,
                         angel_TaskPriority priority,
                         void *a1,
                         void *a2,
                         void *a3,
                         void *a4)
{
    printf( "AQCB> Pri: %d\n", priority );
    fn( a1, a2, a3, a4 );
}


static void my_write_cb(void *v_buff, void *v_len,
                        void *v_status, void *cb_data)
{
    IGNORE(v_buff); IGNORE(v_len); IGNORE(v_status);

    printf( "MWRC> Entered, >%s<\n", (char *)cb_data );
    running = FALSE;
}

static void my_read_cb(void *v_buff, void *v_len,
                       void *v_status, void *cb_data)
{
    p_Buffer  buff   =  (p_Buffer)v_buff;
    unsigned  len    =  (unsigned)v_len;
    DevStatus status =       (int)v_status;

    unsigned i;

    printf( "MREC> Entered, status %s, cb data >%s<\n",
            dev_status[status], (char *)cb_data );

    if ( status == DS_DONE )
    {
        printf( "MREC> Data len %u, >%s<\n", len, buff );
        printf( "      Details: " );
        for ( i = 0; i < len; ++i )
           printf( "%02X ", buff[i] );
        printf( "\n" );
        if ( strcmp( buff, my_string ) == 0 )
           printf( "      Strings compare okay\n" );
        else
           printf( "      String compare FAILED\n" );
    }
    running = FALSE;
}


void LogWarning( char *format, ... )
{
    va_list args;

    va_start( args, format );
    printf( "DWAR> " );
    vprintf( format, args );
    va_end( args );
}


void LogError( char *format, ... )
{
    va_list args;

    va_start( args, format );
    printf( "DERR> " );
    vprintf( format, args );
    va_end( args );

    exit(1);
}


void main( void )
{
    DevError error_code;
    word old_isr_imr;
    int i;
    int out_index = 0;
    unsigned char out_buffer[256];

    error_code = angel_DeviceControl( DI_SERIAL, DC_INIT, NULL );
    ERROR_CHECK("MAIN> dev_init", error_code);

    error_code = angel_DeviceWrite( DI_SERIAL, my_string, strlen(my_string)+1,
                                    my_write_cb, "write cb data", DC_DBUG );
    ERROR_CHECK("MAIN> dev_write", error_code);

    running = TRUE;

    while( running )
    {
        SimSerial.ISR_IMR = ISRTxReady;
        SimSerial.SR_CSR  = SRTxReady;

        old_isr_imr = SimSerial.ISR_IMR;
        
        angel_SerialIRQHandler(IH_SERIAL, SER_DEV_IDENT_0, NULL, 0);
        /* printf( "%02x ", SimSerial.RHR_THR ); */
        out_buffer[out_index++] = SimSerial.RHR_THR;

        if ( ! IRQ_enabled )
           printf( "MAIN> IRQ DISABLED after handler!\n" );

        if ( old_isr_imr != SimSerial.ISR_IMR )
           printf( "MAIN> ISR_IMR changed in IRQ handler, to %02x\n",
                   SimSerial.ISR_IMR );

        old_isr_imr = SimSerial.ISR_IMR;

        if ( processor_cb != NULL )
        {
            processor_cb( processor_cb_data );
            processor_cb = NULL;

            if ( ! IRQ_enabled )
               printf( "MAIN> IRQ DISABLED after processor!\n" );
        }

        if ( old_isr_imr != SimSerial.ISR_IMR )
           printf( "MAIN> ISR_IMR changed in processor, to %02x\n",
                   SimSerial.ISR_IMR );

    }

    printf( "MAIN> Got: " );
    for ( i = 0; i < out_index; ++i )
       printf( "%02X ", out_buffer[i] );
    printf( "\n           " );
    for ( i = 0; i < out_index; ++i )
       printf( "%c", isprint( out_buffer[i] ) ? out_buffer[i] : '.' );
    printf( "\n" );

    error_code = angel_DeviceRegisterRead( DI_SERIAL,
                                           my_read_cb, "read cb data",
                                           angel_ChannelAllocDevBuffer, NULL,
                                           DC_DBUG );
    ERROR_CHECK("MAIN> dev_read", error_code);

    printf( "MAIN> read pass 1\n" );

    running = TRUE;
    out_index = 0;

    while( running )
    {
        SimSerial.ISR_IMR = ISRRxReady;
        SimSerial.SR_CSR  = SRRxReady;

        old_isr_imr = SimSerial.ISR_IMR;
        
        SimSerial.RHR_THR = out_buffer[out_index++];

        angel_SerialIRQHandler(IH_SERIAL, SER_DEV_IDENT_0, NULL, 0);

        if ( ! IRQ_enabled )
           printf( "MAIN> IRQ DISABLED after handler!\n" );

        if ( old_isr_imr != SimSerial.ISR_IMR )
           printf( "MAIN> ISR_IMR changed in IRQ handler, to %02x\n",
                   SimSerial.ISR_IMR );

        old_isr_imr = SimSerial.ISR_IMR;

        if ( processor_cb != NULL && ((out_index % 5) == 0) )
        {
            processor_cb( processor_cb_data );
            processor_cb = NULL;

            if ( ! IRQ_enabled )
               printf( "MAIN> IRQ DISABLED after processor!\n" );
        }

        if ( old_isr_imr != SimSerial.ISR_IMR )
           printf( "MAIN> ISR_IMR changed in processor, to %02x\n",
                   SimSerial.ISR_IMR );

    }

    printf( "MAIN> read pass 2\n" );

    running = TRUE;
    out_index = 0;

    while( running )
    {
        SimSerial.ISR_IMR = ISRRxReady;
        SimSerial.SR_CSR  = SRRxReady;

        old_isr_imr = SimSerial.ISR_IMR;
        
        SimSerial.RHR_THR = out_buffer[out_index++];

        angel_SerialIRQHandler(IH_SERIAL, SER_DEV_IDENT_0, NULL, 0);

        if ( ! IRQ_enabled )
           printf( "MAIN> IRQ DISABLED after handler!\n" );

        if ( old_isr_imr != SimSerial.ISR_IMR )
           printf( "MAIN> ISR_IMR changed in IRQ handler, to %02x\n",
                   SimSerial.ISR_IMR );

        old_isr_imr = SimSerial.ISR_IMR;

        if ( processor_cb != NULL && ((out_index % 5) == 0) )
        {
            processor_cb( processor_cb_data );
            processor_cb = NULL;

            if ( ! IRQ_enabled )
               printf( "MAIN> IRQ DISABLED after processor!\n" );
        }

        if ( old_isr_imr != SimSerial.ISR_IMR )
           printf( "MAIN> ISR_IMR changed in processor, to %02x\n",
                   SimSerial.ISR_IMR );

    }

    printf( "MAIN> DONE!\n" );

}
