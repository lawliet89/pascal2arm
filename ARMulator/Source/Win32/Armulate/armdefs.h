/*
 * armdefs.h -- ARMulator environment definitions.
 * Copyright (C) 1991-96 Advanced RISC Machines Limited. All rights reserved.
 *
 * RCS $Revision: 1.46.2.1 $
 * Checkin $Date: 1997/09/02 12:18:17 $
 * Revising $Author: mwilliam $
 */

#ifndef armdefs_h
#define armdefs_h

#include <stdio.h>
#include <stdlib.h>
#include "host.h"
#include "toolconf.h"
#include "armdbg.h"             /* for ARMword, ARMaddress and ARMhword */

#ifndef TRUE
#  define FALSE 0
#  define TRUE 1
#endif
#define LOW 0
#define HIGH 1
#define LOWHIGH 1
#define HIGHLOW 2

/*
 * Value that is non-zero on big-endian hosts.
 */
#if defined(HOST_ENDIAN_BIG)
#  define HostEndian 1
#elif defined(HOST_ENDIAN_LITTLE)
#  define HostEndian 0
#else
extern unsigned int HostEndian;
#endif


/*
 * Handle onto ARMulator for callbacks
 */

typedef struct ARMul_State ARMul_State ;

/*
 * Functions you can register for when the ARM changes mode or configuration
 * (more efficient than using callbacks every time you want to read one of
 * the changed values)
 *
 * The functions return a handle onto the upcall, or NULL on failure.
 *
 * The functions to remove a handler are passed the handle returned from
 * the install function. (The handle becomes invalid after the model is
 * restarted - i.e. after ExitUpcall's are called.) They return TRUE if
 * the upcall was successfully removed (i.e. FALSE if passed an invalid
 * handle).
 */

/*
 * ModeChangeUpcall is called when the ARM changes processor mode. The values
 *   passed are the new and old modes.
 */

typedef void armul_ModeChangeUpcall(void *handle, ARMword old, ARMword new);
extern void *ARMul_InstallModeChangeHandler(ARMul_State *state,
                                            armul_ModeChangeUpcall *proc,
                                            void *handle);
extern int ARMul_RemoveModeChangeHandler(ARMul_State *state,void *node);

/*
 * TransChangeUpcall is called when the nTRANS signal changes. This will
 *   typically be on mode changes, but also around LDRT (etc.) instructions.
 */

typedef void armul_TransChangeUpcall(void *handle, unsigned old, unsigned new);
extern void *ARMul_InstallTransChangeHandler(ARMul_State *state,
                                             armul_TransChangeUpcall *proc,
                                             void *handle);
extern int ARMul_RemoveTransChangeHandler(ARMul_State *state,void *node);

/*
 * ConfigChangeUpcall is called when the ARM's configuration (bigend, lateabt,
 *   etc.) changes. The values passed are the new and old configs, in
 *   CP15 R1 form.
 */

typedef void armul_ConfigChangeUpcall(void *handle, ARMword old, ARMword new);
extern void *ARMul_InstallConfigChangeHandler(ARMul_State *state,
                                              armul_ConfigChangeUpcall *proc,
                                              void *handle);
extern int ARMul_RemoveConfigChangeHandler(ARMul_State *state,void *node);

/*
 * InterruptUpcall is called when an interrupt is first *spotted* by the core,
 *   even if interrupts are disabled.
 */

/* bit 0 -> fiq    (i.e. bit 0 set -> Nfiq is LOW)
 * bit 1 -> irq
 * bit 2 -> reset
 */
#define ARMul_InterruptUpcallFIQ   0x1
#define ARMul_InterruptUpcallIRQ   0x2
#define ARMul_InterruptUpcallReset 0x4
typedef void armul_InterruptUpcall(void *handle,unsigned int which);
extern void *ARMul_InstallInterruptHandler(ARMul_State *state,
                                           armul_InterruptUpcall *proc,
                                           void *handle);
extern int ARMul_RemoveInterruptHandler(ARMul_State *state,void *node);

