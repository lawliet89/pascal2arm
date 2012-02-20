/* -*-C-*-
 *
 * $Revision: 1.16.4.3 $
 *   $Author: rivimey $
 *     $Date: 1998/01/23 18:47:42 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Implementation of serialiser module
 */

#include "serlock.h"
#include "logging.h"
#include "support.h"
#include "arm.h"
#include "devdriv.h"
#include "devconf.h"
#include "stacks.h"

#if MINIMAL_ANGEL == 0

/* #define DEBUG_TASKS */

/*
 *        Function:  angel_StartTask
 *
 *         Purpose:  To execute a task.
 *
 *       Arguments:  regblock: a pointer to the angel_RegBlock structure
 *                             containing the registers to be instated
 *                             when the tasks executes
 *
 *    Special Note:  This is coded in serlasm.s, as it is not a normal
 *                   function.  However, the prototype is here because
 *                   it is needed by functions in this module.
 *
 *  Pre-conditions:  This code is called from SVC mode with IRQ and FIQ
 *                   disabled.
 *
 *          Effect:  The task described is executed - when that task
 *                   completes control will be returned to angel_NextTask.
 *                   This code just inststes all the registers precisely
 *                   as set up in regblock.  Thus for tasks which are being
 *                   started for the first time, sl, sp, fp, lr and cpsr
 *                   must have already been set up.
 *
 *    Special Note:  A second entry point to this function,
 *                   angel_StartTask_NonSVCEntry, which allows the exception
 *                   handlers a way to restart an aborted task using this
 *                   function, but without having to go into SVC mode.
 */

extern void angel_StartTask(angel_RegBlock * regblock);

/*
 *        Function:  angel_NextTask
 *
 *         Purpose:  Entered when any task completes
 *
 *       Arguments:  None
 *
 *    Special Note:  This is coded in serlasm.s, as it is not a normal
 *                   function.  However, the prototype is here because
 *                   it is needed by functions in this module.
 *
 *  Pre-conditions:  This code is called from either SVC mode or USR mode
 *                   with IRQ and FIQ enabled.
 *
 *          Effect:  SVC mode is entered and the SVC stack grabbed.
 *                   Interrupts are disabled, and then angel_SelectNextTask
 *                   is called.
 */

extern void angel_NextTask(void);


/*
 *        Function:  angel_IdleLoop
 *
 *         Purpose:  Entered when there are no tasks ready to execute and
 *                   there is nothing to do right now.
 *
 *       Arguments:  None
 *
 *    Special Note:  This is coded in serlasm.s, as it is not a normal
 *                   function.  However, the prototype is here because
 *                   it is needed by functions in this module.
 *
 *  Pre-conditions:  This code is called from either SVC mode
 *                   with IRQ and FIQ enabled.
 *
 *          Effect:  The SVC stack is emptied, and then interrupts are enabled
 *                   angel_Yield is then repeatedly called to see if there is
 *                   anything to do.
 */

extern void angel_IdleLoop(void);

/**********************************************************************/

angel_RegBlock Angel_MutexSharedTempRegBlocks[2];


/*
 * angel_TaskQueueItem: each item has a forward pointer a RegBlock
 *                      index and a priority.
 *
 * The index is the bit number used in angel_FreeBitmap.
 */

typedef struct angel_TaskQueueItem
{
    struct angel_TaskQueueItem *next;
    int index;
    angel_TaskPriority pri;
    bool new_task;              /* FALSE if interrupted thread */
    angel_RegBlock rb;
} angel_TaskQueueItem;


static angel_TaskQueueItem *angel_TaskQueueHead;

/*
 * This is the number of RegBlocks we support, and also the number of
 * entries we can cope with in the Task Queue.  If it is made to
 * exceed 31 then we must recode the bitmap manipulation stuff.
 */
#define POOLSIZE   12

/* This holds which of the pool entries are free and which used.
 * A set bit means that entry is FREE
 */
static unsigned angel_FreeBitMap;

