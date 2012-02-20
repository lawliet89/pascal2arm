/*
  Title:        alloc - Storage management (dynamic allocation/deallocation)
  $Revision: 1.4.22.1 $  LDS 31-Jul-87 $ BJK 23-Oct-1987
  $Revision: 1.4.22.1 $  LH 22-Dec-1987
  $Revision: 1.4.22.1 $  LH 03-Feb-1988

  Copyright (C) Advanced Risc Machines Ltd., 1991
*/

/* ***** IMPORTANT ** IMPORTANT ** IMPORTANT ** IMPORTANT ** IMPORTANT *****
 * The #defines which control a large part of this source file are decribed
 * in the header file.
 */

/*
 * NOTES:
 *  Non-implemented (possible) functionality is described under ASSUMPTIONS
 *   and marked with a '!'.
 *  Heap extensions inside the current heap (in a previous heap hole) has not
 *   been tested, but the code is there.
 *  A certain percentage (FRACTION_OF_HEAP_NEEDED_FREE) of the heap is always
 *   kept free, this is a bit wasteful but the number of coalesces and garbage
 *   collections goes down as this percentage rises. It has been found by
 *   experimentation that this fraction should be approximately between 1/8 and
 *   1/4 (currently at 1/6). Large blocks are allocated from the start of the
 *   overflow list ie the low memory addresses and small and medium sized
 *   blocks are allocated from the end of the overflow list. For this reason
 *   the overflow list is a doubly linked list with a head at both ends. A
 *   pointer to the last free block on the heap is also kept so that when the
 *   heap is extended and the old bitmap is returned to the free list (and
 *   merged with any adjacent free block), the last heap block, if it is free,
 *   can be merged with it also.
 * ASSUMPTIONS:
 *  Address units are in bytes.
 *  There are an exact number of address units (bytes) per word.
 *  All target machines are either word aligned or run slower with non word
 *   alignment (so word aligning is a good and right thing to do).
 *  The heap can not grow downwards (all heap extensions must be above the
 *   heap base determined by the first block claimed from OSStorage), and if
 *   two consecutive (in time) blocks are doled out by OSStorage they are only
 *   assumed to be contiguous if the lower limit (arithmetically) of the second
 *   block is equal to the higher limit (arithmetically) of the first block
 *   plus one.
 *  Blocks may be doled out in unspecified address order (but note that
 *   every time a heap extension, which is inside my heap bounds, is given out
 *   the heap has to be scanned in order to find and modify the heap hole in
 *   which the extension has been given.
!*  The range of address units to be found in a single bin can only be the
 *   number of address units in a word, extra code will have to be written to
 *   manage bin ranges other than this size (more trouble than its worth, if
 *   its worth anything at all).
 *  MAXCARD is the largest number representable in a word (ie all bits set).
 * ALLOCATE:
 *  An array of lists of free blocks of similar sizes (bins) is kept so that
 *   when an ALLOCATE of size n is requested the list starting at array entry
 *   n DIV BINRANGE will automatically have as the first element of the list
 *   a block of the correct size (plus the OVERHEADWORDS) or no block at all
 *   (or the block requested may be too big to be in the allocate bins). if
 *   there is no block available in the bin, then bins containing lists of
 *   larger blocks are checked and the block allocated from one of these (if
 *   the bin block is big enough, it is split). if there is still no block
 *   available then the overflow list is checked and if available, the block
 *   is cut from here (the block required is cut from the end of the larger
 *   block if the size required is not large (size < LARGEBLOCK) otherwise it
 *   is taken from the start of the large block). if the remainder of the block
 *   is greater than the largest bin block then it remains in the overflow
 *   list, otherwise it is removed to the correct bin. if the overflow list
 *   does not have a block large enough then the heap is either extended (more
 *   memory claimed from OSStorage), coalesced or garbage collected, depending
 *   on the state of the heap etc and whether garbage collection is enabled.
 *   After coalescing or garbage collection the allocate algorithm is executed
 *   again in order to allocate the block.
 * COALESCE:
 *  if the overflow list does not contain a block large enough and a
 *   reasonable amount of storage has been deallocated since the last coalesce,
 *   (reasonable is difficult to define and is only deducable by
 *   experimentation) then all allocatable blocks (by storage) and all blocks
 *   on the overflow deallocate list are marked free, the heap is scanned and
 *   the blocks scattered into bins and overflow list in increasing address
 *   order.
 * DEALLOCATE:
 *  When a block is DEALLOCATED, if it will fit in a bin then it is put at
 *   the start of the relevant bin list otherwise it is conceptually released
 *   to the overflow deallocate list (there is no need for a list, set the
 *   block's header bits to indicate it is free and it will automatically be
 *   sucked in at the next coalesce).
 * HEAP EXTENSIONS:
 *  Whenever the heap is extended, a certain amount (if available) is allocated
 *   for the garbage collection bit maps (even if garbage collection has not
 *   been enabled.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>             /* for memset(...), memcpy(...) */

#include "alloc.h"
#include "rt.h"             /* for __rt_alloc */

#if defined(VERBOSE)||defined(DEBUG)||defined(STACKCHECK)||defined(ANALYSIS)
#include <stdio.h>
#endif

#include "externs.h"
#include "interns.h"

#if defined alloc_c || defined SHARED_C_LIBRARY

#if defined(VERBOSE)||defined(DEBUG)||defined(STACKCHECK)||defined(ANALYSIS)
#include "riscos.h"
static int n, d, last, iw;
#define dbmsg(f, a, b, c, d) {char v[128]; sprintf(v, f, a, b, c, d); last = 0;\
        for(iw=0;v[iw];__riscos_oswrch(last=v[iw++])); \
        if (last == 10)__riscos_oswrch(13);}
#define LOW_OVERHEAD_F0(v) {last = 0; \
        for(iw=0;v[iw];__riscos_oswrch(last=v[iw++])); \
        if (last == 10)__riscos_oswrch(13);}
