/* watchpnt.c - Memory veneer that implements watchpoints.
 * Copyright (C) Advanced RISC Machines Limited, 1996. All rights reserved.
 *
 * RCS $Revision: 1.5 $
 * Checkin $Date: 1997/02/18 18:45:28 $
 * Revising $Author: mwilliam $
 */

#include <string.h>             /* for memset */
#include "armdefs.h"
#include "armcnf.h"
#include "linklist.h"

#define ModelName (tag_t)"WatchPoints"

typedef struct {
  ARMul_State *state;
  ARMul_MemInterface real;
} wp_state;

/*
 * ARMulator callbacks
 */

/* Watchpoint handling */
#define Watch_AnyRead (RDIWatch_ByteRead+RDIWatch_HalfRead+RDIWatch_WordRead)
#define Watch_AnyWrite (RDIWatch_ByteWrite+RDIWatch_HalfWrite+RDIWatch_WordWrite)

static int RDI_info(void *handle, unsigned type, ARMword *arg1, ARMword *arg2)
{
  IGNORE(handle);
  IGNORE(arg2);
  switch (type) {
  case RDIInfo_Points:
    *arg1 |= (Watch_AnyRead | Watch_AnyWrite)<<2;
    return RDIError_UnimplementedMessage;
  }

  return RDIError_UnimplementedMessage;
}  

/*
 * Memory veneer functions
 * Pass on the call to the real function, then check for the watchpoint.
 */

static int watch[acc_Nrw<<1]; /* need space for this many bits */

static void init_watch(void)
{
  if (watch[BITS_8]==0) {       /* not done yet */
    unsigned int i;

    for (i=0;i<sizeof(watch)/sizeof(watch[0]);i++) {
      if (acc_READ(i)) switch (acc_WIDTH(i)) {
      case BITS_8:
        watch[i]=RDIWatch_ByteRead;
        break;
      case BITS_16:
        watch[i]=RDIWatch_HalfRead;
        break;
      case BITS_32:
        watch[i]=RDIWatch_WordRead;
        break;
      case BITS_64:
        watch[i]=-1;            /* signal "special" case */
        break;
      default:
        watch[i]=0;
        break;
      } else switch (acc_WIDTH(i)) {
      case BITS_8:
        watch[i]=RDIWatch_ByteWrite;
        break;
      case BITS_16:
        watch[i]=RDIWatch_HalfWrite;
        break;
      case BITS_32: case BITS_64:
        watch[i]=RDIWatch_WordWrite;
        break;
      default:
        watch[i]=0;
        break;
      }
    }
  }
}
        
    
static int MemAccess(void *handle,ARMword addr,ARMword *word,ARMul_acc acc)
{
  wp_state *wp=(wp_state *)handle;
  int rv;

  rv=wp->real.x.basic.access(wp->real.handle,addr,word,acc);

  if (rv==1) ARMul_CheckWatch(wp->state,addr,
                              watch[acc & (acc_Nrw | WIDTH_MASK)]);

  return rv;
}

static int MemAccess2(void *handle,ARMword addr,
                      ARMword *word1,ARMword *word2,ARMul_acc acc)
{
  wp_state *wp=(wp_state *)handle;
  int rv;

  rv=wp->real.x.arm8.access2(wp->real.handle,addr,word1,word2,acc);

  if (rv>0) {
    ARMul_State *state=wp->state;
    int access=watch[acc & (acc_Nrw | WIDTH_MASK)];
    if (rv==2) {
      ARMul_CheckWatch(state,addr,access);
      addr+=4;
    }
    ARMul_CheckWatch(state,addr,access);
  }

  return rv;
}

/* Dummy veneer functions */
static unsigned int DataCacheBusy(void *handle)
{
  wp_state *wp=(wp_state *)handle;
  return wp->real.x.strongarm.data_cache_busy(wp->real.handle);
}
static void CoreException(void *handle,ARMword address,ARMword penc)
{
  wp_state *wp=(wp_state *)handle;
  wp->real.x.arm8.core_exception(wp->real.handle,address,penc);
}
static unsigned long GetCycleLength(void *handle)
{
  wp_state *wp=(wp_state *)handle;
  return wp->real.x.basic.get_cycle_length(wp->real.handle);
}
static unsigned long ReadClock(void *handle)
{
  wp_state *wp=(wp_state *)handle;
  return wp->real.read_clock(wp->real.handle);
}
static const ARMul_Cycles *ReadCycles(void *handle)
{
  wp_state *wp=(wp_state *)handle;
  return wp->real.read_cycles(wp->real.handle);
}

/*
 * Initialise the memory interface
 */

static ARMul_Error MemInit(ARMul_State *state,
                           ARMul_MemInterface *interf,
                           ARMul_MemType type,
                           toolconf config)
{
  wp_state *wp;
  ARMul_MemInterface *mem;
  armul_MemInit *stub=NULL;
  tag_t tag;
  toolconf child;
  ARMul_Error err;

  init_watch();

  tag = (tag_t)ToolConf_Lookup(config, ARMulCnf_Memory);

  if (tag)
    stub = ARMul_FindMemoryInterface(state, tag, &child);
  if (tag==NULL || stub==NULL || stub==MemInit)
    return ARMul_RaiseError(state,ARMulErr_NoMemoryChild,ModelName);

  wp=(wp_state *)malloc(sizeof(wp_state));
  if (wp==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  wp->state=state;
  mem=&wp->real;

  err=stub(state,mem,type,child);
  if (err!=ARMulErr_NoError) {
    free(wp);
    return err;
  }

  interf->handle=wp;
  interf->read_clock=(mem->read_clock ? ReadClock : NULL);
  interf->read_cycles=(mem->read_cycles ? ReadCycles : NULL);

  switch (type) {
  case ARMul_MemType_Basic: case ARMul_MemType_BasicCached:
  case ARMul_MemType_16Bit: case ARMul_MemType_16BitCached:
  case ARMul_MemType_Thumb: case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccess;
    interf->x.basic.get_cycle_length=(mem->x.basic.get_cycle_length ?
                                      GetCycleLength : NULL);
    break;
  case ARMul_MemType_ARM8:
    interf->x.arm8.access=MemAccess;
    interf->x.arm8.get_cycle_length=(mem->x.arm8.get_cycle_length ?
                                     GetCycleLength : NULL);
    interf->x.arm8.access2=MemAccess2;
    interf->x.arm8.core_exception=(mem->x.arm8.core_exception ?
                                   CoreException : NULL);
    break;
  case ARMul_MemType_StrongARM:
    interf->x.strongarm.access=MemAccess;
    interf->x.strongarm.get_cycle_length=(mem->x.strongarm.get_cycle_length ?
                                          GetCycleLength : NULL);
    interf->x.strongarm.core_exception=(mem->x.strongarm.core_exception ?
                                        CoreException : NULL);
    interf->x.strongarm.data_cache_busy=DataCacheBusy;
    break;
  default:
    *interf=*mem;
    free(wp);
    ARMul_ConsolePrint(state,"\
Cannot do watchpoints on this type of memory interface.\n");
    return ARMulErr_NoError;
  }

  ARMul_PrettyPrint(state,", Watchpoints");
  
  ARMul_InstallExitHandler(state,free,wp);
  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,wp);

  return ARMulErr_NoError;
}

ARMul_MemStub ARMul_WatchPoints = {
  MemInit,
  ModelName
  };

