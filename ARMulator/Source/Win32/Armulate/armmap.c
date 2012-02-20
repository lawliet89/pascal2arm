/* armmap.c - Memory model that supports an "armsd.map" file */
/* Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved. */

/*
 * RCS $Revision: 1.14.2.2 $
 * Checkin $Date: 1997/08/06 14:58:01 $
 * Revising $Author: mwilliam $
 */

#include <string.h>
#include <ctype.h>
#include "armdefs.h"
#include "armcnf.h"
#include "dbg_stat.h"

#include "linklist.h"

#define ModelName (tag_t)"MapFile"

typedef struct MemDescr {
  RDI_MemDescr desc;
  /* cycle counters */
  /* @@@ N.B. this table RELIES on the bit positions in acc words.
   * for indexing and for it's size.
   */
#if WIDTH_MASK==0xf && (acc_Nrw | acc_seq)==0x30
  unsigned long access[0x40];   /* number of accesses to this region */
  int counter[0x40];            /* wait states - -ve for "special" */
#else
#  error Code relies on things about ARMul_accs
#endif
  struct MemDescr *next;
} MemDescr;

typedef_LIST(MemDescr);

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
  unsigned int bigendSig;
  MemDescr *desc;
  ARMul_Cycles cycles;
  unsigned long wait_states;    /* counter for wait states */
  int cnt;                      /* cycle-by-cycle counter */
  memory mem;
  ARMul_State *state;

  /* Three values give the clock speed: 
   * clk - clockspeed, as cycle length in us
   * mult/period - integer representation of the same, for determining
   * the waitstates. The clock period is period*mult ns. This is done to
   * try to keep precision in division.
   */
  double clk;
  unsigned long mult,period;

  unsigned long prop;
} toplevel;

#define MAP_COUNTWAIT     0x0001
#define MAP_AMBABUSCOUNTS 0x0002

static const struct {
  tag_t option;
  unsigned long flag;
} MapOption[] = {
  ARMulCnf_CountWaitStates, MAP_COUNTWAIT,
  ARMulCnf_AMBABusCounts, MAP_AMBABUSCOUNTS,
  NULL, 0, NULL
};

/*
 * Callback for installing the memory map.
 * Adds to the HEAD of the list
 */

