/* armflat.c - Fast ARMulator memory interface.
 * Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved.
 *
 * RCS $Revision: 1.33.2.1 $
 * Checkin $Date: 1997/07/28 14:25:06 $
 * Revising $Author: mwilliam $
 */

#include <string.h>             /* for memset */
#include "armdefs.h"
#include "armcnf.h"

#define ModelName (tag_t)"Flat"

#define NUMPAGES 64 * 1024
#define PAGESIZE 64 * 1024
#define PAGEBITS 16
#define OFFSETBITS 0xffff
#define OFFSETBITS_WORD 0xfffc

typedef struct {
  ARMword memory[PAGESIZE/4];
} mempage;

typedef struct {
  mempage *page[NUMPAGES];
} memory;

typedef struct {
  int bigendSig;
  unsigned int sa_memacc_flag;
  ARMul_Cycles cycles;
  memory mem;
  double clk;
} toplevel;

/*
 * ARMulator callbacks
 */

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  toplevel *top=(toplevel *)handle;
  IGNORE(old);
  top->bigendSig=((new & MMU_B) != 0);
}

static void Interrupt(void *handle,unsigned int which)
{
  toplevel *top=(toplevel *)handle;
  if (which & ARMul_InterruptUpcallReset) {
    top->cycles.NumNcycles=0;
    top->cycles.NumScycles=0;
    top->cycles.NumIcycles=0;
    top->cycles.NumCcycles=0;
    top->cycles.NumFcycles=0;
  }
}

/*
 * Initialise the memory interface
 */

static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config);

ARMul_MemStub ARMul_Flat = {
  MemInit,
  ModelName
  };

/*
 * Predeclare the memory access functions so that the initialise function
 * can fill them in
 */
static int MemAccess(void *,ARMword,ARMword *,ARMul_acc);
static int MemAccessCached(void *,ARMword,ARMword *,ARMul_acc);
static int MemAccessThumb(void *,ARMword,ARMword *,ARMul_acc);
static int MemAccessSA(void *,ARMword,ARMword *,ARMul_acc);
static int MemAccessBL(void *,ARMword,ARMword *,ARMul_acc);
static int MemAccess2(void *,ARMword,ARMword *,ARMword *,ARMul_acc);
static unsigned int DataCacheBusy(void *);
static void MemExit(void *);
static unsigned long ReadClock(void *handle);
static const ARMul_Cycles *ReadCycles(void *handle);
static unsigned long GetCycleLength(void *handle);

