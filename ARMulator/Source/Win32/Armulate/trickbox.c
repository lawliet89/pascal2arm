/* Model of the validation suite Trick-Box memory */
/* Copyright (c) 1996 Advanced RISC Machines Limited - All Rights Reserved
 *
 * RCS $Revision: 1.11 $
 * Checkin $Date: 1997/05/06 09:15:35 $
 * Revising $Author: clavende $
 */

#include <ctype.h>
#include "armdefs.h"
#include "armcnf.h"

#define PAGESIZE 17

#define ModelName (tag_t)"TrickBox"

typedef struct {
  char b[1<<PAGESIZE];
} page;

typedef struct {
  int ntrans;
  int counter;                  /* 0 indicates off */
  ARMul_State *state;
  int fiq_cnt;
  int irq_cnt;
  int wait_cnt;
  int bigendSig;
  struct {
    struct {
      ARMword start,end;
      int enable;
    } read,write;
  } abort;
  ARMul_Cycles cycles;          /* has to count these for trickbox coproc to work */
  int debug_flag;
  int harvard;                  /* true if strongarm */
  page *pages[1<<(32-PAGESIZE)];
} trickbox;

static void TransChange(void *handle,unsigned old,unsigned new)
{
  IGNORE(old);
  ((trickbox *)handle)->ntrans=new;
}

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  IGNORE(old);
  ((trickbox *)handle)->bigendSig=((new & MMU_B) != 0);
}

static char *mode_names[]={
  "User26", "FIQ26",  "IRQ26",  "SVC26",
  "Dummy04","Dummy05","Dummy06","Dummy07",
  "Dummy08","Dummy09","Dummy0a","Dummy0b",
  "Dummy0c","Dummy0d","Dummy0e","Dummy0f",
  "User32", "FIQ32",  "IRQ32",  "SVC32",
  "Dummy14","Dummy15","Dummy16","Abort32",
  "Dummy18","Dummy19","Dummy1a","Undef32",
  "Dummy1c","Dummy1d","Dummy1e","System"
  };

static void ModeChange(void *handle, ARMword old, ARMword new)
{
  IGNORE(old);
  ARMul_ConsolePrint(((trickbox *)handle)->state,
                     "** Mode change to %s **\n",
                     (new<sizeof(mode_names)/sizeof(mode_names[0])) ?
                     mode_names[new] : "<too high>");
}

static void Interrupt(void *handle, unsigned int which)
{
  ARMul_ConsolePrint(((trickbox *)handle)->state,
                     "** %s %s %s **\n",
                     (which & ARMul_InterruptUpcallFIQ) ? "" : "FIQ",
                     (which & ARMul_InterruptUpcallIRQ) ? "" : "IRQ",
                     (which & ARMul_InterruptUpcallReset) ? "" : "RESET");
}
  

