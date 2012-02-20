/*
 * angel.c
 *
 * This file was armos.c. (and then demon.c)
 *
 * Copyright (C) 1991-1997 Advanced RISC Machines Limited.
 * All rights reserved.
 *
 * RCS $Revision: 1.55.2.2 $
 * Checkin $Date: 1998/03/02 17:31:05 $
 * Revising $Author: ijohnson $
 *
 * This file contains a model of Angel, ARM Ltd's Debug Monitor,
 * including all the SWI's required to support the C library. The code in
 * it is not really for the faint-hearted (especially the abort handling
 * code), but it is a complete example.
 *
 * The NOOS and VALIDATE models are now in separate files.
 */

#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "armdefs.h"

#ifndef NO_ANGEL
/*
 * the following define is for the benefit of ../angel/arm.h
 */
#define ENDIAN_DONT_CARE
#include "arm.h"                /* in angel */
#endif
#include "dbg_hif.h"

#include "armcnf.h"
#ifndef NO_ANGEL
#include "adp.h"
#endif

#ifndef __STDC__
#define remove(s) unlink(s)
#endif

#if defined(__riscos)
  extern int _fisatty(FILE *);
# define isatty_(f) _fisatty(f)
# define EMFILE -1
# define EBADF -1
  int _kernel_escape_seen(void) { return 0 ;}
#elif defined(_WINDOWS) || defined(_CONSOLE)
#  define isatty_(f) (f == stdin || f == stdout)
#elif defined(__ZTC__)
#  include <io.h>
#  define isatty_(f) isatty((f)->_file)
#elif defined(macintosh)
#  ifdef __MWERKS__
#    define EMFILE -1
#    define EBADF -1
#    ifdef USEMPWFCNTL
#      include <fcntl.h>
#      define read HIDE_MW_read
#      define write HIDE_MW_write
#      define unlink HIDE_MW_unlink
#    endif
#    include <unix.h>
#    ifdef USEMPWFCNTL
#      undef read
#      undef write
#      undef unlink
#    endif
#    define isatty_(f) isatty(fileno(f))
#  else
#    include <ioctl.h>
#    define isatty_(f) (~ioctl((f)->_file,FIOINTERACTIVE,NULL))
#  endif
#else
#  define isatty_(f) isatty(fileno(f))
#endif

/*
 * DEMON configuration options
 */

/*
 * SWI numbers
 */

#define SWI_WriteC                 0x0
#define SWI_Write0                 0x2
#define SWI_ReadC                  0x4
#define SWI_CLI                    0x5
#define SWI_GetEnv                 0x10
#define SWI_Exit                   0x11
#define SWI_EnterOS                0x16

#define SWI_GetErrno               0x60
#define SWI_Clock                  0x61
#define SWI_Time                   0x63
#define SWI_Remove                 0x64
#define SWI_Rename                 0x65
#define SWI_Open                   0x66

#define SWI_Close                  0x68
#define SWI_Write                  0x69
#define SWI_Read                   0x6a
#define SWI_Seek                   0x6b
#define SWI_Flen                   0x6c

#define SWI_IsTTY                  0x6e
#define SWI_TmpNam                 0x6f
#define SWI_InstallHandler         0x70
#define SWI_GenerateError          0x71

#ifndef FOPEN_MAX               /* max # of open files. */
#define FOPEN_MAX 64
#endif
#define UNIQUETEMPS 256

/*
 * ANGEL configuration options
 */

#ifdef PIE
# define ANGEL_HEAPBASE 0x50000
# define ANGEL_HEAPLIMIT 0x60000
# define ANGEL_STACKLIMIT 0x78000
# define ANGEL_STACKBASE 0x80000
#else                           /* PID */
# define ANGEL_HEAPBASE  0x02069000 
# define ANGEL_HEAPLIMIT 0x02079000
# define ANGEL_STACKLIMIT 0x02079000
# define ANGEL_STACKBASE 0x02080000
#endif

#define BUFFERSIZE 4096
#define NONHANDLE -1

static void UnwindDataAbort(ARMul_State *state, ARMword addr, ARMword iset);
static void getstring(ARMul_State *state, ARMword from, char *to) ;

#define OLDINSTRUCTIONSIZE (CPSRINSTRUCTIONSIZE(ARMul_GetSPSR(state, ARMul_GetCPSR(state))))

/*
 * OS private Information
 */

typedef struct {
  ARMword ErrorP;               /* as returned by last_error_p */
  ARMword ErrorNo;              /* the "errno" value */
  char *command_line;           /* Command line passed from debugger */
  unsigned vector_catch;        /* VectorCatch passed from debugger */
  FILE *FileTable[FOPEN_MAX];   /* table of open file handles */
  char FileFlags[FOPEN_MAX];    /* ... and the mode of opening */
  char *tempnames[UNIQUETEMPS]; /* array of names of temporary files */
  unsigned long flags;          /* options set (below) */
  ARMul_State *state;           /* back to ARMulator's state */
  const struct Dbg_HostosInterface *hostif;
  toolconf config;              /* config database */
  struct {
    struct {
      ARMword svc,abt,undef;
      ARMword irq,fiq,user;
    } sp;                       /* Stack pointers */
    ARMword soft_vectors;       /* where the soft vectors start */
    ARMword cmd_line;           /* where the command line is placed */
    ARMword handlers;           /* addr/workspace for handlers */
    ARMword soft_vec_code;      /* default handlers */
  } map;
  int angel;                    /* whether we are in fact Angel */
} Demon_State;

typedef struct {
  Demon_State demon;
  ARMword AngelSWIARM;
  ARMword AngelSWIThumb;
  enum {
    angelswi_DEMON, angelswi_ANGEL, angelswi_BOTH
  } handle_swi;                 /* which kind of SWIs we're expecting */
  ARMword heap_base,heap_limit;
  ARMword stack_base,stack_limit;
} Angel_State;

#define NOOP 0
#define BINARY 1
#define READOP 2
#define WRITEOP 4

#ifdef macintosh
#define FIXCRLF(t,c) ((t & BINARY)?c:((c=='\n'||c=='\r')?(c ^ 7):c))
#else
#define FIXCRLF(t,c) c
#endif

static ARMword softvectorcode[] =
{   /* basic: swi tidyexception + event;
     *        mov lr, pc;
     *        ldmia r11,{r11,pc};
     *        swi generateexception + event
     */
  0xef000090, 0xe1a0e00f, 0xe89b8800, 0xef000080, /*Reset*/
  0xef000091, 0xe1a0e00f, 0xe89b8800, 0xef000081, /*Undef*/
  0xef000092, 0xe1a0e00f, 0xe89b8800, 0xef000082, /*SWI  */
  0xef000093, 0xe1a0e00f, 0xe89b8800, 0xef000083, /*Prefetch abort*/
  0xef000094, 0xe1a0e00f, 0xe89b8800, 0xef000084, /*Data abort*/
  0xef000095, 0xe1a0e00f, 0xe89b8800, 0xef000085, /*Address exception*/
  0xef000096, 0xe1a0e00f, 0xe89b8800, 0xef000086, /*IRQ*/
  0xef000097, 0xe1a0e00f, 0xe89b8800, 0xef000087, /*FIQ*/
  0xef000098, 0xe1a0e00f, 0xe89b8800, 0xef000088, /*Error*/
  0xe1a0f00e /* default handler */
};

#define ARMErrorV 32            /* offset, not an address */

/*
 * State defintion
 */

#define AngelFlag_RunningOnThumb  0x00001

/*
 * Memory map configuration
 */

static void Demon_SetOptions(ARMul_State *state,int verbose,
                             Demon_State *demon,toolconf config)
{
  /*
   * Read the address of stack pointers, etc. from the config
   * file.
   */
  const char *option;
  static const struct {
    tag_t name;
    unsigned offset;
    ARMword def;
    const char *desc;
  } options[] = {
    /* supervisor stack space */
    ARMulCnf_AddrSuperStack,  offsetof(Demon_State,map.sp.svc),        0xA00L,
    "Supervisor stack:       ",

    /* abort stack space */
    ARMulCnf_AddrAbortStack,  offsetof(Demon_State,map.sp.abt),        0x800L,
    "Abort mode stack:       ",

    /* undef stack space */
    ARMulCnf_AddrUndefStack,  offsetof(Demon_State,map.sp.undef),      0x700L,
    "Undefined mode stack:   ",

    /* IRQ stack space */
    ARMulCnf_AddrIRQStack,    offsetof(Demon_State,map.sp.irq),        0x500L,
    "IRQ mode stack:         ",

    /* FIQ stack space */
    ARMulCnf_AddrFIQStack,    offsetof(Demon_State,map.sp.fiq),        0x400L,
    "FIQ mode stack:         ",

    /* default user stack start */
    ARMulCnf_AddrUserStack,   offsetof(Demon_State,map.sp.user),       0x80000L,
    "Default user mode stack:",

    /* soft vectors are here */
    ARMulCnf_AddrSoftVectors, offsetof(Demon_State,map.soft_vectors),  0xA40L,
    "Start of soft vectors:  ",

    /* command line is here after a SWI GetEnv */
    ARMulCnf_AddrCmdLine,     offsetof(Demon_State,map.cmd_line),      0xf00L,
    "Command-line space:     ",

    /* address and workspace for installed handlers */
    ARMulCnf_AddrsOfHandlers, offsetof(Demon_State,map.handlers),      0xad0L,
    "Installed handlers:     ",

    /* default handlers */
    ARMulCnf_SoftVectorCode,  offsetof(Demon_State,map.soft_vec_code), 0xb80L,
    "Default handlers:       "
  };
  unsigned int i;

  for (i=0;i<sizeof(options)/sizeof(options[0]);i++) {
    ARMword *w=(ARMword *)(((char *)demon)+options[i].offset);
    option=ToolConf_Lookup(config,options[i].name);
    *w=(option) ? strtoul(option,NULL,0) : options[i].def;
    if (verbose)
      ARMul_ConsolePrint(state,"%s 0x%08x\n",options[i].desc,*w);
  }
}

