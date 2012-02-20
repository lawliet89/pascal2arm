/* tracer.c - module to trace memory accesses and instruction execution
 *            in ARMulator.
 * Copyright (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.31.2.5 $
 * Checkin $Date: 1997/12/10 19:08:54 $
 * Revising $Author: irickard $
 */

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "armdefs.h"
#include "armcnf.h"
#include "tracer.h"
#include "disass.h"

/* NB: this definition is somewhat dubious, it only applies to
 * Thumb-compatible processors.  Really should check first that
 * target processor is Thumb-compatible.
 */
#define CPSR_THUMBSTATE(r)  ((r & 0x20) != 0)

static char *cb_proc(dis_cb_type t,int32 offset,unsigned32 addr,
                     int width,void *arg,char *buf)
{
  IGNORE(t); IGNORE(offset); IGNORE(addr); IGNORE(width); IGNORE(arg);
  return buf;
}

#ifdef SOCKETS
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#define TRACE_INSTR  0x0001
#define TRACE_MEM    0x0002
#define TRACE_IDLE   0x0004
#define TRACE_RANGE  0x0008
#define TRACE_PIPE   0x0010     /* use pclose to close */
#define TRACE_TEXT   0x0020     /* stream is plain text */
#define TRACE_EVENT  0x0040
#define TRACE_DISASS 0x0080     /* disassemble instruction stream */
#define TRACE_FIXON  0x8000     /* tracing always on */

static struct {
  tag_t option;
  unsigned long flag;
  char *print;
} TraceOption[] = {
  ARMulCnf_TraceInstructions, TRACE_INSTR,    "Instructions",
  ARMulCnf_TraceMemory,       TRACE_MEM,      "Memory accesses",
  ARMulCnf_TraceIdle,         TRACE_IDLE,     "Idle cycles",
  ARMulCnf_TraceEvents,       TRACE_EVENT,    "Events",
  ARMulCnf_Disassemble,       TRACE_DISASS,   "Disassemble",
  NULL, 0, NULL
};

#define ModelName (tag_t)"Tracer"

/*
 * The trace function generates a stream of packets, which are either
 * logged into a file or sent to a socket.
 */

#ifndef EXTERNAL_DISPATCH       /* if you want something better... */

static int Tracer_Acc(FILE *f, ARMul_acc acc)
{
  char acc_desc;
  int width=8;
  fputc('M',f);
  switch(acc_CYCLE(acc)) {
  case acc_typeI: acc_desc='I'; break;
  case acc_typeC: acc_desc='C'; break;
  case acc_typeN: acc_desc='N'; break;
  case acc_typeS: acc_desc='S'; break;
  }
  fputc(acc_desc,f);
  fputc(acc_READ(acc) ? 'R' : 'W',f);
  switch (acc_WIDTH(acc)) {
  case BITS_8:  fputc('1',f); width=2; break;
  case BITS_16: fputc('2',f); width=4; break;
  case BITS_32: fputc('4',f); break;
  case BITS_64: fputc('8',f); break;
  default: fputc('_',f); break;
  }
  fputc(acc_OPC(acc) ? 'O' : '_',f);
  fputc(acc_LOCK(acc) ? 'L' : '_',f);
  fputc(acc_SPEC(acc) ? 'S' : '_',f);
  return width;
}