static int angel_ApplicationBlocked;

static angel_TaskPriority angel_LastExecutedTaskPriority;

static angel_TaskQueueItem angel_TQ_Pool[POOLSIZE];

/**********************************************************************/

/*
 * The freeing and allocation of register blocks may be done by index only.
 */

#define ISFREE(n)  ((angel_FreeBitMap & (1 << (n))) != 0)
#define ISALLOC(n) ((angel_FreeBitMap & (1 << (n))) == 0)

#define FREE(n)    angel_FreeBitMap |= (1 << (n))
#define ALLOC(n)   angel_FreeBitMap &= ~(1 << (n))

/* angel_FreeToPool
 *
 * This function is passed the index of a pool entry to be freed.
 *
 * It will modify the queue pointers appropriately as well as marking
 * this pool element as free.
 */

static void 
angel_FreeToPool(int n)
{
    angel_TaskQueueItem *my_tqi = &angel_TQ_Pool[n];
    angel_TaskQueueItem *tqi, *l_tqi;

    if (ISFREE(n))
        LogError(LOG_SERLOCK, ( "Attempt to deallocate free block %d.\n", n));

    FREE(n);

    /* Now we find it on the queue and remove it */
    for (tqi = angel_TaskQueueHead, l_tqi = NULL;
         (tqi != my_tqi && tqi != NULL);)
    {
        l_tqi = tqi;
        tqi = tqi->next;
    }

    if (tqi == NULL)
    {
        LogError(LOG_SERLOCK, ( "Block could not be dequeued (%d).\n", n));
        return;
    }

#ifdef DO_TRACE
    LogTrace(LOG_SERLOCK, "SFreePool: %d\n", n);
#endif

    if (l_tqi == NULL)
        angel_TaskQueueHead = my_tqi->next;
    else
        l_tqi->next = my_tqi->next;
    my_tqi->next = NULL;

}

/* angel_AllocateFromPool
 *
 * This function is passed a priority and 'new task' flag and will return
 * the index number of the pool entry which has been set up.
 *
 * It will modify the queue pointers appropriately, and sets up the
 * priorities and new_flag members.  It does not set up the regblock
 * in any way.
 *
 * A task is put on the front of the part of the queue of the
 * appropriate priority only if it is neither a new task nor a yielded
 * task (ie. a real interrupted task).  All others are put an the end
 * of the part of the queue with the appropriate priority
 */

static int 
angel_AllocateFromPool(angel_TaskPriority pri, bool new_task,
                       bool yielded)
{
    int i;

    for (i = 0; i < POOLSIZE; i++)
    {
        if (ISFREE(i))
        {
            angel_TaskQueueItem *my_tqi = &angel_TQ_Pool[i];
            angel_TaskQueueItem *tqi, *l_tqi;

#ifdef DO_TRACE
            LogTrace(LOG_SERLOCK, "SAllocPool: %d-p%d-n%d\n", i, pri, new_task);
#endif

            ALLOC(i);

            /* Set up the priorities */
            my_tqi->pri = pri;
            my_tqi->new_task = new_task;

            /* And insert the item into the queue.
             * which first involves finding a task of same priority and
             * inserting this task before it if not new, or after it if new.
             */

            /* if new or yielded, ensure it goes after any exisiting entry at pri */
            if (new_task || yielded)
                pri = (angel_TaskPriority) (pri - 1);

            for (tqi = angel_TaskQueueHead, l_tqi = NULL;
                 (tqi != NULL) && (tqi->pri > pri);)
            {
                l_tqi = tqi;
                tqi = tqi->next;
            }

            /* We may have reached the end of the list, or we may not have */
            my_tqi->next = tqi;
            if (l_tqi == NULL)
                angel_TaskQueueHead = my_tqi;
            else
                l_tqi->next = my_tqi;
            
            /* LogInfo(LOG_SERLOCK, ( "Allocated RegBlock %d\n",i)); */

            return i;
        }
    }
    LogError(LOG_SERLOCK, ( "AllocateFromPool: No free RegBlock\n"));
#if DEBUG == 1
    /*
     * for (i = 0; i < POOLSIZE; i++)
     *{
     *  angel_TaskQueueItem *my_tqi = &angel_TQ_Pool[i];
     *
     *  LogWarning(LOG_SERLOCK, ( "Task %d: Pri=%d, New=%d, PC=%8x, PSR=%08x\n",
     *                              i, my_tqi->pri, my_tqi->new_task,
     *                              my_tqi->rb.r[15], my_tqi->rb.cpsr));
     *}
     */
#endif
    return -1;
}