static void Angel_SetOptions(ARMul_State *state,int verbose,
                             Angel_State *angel,toolconf config)
{
  static const struct {
    tag_t name;
    unsigned offset;
    ARMword def;
  } options[] = {
    ARMulCnf_AngelSWIARM,   offsetof(Angel_State,AngelSWIARM),    angel_SWI_ARM,
    ARMulCnf_AngelSWIThumb, offsetof(Angel_State,AngelSWIThumb), angel_SWI_THUMB,
    ARMulCnf_HeapBase,   offsetof(Angel_State,heap_base),   ANGEL_HEAPBASE,
    ARMulCnf_HeapLimit,  offsetof(Angel_State,heap_limit),  ANGEL_HEAPLIMIT,
    ARMulCnf_StackBase,  offsetof(Angel_State,stack_base),  ANGEL_STACKBASE,
    ARMulCnf_StackLimit, offsetof(Angel_State,stack_limit), ANGEL_STACKLIMIT
  };
  const char *option;
  unsigned int i;

  Demon_SetOptions(state,verbose,&angel->demon,config);

  for (i=0;i<sizeof(options)/sizeof(options[0]);i++) {
    ARMword *w=(ARMword *)(((char *)angel)+options[i].offset);
    option=ToolConf_Lookup(config,options[i].name);
    *w=(option) ? strtoul(option,NULL,0) : options[i].def;
  }

  if (verbose) {
    ARMul_ConsolePrint(state,"ARM Angel SWI:           0x%08x\n",
                       angel->AngelSWIARM);
    ARMul_ConsolePrint(state,"Thumb Angel SWI:         0x%04x\n",
                       angel->AngelSWIThumb);
    ARMul_ConsolePrint(state,"Heap:                    0x%08x->0x%08x\n",
                       angel->heap_base,angel->heap_limit);
    ARMul_ConsolePrint(state,"Stack:                   0x%08x->0x%08x\n",
                       angel->stack_limit,angel->stack_base);
  }
}


/*
 * Operating System functions.
 */

static void Exit(void *handle);
static int RDI_info(void *handle, unsigned type, ARMword *arg1, ARMword *arg2);

/*
 * In future versions, SWI's will be handled on an ExceptionUpcall, and
 * Operating System models will just be plain ARMulator models. This model
 * cam be compiled either way.
 */
#ifdef NEW_OS_INTERFACE
static unsigned HandleSWI_ARMulator(
   void *handle, ARMword vector, ARMword pc, ARMword instr);
static unsigned HandleSWI_Demon(
   void *handle, ARMword vector, ARMword pc, ARMword instr);
#  ifndef NO_ANGEL
static unsigned HandleSWI_Angel(
   void *handle, ARMword vector, ARMword pc, ARMword instr);
static unsigned HandleSWI_Both(
   void *handle, ARMword vector, ARMword pc, ARMword instr);
#  endif
#else
static unsigned HandleSWI_ARMulator(void *handle,ARMword number);
static unsigned HandleSWI_Demon(void *handle,ARMword number);
#  ifndef NO_ANGEL
static unsigned HandleSWI_Angel(void *handle,ARMword number);
static unsigned HandleSWI_Both(void *handle,ARMword number);
#  endif
static unsigned Exception(void *handle, ARMword vector, ARMword pc);
#endif

static ARMul_Error Init(ARMul_State *state,
#ifndef NEW_OS_INTERFACE
                        ARMul_OSInterface *interf,
#endif
                        toolconf config
#ifndef NO_ANGEL
                        , unsigned is_angel
#endif
                        )
{
  ARMword instr, i;
  Demon_State *demon;
  const char *option;
  int verbose;

#ifdef NEW_OS_INTERFACE
#  define InstallSWIHandler(state, interf, fn, handle) \
  ARMul_InstallExceptionHandler(state, fn, handle)
#else
#  define InstallSWIHandler(state, interf, fn, handle) \
    (interf)->handle_swi = (fn)
#endif

#ifndef NO_ANGEL
  Angel_State *angel=NULL;

  if (is_angel) {
    unsigned support_demon;
    angel = (Angel_State *)malloc(sizeof(Angel_State));
    demon = &angel->demon;
    support_demon = ToolConf_DLookupBool(config, ARMulCnf_Demon, FALSE);
    if (support_demon) {
      InstallSWIHandler(state, interf, HandleSWI_Both, angel);
      angel->handle_swi=angelswi_BOTH;
    } else {
      InstallSWIHandler(state, interf, HandleSWI_Angel, angel);
      angel->handle_swi=angelswi_ANGEL;
    }
    demon->angel=TRUE;
  } else {
    demon = (Demon_State *)malloc(sizeof(Demon_State));
    InstallSWIHandler(state, interf, HandleSWI_Demon, angel);
    demon->angel=FALSE;
  }
#else
  demon = (Demon_State *)malloc(sizeof(Demon_State));
  InstallSWIHandler(state, interf, HandleSWI_Demon, angel);
  demon->angel=FALSE;
#endif

#ifdef NEW_OS_INTERFACE
  ARMul_InstallExceptionHandler(state, HandleSWI_ARMulator, angel);
#endif

  verbose=ToolConf_DLookupBool(config, ARMulCnf_Verbose,FALSE);

  if (demon == NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

#ifndef NEW_OS_INTERFACE
  /* we need the handle for initialising the FPE */
  interf->handle=(void *)demon;
  interf->exception=Exception;
#endif

  demon->state = state;
  demon->hostif = ARMul_HostIf(state);
  demon->ErrorP = 0;
  demon->config = config;
  demon->command_line = NULL;

#ifdef NO_ANGEL
  ARMul_PrettyPrint(state, ", Soft Demon 1.4 [Demon SWIs");
  Demon_SetOptions(state,verbose,demon,config);
#else
  if (angel==NULL) {
    ARMul_PrettyPrint(state, ", Soft Demon 1.4 [Demon SWIs");
    Demon_SetOptions(state,verbose,demon,config);
  } else {
    ARMul_PrettyPrint(state, ", Soft Angel 1.4 [Angel SWIs");

    if (angel->handle_swi==angelswi_BOTH) {
      ARMul_PrettyPrint(state, ", Demon SWIs");
    }

    Angel_SetOptions(state,verbose,angel,config);

    /* Check that the heap and stack values are valid: */

    /* - heap base is below heap limit */
    if (angel->heap_base >= angel->heap_limit) {
      ARMul_Error err=ARMul_RaiseError(state,ARMulErr_AngelHeap,
                                       angel->heap_base, angel->heap_limit);
      free(demon);
      return err;
    }

    /* - stack base is above stack limit */
    if (angel->stack_base <= angel->stack_limit) {
      ARMul_Error err=ARMul_RaiseError(state,ARMulErr_AngelStack,
                                       angel->stack_base, angel->stack_limit);
      free(demon);
      return err;
    }

    /* - heap and stack do not overlap */
    if (((angel->stack_base > angel->heap_base) &&
         (angel->stack_base <= angel->heap_limit)) ||
        ((angel->stack_limit >= angel->heap_base) &&
         (angel->stack_limit < angel->heap_limit))) {
      ARMul_Error err=ARMul_RaiseError(state,ARMulErr_AngelHeapStack,
                                       angel->stack_limit, angel->stack_base,
                                       angel->heap_base, angel->heap_limit);
      free(demon);
      return err;
    }
      
  }
#endif

  ARMul_PrettyPrint(state,"]");

  /* Check if we're running on a Thumb processor */
  demon->flags=((ARMul_Properties(state) & ARM_Code16_Prop) ?
                AngelFlag_RunningOnThumb : 0);
  /* Set up the stack pointers */
  ARMul_SetReg(state, CURRENTMODE, 13, demon->map.sp.svc);
  ARMul_SetReg(state,(unsigned)SVC32MODE,13,demon->map.sp.svc);
  ARMul_SetReg(state,(unsigned)ABORT32MODE,13,demon->map.sp.abt);
  ARMul_SetReg(state,(unsigned)UNDEF32MODE,13,demon->map.sp.undef);
  ARMul_SetReg(state,(unsigned)IRQ32MODE,13,demon->map.sp.irq);
  ARMul_SetReg(state,(unsigned)FIQ32MODE,13,demon->map.sp.fiq);
  
  instr = 0xe59ff000 | (demon->map.soft_vectors - 8); /* load pc from soft vector */
  for (i = ARMul_ResetV; i <= ARMul_FIQV; i += 4)
    ARMul_WriteWord(state, i, instr); /* write hardware vectors */
  
  /* Install the soft vector code */
  for (i = ARMul_ResetV ; i <= ARMul_FIQV + 4 ; i += 4) {
    ARMul_WriteWord(state, demon->map.soft_vectors + i,
                    demon->map.soft_vec_code + i * 4);
    ARMul_WriteWord(state, demon->map.handlers + 2*i + 4L,
                    demon->map.soft_vec_code + sizeof(softvectorcode) - 4L);
  }
  for (i = 0 ; i < sizeof(softvectorcode) ; i += 4)
    ARMul_WriteWord(state, demon->map.soft_vec_code + i, softvectorcode[i/4]);
  
  /* Initialise our state */
  for (i = 0 ; i < FOPEN_MAX ; i++)
    demon->FileTable[i] = NULL ;
  for (i = 0 ; i < UNIQUETEMPS ; i++)
    demon->tempnames[i] = NULL ;
  
#ifndef NOFPE
  option=ToolConf_Lookup(config, Dbg_Cnf_FPE);
  if (ToolConf_AddFlag(option,0,1,TRUE)) { /* install fpe (conditionally) */
    ARMul_FPEInstall(state);
  }
#endif                          /* NOFPE */

  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,demon); /* for ErrorP */
  ARMul_InstallExitHandler(state,Exit,demon);
  
  return ARMulErr_NoError;
}