static int MemAccess(void *handle,ARMword address,ARMword *word,ARMul_acc acc)
{
  trickbox *tb=(trickbox *)handle;
  ARMul_State *state=tb->state;

  if (acc_ACCOUNT(acc)) {
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) tb->cycles.NumScycles++;
      else tb->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) tb->cycles.NumNcycles++;
    else tb->cycles.NumIcycles++;

    if (!tb->harvard || acc_OPC(acc)) {
      if (tb->counter) tb->counter++;
      
      if (tb->fiq_cnt>0 && --tb->fiq_cnt==0) {
        ARMul_SetNfiq(state,0);
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** send FIQ **\n");
      }
      
      if (tb->irq_cnt>0 && --tb->irq_cnt==0) {
        ARMul_SetNirq(state,0);
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** send IRQ **\n");
      }

      if (tb->wait_cnt>0 && --tb->wait_cnt>0)
        return 0;
    } else {
      if (tb->wait_cnt>0)
        return 0;
    }
  }

  if (acc_MREQ(acc)) {
    page *p=tb->pages[address>>PAGESIZE];
    ARMword *ptr;
    ARMword w;
    
    if (p==NULL) {
      p=tb->pages[address>>PAGESIZE]=(page *)malloc(sizeof(page));
    }
    ptr=(ARMword *)(&p->b[address & (((1<<PAGESIZE)-1) & ~3)]);
    
    if (acc_READ(acc)) {
      if (address>=tb->abort.read.start &&
          address<tb->abort.read.end &&
          tb->abort.read.enable) {
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** read abort region **\n",address);
        return -1;
      }
      switch (address) {
#if 0
        switch (address>>20) {
        case 0x007:
          /* speculative fetch failure area */
          if (acc_SPEC(acc)) return 0;
          break;
        case 0x008: case 0x009: case 0x00a: case 0x00b: case 0x00c:
        case 0x00d: case 0x00e: case 0x00f: case 0x010: case 0x011:
        case 0x012: case 0x013: case 0x014:
          /* always abort */
          return -1;
        case 0x015: case 0x016: case 0x017: case 0x018: case 0x019:
          /* abort in user mode */
          if (!tb->ntrans)
            return -1;
          break;
        case 0x030:                 /* trickbox */
        }
#endif
        
      case 0x03000038:  /* write abort start */
        w=tb->abort.write.start;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** read abort write start %08x **\n",w);
        break;
      case 0x0300003c:          /* write abort end */
        w=tb->abort.write.end;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** read abort write end %08x **\n",w);
        break;
      case 0x03000040:
        w=tb->abort.read.start;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** read abort read start %08x **\n",w);
        break;
      case 0x03000044:
        w=tb->abort.read.end;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** read abort read end %08x **\n",w);
        break;    
      case 0x03000080:              /* counter */
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** read/stop counter %d **\n",
                             tb->counter);
        w=tb->counter;
        tb->counter=0;
        break;
      default:
        switch (acc_WIDTH(acc)) {
        case BITS_32:
          w=*ptr;
          break;
        case BITS_16:
          if (HostEndian!=tb->bigendSig) address^=2;
          w=*((unsigned16 *)(((char *)ptr)+(address & 2)));
          break;
        case BITS_8:
          if (HostEndian!=tb->bigendSig) address^=3;
          w=((unsigned8 *)ptr)[address & 3];
          break;
        }
        break;
      }
      switch (acc_WIDTH(acc)) {
      case BITS_32: *word=w; break;
      case BITS_16: *word=w &0xffff; break;
      case BITS_8: *word=w & 0xff; break;
      }
    } else {
      if (address>=tb->abort.write.start &&
          address<tb->abort.write.end &&
          tb->abort.write.enable) {
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** write abort region **\n",address);
        return -1;
      }
      switch (acc_WIDTH(acc)) {
      case BITS_32: w=*word; break;
      case BITS_16: w=*word & 0xffff; break;
      case BITS_8: w=*word & 0xff; break;
      }
      switch (address) {
      case 0x03000000:
        if (isprint(w & 0xff) || isspace(w & 0xff))
          ARMul_ConsolePrint(state,"%c",w & 0xff);
        else
          ARMul_ConsolePrint(state,"\\%03o",w & 0xff);
        break;
      case 0x03000008:
        tb->fiq_cnt=w;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** Schedule FIQ %d **\n",tb->fiq_cnt);
        if (w==0) {
          ARMul_SetNfiq(state,0);
          if (tb->debug_flag)
            ARMul_ConsolePrint(state,"** send FIQ **\n");
        }
        break;
      case 0x0300000c:
        tb->irq_cnt=w;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** Schedule IRQ %d **\n",tb->irq_cnt);
        if (w==0) {
          ARMul_SetNirq(state,0);
          if (tb->debug_flag)
            ARMul_ConsolePrint(state,"** send IRQ **\n");
        }
        break;
      case 0x03000010:
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** Clear FIQ **\n");
        ARMul_SetNfiq(state,1);
        tb->fiq_cnt=0;
        break;
      case 0x03000014:
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** Clear IRQ **\n");
        ARMul_SetNirq(state,1);
        tb->irq_cnt=0;
        break;
      case 0x03000018:
        /* timer - not implemented */
        ARMul_ConsolePrint(state,"** Timer **\n");
        break;
      case 0x0300001c:
        /* FIQ strobe - don't understand */
        ARMul_ConsolePrint(state,"** FIQ strobe **\n");
        break;
      case 0x03000020:
        /* IRQ strobe - don't understand */
        ARMul_ConsolePrint(state,"** IRQ strobe **\n");
        break;
      case 0x03000024:
        /* FIQ strobe - don't understand */
        ARMul_ConsolePrint(state,"** FIQ strobe **\n");
        break;
      case 0x03000028:
        /* IRQ strobe - don't understand */
        ARMul_ConsolePrint(state,"** IRQ strobe **\n");
        break;
      case 0x03000030:
        /* Stroke nWAIT for n cycles */
        tb->wait_cnt=w;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** strobe nWAIT %d cycles **\n",w);
        return 0;
      case 0x03000034:
        tb->wait_cnt=0;
        break;
      case 0x03000038:
        tb->abort.write.start=w & ~3;
        tb->abort.write.enable=w & 1;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,
                             "** Abort Write - Start address set to 0x%08x (%s) **\n",
                             tb->abort.write.start,
                             tb->abort.write.enable ? "enabled" : "disabled");
        break;
      case 0x0300003c:
        tb->abort.write.end=w & ~3;
        tb->abort.write.enable=w & 1;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,
                             "** Abort Write - End address set to 0x%08x (%s) **\n",
                             tb->abort.write.end,
                             tb->abort.write.enable ? "enabled" : "disabled");
        break;
      case 0x03000040:
        tb->abort.read.start=w & ~3;
        tb->abort.read.enable=w & 1;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,
                             "** Abort Read - Start address set to 0x%08x (%s) **\n",
                             tb->abort.read.start,
                             tb->abort.read.enable ? "enabled" : "disabled");
        break;
      case 0x03000044:
        tb->abort.read.end=w & ~3;
        tb->abort.read.enable=w & 1;
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,
                             "** Abort Read - End address set to 0x%08x (%s) **\n",
                             tb->abort.read.end,
                             tb->abort.read.enable ? "enabled" : "disabled");
        break;
      case 0x03000080:
        /* turn on cycle counter */
        if (tb->debug_flag)
          ARMul_ConsolePrint(state,"** turn on cycle counter **\n");
        tb->counter=1;
        break;
      default:
        switch (acc_WIDTH(acc)) {
        case BITS_32:
          *ptr=w;
          break;
        case BITS_16:
          if (HostEndian!=tb->bigendSig) address^=2;
          *((unsigned16 *)(((char *)ptr)+(address & 2)))=(unsigned16)w;
          break;
        case BITS_8:
          if (HostEndian!=tb->bigendSig) address^=3;
          ((unsigned8 *)ptr)[address & 3]=(unsigned8)w;
          break;
        }
      }
    }
  }

  return 1;
}


