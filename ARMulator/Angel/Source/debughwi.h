/* -*-C-*-
 *
 * $Revision: 1.3 $
 *   $Author: mgray $
 *     $Date: 1996/05/13 14:48:35 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * debughwi.h - This file contains the target independant debug
 * handler functions for Angel.
 */

#ifndef angel_debughwi_h
#define angel_debughwi_h

#include "buffers.h"

int gendbg_unimplemented(p_Buffer *buffer, void *stateptr);

typedef int (*handler_function_pointer)(p_Buffer *buffer, void *stateptr);

extern const handler_function_pointer hfptr[];
extern const handler_function_pointer info_hfptr[];
extern const handler_function_pointer ctrl_hfptr[];
extern const handler_function_pointer hwemul_hfptr[];
extern const handler_function_pointer iceb_hfptr[];
extern const handler_function_pointer icem_hfptr[];
extern const handler_function_pointer profile_hfptr[];

extern const int hfptr_max;
extern const int info_hfptr_max;
extern const int ctrl_hfptr_max;
extern const int hwemul_hfptr_max;
extern const int iceb_hfptr_max;
extern const int icem_hfptr_max;
extern const int profile_hfptr_max;

#endif /* !defined(angel_debughwi_h) */

/* EOF debughwi_h */
