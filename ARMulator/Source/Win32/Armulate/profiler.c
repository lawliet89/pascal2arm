/* profiler.c - module to do ARMsd profiling
 * Copyright (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.16 $
 * Checkin $Date: 1997/03/24 18:54:21 $
 * Revising $Author: mwilliam $
 */

#include <stdlib.h>
#include <string.h>
#include "armdefs.h"
#include "armcnf.h"

typedef struct {
  ARMul_State *state;
  ARMword mapsize;
  ARMword *map;
  ARMword *counts;
  enum {
    PROFILE_usec, PROFILE_instr, PROFILE_cycle,
    PROFILE_event1, PROFILE_event2, PROFILE_eventpc
    } prof_type;
  void *handle;
  union {
    struct {
      unsigned long rate;
      unsigned long interval,last,next;
    } usec;
    struct {
      unsigned long interval;
    } instr;
    struct {
      unsigned long last;
      unsigned long old_pc;
      unsigned long interval;
    } cycle;
    struct {
      unsigned int mask,value;
    } event;
  } x;
} profile;

static void count(profile *p,ARMword pc,unsigned cnt)
{
  ARMword *map = p->map;
  ARMword size = p->mapsize;
  ARMword low = 0, high = size-1;

  /* binary search */
  if (map[low] <= pc && pc < map[high]) {
    ARMword i;
    for (;;) {
      i = (high + low) / 2;
      if (pc >= map[i]) {
        if (pc < map[i+1]) {
          i++; break;
        } else
          low = i;
      } else if (pc >= map[i-1])
        break;
      else
        high = i;
    }
    p->counts[i-1]+=cnt;
  }
}

static void hourglass_usec(void *handle,ARMword pc,ARMword instr)
{
  profile *p=(profile *)handle;
  unsigned int t=ARMul_ReadClock(p->state);
  ARMul_State *state=p->state; 

  /*
   * hourglass function that tries to be called every 'n' usec
   */

  IGNORE(instr);

  if (t > p->x.usec.next) {
    count(p,pc,1);
    p->x.usec.last=t;
    p->x.usec.next+=p->x.usec.interval;
    if (t>p->x.usec.next) {
      if (p->x.usec.rate==1) {
        p->x.usec.next=p->x.usec.last+p->x.usec.interval;
      } else {
        p->x.usec.rate=1;
        ARMul_HourglassSetRate(state,p->handle,p->x.usec.rate);
      }
    }
  } else {
    unsigned long delta_t=t-p->x.usec.last,dt=p->x.usec.next-t;

    if (dt > delta_t) {
      double rate=p->x.usec.rate;

      /* If this is far too soon, then change the hourglass interval */
      if (delta_t)
        rate*=(double)dt / (double)delta_t;
      else
        rate*=2.0;

      p->x.usec.rate=(unsigned long)rate;
      ARMul_HourglassSetRate(state,p->handle,p->x.usec.rate);
    }
  }
}

static void hourglass_instr(void *handle,ARMword pc,ARMword instr)
{
  profile *p=(profile *)handle;
  /*
   * simpler hourglass function that is called every 'n' instructions
   */

  IGNORE(instr);

  count(p,pc,1);
}

static void hourglass_cycle(void *handle,ARMword pc,ARMword instr)
{
  profile *p=(profile *)handle;
  unsigned long t=ARMul_Time(p->state);
  ARMword old_pc;
  unsigned long interval,last,gap;
  unsigned cnt;
  
  /*
   * simpler hourglass function that is called every 'n' instructions
   */

  IGNORE(instr);

  last=p->x.cycle.last;
  interval=p->x.cycle.interval;
  old_pc=p->x.cycle.old_pc;

  for (cnt=0, gap=t-last; gap>interval; cnt++)
    gap-=interval, last+=interval;

  p->x.cycle.last=last;
  p->x.cycle.old_pc=pc;

  if (cnt) count(p,old_pc,cnt);
}