static int MemAccess2(void *handle,ARMword address,ARMword *w1,ARMword *w2,
                      ARMul_acc acc)
{
  IGNORE(w2);
  if (acc_WIDTH(acc)==BITS_64) acc--;
  return MemAccess(handle,address,w1,acc);
}

static unsigned int DataCacheBusy(void *handle)
{
  IGNORE(handle);
  return FALSE;
}

static const ARMul_Cycles *ReadCycles(void *handle)
{
  trickbox *tb=(trickbox *)handle;
  tb->cycles.Total=(tb->cycles.NumNcycles + tb->cycles.NumScycles +
                    tb->cycles.NumIcycles + tb->cycles.NumCcycles);
  return &tb->cycles;
}

static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config)
{
  trickbox *tb;
  unsigned page;

  interf->read_cycles=ReadCycles;

  switch (type) {
  case ARMul_MemType_Basic: case ARMul_MemType_16Bit: case ARMul_MemType_Thumb:
  case ARMul_MemType_BasicCached: case ARMul_MemType_16BitCached:
  case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccess;
    break;
  case ARMul_MemType_ARM8:
    interf->x.arm8.access=MemAccess;
    interf->x.arm8.access2=MemAccess2;
    break;
  case ARMul_MemType_StrongARM:
    interf->x.strongarm.data_cache_busy=DataCacheBusy;
    interf->x.strongarm.access=MemAccess;
    break;
  default:
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }

  tb=(trickbox *)malloc(sizeof(trickbox));
  if (tb==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  tb->counter=0;
  tb->state=state;
  tb->fiq_cnt=tb->irq_cnt=0;

  tb->abort.write.start=0x00800000;
  tb->abort.write.end=0x01a00000;
  tb->abort.write.enable=TRUE;
  tb->abort.read.start=0x00800000;
  tb->abort.read.end=0x01700000;
  tb->abort.read.enable=TRUE;

  tb->harvard=(type==ARMul_MemType_StrongARM);

  tb->debug_flag = ToolConf_DLookupBool(config, ARMulCnf_Debug, FALSE);

    for (page=0; page<(1<<(32-PAGESIZE)); page++) {
    tb->pages[page]=NULL;
  }
  
  ARMul_InstallExitHandler(state,free,tb);
  if (tb->debug_flag) {
    ARMul_InstallModeChangeHandler(state,ModeChange,tb);
    ARMul_InstallInterruptHandler(state,Interrupt,tb);
  }
  ARMul_InstallTransChangeHandler(state,TransChange,tb);
  ARMul_InstallConfigChangeHandler(state,ConfigChange,tb);

  ARMul_PrettyPrint(state,", Trickbox");

  interf->handle=(void *)tb;

  return ARMulErr_NoError;
}

ARMul_MemStub ARMul_TrickBox = {
  MemInit,
  ModelName
  };
