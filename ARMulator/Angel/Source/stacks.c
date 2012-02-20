/* -*-C-*-
 *
 * $Revision: 1.2.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:21 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: stack and memory variables and dynamic relocation of stacks
 */

#include "stacks.h"

extern int Image$$RW$$Limit;

unsigned Angel_TopOfMemory = Angel_DefaultTopOfMemory;

unsigned Angel_StackBase =
#if Angel_StacksAreRelativeToTopOfMemory
Angel_DefaultTopOfMemory + Angel_StackBaseOffsetFromTopMemory;

#else
Angel_FixedStackBase;

#endif

AngelHeapStackDesc angel_heapstackdesc =
{
#ifdef Angel_ApplHeap
    (unsigned *)Angel_ApplHeap,
#else
    0,
#endif

#ifdef Angel_ApplHeapLimit
    (unsigned *)Angel_ApplHeapLimit,
#else
  /* Top of stack = bottom of heap */
    (unsigned *)(Angel_DefaultTopOfMemory + Angel_ApplStackLimitOffset),
#endif

    (unsigned *)(Angel_DefaultTopOfMemory + Angel_ApplStackOffset),
    (unsigned *)(Angel_DefaultTopOfMemory + Angel_ApplStackLimitOffset)
};

#if MEMORY_SIZE_MAY_CHANGE_DYNAMICALLY

unsigned 
angel_RelocateWRTTopOfMemory(unsigned int memorytop)
{
    if (memorytop == Angel_TopOfMemory)
        return 0;

    Angel_TopOfMemory = memorytop;
#if Angel_StacksAreRelativeToTopOfMemory
    Angel_StackBase = memorytop + Angel_StackBaseOffsetFromTopMemory;
#endif

#ifndef Angel_ApplHeapLimit
    /* Top of stack = bottom of heap */
    angel_heapstackdesc.heaplimit = (unsigned *)(memorytop + Angel_ApplStackLimitOffset);
#endif

    angel_heapstackdesc.stacklimit = (unsigned *)(memorytop + Angel_ApplStackLimitOffset);
    angel_heapstackdesc.stacktop = (unsigned *)(memorytop + Angel_ApplStackOffset);

    return 1;
}

#define FINDTOP_BLOCK_STEP              0x80000
#define FINDTOP_FIRST_ADDRESS_TO_CHECK  0x80000
#define FINDTOP_DRAM_WRAP_ADDR          (0x3000000 + FINDTOP_BLOCK_STEP - 4)
#define FINDTOP_DRAM_MAX                ALIASTop

/* This code is intended for the PID cards only, but should be fairly
 * easily modified for other boards - by changing the values from
 * those above
 */

unsigned 
angel_FindTopOfMemory(void)
{
    unsigned memtop = FINDTOP_FIRST_ADDRESS_TO_CHECK;
    unsigned volatile *baseplace = (unsigned *)(FINDTOP_DRAM_WRAP_ADDR);
    unsigned baseval;

    /* Write a unique pattern into the bottom word so that we can detect
     * memory wrapping.
     */
    baseval = *baseplace;
    *baseplace = 0xDEADBEEF;

    /* Start from the DRAM Base and see if memory exists further up
     * If it does then we have memory up to there, so repeat ...
     * If not then we are at the top of memory already.  */
    do
    {
        unsigned tmpdata, origdata;
        unsigned volatile *testaddr = (unsigned *)(memtop + FINDTOP_BLOCK_STEP - 4);
        int failed = 0;

        origdata = *testaddr;
        *testaddr = 0x12345678;

        /* See if memory has wrapped */
        if (*baseplace != 0xDEADBEEF)
        {
            failed = 1;
        }

        tmpdata = *testaddr;
        *testaddr = origdata;

        if (tmpdata != 0x12345678)
        {
            failed = 1;
        }

        if (!failed)
            memtop += FINDTOP_BLOCK_STEP;
        else
            break;
    }
    while (memtop < FINDTOP_DRAM_MAX);

    *baseplace = baseval;
    return memtop;
}

#endif

/* EOF stacks.c */
