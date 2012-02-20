/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/07/25 16:05:39 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Header for sample application
 */

#ifndef angel_appldev_h
#define angel_appldev_h

typedef struct Config {
      unsigned int      name_len;
      char             *name;
} Config;

typedef enum CommandType {
    C_POLL,
    C_RESET_DEFAULT_CONFIG,
    C_GET_CONFIG,
    C_GET_CONFIG_ACK,
    C_SET_CONFIG
} CommandType;

typedef struct Command {
      CommandType       type;
      unsigned int      length;
      /* ... length chars, if appropriate */
} Command;

#define COMMAND_MIN_LENGTH      (8)

#endif /* ndef angel_appldev_h */

/* EOF appldev.h */
