/* example.c - Example ARMulator memory interface.
 * Copyright (C) Advanced RISC Machines Limited, 1996. All rights reserved.
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1996/11/20 13:52:53 $
 * Revising $Author: mwilliam $
 */

#include <string.h>             /* for memset */
#include "armdefs.h"
#include "dbg_hif.h"            /* so we can use the HostosInterface */

#define PAGESIZE (1<<17)

typedef union {
  char byte[PAGESIZE];
  ARMword word[PAGESIZE/4];
} page;

typedef struct {
  page *p[8];                  /* eight pages of memory */
  int mapped_in;
  int Ntrans;
  ARMul_State *state;          /* So we can generate interrupts */
  const Dbg_HostosInterface *hif; /* So we can print out characters */
  int fiq_cnt,irq_cnt;         /* Counters for interrupts */
} ModelState;

static void TransChangeHandler(void *handle,unsigned old,unsigned new)
{
  ModelState *s=(ModelState *)handle;
  
  s->Ntrans=new;
}

static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config);

#define ModelName "Example"

ARMul_MemStub ExampleMemory = {
  MemInit,
  ModelName
  };


/*
 * Predeclare the memory access functions so that the initialise
 * function can fill them in
 */
static int MemAccess(void *,ARMword,ARMword *,ARMul_acc);
extern void free(void *);   /* ANSI definition of 'free' */

#define OFFSET(addr) ((addr) & 0x7fff)
#define WORDOFF(addr) (OFFSET(addr)>>2)

static ARMul_Error MemInit(ARMul_State *state,
                           ARMul_MemInterface *interf,
                           ARMul_MemType type,
                           toolconf config)
{
  ModelState *s;
  int i;
  
  ARMul_PrettyPrint(state, ", User manual example");

  /* don't support ReadClock and ReadCycles */
  interf->read_clock=NULL;
  interf->read_cycles=NULL;
  
  /* We only support the ARM6/ARM7 memory interfaces so fault
   * everything else.
   */
  
  if (type!=ARMul_MemType_Basic && type!=ARMul_MemType_BasicCached) {
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }
  
  /* Now allocate the state */
  
  s=(ModelState *)malloc(sizeof(ModelState));
  if (s==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);
  
  /* and install the function to free it */
  ARMul_InstallExitHandler(state, free, (void *)s);
  
  for (i=0;i<8;i++) {
    s->p[i]=(page *)malloc(sizeof(page));
    if (s->p[i]==NULL) return FALSE;
    memset(s->p[i], 0, sizeof(page));
  }
  
  s->mapped_in=0;
  s->state=state;    /* keep a handle onto the ARMulator state */
  s->hif=ARMul_HostIf(state);   /* and grab the handle onto the HostosInterface */
  
  ARMul_PrettyPrint(state, ", 1Mb memory");
  
  /* Install the mode change handler
   * On a cached processor there is no 'Ntrans' signal, so we
   * treat it as always being HIGH.
   */
  if (type==ARMul_MemType_BasicCached) {
    s->Ntrans=1;
  } else {
    ARMul_InstallTransChangeHandler(state,
                                    TransChangeHandler,
                                    (void *)s);
  }
  
  /* We have to call "SetMemSize" so that the debug monitor knows where
   * to place the stack.        
   */
  ARMul_SetMemSize(state, 8*PAGESIZE);
  
  interf->x.basic.access=MemAccess;
  interf->handle=(void *)s;
  
  return ARMulErr_NoError;
}

static int MemAccess(void *handle,
                     ARMword address,
                     ARMword *data,
                     ARMul_acc acc)
{
  int highpage=(address & (1<<17));
  ModelState *s=(ModelState *)handle;
  page *mem;
  
  /*
   * Get a pointer to the correct page of memory to read/write
   * from.
   */
  if (highpage)
    mem=s->p[s->mapped_in];
  else
    mem=s->p[0];
  
  /*
   * Next, deal with the interrupt counters.
   */
  if (s->fiq_cnt && --s->fiq_cnt==0)
    ARMul_SetNfiq(s->state, 0);
  if (s->irq_cnt && --s->irq_cnt==0)
    ARMul_SetNirq(s->state, 0);
  
  /* acc_MREQ is true if this is a memory cycle */
  if (acc_MREQ(acc)) {
    /* Now decode the top-bits of the address */
    switch (address>>30) {  /* decode bits 30,31 */
    default:
    case 0:             /* 00 - memory access */
      if (acc_READ(acc)) {
        switch (acc_WIDTH(acc)) {
        case BITS_32:
          *data=mem->word[WORDOFF(address)];
          return 1;
        case BITS_8:
          *data=mem->byte[OFFSET(address)];
          return 1;
        default:  /* do not understand this request */
          return -1;
        }
      } else {        /* write */
        /*
         * Ignore writes out of supervisor modes to the "low"
         * page, and writes in svc modes from STRT instructions
         */
        if (highpage ||                 /* not the lowpage */
            !acc_ACCOUNT(acc) ||        /* or from the debugger */
            s->Ntrans) {
          switch (acc_WIDTH(acc)) {
          case BITS_32:           /* word */
            mem->word[WORDOFF(address)]=*data;
            return 1;
          case BITS_8:            /* byte */
            mem->word[OFFSET(address)]=*data;
            return 1;
          default:              /* should not happen */
            return -1;
          }
        }
      }
      
    case 1:             /* 01 - page select in SVC mode */
      /* Changing the mapped in page is simple enough. */
      if (s->Ntrans || !acc_ACCOUNT(acc)) {
        s->mapped_in=(address>>16) & 7;
      }
      return 1;
      
    case 2:             /* 10 - I/O area */
      /*
       * We do a further decode, but only allow these accesses from
       * "privileged" modes.
       */
      if (s->Ntrans)
        switch ((address>>28) & 0x3) {  /* decode bits 28,29 */
        case 0:         /* read byte */
          if (acc_READ(acc))
            *data=s->hif->readc(s->hif->hostosarg);
        case 1:
          if (acc_WRITE(acc))
            s->hif->writec(s->hif->hostosarg,(*data) & 0xff);
          break;
        case 2:         /* Schedule IRQ */
          s->irq_cnt=address & 0xff;
          break;
        case 3:         /* Schedule FIQ */
          s->fiq_cnt=address & 0xff;
          break;
        }
      return 1;
      
    case 3:             /* 11 - generate an abort */
      /*
       * Unlike previous ARMulators we merely return -1 to generate
       * an abort, and we do not have to worry about what kind of abort
       * it is.
       */
      return -1;
    }
  } else {              /* not a memory request */
    /*
     * MemAccess is called for all ARM cycles, not just memory cycles,
     * and must keep count of these I and C cycles. We return 1, just
     * as we would for a memory cycle, as returning 0 indicates that the
     * memory is stalling the processor.
     */
    return 1;
  }
}
  
