/* -*-C-*-
 *
 * $Revision: 1.1.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:51:16 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial writes to 16c552 serial port B.
 */

#ifndef angel_log552_h
#define angel_log552_h

#include "logging.h"
#include "logpriv.h"

#ifdef RAW_ST16C552_B
# if RAW_ST16C552_B == 1
#  error Serial chip B in use!
# endif
#endif

#define LOG552_PORT  ST16C552_IDENT_B

bool log552_PreWarn(WarnLevel level);

int  log552_PutChar(char c);

void log552_PostWarn(unsigned int len);

#define DEFBAUD               115200              /* default baud rate */


#endif /* ndef angel_log552_h */

/* EOF log552.h */