/*
 * UnkRDIInfoUpcall is called to handle unknown "RDI Info" calls. This
 *   allows modules to implement, for example, profiling. Calls each in
 *   until one returns something other than "RDIError_UnimplementedMessage" 
 */

typedef int armul_UnkRDIInfoUpcall(void *handle,unsigned type,ARMword *arg1,
                                   ARMword *arg2);
extern void *ARMul_InstallUnkRDIInfoHandler(ARMul_State *state,
                                            armul_UnkRDIInfoUpcall *proc,
                                            void *handle);
extern int ARMul_RemoveUnkRDIInfoHandler(ARMul_State *state,void *node);

/*
 * Hourglass is called every <n> instructions, regardless of
 *   whether they are executed. The inital value for "n" is initially
 *   set to '1', but it can be changed at any time using
 *   HourglassSetRate (which returns the old rate). This allows code
 *   to, e.g., pace itself. The upcall is first called after <n>
 *   instructions. If <n> is changed to <m>, it won't be called again
 *   for <m> instructions, regardless of how long it is since it was
 *   last called.
 * The value passed to HourglassSetRate is the handle returned from
 *   InstallHourglass. HourglassSetRate returns 0 to indicate an
 *   error (bad handle or bad [zero] rate)
 */

typedef void armul_Hourglass(void *handle,ARMword pc,ARMword instr);

extern void *ARMul_InstallHourglass(ARMul_State *state,
                                    armul_Hourglass *proc,
                                    void *handle);
extern int ARMul_RemoveHourglass(ARMul_State *state,void *node);

extern unsigned long ARMul_HourglassSetRate(ARMul_State *state,
                                            void *node,
                                            unsigned long rate);
                                       
/*
 * Function that can be called to trace the event stream. To trace
 * memory accesses as well, use the memory interface. To trace the
 * instruction stream, use the "Hourglass" interface. The semantics
 * of the "addr1", "addr2" and "event"  arguments are left up to the
 * module providing the EventUpcall function.
 */

typedef void armul_EventUpcall(void *handle,unsigned int event,
                               ARMword addr1,ARMword addr2);

extern void *ARMul_InstallEventUpcall(ARMul_State *state,
                                      armul_EventUpcall *trace,
                                      void *handle);
extern int ARMul_RemoveEventUpcall(ARMul_State *state,void *node);

/* Core events */
#define CoreEvent                0x00000
#define CoreEvent_Reset          0x00001
#define CoreEvent_UndefinedInstr 0x00002
#define CoreEvent_SWI            0x00003
#define CoreEvent_PrefetchAbort  0x00004
#define CoreEvent_DataAbort      0x00005
#define CoreEvent_AddrExceptn    0x00006
#define CoreEvent_IRQ            0x00007
#define CoreEvent_FIQ            0x00008
#define CoreEvent_IRQSpotted     0x00017 /* Before masking with "IF" flags */
#define CoreEvent_FIQSpotted     0x00018 /* to see if it will be taken */
#define CoreEvent_ModeChange     0x00019
#define CoreEvent_Dependancy     0x00020 /* a register dependancy stall */

/* Events defined by MMUlator: */
#define MMUEvent                 0x10000
#define MMUEvent_DLineFetch      0x10001 /* or I line fetch in mixed cache */
#define MMUEvent_ILineFetch      0x10002
#define MMUEvent_WBStall         0x10003
#define MMUEvent_DTLBWalk        0x10004 /* or I TLB walk in mixed cache */
#define MMUEvent_ITLBWalk        0x10005
#define MMUEvent_LineWB          0x10006 /* i.e. replacing a dirty line */
#define MMUEvent_DCacheStall     0x10007 /* line fetch had to stall core */
#define MMUEvent_ICacheStall     0x10008

/* Events from the ARM8 Prefetch Unit */
#define PUEvent                  0x20000
#define PUEvent_Full             0x20001 /* PU fills causing Idle cycles */
#define PUEvent_Mispredict       0x20002 /* PU incorrectly predicts a branch */
#define PUEvent_Empty            0x20003 /* PU empties causing a stall */

