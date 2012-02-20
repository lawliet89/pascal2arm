/* -*-C-*-
 *
 * $Revision: 1.4 $
 *   $Author: amerritt $
 *     $Date: 1996/09/05 10:36:42 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 */

#ifndef angel_debug_h
#define angel_debug_h

/* The 'debug_Status' variable holds the current debug agent state. */
extern unsigned32 debug_Status;
extern int boot_completed;

#define debug_Status_Init     (1 << 0)       /* 1 if debug agent is active */
#define debug_Status_LogRDI   (1 << 1)       /* 1 if RDI level logging on. */
#define debug_Status_LogADP   (1 << 2)       /* 1 if ADP level logging on. */
#define debug_Status_Running  (1 << 3)       /* 1 if running. */

/*
 * The 'debug_VectorCatch' value is a bitmask of ARM exception vectors
 * and Angel vectors that will raise pseudo-breakpoints to the host
 * when taken.  See adp.h - ADP_Ctrl_VectorCatch for details.
 */
extern unsigned32 debug_VectorCatch;

extern void debug_ThreadStopped(word OSinfo1, word OSinfo2, unsigned int event,
                                unsigned int subcode);
void angel_DebugInit(void);

#endif /* !defined(angel_debug_h) */

/* EOF debug.h */
