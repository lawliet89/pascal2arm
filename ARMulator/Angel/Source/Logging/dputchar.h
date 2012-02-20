/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/08/22 15:42:22 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debugging via dputchar - header file.
 */

#ifndef angel_dputchar_h
#define angel_dputchar_h

void putchar(char);

#define dputchar_PreWarn(l)     (IGNORE(l), TRUE)
#define dputchar_PutChar(c)     putchar((c))
#define dputchar_PostWarn(n)    (IGNORE(n), (void *)0)

#endif /* ndef angel_dputchar_h */

/* EOF dputchar.h */