static void Tracer_Dispatch(Trace_State *ts,Trace_Packet *packet)
{
  FILE *f=ts->file;
  if (ts->prop & TRACE_TEXT) {
    switch (packet->type) {
    case Trace_Instr: {
      char buffer[256];
      ARMword instr=packet->u.instr.instr;

      ARMword pc=packet->u.instr.pc;
      unsigned executed=packet->u.instr.executed;
      int thumb=CPSR_THUMBSTATE(packet->u.instr.cpsr);

      if (ts->prop & TRACE_DISASS) {
	if (thumb) {
	  /* Complications for Thumb 2-instruction branches.
	   * Unfortunately the ARM disassembler interface (current instr,
	   * next instr) is not ideal for disassembling instructions as
	   * they are executed, where you want to specify current instr &
	   * prev instr.
	   */
	  if (instr>>11==0x1E) {  /* first instruction of branch pair */
	    ts->prev_instr=instr;
	    strcpy(buffer, "(BL pair)");
	  } else if (instr>>11==0x1F)  /* second instruction */
	    disass_16(ts->prev_instr,instr,pc-2,buffer,NULL,cb_proc);
	  else
	    disass_16(instr,0,pc,buffer,NULL,cb_proc);
	} else {
	  disass(instr,pc,buffer,NULL,cb_proc);
	}
        if (!executed) {
          char *p;
          for (p=buffer; *p; p++)
            if (isupper(*p)) *p=tolower(*p);
        }
      }
      else
	*buffer=0; /* empty string */

        fprintf(f,"I%c %08lX %0*lX %s\n",
                executed ? 'T' : 'S', pc, thumb ? 4 : 8, instr, buffer);

    }
      break;

    case Trace_MemAccess: {
      ARMul_acc acc=packet->u.mem_access.acc;
      unsigned32 addr=packet->u.mem_access.addr;
      int width = Tracer_Acc(f,acc);
      fprintf(f," %08lX",addr);
      if (acc_MREQ(acc)) {
	if (packet->u.mem_access.rv == 0) {
	  fputs(" (wait)", f);
	} else if (packet->u.mem_access.rv < 0) {
	  fputs(" (abort)", f);
	} else {
	  fprintf(f," %0*lX",width,packet->u.mem_access.word1);
	  if (acc_WIDTH(acc)==BITS_64)
	    fprintf(f," %08lX",packet->u.mem_access.word2);
	}
      }
#ifdef DEBUG_PIPELINED
      if (acc_ACCOUNT(acc)) {
	/* display pipeline prediction errors */
	if (packet->u.mem_access.predict.calls != -1) { /* not start-up */
	  if (packet->u.mem_access.predict.calls != 1)
	    fprintf(f," pipe calls %d",
		    packet->u.mem_access.predict.calls);
	  else if (acc != packet->u.mem_access.predict.acc ||
		   addr != packet->u.mem_access.predict.addr) {
	    fprintf(f," pipe err ");
	    Tracer_Acc(f,packet->u.mem_access.predict.acc);
	    fprintf(f," %08lX",packet->u.mem_access.predict.addr);
	  }
	}
      } else {
	/* display non-accounted accesses */
	if (acc_MREQ(acc))
	  fprintf(f, acc_READ(acc) ? " (peek)" : " (poke)");
      }
#endif
      fputc('\n', f);
    }
      break;

    case Trace_Event:
#ifdef PicAlpha
      if (packet->u.event.type >= PiccoloEvent_Instr &&
          packet->u.event.type <= PiccoloEvent_Stalled) {
        ARMword pc = packet->u.event.addr;
        ARMword instr = packet->u.event.addr2;
        char buffer[256];
        disass_pic(instr, pc, buffer);
        switch (packet->u.event.type) {
        case PiccoloEvent_Instr:
          fprintf(f, "PI %08lX %08lX %s\n", pc, instr, buffer);
          break;

        case PiccoloEvent_Skip:
          fprintf(f, "PM %08lX %08lX %s (multi-cycle)\n", pc, instr, buffer);
          break;

        case PiccoloEvent_Stalled:
          fprintf(f, "PS %08lX %08lX %s (stalled)\n", pc, instr, buffer);
          break;
        }

      } else if (packet->u.event.type == PiccoloEvent_ROBLoad) {
        ARMword reg = packet->u.event.addr2 & 0xf;
        fprintf(f, "PR %08lX -> %c%c\n", packet->u.event.addr,
                "AXYZ"[reg/4], (char)('0'+(reg%4)));

      } else if (packet->u.event.type == PiccoloEvent_FIFOOut) {
        fprintf(f, "PF ARM <- %08lx [%d]\n", packet->u.event.addr,
                (unsigned)packet->u.event.addr2);
      } else
#endif
      fprintf(f,"E %08lX %08lX %x\n",
              packet->u.event.addr,packet->u.event.addr2,
              packet->u.event.type);
      break;
    }
  } else {
    fwrite(packet,sizeof(*packet),1,f);
  }
}
    
