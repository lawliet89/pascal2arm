/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/08/22 15:42:25 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debugging via E5 romulator backchannel
 */

#ifndef angel_e5_h
#define angel_e5_h

void BCPutByte(char);

#define e5_PreWarn(l)  (IGNORE(l), TRUE)
#define e5_PutChar(c)  BCPutByte((c))
#define e5_PostWarn(n) (IGNORE(n), (void *)0)

#endif /* ndef angel_e5_h */

/* EOF e5.h */
