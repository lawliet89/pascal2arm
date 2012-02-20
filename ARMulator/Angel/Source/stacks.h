/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: amerritt $
 *     $Date: 1996/09/06 09:22:42 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * stacks.h - holds definitions of offsets of the various stacks
 * within the Angel Stack Area
 */

#ifndef angel_stacks_h
#define angel_stacks_h

#include "devconf.h"

typedef struct {
  unsigned *heapbase;
  unsigned *heaplimit;
  unsigned *stacktop;
  unsigned *stacklimit;
} AngelHeapStackDesc;

/* Note: not const, because profiling *may* alter some of the values */
extern AngelHeapStackDesc angel_heapstackdesc;

#define Angel_CombinedAngelStackSize \
  (Angel_FIQStackSize + Angel_IRQStackSize + Angel_SVCStackSize + \
   Angel_AngelStackSize + Angel_UNDStackSize)

/* We define a global variable which holds the Top of Memory
 * In fact this is the top of contiguous memory.  It can be
 * overwriten at run time by the memory sizer.
 */
extern unsigned Angel_TopOfMemory;

/* This is a global variable which holds the base address of all
 * the Angel stacks.  It can be modified at runtime should the
 * stacks live at the top of memory, so that when the memory
 * sizer detects how much memory is present, the stacks move
 */
extern unsigned Angel_StackBase;

/* All offsets are from Angel_StackBase */

#define Angel_SVCStackLimitOffset 0
#define Angel_SVCStackOffset      (Angel_SVCStackLimitOffset + Angel_SVCStackSize)

#define Angel_FIQStackLimitOffset  Angel_SVCStackOffset
#define Angel_FIQStackOffset      (Angel_FIQStackLimitOffset + Angel_FIQStackSize)

#define Angel_IRQStackLimitOffset Angel_FIQStackOffset
#define Angel_IRQStackOffset      (Angel_IRQStackLimitOffset + Angel_IRQStackSize)

#define Angel_AngelStackLimitOffset Angel_IRQStackOffset
#define Angel_AngelStackOffset    (Angel_AngelStackLimitOffset + Angel_AngelStackSize)

#define Angel_UNDStackLimitOffset Angel_AngelStackOffset
#define Angel_UNDStackOffset      (Angel_UNDStackLimitOffset + Angel_UNDStackSize)

/*
 *  Function: angel_RelocateWRTTopOfMemory
 * 
 *   Purpose: Called after a memory sizer has determined where the top
 *            of memory really is.  It updates dynamically changable
 *            holders of memory areas - eg. Angel stacks
 *
 *    Params: Top of Memory
 *
 *   Returns: 0 if top of memory unchanged
 *            1 if top of memory changed by this call
 *
 *   Special: This fn should only be provided in systems which
 *            can have pluggable DRAM systems or similar.
 */
extern unsigned angel_RelocateWRTTopOfMemory(unsigned int memorytop);


/*
 *  Function: angel_FindTopOfMemory
 * 
 *   Purpose: Called to detect where the top of low memory is
 *
 *    Params: None
 *
 *   Returns: The address of the top of contiguous low memory.
 *
 *   Special: This fn should only be provided in systems which
 *            can have pluggable DRAM systems or similar.
 */
extern unsigned angel_FindTopOfMemory(void);

#endif

/* EOF stacks.h */
