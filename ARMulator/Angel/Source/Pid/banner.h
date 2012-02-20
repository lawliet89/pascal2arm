/* -*-C-*-
 *
 * $Revision: 1.6.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:55:40 $
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

#if ST16C552_NUM_PORTS == 1
# define SER_STR "Serial(x1)"
#else
# define SER_STR "Serial(x2)"
#endif

#if PARALLEL_SUPPORTED > 0
# define PAR_STR ", Parallel"
#else
# define PAR_STR
#endif

#if ETHERNET_SUPPORTED > 0
# define ETH_STR ", Ethernet"
#else
# define ETH_STR
#endif

#if DCC_SUPPORTED > 0
# define DCC_STR ", DCC"
#else
# define DCC_STR
#endif

#if DEBUG == 1
# define DBG_STR ", debug: " ## STRINGIFY(DEBUG_METHOD)
#else
# define DBG_STR
#endif

#ifdef NO_INFO_MESSAGES
# define INFO_STR ", no info"
#else
# define INFO_STR
#endif

#define CONFIG  "Built with " SER_STR PAR_STR ETH_STR DCC_STR \
        DBG_STR INFO_STR

#define ANGEL_BANNER \
"Angel Debug Monitor V" TOOLVER_ANGEL " for PID\n" CONFIG "\n" \
"Rebuilt on " __DATE__ " at " __TIME__ "\n"

#endif

 /* EOF banner_h */