#ifndef NO_ANGEL

#  ifdef NEW_OS_INTERFACE

static ARMul_Error AngelInit(ARMul_State *state, toolconf config)
{
  return Init(state, config, TRUE);
}

static ARMul_Error DemonInit(ARMul_State *state, toolconf config)
{
  return Init(state, config, FALSE);
}

#  else  /* !NEW_OS_INTERFACE */

static ARMul_Error AngelInit(ARMul_State *state,
                             ARMul_OSInterface *interf,
                             toolconf config)
{
  return Init(state, interf, config, TRUE);
}

static ARMul_Error DemonInit(ARMul_State *state,
                             ARMul_OSInterface *interf,
                             toolconf config)
{
  return Init(state, interf, config, FALSE);
}
#  endif/* NEW_OS_INTERFACE */

#endif /* NO_ANGEL */

/*
 * Exit the Debug monitor, freeing it's state
 */

static void Exit(void *handle)
{
  Demon_State *OSptr=(Demon_State *)handle;

  if (OSptr->command_line) free(OSptr->command_line);

  free((char *)OSptr);
}


/*
 * Return the last Operating System Error.
 *
 * We do this by trapping the RDI call requesting it.
 */

static int RDI_info(void *handle, unsigned type,
                    ARMword *arg1, ARMword *arg2)
{
  Demon_State *OSptr = (Demon_State *)handle;
  IGNORE(arg2);
  switch (type) {
  case RDIErrorP:
    *arg1=OSptr->ErrorP;
    return RDIError_NoError;

  case RDISet_Cmdline: {
    char *cmd_line=(char *)arg1;
    size_t len=strlen(cmd_line);
    if (OSptr->command_line) free(OSptr->command_line);
    OSptr->command_line=(char *)malloc(len+1);
    if (OSptr->command_line==NULL)
      return ARMul_RaiseError(OSptr->state,ARMulErr_OutOfMemory);
    memcpy(OSptr->command_line,cmd_line,len+1);
  }
    return RDIError_NoError;

  case RDIVector_Catch:
    OSptr->vector_catch=(unsigned)*arg1;
    return RDIError_NoError;

    /*
     * These won't be called, because we don't reply to RDISemiHosting
     * To do that we also need to support the other SemiHosting SWIs.
     */
  case RDISemiHosting_SetARMSWI:
    if (OSptr->angel) {
      Angel_State *angel = (Angel_State *)OSptr;
      angel->AngelSWIARM = *arg1 & 0x00ffffff;
      return RDIError_NoError;
    }
    break;

  case RDISemiHosting_SetThumbSWI:
    if (OSptr->angel) {
      Angel_State *angel = (Angel_State *)OSptr;
      angel->AngelSWIThumb = *arg1 & 0x00ff;
      return RDIError_NoError;
    }
    break;

  case RDISemiHosting_GetARMSWI:
    if (OSptr->angel) {
      Angel_State *angel = (Angel_State *)OSptr;
      *arg1 = angel->AngelSWIARM & 0x00ffffff;
      return RDIError_NoError;
    }
    break;

  case RDISemiHosting_GetThumbSWI:
    if (OSptr->angel) {
      Angel_State *angel = (Angel_State *)OSptr;
      *arg1 = angel->AngelSWIThumb & 0x00ff;
      return RDIError_NoError;
    }
    break;

  case RDIInfo_CapabilityRequest | RDIErrorP:
  case RDIInfo_CapabilityRequest | RDISet_Cmdline:
  case RDIInfo_CapabilityRequest | RDIVector_Catch:
  case RDIInfo_CapabilityRequest | RDISemiHosting_SetARMSWI:
  case RDIInfo_CapabilityRequest | RDISemiHosting_SetThumbSWI:
  case RDIInfo_CapabilityRequest | RDISemiHosting_GetARMSWI:
  case RDIInfo_CapabilityRequest | RDISemiHosting_GetThumbSWI:
    return RDIError_NoError;
  }
  return RDIError_UnimplementedMessage;
}


#if 0
/*
 * The emulator calls one of these routines when a SWI instruction is
 * encountered. The parameter passed is the SWI number (lower 24 bits of
 * the instruction).
 */

unsigned Demon_PassOnSWI(Demon_State *OSptr,ARMword number)
{
  /* run along the list of children, trying to see if any of them like
   * this SWI */
  Demon_Child *child,*next;

  for (child=OSptr->child;child;child=next) {
    next=child->chain;
    if (child->handle_swi(OSptr,number,child))
      return TRUE;
  }
  return FALSE;
}  
#endif

static unsigned HandleSWI_ARMulator(
#ifdef NEW_OS_INTERFACE
   void *handle, ARMword vector, ARMword pc, ARMword instr
#else
   void *handle, ARMword number
#endif
   )
{
  /* This function handles the ARMulator SWIs
   */

  Demon_State *OSptr = (Demon_State *)handle;
  ARMul_State *state = OSptr->state;

#ifdef NEW_OS_INTERFACE
  ARMword number = instr & 0x00ffffff;
  if (vector != ARM_V_SWI) return FALSE;
  IGNORE(pc);
#endif  

  switch (number) {
  case 0x90: {          /* Branch through zero */
    ARMword oldpsr = ARMul_GetCPSR(state);
    ARMul_SetCPSR(state, (oldpsr & 0xffffffc0) | 0x13);
    ARMul_SetSPSR(state, SVC32MODE, oldpsr);
    ARMul_SetReg(state,CURRENTMODE,14, 0);
    goto TidyCommon;
  }
    
  case 0x98: {          /* Error */
    ARMword errorp = ARMul_GetReg(state,CURRENTMODE,0),
    regp = ARMul_GetReg(state,CURRENTMODE,1);
    unsigned i;
    ARMword errorpsr = ARMul_ReadWord(state, regp + 16*4);
    for (i = 0; i < 15; i++)
      ARMul_SetReg(state,(unsigned)errorpsr,i,
                   ARMul_ReadWord(state, regp + i*4L));
    ARMul_SetReg(state,CURRENTMODE,14, ARMul_ReadWord(state, regp + 15*4L));
    ARMul_SetReg(state,CURRENTMODE,10, errorp);
    ARMul_SetSPSR(state,CURRENTMODE,errorpsr);
    OSptr->ErrorP = errorp;
    goto TidyCommon;
  }
    
  case 0x94: {          /* Data abort */
    ARMword addr = (ARMul_GetReg(state,CURRENTMODE,14) -
                    OLDINSTRUCTIONSIZE * 2);
    ARMword cpsr = ARMul_GetCPSR(state);
    if (ARMul_GetMode(state) < USER32MODE)
      addr = addr & 0x3fffffc;
    ARMul_SetCPSR(state,ARMul_GetSPSR(state,cpsr));
    UnwindDataAbort(state, addr,
                    CPSRINSTRUCTIONSET(ARMul_GetSPSR(state,cpsr)));
#ifndef NOFPE
    if (ARMul_FPEAddressInEmulator(state,addr)) { /* in the FPE */
      ARMword regdump;
      if (ARMul_FPEVersion(state) == 0) {
        /* fpe340. user's registers are the only things on the fpe's stack */
        ARMword sp = ARMul_GetReg(state,CURRENTMODE,13);
        ARMul_SetReg(state,CURRENTMODE,13,sp+64); /* fix the aborting mode sp */
        regdump = sp;
      } else {
        /* new fpe. r12 addresses a frame on the fpe's stack, immediately
           above which are dumped the user's registers. The saved pc is for
           the instruction after the one causing the fault.
           */
        ARMword fp = ARMul_GetReg(state,CURRENTMODE,12);
        /* reset the fpe's stack */
        ARMul_SetReg(state,CURRENTMODE,13, fp + 64);
        regdump = fp;
      }
      ARMul_SetReg(state,CURRENTMODE,14,
                   ARMul_ReadWord(state,regdump + 15*4)); /* and its lr */
    { ARMword spsr = ARMul_GetSPSR(state,CURRENTMODE);
      int i;
      ARMul_SetCPSR(state,spsr);
      for (i = 0; i < 15; i++)
        ARMul_SetReg(state,(unsigned)spsr,i,
                     ARMul_ReadWord(state,regdump + (ARMword)i*4));
      ARMul_SetCPSR(state,cpsr);
      ARMul_SetReg(state,CURRENTMODE,14,
                   ARMul_ReadWord(state,regdump + 15*4) + 
                   INSTRUCTION32SIZE); /* botch it */
      ARMul_SetSPSR(state,CURRENTMODE,spsr);
    }
    } else
#endif
      ARMul_SetCPSR(state,cpsr);
    
    /* and fall through to correct r14 */
  }
    
  case 0x95:                    /* Address Exception */
    ARMul_SetReg(state,CURRENTMODE,14,
                 ARMul_GetReg(state, CURRENTMODE, 14)-OLDINSTRUCTIONSIZE);
  case 0x91:                    /* Undefined instruction */
  case 0x92:                    /* SWI */
  case 0x93:                    /* Prefetch abort */
  case 0x96:                    /* IRQ */
  case 0x97:                    /* FIQ */
    ARMul_SetReg(state,CURRENTMODE,14,
                 ARMul_GetReg(state, CURRENTMODE, 14)-OLDINSTRUCTIONSIZE);
  TidyCommon:
    if (OSptr->vector_catch & (1 << (number - 0x90))) {
      /* the pipelining the the RDI will undo */
      ARMul_SetR15(state, (ARMul_GetReg(state,CURRENTMODE,14)+
                           + OLDINSTRUCTIONSIZE * 2));
      ARMul_SetCPSR(state,ARMul_GetSPSR(state,ARMul_GetCPSR(state)));
      if (number == 0x90) {
        ARMul_HaltEmulation(state, RDIError_BranchThrough0);
      } else {
        ARMul_HaltEmulation(state,number -  0x8f);
      }
    } else {
      ARMword sp = ARMul_GetReg(state,CURRENTMODE,13);
      ARMul_WriteWord(state, sp - 4, ARMul_GetReg(state,CURRENTMODE,14));
      ARMul_WriteWord(state, sp - 8, ARMul_GetReg(state,CURRENTMODE,12));
      ARMul_WriteWord(state, sp - 12, ARMul_GetReg(state,CURRENTMODE,11));
      ARMul_WriteWord(state, sp - 16, ARMul_GetReg(state,CURRENTMODE,10));
      ARMul_SetReg(state,CURRENTMODE,13, sp - 16);
      ARMul_SetReg(state,CURRENTMODE,11, OSptr->map.handlers + 8 * (number-0x90));
    }
    return TRUE;
    
    /* SWI's 0x8x pass an abort of type x to the debugger if a handler returns */
    
  case 0x80: case 0x81: case 0x82: case 0x83:
  case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: {
    ARMword sp = ARMul_GetReg(state,CURRENTMODE,13);
    ARMul_SetReg(state,CURRENTMODE,10, ARMul_ReadWord(state, sp));
    ARMul_SetReg(state,CURRENTMODE,11, ARMul_ReadWord(state, sp + 4));
    ARMul_SetReg(state,CURRENTMODE,12, ARMul_ReadWord(state, sp + 8));
    ARMul_SetReg(state,CURRENTMODE,14, ARMul_ReadWord(state, sp + 12));
    ARMul_SetReg(state,CURRENTMODE,13, sp + 16);
    /* the pipelining the the RDI will undo */
    ARMul_SetR15(state,(ARMul_GetReg(state,CURRENTMODE,14)+
                        OLDINSTRUCTIONSIZE * 2));
    ARMul_SetCPSR(state,ARMul_GetSPSR(state,ARMul_GetCPSR(state)));
    if (number == 0x80)
      ARMul_HaltEmulation(state, RDIError_BranchThrough0); 
    else
      ARMul_HaltEmulation(state,number - 0x7f);
  }
      return TRUE;
      
    }
  
  return FALSE;
}

