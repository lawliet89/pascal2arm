/***************************************************************************\
* armcopro.c                                                                *
* ARMulator II co-processor interface.                                      *
* Copyright (C) 1991 Advanced RISC Machines Limited. All rights reserved.   *
* Written by Dave Jaggar.                                                   *
* Project started on 1st July 1991.                                         *
\***************************************************************************/

/* RCS $Revision: 1.19 $
 * Checkin $Date: 1997/05/06 09:33:23 $
 * Revising $Author: mwilliam $
 */

#include <assert.h>
#include "armdefs.h"
#include "armcnf.h"

/*
 * Here's ARMulator's MMU definition.  A few things to note:
 * 1) it has eight registers, but only two are defined.
 * 2) you can only access its registers with MCR and MRC.
 * 3) MMU Register 0 (ID) returns 0x41440110
 * 4) Register 1 only has 4 bits defined.  Bits 0 to 3 are unused, bit 4
 *    controls 32/26 bit program space, bit 5 controls 32/26 bit data space,
 *    bit 6 controls late abort timimg and bit 7 controls big/little endian.
 */

typedef struct {
  ARMword id;
  ARMword reg[8];
  ARMul_State *state;
} dummy_mmu;

static const unsigned int MMURegBytes[] = {8, 4,4,4,4,4,4,4,4};

#define ModelName (tag_t)"DummyMMU"

/*
 * Function to tell the debugger about us.
 */

static int RDI_info(void *handle, unsigned type,
                    ARMword *arg1, ARMword *arg2)
{
  IGNORE(handle);
  if (type==RDIInfo_MMU) {
    *arg1 = 0x4e4f4e45; IGNORE(arg2);   /* NONE */
    return RDIError_NoError;
  } else if (type==(RDIInfo_MMU | RDIInfo_CapabilityRequest)) {
    return RDIError_NoError;
  }
  return RDIError_UnimplementedMessage;
}

static unsigned MMUMRC(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  int reg = (int)(BITS(16,19) & 7);
  dummy_mmu *mmu=(dummy_mmu *)handle;

  IGNORE(type);

  if (reg == 0)
    *value = mmu->id;
  else
    *value = mmu->reg[reg];
  return(ARMul_DONE) ;
}

static unsigned MMUMCR(void *handle, unsigned type,
                       ARMword instr, ARMword value)
{
  int reg = (int)(BITS(16,19) & 7);
  dummy_mmu *mmu=(dummy_mmu *)handle;

  IGNORE(type);

  mmu->reg[reg] = value;
  if (reg == 1)
    ARMul_SetConfig(mmu->state,(ARMword)~0,value);
  return(ARMul_DONE) ;
}


static unsigned MMURead(void *handle, unsigned reg, ARMword *value)
{
  dummy_mmu *mmu=(dummy_mmu *)handle;
 if (reg == 0)
    *value = mmu->id;
 else if (reg < 8)
    *value = mmu->reg[reg] ;
  return(TRUE);
}

static unsigned MMUWrite(void *handle, unsigned reg, ARMword const *valp)
{
  ARMword value = *valp;
  dummy_mmu *mmu=(dummy_mmu *)handle;
  if (reg < 8)
    mmu->reg[reg] = value;
  if (reg == 1)
    ARMul_SetConfig(mmu->state,value,value);
  return(TRUE);
}

static ARMul_Error MMUInit(ARMul_State *state,
                           unsigned int num,
                           ARMul_CPInterface *interf,
                           toolconf config,
                           void *sibling)
{
  dummy_mmu *mmu=(dummy_mmu *)malloc(sizeof(dummy_mmu));
  const char *option;
  int verbose;
  int i;

  IGNORE(num); IGNORE(sibling);

  if (mmu==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  verbose=ToolConf_DLookupBool(config,ARMulCnf_Verbose,FALSE);

  mmu->state=state;
  mmu->reg[1] = ARMul_SetConfig(state,0,0);
  for (i=2;i<8;i++) mmu->reg[i]=0;

  mmu->id = ToolConf_DLookupUInt(config, ARMulCnf_ChipID, 0x41440110);

  interf->mrc=MMUMRC;
  interf->mcr=MMUMCR;
  interf->read=MMURead;
  interf->write=MMUWrite;
  interf->reg_bytes=MMURegBytes;

  if (verbose) {
    ARMul_ConsolePrint(state,"Dummy MMU:               ID 0x%08x\n",
                       mmu->id);
  } else {
    ARMul_PrettyPrint(state, ", Dummy MMU") ;
  }

  ARMul_InstallExitHandler(state,free,mmu);
  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,mmu);

  interf->handle=mmu;

  return ARMulErr_NoError;
}

const ARMul_CPStub ARMul_DummyMMU = {
  MMUInit,
  ModelName
};