#define LOW_OVERHEAD_FD(i, b) {n = i; d = 1; \
        while (n >= b) {d *= b; n /= b;} n = i; \
        while(d) {if ((n/d) > 9) __riscos_oswrch(n/d+'A'-10); \
                  else __riscos_oswrch(n/d+'0'); \
                  n = n-(n/d)*d; d /= b;}}
#else
#define dbmsg(f, a, b, c, d)
#define LOW_OVERHEAD_F0(v)
#define LOW_OVERHEAD_FD(i, b)
#endif
/* put this here too */
#ifdef VERBOSE
#define F0(f)             LOW_OVERHEAD_F0(f)
#define FD(i, b)          LOW_OVERHEAD_FD(i, b)
#else
#define F0(f)
#define FD(i, b)
#endif
#ifdef DEBUG
#define D0(f)             LOW_OVERHEAD_F0(f)
#define DD(i, b)          LOW_OVERHEAD_FD(i, b)
#define D1(f, a)          dbmsg(f, a, 0, 0, 0)
#define D2(f, a, b)       dbmsg(f, a, b, 0, 0)
#define D3(f, a, b, c)    dbmsg(f, a, b, c, 0)
#define D4(f, a, b, c, d) dbmsg(f, a, b, c, d)
#else
#define D0(f)
#define DD(i, b)
#define D1(f, a)
#define D2(f, a, b)
#define D3(f, a, b, c)
#define D4(f, a, b, c, d)
#endif

#define IGNORE(param) param = param

#define FALSE 0
#define TRUE  1

/*
 * FRACTION_OF_HEAP_NEEDED_FREE is used when deciding whether to coalesce, GC
 * or extend the heap. An attempt is made to keep this amount free, if it is
 * not free then the heap is extended. The amount of free space is the total of
 * all free blocks (without overheads). if there is a bitmap at the end of the
 * heap, it is not included in the heap size.
 */
#define FRACTION_OF_HEAP_NEEDED_FREE 6
/* initialisation for blocks on allocation */

static BlockP heapLow;  /* address of the base of the heap */
static BlockP heapHigh; /* address of heap hole guard at the top of heap */
static BlockP sys_heap_top; /* address of top of system heap, should = heapLow
                               after _init_user_alloc is called          */
/*
 * amount of heap that user can actually write to, does not include bitmaps
 * and block overheads
 */
static size_t totalFree;
static size_t userHeap;  /* size of heap (bytes) excluding gc bitmaps */
static size_t totalHeap; /* size of heap (bytes) including gc bitmaps */
/*
 * The overflow list is a chain of large blocks ready for use, the chain is a
 * doubly linked list of blocks in increasing address order.
 * bin[0] is the start of the overflow list.
 * bin[NBINS+1] is end of the overflow list.
 *
 * bin is an array of pointers to lists of free small blocks ( <= MAXBINSIZE)
 * of the same size. Last deallocated block is at the start of the list.
 */
static BlockP bin[NBINS+2];

static BlockP endOfLastExtension;

static struct {
  char allocates, deallocates;
} check;

static int lookInBins;

static BlockP lastFreeBlockOnHeap;
static int enoughMemoryForGC;
static char *mapForExistingHeap;
static char *mapForNewHeap;
static BlockP endOfExistingHeap;
static BlockP startOfNewHeap;

#define MAXEVENTS 64 /* remember the last MAXEVENTS events */

typedef struct StatsStruct {
  StorageInfo stats;
  EventInfo events[MAXEVENTS];
  int nextEvent;
  /* ShowStats variables */
  unsigned guard;
  size_t size;
  unsigned firstWord;
  BlockP elementBase;
  BlockP nextBase;
  int freeBlk;
  int heapHole;
  int bitmap;
  unsigned totFree;
  unsigned totUsed;
  unsigned totMaps;
  unsigned holeBlocks;
  unsigned totHole;
  unsigned freeBlocks;
  unsigned usedBlocks;
  unsigned mapsBlocks;
  unsigned largestFreeBlock;
  StorageInfo stat;
  EventInfo eventInfo;
  int eventNo;
} StatsRec, *StatsPtr;

/* static StatsPtr statsP; */

#define INITMUTEX
#define ACQUIREMUTEX
#define RELEASEMUTEX {}
#define RELEASEANDRETURN(value) {RELEASEMUTEX return (value);}

/*
 * This code will use a maximum of 32 words of stack excluding any used by
 * the system storage wholesaler.
 *
 * Turn off stack overflow checking.
 */
#pragma -s1

#if defined(VERBOSE) || defined(STACKCHECK)
static int *stackOnEntryToAlloc;
#define ENTRYTOALLOC(local) stackOnEntryToAlloc = (int *)(&local)
#define STACKDEPTH(local, depth) \
        {if (stackOnEntryToAlloc-(int *)(&local) > (depth))\
         {LOW_OVERHEAD_F0("!! stack ") \
          LOW_OVERHEAD_FD(stackOnEntryToAlloc-(int *)(&local), 10) \
          LOW_OVERHEAD_F0(" words\n")}}
#else
#define ENTRYTOALLOC(local)
#define STACKDEPTH(local, depth)
#endif

void _alloc_die(const char *message, int rc)
{
  /* nb rc is here so that it can be examined - otherwise the C compiler
   * tends to lose a useful value.
   */
  IGNORE(rc);
  _sysdie(message, rc == CORRUPT ? ", (heap corrupt)": "");
}

/*static void bad_size(size_t size)
{
  IGNORE(size);
  _alloc_die("Over-large or -ve size request", FAILED);
}
*/
#ifdef STATS
void print_event(int event)
{
  switch (event) {
    case COALESCE:                      D0("Coalesce success:"); break;
    case EXTENSION:                     D0("Heap Extension  :"); break;
    case COALESCE_AND_EXTENSION:        D0("Coalesce-Extend :"); break;
  }
}

