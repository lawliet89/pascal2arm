/* armcnf.h - toolconf tags for ARMulator. */
/* Copyright (C) 1997 Advanced RISC Machines Limited
 *
 * RCS $Revision: 1.5.2.1 $
 * Checkin $Date: 1997/07/03 15:02:03 $
 * Revising $Author: mwilliam $
 */

/* New models can add tags here for their own use */

#ifndef armcnf_h
#define armcnf_h

#include "toolconf.h"
#include "dbg_conf.h"

/*
 * Options used by ARMulator
 */
#define ARMulCnf_Memories (tag_t)"MEMORIES"
#define ARMulCnf_Processors (tag_t)"PROCESSORS"
#define ARMulCnf_CoProcessors (tag_t)"COPROCESSORS"
#define ARMulCnf_EarlyModels (tag_t)"EARLYMODELS"
#define ARMulCnf_Models (tag_t)"MODELS"

#define ARMulCnf_Memory (tag_t)"MEMORY"
#define ARMulCnf_ExternalCoProBus (tag_t)"EXTERNALCOPROBUS"
#define ARMulCnf_CoProBus (tag_t)"COPROBUS"
#define ARMulCnf_Core (tag_t)"CORE"
#define ARMulCnf_Clock (tag_t)"CLOCK"
#define ARMulCnf_Model (tag_t)"MODEL"
#define ARMulCnf_Verbose (tag_t)"VERBOSE"
#define ARMulCnf_Track (tag_t)"TRACK"
#define ARMulCnf_Architecture (tag_t)"ARCHITECTURE"
#define ARMulCnf_OS (tag_t)"OS"
#define ARMulCnf_PLLRange (tag_t)"PLLRANGE"
#define ARMulCnf_sNa (tag_t)"SNA"
#define ARMulCnf_Default (tag_t)"DEFAULT"
#define ARMulCnf_ARMulator (tag_t)"ARMULATOR"
#define ARMulCnf_LateAborts (tag_t)"LATEABORTS"
#define ARMulCnf_MultipleEarlyAborts (tag_t)"MULTIPLEEARLYABORTS"
#define ARMulCnf_LongMultiply (tag_t)"LONGMULTIPLY"
#define ARMulCnf_Halfword (tag_t)"HALFWORD"
#define ARMulCnf_ThumbAware (tag_t)"THUMBAWARE"
#define ARMulCnf_Thumb (tag_t)"THUMB"
#define ARMulCnf_Cached (tag_t)"CACHED"
#define ARMulCnf_System32 (tag_t)"SYSTEM32"
#define ARMulCnf_Fix26 (tag_t)"FIX26"
#define ARMulCnf_Nexec (tag_t)"NEXEC"
#define ARMulCnf_Debug (tag_t)"DEBUG"
#define ARMulCnf_Isync (tag_t)"ISYNC"
#define ARMulCnf_Lock (tag_t)"LOCK"
#define ARMulCnf_StrongARM (tag_t)"STRONGARM"
#define ARMulCnf_Prefetch (tag_t)"PREFETCH"
#define ARMulCnf_NoLDCSTC (tag_t)"NOLDCSTC"
#define ARMulCnf_NoCDP (tag_t)"NOCDP"
#define ARMulCnf_SubPage (tag_t)"SUBPAGE"
#define ARMulCnf_Multiply (tag_t)"MULTIPLY"
#define ARMulCnf_FCLK (tag_t)"FCLK"
#define ARMulCnf_MCLK (tag_t)"MCLK"

/*
 * Cache model options
 */
