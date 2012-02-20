/* errors.c - List of all ARMulator errors
 * (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1996/10/18 16:56:19 $
 * Revising $Author: mwilliam $
 */

/*
 * this file need not be modified - new errors should be added to
 * the END of errors.h
 */

#include "armdefs.h"

static const char *errors[] = {

#define ERROR(N,M) M,

#include "errors.h"

  NULL
  };

/*
 * This function is called by the RDI layer to ask for the error
 * message corresponding to the given error number.
 */

const char *ARMul_ErrorMessage(ARMul_Error code)
{
  if (code<=ARMulErr_ErrorBase || code>=ARMulErr_ErrorTop)
    return NULL;

  return errors[code-ARMulErr_ErrorBase-1];
}