static void MakeEventRec(int thisEvent, Event type, size_t size)
{
  statsP->nextEvent = thisEvent + 1;
  thisEvent %= MAXEVENTS;
  statsP->events[thisEvent].event = type;
  statsP->events[thisEvent].blockThatCausedEvent = size;
  statsP->events[thisEvent].userHeap = statsP->stats.userHeap;
  statsP->events[thisEvent].totalFree = totalFree;
  statsP->events[thisEvent].allocates = statsP->stats.blocksAllocated;
  statsP->events[thisEvent].deallocates = statsP->stats.blocksDeallocated;
  statsP->events[thisEvent].bytesAllocated = statsP->stats.bytesAllocated;
  statsP->events[thisEvent].bytesDeallocated = statsP->stats.bytesDeallocated;
  D0("!!MakeEventRec ");
  print_event(statsP->events[thisEvent].event);
  D1(" blockThatCausedEvent %u\n",
                               statsP->events[thisEvent].blockThatCausedEvent);
  D1("  userHeap %u, ", statsP->events[thisEvent].userHeap);
  D1("totalFree %u, ", statsP->events[thisEvent].totalFree);
  D1("allocates %u, ", statsP->events[thisEvent].allocates);
  D1("deallocates %u\n", statsP->events[thisEvent].deallocates);
  D1("  bytesAllocated %u, ", statsP->events[thisEvent].bytesAllocated);
  D1("bytesDeallocated %u, ", statsP->events[thisEvent].bytesDeallocated);
}

/* ------------------------- Statistics reporting --------------------------*/

extern void _GetStorageInfo(StorageInfoP info)
{
  statsP->stats.currentHeapRequirement = totalHeap - totalFree;
  *info = statsP->stats;
}

extern void _NextHeapElement(BlockP *nextBase, unsigned int *guard, size_t *size,
                             int *free, int *heapHole, int *bitmap, unsigned int *firstWord)
{ BlockP junkBlock;
  if (*nextBase == NULL) {junkBlock = heapLow;} else {junkBlock = *nextBase;}
#ifdef BLOCKS_GUARDED
  *guard = junkBlock->guard;
#else
  *guard = 0;
#endif
  *firstWord = (unsigned int) junkBlock->next;
  *free = FREE(junkBlock);
  if (!*free) {
    if (HEAPHOLEBIT & junkBlock->size) {
      *bitmap = FALSE;
      *heapHole = TRUE;
    } else {
      *heapHole = FALSE; *bitmap = FALSE;
    }
  }
  *size = SIZE(junkBlock);
  ADDBYTESTO(junkBlock, OVERHEAD + *size);
  *nextBase = junkBlock;
  if (*nextBase > heapHigh) *nextBase = NULL;
}

extern int _GetEventData(int event, EventInfoP info)
{ int index;
  int previous;
   if ((event >= statsP->nextEvent) || (event < statsP->nextEvent-MAXEVENTS)
                                                              || (event < 1))
     return FALSE;
   index = event % MAXEVENTS;
   previous = (event-1) % MAXEVENTS;
   *info = statsP->events[index];
   info->allocates -= statsP->events[previous].allocates;
   info->deallocates -= statsP->events[previous].deallocates;
   info->bytesAllocated -= statsP->events[previous].bytesAllocated;
   info->bytesDeallocated -= statsP->events[previous].bytesDeallocated;

   return TRUE;
}

extern int _GetLastEvent(void)
{
  return (statsP->nextEvent-1);
}

static void ShowStats(void)
{
  _GetStorageInfo(&statsP->stat);

  statsP->nextBase = NULL;
  statsP->totFree = 0; statsP->totUsed = 0;
  statsP->totHole = 0; statsP->totMaps = 0;
  statsP->holeBlocks = 0; statsP->freeBlocks = 0;
  statsP->usedBlocks = 0; statsP->mapsBlocks = 0;
  statsP->largestFreeBlock = 0;
  D0("Storage description. (All sizes in bytes)\n");
  D0("Current storage analysis (by traversing heap):");
  do {
    statsP->elementBase = statsP->nextBase;
    _NextHeapElement(&statsP->nextBase, &statsP->guard, &statsP->size,
                    &statsP->freeBlk, &statsP->heapHole, &statsP->bitmap,
                                                       &statsP->firstWord);
    if (statsP->heapHole)
      { statsP->holeBlocks++; statsP->totHole += statsP->size; }
    else if (statsP->freeBlk) {
      if (statsP->size > statsP->largestFreeBlock)
        statsP->largestFreeBlock = statsP->size;
      statsP->freeBlocks++; statsP->totFree += statsP->size;
    } else if (statsP->bitmap)
        {statsP->mapsBlocks++; statsP->totMaps += statsP->size;}
    else {statsP->usedBlocks++; statsP->totUsed += statsP->size;}
  } while (statsP->nextBase != NULL);

  D0("\n");
  D4("Free memory of %d in %d blocks + overhead of %d = %d\n",
     statsP->totFree, statsP->freeBlocks, statsP->freeBlocks*OVERHEAD,
                           statsP->totFree+statsP->freeBlocks*OVERHEAD);
  D1("Largest free block = %d\n", statsP->largestFreeBlock);
  D4("Used memory of %d in %d blocks + overhead of %d = %d\n",
     statsP->totUsed, statsP->usedBlocks, statsP->usedBlocks*OVERHEAD,
                           statsP->totUsed+statsP->usedBlocks*OVERHEAD);
  D4("Memory taken by heap holes = %d in %d blocks + overhead of %d = %d\n",
     statsP->totHole, statsP->holeBlocks, statsP->holeBlocks*OVERHEAD,
                         statsP->totHole + statsP->holeBlocks*OVERHEAD);
  D4("Memory taken by GC bitmaps = %d in %d blocks + overhead of %d = %d\n",
     statsP->totMaps, statsP->mapsBlocks, statsP->mapsBlocks*OVERHEAD,
                         statsP->totMaps + statsP->mapsBlocks*OVERHEAD);
  D1("Current heap requirement (all except user free blocks) = %d\n",
                      (statsP->totHole+statsP->holeBlocks*OVERHEAD) +
                      (statsP->totUsed+statsP->usedBlocks*OVERHEAD) +
                      (statsP->totMaps+statsP->mapsBlocks*OVERHEAD) +
                             (statsP->freeBlocks*OVERHEAD) - OVERHEAD);
  D1("total heap usage = %d\n",
      (statsP->totHole+statsP->holeBlocks*OVERHEAD) +
      (statsP->totUsed+statsP->usedBlocks*OVERHEAD) +
      (statsP->totMaps+statsP->mapsBlocks*OVERHEAD) +
      (statsP->totFree+statsP->freeBlocks*OVERHEAD) - OVERHEAD);
  D0("\n");
  D0("Current storage statistics:\n");
  D3("%d coalesces, %d heap extensions, %d garbage collects\n",
      statsP->stat.coalesces, statsP->stat.heapExtensions,
                                   statsP->stat.garbageCollects);
  D3("Heap base = &%X, heap top = &%X, size of user heap = %d\n",
      (unsigned) statsP->stat.heapLow, (unsigned) statsP->stat.heapHigh,
                                                   statsP->stat.userHeap);
  D2("Maximum storage requested = %d, current storage requested = %d\n",
      statsP->stat.maxHeapRequirement, statsP->stat.currentHeapRequirement);
  D4("total allocated = %d in %d blocks, deallocated = %d in %d\n",
      statsP->stat.bytesAllocated, statsP->stat.blocksAllocated,
      statsP->stat.bytesDeallocated, statsP->stat.blocksDeallocated);
  D0("\n");

  statsP->eventNo = _GetLastEvent();
  D0("Description of past events in storage (most recent first):\n");
  while (_GetEventData(statsP->eventNo, &statsP->eventInfo)) {
    print_event(statsP->eventInfo.event);
    D3(" block size = %d, user heap size %d, %d usable\n",
       statsP->eventInfo.blockThatCausedEvent, statsP->eventInfo.userHeap,
                                               statsP->eventInfo.totalFree);
    D4("   allocated %d in %d, deallocated %d in %d since last event\n",
       statsP->eventInfo.bytesAllocated, statsP->eventInfo.allocates,
       statsP->eventInfo.bytesDeallocated, statsP->eventInfo.deallocates);
    statsP->eventNo--;
  }
}
#endif

