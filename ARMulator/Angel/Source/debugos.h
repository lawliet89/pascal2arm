/* -*-C-*-
 *
 * $Revision: 1.9.4.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:54:40 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * debugos.h - operating system debug support function declarations.
 */

#ifndef angel_debugos_h
#define angel_debugos_h

#include "arm.h"
#include "dbg_cp.h"


/* This is used to store the chosen endianess of the host debugger in.
 * This is needed by ICEMAN2
 */
extern unsigned int angel_debugger_endianess;

/*
 * All the routines return RDIError_NoError if they succeed.
 * Nonzero if they fail.
 *
 * What's missing: Coprocessor description calls. Anything to with
 * vector catching (is this needed?)
 */
extern int angelOS_Initialise(void);

extern struct AngelOSInfo OSinfo_struct;

typedef enum {
  bs_32bit,
  bs_16bit
} BreakPointSize;

struct BreakPoint {
  struct BreakPoint *next;
  word address;
  word inst;
  unsigned count;
  int bpno;                /* Index to array where this structure is saved. */
  BreakPointSize size;
};

extern struct BreakPoint bp[32];
extern word free_bps;

/* This flag gets set only during a debugger requested read or write
 * access - ie. one done by MemRead or MemWrite in this module.  It is
 * needed by the exception handling code in order for it to work out
 * whether to report that the user application has aborted or whether
 * to flag an abort and just resume execution of Angel.  Thus there is
 * also a flag which gets set by the exception routines in this
 * circumstance.
 */
extern int memory_is_being_accessed;

extern volatile int memory_access_aborted;


/* angelOS_SemiHostingEnabled is used to determine whether or not
 * C Library SWI's should cause semihosting to happen or whether
 * they should be treated as unimplemented SWI's by the standard
 * Angel SWI handler
 * 1 => Semihosting is on, 0 => Semihosting is off */
extern int angelOS_SemiHostingEnabled;

extern int angelOS_MemRead(word OSinfo1, word OSinfo2, word Address,
                           word Nbytes, byte *DataArea, word *BytesRead);

extern int angelOS_MemWrite(word OSinfo1, word OSinfo2, word Address,
                            word Nbytes, byte *DataArea, word *BytesWritten);

extern int angelOS_CPUWrite(word OSinfo1, word OSinfo2, word Mode, 
                            word Mask, byte *WriteData);
extern int angelOS_CPURead(word OSinfo1, word OSinfo2, word Mode,
                           word Mask, byte *ReadData);

extern int angelOS_CPWrite(word OSinfo1, word OSinfo2, word CPnum, word Mask,
                           word *Data, word *nbytes);
extern int angelOS_CPRead(word OSinfo1, word OSinfo2, word CPnum, word Mask,
                          word *Data, word *nbytes);

/*
 * The next two structures are used by the Set Watch/Breakpoint calls.
 */
struct SetPointValue {
        word pointAddress;
        byte pointType;
        byte pointDatatype;     /* Only used for watchpoint calls. */
        word pointBound;        /* Optional. */
};

/*
 * These values are optional depending up the call.
 * It could also be defined as a union.
 */
struct PointValueReturn {
        word pointHandle;
        word pointRaddress;
        word pointRbound;
};


extern int angelOS_SetBreak(word OSinfo1, word OSinfo2, 
                           struct SetPointValue *SetData,
                           struct PointValueReturn *ReturnValue);

extern int angelOS_ClearBreak(word OSinfo1, word OSinfo2, word Handle);

extern int angelOS_SetWatch(word OSinfo1, word OSinfo2,
                           struct SetPointValue *SetData,
                           struct PointValueReturn *ReturnValue);

extern int angelOS_ClearWatch(word OSinfo1, word OSinfo2, word Handle);

/*
 * Used to set up a process/task/thread for execution.
 */
extern int angelOS_InitialiseApplication(word OSinfo1, word OSinfo2);

extern int angelOS_Execute(word OSinfo1, word OSinfo2);

extern int angelOS_SetupStep(word OSinfo1, word OSinfo2, word StepCount,
                             void **handle);
extern int angelOS_DoStep(void *handle);

extern int angelOS_InterruptExecution(word OSinfo1, word OSinfo2);