/* Piccolo events */
#define PiccoloEvent             0x30000
#define PiccoloEvent_Instr       0x30001 /* Piccolo executing instruction */
#define PiccoloEvent_Skip        0x30002 /* Piccolo skipping */
#define PiccoloEvent_Stalled     0x30003 /* Piccolo stalled */
#define PiccoloEvent_ROBLoad     0x30004 /* datum loads into the ROB */
#define PiccoloEvent_FIFOOut     0x30005 /* ARM reads from the FIFO */

/*
 * Function to call to invoke an event
 */

extern void ARMul_RaiseEvent(ARMul_State *state,unsigned int event,
                             ARMword addr1,ARMword addr2);



/*
 * Exception upcalls are used by new OS/Monitor models to catch
 * SWIs, etc.
 */

/*
 * If an exception handler returns TRUE, the exception is trapped
 * at that point, and the ARM does not respond.
 * e.g. - a SWI handler could be:
 *
 *  if (vector == ARM_V_SWI) switch (instr & 0x00ffffff) {
 *  case SWI_HelloWorld:
 *    ARMul_ConsolePrint(state, "Hello, World!\n");
 *    return TRUE;
 *  default:
 *    break;
 *  }
 *  return FALSE;
 *
 */
typedef unsigned armul_ExceptionUpcall(
   void *handle, ARMword vector, ARMword pc, ARMword instr);

extern void *ARMul_InstallExceptionHandler(
   ARMul_State *state, armul_ExceptionUpcall *proc, void *handle);

extern int ARMul_RemoveExceptionHandler(ARMul_State *state, void *node);


/*
 * ExitUpcall is called when the emulator exits
 */

typedef void armul_ExitUpcall(void *handle);
extern void *ARMul_InstallExitHandler(ARMul_State *state,
                                      armul_ExitUpcall *proc,
                                      void *handle);
extern int ARMul_RemoveExitHandler(ARMul_State *state,void *node);


/*
 * Initialisation functions return an error number, as defined in
 * dbg_rdi.h and errors.h.
 * To indicate success, return ARMulErr_NoError.
 */

#include "armerrs.h"

/*
 *
 * Basic Model Interface
 *
 */
typedef ARMul_Error armul_Init(ARMul_State *state,toolconf config);

typedef struct {
  armul_Init *init;             /* initialiser */
  tag_t name;                   /* name */
} ARMul_ModelStub;

/*
 *
 * Memory Interface
 *
 */

#include "armmem.h"

extern void ARMul_Icycles(ARMul_State *state,unsigned number, ARMword address) ;
extern void ARMul_Ccycles(ARMul_State *state,unsigned number, ARMword address) ;

/*
 * Memory access functions
 */
extern ARMword ARMul_ReadWord(ARMul_State *state, ARMword address);
extern ARMword ARMul_ReadHalfWord(ARMul_State *state, ARMword address);
extern ARMword ARMul_ReadByte(ARMul_State *state, ARMword address);
extern void ARMul_WriteWord(ARMul_State *state, ARMword address, ARMword data);
extern void ARMul_WriteHalfWord(ARMul_State *state, ARMword address, ARMword data);
extern void ARMul_WriteByte(ARMul_State *state, ARMword address, ARMword data);
extern unsigned long ARMul_ReadClock(ARMul_State *state);

/* Return the cycle count - returns NULL if not available */
extern const ARMul_Cycles *ARMul_ReadCycles(ARMul_State *state);

/*
 * 
 * Event Handling
 *
 */

typedef unsigned armul_EventProc(void *handle);

extern void ARMul_ScheduleEvent(ARMul_State *state, unsigned long delay,
                                armul_EventProc *, void *handle);
extern void ARMul_InvokeEvent(ARMul_State *state) ;
extern unsigned long ARMul_Time(ARMul_State *state) ;

/*
 *
 * Definitions of things in the co-processor interface
 *
 */

