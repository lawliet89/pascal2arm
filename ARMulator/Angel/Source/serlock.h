/* -*-C-*-
 *
 * $Revision: 1.10.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:54:49 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Definitions for the serial kernel of Angel.
 *
 */

#ifndef angel_serlock_h
#define angel_serlock_h

#ifdef TARGET
# include "angel.h"
#else
# include "host.h"
#endif

/****************************************************************************/

/*
 * This definition is central to the way in which the serial core of Angel
 * keeps track of interrupted and schedulable states.
 *
 * The following structure is capable of holding the complete set of
 * registers, sufficient to resume an instruction stream in exactly the same
 * state as it was in when it was interrupted (or interrupted itself). The .r
 * field holds the nominal registers r0 to r15 (= pc), while the .cpsr and
 * .spsr fields hold the Current Program Status register and the Saved
 * Program Status Register respectively.
 *
 * The identity of banked registers is implied by the mode held in the .cpsr
 * field.
 *
 * Note: the order of the fields is optimized so that the entire contents can
 * be placed on a full descending stack.
 */

typedef struct angel_RegBlock
{
    unsigned cpsr;
    unsigned spsr;
    unsigned r[16];
} angel_RegBlock;

typedef enum angel_TaskPriority {
    TP_IdleLoop = 0,
    TP_AngelInit = 1,
    TP_Application = 2,
    TP_ApplCallBack = 3,
    TP_AngelCallBack = 4,
    TP_AngelWantLock = 5
} angel_TaskPriority;

#define TP_MaxEnum  (TP_AngelWantLock)


/****************************************************************************/

/*
 * The following items are the (globally accessible) register blocks
 * used by interrupt handlers, the undefined instruction handler and
 * the SWI handler.  They are used to hold an interrupted_regblock
 * (element 0) and a desired_regblock (element 1) to be used by
 * SerialiseTask.
 */

extern angel_RegBlock Angel_MutexSharedTempRegBlocks[2];

/****************************************************************************/

/*
 *        Function:  Angel_SerialiseTask
 *         Purpose:  To queue a function to be executed in a serial queue of
 *                   actions with "the lock". In this desired state, mutual
 *                   exclusion is automatically achieved, by serialization.
 *
 *       Arguments:  called_by_yield       1 if called by Angel_Yield
 *                                         0 otherwise.
 *                   fn:                   is the function which desires the
 *                                         lock
 *                   state:                is a parameter for fn
 *                   empty_stack:          the value of the stack pointer
 *                                         of the current mode such that its
 *                                         stack is empty - it will be reset
 *                                         to this value, and hence fn must
 *                                         not need to access any items which
 *                                         might have been left there
 *
 * Implicit Argument Angel_MutexSharedTempRegBlocks[0] must hold the
 *                   interrupted regblock on entry to Angel_SerialiseTask.
 *
 *  Pre-conditions:  This function may be called from IRQ, FIQ, UND or SVC
 *                   mode.
 *
 *          Effect:  If the lock is presently unowned, fn will be executed
 *                   immediately in SVC with the I-bit and F-bit clear. If it
 *                   is already owned, however, the complete context needed
 *                   to execute it later is saved.
 *
 *                   This is not a "normal" function, in the sense that it
 *                   does not necessarily preserve sequence.
 *
 *                   When fn is ready to be executed, the registers will be
 *                   as follows:
 *
 *                      r0 (a1) = <state>
 *                      sp     -> (empty) SVC stack area
 *                      sl      = SVC stack limit
 *                      fp      = 0    (no previous frames)
 *                      lr     -> entry point to NextTask
 *                      pc     -> <fn>
 *                      CPSR   => SVC mode, I-clear, F-clear
 *
 *                   Thus, when fn exits, it will invoke NextTask.
 */

typedef void (*angel_SerialisedFn)(void *);

void Angel_SerialiseTask(bool called_by_yield,
                         angel_SerialisedFn fn,
                         void *state,
                         unsigned empty_stack);

/****************************************************************************/