/*
 * Open the trace stream.
 * Under Unix, possibly try doing a socket instead of a plain
 * file. Returns 1 on failure.
 */

static unsigned Tracer_Open(Trace_State *ts)
{
  const char *option;
  time_t now=time(NULL);
  unsigned verbose;

  verbose=ToolConf_DLookupBool(ts->config, ARMulCnf_Verbose, FALSE);

  option=ToolConf_Lookup(ts->config, ARMulCnf_File);
  if (option!=NULL) {
    ts->file=fopen(option,"w");
    if (ts->file==NULL) {
      fprintf(stderr,"Could not open trace file '%s' - abandoning trace.\n",
              option);
      return 1;
    }
    
    fprintf(ts->file, "Date: %s", ctime(&now));
    fprintf(ts->file, "Source: Armul\n");
    fputs("Options: ", ts->file);
    if (ts->prop & TRACE_INSTR)  fputs("Trace Instructions  ", ts->file);
    if (ts->prop & TRACE_DISASS) fputs("(Disassemble)  ", ts->file);
    if (ts->prop & TRACE_MEM)    fputs("Trace Memory Cycles  ", ts->file);
    if (ts->prop & TRACE_IDLE)   fputs("Trace Idle Cycles  ", ts->file);
    if (ts->prop & TRACE_EVENT)  fputs("Trace Events  ", ts->file);
    fputc('\n', ts->file);  
    if (ts->prop & TRACE_RANGE)
      fprintf(ts->file, "Range: 0x%08lX -> 0x%08lX\n",
              ts->range_lo, ts->range_hi);
    if (ts->sample_base)
      fprintf(ts->file, "Sample: %ld\n", ts->sample_base);
    fputc('\n', ts->file);  

    ts->prop|=TRACE_TEXT;

    return 0;
  }

  option=ToolConf_Lookup(ts->config, ARMulCnf_BinFile);
  if (option!=NULL) {
    ts->file=fopen(option,"wb");
    if (ts->file==NULL) {
      fprintf(stderr,"Could not open trace file '%s' - abandoning trace.\n",
              option);
      return 1;
    }
    
    return 0;
  }

#ifdef SOCKETS
  option=ToolConf_Lookup(ts->config, ARMulCnf_Port);
  if (option) {
    long port;
      
    port=strtol(option,NULL,0);

    option=ToolConf_Lookup(ts->config, ARMulCnf_Host);

    if (option) {
      struct hostent *host;
      struct sockaddr_in con;
      int sock;
      
      host=gethostbyname(option);

      if (host==NULL) {
        ARMul_ConsolePrint(ts->state,"Could not resolve host '%s'\n",
                           option);
        return 1;
      }
        
      if (verbose)
        ARMul_ConsolePrint(ts->state,"Tracing to %s:%d\n",option,port);

      sock=socket(AF_INET,SOCK_STREAM,0);
      if (sock==-1) {
        ARMul_ConsolePrint(ts->state,"Could not open trace port\n");
        return 1;
      }
        
      memset(&con,'\0',sizeof(con));
      con.sin_family=AF_INET;
      memcpy(&con.sin_addr,host->h_addr,sizeof(con.sin_addr));
      con.sin_port=htons(port & 0xffff);
        
      if (connect(sock,(struct sockaddr *)&con, sizeof(con))!=0) {
        close(sock);
        ARMul_ConsolePrint(ts->state,"Failed to open socket\n");
        return 1;
      }

      ts->file=fdopen(sock,"wb");
      if (ts->file!=NULL) return 0;

      ARMul_ConsolePrint(ts->state,"Failed to fdopen socket.\n");
      return 1;
    }

    ARMul_ConsolePrint(ts->state,"PORT configured with no HOST.\n");
    return 1;
  }
#endif

#ifdef PIPE
  option=ToolConf_Lookup(ts->config, ARMulCnf_Pipe);
  if (option) {
    ts->prop|=TRACE_PIPE;
    ts->file=popen(option,"w");
    if (ts->file!=NULL) return 0;
    ARMul_ConsolePrint(state,
                       "Could not open pipe to '%s' - abandoning trace.\n",
                       option);
    return 1;
  }
#endif
  
  fprintf(stderr,"No trace file configured - abandoning trace.\n");
  return 1;
}