#ifdef BLOCKS_GUARDED
extern void __heap_checking_on_all_deallocates(int on)
{
  check.deallocates = on;
}

extern void __heap_checking_on_all_allocates(int on)
{
  check.allocates = on;
}
#endif

static int internal_coalesce(void)
{ BlockP block;
  BlockP previous;
  BlockP tail;
#ifndef BLOCKS_GUARDED
  BlockP bin_copy[NBINS+2];
#endif
  size_t size;
  /* where size is used to specify an element of an array it should really be
   * called index, but to generate better code I got rid of the index variable
   */

  F0("!!internal_coalesce...");
#ifdef STATS
  statsP->stats.coalesces++;
#endif

  lookInBins = FALSE;
  totalFree = 0;
  /* set bins and overflow lists to empty */
  for (size = 0; size <= NBINS+1; size++)
  { bin[size] = NULL;
#ifndef BLOCKS_GUARDED
    bin_copy[size] = NULL;
#endif
  }

  block = heapLow;

  /* NULL indicates previous doesn't point to start of free block */
  previous = NULL; tail = NULL;

  while (block <= heapHigh) {
    if (INVALID(block)) return CORRUPT;
    if (FREE(block)) { /* free block */
      if (previous == NULL) previous = block;
    } else if (previous != NULL) {
      size = PTRDIFF(block, previous) - OVERHEAD;
      /* set flags to Free */
      totalFree += size;
      previous->size = (size | FREEBIT);
      if (size <= MAXBINSIZE) { /* return to bin */
        size /= BINRANGE;
        if (bin[size] == NULL) bin[size] = previous;
        else {
          /* if not BLOCKS_GUARDED use guard word of first block in bin to hold
           * a pointer to the last block in the list for this bin otherwise
           * use the bin_copy array. This allows me to keep the list in
           * ascending address order. Remember to put back the guard words at
           * the end of coalescing if BLOCKS_GUARDED.
           */
#ifdef BLOCKS_GUARDED
          ((BlockP) bin[size]->guard)->next = previous;
#else
          (bin_copy[size])->next = previous;
#endif
        }
#ifdef BLOCKS_GUARDED
        bin[size]->guard = (int) previous;
#else
        bin_copy[size] = previous;
#endif
      } else { /* put block on overflow list */
        if (bin[0] == NULL)
          {bin[0] = previous; previous->previous = NULL;}
        else
          {tail->next = previous; previous->previous = tail;}
        tail = previous;
      }
      previous = NULL;
    }
    ADDBYTESTO(block, SIZE(block) + OVERHEAD);
  }

  /* replace the guard words at the start of the bins lists */
  for (size = 1; size <= NBINS; size++) {
    if (bin[size] != NULL) {
      lookInBins = TRUE;
#ifdef BLOCKS_GUARDED
      ((BlockP) bin[size]->guard)->next = NULL;
      bin[size]->guard = GUARDCONSTANT;
#else
      (bin_copy[size])->next = NULL;
#endif
    }
  }

  /* do both ends of overflow list */
  if (bin[0] != NULL) {
    tail->next = NULL;
    bin[NBINS+1] = tail;
  } else { bin[NBINS+1] = NULL; }
  lastFreeBlockOnHeap = bin[NBINS+1];

  F0(" ... complete\n");
  return OK;
}

