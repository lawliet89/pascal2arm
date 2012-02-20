/* -*-C-*-
 *
 * $Revision: 1.38.6.3 $
 *   $Author: rivimey $
 *     $Date: 1998/01/12 21:23:15 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Hardware description of the PID board.
 */

#ifndef angel_pid_devconf_h
#define angel_pid_devconf_h

#include "pid.h"
#include "stacks.h"

/*
 * The number of serial ports used, set to 1 or 2
 *
 * Although the board (and drivers) support 2, select 1 only as the
 * default allowing the other one to be used by the application
 * entirely standalone.
 */
#define ST16C552_NUM_PORTS      1

/*
 * set these defines to 0 if they are not wanted, 1 if they are wanted
 *
 * Note that ETHERNET_SUPPORTED is set to 0 or 1 by the Makefile
 */
#if MINIMAL_ANGEL == 0
# define PARALLEL_SUPPORTED      1
# define PCMCIA_SUPPORTED        1
# define PROFILE_SUPPORTED       1
#else
# define PARALLEL_SUPPORTED      0
# define PCMCIA_SUPPORTED        0
# define PROFILE_SUPPORTED       0
#endif


/*
 * DCC is the Debug Comms Channel on Thumb Processors.  In order to
 * use this device as an Angel Channel you need an EmbeddedICE board
 * running the 'ADP over JTAG' software - not the standard EmbeddedICE
 * software
 */
#define DCC_SUPPORTED           1 /* change as required */
#define CACHE_SUPPORTED         0

/*
 * Choose the mthod for debug output here.  Options supported for PID are:
 *      rombox          neXus Rombox romulator backchannel
 *      e5              E5 romulator backchannel
 *      panicblk        panic string written to RAM
 *      pidulate        via ARMulator (during emualtion only)
 *      logadp          via ADP to host
 *      log552          via 16c552 Serial port B at 115200 baud
 *      logterm         as log552, but interactive. (#define LOGTERM_DEBUGGING in Makefile too!)
 */
#if DEBUG == 1
#if MINIMAL_ANGEL == 0
# define DEBUG_METHOD logterm
#else
# define DEBUG_METHOD panicblk
#endif
#endif

/*
 * These can be set to determine whether the interrupt handlers are
 * connected, to IRQ, FIQ or both.  Be sure to give the relevent
 * modes some stack space - see above for this.
 */
#define HANDLE_INTERRUPTS_ON_IRQ 1
#define HANDLE_INTERRUPTS_ON_FIQ 0

/* See serlock.h for the possible settings */
#define FIQ_SAFETYLEVEL         FIQ_CannotBeOptimised

/*
 * control which devices are raw and which are Angel/shared
 */
#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
#  define RAW_ST16C552_A        1
#  if (ST16C552_NUM_PORTS > 1)
#    define RAW_ST16C552_B      1
#  endif
#  define RAW_DCC               1
#else
#  define RAW_ST16C552_A        0       /* set to 1 if always raw */
#  if (ST16C552_NUM_PORTS > 1)
#    define RAW_ST16C552_B      1       /* set to 0 if B is Angel/shared */
#  endif
#  define RAW_DCC               0       /* set to 1 is always raw */
#endif

#if (ST16C552_NUM_PORTS > 1)
#  define HAVE_RAW_ST16C552   (RAW_ST16C552_A || RAW_ST16C552_B)
#  define HAVE_ANGEL_ST16C552 (!(RAW_ST16C552_A && RAW_ST16C552_B))
#else
#  define HAVE_RAW_ST16C552   (RAW_ST16C552_A)
#  define HAVE_ANGEL_ST16C552 (!RAW_ST16C552_A)
#endif

#if PARALLEL_SUPPORTED
/*
 * define which serial port the parallel "driver" uses
 * for uploads to the remote host
 */
#define DI_PARALLEL             DI_ST16C552_A
#endif

/* the default active device */
#define DE_DEFAULT_ACTIVE       DI_ST16C552_A

/*
 * sanity checks
 */