static void trace_event(void *handle,unsigned int event,ARMword addr1,
                        ARMword addr2)
{
  profile *p=(profile *)handle;
  ARMword addr;

  /*
   * profiling function that lets you profile by event, e.g. cache misses
   */
  
  if ((event & p->x.event.mask)!=p->x.event.value)
    return;
  
  switch (p->prof_type) {
  case PROFILE_event1: addr=addr1; break;
  case PROFILE_event2: addr=addr2; break;
  default:
  case PROFILE_eventpc: addr=ARMul_GetPC(p->state); break;
  }

  count(p,addr,1);
}

static int RDI_info(void *handle, unsigned type, ARMword *arg1, ARMword *arg2)
{
  profile *p=(profile *)handle;
  ARMul_State *state=p->state;

  switch (type) {
  case RDIInfo_Target:
    /* Add the "we can do profiling" capability */
    *arg1 = (*arg1) | RDITarget_CanProfile;
    /* return Unimplemented because this should be passed on */
    return RDIError_UnimplementedMessage;

  case RDIProfile_Stop:
    if (p->handle) switch (p->prof_type) {
    case PROFILE_usec: case PROFILE_instr:
      ARMul_RemoveHourglass(p->state,p->handle);
      break;
    case PROFILE_event1: case PROFILE_event2: case PROFILE_eventpc:
      ARMul_RemoveEventUpcall(p->state,p->handle);
      break;
    }
    p->handle=NULL;
    return RDIError_NoError;
      
  case RDIProfile_Start:
    if (p->handle==NULL) switch (p->prof_type) {
    case PROFILE_usec:
      p->handle=ARMul_InstallHourglass(p->state,hourglass_usec,p);
      p->x.usec.interval = *arg1;
      ARMul_HourglassSetRate(p->state,p->handle,p->x.usec.rate);
      p->x.usec.last=ARMul_ReadClock(state);
      p->x.usec.next=p->x.usec.interval;
      break;

    case PROFILE_instr:
      p->handle=ARMul_InstallHourglass(p->state,hourglass_instr,p);
      p->x.instr.interval = *arg1;
      ARMul_HourglassSetRate(p->state,p->handle,p->x.instr.interval);
      break;

    case PROFILE_cycle:
      p->handle=ARMul_InstallHourglass(p->state,hourglass_cycle,p);
      p->x.cycle.old_pc=ARMul_GetPC(state);
      p->x.cycle.last=ARMul_Time(state);
      p->x.cycle.interval = *arg1;
      ARMul_HourglassSetRate(p->state,p->handle,p->x.cycle.interval);
      break;

    case PROFILE_event1: case PROFILE_event2: case PROFILE_eventpc:
      p->handle=ARMul_InstallEventUpcall(p->state,trace_event,p);
      break;
    }
    return RDIError_NoError;

  case RDIProfile_WriteMap:
    { RDI_ProfileMap *map = (RDI_ProfileMap *)arg1;
      size_t bytes = (size_t)map->len * sizeof(ARMword);
      if (p->mapsize != map->len) {
        if (p->mapsize != 0) {
          free(p->map);
          free(p->counts);
        }
        p->map = NULL;
      }
      if (p->map == NULL) {
        p->map = (ARMword *)malloc(bytes);
        p->counts = (ARMword *)malloc(bytes);
        if (p->map == NULL || p->counts == NULL)
          return RDIError_OutOfStore;
      }
      p->mapsize = map->len;
      memcpy(p->map, map->map, bytes);
      /* and fall through to clear counts */
    }

  case RDIProfile_ClearCounts:
    memset(p->counts, 0, (size_t)p->mapsize * sizeof(ARMword));
    return RDIError_NoError;

  case RDIProfile_ReadMap:
    { ARMword len = *(ARMword *)arg1;
      memcpy(arg2, p->counts, (size_t)len * sizeof(ARMword));
      return RDIError_NoError;
    }

  default:
    if (type & RDIInfo_CapabilityRequest) {
      switch (type & ~RDIInfo_CapabilityRequest) {
      case RDIProfile_Stop:           case RDIProfile_Start:
      case RDIProfile_WriteMap:       case RDIProfile_ClearCounts:
      case RDIProfile_ReadMap:
        return RDIError_NoError;
      }
    }
    break;
  }

  return RDIError_UnimplementedMessage;
}