typedef struct ARMul_cop_interface_str ARMul_CPInterface;

typedef ARMul_Error armul_CPInit(ARMul_State *state,
                                 unsigned num,
                                 ARMul_CPInterface *interf,
                                 toolconf config,
                                 void *sibling);
typedef unsigned armul_LDC(void *handle,unsigned type,ARMword instr,
                           ARMword value);
typedef unsigned armul_STC(void *handle,unsigned type,ARMword instr,
                           ARMword *value);
typedef unsigned armul_MRC(void *handle,unsigned type,ARMword instr,
                           ARMword *value);
typedef unsigned armul_MCR(void *handle,unsigned type,ARMword instr,
                           ARMword value);
typedef unsigned armul_CDP(void *handle,unsigned type,ARMword instr);
typedef unsigned armul_CPRead(void *handle,unsigned reg,ARMword *value);
typedef unsigned armul_CPWrite(void *handle,unsigned reg,
                               ARMword const *value);

#define ARMul_FIRST 0
#define ARMul_TRANSFER 1
#define ARMul_BUSY 2
#define ARMul_DATA 3
#define ARMul_INTERRUPT 4
#define ARMul_DONE 0
#define ARMul_CANT 1
#define ARMul_INC 3

struct ARMul_cop_interface_str {
  void *handle;                 /* A model private handle */
  armul_LDC *ldc;               /* LDC instruction */
  armul_STC *stc;               /* STC instruction */
  armul_MRC *mrc;               /* MRC instruction */
  armul_MCR *mcr;               /* MCR instruction */
  armul_CDP *cdp;               /* CDP instruction */
  armul_CPRead *read;           /* Read CP register */
  armul_CPWrite *write;         /* Write CP register */
  const unsigned int *reg_bytes; /* map of coprocessor register sizes */
};

typedef struct {
  armul_CPInit *init;           /* co-processor initialiser */
  tag_t name;                   /* co-processor name */
} ARMul_CPStub;

/*
 * Functions to load the FPE (floating point emulator)
 */

/* returns TRUE if the FPE installed correctly */
extern int ARMul_FPEInstall(ARMul_State *state);
/* returns TRUE if the supplied address lies inside the FPE */
extern int ARMul_FPEAddressInEmulator(ARMul_State *state,ARMword addr);
/* Returns the FPE version number */
extern int ARMul_FPEVersion(ARMul_State *state);

/*
 * Functions for extracting these values:
 *   these functions also check the FPE, if it has been loaded.
 */
extern unsigned int const *ARMul_CPRegBytes(ARMul_State *state,unsigned cp);

/* Return a pointer to the word after the loaded/written words, or NULL on
 * (unknown co-processor) error
 * Handles accesses to the FPE transparently, with cr8=FPSR cr9=FPCR
 */
extern ARMword *ARMul_CPRead(ARMul_State *state,unsigned cp,
                             unsigned reg,ARMword *value);
extern const ARMword *ARMul_CPWrite(ARMul_State *state,unsigned cp,
                                    unsigned reg,ARMword const *value);


#ifndef NEW_OS_INTERFACE /* newer versions use exception upcalls */
/*
 *
 * OS/Monitor Interface
 *
 */

typedef struct armul_os_interface ARMul_OSInterface;

typedef ARMul_Error armul_OSInit(ARMul_State *state,
                                 ARMul_OSInterface *interf,
                                 toolconf config);
typedef unsigned armul_OSHandleSWI(void *handle,ARMword number);
typedef unsigned armul_OSException(void *handle, ARMword vector, ARMword pc);

struct armul_os_interface {
  void *handle;                 /* A model private handle */
  armul_OSHandleSWI *handle_swi; /* SWI handler */
  armul_OSException *exception; /* Exception handler */
};

typedef struct {
  armul_OSInit *init;           /* O/S initialiser */
  tag_t name;                   /* O/S name */
} ARMul_OSStub;
#endif


/*
 * Execute a program
 */