#if (ST16C552_NUM_PORTS != 1) && (ST16C552_NUM_PORTS != 2)
# error "Number of Serial Ports must be either 1 or 2"
#endif

#if (ETHERNET_SUPPORTED != 0) && (PCMCIA_SUPPORTED == 0)
# error "Ethernet without PCMCIA support"
#endif

/*
 * The PID memory map
 */
#define SSRAMBase               (0x01000000)
#define SSRAMSize               (0x00020000)
#define SSRAMTop                (SSRAMBase + SSRAMSize)

#define SRAMBase                (0x02000000)
#define SRAMSize                (0x00080000)
#define SRAMTop                 (SRAMBase + SRAMSize)

#define ROMBase                 (0x04000000)
#define ROMSize                 (0x00080000)
#define ROMTop                  (ROMBase + ROMSize)

#define IOBase                  (0x08000000)
#define IOSize                  (0x04000000)
#define IOTop                   (IOBase + IOSize)

/* DRAM is optional and can be any 1MB to 16MB in size */
#define DRAMBase                (0x03000000)
#define DRAMMax                 (0x04000000)
/*
 * Although the PID card has no real memory at 0, it is aliased onto
 * other address spaces
 */
#define ALIASBase               (0x00000000)
#define ALIASSize               (0x01000000)
#define ALIASTop                (ALIASBase + ALIASSize)

#define FlashBase               (ROMBase)

#define READ_PERMITTED(a)       ((((a) >= SSRAMBase) && ((a) < SSRAMTop)) || \
                                 (((a) >= SRAMBase) && ((a) < SRAMTop)) || \
                                 (((a) >= ROMBase) && ((a) < ROMTop)) || \
                                 (((a) >= DRAMBase) && ((a) < DRAMMax)) || \
                                 (((a) >= IOBase) && ((a) < IOTop)) || \
                                 (((a) >= NISA_PCMEM) && ((a) < NISA_TOP)) || \
                                 ((a) < ALIASTop))

#define WRITE_PERMITTED(a)      ((((a) >= SSRAMBase) && ((a) < SSRAMTop)) || \
                                 (((a) >= SRAMBase) && ((a) < SRAMTop)) || \
                                 (((a) >= IOBase) && ((a) < IOTop)) || \
                                 (((a) >= DRAMBase) && ((a) < DRAMMax)) || \
                                 (((a) >= NISA_PCMEM) && ((a) < NISA_TOP)) || \
                                 ((a) < ALIASTop))

/* This can be overridden by the memory sizer - if present */
#define Angel_DefaultTopOfMemory 0x80000

/* This should be set to 1 for systems with RAM slots */
#define MEMORY_SIZE_MAY_CHANGE_DYNAMICALLY 1

/*
 * sanity checks on link addresses defined in Makefile
 */
#if ((RWADDR < SSRAMBase || RWADDR >= SSRAMTop) && \
     (RWADDR < SRAMBase || RWADDR >= SRAMTop) && \
     (RWADDR < DRAMBase || RWADDR >= DRAMMax) && \
     (RWADDR < ALIASBase || RWADDR >= ALIASTop))
# error "Illegal value for RWADDR"
#endif

#if ((ROADDR < SSRAMBase || ROADDR >= SSRAMTop) && \
     (ROADDR < SRAMBase || ROADDR >= SRAMTop) && \
     (ROADDR < DRAMBase || ROADDR >= DRAMMax) && \
     (ROADDR < ROMBase || ROADDR >= ROMTop))
# error "Illegal value for ROADDR"
#endif

/*
 * the sizes of the various Angel stacks
 */
#define Angel_UNDStackSize      0x0100

#define Angel_AngelStackSize    0x1000
#define Angel_SVCStackSize      0x0800

/*
 * This is set to 1 only if the system does not remap ROM to 0
 * on a reset
 */
#define BRANCH_TO_HIGH_ROM_ON_RESET 0

#if HANDLE_INTERRUPTS_ON_IRQ
# define Angel_IRQStackSize     0x0100
#else
# define Angel_IRQStackSize     0
#endif

