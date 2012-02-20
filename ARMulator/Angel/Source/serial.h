/* -*-C-*-
 *
 * $Revision: 1.5 $
 *   $Author: mgray $
 *     $Date: 1996/07/18 10:06:46 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Header for serial device driver
 */

#ifndef angel_serial_h
#define angel_serial_h

/*
 * The entry points and constants for the serial device driver
 */

/* Nested header files, if required */

#include "devdriv.h"

typedef enum DeviceControlExtension {
  DC_SET_LED = DC_PRIVATE
} DeviceControlExtension;

/* General purpose constants, macros, enums, typedefs */

#define SER_DEV_IDENT_0   0
#define PAR_DEV_IDENT     (0x80)

/* Publically-accessible globals */
extern const struct angel_DeviceEntry angel_SerialDevice;
extern const ParameterOptions         angel_SerialOptions;
extern const ParameterConfig          angel_SerialDefaults;

/* see devdriv.h for description of Interrupt handler functions */
extern void angel_SerialIntHandler( unsigned, unsigned, unsigned );

#endif /* ndef angel_serial_h */

/* EOF serial.h */
