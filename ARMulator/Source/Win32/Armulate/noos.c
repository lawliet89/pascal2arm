/***************************************************************************\
* armos.c                                                                   *
* ARMulator II Prototype Operating System Interface.                        *
* Copyright (C) 1991 Advanced RISC Machines Limited. All rights reserved.   *
* Written by Dave Jaggar.                                                   *
\***************************************************************************/

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1997/02/18 18:45:19 $
 * Revising $Author: mwilliam $
 */

#include "armdefs.h"

/***************************************************************************\
*                          OS private Information                           *
\***************************************************************************/


/***************************************************************************\
*            Time for the Operating System to initialise itself.            *
\***************************************************************************/

static ARMul_Error OSInit(ARMul_State *state,
                          ARMul_OSInterface *interf,
                          toolconf config)
{
  IGNORE(interf); IGNORE(config);
  ARMul_PrettyPrint(state, ", No Operating System");
  return ARMulErr_NoError;
}

const ARMul_OSStub ARMul_NoOS = {
  OSInit,                       /* init */
  (tag_t)"None"
  };
