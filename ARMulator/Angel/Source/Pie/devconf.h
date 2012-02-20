/* -*-C-*-
 *
 * $Revision: 1.28.2.5 $
 *   $Author: rivimey $
 *     $Date: 1998/03/11 10:36:26 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Hardware description of the PIE board.
 */

#ifndef angel_pie_devconf_h
#define angel_pie_devconf_h

#include "stacks.h"

#ifdef ICEMAN2
/* Set to 1 if parallel support required, 0 otherwise
 * Note that both the ADP ICEman and ADP over JTAG set ICEMAN2
 */
# define PARALLEL_SUPPORTED 1
#else
# define PARALLEL_SUPPORTED 0
#endif

/*
 * There is no PCMCIA or Ethernet support on the PIE card
 */
#define PCMCIA_SUPPORTED        0
#define ETHERNET_SUPPORTED      0
#define CACHE_SUPPORTED         0
#ifdef ICEMAN2
# define PROFILE_SUPPORTED      0
#else
# if MINIMAL_ANGEL
#  define PROFILE_SUPPORTED      0
# else
#  define PROFILE_SUPPORTED      1
# endif
#endif

/*
 * Choose the method for debug output here.  Options supported for PIE are:
 *      panicblk        panic string written to RAM
 *      pidulate        via ARMulator (during emualtion only)
 *      logadp          via ADP to host (NOT for minimal angel though!)
 */
#if DEBUG == 1
#  if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
#    define DEBUG_METHOD panicblk
#  else
#    define DEBUG_METHOD panicblk
#  endif
#endif

/* These can be set to determine whether the interrupt handlers are
 * connected, to IRQ, FIQ or both.
 */
#define HANDLE_INTERRUPTS_ON_IRQ 1
#define HANDLE_INTERRUPTS_ON_FIQ 1

/* See serlock.h for the possible settings */
#define FIQ_SAFETYLEVEL FIQ_CannotBeOptimised

/* control whether the serial device is raw or Angel/shared */
#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
#  define RAW_PIE_SERIAL 0      /* change to 1 if should always be raw */
#else
#  define RAW_PIE_SERIAL 1
#endif

/* Set this to 1 if (as on the PIE60 and PIE7) the serial controller
 * provides buffering of characters.  This will ensure the serial
 * driver uses an efficient (in that system) mechanism.
 *
 * It is vital that if the serial I/O is not buffered that this flag
 * is set to 0, so that a scheme which allows the first byte of a
 * packet to cause an interrupt, and then poll the rest of the packet
 * in with interrupts reenabled, and the serial chip no longer causing
 * Rx interrupts.  For this to work correctly it the size of the ring
 * buffer needs to be large enough to ensure that an entire packet can
 * be put into the ring buffer.
 */
#define BUFFERED_SERIAL_INPUT 0

#if PARALLEL_SUPPORTED
/*
 * define which serial port the parallel "driver" uses
 * for uploads to the remote host */
#define DI_PARALLEL             DI_SERIAL
#endif

/* the default active device */
#define DE_DEFAULT_ACTIVE DI_SERIAL

/* the device which controls the LED's */
#define DI_LED_DEVICE DI_SERIAL

/* The PIE memory map is structured as follows. */

#define RAMBase         (0x00000000)      /* normal SRAM location */
#define RAMSize         (0x00080000)
#define RAMTop          (RAMBase + RAMSize)

#define ExpBase         (0x40000000)      /* expansion, and PIO chip */
#define ExpSize         (0x40000000)

#define IOBase          (0x80000000)
#define IOSize          (0x40000000)

#define ROMBase         (0xc0000000)      /* ROM/EPROM etc. */
#define ROMSize         (0x40000000)
/*
 * Define ROMTop as 1 byte short of true value, to prevent 32-bit overflow
 */
#define ROMTop          (ROMBase + (ROMSize - 1))

/*
 * The ARM 7 PIE allows unrestricted access to the entire memory space.
 * To stop the debugger acessing areas it should not access these two
 * macros are used to determine if an access should occur.
 *
 * The ARM 6 PIE card prohibits user mode code from reading the I/O space.
 * So these macros are superfluous in that case, but kept for simplicity.
 */
#define READ_PERMITTED(__addr__) ((__addr__) < IOBase || (__addr__) >= ROMBase)
#define WRITE_PERMITTED(__addr__) ((__addr__) < IOBase)

