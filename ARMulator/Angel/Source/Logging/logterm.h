/* -*-C-*-
 *
 * $Revision: 1.1.2.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:54:01 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial writes to 16c552 serial port B.
 */

#ifndef angel_logterm_h
#define angel_logterm_h

#include "logging.h"
#include "logpriv.h"
#include "stdarg.h"

#ifdef RAW_ST16C552_B
# if RAW_ST16C552_B == 1
#  error Serial chip B in use!
# endif
#endif

#define LOGTERM_PORT  ST16C552_IDENT_B

bool logterm_PreWarn(WarnLevel level);
int  logterm_PutChar(char c);
void logterm_PostWarn(unsigned int len);

#define DEFBAUD               115200              /* default baud rate */

void angel_LogtermIntHandler(unsigned int ident, unsigned int devid,
                             unsigned int empty_stack);

bool logterm_Initialise(void);

struct LogSaveBuffer *log_getlogtermbuf(void);

#endif /* ndef angel_logterm_h */

/* EOF logterm.h */
