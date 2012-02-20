/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/08/22 15:42:44 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debugging via rombox - header file.
 */

#ifndef angel_rombox_h
#define angel_rombox_h

void RomboxPutByte(char);

#define rombox_PreWarn(l)       (IGNORE(l), TRUE)
#define rombox_PutChar(c)       RomboxPutByte((c))
#define rombox_PostWarn(n)      (IGNORE(n), (void *)0)

#endif /* ndef angel_rombox_h */

/* EOF rombox.h */