static ARMul_Error MemInit(ARMul_State *state,
                           ARMul_MemInterface *interf,
                           ARMul_MemType type,
                           toolconf config)
{
  memory *mem;
  unsigned page;
  ARMword clk_speed = 0;
  const char *option;
  toplevel *top;

  interf->read_clock=ReadClock;
  interf->read_cycles=ReadCycles;

  /* Fill in my functions */
  switch (type) {
  case ARMul_MemType_Basic:
  case ARMul_MemType_16Bit:
    interf->x.basic.access=MemAccess;
    interf->x.basic.get_cycle_length=GetCycleLength;
    break;
  case ARMul_MemType_Thumb:
    interf->x.basic.access=MemAccessThumb;
    interf->x.basic.get_cycle_length=GetCycleLength;
    break;
  case ARMul_MemType_BasicCached:
  case ARMul_MemType_16BitCached:
  case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccessCached;
    interf->x.basic.get_cycle_length=GetCycleLength;
    break;
  case ARMul_MemType_ARM8:
    interf->x.arm8.access=MemAccess;
    interf->x.arm8.get_cycle_length=GetCycleLength;
    interf->x.arm8.access2=MemAccess2;
    ARMul_PrettyPrint(state,", double-bandwidth");
    break;
  case ARMul_MemType_StrongARM:
    interf->x.strongarm.access=MemAccessSA;
    interf->x.strongarm.get_cycle_length=GetCycleLength;
    interf->x.strongarm.data_cache_busy=DataCacheBusy;
    ARMul_PrettyPrint(state,", dual-ported");
    break;
  case ARMul_MemType_ByteLanes:
    interf->x.basic.access=MemAccessBL;
    interf->x.basic.get_cycle_length=GetCycleLength;
    ARMul_PrettyPrint(state,", byte-laned");
    break;
  default:
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }

  top=(toplevel *)malloc(sizeof(toplevel));
  if (top == NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);
  mem=&top->mem;

  option=ToolConf_Lookup(config,ARMulCnf_MCLK);
  if (option != NULL) clk_speed = ToolConf_Power(option,FALSE);
  if (option == NULL || clk_speed == 0) {
    top->clk = 1.0;
  } else {
    top->clk=1000000.0/clk_speed; /* in microseconds */
  }

  /* only report the speed if "CPU Speed" has been set in the config */
  if (ToolConf_Lookup(config, Dbg_Cnf_CPUSpeed) != NULL) {
    char *fac;
    double clk=ARMul_SIRange(clk_speed,&fac,FALSE);
    ARMul_PrettyPrint(state,", %.1f%sHz",clk,fac);
  }

  for (page=0; page<NUMPAGES; page++) {
    mem->page[page]=NULL;
  }

  ARMul_PrettyPrint(state, ", 4GB");

  top->cycles.NumNcycles=0;
  top->cycles.NumScycles=0;
  top->cycles.NumIcycles=0;
  top->cycles.NumCcycles=0;
  top->cycles.NumFcycles=0;

{
  unsigned long memsize=0;
  option = ToolConf_Lookup(config, Dbg_Cnf_MemorySize);
  if (option) memsize=ToolConf_Power(option,TRUE);
  else memsize=0x80000000;
  ARMul_SetMemSize(state,memsize);
}

  ARMul_InstallConfigChangeHandler(state,ConfigChange,top);
  ARMul_InstallInterruptHandler(state,Interrupt,top);
  ARMul_InstallExitHandler(state,MemExit,top);

  interf->handle=top;

  return ARMulErr_NoError;
}

/*
 * Remove the memory interface
 */

static void MemExit(void *handle)
{
  ARMword page;
  toplevel *top=(toplevel *)handle;
  memory *mem=&top->mem;

  /* free all truly allocated pages */
  for (page=0; page<NUMPAGES; page++) {
    mempage *pageptr= mem->page[page];
    if (pageptr) {
      free((char *)pageptr);
    }
  }

  /* free top-level structure */
  free(top);

  return;
}

static mempage *NewPage(void)
{
  unsigned int i;
  mempage *page=(mempage *)malloc(sizeof(mempage));
  /*
   * We fill the new page with data that, if the ARM tried to execute
   * it, it will cause an undefined instruction trap (whether ARM or
   * Thumb)
   */
  for (i=0;i<PAGESIZE/4;) {
    page->memory[i++]=0xe7ff0010; /* an ARM undefined instruction */
    /* The ARM undefined insruction has been chosen such that the
     * first halfword is an innocuous Thumb instruction (B 0x2)
     */
    page->memory[i++]=0xe800e800; /* a Thumb undefined instruction */
  }
  return page;
}

/*
 * Generic memory interface.
 */

/*
 * This is the most basic memory access function - an ARM6/ARM7 interface. 
 */