/**********************************************************************/

/*
 * This routine looks for an interrupted queued task at the given
 * priority
 */
static angel_RegBlock *
angel_AccessQueuedRegBlock(angel_TaskPriority pri)
{
    angel_TaskQueueItem *tqi;

    /* Find a task with the required priority, not marked as new */
    for (tqi = angel_TaskQueueHead;
         (tqi != NULL) && !((tqi->pri == pri) && (!tqi->new_task));
        )
    {
        tqi = tqi->next;
    }

    if (tqi == NULL)
    {
        LogTrace1("SAccessQueue: p%d-NT ", pri);
        return NULL;
    }
    else
    {
        LogTrace2("SAccessQueue: p%d-t%d ", pri, tqi->index);
        return &(tqi->rb);
    }
}

angel_RegBlock *
Angel_AccessApplicationRegBlock(void)
{
    angel_RegBlock *rb;

    /* Find a task with the Application priority */
    rb = angel_AccessQueuedRegBlock(TP_Application);

    if (rb == NULL)
    {
        LogWarning(LOG_SERLOCK, ( "Could not find application task.\n"));
        return NULL;
    }

    return rb;
}

void 
Angel_BlockApplication(int value)
{
    LogInfo(LOG_SERLOCK, ( "angel_BlockApplication - set to %d.\n", value));
    angel_ApplicationBlocked = value;  /* yes, folks - that simple */
}

int 
Angel_IsApplicationBlocked(void)
{
    LogInfo(LOG_SERLOCK, ("angel_IsApplicationBlocked - returns %d.\n",
              angel_ApplicationBlocked));
    return angel_ApplicationBlocked;
}

int 
Angel_IsApplicationRunning(void)
{
    int running = (angel_LastExecutedTaskPriority == TP_Application) ||
    (angel_LastExecutedTaskPriority == TP_ApplCallBack);

    LogInfo(LOG_SERLOCK, ( "angel_IsApplicationRunning - returns %d.\n", running));
    return running;
}

void 
Angel_FlushApplicationCallbacks(void)
{
    angel_TaskQueueItem *tqi;
    int found;

    LogInfo(LOG_SERLOCK, ( "angel_FlushApplicationCallBakcs entered\n"));

    /* Find all tasks with the Application priority.
     * We have to start the loop from scratch each time we find one
     * because angel_FreeToPool changes all the pointers !
     */
    do
    {
        for (found = 0, tqi = angel_TaskQueueHead;
             (tqi != NULL && tqi->pri != TP_ApplCallBack);)
        {
            tqi = tqi->next;
        }

        if (tqi != NULL)
        {
            LogInfo(LOG_SERLOCK, ( "angel_FlushApplicationCallbacks - found task to flush\n"));
            angel_FreeToPool(tqi->index);
            found = 1;
        }
    }
    while (found == 1);
}

/*
 *        Function:  angel_SetUpTaskEnvironment
 *
 *         Purpose:  To set up the execution environment for a task
 *                   depeneding on its priority.
 *
 *       Arguments:  rb    a pointer to the RegBlock to be updated.
 *
 *                   pri   the priority of the task
 *
 *   Pre-conditions: None
 *
 *          Effect:  sl, sp, fp, lr, cpsr are set up appropriately
 *                   according to the priority requested.
 *
 *  Post-conditions: None
 */

