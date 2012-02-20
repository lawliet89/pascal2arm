/* -*-C-*-
 *
 *    CVS $Revision: 1.1.2.4 $
 * Revising $Author: rivimey $
 *            $Date: 1998/01/06 11:25:01 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * logtarget.h - methods for logging warnings, errors and trace info when
 *               running on the target (PID etc)
 *
 */

#ifndef angel_loghost_h
#define angel_loghost_h

#include <setjmp.h>

/*
 * The next routines only used when debugging (calls to LogError,
 * in release mode call log_angel_restart; calls to LogInfo, etc,
 * are compiled out entirely).
 */
#if DEBUG == 1

/*---------------------------------------------------------------------------*/

/*
 * The next section defines macros intended for use in the rest of Angel.
 * See the comment in logging.h for details of how to use them.
 */

#define LogFatalError(id, stuff)  (log_cond(id) ? \
                                     (Log_logmsginfo( log_file, __LINE__, id), \
                                      log_logerror stuff, 0 ) : 0, \
                                      angel_Die())

#define LogError(id, stuff)       (log_cond(id) ? \
                                     (Log_logmsginfo( log_file, __LINE__, id), \
                                      log_logerror stuff , 0 ): 0)

#define LogWarning(id, stuff)     (log_cond(id) ? \
                                     (Log_logmsginfo( log_file, __LINE__, id), \
                                      log_logwarning stuff, 0 ): 0)

#ifdef NO_LOG_INFO
#define LogInfo(id, stuff)        (void)0
#else
#define LogInfo(id, stuff)        (log_cond(id) ? \
                                     (Log_logmsginfo( log_file, __LINE__, id), \
                                      log_loginfo stuff, 0 ): 0)
#endif

#else /* !DEBUG -- i.e. release code */

#define LogFatalError(id, stuff)   angel_Die()
#define LogError(id, stuff)       (void)0
#define LogWarning(id, stuff)     (void)0
#define LogInfo(id, stuff)        (void)0

#endif
    
extern bool angel_is_broken;
extern jmp_buf angel_RDI_start_env;

#define angel_Die()                 angel_is_broken = 1,longjmp(angel_RDI_start_env, 1)

#if ASSERT_ENABLED == 1
# if DEBUG == 1

#  define ASSERT(cond, stuff)       ((cond) ? 0 : \
                                     (Log_logmsginfo( log_file, __LINE__, 0 ), \
                                      log_logerror("Assert Failed: \"%s\" : ", STRINGIFY(cond)), \
                                      log_logerror stuff, angel_Die(), 0))

# else

#  define ASSERT(cond, stuff)       ((cond) ? 0 : (angel_Die(),0) )

# endif

#else /* !ASSERTIONS_ENABLED */
/* no assertions wanted */
# define ASSERT(cond, stuff)
#endif

/*---------------------------------------------------------------------------*/

#endif /* ndef angel_loghost_h */

/* EOF angel_loghost.h */