static int MemAccess(void *handle,
                     ARMword address,
                     ARMword *data,
                     ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;

  if (acc_ACCOUNT(acc)) {
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) top->cycles.NumScycles++;
      else top->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) top->cycles.NumNcycles++;
    else top->cycles.NumIcycles++;
  }

  pageno=address>>PAGEBITS;
  page=top->mem.page[pageno];

  if (page==NULL) {
    top->mem.page[pageno]=page=NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);

  if (acc==acc_LoadInstrS) {
    *data=*ptr;
    return 1;
  } else if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
        if (HostEndian!=top->bigendSig) address^=3;
        *data = ((unsigned8 *)ptr)[address & 3];
        break;
        
      case BITS_16: {           /* read half-word */
        /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
        /*
         * unsigned16 is always a 16-bit type, but if there is no native
         * 16-bit type (e.g. ARM!) then we can do something a bit more
         * cunning.
         */
        if (HostEndian!=top->bigendSig) address^=2;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        if (HostEndian!=state->bigendSig) address^=2;
        if (address & 2) datum<<=16;
        *data = (datum>>16);
#endif
      }
        break;
        
      case BITS_32:             /* read word */
        *data=*ptr;
        break;

      default:
        return -1;
      }
    } else {
      switch (acc & WIDTH_MASK) {
        /* extract byte */
      case BITS_8:              /* write_byte */
        if (HostEndian!=top->bigendSig) address^=3;
        ((unsigned8 *)ptr)[address & 3]=(unsigned8)(*data);
        break;
        
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2))) = (unsigned16)(*data);
        break;

      case BITS_32:             /* write word */
        *ptr=*data;
        break;

      default:
        return -1;
      }
    }                           /* internal cycle */
  }

  return 1;
}

/*
 * This is the most basic memory access function - an ARM610/ARM710 interface.
 *
 * Optimised for word loads and idle cycles
 */
static int MemAccessCached(void *handle,
                           ARMword address,
                           ARMword *data,
                           ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;

  if (acc==acc_Icycle) {
    top->cycles.NumIcycles++;
    return 1;
  }

  if (acc_ACCOUNT(acc))
    top->cycles.NumScycles++;

  pageno=address>>PAGEBITS;
  page=top->mem.page[pageno];

  if (page==NULL) {
    top->mem.page[pageno]=page=NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);

  if (acc==acc_LoadWordS) {
    *data=*ptr;
    return 1;
  } else if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
        if (HostEndian!=top->bigendSig) address^=3;
        *data = ((unsigned8 *)ptr)[address & 3];
        break;
        
      case BITS_16: {           /* read half-word */
        /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
        /*
         * unsigned16 is always a 16-bit type, but if there is no native
         * 16-bit type (e.g. ARM!) then we can do something a bit more
         * cunning.
         */
        if (HostEndian!=top->bigendSig) address^=2;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        if (HostEndian!=state->bigendSig) address^=2;
        if (address & 2) datum<<=16;
        *data = (datum>>16);
#endif
      }
        break;
        
      case BITS_32:             /* read word */
        *data=*ptr;
        break;

      default:
        return -1;
      }
    } else {
      switch (acc & WIDTH_MASK) {
        /* extract byte */
      case BITS_8:              /* write_byte */
        if (HostEndian!=top->bigendSig) address^=3;
        ((unsigned8 *)ptr)[address & 3]=(unsigned8)(*data);
        break;
        
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2))) = (unsigned16)(*data);
        break;

      case BITS_32:             /* write word */
        *ptr=*data;
        break;

      default:
        return -1;
      }
    }                           /* internal cycle */
  }

  return 1;
}

/*
 * Same function, but optimised for Thumb accesses
 */
static int MemAccessThumb(void *handle,
                          ARMword address,
                          ARMword *data,
                          ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;

  if (acc_ACCOUNT(acc)) {
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) top->cycles.NumScycles++;
      else top->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) top->cycles.NumNcycles++;
    else top->cycles.NumIcycles++;
  }

  pageno=address>>PAGEBITS;
  page=top->mem.page[pageno];

  if (page==NULL) {
    top->mem.page[pageno]=page=NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);

  if (acc==acc_LoadInstr16S) {
    /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
    /*
     * unsigned16 is always a 16-bit type, but if there is no native
     * 16-bit type (e.g. ARM!) then we can do something a bit more
     * cunning.
     */
    if (HostEndian!=top->bigendSig) address^=2;
    *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
    unsigned32 datum;
    datum=*ptr;
    if (HostEndian!=state->bigendSig) address^=2;
    if (address & 2) datum<<=16;
    *data = (datum>>16);
#endif
    return 1;
  } else if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
        if (HostEndian!=top->bigendSig) address^=3;
        *data = ((unsigned8 *)ptr)[address & 3];
        break;
        
      case BITS_16: {           /* read half-word */
        /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
        /*
         * unsigned16 is always a 16-bit type, but if there is no native
         * 16-bit type (e.g. ARM!) then we can do something a bit more
         * cunning.
         */
        if (HostEndian!=top->bigendSig) address^=2;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        if (HostEndian!=state->bigendSig) address^=2;
        if (address & 2) datum<<=16;
        *data = (datum>>16);
#endif
      }
        break;
        
      case BITS_32:             /* read word */
        *data=*ptr;
        break;

      default:
        return -1;
      }
    } else {
      switch (acc & WIDTH_MASK) {
        /* extract byte */
      case BITS_8:              /* write_byte */
        if (HostEndian!=top->bigendSig) address^=3;
        ((unsigned8 *)ptr)[address & 3]=(unsigned8)(*data);
        break;
        
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2))) = (unsigned16)(*data);
        break;

      case BITS_32:             /* write word */
        *ptr=*data;
        break;

      default:
        return -1;
      }
    }                           /* internal cycle */
  }

  return 1;
}