static void 
angel_SetUpTaskEnvironment(angel_RegBlock * rb,
                           angel_TaskPriority pri)
{
    angel_RegBlock *env_rb;

    /* LogInfo(LOG_SERLOCK, ("SetUpTaskEnvironment: p%d\n", pri)); */

    switch (pri)
    {
        case TP_ApplCallBack:
            {
                LogInfo(LOG_SERLOCK, ( "SetUpTask: ApplCallBack\n"));
                
                env_rb = Angel_AccessApplicationRegBlock();
                rb->r[10] = env_rb->r[10];  /* Copy sl from Application */
                rb->r[13] = env_rb->r[13];  /* Copy sp from Application */
                rb->r[11] = 0;      /* fp = 0 */
                rb->r[14] = (unsigned)angel_NextTask;
                rb->cpsr = USRmode;
                break;
            }
            
        case TP_AngelCallBack:
            {
                angel_TaskQueueItem *tqi;

                LogInfo(LOG_SERLOCK, ( "SetUpTask: AngelCallBack\n"));

                /*
                 * Find a task with priority AngelCallBack or AngelInit (used
                 * when initialising) which is not marked as new
                 */
                tqi = angel_TaskQueueHead;
                while ((tqi != NULL) &&
                ((tqi->pri != TP_AngelInit && tqi->pri != TP_AngelCallBack) ||
                 tqi->new_task))
                {
                    tqi = tqi->next;
                }

                if (tqi != NULL)
                {
                    /* There is at least one such task queued */
                    env_rb = &(tqi->rb);

                    LogInfo(LOG_SERLOCK, ("... found 1st task sp=%X, sl=%X, pc=%X, pri=%d\n",
                                            env_rb->r[13], env_rb->r[10], env_rb->r[15], tqi->pri));

                    /*
                     * Look for other (non new) tasks with a lower sp with the same
                     * priority.
                     *
                     * XXX
                     *
                     * This code relies upon:
                     *
                     * 1)  The Task Queue being ordered on priority (highest first)
                     *
                     * 2)  Queued processes of priority TP_AngelCallBack using lower
                     *     regions of the Angel stack than processes of priority
                     *     TP_AngelInit.
                     *
                     * Fortunately, both of these conditions are true.
                     */
                    do
                    {
                        tqi = tqi->next;

                        if (tqi == NULL ||
                            (tqi->pri != TP_AngelInit && tqi->pri != TP_AngelCallBack))
                            break;

                        if (tqi->new_task)
                            continue;

                        LogInfo(LOG_SERLOCK, ("... found nth task sp=%X, sl=%X, pc=%x, pri=%d\n",
                                  tqi->rb.r[13], tqi->rb.r[10], tqi->rb.r[15], tqi->pri));

                        if (tqi->rb.r[13] >= env_rb->r[13])
                            continue;

                        env_rb = &(tqi->rb);
                    }
                    while (TRUE);

                    LogInfo(LOG_SERLOCK, ( "stack being stolen for process @%X\n", rb->r[15]));

                    /*
                     * Now we know that env_rb hold the regs of the task of this priority
                     * with the lowest sp.
                     */
                    env_rb->r[10] = env_rb->r[13] - Angel_AngelStackFreeSpace;
                    rb->r[13] = env_rb->r[10];
                }
                else
                {
                    /* There were no tasks of the sought priority */
                    LogInfo(LOG_SERLOCK, ( "...no task found\n"));
                    rb->r[13] = Angel_StackBase + Angel_AngelStackOffset;
                }

                LogTrace2("SUE-sl%8X-sp%8X ", rb->r[10], rb->r[13]);
                
                rb->r[10] = Angel_StackBase + Angel_AngelStackLimitOffset;
                rb->r[11] = 0;  /* fp = 0 */
                rb->r[14] = (unsigned)angel_NextTask;
                rb->cpsr = USRmode;
                break;
            }

        case TP_AngelWantLock:
            {
                /* LogInfo(LOG_SERLOCK, ( "SetUpTask: AngelWantLock\n")); */
                
                rb->r[10] = Angel_StackBase + Angel_SVCStackLimitOffset;
                rb->r[13] = Angel_StackBase + Angel_SVCStackOffset;
                rb->r[11] = 0;      /* fp = 0 */
                rb->r[14] = (unsigned)angel_NextTask;
                rb->cpsr = SVCmode;
                break;
            }
            
        default:
            /*
             * Never set up anything for TP_Application, TP_IdleLoop or
             * TP_AngelInit
             */
            break;
    }
}