static int InsertBlockInOverflowList(BlockP block)
{
#if HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER
  F0("!!InsertBlockInOverflowList &")
  FD((unsigned)block, 16)
  F0(" at end of list\n")
  /* OK to add remainder of block to tail of overflow list */
  if (bin[0] == NULL) {bin[0] = block; block->previous = NULL;}
  else {bin[NBINS+1]->next = block; block->previous = bin[NBINS+1];}
  bin[NBINS+1] = block; block->next = NULL;
#else
  BlockP previous;
  BlockP tail;
  F0("!!InsertBlockInOverflowList &")
  FD((unsigned)block, 16);
  if (bin[0] == NULL) {
    F0(" at end of list\n");
    /* OK to add remainder of block to tail of overflow list */
    if (bin[0] == NULL) {bin[0] = block; block->previous = NULL;}
    else {bin[NBINS+1]->next = block; block->previous = bin[NBINS+1];}
    bin[NBINS+1] = block; block->next = NULL;
  } else {
    /* insert remainder block at right position in overflow list */
    F0(" walk chain to determine where\n");
    tail = bin[0];
    while (tail != NULL && tail < block) {
      if (INVALID(tail)) return CORRUPT;
      previous = tail; tail = tail->next;
    }
    if (tail == bin[0]) {
      block->next = bin[0]; block->previous = NULL;
      bin[0]->previous = block; bin[0] = block;
    } else {
      block->next = previous->next; block->previous = previous;
      previous->next = block;
      if (tail == NULL) bin[NBINS+1] = block; else tail->previous = block;
    }
  }
#endif
  return OK;
}

static int GetMoreOSHeap(size_t minSize, BlockP *base_ptr, size_t *size_ptr)
{ size_t size = *size_ptr;
  BlockP base = *base_ptr;
#if !HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER
  BlockP tempBlock;
#endif
  BlockP bitmap;
  int gotWhatWasWanted;
#ifdef STACKCHECK
  LOW_OVERHEAD_F0("stack on entry to GetMoreOSHeap = &")
  LOW_OVERHEAD_FD((unsigned) &gotWhatWasWanted, 16)
  LOW_OVERHEAD_F0("\n");
  STACKDEPTH(gotWhatWasWanted, 20);
#endif

#ifdef STATS
  if (statsP != NULL) statsP->stats.heapExtensions++;
#endif
  minSize += OVERHEAD + HOLEOVERHEAD;
  if (userHeap/FRACTION_OF_HEAP_NEEDED_FREE > totalFree)
    minSize += userHeap / FRACTION_OF_HEAP_NEEDED_FREE - totalFree;
  F0("!!GetMoreOSHeap: ") FD(minSize, 10)
  F0(" bytes, old heap top ")
  FD((unsigned)endOfLastExtension, 16) F0("\n")

  base = endOfLastExtension;

  size = __rt_alloc(BYTESTOWORDS(minSize),(void **)&base) * BYTESPERWORD;
  F0("!!size = ") FD(size, 10)
  F0(" bytes, base = ")
  FD((unsigned)base, 16) F0("\n")
  if (base == ADDBYTES(endOfLastExtension, HOLEOVERHEAD)) {
    base = endOfLastExtension;
    size += HOLEOVERHEAD;
  }
  gotWhatWasWanted = (size >= minSize);
  if (size <= HOLEOVERHEAD) {size = 0; base = NULL;}
  else size -= HOLEOVERHEAD;
  F0("  got ") FD(size, 10)
  F0(" at &")
  FD((unsigned)base, 16) F0("\n")

  bitmap = base;
  if (base == endOfLastExtension) {
    /* extension contiguous with last block on heap. */
    if (lastFreeBlockOnHeap != NULL &&
            ADDBYTES(lastFreeBlockOnHeap,
                          SIZE(lastFreeBlockOnHeap)+OVERHEAD) == bitmap) {
      /* so do the merge of the extension and last block on the heap */
      lastFreeBlockOnHeap->size = SIZE(lastFreeBlockOnHeap) + OVERHEAD;
      totalFree -= lastFreeBlockOnHeap->size;
      size += lastFreeBlockOnHeap->size;

      bitmap = lastFreeBlockOnHeap;
      if (lastFreeBlockOnHeap == bin[NBINS+1]) {
        /* remove block from end of overflow list */
        if (lastFreeBlockOnHeap->previous == NULL) bin[0] = NULL;
        else lastFreeBlockOnHeap->previous->next = NULL;
        bin[NBINS+1] = lastFreeBlockOnHeap->previous;
      } /* else it is not in any list ie waiting for coalesce */
    }
  }

  /* SEE WHAT TO DO WITH NEW BLOCK (IF THERE IS ONE) */
  if (size > MAXBINSIZE+OVERHEAD) {
    F0("\n");
    /* block is big enough to do something with */
    /* HANDLE BEING DROPPED INTO A HEAP HOLE, AND CREATING THE HEAP HOLE
       MARKER AT THE END OF THE NEW EXTENSION BLOCK. */
    if (base >= heapHigh) {
      if (endOfLastExtension != NULL && base != endOfLastExtension) {
        /* heap hole, mark it as allocated */
        F0("  extension not contiguous with heap, heap hole created\n");
        endOfLastExtension->size =
                 (PTRDIFF(base, endOfLastExtension) - HOLEOVERHEAD) | HEAPHOLEBIT;
      } else F0("  extension contiguous with heap\n");
      endOfLastExtension = ADDBYTES(bitmap, size);
#ifdef BLOCKS_GUARDED
      endOfLastExtension->guard = GUARDCONSTANT;
#endif
      endOfLastExtension->size = 0; /* as an end marker for Coalesce */
    }
#if !HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER
      else { /* find the heap hole I've been dropped in and modify it */
      BlockP holeStart;
      BlockP hole;
      F0("  extension is in a heap hole\n");
      hole = heapLow; holeStart = NULL;
      while (hole <= base) {
        if (HEAPHOLE(hole)) holeStart = hole;
        ADDBYTESTO(hole, SIZE(hole)+OVERHEAD);
      }
      if (holeStart != base) /* extension is NOT at start of heap hole */
        holeStart->size = PTRDIFF(base, holeStart) - HOLEOVERHEAD | HEAPHOLEBIT;
      else if (ADDBYTES(holeStart, HOLEOVERHEAD) == base) {
        base = holeStart;
        size += HOLEOVERHEAD;
      }
      if (ADDBYTES(base, size+HOLEOVERHEAD) == hole) size += HOLEOVERHEAD;
      else { /* create a new hole at the end of the extension */
        tempBlock = ADDBYTES(base ,size);
#ifdef BLOCKS_GUARDED
        tempBlock->guard = GUARDCONSTANT;
#endif
        tempBlock->size = (PTRDIFF(hole, tempBlock) - HOLEOVERHEAD) | HEAPHOLEBIT;
      }
    }
#endif /* EXTENSIONS_IN_HEAP_HOLES */

    /* INITIALISE HEADER OF NEW BLOCK */
    base = bitmap;
    if (base > lastFreeBlockOnHeap) lastFreeBlockOnHeap = base;
    size -= OVERHEAD;
#ifdef BLOCKS_GUARDED
    base->guard = GUARDCONSTANT;
#endif
    /* set flags to Free */
    base->size = size | FREEBIT;
    totalFree += size;
    if (!gotWhatWasWanted) {
      F0("  extension too small, ");
      if (InsertBlockInOverflowList(base) != OK) return FAILED;
    }
  } else /* block is not big enough to worry about, throw it away */
    F0(", no heap extension\n");

  /* endOfLastExtension is the address of the storage after the end of the
     block (used to handle heap holes) */
  if (endOfLastExtension > heapHigh) heapHigh = endOfLastExtension;
  if (base < heapLow && base != NULL) heapLow = base;
  totalHeap = PTRDIFF(heapHigh, heapLow);
  userHeap = totalHeap;
#ifdef STATS
  if (statsP != NULL) {
    statsP->stats.userHeap = userHeap;
    statsP->stats.heapLow = heapLow;
    statsP->stats.heapHigh = heapHigh;
  }
#endif

  *size_ptr = size;
  *base_ptr = base;
  if (gotWhatWasWanted) return OK; else return FAILED;
}