/*
 * This function is used by ARM8. Effectively we model a memory
 * system which can return two words per cycle.
 */

static int MemAccess2(void *handle,
                      ARMword address,
                      ARMword *data,ARMword *data2,
                      ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;
  int words=1;

  if (acc_ACCOUNT(acc)) {
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) top->cycles.NumScycles++;
      else top->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) top->cycles.NumNcycles++;
    else top->cycles.NumIcycles++;
  }

  pageno=address>>PAGEBITS;
  page=top->mem.page[pageno];

  if (page==NULL) {
    top->mem.page[pageno]=page=NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);

  if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
        if (HostEndian!=top->bigendSig) address^=3;
        *data = ((unsigned8 *)ptr)[address & 3];
        break;
        
      case BITS_16: {           /* read half-word */
        /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
        /*
         * unsigned16 is always a 16-bit type, but if there is no native
         * 16-bit type (e.g. ARM!) then we can do something a bit more
         * cunning.
         */
        if (HostEndian!=top->bigendSig) address^=2;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        if (HostEndian!=top->bigendSig) address^=2;
        if (address & 2) datum<<=16;
        *data = (datum>>16);
#endif
      }
        break;
        
      case BITS_32:             /* read word */
        *data=*ptr;
        break;

      case BITS_64:             /* read two words */
        *data=*ptr;
        if ((offset+4) & OFFSETBITS_WORD) { *data2=ptr[1]; words=2; }
        break;

      default:
        return -1;
      }
    } else {
      switch (acc & WIDTH_MASK) {
        /* extract byte */
      case BITS_8:              /* write_byte */
        if (HostEndian!=top->bigendSig) address^=3;
        ((unsigned8 *)ptr)[address & 3]=(unsigned8)(*data);
        break;
        
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2)))=(unsigned16)(*data);
        break;

      case BITS_32:             /* write word */
        *ptr=*data;
        break;

      default:
        return -1;
      }
    }                           /* internal cycle */
  }

  return words;
}

/*
 * A memory model used by StrongARM. This only accounts
 * for cycles on opcode boundaries - i.e. we allow both a data
 * and an instruction fetch on one cycle.
 */