#define ARMulCnf_CacheSize (tag_t)"CACHESIZE"
#define ARMulCnf_CacheWrite (tag_t)"CACHEWRITE"
#define ARMulCnf_CacheType (tag_t)"CACHETYPE"
#define ARMulCnf_LockDownCacheFlushBase (tag_t)"LOCKDOWNCACHEFLUSHBASE"
#define ARMulCnf_LockDownCacheValid (tag_t)"LOCKDOWNCACHEVALID"
#define ARMulCnf_Replacement (tag_t)"REPLACEMENT"
#define ARMulCnf_RNG (tag_t)"RNG"
#define ARMulCnf_LockDownTLBFlushBase (tag_t)"LOCKDOWNTLBFLUSHBASE"
#define ARMulCnf_LockDownTLBValid (tag_t)"LOCKDOWNTLBVALID"
#define ARMulCnf_HasRFlag (tag_t)"HASRFLAG"
#define ARMulCnf_Has26BitConfig (tag_t)"HAS26BITCONFIG"
#define ARMulCnf_HasBranchPrediction (tag_t)"HASBRANCHPREDICTION"
#define ARMulCnf_HasUpdateable (tag_t)"HASUPDATEABLE"
#define ARMulCnf_BufferedSwap (tag_t)"BUFFEREDSWAP"
#define ARMulCnf_WriteBackCache (tag_t)"WRITEBACKCACHE"
#define ARMulCnf_CacheBlockInterlock (tag_t)"CACHEBLOCKINTERLOCK"
#define ARMulCnf_CacheWriteBackInterlock (tag_t)"CACHEWRITEBACKINTERLOCK"
#define ARMulCnf_LockDownCache (tag_t)"LOCKDOWNCACHE"
#define ARMulCnf_LockDownTLB (tag_t)"LOCKDOWNTLB"
#define ARMulCnf_PLLClock (tag_t)"PLLCLOCK"
#define ARMulCnf_InvalidP15AccessesUndefined (tag_t)"INVALIDP15ACCESSESUNDEFINED"
#define ARMulCnf_IdleCycles (tag_t)"IDLECYCLES"
#define ARMulCnf_HasWriteBuffer (tag_t)"HASWRITEBUFFER"
#define ARMulCnf_LRURead (tag_t)"LRUREAD"
#define ARMulCnf_LRUWrite (tag_t)"LRUWRITE"
#define ARMulCnf_CacheWritebackInterlock (tag_t)"CACHEWRITEBACKINTERLOCK"
#define ARMulCnf_MCCfg (tag_t)"MCCFG"
#define ARMulCnf_RefClkCfg (tag_t)"REFCLKCFG"
#define ARMulCnf_PLLCfg (tag_t)"PLLCFG"
#define ARMulCnf_WritebackCache (tag_t)"WRITEBACKCACHE"
#define ARMulCnf_CacheBlocks (tag_t)"CACHEBLOCKS"
#define ARMulCnf_CacheWords (tag_t)"CACHEWORDS"
#define ARMulCnf_CacheAssociativity (tag_t)"CACHEASSOCIATIVITY"
#define ARMulCnf_ReplaceTicks (tag_t)"REPLACETICKS"
#define ARMulCnf_LockdownCacheFlushBase (tag_t)"LOCKDOWNCACHEFLUSHBASE"
#define ARMulCnf_LockdownCacheValid (tag_t)"LOCKDOWNCACHEVALID"
#define ARMulCnf_TLBSize (tag_t)"TLBSIZE"
#define ARMulCnf_WriteBufferWords (tag_t)"WRITEBUFFERWORDS"
#define ARMulCnf_WriteBufferAddrs (tag_t)"WRITEBUFFERADDRS"
#define ARMulCnf_ChipNumber (tag_t)"CHIPNUMBER"
#define ARMulCnf_Revision (tag_t)"REVISION"
#define ARMulCnf_Counters (tag_t)"COUNTERS"

/*
 * StrongARM configuration options
 */
#define ARMulCnf_Config (tag_t)"CONFIG"
#define ARMulCnf_CCCfg (tag_t)"CCCFG"
#define ARMulCnf_CCLK (tag_t)"CCLK"
#define ARMulCnf_TimeScaleFactor (tag_t)"TIME_SCALE_FACTOR"
#define ARMulCnf_ICacheLines (tag_t)"ICACHE_LINES"
#define ARMulCnf_ICacheAssociativity (tag_t)"ICACHE_ASSOCIATIVITY"
#define ARMulCnf_DCacheLines (tag_t)"DCACHE_LINES"
#define ARMulCnf_DCacheAssociativity (tag_t)"DCACHE_ASSOCIATIVITY"
#define ARMulCnf_ClockSwitching (tag_t)"CLOCKSWITCHING"

/*
 * Supplied models
 */

/* dummymmu.c */
#define ARMulCnf_ChipID (tag_t)"CHIPID"

/* demon.c */
#define ARMulCnf_AddrSuperStack (tag_t)"ADDRSUPERSTACK"
#define ARMulCnf_AddrAbortStack (tag_t)"ADDRABORTSTACK"
#define ARMulCnf_AddrUndefStack (tag_t)"ADDRUNDEFSTACK"
#define ARMulCnf_AddrIRQStack (tag_t)"ADDRIRQSTACK"
#define ARMulCnf_AddrFIQStack (tag_t)"ADDRFIQSTACK"
#define ARMulCnf_AddrUserStack (tag_t)"ADDRUSERSTACK"
#define ARMulCnf_AddrSoftVectors (tag_t)"ADDRSOFTVECTORS"
#define ARMulCnf_AddrCmdLine (tag_t)"ADDRCMDLINE"
#define ARMulCnf_AddrsOfHandlers (tag_t)"ADDRSOFHANDLERS"
#define ARMulCnf_SoftVectorCode (tag_t)"SOFTVECTORCODE"

