/* -*-C-*-
 *
 *    CVS $Revision: 1.1.2.3 $
 * Revising $Author: rivimey $
 *            $Date: 1997/12/22 14:14:30 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * logtarget.h - methods for logging warnings, errors and trace info when
 *               running on the target (PID etc)
 *
 */

#ifndef angel_logtarget_h
#define angel_logtarget_h

/*
 * log_angel_restart
 * -------------------------
 * Called when Angel has paniced and thinks it can't continue; only
 * used in release versions (debugging versions use uninterruptible_loop).
 * Does *most* of the things pressing the "reset" switch would do.
 */
void __rt_angel_restart(void);

/*
 * log_uninterruptable_loop
 * -------------------------
 * Called when Angel has paniced and thinks it can't continue; only
 * used in debugging versions (release versions use restart instead).
 * Useful only because you can breakpoint the routine to catch the
 * errors.
 */
void __rt_uninterruptable_loop(void);


/*
 * The next routines only used when debugging (calls to LogError,
 * in release mode call log_angel_restart; calls to LogInfo, etc,
 * are compiled out entirely).
 */

#if defined(DEBUG) && (DEBUG == 1)

/*---------------------------------------------------------------------------*/

/*
 * The next section defines macros intended for use in the rest of Angel.
 * See the comment in logging.h for details of how to use them.
 */

#define LogFatalError(id, stuff) (log_cond(id) ? \
                                         (Log_logmsginfo( log_file, __LINE__, id), \
                                          log_logerror stuff, 0 ) : 0, \
                                          __rt_uninterruptable_loop())

#define LogError(id, stuff)       (log_cond(id) ? \
                                        (Log_logmsginfo( log_file, __LINE__, id), \
                                         log_logerror stuff , 0 ): 0)

#define LogWarning(id, stuff)   (log_cond(id) ? \
                                        (Log_logmsginfo( log_file, __LINE__, id), \
                                         log_logwarning stuff, 0 ): 0)

/* set NO_LOG_INFO to kill the info calls, but leave warn, error ... */
/* ... mainly to reduce the size of the image, and speed it up a bit */
#ifdef NO_LOG_INFO
#define LogInfo(id, stuff)        (void)0
#else
#define LogInfo(id, stuff)        (log_cond(id) ? \
                                        (Log_logmsginfo( log_file, __LINE__, id), \
                                         log_loginfo stuff, 0 ): 0)
#endif /* NO_LOG_INFO */

#else /* !DEBUG -- i.e. release code */

#define LogFatalError(id, stuff)  __rt_angel_restart()
#define LogError(id, stuff)       (void)0
#define LogWarning(id, stuff)     (void)0
#define LogInfo(id, stuff)        (void)0

#endif


#if defined(ASSERT_ENABLED) && (ASSERT_ENABLED == 1)
# if defined(DEBUG) && (DEBUG == 1)

#  define ASSERT(cond, stuff)       ((cond) ? 0 : \
        (Log_logmsginfo( log_file, __LINE__, LOG_NEVER ), \
         log_logerror("Assert Failed: \"%s\" : ", STRINGIFY(cond)), \
         log_logerror stuff, __rt_uninterruptable_loop(), 0))

# else

#  define ASSERT(cond, stuff)       ((cond) ? 0 : (__rt_angel_restart(),0) )

# endif

#else /* !ASSERTIONS_ENABLED */
# define ASSERT(cond, stuff)
#endif

/*---------------------------------------------------------------------------*/

#endif /* ndef angel_logtarget_h */

/* EOF angel_logtarget.h */