#if HANDLE_INTERRUPTS_ON_FIQ
# define Angel_FIQStackSize     0x0100
#else
# define Angel_FIQStackSize     0
#endif

/* And where should the Angel stacks live ?
 * They can either live at an absolute address (eg the bottom of memory)
 * or somewhere relative to the top of memory (which is dynamic!), so
 * this must be specified here.  You may change this if you require
 * a different memory map.
 */
#define Angel_StacksAreRelativeToTopOfMemory 1

#if Angel_StacksAreRelativeToTopOfMemory
# define Angel_StackBaseOffsetFromTopMemory  (-(Angel_CombinedAngelStackSize))
# define Angel_ApplStackOffset     Angel_StackBaseOffsetFromTopMemory
#else
# define Angel_FixedStackBase      0x100
# define Angel_ApplStackOffset     Angel_DefaultTopOfMemory
#endif

/* Similarly, the default is for the Fusion heap to sit towards the top
 * of memory.
 */
#if ETHERNET_SUPPORTED
# define Angel_FusionHeapSize (22 * 1024)
# if defined(HEAP_SIZE)
#  if (Angel_FusionHeapSize != HEAP_SIZE)
#   error Angel_FusionHeapSize must match HEAP_SIZE
#  endif
# endif

# define Angel_FusionHeapIsRelativeToTopOfMemory 1

# if Angel_FusionHeapIsRelativeToTopOfMemory
#  define Angel_FusionHeapOffsetFromTopMemory \
                   (Angel_StackBaseOffsetFromTopMemory - Angel_FusionHeapSize)
#  undef Angel_ApplStackOffset
#  define Angel_ApplStackOffset Angel_FusionHeapOffsetFromTopMemory
# else
#  define Angel_FixedFusionHeapBase ???
# endif
#else
# define Angel_FusionHeapSize 0
# define Angel_FusionHeapOffsetFromTopMemory Angel_StackBaseOffsetFromTopMemory
#endif

/* We also allocate a lump of memory for use by profiling counts.
 * only allocate this space if profiling is supported.
 */
#if PROFILE_SUPPORTED
/* Set the area to 0 if no special memory area is to be set aside
 * Here we don't so it will be stolen off the heap instead
 */
# define Angel_ProfileAreaSize 0

# define Angel_ProfileAreaIsRelativeToTopOfMemory 1

# if Angel_ProfileAreaIsRelativeToTopOfMemory
#  define Angel_ProfileAreaBaseOffsetFromTopOfMemory \
                    (Angel_FusionHeapOffsetFromTopMemory - Angel_ProfileAreaSize)
#  undef Angel_ApplStackOffset
#  define Angel_ApplStackOffset Angel_ProfileAreaBaseOffsetFromTopOfMemory
# else
#  define Angel_FixedProfileAreaBase ???
# endif
#endif

/*
 * The application stack will sit at the top of memory, below the
 * Angel stacks (if situated there) and the Fusion heap (if there is one)
 */
#define Angel_ApplStackSize        8192
#define Angel_ApplStackLimitOffset (Angel_ApplStackOffset - Angel_ApplStackSize)

/*
 * By default the heap starts at the end of the ROM image and the heap
 * limit is the bottom of the application stack.  These are the
 * defaults unless these two #defines are turned on in which case the
 * addresses specified are used by the C Library.
 */
#if 0
# define Angel_ApplHeapLimit  ???
# define Angel_ApplHeap       ???
#endif

/* Where to download a new agent to */
#define Angel_DownloadAgentArea        0x8000

/*
 * This constant should be changed only with great care.  It is the
 * amount of empty stack space left between two callback's stacks on
 * the AngelStack.
 */
#define Angel_AngelStackFreeSpace 0x400


/* In order to improve download speed over parallel we
 * increase the size of the Long Download buffer from the default
 * of 2kb.  7Kb is all we can fit below 0x8000, so that will do.
 * Note however, that for ethernet, it is best to ensure that
 * the longest buffer will fit in a single packet.
 */
