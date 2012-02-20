/****************************************************
 * windebug.h - ARM Windows debugging               *
 *==================================================*
 * Outputs debugging information to requested place *
 * Currently only supports up to 255 characters at  *
 *     a time...                                    *
 ****************************************************/

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1996/06/25 14:49:07 $
 * Revising $Author: jporter $
 */

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef ARM_DEBUGGING

    #include "stddef.h"
    #include "windef.h"
    #include "debug.h"
    #define DllImport __declspec(dllimport)

    DllImport BOOL WINAPI ARM_DebugInit(debug_output_type type, char* filename);
    DllImport void WINAPI ARM_DebugComplete();
    DllImport void WINAPI ARM_Debug(char* chk, char *fmt, ...);
    #define ADBG    ARM_Debug

    DllImport void  WINAPI ARM_DebugTimerStart();
    DllImport DWORD WINAPI ARM_DebugTimerRead();
    DllImport void  WINAPI ARM_DebugTimerOutput();

#else
    __inline void ARM_Debug(char* chk, char* fmt, ...) { }
    #define ADBG     1 ? (void)0 : ARM_Debug
    #define ARM_DebugInit(type, filename)
    #define ARM_DebugComplete()
    #define ARM_DebugTimerStart()
    #define ARM_DebugTimerRead()
    #define ARM_DebugTimerOutput()
#endif

#ifdef __cplusplus
    }
#endif