static void Tracer_Close(Trace_State *ts)
{
#ifdef PIPE
  if (ts->prop & TRACE_PIPE) pclose(ts->file);
  else
#endif
    fclose(ts->file);
}

static void Tracer_Flush(Trace_State *ts)
{
  fflush(ts->file);
}

#else

extern unsigned Tracer_Open(Trace_State *ts);
extern void Tracer_Dispatch(Trace_State *ts,Trace_Packet *packet);
extern void Tracer_Close(Trace_State *ts);
extern void Tracer_Flush(Trace_State *ts);

#endif                          /* EXTERNAL_DISPATCH */

/*
 * The function is called from the ARMulator when rdi_log & 16 is true.
 * It is used to generate an executed instruction stream trace.
 */
static void trace_event(void *handle, unsigned int event,
                        ARMword addr1, ARMword addr2)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;

  /* Mask events */
  if ((event & ts->event_mask)!=ts->event_set) return;

  if ((ts->prop & TRACE_RANGE) && (addr1<ts->range_lo ||
                                   (addr1>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return;
    ts->sample=ts->sample_base-1;
  }

  packet.type=Trace_Event;

  packet.u.event.addr=addr1;
  packet.u.event.addr2=addr2;
  packet.u.event.type=event;

  Tracer_Dispatch(ts,&packet);
}

/*
 * this function is called for every instruction
 */

static void trace(void *handle, ARMword pc, ARMword instr)
{
  Trace_State *ts=(Trace_State *)handle;
  int temp;
  Trace_Packet packet;

  if ((ts->prop & TRACE_RANGE) && (pc<ts->range_lo ||
                                   (pc>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return;
    ts->sample=ts->sample_base-1;
  }

  temp=TOPBITS(28);
  packet.type=Trace_Instr;

  packet.u.instr.pc=pc;
  packet.u.instr.instr=instr;
  packet.u.instr.cpsr=ARMul_GetCPSR(ts->state);
  /* Nasty code below is because there is no Thumb-aware ARMul_CondCheckInstr
   * Instead we have to check if in Thumb state, then special-case
   * Thumb conditional branch (the only conditional instruction).
   */
  if (CPSR_THUMBSTATE(packet.u.instr.cpsr)) {
    if ((instr>>12)==0xD)  /* cond branch */
      /* make it look like an ARM instruction by shift up cond bits */
      packet.u.instr.executed=ARMul_CondCheckInstr(ts->state,instr<<20);
    else
      packet.u.instr.executed=1;
  } else
    packet.u.instr.executed=ARMul_CondCheckInstr(ts->state,instr);

  Tracer_Dispatch(ts,&packet);
}


static void TracerExit(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;

  if (ts->trace_opened) {
    Tracer_Close(ts);
    ts->trace_opened=FALSE;
  }

  free(ts);
}

/*
 * These are the veneer memory functions that intercept core memory
 * activity and report it to the trace stream.
 */
static int MemAccess(void *handle,ARMword addr,ARMword *word,
                     ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;
  int rv;

  rv=ts->real.x.basic.access(ts->real.handle,addr,word,acc); 

  if (ts->not_tracing) return rv;

#ifndef DEBUG_PIPELINED
  /* TBD: want option to trace RDI accesses */
  if (!acc_ACCOUNT(acc)) return rv;
#endif

  if ((ts->prop & TRACE_RANGE) && (addr<ts->range_lo ||
                                   (addr>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return rv;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return rv;
    ts->sample=ts->sample_base-1;
  }


  if (ts->prop & acc_MREQ(acc) ? TRACE_MEM : TRACE_IDLE) {
    packet.type=Trace_MemAccess;
    packet.u.mem_access.acc=acc;
    packet.u.mem_access.addr=addr;
#ifdef DEBUG_PIPELINED
    packet.u.mem_access.predict=ts->current;
#endif
    if (acc_MREQ(acc))
      packet.u.mem_access.word1=*word;  /* word valid iff memory request */
    packet.u.mem_access.rv=rv;
    Tracer_Dispatch(ts,&packet);
  }

#ifdef DEBUG_PIPELINED
  /* The idea here is to check the predicted NextCycle info.
   * First, the NextCycle data is temporarily kept in ts->advance
   * (we also count the number of NextCycle calls).
   * Then the prediction information is delayed by one cycle
   * so that it can be compared with the actual cycle that happens.
   * NextCycle info must be supplied for I-cycles and memory accesses.
   */
  if (acc_ACCOUNT(acc) && (rv != 0)) {
    ts->current=ts->advance;  /* delay next cycle information by 1 cycle */
    ts->advance.calls=0;  /* reset counter */
  }
#endif
  
  return rv;
}

#ifdef DEBUG_PIPELINED
static void NextCycle(void *handle,ARMword addr,ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;

  ts->real.x.basic.next(ts->real.handle,addr,acc); 

/* NextCycle info before non-accounted accesses should
 *   - ideally not happen
 *   - be marked as non-accounted if it does happen
 * This enables the NextCycle info from the previous real
 * access to relate to the next real access regardless of
 * intervening non-accounted accesses.
 */
  if (acc_ACCOUNT(acc)) {
    /* store next cycle information */
    ts->advance.addr=addr;
    ts->advance.acc=acc;
    ts->advance.calls++;  /* count the number of NextCycle calls */
  }
}
#endif

static int MemAccess2(void *handle,ARMword addr,
                      ARMword *word1,ARMword *word2,
                      ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;
  int rv;

  rv=ts->real.x.arm8.access2(ts->real.handle,addr,word1,word2,acc);

  if (ts->not_tracing) return rv;

  if (!acc_ACCOUNT(acc)) return rv;

  if ((ts->prop & TRACE_RANGE) && (addr<ts->range_lo || /* range test */
                                   (addr>=ts->range_hi && ts->range_hi!=0)))
    return rv;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return rv;
    ts->sample=ts->sample_base-1;
  }

  if (ts->prop & acc_MREQ(acc) ? TRACE_MEM : TRACE_IDLE) {
    packet.type=Trace_MemAccess;
    packet.u.mem_access.acc=acc;
    packet.u.mem_access.addr=addr;
    packet.u.mem_access.word1=*word1;
    packet.u.mem_access.word2=*word2;
    packet.u.mem_access.rv=rv;
    Tracer_Dispatch(ts,&packet);
  }    

  return rv;
}

/* Dummy veneer functions */
static unsigned int DataCacheBusy(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.x.strongarm.data_cache_busy(ts->real.handle);
}
static void CoreException(void *handle,ARMword address,ARMword penc)
{
  Trace_State *ts=(Trace_State *)handle;
  ts->real.x.arm8.core_exception(ts->real.handle,address,penc);
}
static unsigned long GetCycleLength(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.x.basic.get_cycle_length(ts->real.handle);
}
static unsigned long ReadClock(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.read_clock(ts->real.handle);
}
static const ARMul_Cycles *ReadCycles(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.read_cycles(ts->real.handle);
}

/* Function called when RDI_LOG changes, so we can control logging
 * memory accesses */

static void trace_on(Trace_State *ts)
{
  unsigned32 prop=ts->prop;
  ARMul_State *state=ts->state;
  
  ts->not_tracing=FALSE;
  
  if (!ts->trace_opened) {
    /* open the tracing file */
    if (Tracer_Open(ts)) return;
    ts->trace_opened=TRUE;
  }

#ifdef DEBUG_PIPELINED
  ts->current.calls=-1; /* rogue value    */
  ts->advance.calls=0;  /* 0 calls so far */
#endif
  
  /* install instruction and event tracing functions */
  if (prop & TRACE_INSTR)
    ts->hourglass=ARMul_InstallHourglass(state,trace,ts);
  if (prop & TRACE_EVENT)
    ts->trace_event=ARMul_InstallEventUpcall(state,trace_event,ts);
}

static void trace_off(Trace_State *ts)
{
  unsigned32 prop=ts->prop;
  ARMul_State *state=ts->state;
  
  ts->not_tracing=TRUE;
  /* remove instruction and event tracing functions */
  if (prop & TRACE_INSTR) ARMul_RemoveHourglass(state,ts->hourglass);
  if (prop & TRACE_EVENT) ARMul_RemoveEventUpcall(state,ts->trace_event);
  ts->hourglass=ts->trace_event=NULL;
  Tracer_Flush(ts);
}

static int RDI_info(void *handle,unsigned type,ARMword *arg1,ARMword *arg2)
{
  IGNORE(arg2);
  if (type==RDIInfo_SetLog) {
    Trace_State *ts=(Trace_State *)handle;
    int new=(int)*arg1;

    if (new & RDILog_Trace) {
      if (ts->not_tracing)      /* tracing enable */
        trace_on(ts);
    } else {
      if (!ts->not_tracing)     /* tracing disable */
        trace_off(ts);
    }
  }

  return RDIError_UnimplementedMessage;
}

static ARMul_Error CommonInit(ARMul_State *state,Trace_State *ts,
                              toolconf config)
{
  unsigned int i;
  const char *option;
  int verbose;

  ARMul_PrettyPrint(state,", Tracer");

  verbose=ToolConf_DLookupBool(config, ARMulCnf_Verbose, FALSE);

  ARMul_InstallExitHandler(state,TracerExit,ts);

  ts->state=state;
  ts->not_tracing=TRUE;

  ts->config=config;
  if (ts->config==NULL) {
    free(ts);
    return ARMul_RaiseError(state,ARMulErr_NoConfigFor,ModelName);
  }

  ts->file=NULL;
  ts->prop=0;
  ts->trace_opened=FALSE;
  ts->prev_instr=0;  /* previous instr of BL pair, shouldn't be used */

  for (i=0;TraceOption[i].option!=NULL;i++) {
    const char *option=ToolConf_Lookup(ts->config,TraceOption[i].option);
    if (option) {
      if (verbose)
        ARMul_ConsolePrint(state,"%s%s",
                           ts->prop ? ", " : "Tracing:                 ",
                           TraceOption[i].print);      
      ts->prop=ToolConf_AddFlag(option,ts->prop,TraceOption[i].flag,TRUE);
    }
  }

  if (ts->prop & TRACE_EVENT) {
    const char *option=ToolConf_Lookup(ts->config, ARMulCnf_EventMask);
    if (option) {
      char *p;
      ts->event_mask = strtoul(option, &p, 0);
      ts->event_set = p ? strtoul(p+1, NULL, 0) : ts->event_mask;
      if (verbose)
        ARMul_ConsolePrint(state," Mask 0x%08x-0x%08x",
                           ts->event_mask,ts->event_set);
    } else {
      option=ToolConf_Lookup(config,ARMulCnf_Event);
      if (option) {
        ts->event_mask=(unsigned int)~0;
        ts->event_set=strtoul(option,NULL,0);
        if (verbose)
          ARMul_ConsolePrint(state," 0x%08x",ts->event_set);
      } else
        ts->event_mask = ts->event_set = 0;
    }
  }

  if (ts->prop & TRACE_FIXON) {
    trace_on(ts);
  } else {
    /* Install rdi_log handler */
    ARMul_InstallUnkRDIInfoHandler(state,RDI_info,ts);
  }

  option=ToolConf_Lookup(ts->config,ARMulCnf_Range);
  if (option) {
    if (sscanf(option,"%li,%li",&ts->range_lo,&ts->range_hi)==2) {
      ts->prop|=TRACE_RANGE;
      if (verbose) ARMul_ConsolePrint(state," Range %08x->%08x",
                                      ts->range_lo,ts->range_hi);
    } else
      ARMul_ConsolePrint(state,"TRACER: Did not understand range '%s'\n",
                         option);
  }

  option=ToolConf_Lookup(ts->config,ARMulCnf_Sample);
  if (option) {
    ts->sample_base=strtoul(option,NULL,0);
    ts->sample=0;
    if (verbose) ARMul_ConsolePrint(state," Sample rate %d",ts->sample_base);
  } else {
    ts->sample_base=0;
  }

  if (ts->prop!=0 && verbose) ARMul_ConsolePrint(state,"\n");

  return ARMulErr_NoError;
}

static ARMul_Error MemoryInit(ARMul_State *state,ARMul_MemInterface *interf,
                              ARMul_MemType type,toolconf config)
{
  /*
   * Tracer installed as a memory model - can trace memory accesses
   * if so configured.
   */
  Trace_State *ts;
  armul_MemInit *stub=NULL;
  toolconf child;
  ARMul_Error err;
  tag_t tag;
  
  tag = (tag_t)ToolConf_Lookup(config,ARMulCnf_Memory);

  if (tag)
    stub=ARMul_FindMemoryInterface(state,tag,&child);
  if (tag==NULL || stub==NULL || stub==MemoryInit)
    return ARMul_RaiseError(state,ARMulErr_NoMemoryChild,ModelName);

  ts=(Trace_State *)malloc(sizeof(Trace_State));
  if (ts==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  err=CommonInit(state,ts,config);
  if (err!=ARMulErr_NoError) return err;

  if (child==NULL) child=ts->config;
  else {                        /* pass on clock speed */
    const char *option = ToolConf_FlatLookup(config, ARMulCnf_MCLK);
    if (option) ToolConf_UpdateTagged(child, ARMulCnf_MCLK, option);
  }

  if (ts->prop & (TRACE_MEM | TRACE_IDLE)) {
    err=stub(state,&ts->real,type,child);
    if (err!=ARMulErr_NoError) {
      free(ts);
      return err;
    }
    interf->handle=(void *)ts;
    interf->read_clock=ReadClock;
    interf->read_cycles=ReadCycles;
    switch (type) {
    case ARMul_MemType_Basic: case ARMul_MemType_BasicCached:
    case ARMul_MemType_16Bit: case ARMul_MemType_16BitCached:
    case ARMul_MemType_Thumb: case ARMul_MemType_ThumbCached:
      interf->x.basic.access=MemAccess;
      interf->x.basic.get_cycle_length=GetCycleLength;
#ifdef DEBUG_PIPELINED
      interf->x.basic.next=NextCycle;
#endif
      break;
    case ARMul_MemType_ARM8:
      interf->x.arm8.access=MemAccess;
      interf->x.arm8.get_cycle_length=GetCycleLength;
      interf->x.arm8.access2=MemAccess2;
      interf->x.arm8.core_exception=CoreException;
      break;
    case ARMul_MemType_StrongARM:
      interf->x.strongarm.access=MemAccess;
      interf->x.strongarm.get_cycle_length=GetCycleLength;
      interf->x.strongarm.core_exception=CoreException;
      interf->x.strongarm.data_cache_busy=DataCacheBusy;
      break;
    default:
      *interf=ts->real;         /* copy real memory interface across */
      ARMul_ConsolePrint(state,"\
TRACER: Cannot trace this type of memory interface.\n");
      break;
    }
    return ARMulErr_NoError;
  } else {
    return stub(state,interf,type,child);
  }
}

const ARMul_MemStub ARMul_TracerMem = {
  MemoryInit,
  ModelName
  };

static ARMul_Error ModelInit(ARMul_State *state,toolconf config)
{
  /*
   * Tracer installed as a basic model - can only trace instructions and
   * events.
   */
  Trace_State *ts;
  ARMul_Error err;

  ts=(Trace_State *)malloc(sizeof(Trace_State));
  if (ts==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  err=CommonInit(state,ts,config);
  if (err!=ARMulErr_NoError) return err;

  if (ts->prop & (TRACE_MEM | TRACE_IDLE)) {
    ARMul_ConsolePrint(state,"\
Tracer needs to be installed as a memory model to trace memory accesses.\n");
  }

  return ARMulErr_NoError;
}

const ARMul_ModelStub ARMul_TracerModel = {
  ModelInit,
  ModelName
  };