/*
 *        Function:  Angel_QueueCallback
 *         Purpose:  This routine enables device drivers, the breakpoint
 *                   (undefined instruction) handler, the SWI handler or the
 *                   "yield" code to queue requests.
 *
 *                   It is, in fact, just a veneer on QueueTask - for the
 *                   sake of ease of use.
 *
 *       Arguments:  fn:       the function to be placed in the appropriate
 *                             queue
 *                   priority: this identifies the queue into which the
 *                             request is to b placed
 *                   a1:       first argument to <fn> (goes in r0)
 *                   a2:       second argument to <fn> (goes in r1)
 *                   a3:       third argument to <fn> (goes in r2)
 *                   a4:       fourth argument to <fn> (goes in r3)
 *
 *  Pre-conditions:  This code must be called in SVC mode, with the lock.
 *
 *          Effect:  QueueTask is called, with registers set up as above,
 *                   and in addition:
 *
 *                     fp   = 0  (no previous frames)
 *                     lr  -> entry point of NextTask
 *                     pc  -> <fn>
 *                     cpsr = USR, I-clear, F-clear
 *                     sl, sp are set to be the Application stack
 *
 *            Note:  This routine is not atomic, but it does call QueueTask,
 *                   which is.
 */

typedef void (*angel_CallbackFn)(void *a1,
                                 void *a2,
                                 void *a3,
                                 void *a4);

void Angel_QueueCallback(angel_CallbackFn fn,
                         angel_TaskPriority priority,
                         void *a1,
                         void *a2,
                         void *a3,
                         void *a4);

/****************************************************************************/

/*
 *        Function:  Angel_BlockApplication
 *         Purpose:  To block the application (both its "normal level" and
 *                   any callbacks).
 *
 *        Argument:  The argument value controls whether the application is
 *                   to be blocked or unblocked:
 *
 *                       0 => unblock application tasks
 *                       1 => block application tasks
 *
 *  Pre-conditions:  None - this function does not need to be atomic.
 *
 *          Effect:  The application is blocked or unblocked, according to
 *                   the value of "value".
 */

void Angel_BlockApplication(int value);

/*
 *        Function:  Angel_IsApplicationBlocked
 *         Purpose:  To return whether or not the Application and 
 *                   Application Callbacks have been blocked by Angel
 *
 *       Arguments:  None
 *
 *  Pre-conditions:  None - this function does not need to be atomic.
 *
 *          Effect:  0 is returned if the Application is not blocked
 *                   1 is returned if the Application is blocked
 */

int Angel_IsApplicationBlocked(void);

/*
 *        Function:  Angel_IsApplicationRunning
 *         Purpose:  To return whether or not the Application or an
 *                   Application Callback is running.  This can be used
 *                   to determine whether or not it is safe to switch
 *                   tasks in an OS scheduler.
 *
 *       Arguments:  None
 *
 *  Pre-conditions:  None - this function does not need to be atomic.
 *
 *          Effect:  1 is returned if the Application/Appl Callback is running
 *                   0 is returned if not
 */

int Angel_IsApplicationRunning(void);

/*
 *        Function:  Angel_AccessApplicationRegblock
 *         Purpose:  To obtain a pointer to the register block structure which
 *                   contains the registers of the application.
 *
 *  Pre-conditions:  This function does not need to be atomic. It is assumed
 *                   that the application is blocked.
 *
 *          Effect:  A pointer to the relevant data structure is returned.
 *                   This structure will remain valid until the application
 *                   is unblocked.
 */

angel_RegBlock *Angel_AccessApplicationRegBlock(void);

/*
 *        Function:  Angel_FlushApplicationCallbacks
 *         Purpose:  To remove any outstanding callbacks from the application
 *                   task - that is, any with priority ApplCallBack.
 *
 *  Pre-conditions:  This must be executed in SVC, either with the lock, or
 *                   with interrupts disabled.
 *
 *          Effect:  All outstanding callbacks are removed from the queue
 *                   waiting to be effected by the application.
 *
 *                   This routine is needed in two circumstances: on loading
 *                   a new application, or on reloading the current one.
 *
 *                   It should in fact be called only from the function
 *                   InitialiseApplication.
 */

