/* tracer.h -- data structure for tracer module
 * Copyright (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.5.6.1 $
 * Checkin $Date: 1997/11/03 19:15:01 $
 * Revising $Author: irickard $
 */

#ifndef tracer_h
#define tracer_h

#include "host.h"
#include "armdefs.h"

typedef struct {
  ARMword addr;
  ARMul_acc acc;
  int calls;                    /* number of calls to NextCycle */
} Trace_Pipeline;

typedef struct {
  ARMul_State *state;
  unsigned int not_tracing;     /* zero when RDILog_Trace is set */
  unsigned int trace_opened;    /* set to one once Tracer_Open called */
  void *config;
  FILE *file;
  unsigned32 prop;
  ARMul_MemInterface real;
  ARMword range_lo,range_hi;
  unsigned long sample,sample_base;
  unsigned int event_mask,event_set; /* masks for events */
  void *hourglass,*trace_event; /* handles for ARMulator callbacks */
#ifdef DEBUG_PIPELINED
  Trace_Pipeline current, advance;
#endif
  unsigned prev_instr;          /* for disassembly of 2-cycle branches */
} Trace_State;

typedef struct {
  enum {
    Trace_Instr,                /* instruction execution */
    Trace_MemAccess,            /* memory cycles and idle cycles */
    Trace_Event                 /* other misc events */
    } type;
  union {
    struct {
      unsigned32 instr;
      unsigned32 pc;
      ARMword cpsr;
      int executed;             /* 1 if executed, 0 otherwise */
    } instr;
    struct {
      ARMul_acc acc;
      unsigned32 addr;
      unsigned32 word1,word2;
      int rv;                   /* return value from mem_access call */
#ifdef DEBUG_PIPELINED
      Trace_Pipeline predict;   /* for checking against actual cycle */
#endif
    } mem_access;
    struct {
      unsigned32 addr,addr2;
      unsigned int type;        /* see note in armdefs.h */
    } event;
  } u;
} Trace_Packet;

#endif