void 
Angel_QueueCallback(angel_CallbackFn fn,
                    angel_TaskPriority priority,
                    void *a1,
                    void *a2,
                    void *a3,
                    void *a4)
{
    int blockno;
    unsigned int s;
    angel_TaskQueueItem *tqi;

    LogInfo(LOG_SERLOCK, ("Angel_QueueCallback: func: %p, pri: %d\n",
              (unsigned)fn, (int)priority));

    s = Angel_DisableInterruptsFromSVC();

    blockno = angel_AllocateFromPool(priority, TRUE, FALSE);
    if (blockno == -1) /* sorry, can't do it! */
    {
        Angel_RestoreInterruptsFromSVC(s);
        LogError(LOG_SERLOCK, ("Angel_QueueCallback: Cannot Queue Callback.\n"));
        return;
    }
    
    tqi = &angel_TQ_Pool[blockno];

    tqi->rb.r[0] = (unsigned)a1;
    tqi->rb.r[1] = (unsigned)a2;
    tqi->rb.r[2] = (unsigned)a3;
    tqi->rb.r[3] = (unsigned)a4;
    tqi->rb.r[15] = (unsigned)fn;

    Angel_RestoreInterruptsFromSVC(s);
}

/*
 *        Function:  angel_QueueTask
 *         Purpose:  To queue action to be performed at a later time.
 *                   It is intended only for use by angel_SerialiseTaskCore
 *
 *       Arguments:  regblock: points to a block of desired registers
 *                   new_pri: indicates the type of task to be queued
 *                   new_task: is set for newly-queued serialised tasks,
 *                             but NOT for interrupted tasks.  Used in 
 *                             AllocateFromPool, and later in SelectNextTask.
 *                   yielded indicates whether this task is being queued
 *                           because it called Angel_Yield - this alters
 *                           whether it should be resumed before or after
 *                           other tasks of the same priority.
 *  Pre-conditions:  This may ONLY be called by angel_SerialiseTaskCore.
 *
 *          Effect:  This function is atomic in its action. It is a normal
 *                   function, returning in the usual way.
 *
 *                   It places an entry in the queue with priority <priority>.
 */

static void 
angel_QueueTask(angel_RegBlock * regblock,
                angel_TaskPriority pri,
                bool new_task,
                bool yielded)
{
    int blockno;
    angel_TaskQueueItem *tqi;

    blockno = angel_AllocateFromPool(pri, new_task, yielded);
    if (blockno == -1) /* sorry, can't do it! */
    {
        LogFatalError(LOG_SERLOCK, ("angel_QueueTask: Cannot Queue Task!\n"));
        /*NOTREACHED*/
    }
    tqi = &angel_TQ_Pool[blockno];
    __rt_memcpy((void *)&(tqi->rb), (const void *)regblock,
                sizeof(angel_RegBlock));
}


