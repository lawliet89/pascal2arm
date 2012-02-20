/* Example usage of DC_APPL channel */
/* $Revision: 1.5.6.2 $ */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "devappl.h"    /* API for application access to devices */
#include "devconf.h"    /* Used to work out which board we're compiling for */
#include "logging.h"    /* va_warn(), etc. */
#include "support.h"    /* __rt_memcpy(), etc. */
#include "msgbuild.h"   /* msgbuild() and unpack_message() */

/* We need a memcpy() of some description */
#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
#  define memcpy(d, s, c) __rt_memcpy((d), (s), (c))
#else
#  include "stdlib.h"
#  define DEBUG 1
#endif

#define MAX_NAME_LEN 64

/* Work out which device to use */
#ifndef APPL_DEV                /* can define to compiler */
#  ifdef ST16C552_NUM_PORTS       /* PID or similar */
#    if ST16C552_NUM_PORTS > 1
#      define APPL_DEV    DI_ST16C552_B /* use 2nd serial port if it exists */
#    else
#      define APPL_DEV    DI_ST16C552_A /* else use 1st serial port */
#    endif
#  else  /* assume PIE */
#    define APPL_DEV      DI_SERIAL
#  endif
#endif

#include "appldev.h"            /* header for this example */

/* The default configuration */
static char     default_name[] = "DevAppl Trial Config";
static Config   default_config = {
    sizeof(default_name) / sizeof(char),
    default_name
};

/* The active configuration */
static char     current_name[MAX_NAME_LEN+1];
static Config   current_config = { 
    0, current_name
};

/* Copy one config to another */
static void copy_config( Config *const dest, const Config *const src )
{
    dest->name_len = src->name_len;
    memcpy( dest->name, src->name, src->name_len );
}

/* Provide debugging output a la printf() */
static int debugf( char *format, ... )
{
    va_list     args;
    int         len = 0;

    va_start( args, format );
#if DEBUG == 1
# if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
    log_vlog( WL_TRACE, format, args );
# else
    len = vprintf( format, args );
# endif
#endif
    va_end( args );

    return len;
}


/* send a command in little-endian format */
static DevError send_command( const Command *command )
{
    DevError      err;
    unsigned char cmd_buffer[ COMMAND_MIN_LENGTH ];
    unsigned int  len;

    len = msgbuild( cmd_buffer, "%w%w", command->type, command->length );
    err = Angel_ApplDeviceWrite( APPL_DEV, (p_Buffer)cmd_buffer, len );
    
    return err;
}


/* get a command in little-endian format */
static DevError get_command( Command *command )
{
    DevError      err;
    unsigned char cmd_buffer[ COMMAND_MIN_LENGTH ];

    err = Angel_ApplDeviceRead( APPL_DEV, (p_Buffer)cmd_buffer,
                                COMMAND_MIN_LENGTH );
    if ( err == DE_OKAY )
       unpack_message( cmd_buffer, "%w%w", &command->type, &command->length );

    return err;
}


/* Process a config command */
static void process_config_response( Command *response )
{
    DevError    err;

    switch ( response->type )
    {
        case C_POLL:
        {
            /* idle - ignore */
            debugf( "\tidle\n" );
            break;
        }

        case C_RESET_DEFAULT_CONFIG:
        {
            debugf( "\treset default config\n" );
            copy_config( &current_config, &default_config );
            break;
        }

        case C_GET_CONFIG:
        {
            /* send back config and await ack */
            debugf( "\tget config\n" );
            response->length = current_config.name_len;
            err = send_command( response );
            err = Angel_ApplDeviceWrite( APPL_DEV,
                                         (p_Buffer)current_config.name,
                                         response->length               );
                        
            err = get_command( response );
            if ( err != DE_OKAY || response->type != C_GET_CONFIG_ACK )
               debugf( "BAD RESPONSE to get config message\n" );

            break;
        }

        case C_SET_CONFIG:
        {
            debugf( "\tset config\n" );
            if ( response->length > 0 )
            {
                current_config.name_len = response->length;
                debugf( "reading new config, len %d\n", response->length );

                err = Angel_ApplDeviceRead( APPL_DEV,
                                            (p_Buffer)current_config.name,
                                            response->length             );
                if ( err == DE_OKAY )
                   debugf( "got new config: \"%s\"\n", current_config.name );
                else
                   debugf( "error %d reading config\n", err );
            }
            break;
        }

        default:
        {
            /* bad response */
            debugf( "\tUNKNOWN: %d\n", response->type );
            break;
        }
    }
}

int main( int argc, char *argv[] )
{
    Command     poll = { C_POLL, 0 };
    Command     response;
    DevError    err;

    IGNORE( argc ); IGNORE( argv );

    debugf( "appldev example program\n" );

    Angel_ApplInitialiseDevices();
    debugf( "initialised\n" );

    copy_config( &current_config, &default_config );

    while (TRUE)
    {
        /* poll host */
        debugf( "polling host\n" );
        err = send_command( &poll );
        if ( err == DE_OKAY )
        {
            debugf( "poll sent - awaiting reply\n" );
            err = get_command( &response );
            if ( err == DE_OKAY )
            {
                debugf( "got reply\n" );
                process_config_response( &response );
            }
            else
            {
                /* command read error */
                debugf( "poll reply read error: %d\n", err );
            }
        }
        else
        {
            /* poll write error */
            debugf( "poll write error: %d\n", err );
        }
    }
}

/* EOF appldev.c */