static int InstallMemDescr(void *handle, RDI_MemDescr *md)
{
  toplevel *top=(toplevel *)handle;
  MemDescr *list;
  int long cnt=1,counter;
  int i;
  ARMul_State *state=top->state;
  unsigned long mult=top->mult,period=top->period;

  list=linklist_new(MemDescr);
  if (list==NULL) return 1;

  if (top->desc==NULL) {
    ARMul_ConsolePrint(state,"Memory map:\n");
  }

  list->next=top->desc;
  top->desc=list;

  list->desc=*md;

  list->desc.limit+=list->desc.start-1;
  list->desc.width+=BITS_8;     /* 0->8 bits, 1->16, 2->32 */

  ARMul_ConsolePrint(state,"\
  %08x..%08x, %.2d-Bit, %c%c%c, wait states:",
                    list->desc.start,list->desc.limit,
                    1<<list->desc.width,
                    (list->desc.access & 4) ? '*' : ' ',
                    (list->desc.access & 2) ? 'w' : '-',
                    (list->desc.access & 1) ? 'r' : '-');

  for (i=0;i<0x40;i++) {
    counter=1;
    switch (acc_WIDTH(i)) {
    case BITS_32:
      counter=((list->desc.width==BITS_8) ? cnt*4 :
               (list->desc.width==BITS_16) ? cnt*2 : cnt);
      if (acc_nSEQ(i) && acc_MREQ(i) && counter &&
          (top->prop & MAP_AMBABUSCOUNTS))
        counter++;              /* AMBA decode cycle for N */
      break;
    case BITS_16:
      counter=((list->desc.width==BITS_8) ? cnt*2 : cnt);
      if (!acc_SEQ(i) && counter && top->prop & MAP_AMBABUSCOUNTS)
        counter++;              /* AMBA decode cycle for N */
      if (acc_READ(i) && acc_SEQ(i) && (list->desc.access & 4) &&
          counter>1) {
        /* latched read possibly */
        counter=-counter;
      }
      break;
    default:
    case BITS_8:
      counter=cnt;
      if (!acc_SEQ(i) && counter && top->prop & MAP_AMBABUSCOUNTS)
         counter++;             /* AMBA decode cycle for N */
      break;
    case 0:
      /* First time round for this width - get the base figure. */
      if (acc_READ(i)) {
        if (list->desc.access & 1) { /* read access okay */
          if (acc_SEQ(i)) {
            cnt=list->desc.Sread_ns*mult;
            cnt=(cnt/period)+((cnt % period) || cnt==0);
          } else {
            cnt=list->desc.Nread_ns*mult;
            cnt=(cnt/period)+((cnt % period) || cnt==0);
          }
        } else {
          cnt=0;
        }
      } else {
        if (list->desc.access & 2) { /* write access okay */
          if (acc_SEQ(i)) {
            cnt=list->desc.Swrite_ns*mult;
            cnt=(cnt/period)+((cnt % period) || cnt==0);
          } else {
            cnt=list->desc.Nwrite_ns*mult;
            cnt=(cnt/period)+((cnt % period) || cnt==0);
          }
        } else {
          cnt=0;
        }
      }
      ARMul_ConsolePrint(state," %c%c",
                        acc_READ(i) ? 'R' : 'W',
                        acc_SEQ(i) ? 'S' : 'N');
      if (cnt) ARMul_ConsolePrint(state,"=%d",cnt-1);
      else ARMul_ConsolePrint(state,"=Abt");
    }
    list->access[i]=0;
    /*
     * normally "counter" will be the number of cycles the access takes, so
     *   "counter-1" is the number of wait states.
     * if this would abort, "counter==0", so wait states becomes -ve - i.e. -1.
     * if a 16-bit sequential read to latched memory, then the value is -ve
     *   number of cycles. "counter-1" could only ==-1 iff counter=-0, i.e. abort,
     *   so these -ve numbers don't overlap with the "abort" signal. The real
     *   number of wait states is "(-counter)-1" i.e. "-(-counter-1)-2"
     */
    list->counter[i]=counter-1; /* number of wait states, or special */
  }

  ARMul_ConsolePrint(state,"\n");

  return 0;
}

/*
 * Callback for telling about memory stats
 */
static const RDI_MemAccessStats *GetAccessStats(void *handle,
                                                RDI_MemAccessStats *stats,
                                                ARMword s_handle)
{
  MemDescr *desc;
  toplevel *top=(toplevel *)handle;

  desc=top->desc;

  while (desc) {
    if (desc->desc.handle==s_handle) {
      int i;

      stats->Nreads=stats->Nwrites=stats->Sreads=stats->Swrites=0;
      stats->ns=stats->s=0;

      for (i=0;i<0x40;i++) {
        unsigned long ns,count;
        count=desc->access[i];
        if (acc_READ(i)) {
          if (acc_SEQ(i))
            stats->Sreads+=count;
          else
            stats->Nreads+=count;
        } else {
          if (acc_SEQ(i))
            stats->Swrites+=count;
          else
            stats->Nwrites+=count;
        }

        if (desc->counter[i]>=0) {
          ns=(unsigned long)((double)(count*(desc->counter[i]+1))*top->clk*1000.0);
        } else {
          ns=(unsigned long)((double)(count)*top->clk*1000.0);
        }

        while (ns>(unsigned long)1e9) {
          ns-=(unsigned long)1e9;
          stats->s++;
        }

        stats->ns+=ns;

        /* I don't think this can go round more than once, but there's no harm
         * being safe - this isn't time critical code */
        while (stats->ns>(unsigned long)1e9) {
          stats->ns-=(unsigned long)1e9;
          stats->s++;
        }
      }

      return stats;
    }
    desc=desc->next;
  }

  return NULL;                  /* not found */
}

/*
 * Function to deal with RDI calls related to memory maps
 */