extern ARMword ARMul_DoProg(ARMul_State *state);
/* Terminate execution at the end of this instruction. Value passed is
 * the reason code to return to the debugger, defined in dbg_rdi.h
 * ModelBroken: specialised version to call if we break somehow (e.g. if
 * the model hangs. always returns FALSE)
 */
extern void ARMul_HaltEmulation(ARMul_State *state,unsigned end_condition);
/* report to the debugger than someone else made us stop */
extern void ARMul_HaltEmulationRDI(ARMul_State *state,unsigned end_condition,Dbg_MCState *);
extern int ARMul_ModelBroken(ARMul_State *state);
/* See how it finished */
extern unsigned ARMul_EndCondition(ARMul_State *state);

/*
 * Execute a single instruction
 */
extern ARMword ARMul_DoInstr(ARMul_State *state);

/*
 * Set/Get a normal register
 */
extern ARMword ARMul_GetReg(ARMul_State *state,unsigned mode,unsigned reg);
extern void ARMul_SetReg(ARMul_State *state,unsigned mode,unsigned reg,ARMword value);

/*
 * Set/Get the PC
 */
extern ARMword ARMul_GetPC(ARMul_State *state);
extern void ARMul_SetPC(ARMul_State *state,ARMword value);

/*
 * Set/Get R15 (PC + PSR flags in 26-bit modes)
 */
extern ARMword ARMul_GetR15(ARMul_State *state);
extern void ARMul_SetR15(ARMul_State *state,ARMword value);

/*
 * Set/Get the CPSR
 */
extern ARMword ARMul_GetCPSR(ARMul_State *state);
extern void ARMul_SetCPSR(ARMul_State *state,ARMword value);

/*
 * Set/Get an SPSR
 */
extern ARMword ARMul_GetSPSR(ARMul_State *state,ARMword mode);
extern void ARMul_SetSPSR(ARMul_State *state,ARMword mode,ARMword value);

/*
 *
 * ARM Core Properties
 *
 */


/*
 * Types of ARM we know about
 */

/* The bitflags */
#define ARM_Fix26_Prop      0x00001 /* Fixed in 26-bit mode */
#define ARM_Nexec_Prop      0x00002
#define ARM_Abort7_Prop     0x00004 /* Late aborts */
#define ARM_Mult64_Prop     0x00008 /* Long multiplies */
#define ARM_Debug_Prop      0x00010
#define ARM_Isync_Prop      ARM_Debug_Prop
#define ARM_Lock_Prop       0x00020 /* SWP and SWPB */
#define ARM_Halfword_Prop   0x00040 /* Halfword instructions */
#define ARM_Code16_Prop     0x00080 /* Thumb support */
#define ARM_System32_Prop   0x00100 /* System mode */
#define ARM_Cache_Prop      0x00200 /* Cached */
#define ARM_ARM8_Prop       0x00400 /* ARM8 properties */
#define ARM_StrongARM_Prop  0x00800 /* StrongARM properties */
#define ARM_NoLDCSTC_Prop   0x01000 /* LDC/STC bounce to undef vector */
#define ARM_SubPage_Prop    0x02000 /* ARM7 "sub-page" bug */
#define ARM_VectorLoad_Prop 0x04000 /* does vector checking on loads */
#define ARM_Abort8_Prop     0x08000 /* aborts leave LDM base unchanged */
#define ARM_Arch4_Prop      0x10000 /* architecture vsn. 4 */
#define ARM_Mult32_Prop     0x20000 /* Short multiplies */
#define ARM_Fix32_Prop      0x40000 /* Fixed in 32-bit modes */

/* Archtectures */
#define ARM_Arch_1  ARM_Fix26_Prop
#define ARM_Arch_2  ARM_Arch_1  | ARM_Mult32_Prop
#define ARM_Arch_2a ARM_Arch_2  | ARM_Lock_Prop
#define ARM_Arch_3  ARM_Mult32_Prop | ARM_Lock_Prop
#define ARM_Arch_3G ARM_Arch_3  | ARM_Fix32_Prop
#define ARM_Arch_3M ARM_Arch_3  | ARM_Mult64_Prop
#define ARM_Arch_4  ARM_Arch_3M | ARM_Arch4_Prop | ARM_Halfword_Prop | ARM_System32_Prop
#define ARM_Arch_4T ARM_Arch_4  | ARM_Code16_Prop | ARM_Fix32_Prop

