/* -*-C-*-
 *
 * $Revision: 1.4.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:55:57 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: The message to send up when we boot
 */

#ifndef angel_banner_h
#define angel_banner_h

#include "toolver.h"

#if PARALLEL_SUPPORTED > 0
# define P_STR ", parallel"
#else
# define P_STR
#endif

#ifdef JTAG_ADP_SUPPORTED
# define J_STR ", JTAG"
#else
# define J_STR
#endif

#if DEBUG == 1
# define STRINGIFY2(x) #x
# define STRINGIFY(x)  STRINGIFY2(x)
# define D_STR ", debug: " ## STRINGIFY(DEBUG_METHOD)
#else
# define D_STR
#endif

#ifdef NO_INFO_MESSAGES
# define I_STR ", no info"
#else
# define I_STR
#endif

#define EXTS "serial" P_STR J_STR D_STR I_STR

#define ANGEL_BANNER \
"Angel Debug Monitor for PIE (" EXTS ") " TOOLVER_ANGEL \
"\nAngel Debug Monitor rebuilt on " __DATE__ " at " __TIME__"\n"

#endif

 /* EOF banner_h */