static int RDI_info(void *handle, unsigned type, ARMword *arg1, ARMword *arg2)
{
  switch (type) {
  case RDIMemory_Access:
    if (GetAccessStats(handle,(RDI_MemAccessStats *)arg1,*arg2))
      return RDIError_NoError;
    else
      return RDIError_NoSuchHandle;
    
  case RDIMemory_Map: {
    int n=(int)*arg2;
    RDI_MemDescr *p=(RDI_MemDescr *)arg1;
    while (--n>=0)
      if (InstallMemDescr(handle,p)!=0)
        return RDIError_Error;
  }
    return RDIError_NoError;

  case RDIInfo_Memory_Stats:
    return RDIError_NoError;

  default:
    /* check for capability messages */
    if (type & RDIInfo_CapabilityRequest)
      switch (type & ~RDIInfo_CapabilityRequest) {
      case RDIMemory_Access:
      case RDIMemory_Map:
      case RDIInfo_Memory_Stats:
        return RDIError_NoError;

      default:
        break;                  /* fall through */
      }
    return RDIError_UnimplementedMessage;
  }
}

/*
 * ARMulator callbacks
 */

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  toplevel *top=(toplevel *)handle;  

  IGNORE(old);

  top->bigendSig=((new & MMU_B) != 0);
}

/*
 * Initialise the memory interface
 */

static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config);

ARMul_MemStub ARMul_MapFile = {
  MemInit,
  ModelName
  };

/*
 * Predeclare the memory access functions so that the initialise function
 * can fill them in
 */
static int MemAccess(void *,ARMword,ARMword *,ARMul_acc);
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
  unsigned page, i;
  unsigned long clk;
  ARMword clk_speed = 0;
  const char *option;
  toplevel *top;
  long nano_mult;
  
  /* Fill in my functions */
  switch (type) {
  case ARMul_MemType_Basic: case ARMul_MemType_BasicCached:
  case ARMul_MemType_16Bit: case ARMul_MemType_16BitCached:
  case ARMul_MemType_Thumb: case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccess;
    interf->x.basic.get_cycle_length=GetCycleLength;
    break;
  default: {
    /* Cannot support .map files for this type of memory system,
     * so we default to whatever we can. */
    extern ARMul_MemStub *ARMul_Memories[];
    if (ARMul_Memories[0] && ARMul_Memories[0]->init &&
        ARMul_Memories[0]->init!=MemInit) {
      ARMul_ConsolePrint(state,"Warning: '.map' file will be ignored\n");
      return ARMul_Memories[0]->init(state,interf,type,config);
    } else {
      return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,
                              ModelName);
    }
  }
  }

  interf->read_clock=ReadClock;
  interf->read_cycles=ReadCycles;

  top=(toplevel *)malloc(sizeof(toplevel));
  if (top == NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);
  mem=&top->mem;

  top->state=state;

  ARMul_PrettyPrint(state,", Memory map");

  top->desc=NULL;

  option=ToolConf_Lookup(config,ARMulCnf_MCLK);
  if (option != NULL) clk_speed = ToolConf_Power(option, FALSE);
  if (option == NULL || clk_speed == 0) {
    top->mult = 1;
    top->period = 1000;
    top->clk = 1.0;
  } else {
    clk=clk_speed/10000; clk=clk ? clk : 1;
    clk=100000000/clk;            /* in picoseconds */

    /* shrink this to four sig. figures */
    nano_mult=1000;               /* nanoseconds->picoseconds */
    while ((clk>=10000 || (clk/10)*10==clk) && nano_mult>10) {
      clk/=10;                    /* range reduce */
      nano_mult/=10;
    }
    top->mult=nano_mult; top->period=clk;

    top->clk=1000000.0/clk_speed; /* in microseconds */
  }

  /* only report the speed if "CPU Speed" has been set in the config */
  if (ToolConf_Lookup(config, Dbg_Cnf_CPUSpeed) != NULL) {
    char *fac;
    double clk=ARMul_SIRange(clk_speed,&fac,FALSE);
    ARMul_PrettyPrint(state,", %.1f%sHz",clk,fac);
  }

  top->prop = 0;
  for (i=0; MapOption[i].option!=NULL; i++) {
    const char *option=ToolConf_Lookup(config, MapOption[i].option);
    if (option)
      top->prop = ToolConf_AddFlag(option, top->prop, MapOption[i].flag, TRUE);
  }

  for (page=0; page<NUMPAGES; page++) {
    mem->page[page]=NULL;
  }

  top->cycles.NumNcycles=0;
  top->cycles.NumScycles=0;
  top->cycles.NumIcycles=0;
  top->cycles.NumCcycles=0;
  top->cycles.NumFcycles=0;
  top->wait_states=0;

  top->cnt=0;

  ARMul_PrettyPrint(state, ", 4GB");