/*
 *        Function:  angel_SerialiseTaskCore
 *         Purpose:  To queue a task to be executed in a serial queue of
 *                   actions with "the lock". In this desired state, mutual
 *                   exclusion is automatically achieved, by serialization.
 *
 *       Arguments:  called_by_yield       1 if called by Angel_Yield
 *                                         0 otherwise.
 *                   desired_regblock:     a pointer to the angel_RegBlock
 *                                         structure containing the register
 *                                         values to be instated when the
 *                                         desired task is executed.
 *
 * Implicit Argument Angel_MutexSharedTempRegBlocks[0] must hold the
 *                   interrupted regblock on entry to Angel_SerialiseTask.
 
 *   Pre-conditions: This function will be called from SVC mode with the
 *                   SVC stack set up for use.  On entry IRQ's and
 *                   FIQ's will be disabled.  It will only ever be
 *                   called from the assembler veneer code in
 *                   Angel_SerialiseTask
 *
 *          Effect:  If the lock is presently unowned, desired_regblock will
 *                   be executed immediately in SVC with the I-bit and F-bit
 *                   clear. If it is already owned, however, desired_regblock
 *                   is saved away for later.
 *
 *                   This is an APCS conformant function, but the veneer
 *                   which calls it and the veneer which it calls on exit
 *                   are not APCS conformant.
 *
 *  Post-conditions: This function should be exited by calling
 *                   angel_StartTask to execute the appropriate task.
 */

extern void angel_SerialiseTaskCore(int called_by_yield,
                                    angel_RegBlock * desired_regblock);

void 
angel_SerialiseTaskCore(int called_by_yield,
                        angel_RegBlock * desired_regblock)
{
    /* We can determine that nothing has the "lock" by seeing if the
     * last task that executed was the Application or the Idle Loop.  If
     * it was, then nothing has asked for the "lock" - ie. no non
     * Application task has been queued, or is executing.
     */
    angel_RegBlock *interrupted_regblock = &Angel_MutexSharedTempRegBlocks[0];

    /* LogInfo(LOG_SERLOCK, ("SerialiseTaskCore: lastpri %d, old r15 %8X, new r15 %8X yield? %d\n",
                            angel_LastExecutedTaskPriority, interrupted_regblock->r[15],
                            desired_regblock->r[15], called_by_yield)); */

#if 0
    if (angel_LastExecutedTaskPriority == TP_Application)
    {
        int i;

        LogWarning(LOG_SERLOCK, ( "Application stopped:\n"));
        for (i = 0; i < 16; i++)
        {
            LogWarning(LOG_SERLOCK, ( "%8X ", interrupted_regblock->r[i]));
        }
        LogWarning(LOG_SERLOCK, ( "CPSR=%8X\n", interrupted_regblock->cpsr));
    }
#endif

    switch (angel_LastExecutedTaskPriority)
    {
        case TP_Application:
        case TP_AngelInit:
        case TP_ApplCallBack:
        case TP_AngelCallBack:
            /* Queue the interrupted task, and start the task that wants the lock */
            angel_QueueTask(interrupted_regblock, angel_LastExecutedTaskPriority,
                            FALSE, called_by_yield);
            angel_LastExecutedTaskPriority = TP_AngelWantLock;
            LogTrace1("STC-e%x ", desired_regblock->r[15]);
            angel_SetUpTaskEnvironment(desired_regblock, TP_AngelWantLock);
            angel_StartTask(desired_regblock);
            /* Never get here */

        case TP_IdleLoop:
            /* Throw away the Idle Task and start the task that wants the lock */
            angel_LastExecutedTaskPriority = TP_AngelWantLock;
            LogTrace1("STC-e%x ", desired_regblock->r[15]);
            angel_SetUpTaskEnvironment(desired_regblock, TP_AngelWantLock);
            angel_StartTask(desired_regblock);
            /* Never get here */

        case TP_AngelWantLock:
            /* Queue the task that wants the lock, leave the old one running
             * Leave angel_LastExecutedTaskPriority unchanged
             */
            angel_QueueTask(desired_regblock, TP_AngelWantLock, TRUE, FALSE);
            angel_StartTask(interrupted_regblock);
            /* Never get here */
    }
}


void 
Angel_YieldCore(void)
{

#ifdef DO_TRACE
    LogTrace(LOG_SERLOCK, "SYC ");
#endif

    angel_DeviceYield();
}

