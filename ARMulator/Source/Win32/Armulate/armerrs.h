/* errors.h - List of all ARMulator errors
 * (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1996/10/18 16:55:21 $
 * Revising $Author: mwilliam $
 */

#ifndef errlist_h
#define errlist_h

#include "dbg_rdi.h"

#define ERROR(n,m) n ,

typedef enum {
  ARMulErr_NoError=RDIError_NoError,

  ARMulErr_ErrorBase=RDIError_TargetErrorBase,

#include "errors.h"             /* include full list of errors */

  ARMulErr_ErrorTop
  } ARMul_Error;

#undef ERROR

/* The user provides the "ARMul_ErrorMessage" function - this allows extra
 * error messages to be added. */
extern const char *ARMul_ErrorMessage(ARMul_Error errcode);
/* ARMulator provides the raise error function. It returns the error code
 * passed in, but will look up the error string, and format an error message
 * to be passed back to the debugger. */
extern ARMul_Error ARMul_RaiseError(ARMul_State *state,ARMul_Error errcode,...);

#endif