static unsigned HandleSWI_Demon(
#ifdef NEW_OS_INTERFACE
   void *handle, ARMword vector, ARMword pc, ARMword instr
#else
   void *handle, ARMword number
#endif
   )
{
  ARMword addr, temp;
  char buffer[BUFFERSIZE], *cptr;
  const char *ccptr;
  FILE *fptr;
  Demon_State *OSptr = (Demon_State *)handle;
  ARMul_State *state = OSptr->state;

#ifdef NEW_OS_INTERFACE
  ARMword number = instr & 0x00ffffff;
  if (vector != ARM_V_SWI) return FALSE;
#endif  

  /* Check that this really is a SWI, and not just a bug
   */

  if (!(OSptr->flags & AngelFlag_RunningOnThumb)) {
#ifndef NEW_OS_INTERFACE
    ARMword instr, pc;
    pc=ARMul_GetReg(state,CURRENTMODE,15)-8;
    instr=ARMul_ReadWord(state,pc);
#endif
    if (((instr & 0x0f000000)!=0x0f000000) ||
        ARMul_CondCheckInstr(state,instr)==0) {
      /* this is an erroneous SWI caused by the ARM7 bug */
      ARMul_ConsolePrint(state,"Demon: Demon thinks this isn't a real SWI.\n");
      ARMul_ConsolePrint(state,"       %08x: %08x\n",pc,instr);
      ARMul_SetR15(state, pc);
      return TRUE;
    }
  }

  switch (number) {
  case SWI_WriteC:
    /*
     * Write a byte, passed in r0, to the debug channel.
     */
    OSptr->hostif->writec(OSptr->hostif->hostosarg,
                          (int)ARMul_GetReg(state,CURRENTMODE,0));
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_Write0:
    /*
     * Write nul-terminated terminated string, pointed to by r0, to the
     * debug channel.
     */
    addr = ARMul_GetReg(state,CURRENTMODE,0);
    while ((temp = ARMul_ReadByte(state,addr++)) != 0)
      OSptr->hostif->writec(OSptr->hostif->hostosarg,(char)temp);
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_ReadC:
    /*
     * Read a byte from the debug channel, returning it in r0.
     */
    ARMul_SetReg(state,CURRENTMODE,0, (ARMword)OSptr->hostif->readc(OSptr->hostif->hostosarg));
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_CLI:                 /* In the angel code this is called CL_System
                                 * but the functionality is the same */
    /*
     * Pass the string, pointed to by r0, to the host's command-line
     * interpreter.
     */
    addr = ARMul_GetReg(state,CURRENTMODE,0);
    getstring(state,addr,buffer);
    ARMul_SetReg(state,CURRENTMODE,0, (ARMword)system(buffer));
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_GetEnv: {            /* Under angel this has changed to
                                 * CL_GetCmdLine and has a very different
                                 * function */
    /* @@@ More work needed here */
    /*
     * Returns in r0 the address of the command-line string used to invoke
     * the program, and in r1 the highest available address in user
     * memory.
     */
    ARMword memsize;
    ARMul_SetReg(state,CURRENTMODE,0, OSptr->map.cmd_line);
    /* The "memsize" has to be word-aligned */
    memsize=ARMul_GetMemSize(state) & ~3;
    if (memsize==0) memsize=OSptr->map.sp.user;
    if ((ARMul_SetConfig(state,0,0) & MMU_D) ||
        memsize<=0x04000000)
      ARMul_SetReg(state,CURRENTMODE,1,memsize);
    else                        /* 26-bit configuration - put stack inside memory! */
      ARMul_SetReg(state,CURRENTMODE,1,0x04000000);
  }
    addr = ARMul_GetReg(state,CURRENTMODE,0);
    ccptr = OSptr->command_line;
    if (ccptr == NULL)
      ccptr = "\0";
    do {
      temp = (ARMword)*ccptr++;
      ARMul_WriteByte(state,addr++,temp);
    } while (temp != 0);
    return TRUE;
    
  case SWI_Exit:                /* @@@ do we want to allow this under angel? */
    /*
     * Halt emulation. This is the way a program exits cleanly, returning
     * control to the debugger.
     */
    ARMul_HaltEmulation(state, RDIError_NoError);
#ifndef NO_ANGEL
    /*
     * Return to the "we could be demon or angel" case
     */
    if (OSptr->angel) {
      Angel_State *angel=(Angel_State *)handle;
      angel->handle_swi=angelswi_BOTH;
    }
#endif
    return TRUE;
    
  case SWI_EnterOS:             /* @@@ do we want to allow this under angel? */
    /*
     * Put the processor into supervisor mode. If the processor is 
     * currently in a 26-bit mode, then SVC26 is entered; otherwise SVC32
     * is entered.
     */
#if !defined(NO_SARMSD)
#if 0                           /* new strongarmulator doesn't need this */
    if (state->Processor & ARM_StrongARM_Prop) {
      /* Temp fix - clear the pipe since reading the CPSR does not at present
         always give the fully up to date result */
      extern void SARMul_debug_clear_pipe(void);
      SARMul_debug_clear_pipe();
    }
#endif
#endif
    temp = ARMul_GetCPSR(state);
    /* SWI_EnterOS always goes into 32 bit mode */
    if (ARMul_SetConfig(state,0,0) & MMU_P) {
      ARMul_SetCPSR(state, (temp & 0xffffffe0) | 0x13);
      ARMul_SetSPSR(state, CURRENTMODE, temp);
      ARMul_SetReg(state, CURRENTMODE, 14, ARMul_GetPC(state)-4);
    } else {
      ARMul_SetCPSR(state, (temp & 0xffffffe0) | 0x3);
      ARMul_SetSPSR(state, CURRENTMODE, temp);
      ARMul_SetReg(state, CURRENTMODE, 14, ARMul_GetPC(state)-4);
    }
    return TRUE ;

  case SWI_GetErrno: 

    /*
     * Return, in r0, the value of the C library "errno" variable
     * associated with the host support for this debug monitor.
     * "errno" may be set by a number of C library support SWIs;
     * including SWI_Remove, SWI_Rename, SWI_Open, SWI_Close,
     * SWI_Read, SWI_Write, SWI_Seek, etc.  Whether or not, and to
     * what value, "errno" is set is completely host-specific, except
     * where the ANSI C Standard defines the behaviour.
     */
    ARMul_SetReg(state,CURRENTMODE,0, OSptr->ErrorNo);
    return TRUE;
    
  case SWI_Clock:
    /*
     * Return in r0 the number of centi-seconds since the support code
     * began execution. In general only the difference between successive
     * calls to SWI_Clock can be meaningful.
     *
     * If the memory model supplies a read_clock function, use it
     * (i.e. using an emulated time), otherwise we use the ANSI clock()
     * function (i.e. using real time).
     */
    
    ARMul_SetReg(state, CURRENTMODE, 0, ARMul_ReadClock(state)/10000);
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_Time:
    /*
     * Return in r0 the number of seconds since the beginning of 1970
     * (the Unix time origin).
     */
    ARMul_SetReg(state,CURRENTMODE,0, (ARMword)time(NULL));
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_Remove:
    /*
     * Delete (remove, un-link, wipe, destroy) the file named by the
     * nul-terminated string addressed by r0. Return in r0 zero if
     * the removal succeeds, or a non-zero, host-specific, error code
     * if it fails.
     */
    getstring(state,ARMul_GetReg(state,CURRENTMODE,0),buffer);
    ARMul_SetReg(state,CURRENTMODE,0, remove(buffer));
    OSptr->ErrorNo = errno;
    return TRUE;
    
  case SWI_Rename: {
    /*
     * Rename the file named by the nul-terminated string pointed to by
     * r0 to the nul-terminated string pointed by r1. If the rename
     * succeeds zero is returned in r0. Otherwise a non-zero, host-
     * specific, error code is returned in r0.
     */
    char buffer2[BUFFERSIZE];
    
    getstring(state,ARMul_GetReg(state,CURRENTMODE,0),buffer);
    getstring(state,ARMul_GetReg(state,CURRENTMODE,1),buffer2);
    ARMul_SetReg(state,CURRENTMODE,0, rename(buffer,buffer2));
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_Open: {
    /*
     * Open the file (or device) named by the nul-terminated string
     * pointed to by r0, in the mode indicated by r1 (see the table
     * 'fmode' below for a translation). If the open succeeds a non-zero
     * file handle is returned in r0, which can then be quoted to
     * SWI_Close, SWI_Read, SWI_Write, SWI_Seek, SWI_Flen and
     * SWI_IsTTY. Nothing else may be asserted about the value of the
     * handle. If the open fails then zero is returned in r0.
     */
    static char* fmode[] = {"r","rb","r+","r+b",
                              "w","wb","w+","w+b",
                              "a","ab","a+","a+b",
                              "r","r","r","r"} /* last 4 are illegal */;
    char type;
    
    type = (char)(ARMul_GetReg(state,CURRENTMODE,1) & 15);
    getstring(state,ARMul_GetReg(state,CURRENTMODE,0),buffer);
    if (strcmp(buffer,":tt")==0) {         /* opening tty... */
      if (type==0 || type==1)              /* ... "r" */
        fptr=stdin;
      else if (type==4 || type==5)         /* ... "w" */
        fptr=stdout;
      else                                 /* ... invalid */
        fptr=NULL;
    } else
      fptr = fopen(buffer,fmode[(int)type]);
    
    if (fptr != NULL) {
      for (temp = 0; temp < FOPEN_MAX; temp++)
        if (OSptr->FileTable[temp] == NULL ||
            (OSptr->FileTable[temp] == fptr &&
             (OSptr->FileFlags[temp] & 1) == (type & 1))) {
          OSptr->FileTable[temp] = fptr;
          OSptr->FileFlags[temp] = type & 1; /* preserve the binary bit */
          ARMul_SetReg(state,CURRENTMODE,0, (ARMword)(temp + 1));
          OSptr->ErrorNo = errno;
          return TRUE;                     /* return here */
        }

      /* Too many open files */
      OSptr->ErrorNo = EMFILE;
    } else
      OSptr->ErrorNo = errno;
  }
    /* SWI_Open failed - return 0 */
    ARMul_SetReg(state,CURRENTMODE,0, 0);
    return TRUE;
    
  case SWI_Close:
    /*
     * On entry, r0 must be a handle for an open file, previously returned
     * by SWI_Open. If the close succeeds zero is returned in r0; otherwise
     * a non-zero value is returned.
     */
    temp = ARMul_GetReg(state,CURRENTMODE,0);
    if (temp == 0 || temp > FOPEN_MAX || OSptr->FileTable[temp - 1] == 0) {
      OSptr->ErrorNo = EBADF;
      ARMul_SetReg(state,CURRENTMODE,0, -1L);
      return TRUE;
    }
    temp--;
    fptr = OSptr->FileTable[temp];

    if ((fptr == stdin) || (fptr == stdout)) {
      ARMul_SetReg(state,CURRENTMODE,0, 0);
    } else {
      ARMul_SetReg(state,CURRENTMODE,0, fclose(fptr));
      /* Don't NULL the FileTable entry in the case where fptr is
       * stdin/stdout, since that would effectively close *all* handles
       * opened onto ":tt"
       */
      OSptr->FileTable[temp] = NULL;
      OSptr->ErrorNo = errno;
    }
    return TRUE;
    
  case SWI_Write: {
    /*
     * On entry, r0 must contain a handle for a previously opened file;
     * r1 points to a buffer in the callee; and r2 contains the number
     * of bytes to be written from the buffer to the file. SWI_Write
     * returns in r0 the number of bytes /not/ written, and so indicates
     * success with a zero value.
     */
    unsigned size, upto, type;
    char ch;
    
    temp = ARMul_GetReg(state,CURRENTMODE,0);
    if (temp == 0 || temp > FOPEN_MAX || OSptr->FileTable[temp - 1] == 0) {
      OSptr->ErrorNo = EBADF;
      ARMul_SetReg(state,CURRENTMODE,0, -1L);
      return TRUE;
    }
    temp--;
    fptr = OSptr->FileTable[temp];
    type = OSptr->FileFlags[temp];
    addr = ARMul_GetReg(state,CURRENTMODE,1);
    size = (unsigned)ARMul_GetReg(state,CURRENTMODE,2);
    
    if (type & READOP)
      fseek(fptr,0L,SEEK_CUR);
    OSptr->FileFlags[temp] = (type & BINARY) | WRITEOP;;
    while (size > 0) {
      if (size >= BUFFERSIZE)
        upto = BUFFERSIZE;
      else
        upto = size;
      for (cptr = buffer; (cptr - buffer) < upto; cptr++) {
        ch = (char)ARMul_ReadByte(state,(ARMword)addr++);
        *cptr = FIXCRLF(type,ch);
      }
      temp = (fptr == stderr || fptr == stdout)
        ? OSptr->hostif->write(OSptr->hostif->hostosarg, buffer, upto)
          : fwrite(buffer,1,upto,fptr);
      if (temp < upto) {
        ARMul_SetReg(state,CURRENTMODE,0, (ARMword)(size - temp));
        OSptr->ErrorNo = errno;
        return TRUE;
      }
      size -= upto;
    }
    ARMul_SetReg(state,CURRENTMODE,0, 0);
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_Read: {
    /*
     * On entry, r0 must contain a handle for a previously opened file
     * or device; r1 points to a buffer in the callee; and r2 contains
     * the number of bytes to be read from the file into the buffer.
     * SWI_Read returns in r0 the number of bytes not read, and so
     * indicates the success of the read by returning zero. If the handle
     * is for an interactive device (SWI_IsTTY returns non-zero for this
     * handle), then a non-zero return from SWI_Read indicates that the
     * line did not fill the buffer.
     */
    unsigned size, upto, type;
    char ch;
    
    temp = ARMul_GetReg(state,CURRENTMODE,0);
    if (temp == 0 || temp > FOPEN_MAX || OSptr->FileTable[temp - 1] == 0) {
      OSptr->ErrorNo = EBADF;
      ARMul_SetReg(state,CURRENTMODE,0, -1L);
      return TRUE;
    }
    temp--;
    fptr = OSptr->FileTable[temp];
    addr = ARMul_GetReg(state,CURRENTMODE,1);
    size = (unsigned)ARMul_GetReg(state,CURRENTMODE,2);
    type = OSptr->FileFlags[temp];
    
    if (type & WRITEOP)
      fseek(fptr,0L,SEEK_CUR);
    OSptr->FileFlags[temp] = (type & BINARY) | READOP;;
    while (size > 0) {
      if (isatty_(fptr)) {
        upto = (size >= BUFFERSIZE)?BUFFERSIZE:size + 1;
        if (OSptr->hostif->gets(OSptr->hostif->hostosarg, buffer, upto) != 0)
          temp = strlen(buffer);
        else
          temp = 0;
        upto--;         /* 1 char used for terminating null */
      } else {
        upto = (size>=BUFFERSIZE)?BUFFERSIZE:size;
        temp = fread(buffer,1,upto,fptr);
      }
      for (cptr = buffer; (cptr - buffer) < temp; cptr++) {
        ch = *cptr;
        ARMul_WriteByte(state,(ARMword)addr++,FIXCRLF(type,ch));
      }
      if (temp < upto) {
        ARMul_SetReg(state,CURRENTMODE,0, (ARMword)(size - temp));
        OSptr->ErrorNo = errno;
        return TRUE;
      }
      size -= upto;
    }
    ARMul_SetReg(state,CURRENTMODE,0, 0);
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_Seek: {
    /*
     * On entry r0 must contain a handle for a seekable object, and r1
     * the absolute byte position to be sought to. If the request can be
     * honoured then SWI_Seek returns 0 in r0; otherwise it returns a
     * host-specific non-zero value. Note that the effect of seeking
     * outside of the current extent of the file object is undefined.
     */
    int handle=ARMul_GetReg(state,CURRENTMODE,0);
    if (handle == 0 ||
        handle > FOPEN_MAX ||
        OSptr->FileTable[handle - 1] == 0) {
      OSptr->ErrorNo = EBADF;
      ARMul_SetReg(state,CURRENTMODE,0, -1L);
      return TRUE;
    }
    fptr = OSptr->FileTable[handle - 1];
    ARMul_SetReg(state,CURRENTMODE,0,
                 fseek(fptr,(long)ARMul_GetReg(state,CURRENTMODE,1),
                       SEEK_SET));
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_Flen: {
    /*
     * On entry r0 contains a handle for a previously opened seekable
     * file object. SWI_Flen returns in r0 the current length of the
     * file object, otherwise it returns -1.
     */
    int handle=ARMul_GetReg(state,CURRENTMODE,0);
    if (handle == 0 ||
        handle > FOPEN_MAX ||
        OSptr->FileTable[handle - 1] == 0) {
      OSptr->ErrorNo = EBADF;
      ARMul_SetReg(state,CURRENTMODE,0, -1L);
      return TRUE;
    }
    fptr = OSptr->FileTable[handle - 1];
    addr = (ARMword)ftell(fptr);
    if (fseek(fptr,0L,SEEK_END) < 0)
      ARMul_SetReg(state,CURRENTMODE,0, -1);
    else {
      ARMul_SetReg(state,CURRENTMODE,0, (ARMword)ftell(fptr));
      (void)fseek(fptr,addr,SEEK_SET);
    }
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_IsTTY: {
    /*
     * On entry r0 must contain a handle for a handle for a previously
     * opened file or device object. On exit r0 contains 1 if the handle
     * identifies an interactive device; 0 otherwise.
     */
    int handle=ARMul_GetReg(state,CURRENTMODE,0);
    if (handle == 0 ||
        handle > FOPEN_MAX ||
        OSptr->FileTable[handle - 1] == 0) {
      OSptr->ErrorNo = EBADF;
      ARMul_SetReg(state,CURRENTMODE,0, -1L);
      return TRUE;
    }
    fptr = OSptr->FileTable[handle - 1];
    ARMul_SetReg(state,CURRENTMODE,0, isatty_(fptr));
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_TmpNam: {
    /*
     * On entry r0 points to a buffer and r1 contains its length (r1 should
     * be at least the value of L_tmpnam on the host system). On successful
     * return, r0 points to the buffer which contains a host temporary file
     * name. If the request cannot be satisfied (e.g. because the buffer is
     * too small) then zero is returned in r0.
     */
    ARMword size;
    
    addr = ARMul_GetReg(state,CURRENTMODE,0);
    temp = ARMul_GetReg(state,CURRENTMODE,1) & 0xff;
    size = ARMul_GetReg(state,CURRENTMODE,2);
    if (OSptr->tempnames[temp] == NULL) {
      if ((OSptr->tempnames[temp] = (char *)malloc(L_tmpnam)) == NULL) {
        ARMul_SetReg(state,CURRENTMODE,0, 0);
        return TRUE;
      }
      (void)tmpnam(OSptr->tempnames[temp]);
    }
    cptr = OSptr->tempnames[temp];
    if (strlen(cptr) > ARMul_GetReg(state,CURRENTMODE,2))
      ARMul_SetReg(state,CURRENTMODE,0, 0);
    else
      do {
        ARMul_WriteByte(state,addr++,*cptr);
      } while (*cptr++ != 0);
    OSptr->ErrorNo = errno;
  }
    return TRUE;
    
  case SWI_InstallHandler: {
    /*
     * SWI_InstallHandler installs a handler for a hardware exception. On
     * entry r0 contains the exception number; r1 contains a value to pass
     * to the handler when it is eventually invoked; and r2 contains the
     * address of the handler. On return r2 contains the adderss of the
     * previous handler and r1 contains its argument.
     */
    ARMword handlerp = (OSptr->map.handlers + 
                        ARMul_GetReg(state,CURRENTMODE,0) * 8);
    ARMword oldr1 = ARMul_ReadWord(state, handlerp),
    oldr2 = ARMul_ReadWord(state, handlerp + 4);
    ARMul_WriteWord(state, handlerp, ARMul_GetReg(state,CURRENTMODE,1));
    ARMul_WriteWord(state, handlerp + 4, ARMul_GetReg(state,CURRENTMODE,2));
    ARMul_SetReg(state,CURRENTMODE,1, oldr1);
    ARMul_SetReg(state,CURRENTMODE,2, oldr2);
  }
    return TRUE;
    
  case SWI_GenerateError:
    /*
     * On entry r0 points to an error block (containing a 32-bit number,
     * followed by a zero-terminated error string); r1 points to a 17-word
     * block containing the values of the ARM CPU registers at the instant
     * the error occured (the 17th word contains the PSR).
     */
    ARMul_SWIHandler(state,
                     ARMul_ReadWord(state, OSptr->map.soft_vectors + ARMErrorV));
#if 0                           /* was... */
    ARMul_Abort(state, ARMSWIV);
    if (state->Emulate)
      ARMul_SetR15(state, ARMul_ReadWord(state, OSptr->map.soft_vectors + ARMErrorV));
#endif
    return TRUE;
    
    /* SWI's 0x9x unwind the state of the CPU after an abort of type x */
  }

#ifdef NEW_OS_INTERFACE
  return FALSE;
#else
  return HandleSWI_ARMulator(handle,number);
#endif
}


#ifndef NO_ANGEL
/*
 * This is the function that is eventually called for an Angel compiled
 * library.
 */

static unsigned HandleSWI_Angel(
#ifdef NEW_OS_INTERFACE
   void *handle, ARMword vector, ARMword pc, ARMword instr
#else
   void *handle, ARMword number
#endif
   )
{
  ARMword addr, temp;
  char buffer[BUFFERSIZE], *cptr;
  const char *ccptr;
  FILE *fptr;
  Angel_State *angel = (Angel_State *)handle;
  Demon_State *OSptr = &angel->demon;
  ARMul_State *state = OSptr->state;
  unsigned int angel_swi;

#ifdef NEW_OS_INTERFACE
  ARMword number = instr & 0x00ffffff;
  if (vector != ARM_V_SWI) return FALSE;
#endif  

  if (!(OSptr->flags & AngelFlag_RunningOnThumb)) {
#ifndef NEW_OS_INTERFACE
    ARMword instr, pc;
    pc=ARMul_GetReg(state,CURRENTMODE,15)-8;
    instr=ARMul_ReadWord(state,pc);
#endif
    if (((instr & 0x0f000000)!=0x0f000000) ||
        ARMul_CondCheckInstr(state,instr)==0) {
      /* this is an erroneous SWI caused by the ARM7 bug */
      ARMul_ConsolePrint(state,"Demon: Demon thinks this isn't a real SWI.\n");
      ARMul_ConsolePrint(state,"       %08x: %08x\n",pc,instr);
      ARMul_SetR15(state, pc);
      return TRUE;
    }
  }

  angel_swi=(angel->AngelSWIARM & 0x00ffffff);
  if ((OSptr->flags & AngelFlag_RunningOnThumb) &&
      (ARMul_GetCPSR(angel->demon.state) & (1<<5))) {
    /* running on a Thumb-aware processor, in a Thumb mode */
    angel_swi=angel->AngelSWIThumb & 0xff;
  }    

  if (number == angel_swi) {
    number = (ARMul_GetReg(state,CURRENTMODE,0));
    if (number == angel_SWIreason_EnterSVC) {
      if (ARMul_GetMode(state) >= USER32MODE) { /* Stay in entry (ARM/THUMB) state */
        ARMul_SetCPSR(state, (ARMul_GetCPSR(state) & 0xffffffe0) | 0x13);
      } else {
        ARMul_SetCPSR(state, (ARMul_GetCPSR(state) & 0xffffffc0) | 0x3);
      }
      /* The real Angel SWI also sets R0 to be the address of the Angel fn
       * Angel_ExitToUSR but that is impossible here (there is in all likelihood
       * no Angel present under ARMulator !), but since we can't set R0 to NULL
       * so that this can be detected.
       */
      ARMul_SetReg(state,CURRENTMODE,0, 0);
      return(TRUE);
    }
    
    /* Everything looks good for angel Lets fake it to look like a normal
     * swi call and copy the arguments into args */
    switch (number & 0xff) {
    case SYS_WRITEC:
      OSptr->hostif->writec(OSptr->hostif->hostosarg,
                            ARMul_ReadByte(state, ARMul_GetReg(state,CURRENTMODE,1)));
      OSptr->ErrorNo = errno;
      return(TRUE);
        
    case SYS_WRITE0:
      addr = ARMul_GetReg(state,CURRENTMODE,1);
      while ((temp = ARMul_ReadByte(state,addr++)) != 0)
        OSptr->hostif->writec(OSptr->hostif->hostosarg,(char)temp);
      OSptr->ErrorNo = errno;
      return(TRUE);
      
    case SYS_READC:
      ARMul_SetReg(state,CURRENTMODE,0, (ARMword)OSptr->hostif->readc(OSptr->hostif->hostosarg));
      OSptr->ErrorNo = errno;
      return(TRUE);
      
    case SYS_SYSTEM: {
      unsigned int c, len;
      /*
       * In the angel code this is called CL_System
       * but the functionality is the same
       */
      addr = ARMul_GetReg(state, CURRENTMODE,1);
      len = ARMul_ReadWord(state, addr + 4);
      if (ARMul_ReadWord(state, addr) == (unsigned int)NULL) {
        /* handle the "check for copmmand processor case */
        ARMul_SetReg(state, CURRENTMODE, 0, 1);
        return TRUE;
      }
      for (c = 0; c < len ; c++) {
        buffer[c] = ARMul_ReadByte(state, ARMul_ReadWord(state, addr) + c);
      }
      
      ARMul_SetReg(state, CURRENTMODE, 0, (ARMword)system(buffer));
      OSptr->ErrorNo = errno;
      return(TRUE);
    }
    case SYS_GET_CMDLINE:
      /*
       * under angel this has changed to CL_GetCmdLine and
       * has a very different function
       */
      /* @@@ More work needed here */
#ifdef OldCode
      ARMul_GetReg(state,CURRENTMODE,1) = demon->map.cmd_line;
      if (ARMul_GetMemSize(state))
        ARMul_GetReg(state,CURRENTMODE,1) = ARMul_GetMemSize(state);
      else
        ARMul_GetReg(state,CURRENTMODE,1) = demon->map.sp.user;*/
#endif
      
      addr = ARMul_ReadWord(state, ARMul_GetReg(state,CURRENTMODE,1));
      ccptr = OSptr->command_line;
      if (ccptr == NULL)
        ccptr = "\0";
      do {
        temp = (ARMword)*ccptr++;
        ARMul_WriteByte(state,addr++,temp);
      } while (temp != 0);
      ARMul_SetReg(state,CURRENTMODE,0, 0);
      return(TRUE);
      
    case SYS_ISERROR:
      ARMul_SetReg(state,CURRENTMODE,0, 0);
      return (TRUE);
      
    case SYS_ERRNO:
      ARMul_SetReg(state, CURRENTMODE, 0, OSptr->ErrorNo);
      return(TRUE);
      
    case SYS_CLOCK: {
      ARMul_SetReg(state, CURRENTMODE, 0, ARMul_ReadClock(state)/10000);
      OSptr->ErrorNo = errno;
      return TRUE;
    }
    return(TRUE);
      
    case SYS_TIME:
      ARMul_SetReg(state,CURRENTMODE,0, (ARMword)time(NULL));
      OSptr->ErrorNo = errno;
      return(TRUE);
      
    case SYS_REMOVE:
      addr = ARMul_ReadWord(state, ARMul_GetReg(state,CURRENTMODE,1));
      getstring(state,ARMul_ReadWord(state,
                                     ARMul_GetReg(state,CURRENTMODE,1)), buffer);
      ARMul_SetReg(state,CURRENTMODE,0,remove(buffer));
      OSptr->ErrorNo = errno;
      return(TRUE);
      
    case SYS_RENAME: {
      char buffer2[BUFFERSIZE];
      addr = ARMul_GetReg(state,CURRENTMODE,1);
      
      getstring(state,ARMul_ReadWord(state, addr), buffer);
      getstring(state,ARMul_ReadWord(state, addr+8), buffer2);
      ARMul_SetReg(state,CURRENTMODE,0, rename(buffer,buffer2));
      OSptr->ErrorNo = errno;
    }
    return(TRUE);
      
    case SYS_OPEN: {
      static char* fmode[] = {"r","rb","r+","r+b",
                              "w","wb","w+","w+b",
                              "a","ab","a+","a+b",
                              "r","r","r","r"} /* last 4 are illegal */;
      char type;
      ARMword argbase = ARMul_GetReg(state,CURRENTMODE,1);
      
      type = (char)(ARMul_ReadWord(state, argbase+4) & 0xf);
      getstring(state,ARMul_ReadWord(state, argbase),buffer);
      if (strcmp(buffer,":tt")==0 && (type == 0 || type == 1)) /* opening tty "r" */
        fptr = stdin;
      else if (strcmp(buffer,":tt")==0 && (type==4 || type==5)) /* opening tty "w" */
        fptr = stdout;
      else {
        fptr = fopen(buffer,fmode[(int)type]);
      }
      ARMul_SetReg(state,CURRENTMODE,0, (ARMword)NULL);
      if (fptr != NULL) {
        for (temp = 0; temp < FOPEN_MAX; temp++)
          if (OSptr->FileTable[temp] == NULL ||
              (OSptr->FileTable[temp] == fptr &&
               (OSptr->FileFlags[temp] & 1) == (type & 1))) {
            OSptr->FileTable[temp] = fptr;
            OSptr->FileFlags[temp] = type & 1; /* preserve the binary bit */
            ARMul_SetReg(state,CURRENTMODE,0, (ARMword)(temp + 1));
            break;
          }
        if (ARMul_GetReg(state,CURRENTMODE,0) == 0)
          OSptr->ErrorNo = EMFILE; /* too many open files */
        else
          OSptr->ErrorNo = errno;
      }
      else {
        OSptr->ErrorNo = errno;
        ARMul_SetReg(state,CURRENTMODE,0, NONHANDLE);
      }
    }
    return(TRUE);
      
    case SYS_CLOSE:
      temp = ARMul_ReadWord(state, ARMul_GetReg(state,CURRENTMODE,1));
      if (temp == 0 || temp > FOPEN_MAX || OSptr->FileTable[temp - 1] == 0) {
        OSptr->ErrorNo = EBADF;
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
        return(TRUE);
      }
      temp--;
      fptr = OSptr->FileTable[temp];
      if (fptr == stdin || fptr == stdout)
        ARMul_SetReg(state,CURRENTMODE,0, 0);
      else {
        ARMul_SetReg(state,CURRENTMODE,0, fclose(fptr));
        /* Don't NULL the FileTable entry in the case where fptr is
         * stdin/stdout, since that would effectively close *all* handles
         * opened onto ":tt"
         */
        OSptr->FileTable[temp] = NULL;
        OSptr->ErrorNo = errno;
      }
      return(TRUE);
      
    case SYS_WRITE: {
      unsigned size, upto, type;
      char ch;
      ARMword argbase = ARMul_GetReg(state,CURRENTMODE,1);
      
      temp = ARMul_ReadWord(state, argbase);
      if (temp == 0 || temp > FOPEN_MAX || OSptr->FileTable[temp - 1] == 0) {
        OSptr->ErrorNo = EBADF;
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
        return(TRUE);
      }
      temp--;
      fptr = OSptr->FileTable[temp];
      type = OSptr->FileFlags[temp];
      addr = ARMul_ReadWord(state, argbase+4);
      size = (unsigned)ARMul_ReadWord(state, argbase+8);
      if (type & READOP)
        fseek(fptr,0L,SEEK_CUR);
      OSptr->FileFlags[temp] = (type & BINARY) | WRITEOP;;
      while (size > 0) {
        if (size >= BUFFERSIZE)
          upto = BUFFERSIZE;
        else
          upto = size;
        for (cptr = buffer; (cptr - buffer) < upto; cptr++) {
          ch = (char)ARMul_ReadByte(state,(ARMword)addr++);
          *cptr = FIXCRLF(type,ch);
        }
        temp = (fptr == stderr || fptr == stdout)
          ? OSptr->hostif->write(OSptr->hostif->hostosarg, buffer, upto)
          : fwrite(buffer,1,upto,fptr);
        if (temp < upto) {
          ARMul_SetReg(state,CURRENTMODE,0, (ARMword)(size - temp));
          OSptr->ErrorNo = errno;
          return(TRUE);
        }
        size -= upto;
      }
      ARMul_SetReg(state,CURRENTMODE,0, 0);
      OSptr->ErrorNo = errno;
    }
    return(TRUE);
      
    case SYS_READ: {
      unsigned size, upto, type;
      char ch;
      
      ARMword argbase = ARMul_GetReg(state,CURRENTMODE,1);
      temp = ARMul_ReadWord(state, argbase);
      if (temp == 0 || temp > FOPEN_MAX || OSptr->FileTable[temp - 1] == 0) {
        OSptr->ErrorNo = EBADF;
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
        return(TRUE);
      }
      temp--;
      fptr = OSptr->FileTable[temp];
      addr = ARMul_ReadWord(state, argbase+4);
      size = (unsigned)ARMul_ReadWord(state, argbase+8);
      type = OSptr->FileFlags[temp];
      if (type & WRITEOP)
        fseek(fptr,0L,SEEK_CUR);
      OSptr->FileFlags[temp] = (type & BINARY) | READOP;;
      while (size > 0) {
        if (isatty_(fptr)) {
          upto = (size >= BUFFERSIZE)?BUFFERSIZE:size + 1;
          if (OSptr->hostif->gets(OSptr->hostif->hostosarg, buffer, upto) != 0)
            temp = strlen(buffer);
          else
            temp = 0;
          upto--;               /* 1 char used for terminating null */
        }
        else {
          upto = (size>=BUFFERSIZE)?BUFFERSIZE:size;
          temp = fread(buffer,1,upto,fptr);
        }
        for (cptr = buffer; (cptr - buffer) < temp; cptr++) {
          ch = *cptr;
          ARMul_WriteByte(state,(ARMword)addr++,FIXCRLF(type,ch));
        }
        if (temp < upto) {
          ARMul_SetReg(state,CURRENTMODE,0, (ARMword)(size - temp));
          OSptr->ErrorNo = errno;
          return(TRUE);
        }
        size -= upto;
      }
      ARMul_SetReg(state,CURRENTMODE,0, 0);
      OSptr->ErrorNo = errno;
    }
    return(TRUE);
      
    case SYS_SEEK: {
      ARMword argbase = ARMul_GetReg(state,CURRENTMODE,1);
      if (ARMul_ReadWord(state, argbase)== 0 ||
          ARMul_ReadWord(state, argbase) > FOPEN_MAX ||
          OSptr->FileTable[ARMul_ReadWord(state, argbase) - 1] == 0) {
        OSptr->ErrorNo = EBADF;
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
        return(TRUE);
      }
      fptr = OSptr->FileTable[ ARMul_ReadWord(state, argbase)- 1];
      ARMul_SetReg(state,CURRENTMODE,0,
                   fseek(fptr,(long)ARMul_ReadWord(state, argbase+4),SEEK_SET));
      OSptr->ErrorNo = errno;
    }
    return(TRUE);
      
    case SYS_FLEN: {
      unsigned int arg = ARMul_ReadWord(state, ARMul_GetReg(state,CURRENTMODE,1));
      if (arg == 0 || arg > FOPEN_MAX
          || OSptr->FileTable[arg - 1] == 0) {
        OSptr->ErrorNo = EBADF;
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
        return(TRUE);
      }
      fptr = OSptr->FileTable[arg - 1];
      addr = (ARMword)ftell(fptr);
      if (fseek(fptr,0L,SEEK_END) < 0)
        ARMul_SetReg(state,CURRENTMODE,0, -1);
      else {
        ARMul_SetReg(state,CURRENTMODE,0, (ARMword)ftell(fptr));
        (void)fseek(fptr,addr,SEEK_SET);
      }
      OSptr->ErrorNo = errno;
    }
    return(TRUE);
      
    case SYS_ISTTY:
      temp = ARMul_ReadWord(state, ARMul_GetReg(state,CURRENTMODE,1));
      if (temp == 0 || temp > FOPEN_MAX
          || OSptr->FileTable[temp - 1] == 0) {
        OSptr->ErrorNo = EBADF;
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
        return(TRUE);
      }
      fptr = OSptr->FileTable[temp - 1];
      ARMul_SetReg(state,CURRENTMODE,0, isatty_(fptr));
      OSptr->ErrorNo = errno;
      return(TRUE);
      
    case SYS_TMPNAM:{
      ARMword argbase =ARMul_GetReg(state,CURRENTMODE,1);
      ARMword size;
      
      addr = ARMul_ReadWord(state, argbase);
      temp = ARMul_ReadWord(state, argbase+4) & 0xff;
      size = ARMul_ReadWord(state, argbase+8);
      ARMul_SetReg(state,CURRENTMODE,0, 0); /*IDJ: default to sucess */
      if (OSptr->tempnames[temp] == NULL) {
        if ((OSptr->tempnames[temp] = (char *)malloc(L_tmpnam)) == NULL) {
          ARMul_SetReg(state,CURRENTMODE,0, -1L);
          return(TRUE);
        }
        (void)tmpnam(OSptr->tempnames[temp]);
      }
      cptr = OSptr->tempnames[temp];
      if (strlen(cptr) > size)
        ARMul_SetReg(state,CURRENTMODE,0, -1L);
      else
        do {
          ARMul_WriteByte(state,addr++,*cptr);
        } while (*cptr++ != 0);
      OSptr->ErrorNo = errno;
    }
    return(TRUE);

    case SYS_HEAPINFO: {
      ARMword r1=ARMul_GetReg(state,CURRENTMODE,1);
      ARMword base=ARMul_ReadWord(state, r1);
      ARMul_WriteWord(state, base, angel->heap_base);
      ARMul_WriteWord(state, base+4, angel->heap_limit);
      ARMul_WriteWord(state, base+8, angel->stack_base);
      ARMul_WriteWord(state, base+12, angel->stack_limit);
    }
    return (TRUE);

    case angel_SWIreason_ReportException: {
      unsigned int reason;

      reason = ARMul_GetReg(state, CURRENTMODE, 1);
      switch (reason) {
      case ADP_Stopped_ApplicationExit:
        ARMul_HaltEmulation(state, RDIError_NoError);
        /*
         * Return to the "we could be demon or angel" case
         */
        angel->handle_swi=angelswi_BOTH;
        return TRUE;
        /* 
         * most of these should never happen, do it for completeness in
         * any case and in future we may want to do some special handling
         */
      case ADP_Stopped_BranchThroughZero:
        ARMul_HaltEmulation(state, RDIError_BranchThrough0);
        return TRUE;
      case ADP_Stopped_UndefinedInstr:
        ARMul_HaltEmulation(state, RDIError_UndefinedInstruction);
        return TRUE;
      case ADP_Stopped_PrefetchAbort:
        ARMul_HaltEmulation(state, RDIError_PrefetchAbort);
        return TRUE;
      case ADP_Stopped_DataAbort:
        ARMul_HaltEmulation(state, RDIError_DataAbort);
        return TRUE;
      case ADP_Stopped_AddressException:
        ARMul_HaltEmulation(state, RDIError_AddressException);
        return TRUE;
      case ADP_Stopped_FIQ:
        ARMul_HaltEmulation(state, RDIError_FIQ);
        return TRUE;
      case ADP_Stopped_IRQ:
        ARMul_HaltEmulation(state, RDIError_IRQ);
        return TRUE;
      case ADP_Stopped_BreakPoint:
        ARMul_HaltEmulation(state, RDIError_BreakpointReached);
        return TRUE;
      case ADP_Stopped_WatchPoint:
        ARMul_HaltEmulation(state, RDIError_WatchpointAccessed);
        return TRUE;
      case ADP_Stopped_StepComplete:
        ARMul_HaltEmulation(state, RDIError_ProgramFinishedInStep);
        return TRUE;
      case ADP_Stopped_UserInterruption:
        ARMul_HaltEmulation(state, RDIError_UserInterrupt);
        return TRUE;
      case ADP_Stopped_StackOverflow:
      case ADP_Stopped_DivisionByZero:
      case ADP_Stopped_OSSpecific:
      case ADP_Stopped_InternalError:
      case ADP_Stopped_RunTimeErrorUnknown:
      default :
        /* fall through but print a warning */
        ARMul_ConsolePrint(state,
                           "ERROR: Unhandled ADP_Stopped exception 0x%x\n",
                           reason);
        return TRUE;
      }
    }
    default:
      ARMul_ConsolePrint(state,"ERROR: Untrapped Angel SWI 0x%x\n", number);
      ARMul_HaltEmulation(state, RDIError_SoftwareInterrupt);
      return TRUE;
    }
  }

#ifdef NEW_OS_INTERFACE
  return FALSE;
#else
  /* try to pass on SWI */
  return HandleSWI_ARMulator(OSptr,number);
#endif
}

static unsigned HandleSWI_Both(
#ifdef NEW_OS_INTERFACE
   void *handle, ARMword vector, ARMword pc, ARMword instr
#else
   void *handle, ARMword number
#endif
   )
{
  Angel_State *angel=(Angel_State *)handle;

#ifdef NEW_OS_INTERFACE
  ARMword number = instr & 0x00ffffff;
  if (vector != ARM_V_SWI) return FALSE;
#endif  

  if (angel->handle_swi==angelswi_BOTH) {
    unsigned int angel_swi;
    
    angel_swi=(angel->AngelSWIARM & 0x00ffffff);
    if ((angel->demon.flags & AngelFlag_RunningOnThumb) &&
        (ARMul_GetCPSR(angel->demon.state) & (1<<5))) {
      /* running on a Thumb-aware processor, in a Thumb mode */
      angel_swi=angel->AngelSWIThumb & 0xff;
    }    
    
    angel->handle_swi=(number==angel_swi) ? angelswi_ANGEL : angelswi_DEMON;
  }

#ifdef NEW_OS_INTERFACE
  if (angel->handle_swi==angelswi_ANGEL)
    return HandleSWI_Angel(handle,vector,pc,instr);
  else
    return HandleSWI_Demon(handle,vector,pc,instr);
#else
  if (angel->handle_swi==angelswi_ANGEL)
    return HandleSWI_Angel(handle,number);
  else
    return HandleSWI_Demon(handle,number);
#endif
}
#endif


#ifndef NEW_OS_INTERFACE
/*
 * The emulator calls this routine when an Exception occurs.  The second
 * parameter is the address of the relevant exception vector.  Returning
 * FALSE from this routine causes the trap to be taken, TRUE causes it to
 * be ignored (so set state->Emulate to FALSE!).
 */

static unsigned Exception(void *handle, ARMword vector, ARMword pc)
{
  IGNORE(handle); IGNORE(vector); IGNORE(pc);
  /* don't use this here */
  return FALSE;
}
#endif

/*
 * Unwind a data abort
 */

static void UnwindDataAbort(ARMul_State *state, ARMword addr, ARMword iset)
{
#ifndef CODE16
  IGNORE(iset);
#else
  if (iset == INSTRUCTION16) {
    ARMword instr = ARMul_ReadWord(state, addr);
    ARMword offset;
    unsigned long regs;
    if (state->bigendSig ^ ((addr & 2) == 2))
      /* get instruction into low 16 bits */
      instr = instr >> 16;
    switch (BITS(11,15)) {
    case 0x16:
    case 0x17:
      if (BITS(9,10) == 2) { /* push/pop */
        regs = BITS(0, 8);
        offset = 0;
        for (; regs != 0; offset++)
          regs ^= (regs & -regs);
        /* pop */
        if (BIT(11))
          ARMul_SetReg(state, CURRENTMODE,13,
                       ARMul_GetReg(state, CURRENTMODE,13) - offset*4);
        else
          ARMul_SetReg(state, CURRENTMODE,13,
                       ARMul_GetReg(state, CURRENTMODE,13) + offset*4);
      }
      break;
    case 0x18:
    case 0x19:
      regs = BITS(0,7);
      offset = 0;
      for (; regs != 0; offset++)
        regs ^= (regs & -regs);
      /* pop */
      if (BITS(11,15) == 0x19) { /* ldmia rb! */
        ARMul_SetReg(state, CURRENTMODE,BITS(8,10),
                     ARMul_GetReg(state, CURRENTMODE,BITS(8,10)) - offset*4);
      } else {          /* stmia rb! */
        ARMul_SetReg(state, CURRENTMODE,BITS(8,10),
                     ARMul_GetReg(state, CURRENTMODE,BITS(8,10)) + offset*4);
      }
      break;
    default:
      break;
    }
  } else
#endif
  {
    ARMword instr = ARMul_ReadWord(state, addr);
    ARMword rn = BITS(16, 19);
    ARMword itype = BITS(24, 27);
    ARMword offset;

    if (rn == 15) return;
    if (itype == 8 || itype == 9) {
      /* LDM or STM */
      unsigned long regs = BITS(0, 15);
      offset = 0;
      if (!BIT(21)) return; /* no wb */
      for (; regs != 0; offset++)
        regs ^= (regs & -regs);
      if (offset == 0) offset = 16;
    } else if (itype == 12 || /* post-indexed CPDT */
               (itype == 13 && BIT(21))) { /* pre_indexed CPDT with WB */
      offset = BITS(0, 7);
    } else
      return;
    
    /* pop */
    if (BIT(23))
      ARMul_SetReg(state,CURRENTMODE,rn,
                   ARMul_GetReg(state,CURRENTMODE,rn) - offset*4);
    else
      ARMul_SetReg(state,CURRENTMODE,rn,
                   ARMul_GetReg(state,CURRENTMODE,rn) + offset*4);
  }
}

/*
 * Copy a string from the debuggee's memory to the host's
 */

static void getstring(ARMul_State *state, ARMword from, char *to)
{
  do {
    *to = (char)ARMul_ReadByte(state,from++);
  } while (*to++ != '\0') ;
}

#ifdef NEW_OS_INTERFACE
#  define STUB ARMul_ModelStub
#else
#  define STUB ARMul_OSStub
#endif

#ifdef NO_ANGEL
const STUB ARMul_Demon = {
  Init,
  (tag_t)"Demon"
  };
#else
const STUB ARMul_Demon = {
  DemonInit,
  (tag_t)"Demon"
  };

const STUB ARMul_Angel = {
  AngelInit,                    /* init */
  (tag_t)"Angel"                /* handle */
  };
#endif