/*
 *       Function:  angel_SelectNextask
 *
 *        Purpose:  To select the first available task in the highest
 *                  priority queue which is not empty, and to effect it
 *                  by calling angel_StartTask.
 *
 *      Arguments:  None.
 *
 *  Pre-conditions: This routine must be called in SVC, with the I-bit
 *                  and F-bit set.
 *
 *         Effect:  The queue is scanned in descending order
 *                  of priority. The first non blocked item in the 
 *                  queue is removed, and then effected.
 *                  If there is no task then go into an idle loop.
 */

extern void angel_SelectNextTask(void);

void 
angel_SelectNextTask(void)
{
    angel_TaskQueueItem *tqi;

    /* Find the task with the highest priority which is not blocked */
    for (tqi = angel_TaskQueueHead; tqi != NULL; tqi = tqi->next)
    {
        /* Ignore this task if it is the application or an Appl Callback
         * and the Application is blocked
         */
        if (angel_ApplicationBlocked &&
            (tqi->pri == TP_Application || tqi->pri == TP_ApplCallBack))
        {
            LogTrace2("SNT-SKIP-t%d-p%d ", tqi->index, tqi->pri);
            continue;
        }

        /* Otherwise this is the highest priority task so execute it ! */

#ifdef DEBUG_TASKS
        LogInfo(LOG_SERLOCK, ("SelectNewTask: Selecting pc %x - t%d - p%d - n%d ",
                              tqi->rb.r[15], tqi->index, tqi->pri, tqi->new_task));
#endif
        
        angel_LastExecutedTaskPriority = tqi->pri;
        angel_FreeToPool(tqi->index);

#if 0
        if (tqi->pri == TP_Application)
        {
            int i;

            LogWarning(LOG_SERLOCK, ( "Application starting:\n"));
            for (i = 0; i < 16; i++)
            {
                LogWarning(LOG_SERLOCK, ( "%8X ", tqi->rb.r[i]));
            }
            LogWarning(LOG_SERLOCK, ( "CPSR=%8X\n", tqi->rb.cpsr));
        }
#endif

        /* if it's not an interrupted thread, we need to set its environment */
        if (tqi->new_task)
            angel_SetUpTaskEnvironment(&(tqi->rb), angel_LastExecutedTaskPriority);
        else
            LogTrace("\n");

        angel_StartTask(&(tqi->rb));  /* doesn't return! */
    }

    if (tqi == NULL)
    {
        LogTrace("SNT-IDLE ");
        angel_LastExecutedTaskPriority = TP_IdleLoop;
        angel_IdleLoop();
    }
}

void 
angel_InitialiseOneOff(void)
{
    int i;
    int app_num;

    LogInfo(LOG_SERLOCK, ( "angel_InitialiseOneOff entered\n"));

    angel_FreeBitMap = (1 << POOLSIZE) - 1;
    Angel_BlockApplication(1);
    angel_LastExecutedTaskPriority = TP_AngelInit;

    angel_TaskQueueHead = NULL;

    for (i = 0; i < POOLSIZE; i++)
    {
        angel_TQ_Pool[i].next = NULL;
        angel_TQ_Pool[i].index = i;
    }

    /* The application: always allocate one item anyway. */
    app_num = angel_AllocateFromPool(TP_Application, FALSE, FALSE);
    if (app_num == -1) /* sorry, can't do it! */
    {
        LogFatalError(LOG_SERLOCK, ("angel_InitialiseOneOff: CANT ALLOCATE APPL TASK!\n"));
    }    
}

void 
Angel_InitialiseTaskFinished(void)
{
    ASSERT(angel_LastExecutedTaskPriority == TP_AngelInit, ("bad priority"));
    LogInfo(LOG_SERLOCK, ( "Angel_InitialiseTaskFinished\n"));

    angel_LastExecutedTaskPriority = TP_IdleLoop;
}

#endif /* NOT MINIMAL_ANGEL */

/* This variable is for use by serlasm.s only, and so is not exported
 * to the whole world.  See serlasm.s - Angel_ExitToUser for info.
 */
extern unsigned angel_SVCEntryFlag;

unsigned angel_SVCEntryFlag;

/**********************************************************************/

/* EOF serlock.c */
