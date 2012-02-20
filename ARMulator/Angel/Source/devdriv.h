/* -*-C-*-
 *
 * $Revision: 1.7 $
 *   $Author: kwelton $
 *     $Date: 1996/07/30 12:52:29 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Device driver framework interface.
 */

#ifndef angel_devdriv_h
#define angel_devdriv_h

/*
 * This header exports the interface to the device driver framework
 * for use within specific device drivers.  These calls are not for use
 * by the body of Angel or of an application.
 */

#include "devconf.h"
#include "devclnt.h"
#include "serlock.h"            /* angel_RegBlock */
#include "params.h"

/* General purpose constants, macros, enums, typedefs */

/*
 * The following typedefs provide the prototypes for the various
 * entry points into a device driver.
 */

/*
 * device driver write entry point
 * ===============================
 *
 * + see angel_DeviceWrite() in devclnt.h for parameters.
 * + callbacks handled by the framework via angel_DD_SentPacket() below.
 */
typedef DevError (*angel_DeviceWriteFn)(DeviceID devID,
                                        p_Buffer buff, unsigned length,
                                        DevChanID chanID);

typedef DevError (*angel_RawWriteFn)(DeviceID devID,
                                     p_Buffer buff, unsigned length);

/*
 * device driver register read entry point
 * =======================================
 *
 * + see angel_DeviceRegisterRead() in devclnt.h for parameters.
 * + callbacks handled by the framework via angel_DD_GotPacket() below.
 */
typedef DevError (*angel_DeviceRegisterReadFn)(
                                        DeviceID devID, DevChanID chanID);

typedef DevError (*angel_RawReadFn)(DeviceID devID,
                                    p_Buffer buff, unsigned length);

/*
 * device driver control entry point
 * =================================
 *
 * + see angel_DeviceControl() in devclnt.h for parameters
 */
typedef DevError (*angel_DeviceControlFn)(DeviceID devID,
                                          DeviceControl op, void *arg);

/*
 * Interrupt entry point
 * =====================
 * 
 * Parameters:
 *             ident        the ident supplied by the HW-specific GETSOURCE
 *                          macro, which corresponds to the position in the
 *                          angel_IntHandler[] table in devices.c
 *
 *             data         the data entry in the angel_IntHandler[] table.
 *
 *             empty_stack  where to reset the stack to if the serialiser
 *                          is called.
 *                          This will be required should the handler wish
 *                          to use the serialiser to get the lock [by
 *                          calling Angel_SerialiseTask()]
 *
 * If the Interrupt handler can complete its servicing very quickly, then it
 * can simply return in the ordinary way.
 *
 * Any processing that will impact interrupt latency should be performed
 * as a serialised function, queued by a call to Angel_SerialiseTask(),
 * in which case the handler will appear not to return.
 */
typedef void (*angel_IntHandlerFn)(
    unsigned        ident,           /* hw-specific ident */
    unsigned        data,            /* data in Int table */
    unsigned        empty_stack      /* to be passed thru to SerialiseTask */
    );

/*
 * poll entry point
 * ================
 *
 * Parameters:
 *              data  the corresponding data entry in the angel_PollHandler[]
 *                    table in devices.c
 *
 * Note that there are two entries per polled device in the angel_PollHandler[]
 * table, one for 'read' and one for 'write'.
 *
 * Each entry in the angel_PollHandler[] table is called once
 * per Angel_Yield().  The poll handler should keep its own record of whether
 * it should perform any processing, and should return immediately if not.
 *
 * Note that poll handlers are called in SVC mode with the lock.
 */
typedef void (*angel_PollHandlerFn)(unsigned data);

/* Angel or raw device? */
typedef enum angel_DeviceType {
    DT_ANGEL,
    DT_RAW
} angel_DeviceType;

/* Read and write for Angel/shared devices */
typedef struct angel_AngelReadWrite {
    angel_DeviceWriteFn         async_write;
    angel_DeviceRegisterReadFn  register_read;
} angel_AngelReadWrite;

/* Read and write for raw devices */
typedef struct angel_RawReadWrite {
    angel_RawWriteFn            write;
    angel_RawReadFn             read;
} angel_RawReadWrite;

/* An entry in the main table of device drivers */
struct angel_DeviceEntry {
  angel_DeviceType           type;
  union {
    angel_AngelReadWrite angel;
    angel_RawReadWrite   raw;
  }                          rw;
  angel_DeviceControlFn      control;
  const void                *data;
  ParameterOptions           options;
  ParameterConfig            default_config;
};

/* Interrupt handler table entry */
struct angel_IntHandlerEntry {
  angel_IntHandlerFn    handler; /* the handler function */
  unsigned              data;    /* data passed to handler */
};

