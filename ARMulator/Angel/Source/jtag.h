/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: amerritt $
 *     $Date: 1996/06/21 14:01:06 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Test header for jtag device driver - for EmbeddedICE
 */

#ifndef angel_jtag_h
#define angel_jtag_h

/*
 * 
 */

/* Nested header files, if required */

#include "devdriv.h"


/* General purpose constants, macros, enums, typedefs */

#define JTAG_DEV_IDENT_0   0


/* Publically-accessible globals */
extern const struct angel_DeviceEntry angel_JTAGDevice;


/* see devdriv.h for description of POLL handler functions */
extern void angel_JTAGPollHandler( unsigned );


#endif /* ndef angel_jtag_h */

/* EOF jtag.h */
