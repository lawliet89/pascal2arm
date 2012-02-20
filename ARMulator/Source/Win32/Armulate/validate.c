/* validate.c - ARM validation suite co-processor and OS. (see also trickbox)
 *
 * Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved.
 *
 * RCS $Revision: 1.17 $
 * Checkin $Date: 1997/05/09 09:59:47 $
 * Revising $Author: mwilliam $
 */

#include "armdefs.h"

/*
 * What follows is the Validation Suite Coprocessor.  It uses two
 * co-processor numbers (4 and 5) and has the follwing functionality.
 * Sixteen registers.
 * Both co-processor nuimbers can be used in an MCR and MRC to access
 *   these registers.
 * CP 4 can LDC and STC to and from the registers.
 * CP 4 and CP 5 CDP 0 will busy wait for the number of cycles
 *   specified by a CP register.
 * CP 5 CDP 1 issues a FIQ after a number of cycles (specified in a CP
 *   register),
 * CDP 2 issues an IRQW in the same way, CDP 3 and 4 turn of the FIQ
 *   and IRQ source, and CDP 5 stores a 32 bit time value in a CP
 *   register (actually it's the total number of N, S, I, C and F
 *   cyles)
 */

static ARMword ValReg[16] ;

static unsigned ValLDC(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{static unsigned words ;

 IGNORE(handle);

 if (type != ARMul_DATA) {
    words = 0 ;
    return(ARMul_DONE) ;
    }
 if (BIT(22)) { /* it's a long access, get two words */
    ValReg[BITS(12,15)] = data ;
    if (words++ == 4)
       return(ARMul_DONE) ;
    else
       return(ARMul_INC) ;
    }
 else { /* get just one word */
    ValReg[BITS(12,15)] = data ;
    return(ARMul_DONE) ;
    }
 }

static unsigned ValSTC(void *handle, unsigned type,
                          ARMword instr, ARMword *data)
{static unsigned words ;

 IGNORE(handle);

 if (type != ARMul_DATA) {
    words = 0 ;
    return(ARMul_DONE) ;
    }
 if (BIT(22)) { /* it's a long access, get two words */
    *data = ValReg[BITS(12,15)] ;
    if (words++ == 4)
       return(ARMul_DONE) ;
    else
       return(ARMul_INC) ;
       }
 else { /* get just one word */
    *data = ValReg[BITS(12,15)] ;
    return(ARMul_DONE) ;
    }
 }

static unsigned ValMRC(void *handle, unsigned type, ARMword instr,ARMword *value)
{
 IGNORE(handle); IGNORE(type);
 *value = ValReg[BITS(16,19)] ;
 return(ARMul_DONE) ;
 }

static unsigned ValMCR(void *handle, unsigned type, ARMword instr, ARMword value)
{
 IGNORE(handle); IGNORE(type);
 ValReg[BITS(16,19)] = value ;
 return(ARMul_DONE) ;
 }

static unsigned ValCDP(void *handle, unsigned type, ARMword instr)
{
 static unsigned long finish = 0 ;
 ARMword howlong ;
 ARMul_State *state=(ARMul_State *)handle;

 howlong = ValReg[BITS(0,3)] ;
 if (BITS(20,23)==0) {
    if (type == ARMul_FIRST) { /* First cycle of a busy wait */
       finish = ARMul_Time(state) + howlong ;
       if (howlong == 0)
          return(ARMul_DONE) ;
       else
          return(ARMul_BUSY) ;
       }
    else if (type == ARMul_BUSY) {
       if (ARMul_Time(state) >= finish)
          return(ARMul_DONE) ;
       else
          return(ARMul_BUSY) ;
       }
    }
 return(ARMul_CANT) ;
 }

static unsigned DoAFIQ(void *handle)
{
  ARMul_SetNfiq((ARMul_State *)handle,LOW);
  return(0) ;
}

static unsigned DoAIRQ(void *handle)
{
  ARMul_SetNirq((ARMul_State *)handle,LOW);
  return(0) ;
}

static unsigned IntCDP(void *handle, unsigned type, ARMword instr)
{static unsigned long finish ;
 ARMword howlong ;
 ARMul_State *state=(ARMul_State *)handle;

 howlong = ValReg[BITS(0,3)] ;
 switch((int)BITS(20,23)) {
    case 0 : if (type == ARMul_FIRST) { /* First cycle of a busy wait */
                finish = ARMul_Time(state) + howlong ;
                if (howlong == 0)
                   return(ARMul_DONE) ;
                else
                   return(ARMul_BUSY) ;
                }
             else if (type == ARMul_BUSY) {
                if (ARMul_Time(state) >= finish)
                   return(ARMul_DONE) ;
                else
                   return(ARMul_BUSY) ;
                   }
             return(ARMul_DONE) ;
    case 1 : if (howlong == 0)
                ARMul_SetNfiq(state,TRUE) ;
             else
                ARMul_ScheduleEvent(state,howlong,DoAFIQ,state);
             return(ARMul_DONE) ;
    case 2 : if (howlong == 0)
                ARMul_SetNirq(state,TRUE) ;
             else
                ARMul_ScheduleEvent(state,howlong,DoAIRQ,state);
             return(ARMul_DONE) ;
    case 3 : ARMul_SetNfiq(state,HIGH);
             return(ARMul_DONE) ;
    case 4 : ARMul_SetNirq(state,HIGH);
             return(ARMul_DONE) ;
    case 5 : ValReg[BITS(0,3)] = ARMul_Time(state) ;
             return(ARMul_DONE) ;
    }
 return(ARMul_CANT) ;
 }

static ARMul_Error Val1Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling)
{
  IGNORE(num); IGNORE(config); IGNORE(sibling);

  interf->handle=(void *)state;
  interf->ldc=ValLDC;
  interf->stc=ValSTC;
  interf->mrc=ValMRC;
  interf->mcr=ValMCR;
  interf->cdp=ValCDP;

  return ARMulErr_NoError;
}

