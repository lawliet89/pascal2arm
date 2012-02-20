/* pidprint.c - model for supporting PID print SWIs. */
/* (c) 1995 Advanced RISC Machines Limited. All Rights Reserved. */

#include <stdlib.h>
#include <stdio.h>
#include "armdefs.h"
#include "dbg_rdi.h"

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1997/03/26 13:03:59 $
 * Revising $Author: mwilliam $
 */

static unsigned HandlePIDprint(
   void *handle, ARMword vector, ARMword pc, ARMword instr);

static ARMul_Error PIDInit(ARMul_State *state, toolconf config)
{
  IGNORE(config);

  ARMul_PrettyPrint(state, ", PIDprint");

  ARMul_InstallExceptionHandler(state, HandlePIDprint, (void *)state);

  return ARMulErr_NoError;
}

static unsigned HandlePIDprint(
   void *handle, ARMword vector, ARMword pc, ARMword instr)
{
  ARMul_State *state=(ARMul_State *)handle;

  IGNORE(pc);
  if (vector != ARM_V_SWI) return FALSE;

  switch (instr & 0x00FFFF00) {
  case 0xFFFF00: /* character encoding in SWI number */
    ARMul_ConsolePrint(state,"%c",(char)(instr & 0xFF)) ;
    return TRUE ;
    
  case 0xFFFE00: /* NUL terminated string pointed to by reg */
  {
    ARMword addr = ARMul_GetReg(state,CURRENTMODE,instr & 0x0F);
    ARMword temp ;
    while ((addr != (ARMword)NULL) &&
           ((temp = ARMul_ReadByte(state,addr++)) != '\0'))
      ARMul_ConsolePrint(state,"%c",(char)temp) ;
  }
    return TRUE ;
    
  case 0xFFFD00: /* 32bit hex value in reg */
  {
    ARMword addr = ARMul_GetReg(state,CURRENTMODE,instr & 0x0F);
    ARMul_ConsolePrint(state,"%08X",(unsigned int)addr) ;
  }
    return TRUE ;
    
  case 0xFFFC00: /* 8bit hex value in reg */
  {
    ARMword addr = ARMul_GetReg(state,CURRENTMODE,instr & 0x0F);
    ARMul_ConsolePrint(state,"%02X",(unsigned int)(addr & 0xFF)) ;
  }
    return TRUE ;
    
  case 0xFFFB00: /* character in reg */
  {
    ARMword addr = ARMul_GetReg(state,CURRENTMODE,instr & 0x0F);
    ARMul_ConsolePrint(state,"%c",(char)(addr & 0xFF)) ;
  }
    return TRUE ;
    
  case 0xFFFA00: /* decimal value */
  {
    ARMword addr = ARMul_GetReg(state,CURRENTMODE,instr & 0x0F);
    ARMul_ConsolePrint(state,"%d",(int)addr) ;
  }
    return TRUE;
    
  }

  return FALSE;
}

ARMul_ModelStub PIDprint = {
  PIDInit,
  (tag_t)"PIDprint"             /* name */
};
