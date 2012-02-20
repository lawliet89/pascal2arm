/****************************************************
 * debug.h - ARM Windows debugging                  *
 *==================================================*
 *                                                  *
 ****************************************************/

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1996/06/25 14:49:03 $
 * Revising $Author: jporter $
 */

#ifdef __cplusplus
    extern "C" {
#endif

#define DllExport __declspec(dllexport)

typedef enum
{
    none    = 0,
    console,
    file
    //network etc.
} debug_output_type;

#ifdef __cplusplus
    }
#endif