static ARMul_Error ProfileInit(ARMul_State *state, toolconf config)
{
  profile *p;
  const char *option;
  int verbose;

  IGNORE(config);

  ARMul_PrettyPrint(state,", Profiler");

  verbose = ToolConf_DLookupBool(config, ARMulCnf_Verbose, FALSE);
  
  p=(profile *)malloc(sizeof(profile));
  if (p==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  p->prof_type=PROFILE_usec;

  option=ToolConf_Lookup(config,ARMulCnf_Type);
  if (option) {
    if (ToolConf_Cmp(option,"MICROSECOND"))
      p->prof_type=PROFILE_usec;
    else if (ToolConf_Cmp(option,"INSTRUCTION"))
      p->prof_type=PROFILE_instr;
    else if (ToolConf_Cmp(option,"CYCLE"))
      p->prof_type=PROFILE_cycle;
    else if (ToolConf_Cmp(option,"EVENT"))
      p->prof_type=PROFILE_eventpc;
    else
      ARMul_ConsolePrint(state,"\
Profiling type '%s' not understood (defaulting to \"MICROSECOND\").\n",option);
  }


  if (verbose)
    ARMul_ConsolePrint(state, "Profiling:               ");

  switch (p->prof_type) {
  case PROFILE_usec:
    if (verbose) ARMul_ConsolePrint(state,"Microsecond based");
    p->x.usec.rate=1;
    p->x.usec.interval=0;
    p->x.usec.last=p->x.usec.next=0;
    break;

  case PROFILE_instr:
    if (verbose) ARMul_ConsolePrint(state,"Instruction based");
    break;

  case PROFILE_cycle:
    if (verbose) ARMul_ConsolePrint(state, "Cycle based");
    break;

  case PROFILE_eventpc:
    if (verbose) ARMul_ConsolePrint(state,"Event");
    option=ToolConf_Lookup(config, ARMulCnf_EventWord);
    if (option) {
      if (ToolConf_Cmp(option,"WORD1") || ToolConf_Cmp(option,"1"))
        p->prof_type=PROFILE_event1;
      else if (ToolConf_Cmp(option,"WORD2") || ToolConf_Cmp(option,"2"))
        p->prof_type=PROFILE_event2;
      else if (ToolConf_Cmp(option,"PC"))
        p->prof_type=PROFILE_eventpc;
      else
        ARMul_ConsolePrint(state,"\
Event profiling type '%s' not understood (defaulting to \"WORD1\").\n",option);
    }
    option=ToolConf_Lookup(config, ARMulCnf_EventMask);
    if (option) {
      char *q;
      p->x.event.mask = strtoul(option, &q, 0);
      p->x.event.value = q ? strtoul(q+1, NULL, 0) : p->x.event.mask;
      if (verbose) ARMul_ConsolePrint(state," Mask 0x%08x-0x%08x",
                                     p->x.event.mask,p->x.event.value);
    } else {
      option=ToolConf_Lookup(config, ARMulCnf_Event);
      if (option) {
        p->x.event.mask=(unsigned int)~0;
        p->x.event.value=strtoul(option,NULL,0);
        if (verbose) ARMul_ConsolePrint(state," 0x%08x",p->x.event.value);
      } else {
        p->x.event.mask=p->x.event.value=0;
        if (verbose) ARMul_ConsolePrint(state,"s");
      }
    }
    break;
  }

  if (verbose) ARMul_ConsolePrint(state, "\n");

  p->handle=NULL;
  p->state=state;
  p->mapsize=0;
  p->map=NULL;
  p->counts=NULL;

  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,p);
  ARMul_InstallExitHandler(state,free,p); /* free on exit */

  return ARMulErr_NoError;
}

const ARMul_ModelStub ARMul_Profiler = {
  ProfileInit,
  (tag_t)"Profiler"
  };
