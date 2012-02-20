/* -*-C-*-
 *
 * $Revision: 1.1.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/01/05 13:36:52 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * logarmdbg.c - 
 *
 */

#include <stdarg.h>             /* ANSI varargs support */
#include <stdio.h>

#include "logging.h"
#include "logpriv.h"

#include "dbg_hif.h"

int logarmdbg_setlogfile(const char *string);
void logarmdbg_print(const char *string);
void logarmdbg_PutChar(char c);
int logarmdbg_PreWarn(WarnLevel l);
void logarmdbg_PostWarn(int n);