void Angel_FlushApplicationCallbacks(void);

/****************************************************************************/

/*
 *        Function:  Angel_Yield
 *         Purpose:  This is a voluntary yield function, which allows the
 *                   polling loop to execute; this permits an application to
 *                   give control to the polling loop and perform any necessary
 *                   polling actions.
 *
 *  Pre-conditions:  This routine may be called either from USR or from SVC.
 *                   In the latter case, the lock should be held.
 *
 *          Effect:  If not in SVC, a transparent stack switch is made (the
 *                   USR stack pointer and stack limit register are copied to
 *                   the SVC ones) so that the code is APCS conformant. The
 *                   idle loop is then called so that any polled devices may
 *                   be checked.
 *
 *                   On exit, a transparent switch is made back to USR if the
 *                   original call came from USR - in which case the SVC stack
 *                   is also reset so that it is empty.
 */

void Angel_Yield(void);


/*
 *        Function:  Angel_YieldCore
 *         Purpose:  Given a uniform enviroment with the lock, give
 *                   polled devices a chance to do their work.
 *
 *       Arguments:  None.
 *
 *   Pre-conditions: This function will be called from SVC mode with the
 *                   SVC stack set up for use.  On entry IRQ's and
 *                   FIQ's will be enabled, and the lock will be held
 *                   by this function.  It will only ever be called from
 *                   the assembler veneer code in Angel_Yield
 *
 *                   In practice this means it should only be called via
 *                   the serialiser, and the only user code which might
 *                   ever want to do this is code to call Angel Yield at
 *                   regular intervals off an interrupt.
 *
 *          Effect:  All polled devices will be given a chance to do
 *                   their work.
 *
 *                   This is an APCS conformant function, but the veneer
 *                   which calls it is not.
 *
 *  Post-conditions: This function should be exited just by using return.
 *                   This will actually return to angel_NextTask.
 */

extern void Angel_YieldCore(void);


/****************************************************************************/

/*
 *        Function:  Angel_EnterSVC
 *         Purpose:  Switch to SVC mode from USR mode, setting the I-bit
 *                   and the F-bit.
 *
 *  Pre-conditions:  The caller must presently be executing in USR.
 *
 *          Effect:  Execution continues in SVC transparently. The I-bit and
 *                   the F-bit are both set. A switch of stacks occurs (the
 *                   USR stack pointer and stack limit are copied to the
 *                   SVC registers), so that the transition is transparent to
 *                   APCS.
 */

void Angel_EnterSVC(void);

/*
 *        Function:  Angel_ExitToUSR
 *         Purpose:  Switch back to USR mode after executing for a while
 *                   in SVC.
 *
 *  Pre-conditions:  The caller must presently be executing in SVC.
 *
 *          Effect:  Execution continues in USR mode transparently. The I-bit
 *                   and F-bit are both cleared. The SVC stack pointer and
 *                   stack limit are copied to the USR registers, so that the
 *                   transition is transparent to APCS, and the SVC stack
 *                   is reset to an empty state.
 */

void Angel_ExitToUSR(void);


/*
 *        Function:  Angel_DisableInterruptsFromSVC
 *         Purpose:  Disable interrupts while executing in SVC mode
 *
 *  Pre-conditions:  The caller must presently be executing in SVC mode
 *                   and must have obtained the serialiser lock.
 *
 *          Inputs:  None
 *
 *         Returns:  Processor State on entry to the routine
 *
 *          Effect:  Interrupts are disabled.
 */
unsigned int Angel_DisableInterruptsFromSVC(void);

/*
 *        Function:  Angel_EnableInterruptsFromSVC
 *         Purpose:  Enable interrupts while executing in SVC mode
 *
 *  Pre-conditions:  The caller must presently be executing in SVC mode
 *                   and must have obtained the serialiser lock.
 *
 *          Inputs:  Nothing
 *
 *         Returns:  Processor State on entry to the routine
 *
 *          Effect:  Interrupts are enabled.
 */
