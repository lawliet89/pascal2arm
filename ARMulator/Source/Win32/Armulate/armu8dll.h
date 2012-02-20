/* RCS $Revision: 1.4 $
 * Checkin $Date: 1997/02/26 12:51:23 $
 * Revising $Author: hbullman $
 */

#define BUFSIZE 80
#include <setjmp.h>
#include "armdbg.h"
#include "dbg_rdi.h"

#ifdef __WATCOMC__
#define DLL_EXPORT __export
#elif _MSC_VER
#define DLL_EXPORT _declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define NOT_IMPLEMENTED "__NotImplemented"
/******************************************************************************\
*                               GLOBAL VARIABLES
\******************************************************************************/
HANDLE ghArmulateMod;

/******************************************************************************\
*                              FUNCTION PROTOTYPES
\******************************************************************************/

#ifndef PICCOLO
//RDIProcVec *ARMul_DLL_GetRDI();
RDI_NameList const *ARMul_RDI_cpunames(void);
INT ARMul_RDI_open(unsigned type, const Dbg_ConfigBlock *config,
                    const Dbg_HostosInterface *hostif,
                struct Dbg_MCState *dbg_state);

INT ARMul_RDI_close(void);
INT ARMul_RDI_read(ARMword source, void *dest, unsigned *nbytes);
INT ARMul_RDI_write(const void *source, ARMword dest, unsigned *nbytes);
INT ARMul_RDI_CPUread(unsigned mode, unsigned long mask, ARMword *buffer);
INT ARMul_RDI_CPUwrite(unsigned mode, unsigned long mask, ARMword const *buffer);
INT ARMul_RDI_CPread(unsigned CPnum, unsigned long mask, ARMword *buffer);
INT ARMul_RDI_CPwrite(unsigned CPnum, unsigned long mask, ARMword const *buffer);
INT ARMul_RDI_setbreak(ARMword address, unsigned type, ARMword bound,
                        PointHandle *handle);
INT ARMul_RDI_clearbreak(PointHandle handle);
INT ARMul_RDI_setwatch(ARMword address, unsigned type, unsigned datatype,
                        ARMword bound, PointHandle *handle);
INT ARMul_RDI_clearwatch(PointHandle handle);
INT ARMul_RDI_execute(PointHandle *handle);
INT ARMul_RDI_step(unsigned ninstr, PointHandle *handle);
INT ARMul_RDI_info(unsigned type, ARMword *arg1, ARMword *arg2);
INT ARMul_RDI_errmess(char *buf, int buflen, int errnum);
#endif


#ifdef __cplusplus
}
#endif