/* This can be overridden by the memory sizer - if present */
#define Angel_DefaultTopOfMemory RAMTop

/* This should be set to 1 for systems with RAM slots */
#define MEMORY_SIZE_MAY_CHANGE_DYNAMICALLY 0

/*
 * sanity checks on link addresses defined in Makefile
 */
#ifndef ICEMAN2
# if (RWADDR < RAMBase || RWADDR >= (RAMTop))
#   error "Illegal value for RWADDR"
# endif

# if ((ROADDR < RAMBase || ROADDR >= RAMTop) && \
     (ROADDR < ROMBase || ROADDR >= ROMTop))
#  error "Illegal value for ROADDR"
# endif
#endif

/* The following are the sizes of the various Angel stacks */
#define Angel_SVCStackSize     0x400

#if HANDLE_INTERRUPTS_ON_IRQ
# define Angel_IRQStackSize    0x100
#else
# define Angel_IRQStackSize    0
#endif

#if HANDLE_INTERRUPTS_ON_FIQ
# define Angel_FIQStackSize    0x100
#else
# define Angel_FIQStackSize    0
#endif

#ifdef ICEMAN2
#  define Angel_AngelStackSize  (16*1024) /* big stack for Iceman */
#else
#  define Angel_AngelStackSize  0x800
#endif

#define Angel_UNDStackSize    0x100

/* This is set to 1 only if the system does not remap ROM to 0
 * on a reset
 */
#define BRANCH_TO_HIGH_ROM_ON_RESET 0

/* Determine whether Angel will run from ROM or RAM - this
 * should be as close to the top of memory as possible
 */
#if (ROADDR < RAMTop) 
# define Angel_InRAMAddr ROADDR
#endif

/* And where should the Angel stacks live ?
 * They can either live at an absolute address (eg the bottom of memory)
 * or somewhere relative to the top of memory, so
 * this must be specified here.  You may change this if you require
 * a different memory map.
 */

#ifdef ICEMAN2
/* This is the setup for Angel when used as part of ICEman2,
 * in which Angel is linked as part of the application
 */
# define Angel_StacksAreRelativeToTopOfMemory 1

# define Angel_StackBaseOffsetFromTopMemory  (-(Angel_CombinedAngelStackSize))
# define Angel_ApplStackOffset     Angel_StackBaseOffsetFromTopMemory
#else

/* The default case for Angel in RAM at the top of memory */
# define Angel_StacksAreRelativeToTopOfMemory 0

# if Angel_StacksAreRelativeToTopOfMemory
#  define Angel_StackBaseOffsetFromTopMemory  (-(Angel_CombinedAngelStackSize))
#  define Angel_ApplStackOffset     Angel_StackBaseOffsetFromTopMemory
# else

/* Note that this address should be high enough that it is above the
 * RW Data stored in low memory, but below the Application space which
 * starts at 0x8000.  To find out where the RW data ends look for _end
 * in the symbols file.
 */
#  define Angel_FixedStackBase      0x6000
#  ifdef Angel_InRAMAddr
#   define Angel_ApplStackOffset    (Angel_InRAMAddr - Angel_DefaultTopOfMemory)
#  else
#   define Angel_ApplStackOffset    Angel_DefaultTopOfMemory
#  endif
# endif
#endif

/* We can also allocate a lump of memory for use by profiling counts.
 * only allocate this space is profiling is supported.  */
#if PROFILE_SUPPORTED
/* Set the area to 0 if no special memory area is to be set aside
 * Here we don't so it will be stolen off the heap instead
 */
# define Angel_ProfileAreaSize 0

# define Angel_ProfileAreaIsRelativeToTopOfMemory 0

# if Angel_ProfileAreaIsRelativeToTopOfMemory
#  define Angel_ProfileAreaBaseOffsetFromTopOfMemory ???
#  undef Angel_ApplStackOffset
#  define Angel_ApplStackOffset ???
# else
#  define Angel_FixedProfileAreaBase ???
# endif
#endif

/*
 * The application stack will sit at the top of memory, below the
 * Angel stacks (if situated there) and the profiling space (if there is any)
 */
#define Angel_ApplStackSize        8192
#define Angel_ApplStackLimitOffset (Angel_ApplStackOffset - Angel_ApplStackSize)