unsigned int Angel_EnableInterruptsFromSVC(void);

/*
 *        Function:  Angel_RestoreInterruptsFromSVC
 *         Purpose:  Enable interrupts while executing in SVC mode
 *
 *  Pre-conditions:  The caller must presently be executing in SVC mode.
 *
 *          Inputs:  state      State returned from previous call to
 *                              Angel_DisableInterruptsFromSVC() or
 *                              Angel_EnableInterruptsFromSVC().
 *
 *          Effect:  The interrupt state is restored.
 */
void Angel_RestoreInterruptsFromSVC(unsigned int state);

/****************************************************************************/

/*        Function:  angel_InitialiseOneOff
 *         Purpose:  To initialize the free angel_RegBlocks and all the
 *                   associated structures, returning with the application
 *                   task flagged as as blocked, and returning a pointer to
 *                   a valid angel_RegBlock (whose contents are meaningless
 *                   at this point).
 *
 *  Pre-conditions:  This routine is called once only, by the main Angel
 *                   initialization code. It is assumed that the I-bit and the
 *                   F-bit are set, and that the routine is called in SVC.
 *
 *          Inputs:  None.
 *
 *         Outputs:  None. (The angel_RegBlock allocated may be accessed via
 *                   the standard route.)
 */

void angel_InitialiseOneOff(void);

/*
 *
 */
void Angel_InitialiseTaskFinished(void);


/*        Function:  angel_ReadBankedRegs
 *         Purpose:  To save the state of banked registers when the debugger
 *                   asks for them in debogos.c
 *
 *  Pre-conditions:  It is called in USR mode
 *
 *          Inputs:  A Regblock in which to save the state of banked regs
 *                   The mode of the regs to read.
 *
 *         Outputs:  None
 */

void angel_ReadBankedRegs(angel_RegBlock *rb, unsigned mode);


/****************************************************************************/

/*
 *  The following enumerated constants control the handling of the F-bit,
 *  depending on the use made of FIQs in the particular configuration.
 *
 *  The definitions are as follows:
 *
 *    FIQ_CannotBeOptimised:
 *
 *        If this option is selected, the F-bit and the I-bit will be
 *        disabled together - it is assumed that an FIQ may do anything
 *        which an IRQ may do (such as enter the serialiser and perform
 *        any actions available to it in that state).
 *
 *    FIQ_NeverUsesSerialiser:
 *
 *        If this option is selected, it is assumed that FIQs never attempt
 *        to gain the lock, and so are completely independent of the lock
 *        mechanism.
 *
 *    FIQ_NeverUsesSerialiser_DoesNotReschedule:
 *
 *        If this option is selected, there is added an additional constraint:
 *        FIQs must _never_ be used for rescheduling in any third party OS.
 *
 *    FIQ_NeverUsesSerialiser_DoesNotReschedule_HasNoBreakpoints:
 *
 *        If this option is selected, no breakpoint must ever occur in code
 *        executed in FIQ mode.
 *
 *  The #define below is the default (i.e. safest) option.
 */

typedef enum angel_FIQSafety {
  FIQ_CannotBeOptimised = 0,
  FIQ_NeverUsesSerialiser = 1,
  FIQ_NeverUsesSerialiser_DoesNotReschedule = 2,
  FIQ_NeverUsesSerialiser_DoesNotReschedule_HasNoBreakpoints = 3
} angel_FIQSafety;

/* Note that FIQ_SAFETYLEVEL is defined in devconf,h */

/*
 * Macros Enter / Leave a critical section, that is, an area of code in
 * which the task must not be interrupted, e.g. by a device driver.
 *
 * Under Angel, this is implemented by calls to enter supervisor mode,
 * which are queued if more than one such task makes the request.
 */
#define Angel_EnterCriticalSection()  Angel_EnterSVC()
#define Angel_LeaveCriticalSection()  Angel_ExitToUSR()

#endif /* ndef angel_serlock_h */

/* EOF serlock.h */