#ifdef BLOCKS_GUARDED
static int check_heap(void)
{ BlockP block;
  if (userHeap > 0) {
    for (block = heapLow; ; ) {
      if (block >= heapHigh) {
        if (block > ADDBYTES(heapHigh,OVERHEAD)) return CORRUPT;
        else return OK;
      }
      if (INVALID(block)) return CORRUPT;
      ADDBYTESTO(block, SIZE(block)+OVERHEAD);
    }
  }
  return OK;
}
#endif

#define COALESCED     (1U<<31)
#define FORCECOALESCE (1U<<30)

static int _primitive_alloc(size_t size/*words*/)
{ BlockP block;
  size_t actualSize;
  register int index;
  int fromHighMemory;
  unsigned status = 0;

  ACQUIREMUTEX;
#ifdef BLOCKS_GUARDED
  if (check.allocates && check_heap() != OK) RELEASEANDRETURN(CORRUPT)
#endif
  /* convert size from words to addresss units */
  size *= BYTESPERWORD;
  F0("!!primitive_alloc: size ")
  FD(size, 10)
  F0(" bytes")
  if (size >= MAXBYTES) RELEASEANDRETURN(FAILED)
  else if (size == 0) RELEASEANDRETURN(NULL)

  index = 0;
  fromHighMemory = ((size <= LARGEBLOCK) && sys_heap_top);
  for (;;) {
    if (size <= MAXBINSIZE && lookInBins) { /* get from bin (if not empty) */
      F0("  looking in bins");
      index = size / BINRANGE;
      do {
        block = bin[index];
        if (block != NULL) { /* got a block */
          if (INVALID(block)) RELEASEANDRETURN(CORRUPT)
          bin[index] = block->next;
          actualSize = SIZE(block);
          F0(" ");
          FD(index, 10);
          goto got_block;
        } /* else try other bins */
      } while (++index <= NBINS);
    }

    /* block bigger than largest bin / bin is empty, check overflow list */
    /* if large block required, take it from high memory otherwise from low */
get_from_overflow:
    F0("  looking in overflow ");
    if (fromHighMemory) {block = bin[NBINS+1]; F0("<");}
    else {block = bin[0]; F0(">");}

    while (block != NULL) {
      if (INVALID(block)) RELEASEANDRETURN(CORRUPT)
      actualSize = SIZE(block);
      if (actualSize >= size) {
        /* got a block big enough, now see if it needs splitting */
        if (actualSize-size <= MAXBINSIZE+OVERHEAD) {
          /* remove all of block from overflow list */
          if (block == lastFreeBlockOnHeap) lastFreeBlockOnHeap = NULL;
          if (block->previous == NULL) bin[0] = block->next;
          else block->previous->next = block->next;
          if (block->next == NULL) bin[NBINS+1] = block->previous;
          else block->next->previous = block->previous;
          goto got_block;
        } else { /* split and leave unwanted part of the block in list */
          goto split_block;
        }
      } else {
          if (fromHighMemory) {block = block->previous; F0("<");}
          else {block = block->next; F0(">");}
      }
    }
    F0("\n");

    /* no block in bin or overflow list, try coalesce if desirable */
    if (!(COALESCED & status) &&
         ((totalFree > (size + 4096) &&
           totalFree > userHeap/FRACTION_OF_HEAP_NEEDED_FREE)
         || FORCECOALESCE & status)) {
#ifdef STATS
      MakeEventRec(statsP->nextEvent, COALESCE, size);
#endif
      if (internal_coalesce() != OK) RELEASEANDRETURN(CORRUPT)
      status |= COALESCED;
      continue; /* try the allocation again */
    } else
      /* no block available in Storage, must go to OSStorage to get one */

#ifdef STATS
    if (COALESCED & status) {
      MakeEventRec(statsP->nextEvent-1, COALESCE_AND_EXTENSION, size);
    } else if (heapHigh > heapLow)
        MakeEventRec(statsP->nextEvent, EXTENSION, size);
#endif

    { BlockP blockCopy;
      size_t actual;
      /* now we have to get more heap */
      switch (GetMoreOSHeap(size, &blockCopy, &actual)) {
        case OK:
          block = blockCopy; actualSize = actual;
          if (InsertBlockInOverflowList(block) != OK) RELEASEANDRETURN(CORRUPT)
          goto get_from_overflow;
        case FAILED:
          block = blockCopy; actualSize = actual;
          if (!enoughMemoryForGC) {
            if (FORCECOALESCE & status) RELEASEANDRETURN(FAILED)
            else status |= FORCECOALESCE;
          }
          enoughMemoryForGC = FALSE;
          break;
        case CORRUPT:
          D0("**Heap CORRUPT getting more OS heap\n");
          return CORRUPT;
#ifdef DEBUG
        default: _alloc_die("internal error: bad switch selector", FAILED);
#endif
      }
    }
  }

got_block:
  if (fromHighMemory && (actualSize > size+MINBLOCKSIZE)) {
    /* split and put unwanted part of block into a bin or on overflow list*/
split_block:
    F0(", got block ")
    FD((unsigned)block, 16)
    F0(" to split, ")
    { BlockP tempBlock = block;
      totalFree -= OVERHEAD;
      /* large block taken from bottom of this block */
      /* medium and small blocks (and bitmaps) taken off top of this block */
      if ((size > LARGEBLOCK) || (!sys_heap_top)) ADDBYTESTO(tempBlock, size+OVERHEAD);
      else ADDBYTESTO(block, actualSize-size);
      block->size = size;
      /* set flags on block to Free */
      size = actualSize - (size + OVERHEAD);
      tempBlock->size = size | FREEBIT;

      if (!fromHighMemory) {
      /* The block has been cut from the start of the overflow block.
         This means that the large block that was in the overflow list
         has to be replaced with new one (tempBlock).
       */
        tempBlock->previous = block->previous;
        tempBlock->next = block->next;
        if (tempBlock->previous == NULL) bin[0] = tempBlock;
        else tempBlock->previous->next = tempBlock;
        if (tempBlock->next == NULL) bin[NBINS+1] = tempBlock;
        else tempBlock->next->previous = tempBlock;
      }
#ifdef BLOCKS_GUARDED
      tempBlock->guard = GUARDCONSTANT;
#endif

      if (size <= MAXBINSIZE) {
        /* work out the bin number */
        lookInBins = TRUE;
        index = size / BINRANGE;
        F0("remainder --> bin ") FD(index, 10)
        F0("\n")
        tempBlock->next = bin[index]; bin[index] = tempBlock;
      } else
        F0("remainder --> overflow list\n");
    }
  } else  /* no split, take the whole block */
    F0("no split, take the lot\n");

  size = SIZE(block);
#ifdef ANALYSIS
  LOW_OVERHEAD_F0("+") LOW_OVERHEAD_FD(size, 10) LOW_OVERHEAD_F0("\n")
#endif
  F0("  new allocated block at &") FD((unsigned) block, 16)
  F0(", size ") FD(size, 10) F0("\n")
  /* set flags to not Free, and gcbits */
  block->size = size;
#ifdef BLOCKS_GUARDED
  block->guard = GUARDCONSTANT;
#endif
  totalFree -= size;
  if (bin[NBINS+1] > lastFreeBlockOnHeap) lastFreeBlockOnHeap = bin[NBINS+1];
#ifdef STATS
  if (statsP != NULL) {
    statsP->stats.blocksAllocated++;
    statsP->stats.bytesAllocated += size;
    if (totalHeap-totalFree > statsP->stats.maxHeapRequirement)
      statsP->stats.maxHeapRequirement = totalHeap-totalFree;
  }
#endif
  ADDBYTESTO(block, OVERHEAD);
  RELEASEANDRETURN((int)block)
}

