/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/08/22 15:42:40 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debugging via panic block - header file.
 */

#ifndef angel_pidulate_h
#define angel_pidulate_h

#define DIRECT_OUT_ADDR 0x3000000
#define DIRECT_OUT ((unsigned char *) DIRECT_OUT_ADDR)

#define pidulate_PreWarn(l)  (IGNORE(l), TRUE)
#define pidulate_PutChar(c)  (*DIRECT_OUT = (c))
#define pidulate_PostWarn(n) (IGNORE(n), (void *)0)

#endif /* ndef angel_pidulate_h */

/* EOF pidulate.h */
