/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: ijohnson $
 *     $Date: 1996/05/09 17:08:55 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Test header for serial/parallel device driver
 */

#ifndef angel_serpar_h
#define angel_serpar_h

/*
 * 
 */

/* Nested header files, if required */

#include "devdriv.h"


/* General purpose constants, macros, enums, typedefs */

#define SERPAR_DEV_IDENT_0   0


/* Publically-accessible globals */
extern const struct angel_DeviceEntry angel_SerParDevice;


/* see devdriv.h for description of POLL handler functions */
extern void angel_SerParReadPollHandler( unsigned );
extern void angel_SerParWritePollHandler( unsigned );


#endif /* ndef angel_serpar_h */

/* EOF serial.h */