int _primitive_dealloc(BlockP block)
{ int size;
  ACQUIREMUTEX;
  F0("!!primitive_dealloc: block ")
  FD((unsigned)block, 16);  F0("\n")

  if ((block <= heapLow) || (block >= heapHigh)) {
    if (block == NULL) RELEASEANDRETURN(OK)
    else RELEASEANDRETURN(FAILED)
  }
  ADDBYTESTO(block, -OVERHEAD);

#ifdef BLOCKS_GUARDED
  if (check.deallocates) {
    BlockP searchBlock = heapLow;
    for (; searchBlock != block; ) {
      if (searchBlock >= heapHigh) RELEASEANDRETURN(FAILED)
      if (INVALID(searchBlock)) RELEASEANDRETURN(CORRUPT)
      ADDBYTESTO(searchBlock, OVERHEAD + SIZE(searchBlock));
    }
  }

  if (INVALID(block)) RELEASEANDRETURN(CORRUPT)
#endif
  size = block->size;
  if (FREEBIT & size) RELEASEANDRETURN(FAILED)
  /* set flags to Free */
  size &= SIZEMASK;
#ifdef ANALYSIS
  LOW_OVERHEAD_F0("-") LOW_OVERHEAD_FD(size, 10) LOW_OVERHEAD_F0("\n")
#endif
  block->size = size | FREEBIT;
#ifdef STATS
  statsP->stats.blocksDeallocated++;
  statsP->stats.bytesDeallocated += size;
#endif
  totalFree += size;

  if (size <= MAXBINSIZE) { /* return to bin */
    lookInBins = TRUE; size /= BINRANGE;
    block->next = bin[size]; bin[size] = block;
  } else {
    /* put block on deallocate overflow list, for reuse after coalesce */
    if (block > lastFreeBlockOnHeap) lastFreeBlockOnHeap = block;
  }

  RELEASEANDRETURN(OK)
}

extern size_t _byte_size(void *p)
{ BlockP block = (BlockP)p;
  if (block != NULL) {
    /* decrement the pointer (block) by the number of overhead bytes */
    ADDBYTESTO(block, -OVERHEAD);
    if (!INVALID(block)) return (SIZE(block));
  }
  return 0;
}