/* ARM2 family */
#define ARM2    (ARM_Fix26_Prop)
#define ARM2as  ARM2
#define ARM61   ARM2
#define ARM3    (ARM2 | ARM_Cache_Prop)

/* ARM6 family */
#define ARM6    (ARM_Lock_Prop)
#define ARM60   ARM6
#define ARM600  (ARM6 | ARM_Cache_Prop)
#define ARM610  ARM600
#define ARM620  ARM600

/* ARM7 family */
#define ARM7base (ARM_Nexec_Prop | ARM_Abort7_Prop)
#define ARM7    (ARM7base | ARM_SubPage_Prop)
#define ARM70   ARM7
#define ARM700  (ARM7 | ARM_Cache_Prop)
#define ARM710  (ARM7 | ARM_Cache_Prop)
#define ARM710a (ARM7 | ARM_Cache_Prop)
/* ...with debug */
#define ARM7d   (ARM7base | ARM_Debug_Prop)
#define ARM70d  ARM7d
/* ...with extended multiply */
#define ARM7dm  (ARM7d | ARM_Mult64_Prop)
#define ARM70dm ARM7dm
/* ...with halfwords and 16 bit instruction set and system mode */
#define ARM7tdm (ARM7dm | ARM_Halfword_Prop | ARM_Code16_Prop | \
                 ARM_System32_Prop)
#define ARM7tdmi ARM7tdm

/* ARM8 family */
#define ARM8    ((ARM7tdm | ARM_ARM8_Prop) & \
                 ~(ARM_Code16_Prop | ARM_Debug_Prop))
#define ARM810  (ARM8 | ARM_Cache_Prop)

/* StrongARM family */
#define StrongARM ((ARM8 | ARM_StrongARM_Prop) & ~ARM_ARM8_Prop)
#define StrongARM110 (StrongARM | ARM_Cache_Prop)

/*
 * Get this flag word
 */
extern ARMword ARMul_Properties(ARMul_State *state);

/*
 * RDI Log bits
 */

#define RDILog_Verbose      0x0001
#define RDILog_CallDebug    0x0004
#define RDILog_NoDebug      0x0008 /* disable DebugPrint calls */
#define RDILog_Trace        0x0010 /* enable tracing */
                      
extern unsigned32 ARMul_RDILog(ARMul_State *state);

/*
 * Macros to extract instruction fields
 */

#define BIT(n) ( (ARMword)(instr>>(n))&1)   /* bit n of instruction */
#define BITS(m,n) ( (ARMword)(instr<<(31-(n))) >> ((31-(n))+(m)) ) /* bits m to n of instr */
#define TOPBITS(n) (instr >> (n)) /* bits 31 to n of instr */

/*
 * Mode Constants - obsolete names
 */

#define USER26MODE    ARM_M_USER26
#define FIQ26MODE     ARM_M_FIQ26
#define IRQ26MODE     ARM_M_IRQ26
#define SVC26MODE     ARM_M_SVC26
#define USER32MODE    ARM_M_USER32
#define FIQ32MODE     ARM_M_FIQ32
#define IRQ32MODE     ARM_M_IRQ32
#define SVC32MODE     ARM_M_SVC32
#define ABORT32MODE   ARM_M_ABORT32
#define UNDEF32MODE   ARM_M_UNDEF32
#define SYSTEM32MODE  ARM_M_SYSTEM32

#define CURRENTMODE RDIMode_Curr

/*
 * Return the ARMulator's mode.
 */
extern ARMword ARMul_GetMode(ARMul_State *state);

/*
 * Vectors
 */

