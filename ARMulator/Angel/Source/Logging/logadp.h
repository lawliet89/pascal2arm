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
 *     Title: Debugging via panic block - header file.
 */

#ifndef angel_logadp_h
#define angel_logadp_h

#include "logging.h"
#include "logpriv.h"

extern char    *logadp_pos;
extern char    *logadp_end;

bool logadp_PreWarn(WarnLevel level);
#define logadp_PutChar(c) (logadp_pos < logadp_end ? (*logadp_pos++ = (c)) : 0)
void logadp_PostWarn(unsigned int len);

#endif /* ndef angel_logadp_h */

/* EOF logadp.h */