extern void *malloc(size_t size)
{ void *ptr;

  ENTRYTOALLOC(ptr);
  ptr = (void *)_primitive_alloc(BYTESTOWORDS(size));
  if ((int)ptr == FAILED || (int)ptr == CORRUPT) 
  {
#ifdef STATS
    ShowStats();
#endif
    if ((int)ptr == CORRUPT) _alloc_die("malloc failed", CORRUPT);
    else return NULL;
  }
  return ptr;
}

extern void free(void *p)
{ int rc = _primitive_dealloc((BlockP)p);
  /* following line may not be correct ANSI - but for the moment we
   * have problems if we don't detect invalid free's.
   */
  if (rc != OK) {
#ifdef STATS
    ShowStats();
#endif
    _alloc_die("free failed", rc);
  }
}

/*
 * End of veneer functions
 *
 * Garbage collection interface.
 */

extern int __coalesce(void)
{ int rc;
  ACQUIREMUTEX;
  rc = internal_coalesce();
  RELEASEMUTEX;
  if (rc != OK) {
#ifdef STATS
    ShowStats();
#endif
    _alloc_die("_coalesce failed", rc);
  }
  return rc;
}

#ifdef STATS
static void init_stats(void)
{
  /* grab stats record from heap */
  statsP = (StatsPtr) _primitive_alloc(BYTESTOWORDS(sizeof(StatsRec)));
  statsP->stats.coalesces = 0; statsP->stats.heapExtensions = 0;
  statsP->stats.heapHigh = heapHigh; statsP->stats.heapLow = heapLow;
  statsP->stats.userHeap = userHeap; statsP->stats.maxHeapRequirement = 0;
  statsP->stats.blocksAllocated = 0; statsP->stats.bytesAllocated = 0;
  statsP->stats.blocksDeallocated = 0; statsP->stats.bytesDeallocated = 0;
  statsP->events[0].allocates = 0; statsP->events[0].deallocates = 0;
  statsP->events[0].bytesAllocated = 0; statsP->events[0].bytesDeallocated = 0;
  statsP->nextEvent = 1;
}
#endif

extern void _terminate_user_alloc(void)
{
  heapLow = sys_heap_top;
}

extern void _init_user_alloc(void)
{
  sys_heap_top = heapLow;
  heapLow = bin[0];
  totalHeap = PTRDIFF(heapHigh, heapLow);
  userHeap = totalHeap;
}

extern void _init_alloc(void)
{ int j;
  INITMUTEX;
  lastFreeBlockOnHeap = NULL;
  mapForExistingHeap = NULL;
  check.deallocates = FALSE;
  check.allocates = FALSE;
  enoughMemoryForGC = TRUE;
  /* to get rid of warnings */
  mapForExistingHeap = NULL;
  mapForNewHeap = NULL;
  endOfExistingHeap = NULL;
  startOfNewHeap = NULL;
  lookInBins = FALSE;
  totalFree = 0;
  endOfLastExtension = NULL;
  /* set allocate bins and overflow lists to empty */
  for (j=0; j <= NBINS+1; ++j) { bin[j] = NULL; }
  totalHeap = 0; userHeap = 0;
#ifdef STATS
  statsP = NULL;
  init_stats();
#endif
  sys_heap_top = heapHigh = 0; heapLow = (BlockP) 0x7fffffff;
  __rt_register_allocs(&malloc, &free);
}

#endif /* alloc_c */

#if defined realloc_c || defined SHARED_C_LIBRARY

void *realloc(void *p, size_t size)
{ int rc;
  size_t oldsize;
  void *newb = NULL;

  size = BYTESTOWORDS(size)*BYTESPERWORD;
  if (p == NULL) return malloc(size);
  if (BADUSERBLOCK(p)) _alloc_die("realloc failed, (bad user block)", FAILED);

  oldsize = _byte_size(p);
  if (oldsize < size) {
    newb = malloc(size);
    if (newb == NULL) return NULL;
    memcpy(newb, p, oldsize);   /* copies 0 words for bad p! */
  }

  if ((oldsize < size) || (size == 0) ||
      (oldsize > size+MINBLOCKSIZE+BYTESPERWORD)) {
    if ((oldsize > size+MINBLOCKSIZE+BYTESPERWORD) && (size != 0)) {
      BlockP b = ADDBYTES(p, -OVERHEAD);
      b->size = size+BYTESPERWORD | (b->size&(!SIZEMASK));
      newb = p;
      ADDBYTESTO(b, size+BYTESPERWORD+OVERHEAD);
#ifdef BLOCKS_GUARDED
      b->guard = GUARDCONSTANT;
#endif
      b->size = (oldsize-OVERHEAD-BYTESPERWORD-size);
      p = ADDBYTES(b, OVERHEAD);
    }
    rc = _primitive_dealloc((BlockP) p);
    if (rc != OK) {
#ifdef STATS
      ShowStats();
#endif
      _alloc_die("deallocate of old block in realloc failed", rc);
    }
    return newb;
  } else
    return p;
}

#endif

#if defined calloc_c || defined SHARED_C_LIBRARY

static void bad_size(size_t size)
{
  size = size;
  _alloc_die("Over-large or -ve size request", FAILED);
}

extern void *calloc(size_t count, size_t size)
{ void *r;
/*
 * This miserable code computes a full 64-bit product for count & size
 * just so that it can verify that the said product really is in range
 * for handing to malloc.
 */
  unsigned h = (count>>16)*(size>>16);
  unsigned m1 = (count>>16)*(size&0xffff);
  unsigned m2 = (count&0xffff)*(size>>16);
  unsigned l = (count&0xffff)*(size&0xffff);
  h += (m1>>16) + (m2>>16);
  m1 = (m1&0xffff) + (m2&0xffff) + (l>>16);
  l = (l&0xffff) | (m1<<16);
  h += m1>>16;
  if (h) l = (unsigned)(-1);
  if (l >= MAXBYTES) bad_size(l);
  r = malloc(l);
#ifdef GC
  /* if garbage collecting, the block will already have been zeroed */
  if ((r != NULL) && (!garbageCollecting)) memset(r, 0, l);
#else
  if (r != NULL) memset(r, 0, l);
#endif
  return r;
}

#endif