#define ARMul_ResetV          ARM_V_RESET
#define ARMul_UndefinedInstrV ARM_V_UNDEF
#define ARMul_SWIV            ARM_V_SWI
#define ARMul_PrefetchAbortV  ARM_V_PREFETCH
#define ARMul_DataAbortV      ARM_V_ABORT
#define ARMul_AddrExceptnV    ARM_V_ADDREXCEPT
#define ARMul_IRQV            ARM_V_IRQ
#define ARMul_FIQV            ARM_V_FIQ

#define INSTRUCTION16SIZE 2
#define INSTRUCTION32SIZE 4

/*
 * ARM / THUMB differences
 */

#define INSTRUCTION32 (0)

#ifdef CODE16
/* state->Instruction_set values */

#define INSTRUCTION16 (1)

/* bits in registers */

#define BXINSTRUCTIONSETBITS (0x1)
#define CPSRINSTRUCTIONSETBITPOSN (5)
#define CPSRINSTRUCTIONSETBITS (0x1 << CPSRINSTRUCTIONSETBITPOSN)

#define BXINSTRUCTIONSET(r) ((r & BXINSTRUCTIONSETBITS) ? INSTRUCTION16 : INSTRUCTION32)
#define CPSRINSTRUCTIONSET(r) ((r & CPSRINSTRUCTIONSETBITS) ? INSTRUCTION16 : INSTRUCTION32)
#define CPSRINSTRUCTIONSIZE(r) ((r & CPSRINSTRUCTIONSETBITS) ? INSTRUCTION16SIZE : INSTRUCTION32SIZE)

#else
#define INSTRUCTIONSIZE INSTRUCTION32SIZE
#define CPSRINSTRUCTIONSET(r) (INSTRUCTION32)
#define CPSRINSTRUCTIONSIZE(r) (INSTRUCTION32SIZE)
#endif

/*
 * System co-processor flags
 */

#define MMU_M_bit  ARM_MMU_M_pos /* MMU enable */
#define MMU_A_bit  ARM_MMU_A_pos /* Address fault enable */
#define MMU_C_bit  ARM_MMU_C_pos /* Cache enable */
#define MMU_W_bit  ARM_MMU_W_pos /* WB enable */
#define MMU_P_bit  ARM_MMU_P_pos /* 32/26 program mode */
#define MMU_D_bit  ARM_MMU_D_pos /* 32/26 data mode */
#define MMU_L_bit  ARM_MMU_L_pos /* Late abort timing mode */
#define MMU_B_bit  ARM_MMU_B_pos /* Big endian mode */
#define MMU_S_bit  ARM_MMU_S_pos /* System control */
#define MMU_R_bit  ARM_MMU_R_pos /* Other (USR) system control */
#define MMU_F_bit  ARM_MMU_F_pos /*  */
#define MMU_Z_bit  ARM_MMU_Z_pos /* Branch-prediction enable */
#define MMU_I_bit  ARM_MMU_I_pos /* I-cache enable */
#define MMU_RS_bit ARM_MMU_S_pos /* Position of combined R/S flags */

#define MMU_M   ARM_MMU_M
#define MMU_A   ARM_MMU_A
#define MMU_C   ARM_MMU_C
#define MMU_W   ARM_MMU_W
#define MMU_P   ARM_MMU_P
#define MMU_D   ARM_MMU_D
#define MMU_L   ARM_MMU_L
#define MMU_B   ARM_MMU_B
#define MMU_S   ARM_MMU_S
#define MMU_R   ARM_MMU_R
#define MMU_F   ARM_MMU_F
#define MMU_Z   ARM_MMU_Z
#define MMU_I   ARM_MMU_I
#define MMU_RS (ARM_MMU_R | ARM_MMU_S)

#define ARMul_MMUEnabled        ARM_MMU_M
#define ARMul_AlignmentFault    ARM_MMU_A
#define ARMul_CacheEnabled      ARM_MMU_C
#define ARMul_WBEnabled         ARM_MMU_W
#define ARMul_Prog32            ARM_MMU_P
#define ARMul_Data32            ARM_MMU_D
#define ARMul_LateAbt           ARM_MMU_L
#define ARMul_BigEnd            ARM_MMU_B
#define ARMul_SystemProt        ARM_MMU_S
#define ARMul_ROMProt           ARM_MMU_R
#define ARMul_FBit              ARM_MMU_F
#define ARMul_BranchPredict     ARM_MMU_Z
#define ARMul_InstrCacheEnabled ARM_MMU_I