/*
 * By default the heap starts at the end of the application image and
 * the heap limit is the bottom of the application stack.  These are
 * the defaults unless these two #defines are turned on in which case
 * the addresses specified are used by the C Library.
 */
#if 0
# define Angel_ApplHeapLimit  ???
# define Angel_ApplHeap       ???
#endif

/*
 * where to download a new agent to
 */
#ifdef ICEMAN2
# define Angel_DownloadAgentArea        0x30000
#else
# define Angel_DownloadAgentArea        0x8000
#endif

/*
 * This constant should be changed only with great care.  It is the
 * amount of empty stack space left between two callback's stacks on
 * the AngelStack.
 */
#ifdef ICEMAN2
#  define Angel_AngelStackFreeSpace 0x800 /* lots of room for Iceman */
#else
#  define Angel_AngelStackFreeSpace 0x100
#endif

#ifdef ICEMAN2
# ifdef JTAG_ADP_SUPPORTED
/* The ADP over JTAG system must be able to cope with a long buffer at
 * least as large as any of the target boards it will communicate
 * with since they will request that their largest buffer be used for
 * download.
 */
#  define BUFFERLONGSIZE (16*1024)
# else
/* In order to improve download speed over parallel we
 * insist upon 3Kb for ICEMan2 - this seems the best size as there
 * is a tricky tradeoff:
 * Small buffers => start sending data down JTAG sooner
 * Large buffers => less overhead in download
 * Much less than 3Kb or much greater both seem to degrade download
 * performance.
 */
#  define BUFFERLONGSIZE (3*1024)
# endif
#endif

/*
 * the devices available
 * ORDER AND NUMBER MUST MATCH angel_Device[] TABLE IN devices.c
 */
typedef enum DeviceIdent
{
  DI_SERIAL = 0,
#ifdef JTAG_ADP_SUPPORTED
  DI_JTAG,
#endif
  DI_NUM_DEVICES
} DeviceIdent;

/*
 * the interrupt handlers available
 * ORDER AND NUMBER MUST MATCH angel_IntHandler[] in devices.c
 */
typedef enum IntHandlerID
{
    IH_SERIAL = 0,
#if PARALLEL_SUPPORTED
    IH_PARALLEL,
#endif
#if PROFILE_SUPPORTED
    IH_TIMER,
#endif
    IH_NUM_DEVICES
} IntHandlerID;

#if PARALLEL_SUPPORTED
#  if PROFILE_SUPPORTED
#    define DE_NUM_INT_HANDLERS 3   /* must be no. of entries in IntHandlerID */
#  else
#    define DE_NUM_INT_HANDLERS 2   /* must be no. of entries in IntHandlerID */
#  endif
#else
#  if PROFILE_SUPPORTED
#    define DE_NUM_INT_HANDLERS 2   /* must be no. of entries in IntHandlerID */
#  else
#    define DE_NUM_INT_HANDLERS 1   /* must be no. of entries in IntHandlerID */
#  endif
#endif

/*
 * the POLL handlers available
 * ORDER AND NUMBER MUST MATCH angel_PollHandler[] in devices.c
 */

typedef enum PollHandlerID
{
#ifdef JTAG_ADP_SUPPORTED
    PH_JTAG,
#endif
    PH_NUM_DEVICES
} PollHandlerID;

#if defined(JTAG_ADP_SUPPORTED)
# define DE_NUM_POLL_HANDLERS 1
#else
# define DE_NUM_POLL_HANDLERS 0
#endif

/*
 * ring buffer size
 *
 * NOTE - the ring buffer macros are more efficient
 * when ring buffer size is declared as a power of 2
 */

/* Note that the ADP over JTAG S/W for EmbeddedICE reguires a very
 * large ring buffer as it cannot immediately deal with incoming
 * large packets at all quickly.
 */

#if defined(JTAG_ADP_SUPPORTED) || (PARALLEL_SUPPORTED > 0) || \
                                    (!BUFFERED_SERIAL_INPUT)
# define RING_SIZE            8192
#else
# define RING_SIZE            256
#endif

#define RING_LOW_WATER_MARK   (RING_SIZE / 4)   /* when to fill tx buffer */
#define RING_HIGH_WATER_MARK  (RING_SIZE / 2)   /* when to empty rx buffer */

#endif /* ndef angel_pie_devconf_h */

/* EOF devconf_h */
