/* -*-C-*-
 *
 * $Revision: 1.1.2.8 $
 *   $Author: rivimey $
 *     $Date: 1998/03/11 19:07:40 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * logarmdbg.c - host (remote_a) side logging routines.
 *
 */

#include "host.h"
#include <stdarg.h>             /* ANSI varargs support */
#include <stdio.h>
#include "dbg_hif.h"
#include "logarmdbg.h"
#include <string.h>

#define LOG_SIZE 256

#if DEBUG == 1

/****************************************************************************/
/*                      H O S T   O U T P U T                               */
/****************************************************************************/

extern struct Dbg_HostosInterface const *angel_hostif;

static int log_index = 0;
static char log_block[LOG_SIZE];
static char log_filename[256] = { 0 };

static FILE *log_fp = NULL;

void logarmdbg_PutChar(char c)
{
    if (log_index >= LOG_SIZE-1 || c == '\n')
    {
        log_block[log_index++] = '\n';
        log_block[log_index] = '\0';
        logarmdbg_print(log_block);
        log_index = 0;
    }
    else
        log_block[log_index++] = c;    
}

int logarmdbg_PreWarn(WarnLevel l)
{
    log_index = 0;
    return TRUE;
}

void logarmdbg_PostWarn(int n)
{
    log_block[log_index] = '\0';
    logarmdbg_print(log_block);
}

/* this is needed because dbgprint is a varargs function, and we can't use the
   format string to pass the string to print (it may contain % characters, which
   get misinterpreted).
*/

static void apr(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    angel_hostif->dbgprint(angel_hostif->dbgarg, fmt, ap);
    va_end(ap);
}

void logarmdbg_print(const char *string)
{
    if (log_fp != NULL)
    {
        fprintf(log_fp, "%s", string);
        fflush(log_fp);
    }
    else if (angel_hostif != NULL)
    {
        apr("%s", string);
    }
}

int logarmdbg_setlogfile(const char *string)
{
    FILE *fp;
    
    if (string != NULL && *string != 0)
    {
        strcpy(log_filename, string);
        
        if (fp = (fopen(log_filename, "a")))
        {
            if (log_fp != NULL)
                fclose(log_fp);
            
            log_fp = fp;
            
            fprintf(log_fp, "\n\nARM Symbolic Debugger: Angel Operation Log\n"
                            "Rebuilt on " __DATE__ " at " __TIME__ "\n"
                            "Use $rdi_log, RDI_VERBOSE, RDI_LOG_FILE and RDI_LINE_FORMAT\n"
                            "to control operation.\n\n");
            return TRUE;
        }
        else
            return FALSE;
    }
    else
    {
        fp = log_fp;
        log_fp = NULL;
        fclose(fp);
        
        return TRUE;
    }
}

#else

void logarmdbg_setlogfile(const char *string)
{
}

#endif /* DEBUG */