/*
 * Called when a thread stops. 'type' is one of the ADP_Stopped_
 * codes.
 *
 * This fn runs with the serialiser lock in SVC mode.  All it should
 * do is Queue a Callback to deal with the thread stopping.
 */
extern void angelOS_ThreadStopped(word type);

/*
 * The Angel execptions code has a set of soft vectors set up.
 * May be the install handler functionality should go into Angel to
 * Here is the call the OS needs to install its own handlers.
 * 
 * VectorID is an integer (Assigned in the conventional manner 
 * i.e. Reset vector=0 through to FIQ vector = 7)
 * oldHandler is used to return the address of the previous handler. If
 * this pointer is null, the routine will not try to return it.
 */
extern int AngelOS_SetHandler(int VectorID, void (*handler)(),
                              void *(*oldHandler)());

/*
 * Structure used by angelOS_ReturnInfo.
 * The OS side needs to be `understand' the ADP bit masks for the 
 * first two values.
 */
struct AngelOSInfo {
        word infoPoints; /* Break/watch point availability */
        word infoStep;   /* Single stepping ability */
        word infoBitset; /* General abilities -- see adp.h */
        word infoModel;  /* Target hardware ID word */
};

/*
 * These ADP subreason codes require information that is target specific:
 *      ADP_Info_Points
 *      ADP_Info_Step
 *      ADP_Info_Target_Interrupt
 *
 * This call returns a pointer to such info.
 */
extern struct AngelOSInfo *angelOS_ReturnInfo(void);
        
extern void angelOS_ExitToOS(void);  /* Never returns */

extern void angelOS_FatalError(char *msg);  /* Never returns */

extern int angelOS_End(word debugID); /* Host debugger is shutting down */

#if DEBUG == 1
extern int angelOS_PrintState(word OSinfo1, word OSinfo2);
#endif

extern int angelOS_MemInfo( word *meminfo );
extern int angelOS_DescribeCoPro( byte cpno, struct Dbg_CoProDesc *cpd );
extern int angelOS_RequestCoProDesc( byte cpno, struct Dbg_CoProDesc *cpd );
extern int angelOS_VectorCatch( word OSinfo1, word OSinfo2, word VectorCatch );
extern int angelOS_WatchPointStatus(word OSinfo1, word OSinfo2, word Handle,
                                    word *Hw, word *Type);
extern int angelOS_BreakPointStatus(word OSinfo1, word OSinfo2, word Handle,
                                    word *Hw, word *Type);
extern int angelOS_SemiHosting_SetState( word OSinfo1, word OSinfo2,
                                         word SemiHosting_State );
extern int angelOS_SemiHosting_GetState( word OSinfo1, word OSinfo2,
                                         word *SemiHosting_State );
extern int angelOS_SemiHosting_SetVector( word OSinfo1, word OSinfo2,
                                          word SemiHosting_Vector );
extern int angelOS_SemiHosting_GetVector( word OSinfo1, word OSinfo2,
                                          word *SemiHosting_Vector );
extern int angelOS_Ctrl_Log( word OSinfo1, word OSinfo2, word *LogSetting );
extern int angelOS_Ctrl_SetLog( word OSinfo1, word OSinfo2, word LogSetting );
extern int angelOS_CanChangeSHSWI( void );
extern int angelOS_SemiHosting_SetARMSWI( word OSinfo1, word OSinfo2,
                                          word semihosting_armswi );
extern int angelOS_SemiHosting_GetARMSWI( word OSinfo1, word OSinfo2,
                                          word *semihosting_armswi );
extern int angelOS_SemiHosting_SetThumbSWI( word OSinfo1, word OSinfo2,
                                          word semihosting_thumbswi );
extern int angelOS_SemiHosting_GetThumbSWI( word OSinfo1, word OSinfo2,
                                          word *semihosting_thumbswi );
extern int angelOS_LoadConfigData( word OSinfo1, word OSinfo2,
                                   word nbytes, byte const *data );
extern void angelOS_ExecuteNewAgent( word startaddress );
extern int angelOS_LoadAgent( word OSinfo1, word OSinfo2, word loadaddress,
                              word nbytes );

#endif /* !defined(angel_debugos_h) */

/* EOF debugos.h */