static int MemAccessSA(void *handle,
                       ARMword address,
                       ARMword *data,
                       ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;

  /*
   * On StrongARM there are four types of cycle - we'll reuse
   * the four cycle counters for these:
   *
   *  Instruction fetched, No data fetched      N
   *  Instruction fetched, data fetched         S
   *  No instruction fetched, No data fetched   I
   *  No instruction fetched, data fetched      C
   */

  if (acc_ACCOUNT(acc)) {
    if (acc_OPC(acc)) {
      /* End of cycle - account for access */
      /* This access is either acc_LoadInstrN or acc_NoFetch */
      if (top->sa_memacc_flag) {
        /* data fetched */
        top->sa_memacc_flag=0;
        if (acc_MREQ(acc)) top->cycles.NumScycles++;
        else top->cycles.NumCcycles++;
      } else {
        /* no data fetched */
        if (acc_MREQ(acc)) top->cycles.NumNcycles++;
        else top->cycles.NumIcycles++;
      }
    } else if (acc_MREQ(acc)) { /* should be */
      top->sa_memacc_flag=1;    /* flag */
    }
  }

  pageno=address>>PAGEBITS;
  page=top->mem.page[pageno];

  if (page==NULL) {
    top->mem.page[pageno]=page=NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);

  if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
        if (HostEndian!=top->bigendSig) address^=3;
        *data = ((unsigned8 *)ptr)[address & 3];
        break;
        
      case BITS_16: {           /* read half-word */
        /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
        /*
         * unsigned16 is always a 16-bit type, but if there is no native
         * 16-bit type (e.g. ARM!) then we can do something a bit more
         * cunning.
         */
        if (HostEndian!=top->bigendSig) address^=2;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        if (HostEndian!=state->bigendSig) address^=2;
        if (address & 2) datum<<=16;
        *data = (datum>>16);
#endif
      }
        break;
        
      case BITS_32:             /* read word */
        *data=*ptr;
        break;

      default:
        return -1;
      }
    } else {
      switch (acc & WIDTH_MASK) {
        /* extract byte */
      case BITS_8:              /* write_byte */
        if (HostEndian!=top->bigendSig) address^=3;
        ((unsigned8 *)ptr)[address & 3]=(unsigned8)(*data);
        break;
        
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2)))=(unsigned16)(*data);
        break;

      case BITS_32:             /* write word */
        *ptr=*data;
        break;

      default:
        return -1;
      }
    }                           /* internal cycle */
  }

  return 1;
}

/*
 * Memory access function that supports byte lanes
 */

static int MemAccessBL(void *handle,
                       ARMword address,
                       ARMword *data,
                       ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;

  if (acc_ACCOUNT(acc)) {
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) top->cycles.NumScycles++;
      else top->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) top->cycles.NumNcycles++;
    else top->cycles.NumIcycles++;
  }

  pageno=address>>PAGEBITS;
  page=top->mem.page[pageno];

  if (page==NULL) {
    top->mem.page[pageno]=page=NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);

  if (acc_MREQ(acc)) {
    if (acc_BYTELANE(acc)==BYTELANE_MASK) { /* word */
      if (acc_READ(acc)) *data = *ptr;
      else *ptr = *data;
    } else {
      unsigned32 mask;
      static const unsigned32 masks[] = {
        0x00000000, 0x000000ff,
        0x0000ff00, 0x0000ffff,
        0x00ff0000, 0x00ff00ff,
        0x00ffff00, 0x00ffffff,
        0xff000000, 0xff0000ff,
        0xff00ff00, 0xff00ffff,
        0xffff0000, 0xffff00ff,
        0xffffff00, 0xffffffff,
      };
      mask=masks[acc_BYTELANE(acc)];
      if (acc_READ(acc)) *data = *ptr & mask;
      else *ptr = (*ptr & ~mask) | (*data & mask);
    }                           /* internal cycle */
  }

  return 1;
}

/*
 * Utility functions:
 */

static unsigned long ReadClock(void *handle)
{
  /* returns a us count */
  toplevel *top=(toplevel *)handle;
  double t=((double)(top->cycles.NumNcycles) +
            (double)(top->cycles.NumScycles) +
            (double)(top->cycles.NumIcycles) +
            (double)(top->cycles.NumCcycles))*top->clk;
  return (unsigned long)t;
}

static const ARMul_Cycles *ReadCycles(void *handle)
{
  toplevel *top=(toplevel *)handle;
  top->cycles.Total=(top->cycles.NumNcycles + top->cycles.NumScycles +
                     top->cycles.NumIcycles + top->cycles.NumCcycles);
  return &(top->cycles);
}

static unsigned int DataCacheBusy(void *handle)
{
  IGNORE(handle);
  return FALSE;
}

static unsigned long GetCycleLength(void *handle)
{
  /* Returns the cycle length in tenths of a nanosecond */
  toplevel *top=(toplevel *)handle;
  return (unsigned long)(top->clk*10000.0);
}
