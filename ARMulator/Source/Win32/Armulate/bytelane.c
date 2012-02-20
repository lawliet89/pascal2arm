/* Veneer for converting memory accesses to/from bytelane versions */
/* Copyright (c) 1996 Advanced RISC Machines Limited - All Rights Reserved
 *
 * RCS $Revision: 1.7 $
 * Checkin $Date: 1997/02/18 18:45:06 $
 * Revising $Author: mwilliam $
 */

#include "armdefs.h"
#include "armcnf.h"

#define ModelName (tag_t)"ByteLaneVeneer"

typedef struct {
  unsigned int bigendSig;
  ARMul_MemInterface child;
} bytelane;



static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  bytelane *state=(bytelane *)handle;
  IGNORE(old);
  state->bigendSig=((new & MMU_B) != 0);
}


static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config);

ARMul_MemStub ARMul_BytelaneVeneer = {
  MemInit,
  ModelName
  };

/*
 * Predeclare the memory access functions so that the initialise function
 * can fill them in
 */
static int MemAccess(void *,ARMword,ARMword *,ARMul_acc);
static unsigned long ReadClock(void *handle);
static const ARMul_Cycles *ReadCycles(void *handle);
static unsigned long GetCycleLength(void *handle);

static ARMul_Error MemInit(ARMul_State *state,
                           ARMul_MemInterface *interf,
                           ARMul_MemType type,
                           toolconf config)
{
  bytelane *st;
  armul_MemInit *child_init;
  toolconf child;
  ARMul_Error err;

  interf->read_clock=ReadClock;
  interf->read_cycles=ReadCycles;

  switch (type) {
  case ARMul_MemType_Basic: case ARMul_MemType_BasicCached:
  case ARMul_MemType_16Bit: case ARMul_MemType_16BitCached:
  case ARMul_MemType_Thumb: case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccess;
    interf->x.basic.get_cycle_length=GetCycleLength;
    break;
  default:
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }

  child_init=ARMul_FindMemoryInterface(state,
                                       (tag_t)ToolConf_Lookup(config,ARMulCnf_Memory),
                                       &child);
  if (child_init==NULL || child_init==MemInit) {
    return ARMul_RaiseError(state,ARMulErr_NoMemoryChild,ModelName);
  }
  
  st=(bytelane *)malloc(sizeof(bytelane));
  if (st==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  if (child==NULL) child=config;
  err=child_init(state,&st->child,ARMul_MemType_ByteLanes,child);

  if (err!=ARMulErr_NoError) {
    free(st);
    return err;
  }

  ARMul_InstallConfigChangeHandler(state,ConfigChange,st);
  ARMul_InstallExitHandler(state,free,(void *)st);

  interf->handle=(void *)st;

  return ARMulErr_NoError;
}

static unsigned long GetCycleLength(void *handle)
{
  bytelane *bl=(bytelane *)handle;
  return bl->child.x.basic.get_cycle_length(bl->child.handle);
}
static unsigned long ReadClock(void *handle)
{
  bytelane *bl=(bytelane *)handle;
  return bl->child.read_clock(bl->child.handle);
}
static const ARMul_Cycles *ReadCycles(void *handle)
{
  bytelane *bl=(bytelane *)handle;
  return bl->child.read_cycles(bl->child.handle);
}


static int MemAccess(void *handle,
                     ARMword address,
                     ARMword *data,
                     ARMul_acc acc)
{
  bytelane *bl=(bytelane *)handle;
  ARMul_acc acc2;
  armul_MemAccess *mem_access=bl->child.x.basic.access;
  void *chandle=bl->child.handle;

  acc2=acc & ~WIDTH_MASK;

  if (acc_WIDTH(acc)==BITS_32) {
    return mem_access(chandle,address,data,acc2 | 0xf);
  }

  if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc_WIDTH(acc)) {
      case BITS_16:
        if ((address & 0x2) ? bl->bigendSig : !bl->bigendSig) {
          ARMword temp;
          int result=mem_access(chandle,address,&temp,acc2 | 0xc);
          *data=temp>>16;
          return result;
        } else {
          return mem_access(chandle,address,data,acc2 | 0x3);
        }
      case BITS_8:
        switch (address & 0x3) {
        case 0: default:
          if (bl->bigendSig) {
            ARMword temp;
            int result=mem_access(chandle,address,&temp,acc2 | 0x8);
            *data=temp>>24;
            return result;
          } else {
            return mem_access(chandle,address,data,acc2 | 0x1);
          }
        case 1: {
          ARMword temp;
          int result;
          if (bl->bigendSig) {
            result=mem_access(chandle,address,&temp,acc2 | 0x4);
            *data=temp>>16;
          } else {
            result=mem_access(chandle,address,&temp,acc2 | 0x2);
            *data=temp>>8;
          }
          return result;
        }
        case 2: {
          ARMword temp;
          int result;
          if (bl->bigendSig) {
            result=mem_access(chandle,address,&temp,acc2 | 0x2);
            *data=temp>>8;
          } else {
            result=mem_access(chandle,address,&temp,acc2 | 0x4);
            *data=temp>>16;
          }
          return result;
        }
        case 3:
          if (bl->bigendSig) {
            return mem_access(chandle,address,data,acc2 | 0x1);
          } else {
            ARMword temp;
            int result=mem_access(chandle,address,&temp,acc2 | 0x8);
            *data=temp>>24;
            return result;
          }
        }
      default:
        return -1;
      }
    } else {                    /* write */
      ARMword temp;
      switch (acc_WIDTH(acc)) {
      case BITS_16:
        if ((address & 0x2) ? bl->bigendSig : !bl->bigendSig) {
          temp=*data<<16; data=&temp;
          acc2 |= 0xc;
        } else {
          acc2 |= 0x3;
        }
        break;
      case BITS_8:
        switch (address & 0x3) {
        case 0: default:
          if (bl->bigendSig) {
            temp=*data<<24; data=&temp;
            acc2 |= 0x8;
          } else {
            acc2 |= 0x1;
          }
          break;
        case 1:
          if (bl->bigendSig) {
            temp=*data<<16; data=&temp;
            acc2 |= 0x4;
          } else {
            temp=*data<<8; data=&temp;
            acc2 |= 0x2;
          }
          break;
        case 2:
          if (bl->bigendSig) {
            temp=*data<<8; data=&temp;
            acc2 |= 0x2;
          } else {
            temp=*data<<16; data=&temp;
            acc2 |= 0x4;
          }
          break;
        case 3:
          if (bl->bigendSig) {
            acc2 |= 0x1;
          } else {
            temp=*data<<24; data=&temp;
            acc2 |= 0x8;
          }
          break;
        }
        break;
      default:
        return -1;
      }
      return mem_access(chandle,address,data,acc2);
    }
  } else {
    return mem_access(chandle,address,data,acc);
  }
}