/*
 * Co-processor model interface
 */

extern ARMul_Error ARMul_CoProAttach(ARMul_State *state, unsigned number,
                                     armul_CPInit *init,
                                     toolconf config,void *my_handle);
extern void ARMul_CoProDetach(ARMul_State *state, unsigned number);

/*
 * Host environment
 */

extern void ARMul_SetException(ARMul_State *state);
extern ARMword ARMul_Debug(ARMul_State *state, ARMword pc, ARMword instr) ;
extern void ARMul_CheckWatch(ARMul_State *state, ARMword addr, int access) ;

/*
 * Shared ARMulator library functions
 */

extern void ARMul_DebugPrint(ARMul_State *state, const char *format, ...);
extern void ARMul_DebugPrint_i(const Dbg_HostosInterface *hostif,
                               const char *format, ...);
extern void ARMul_DebugPause(ARMul_State *state);

extern void ARMul_ConsolePrint(ARMul_State *state, const char *format, ...);
extern void ARMul_ConsolePrint_i(const Dbg_HostosInterface *hostif,
                                 const char *format, ...);
extern void ARMul_PrettyPrint(ARMul_State *state, const char *format, ...);

/*
 * Function that tells me whether an instruction would currently pass
 * it's condition codes.
 */
extern unsigned ARMul_CondCheckInstr(ARMul_State *state,ARMword instr);

#ifndef IGNORE
#  define IGNORE(X) ((X)=(X))
#endif

/*
 * Miscellaneous utility functions
 */

/*
 * Return a pointer to the ARMulator's back channel to the debugger,
 * or the debugger's state
 */
extern const Dbg_HostosInterface *ARMul_HostIf(ARMul_State *state);
extern const Dbg_MCState *ARMul_DbgState(ARMul_State *state);

/* Set the hostos interface */
extern const Dbg_HostosInterface *ARMul_SetHostIf(
    ARMul_State *state, Dbg_HostosInterface *new_hif);

/*
 * Raise/lower an interrupt, return old value
 */
extern unsigned ARMul_SetNirq(ARMul_State *state,unsigned value);
extern unsigned ARMul_SetNfiq(ARMul_State *state,unsigned value);
extern unsigned ARMul_SetNreset(ARMul_State *state,unsigned value);

/*
 * Set and Read the "MemSize" flag. ARMulator ignores this, but it can
 * be used by an OS/Memory model pair to communicate where to place stacks,
 * etc.
 * "Set" returns the old value.
 */
extern ARMword ARMul_SetMemSize(ARMul_State *state,ARMword size);
extern ARMword ARMul_GetMemSize(ARMul_State *state);

/*
 * Change the config - bigend, etc. - and call upcalls.
 *
 * "changed" is a bit-mask of bits in the config to change; "config" is a
 * bit-mask of the new values
 */
extern ARMword ARMul_SetConfig(ARMul_State *state,
                               ARMword changed,
                               ARMword config);

/*
 * Branch to some SWI Handler code
 *
 * This function changes to SWI mode, and then gets the ARMulator to
 * start fetching from the specified address, as if a SWI handler had
 * decoded a SWI to be handled at that address and branched to it.
 */
extern void ARMul_SWIHandler(ARMul_State *state,ARMword address);

/*
 * Interface onto disassembler
 */
extern char *ARMul_Disass(ARMword instr, ARMword address, ARMword cpsr);

/*
 * Utility function - take a value and return a suitable
 * method for printing it.
 * Returns "k", "M" or "G" for "mult". Reduce "value" to a double.
 */
extern double ARMul_SIRange(ARMword value, char **mult, bool power_of_two);


#endif