static ARMul_Error Val2Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling)
{
  IGNORE(num); IGNORE(config); IGNORE(sibling);

  interf->handle=(void *)state;
  interf->mrc=ValMRC;
  interf->mcr=ValMCR;
  interf->cdp=IntCDP;

  return ARMulErr_NoError;
}


/* Validation suite co-processor model */

#include <time.h>
#include <errno.h>
#include <string.h>

#include "armdefs.h"
#include "dbg_hif.h"

/*
 * Time for the Operating System to initialise itself.
 */

static ARMul_Error OSInit(ARMul_State *state,
#ifndef NEW_OS_INTERFACE
                          ARMul_OSInterface *interf,
#endif
                          toolconf config);

#ifdef NEW_OS_INTERFACE
const ARMul_ModelStub ARMul_ValidateOS = {
  OSInit,
  "ValidateOS"
};
static unsigned OSHandleSWI(
   void *handle, ARMword vector, ARMword pc, ARMword instr);
#else
const ARMul_OSStub ARMul_ValidateOS = {
  OSInit,
  "ValidateOS"
};
static unsigned OSHandleSWI(void *handle,ARMword number);
static unsigned OSException(void *handle, ARMword vector, ARMword pc);
#endif

static ARMul_Error OSInit(ARMul_State *state,
#ifndef NEW_OS_INTERFACE
                          ARMul_OSInterface *interf,
#endif
                          toolconf config)
{
  ARMul_Error err;

  ARMul_PrettyPrint(state, ", ARM Validation OS");

  err=ARMul_CoProAttach(state,4,Val1Init,config,state);
  if (err==ARMulErr_NoError) err=ARMul_CoProAttach(state,5,Val2Init,config,state);
  if (err!=ARMulErr_NoError) return err;

#ifdef NEW_OS_INTERFACE
  ARMul_InstallExceptionHandler(state, OSHandleSWI, (void *)state);
#else
  interf->handle_swi=OSHandleSWI;
  interf->exception=OSException;
  interf->handle=(void *)state;
#endif

  return ARMulErr_NoError;
}

/*
 * The emulator calls this routine when a SWI instruction is encuntered. The
 * parameter passed is the SWI number (lower 24 bits of the instruction).
 */

static unsigned OSHandleSWI(
#ifdef NEW_OS_INTERFACE
    void *handle, ARMword vector, ARMword pc, ARMword instr
#else
    void *handle,ARMword number
#endif
)
{
  ARMul_State *state=(ARMul_State *)handle;
#ifdef NEW_OS_INTERFACE
  ARMword number;
  IGNORE(pc);
  if (vector != ARM_V_SWI) return FALSE;
  number = instr & 0xffffff;
#endif
  switch (number) {
  case 0x11:
    ARMul_HaltEmulation(state,FALSE);
    return(TRUE) ;
  case 0x01:
    if (ARMul_GetMode(state)>USER32MODE) /* Stay in entry (ARM/THUMB) state */
      ARMul_SetCPSR(state, (ARMul_GetCPSR(state) & 0xffffffe0) | 0x13) ;
    else
      ARMul_SetCPSR(state, (ARMul_GetCPSR(state) & 0xffffffc0) | 0x3) ;
    return(TRUE) ;
  default:
    return(FALSE) ;
  }
}

#ifndef NEW_OS_INTERFACE
/*
 * The emulator calls this routine when an Exception occurs.  The second
 * parameter is the address of the relevant exception vector.  Returning
 * FALSE from this routine causes the trap to be taken, TRUE causes it to
 * be ignored (so set state->Emulate to FALSE!).
 */

static unsigned OSException(void *handle, ARMword vector, ARMword pc)
{ /* don't use this here */
  IGNORE(handle); IGNORE(vector); IGNORE(pc);
  return(FALSE) ;
}
#endif

/*
 * install ourselves as a Coprocessor, without claiming to be an OS model
 */
static ARMul_Error CPInit(ARMul_State *state,
                          toolconf config)
{
    ARMul_Error err;

    ARMul_PrettyPrint(state, ", ARM Validation system");

    err = ARMul_CoProAttach(state, 4, Val1Init, config, state);

    if (err == ARMulErr_NoError)
        err = ARMul_CoProAttach(state, 5, Val2Init, config, state);

    if (err != ARMulErr_NoError)
        return err;

    return ARMulErr_NoError;
}

const ARMul_ModelStub ARMul_ValidateCP =
{
    CPInit,
    "ValidateCP"
};

/* EOF validate.c */