{
  unsigned long memsize=0;
  option=ToolConf_Lookup(config,Dbg_Cnf_MemorySize);
  if (option) memsize=ToolConf_Power(option,TRUE);
  else memsize=0x80000000;
  ARMul_SetMemSize(state,memsize);
}

  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,top);
  ARMul_InstallConfigChangeHandler(state,ConfigChange,top);
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
  MemDescr *desc,*next;

  /* free all truly allocated pages */
  for (page=0; page<NUMPAGES; page++) {
    mempage *pageptr= mem->page[page];
    if (pageptr) {
      free((char *)pageptr);
    }
  }

  for (desc=top->desc; desc; desc=next) {
    next=desc->next;
    linklist_free(MemDescr,desc);
  }

  /* free top-level structure */
  free(top);
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
 * Generic memory interface. Just alter this for a memtype memory system
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
    if (top->cnt==0) {
      /* first cycle of the access */
      MemDescr *desc;
      
      for (desc=top->desc; desc; desc=desc->next) {
        if (address>=desc->desc.start &&
            address<=desc->desc.limit) {

          desc->access[acc & (WIDTH_MASK | acc_seq | acc_Nrw)]++;
          top->cnt=desc->counter[acc & (WIDTH_MASK | acc_seq | acc_Nrw)];

          if (top->cnt) {
            if (top->cnt<0) {
              if (top->cnt==-1) {/* abort */
                top->cnt=0;
                return -1;
              }
              /*
               * otherwise, this is a 16-bit SEQ read
               * - takes 1 cycle if an even address,
               * - takes -cnt-2 cycles otherwise.
               */
              if (address & 1) top->cnt=-top->cnt-2;
              else break;       /* i.e. fall through - 1 cycle */
            }
            top->wait_states++;
            return 0;
          }
          break;                /* fall through */
        }
      }
      if ((top->prop & MAP_AMBABUSCOUNTS) &&
          acc_MREQ(acc) && acc_nSEQ(acc)) {
        /* needs an address decode cycle */
        top->cnt = 1;
        top->wait_states++;
        return 0;
      }
      /* no memory desc found - fall through */
    } else {
      /* not the first cycle */
      if (--top->cnt) {
        top->wait_states++;
        return 0;
      }
      /* else fall through */
    }
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) top->cycles.NumScycles++;
      else top->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) top->cycles.NumNcycles++;
    else top->cycles.NumIcycles++;

    top->cnt=0;
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
        ((unsigned8 *)ptr)[address & 3]=*data;
        break;
          
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2))) = *data;
#if 0   
        if (address & 2)
          *ptr=(*ptr & ~0xffffL) | (*data & 0xffffL);
        else 
          *ptr=(*ptr & 0xffffL) | (*data << 16);
#endif
        break;
          
      case BITS_32:             /* write word */
        *ptr=*data;
        break;
        
      default:
        return -1;
      }
    }
  }
    
  return 1;
}

static const ARMul_Cycles *ReadCycles(void *handle)
{
  static ARMul_Cycles cycles;
  toplevel *top=(toplevel *)handle;

  if (top->prop & (MAP_COUNTWAIT | MAP_AMBABUSCOUNTS)) {
    cycles.NumNcycles = top->cycles.NumNcycles;
    cycles.NumScycles = top->cycles.NumScycles;
    cycles.NumIcycles = top->cycles.NumIcycles;
    cycles.NumCcycles = top->wait_states;;
    cycles.Total = 0;
  } else {
    cycles = top->cycles;
    cycles.Total = top->wait_states;
  }
  
  cycles.Total += (cycles.NumNcycles + cycles.NumScycles +
                   cycles.NumIcycles + cycles.NumCcycles);

  return &cycles;
}

static unsigned long ReadClock(void *handle)
{
  const ARMul_Cycles *cycles = ReadCycles(handle);
  /* returns a us count */
  toplevel *top = (toplevel *)handle;
  double t = (double)(cycles->Total) * top->clk;
  return (unsigned long)t;
}

static unsigned long GetCycleLength(void *handle)
{
  /* Returns the cycle length in tenths of a nanosecond */
  toplevel *top=(toplevel *)handle;
  return top->clk*10000.0;
}