#define ARMulCnf_AngelSWIARM (tag_t)"ANGELSWIARM"
#define ARMulCnf_AngelSWIThumb (tag_t)"ANGELSWITHUMB"
#define ARMulCnf_HeapBase (tag_t)"HEAPBASE"
#define ARMulCnf_HeapLimit (tag_t)"HEAPLIMIT"
#define ARMulCnf_StackBase (tag_t)"STACKBASE"
#define ARMulCnf_StackLimit (tag_t)"STACKLIMIT"

#define ARMulCnf_Demon (tag_t)"DEMON"

/* tracer.c */
#define ARMulCnf_TraceInstructions (tag_t)"TRACEINSTRUCTIONS"
#define ARMulCnf_TraceMemory (tag_t)"TRACEMEMORY"
#define ARMulCnf_TraceIdle (tag_t)"TRACEIDLE"
#define ARMulCnf_TraceEvents (tag_t)"TRACEEVENTS"
#define ARMulCnf_Disassemble (tag_t)"DISASSEMBLE"
#define ARMulCnf_StartOn (tag_t)"STARTON"
#define ARMulCnf_File (tag_t)"FILE"
#define ARMulCnf_BinFile (tag_t)"BINFILE"
#define ARMulCnf_Port (tag_t)"PORT"
#define ARMulCnf_Host (tag_t)"HOST"
#define ARMulCnf_Pipe (tag_t)"PIPE"
#define ARMulCnf_EventMask (tag_t)"EVENTMASK"
#define ARMulCnf_Event (tag_t)"EVENT"
#define ARMulCnf_Range (tag_t)"RANGE"
#define ARMulCnf_Sample (tag_t)"SAMPLE"
#define ARMulCnf_TraceBus (tag_t)"TRACEBUS"
#define ARMulCnf_RDILog (tag_t)"RDILOG"

/* profiler.c */
#define ARMulCnf_Type (tag_t)"TYPE"
#define ARMulCnf_EventWord (tag_t)"EVENTWORD"

/* pagetab.c */
#define ARMulCnf_MMU (tag_t)"MMU"
#define ARMulCnf_AlignFaults (tag_t)"ALIGNFAULTS"
#define ARMulCnf_Cache (tag_t)"CACHE"
#define ARMulCnf_WriteBuffer (tag_t)"WRITEBUFFER"
#define ARMulCnf_Prog32 (tag_t)"PROG32"
#define ARMulCnf_Data32 (tag_t)"DATA32"
#define ARMulCnf_LateAbort (tag_t)"LATEABORT"
#define ARMulCnf_BigEnd (tag_t)"BIGEND"
#define ARMulCnf_SystemProt (tag_t)"SYSTEMPROT"
#define ARMulCnf_ROMProt (tag_t)"ROMPROT"
#define ARMulCnf_BranchPredict (tag_t)"BRANCHPREDICT"
#define ARMulCnf_ICache (tag_t)"ICACHE"
#define ARMulCnf_DAC (tag_t)"DAC"
#define ARMulCnf_PageTableBase (tag_t)"PAGETABLEBASE"
#define ARMulCnf_VirtualBase (tag_t)"VIRTUALBASE"
#define ARMulCnf_PhysicalBase (tag_t)"PHYSICALBASE"
#define ARMulCnf_Pages (tag_t)"PAGES"
#define ARMulCnf_Cacheable (tag_t)"CACHEABLE"
#define ARMulCnf_Bufferable (tag_t)"BUFFERABLE"
#define ARMulCnf_Updateable (tag_t)"UPDATEABLE"
#define ARMulCnf_Domain (tag_t)"DOMAIN"
#define ARMulCnf_AccessPermissions (tag_t)"ACCESSPERMISSIONS"
#define ARMulCnf_Translate (tag_t)"TRANSLATE"

/* winglass.c */
#define ARMulCnf_Rate (tag_t)"RATE"

/* armmap.c */
#define ARMulCnf_CountWaitStates (tag_t)"COUNTWAITSTATES"
#define ARMulCnf_AMBABusCounts (tag_t)"AMBABUSCOUNTS"
#define ARMulCnf_SpotISCycles (tag_t)"SPOTISCYCLES"

/* peripheral models */
#define ARMulCnf_BaseAddress (tag_t)"BASEADDRESS"
#define ARMulCnf_Limit (tag_t)"LIMIT"

#endif