#define BUFFERLONGSIZE (4*1024)
#define ETHERNET_BUFFERLONGSIZE 1400

/*
 * SYSCLOCK - used as a sanity check in delay (ethersup.s)
 * and also by timer.c.
 * The values below are the standard switched values: uncomment the
 * one used (or add an alternative value if an external source is used).
 * The number is held in units of 1000 system clocks to make calculations
 * using these values easier.
 */
/*#define SYSCLOCK        (4 * 1000) */
/*#define SYSCLOCK        (8 * 1000) */
#define SYSCLOCK        (16 * 1000)
/*#define SYSCLOCK        (20 * 1000) */

/*
 * the devices available
 * ORDER AND NUMBER MUST MATCH angel_Device[] TABLE IN devices.c
 */
typedef enum DeviceIdent
{
  DI_ST16C552_A = 0,
  DI_ST16C552_B,
  DI_ETHERNET,
  DI_DCC,
  DI_NUM_DEVICES
} DeviceIdent;

#define DI_LED_DEVICE DI_ST16C552_A

/*
 * Interrupt masks in the PID board Interrupt controller
 */
#if HANDLE_INTERRUPTS_ON_FIQ
/* In order to test out interrupts on FIQ you can set SERIAL_INTERRUPTS_ON_FIQ
 * to 1 and then set the mask below to specifiy serial port A or B.  The
 * interrupts for that port will then occur on FIQ not IRQ.
 * This setup is definitely not recommended for real use - keep serial
 * interrupts on IRQ !!!
 */
# define SERIAL_INTERRUPTS_ON_FIQ  1
# define ST16C552_FIQSELECT        (FIQ_SERIALA)
#else
# define SERIAL_INTERRUPTS_ON_FIQ  0
#endif

#if (ST16C552_NUM_PORTS > 1)
# define ST16C552_SERIALIRQMASK         (IRQ_SERIALA | IRQ_SERIALB)
#else
# define ST16C552_SERIALIRQMASK         (IRQ_SERIALA)
#endif

#if PARALLEL_SUPPORTED
# define ST16C552_PARALLELIRQMASK       (IRQ_PARALLEL)
#else
# define ST16C552_PARALLELIRQMASK       (0)
#endif

#define ST16C552_IRQMASK                (ST16C552_SERIALIRQMASK | \
                                         ST16C552_PARALLELIRQMASK)

/*
 * the Int handlers available
 * ORDER AND NUMBER MUST MATCH angel_IntHandler[] in devices.c
 */
typedef enum IntHandlerID
{
    IH_ST16C552_A = 0,
    IH_ST16C552_B,
    IH_PARALLEL,
    IH_PCMCIA_A,
    IH_PCMCIA_B,
#if PROFILE_SUPPORTED
    IH_PROFILETIMER,
#endif
    IH_NUM_DEVICES
} IntHandlerID;

/*
 * must be no. of entries in IntHandlerID
 */
#if PROFILE_SUPPORTED
#define DE_NUM_INT_HANDLERS     (6)
#else
#define DE_NUM_INT_HANDLERS     (5)
#endif

/*
 * the POLL handlers available
 * NUMBER MUST MATCH angel_PollHandler[] in devices.c
 */
#if ETHERNET_SUPPORTED
# define NETHERNET_PH           1
#else
# define NETHERNET_PH           0
#endif

#if DCC_SUPPORTED
# define NDCC_PH                1
#else
# define NDCC_PH                0
#endif

#define DE_NUM_POLL_HANDLERS    (NETHERNET_PH + NDCC_PH)

/*
 * ring buffer size
 *
 * NOTE - the ring buffer macros are more efficient
 * when ring buffer size is declared as a power of 2
 */
#define RING_SIZE (512)

#define RING_LOW_WATER_MARK   (32)             /* when to fill tx buffer */
#define RING_HIGH_WATER_MARK  (RING_SIZE-32)  /* when to empty rx buffer */

#endif /* ndef angel_pid_devconf_h */

/* EOF devconf.h */