/* Poll handler table entry */
struct angel_PollHandlerEntry {
  angel_PollHandlerFn   read_handler;   /* the poll handler for reads */
  unsigned              read_data;      /* data passed to read handler */
  angel_PollHandlerFn   write_handler;  /* the poll handler for writes */
  unsigned              write_data;     /* data passed to write handler */
};

/* flags for public device status -- see angel_DeviceStatus below */
#define DEV_COMMON_FLAGS_MASK  (0xFF)
#define DEV_PRIVATE_FLAGS_MASK ((2^sizeof(unsigned)-1)^DEV_COMMON_MASK)

/*
 * DEV_READ_BUSY is set once a reader has been registered
 * for the corresponding device channel
 */
#define DEV_READ_BUSY(devchan) (1<<(devchan))
#define DEV_READ_APPL_BUSY  (1<<0)
#define DEV_READ_DBUG_BUSY  (1<<1)
#define DEV_READ_BUSY_MASK  (DEV_READ_APPL_BUSY | DEV_READ_DBUG_BUSY)

/*
 * DEV_WRITE_BUSY is set whenever a write is in progress
 */
#define DEV_WRITE_BUSY      (1<<7)
/* other bits reserved */


/* Publically-accessible globals */

/*
 * the public state of the devices
 * -- bits 0 thru 7 used by DEV_WRITE_BUSY and DEV_READ_BUSY, etc.
 * -- remaining bits free for use by device driver
 */
extern volatile unsigned int angel_DeviceStatus[DI_NUM_DEVICES];

/*
 * the main device driver table
 *
 * DI_NUM_DEVICES is in devconf.h
 */
extern const struct angel_DeviceEntry * const angel_Device[DI_NUM_DEVICES];

/*
 * a standard do-nothing device which is used as a place filler for
 * devices which are not configured into Angel
 */
extern const struct angel_DeviceEntry angel_NullDevice;

/*
 * an error-reporting interrupt handler, used as a place filler for
 * interrupt-driven devices which are not configured into Angel
 */
extern void angel_NodevIntHandler(unsigned int ident, unsigned int data,
                                  unsigned int empty_stack);

/* the Interrupt handler table */
#if DE_NUM_INT_HANDLERS>0
extern const struct angel_IntHandlerEntry
                    angel_IntHandler[];
#endif

/* the poll handler table */
#if DE_NUM_POLL_HANDLERS>0
extern const struct angel_PollHandlerEntry 
                    angel_PollHandler[];
#endif


/* Public functions */

/*
 * The _DD_ functions are convenience functions to be called from
 * the device drivers, which handle the details of callbacks, etc.
 */

/*
 * Function: angel_DD_GetBuffer
 *  Purpose: Grab a buffer for receiving a packet into, called once
 *           the device driver knows which Device Channel the read
 *           is for, and how much space is required.
 *
 *   Params:
 *              Input: devid     device ID of the driver
 *                     devchan   which device channel the request is for
 *                     req_size  required buffer size
 */
p_Buffer angel_DD_GetBuffer( DeviceID   devid,
                             DevChanID  devchan,
                             unsigned   req_size );

/*
 * Function: angel_DD_GotPacket
 *  Purpose: Allows the device driver to queue a callback on completion of
 *           reading a packet, or when a read error occurs once the DevChanID
 *           of the packet in error has been established.
 *
 *   Params:
 *              Input:  devid      device ID of the driver
 *                      buff       buffer with packet in
 *                      length     length of packet
 *                      status     read completion status
 *                      devchanID  which device channel the packet is for
 */
void angel_DD_GotPacket(DeviceID        devID,
                        p_Buffer        buff,
                        unsigned        length,
                        DevStatus       status,
                        DevChanID       devchanID);

/*
 * Function: angel_DD_SentPacket
 *  Purpose: Allows the device driver to queue a callback on completion of
 *           sending a packet.
 *
 *  Pre-conditions: Must be called in SVC mode
 *
 *   Params:
 *              Input:  devid      device ID of the driver
 *                      buff       buffer with packet in
 *                      length     length of packet
 *                      status     write completion status
 *                      devchanID  which device channel the write was for
 */
void angel_DD_SentPacket(DeviceID        devID,    /* which device */
                         p_Buffer        buff,     /* pointer to data */
                         unsigned        length,   /* how much done */
                         DevStatus       status,   /* success code */
                         DevChanID       devchanID); /* appl or chan */


/*
 * Function: angel_DeviceYield
 *  Purpose: allow any polled devices to do a poll
 *
 *   Params:    None
 *
 * Will be called from Angel_Yield to call all the neccessary poll
 * functions.
 */
void angel_DeviceYield(void);


#endif /* ndef angel_devdriv_h */

/* EOF devdriv.h */
