/* multirdi.h - processor lists */

/* The real "processor list" is stored in "multirdi" rather in armrdi. */

/*
 * RCS $Revision: 1.9 $
 * Checkin $Date: 1997/02/18 14:56:13 $
 * Revising $Author: mwilliam $
 */

#ifndef multirdi_h
#define multirdi_h

#include "dbg_rdi.h"
#include "dbg_conf.h"

#include "toolconf.h"

extern char const **ARMul_Processors;

extern RDI_NameList const *ARMul_RDI_cpunames(void);

extern toolconf ARMul_ToolConf,ARMul_ToolConfBase;

int ARMul_RDI_open(unsigned type,
#ifdef PICCOLO
                   const toolconf config,
#else
                   const Dbg_ConfigBlock *config,
#endif
                   const Dbg_HostosInterface *hostif,
                   struct Dbg_MCState *dbg_state);

int ARMul_RDI_close(void);
int ARMul_RDI_read(ARMword source, void *dest, unsigned *nbytes);
int ARMul_RDI_write(const void *source, ARMword dest, unsigned *nbytes);
int ARMul_RDI_CPUread(unsigned mode, unsigned long mask, ARMword *buffer);
int ARMul_RDI_CPUwrite(unsigned mode, unsigned long mask, ARMword const *buffer);
int ARMul_RDI_CPread(unsigned CPnum, unsigned long mask, ARMword *buffer);
int ARMul_RDI_CPwrite(unsigned CPnum, unsigned long mask, ARMword const *buffer);
int ARMul_RDI_setbreak(ARMword address, unsigned type, ARMword bound,
                        PointHandle *handle);
int ARMul_RDI_clearbreak(PointHandle handle);
int ARMul_RDI_setwatch(ARMword address, unsigned type, unsigned datatype,
                        ARMword bound, PointHandle *handle);
int ARMul_RDI_clearwatch(PointHandle handle);
#ifdef PICCOLO
int ARMul_RDI_execute(Dbg_MCState **, PointHandle *handle);
int ARMul_RDI_step(Dbg_MCState **, unsigned ninstr, PointHandle *handle);
#else
int ARMul_RDI_execute(PointHandle *handle);
int ARMul_RDI_step(unsigned ninstr, PointHandle *handle);
#endif
int ARMul_RDI_info(unsigned type, ARMword *arg1, ARMword *arg2);
int ARMul_RDI_errmess(char *buf, int buflen, int errnum);

#endif
