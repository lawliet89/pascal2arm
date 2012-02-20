/* ebsa110.c - Fast ARMulator memory interface. */
/* Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved. */

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1997/03/24 18:54:19 $
 * Revising $Author: mwilliam $
 */

#include <string.h>             /* for memset */
#include "armdefs.h"
#include "armcnf.h"
#ifndef COMPILING_ON_WINDOWS
# ifndef COMPILING_ON_MSDOS
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <sys/termios.h>
# endif
#endif
#define ModelName (tag_t)"EBSA110"

typedef struct {
  int bigendSig;
  unsigned int sa_memacc_flag;
  ARMul_Cycles cycles;
  void *config_change, *interrupt, *exit;
  double clk;
  ARMul_State *state;
  unsigned int pit_timer[3] ;
  unsigned int pit_timer_mask ;
  unsigned8 pit_ctlw ;
} toplevel_t;

/*
 * ARMulator callbacks
 */

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  toplevel_t *top=(toplevel_t *)handle;
  IGNORE(old);
  top->bigendSig=((new & MMU_B) != 0);
}

static void Interrupt(void *handle,unsigned int which)
{
  toplevel_t *top=(toplevel_t *)handle;
  if (which & (1<<2)) {         /* reset */
    top->cycles.NumNcycles=0;
    top->cycles.NumScycles=0;
    top->cycles.NumIcycles=0;
    top->cycles.NumCcycles=0;
    top->cycles.NumFcycles=0;
  }
}

/*
 * Initialise the memory interface
 */

static double range(ARMword value,char **mult)
{
  static char *multiplier[]={ "", "k", "M", "G" };
  double v=value;
  int i=0;

  while (v>=100.0 && i<4) v/=1000.0,i++;

  *mult=multiplier[i];
  return v;
}

/*
 * Predeclare the memory access functions so that the initialise function
 * can fill them in
 */
static int EBSA110MemAccessBL(void *,ARMword,ARMword *,ARMul_acc);
static unsigned int DataCacheBusy(void *);
static void MemExit(void *);
static unsigned long ReadClock(void *handle);
static const ARMul_Cycles *ReadCycles(void *handle);
static unsigned long GetCycleLength(void *handle);

static ARMul_Error EBSA110MemInit(ARMul_State *state,
                                  ARMul_MemInterface *interf,
                                  ARMul_MemType type,
                                  toolconf config);


ARMul_MemStub ARMul_EBSA110 = {
  EBSA110MemInit,
  ModelName
  };


/************************************************************************/
/* EBSA-110 specific code and definitions                               */
/************************************************************************/
/*----------------------------------------------------------------------*/
/* EBSA-110 Memory map                                                  */
/*----------------------------------------------------------------------*/
# ifndef SRAM_KBYTES
#  define SRAM_KBYTES (128)
# endif
# ifndef DRAM_MBYTES
#  define DRAM_MBYTES (8)
# endif
# ifndef PAGESIZE
# define PAGESIZE 12
# endif

typedef struct {
  unsigned char b[1<<PAGESIZE];
} page_t;

typedef page_t *page_table_t ;

#define DRAM_PAGES (DRAM_MBYTES<<(20-PAGESIZE))
static page_table_t *dram ;

#define SRAM_PAGES (SRAM_KBYTES>>(PAGESIZE-10))
static page_table_t *sram ;

#define EPROM_PAGES (512>>(PAGESIZE-10))
static page_table_t *eprom;

#define FLASH_PAGES (1024>>(PAGESIZE-10))
static page_table_t *flash;

#define Page(block,addr,pages) ((block)[(((addr)>>PAGESIZE) & ((pages)-1))])
#define Offset(addr) ((addr) & ((1<<PAGESIZE)-1))
#define Undummy(block,addr,pages) \
  if (Page(block,addr,pages)==NULL) \
    Page(block,addr,pages)=(page_t *)calloc(1,sizeof(page_t))

#define __Write(block,addr,pages,byte) { \
  page_t *p=Page(block,addr,pages); \
  if (p==NULL) p=Page(block,addr,pages)=(page_t *)calloc(1,sizeof(page_t)); \
  p->b[Offset(addr)]=(byte); \
  }

static ARMword __Read(page_table_t *block, ARMword address, int pages)
{
  page_t *p=Page(block,address,pages); 

  if (p==NULL) 
    return 0 ; 
  else 
    return p->b[Offset(address)] ; 
}


/*----------------------------------------------------------------------*/
/* EBSA-110 UART and I/O reads/writes                                   */
/*----------------------------------------------------------------------*/

unsigned char fiq_mask;
unsigned char fiq_int_list; /* Does not exist on real board */
unsigned char irq_mask;
unsigned char irq_int_list; /* Does not exist on real board */

static int in_fd1=0;
static int out_fd1=2;
static int in_fd2=0;
static int out_fd2=2;

/* Serial port control registers */
struct serial_port {
    unsigned char int_enable;
    unsigned char fifo_control;
    unsigned char int_id;
    unsigned char line_control;
    unsigned char modem_control;
    unsigned char divisor_ls;
    unsigned char divisor_ms;
    unsigned char line_status;
    unsigned char modem_status;
    unsigned char scratch;
    unsigned char loop_char;
} COM1, COM2;

static void UARTInit(void)
{
  char *com_fn;
# if !defined(COMPILING_ON_WINDOWS) && !defined(COMPILING_ON_MSDOS)
  char *com_tty=getenv("EBSARMUL_COM1_TTY");

  if(com_tty && (strlen(com_tty) > 0)) {
    struct termios tdesc;

    in_fd1 = out_fd1 = open(com_tty, O_RDWR|O_SYNC, 0x777);
    tcgetattr(in_fd1, &tdesc);
    tdesc.c_iflag = IGNBRK | IGNPAR;
    tdesc.c_oflag = 0;
    tdesc.c_lflag = 0;
    tdesc.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    tdesc.c_cc[VMIN]=0;
    tdesc.c_cc[VTIME]=0;
    tcsetattr(in_fd1, TCSANOW, &tdesc);
  } else {
    com_fn=getenv("EBSARMUL_COM1_SOCKET");
    if(com_fn && (strlen(com_fn) > 0)) {
      int fd_sock;
      struct sockaddr_un address;

      fd_sock=socket(AF_UNIX, SOCK_STREAM, 0);
      address.sun_family = AF_UNIX;
      strcpy(address.sun_path, com_fn);
      bind(fd_sock, &address, strlen(com_fn) + 3);
      listen(fd_sock, 1);
      in_fd1 = out_fd1 = accept(fd_sock, 0, 0);
    }
  }
# endif

  COM1.int_enable = 0;
  COM1.fifo_control = 0;
  COM1.line_control = 0;
  COM1.modem_control = 0;
  COM1.line_status = 0x60;
  COM1.int_id =0x01;
# if !defined(COMPILING_ON_WINDOWS) && !defined(COMPILING_ON_MSDOS)
  com_fn=getenv("EBSARMUL_COM2_FILE");
  if(com_fn && (strlen(com_fn) > 0)) {
    in_fd2=out_fd2=open(com_fn, O_RDWR|O_SYNC, 0x777);
  }
# endif
  COM2.int_enable = 0;
  COM2.fifo_control = 0;
  COM2.line_control = 0;
  COM2.modem_control = 0;
  COM2.line_status = 0x60;
  COM2.int_id =0x01;
}

unsigned char soft = 0 ;

#define SetClearIRQ(top, value) \
{ \
  if ((value) != 0) {\
      ARMul_SetNirq(top->state, LOW); \
  } else {\
      ARMul_SetNirq(top->state, HIGH); \
  } \
}

#define SetClearFIQ(top, value) \
{ \
  if (value != 0) \
      ARMul_SetNfiq(top->state, LOW); \
  else \
      ARMul_SetNfiq(top->state, HIGH); \
}

#define PIT_CNT0                        0xf2000001
#define PIT_CNT1                        0xf2000005
#define PIT_CNT2                        0xf2000009
#define PIT_CTLW                        0xf200000d

static void CheckForInput(toplevel_t *top)
{
  fd_set select_mask;
  struct timeval timeout={0,0};

  /* Check for input */
  FD_ZERO(&select_mask);
  FD_SET(in_fd1, &select_mask);
  FD_SET(in_fd2, &select_mask);
  if(select(FD_SETSIZE, &select_mask, 0, 0, &timeout) > 0) {
    if(FD_ISSET(in_fd1, &select_mask)) {
      COM1.line_status |= 1;
      if((COM1.line_status & 1) == 1) {
        COM1.line_status |=2;
      }
      if((COM1.int_enable & 1) == 1) {
        COM1.int_id = 0x04;
        fiq_int_list |=0x02;
        SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
        irq_int_list |=0x02;
        SetClearIRQ(top, (irq_int_list & irq_mask)) ;
      }
    }
    if(FD_ISSET(in_fd2, &select_mask)){
      if((COM2.line_status & 1) == 1) {
        COM2.line_status |=2;
      }
      COM2.line_status |= 1;
      if( (COM2.int_enable & 1) == 1) {
        COM2.int_id = 0x04;
        fiq_int_list |=0x04;
        SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
        irq_int_list |=0x04;
        SetClearIRQ(top, (irq_int_list & irq_mask)) ;
      }
    }
  }
}

static void InitPitCounter(toplevel_t *top) 
{
  int i ; 

  for (i = 0 ; i < 3; i++) 
    top->pit_timer[i] = 0 ;
  top->pit_ctlw = 0 ;
  top->pit_timer_mask = 0 ;
}

static void SetPitN(toplevel_t *top, int i, unsigned8 data) 
{
  /* Check to see if this timer is already running. */
  if ((top->pit_timer_mask & (1 << i)) != 0) {
    /* yes, it is running, so disable it */
    top->pit_timer_mask &= ~(1 << i) ;
    top->pit_timer[i] = 0 ;
  }

  /* Now take the value and write it to the timer */
  if (top->pit_timer[i] == 0)  {
    top->pit_timer[i] = (unsigned8) data ;
    
    /* clear the interrupt/fiq */
    irq_int_list &= ~((0x10 << (i - 1)));
    SetClearIRQ(top, (irq_int_list & irq_mask)) ;
    fiq_int_list &= ~((0x10 << (i - 1)));
    SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
  } else {
    top->pit_timer[i] |= ((unsigned8) data << 8)  ;
    top->pit_timer_mask |= 1 << i ;
#if 0
    ARMul_DebugPrint(top->state, "*** Setting PIT %d to %d\n",
                     i, top->pit_timer[i]); 
#endif
    top->pit_timer[i] = top->pit_timer[i] ;
  }
}

static void IOWrite(toplevel_t *top, unsigned8 data, ARMword address)
{
  /* I/O space; only some addresses handled */
  if((address & 0xf2000003) == 0xf2000000) {
    /* ISAIO external decode space */ 
    switch(address & 0xf3C00000){
    case 0xf2400000:
      /* SOFT register, only LED bit handled for now */
      if((soft & 0x80) != (data & 0x80)) {
        if((data & 0x80) == 0x80) 
          ARMul_DebugPrint(top->state,
                           "*** LED off ***\n") ;
        else 
          ARMul_DebugPrint(top->state,
                           "*** LED on ***\n") ;
        soft = data;
        break;
      }
      break;
    case 0xf2800000:
      /* FIQ mask */
      fiq_mask = data;
      ARMul_DebugPrint(top->state,
                       "*** Setting FIQ mask to %x\n",data);
      if(data &0x80) fiq_int_list = fiq_int_list & 0x7f;
      break;      
    case 0xf2c00000:
      /* IRQ mask */
      /* use CTB_OS style interrupts */
      irq_mask |= data;
          ARMul_DebugPrint(top->state,
                           "*** Setting IRQ mask to %x\n",data);
      SetClearIRQ(top, (irq_int_list & irq_mask)) ;
      break;
#if EBSARM_ARCH_TRICK_BOX
    case 0xf3800000:
      /* Attempt to blast off a FIQ */
      SARMul_ConsolePrint("*** Setting FIQ counter to %d\n",data);
      x_fiq_cnt = data & 0xff;
      break;
    case 0xf3c00000:
      /* Attempt to blast off an IRQ */
      SARMul_ConsolePrint("*** Setting IRQ counter to %d\n",data); 
      x_irq_cnt = data & 0xff;
      break;
# endif
      }
  } else {
    if ((address | 0xf200000f) == 0xf200000f) {
      switch(address & 0xf200000f) {
/*
 * The Interval timer stuff.
 */
      case PIT_CNT0:
        SetPitN(top, 0, data) ;
        break ;
      case PIT_CNT1:
        SetPitN(top, 1, data) ;
        break ;
      case PIT_CNT2:
        SetPitN(top, 2, data) ;
        break ;
      case PIT_CTLW:
        /* which PIT counter is being selected */
        top->pit_ctlw = (unsigned8) data ;
        if ((top->pit_ctlw & 0xC0) == 0x00) {
          top->pit_timer_mask &= ~0x1 ;
          top->pit_timer[0] = 0 ;
        }
        if ((top->pit_ctlw & 0xC0) == 0x40)  {
          top->pit_timer_mask &= ~0x2 ;
          top->pit_timer[1] = 0 ;
        }
        if ((top->pit_ctlw & 0xC0) == 0x80)  {
          top->pit_timer_mask &= ~0x4 ;
          top->pit_timer[2] = 0 ;
        }
        break;
      }
    }
    if((address & 0xf2000003) == 0xf0000000) {
      /* ISAIO self decode space; note that only the
       * bottom byte lane is used */
      switch((address & 0xffc) >> 2) {
        /* COM 1 addresses */
      case 0x3f8:
        /* COM 1 output goes to com1out */
        if((COM1.line_control & 0x80) == 0) {
          if((COM1.modem_control & 0x10) == 0) {
            write(out_fd1, &data, 1);
            COM1.line_status |= 0x60;
          } else {
            COM1.loop_char = data;
            if((COM1.line_status & 1) == 1) {
              COM1.line_status |=2;
            }
            COM1.line_status |= 1;
            COM1.line_status &= 0x9F;
            if((COM1.int_enable & 1) == 1) {
              COM1.int_id = 0x04;
              fiq_int_list |=0x02;
              fiq_int_list |=0x02;
              SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
              irq_int_list |=0x02;
              SetClearIRQ(top, (irq_int_list & irq_mask)) ;
            }
          }
        } else {
          COM1.divisor_ls = data;
        }
        break;
      case 0x3f9:
        if((COM1.line_control & 0x80) == 0) {
          COM1.int_enable = data;
        } else {
          COM1.divisor_ms = data;
        }
        break;
      case 0x3fa:
        /* FIFO control; ignore */
        break;
      case 0x3fb:
        COM1.line_control = data;
        break;
      case 0x3fc:
        COM1.modem_control = data;
        break;
      case 0x3fd:
        COM1.line_status = data;
        break;
      case 0x3fe:
        COM1.modem_status = data;
        break;
      case 0x3ff:
        COM1.scratch = data;
        break;
        
        /* COM 2 registers */
      
      case 0x2f8:
        /* COM 2 output goes to com2out */
        if((COM2.line_control & 0x80) == 0) {
          if((COM2.modem_control & 0x10) == 0) {
            write(out_fd2, &data, 1);
            COM2.line_status |= 0x60;
          } else {
            COM2.loop_char = data;
            if((COM2.line_status & 1) == 1) {
              COM2.line_status |=2;
            }
            COM2.line_status |= 1;
            COM2.line_status &= 0x9F;
            if((COM2.int_enable & 1) == 1) {
              COM2.int_id = 0x04;
              fiq_int_list |=0x04;
              SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
              irq_int_list |=0x04;
              SetClearIRQ(top, (irq_int_list & irq_mask)) ;
            }
          }
        } else {
          COM2.divisor_ls = data;
        }
        break;
      case 0x2f9:
        if((COM2.line_control & 0x80) == 0) {
          COM2.int_enable = data;
        } else {
          COM2.divisor_ms = data;
        }
        break;
      case 0x2fa:
        /* FIFO control; ignore */
        break;
      case 0x2fb:
        COM2.line_control = data;
        break;
      case 0x2fc:
        COM2.modem_control = data;
        break;
      case 0x2fd:
        COM2.line_status = data;
        break;
      case 0x2fe:
        COM2.modem_status = data;
        break;
      case 0x2ff:
        COM2.scratch = data;
        break;
        
        /* Other addresses to be added */
      }
    }
  }
}

static unsigned8 IORead(toplevel_t *top, ARMword address)
{
  unsigned char line_status_temp ;

  /* I/O space; only some addresses handled */
  if((address & 0xf2000003) == 0xf2000000) {
    /* ISAIO external decode space */ 
    switch(address & 0xf3C00000){
    case 0xf2400000:
      return soft;
    case 0xf3400000: /* trick 5, IRQ_RAW */
      return irq_int_list ;
    case 0xf3000000: /* trick 4, IRQ_MSKD */
      return irq_int_list & irq_mask ;
    case 0xf2c00000: /* trick 3, IRQ_MASK */
      return irq_mask ; 
    }
  } else if((address & 0xf2000003) == 0xf0000000) {
    /* 
     * ISAIO self decode space; note that only the
     * bottom byte lane is used 
     */
    switch((address & 0xffc) >> 2) {
      /* COM 1 addresses */
    case 0x3f8:
      /* COM 1 input comes from in_fd1 */
      if((COM1.line_control & 0x80) == 0) {
        COM1.line_status &= 0xE2;
        /* This should in theory test for other interrupts,
         * but at the moment I only ever generate receive
         * interrupts */
        COM1.int_id = 0x01;
        fiq_int_list &= 0xFD;
        SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
        irq_int_list &= 0xFD;
        SetClearIRQ(top, (irq_int_list & irq_mask)) ;
        if((COM1.modem_control & 0x10) == 0) {
          char ch;
          read(in_fd1,&ch,1);
          return ch;
        } else {
          COM1.line_status |=60;
          return COM1.loop_char;
        }
      } else {
        return COM1.divisor_ls;
      }
    case 0x3f9:
      if((COM1.line_control & 0x80) == 0) 
        return COM1.int_enable;
      else 
        return COM1.divisor_ms;
      break;
    case 0x3fa:
      /* Interrupt id; to be added */
      return COM1.int_id;
    case 0x3fb:
      return COM1.line_control;
    case 0x3fc:
      return COM1.modem_control;
      break;
    case 0x3fd:
      line_status_temp =  COM1.line_status;
      COM1.line_status &= 0xFD;
      return line_status_temp;
      break;
    case 0x3fe:
      return COM1.modem_status;
      break;
    case 0x3ff:
      return COM1.scratch;
      break;
          
      /* COM 2 addresses */
    case 0x2f8:
      /* COM 2 output goes to com2out */
      if((COM2.line_control & 0x80) == 0) {
        COM2.line_status &= 0xE2;
        /* This should in theory test for other interrupts,
         * but at the moment I only ever generate receive
         * interrupts */
        COM2.int_id = 0x01;
        fiq_int_list &= 0xFB;
        SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;
        irq_int_list &= 0xFB;
        SetClearIRQ(top, (irq_int_list & irq_mask)) ; 
        if((COM2.modem_control & 0x10) == 0) {
          char ch;
          read(in_fd2,&ch,1);
          return ch;
        } else {
          COM2.line_status |=60;
          return COM2.loop_char;
        }
      } else {
        return COM2.divisor_ls;
      }
    case 0x2f9:
      if((COM2.line_control & 0x80) == 0) 
        return COM2.int_enable;
      else 
        return COM2.divisor_ms;
      break;
    case 0x2fa:
      /* Interrupt id; to be added */
      return COM2.int_id;
    case 0x2fb:
      return COM2.line_control;
    case 0x2fc:
      return COM2.modem_control;
      break;
    case 0x2fd:
      line_status_temp =  COM2.line_status;
      COM2.line_status &= 0xFD;
      return line_status_temp;
      break;
    case 0x2fe:
      return COM2.modem_status;
      break;
    case 0x2ff:
      return COM2.scratch;
      break;
      /* Other addresses to be added */
    }
  }
  return 0 ;
}

/*----------------------------------------------------------------------*/
/* EBSA-110 Memory reads/writes                                         */
/*----------------------------------------------------------------------*/
static int first_write_done = FALSE ;

static unsigned8 EBSA110Read8(toplevel_t *top, ARMword address)
{
  /* 
   * Read from a block of memory.  At reset the memory is in a 
   * different order (EPROM/FLASH is mapped to start at zero) until the
   * first write happens.
   */

  switch (address & 0xC0000000) {
  case 0x00000000:          /* 31:30 = 00, reset ? EPROM/FLASH : DRAM */
    if (first_write_done) 
      return __Read(dram, address, DRAM_PAGES) ;
    else {
      /* EPROM is in the top half of this block */
      if (address & 0x20000000) 
        return __Read(eprom, address, EPROM_PAGES) ;
      else
        return __Read(flash, address, FLASH_PAGES) ;
    }
    break ;
  case 0x40000000:          /* 31:30 = 01, SRAM */
    return __Read(sram, address, SRAM_PAGES) ;
    break ;
  case 0x80000000:          /* 31:30 = 10, EPROM/FLASH  */
    /* EPROM is in the top half of this block */
    if (address & 0x20000000) 
      return __Read(eprom, address, EPROM_PAGES) ;
    else
      return __Read(flash, address, FLASH_PAGES) ;
    break ;
  case 0xC0000000:          /* 31:30 = 11, I/O */
    return IORead(top, address) ;
    break ;
  }
  return 0;
}

static unsigned16 EBSA110Read16(toplevel_t *top, ARMword address)
{
  unsigned16 result ;

  result = EBSA110Read8(top, address) | (EBSA110Read8(top, address + 1) << 8) ;
  return result ;
}

static unsigned32 EBSA110Read32(toplevel_t *top, ARMword address)
{
  unsigned32 result ;
  int i ; 
  int shift ;

  result = 0 ;
  shift = 0 ;
  for (i = 0 ; i < 4 ; i++) {
    result |= (EBSA110Read8(top, address++) << shift) ;
    shift += 8 ;
  }
  return result ;
}

static void EBSA110Write8(toplevel_t *top, unsigned8 data, ARMword address,
                          int debugger)
{
  /* 
   * Write to a block of memory.  At reset the memory is in a 
   * different order (EPROM/FLASH is mapped to start at zero) until the
   * first write happens.  Hey! This might be it now!
   */

  switch (address & 0xC0000000) {
  case 0x00000000:          /* 31:30 = 00, reset ? EPROM/FLASH : DRAM */
    __Write(dram, address, DRAM_PAGES, data) ;
    break ;
  case 0x40000000:          /* 31:30 = 01, SRAM */
    __Write(sram, address, SRAM_PAGES, data) ;
    break ;
  case 0x80000000:          /* 31:30 = 10, EPROM/FLASH  */
    /* only the debugger can write to ROM */
    if (debugger) {
      /* EPROM is in the top half of this block */
      if (address & 0x20000000) 
        __Write(eprom, address, EPROM_PAGES, data) 
      else
        __Write(flash, address, FLASH_PAGES, data) 
    }
    break ;
  case 0xC0000000:          /* 31:30 = 11, I/O */
    IOWrite(top, data, address) ;
    break ;
  }

  /* If it wasn't the debugger writing to memory, then set the
     first_write_done flag */
  if (!debugger)
    first_write_done = TRUE ;
}

static void EBSA110Write16(toplevel_t *top, unsigned16 data, ARMword address,
                           int debugger)
{
  int i ; 

  for (i = 0 ; i < 2 ; i++) {
    EBSA110Write8(top, data & 0xFF, address++, debugger) ;
    data = data >> 8 ;
  }
}

static void EBSA110Write32(toplevel_t *top, unsigned32 data, ARMword address,
                           int debugger)
{
  int i ; 

  for (i = 0 ; i < 4 ; i++) {
    EBSA110Write8(top, data & 0xFF, address++, debugger) ;
    data = data >> 8 ;
  }
}

static page_table_t *EBSA110AllocatePageTable(int size) 
{
  page_table_t *table ;
  int i ;

  table = (page_table_t *)calloc(size, sizeof(page_table_t *)) ;
  if (table) {
    for (i = 0 ; i < size ; i++) table[i] = (page_t *)NULL ;
  }
  return table ;
}

static int EBSA110MemoryInit(toplevel_t *top)
{
  int i ;

  /* allocate the page tables */
  dram = EBSA110AllocatePageTable(DRAM_PAGES) ;
  if (dram == NULL) return FALSE ;

  sram = EBSA110AllocatePageTable(SRAM_PAGES) ;
  if (sram == NULL) return FALSE ;

  eprom = EBSA110AllocatePageTable(EPROM_PAGES) ;
  if (eprom == NULL) return FALSE ;

  flash = EBSA110AllocatePageTable(FLASH_PAGES) ;
  if (flash == NULL) return FALSE ;

  /* initialise the PIT timer */
  InitPitCounter(top) ;

  /* return successfully */
  return TRUE ;
}

static void FreePageTable(page_table_t *table, int size) 
{
  int i ;

  for (i = 0 ; i < size ; i++) {
    if (table[i] != NULL) {
      free(table[i]) ;
      table[i] = NULL ;
    }
  }
  free(table) ;
}

static void EBSA110MemoryExit(void)
{
  if (dram != NULL) {
    FreePageTable(dram, DRAM_PAGES) ;
    dram = NULL ;
  }
  if (sram != NULL) {
    FreePageTable(sram, SRAM_PAGES) ;
    sram = NULL ;
  }
  if (eprom != NULL) {
    FreePageTable(eprom, EPROM_PAGES) ;
    eprom = NULL ;
  }
  if (flash != NULL) {
    FreePageTable(flash, FLASH_PAGES) ;
    flash = NULL ;
  }
}

/************************************************************************/
/* StrongARM trick box specfic code                                     */
/************************************************************************/
/************************************************************************/
/* ARM model interface specfic code                                     */
/************************************************************************/

static ARMul_Error EBSA110MemInit(ARMul_State *state,
                     ARMul_MemInterface *interf,
                     ARMul_MemType type,
                     toolconf config)
{
  ARMword clk_speed;
  const char *option;
  toplevel_t *top;

  interf->read_clock=ReadClock;
  interf->read_cycles=ReadCycles;

  /* Fill in my functions */
  switch (type) {
  case ARMul_MemType_ByteLanes:
    interf->x.basic.access=EBSA110MemAccessBL;
    interf->x.basic.get_cycle_length=GetCycleLength;
    ARMul_PrettyPrint(state,", EBSA-110 byte-laned");
    break ;
  case ARMul_MemType_StrongARM:
  case ARMul_MemType_Basic:
  case ARMul_MemType_16Bit:
  case ARMul_MemType_BasicCached:
  case ARMul_MemType_16BitCached:
  case ARMul_MemType_ARM8:
  case ARMul_MemType_Thumb:
  case ARMul_MemType_ThumbCached:
  default:
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }

  top=(toplevel_t *)malloc(sizeof(toplevel_t));
  if (top == NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  option=ToolConf_Lookup(config,ARMulCnf_MCLK);
  clk_speed=option ? ToolConf_Power(option,FALSE) : 33000000;

  top->clk=1000000.0/clk_speed; /* in microseconds */

  /* only report the speed if "CPU Speed" has been set in the config */
  if (ToolConf_Lookup(config, Dbg_Cnf_CPUSpeed) != NULL) {
    char *fac;
    double clk=range(clk_speed,&fac);
    ARMul_PrettyPrint(state,", %.2f%sHz",clk,fac);
  }

  /* initialise the page maps */
  if (!EBSA110MemoryInit(top)) {
    EBSA110MemoryExit() ;
    return  ARMul_RaiseError(state,ARMulErr_OutOfMemory);
  }

  UARTInit();
  /* initialise any local variables that we care about. */
  fiq_mask = 0 ;
  fiq_int_list = 0 ; 
  irq_mask = 0 ;
  irq_int_list = 0 ; 

  ARMul_PrettyPrint(state, ", 4GB");

  top->cycles.NumNcycles=0;
  top->cycles.NumScycles=0;
  top->cycles.NumIcycles=0;
  top->cycles.NumCcycles=0;
  top->cycles.NumFcycles=0;

  {
    unsigned long memsize=0;
    option=ToolConf_Lookup(config,Dbg_Cnf_MemorySize);
    if (option) memsize=ToolConf_Power(option,TRUE);
    else memsize=0x80000000;
    ARMul_SetMemSize(state,memsize);
  }

  top->state = state ;

  top->config_change=ARMul_InstallConfigChangeHandler(state,ConfigChange,top);
  top->interrupt=ARMul_InstallInterruptHandler(state,Interrupt,top);
  top->exit=ARMul_InstallExitHandler(state,MemExit,top);

  interf->handle = (void *) top;
  return ARMulErr_NoError;

}

/*
 * Remove the memory interface
 */

static void MemExit(void *handle)
{
  toplevel_t *top=(toplevel_t *)handle;

  /* free all truly allocated pages */
  EBSA110MemoryExit() ;

  /* free top-level structure */
  free(top);

  return;
}

/*
 * Memory access function that supports byte lanes
 */

static int EBSA110MemAccessBL(void *handle,
                       ARMword address,
                       ARMword *data,
                       ARMul_acc acc)
{
  toplevel_t *top=(toplevel_t *)handle;
  int debugger=!acc_ACCOUNT(acc);

  /*
   * Clock cycles
   */
  if (acc_ACCOUNT(acc)) {
    int i ; 

    /* count some cycles */
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) 
        top->cycles.NumScycles++;
      else 
        top->cycles.NumCcycles++;
    } else if (acc_MREQ(acc)) top->cycles.NumNcycles++;
    else {
      /* It is an instruction cycle */
      top->cycles.NumIcycles++;

      /* Handle the PIT timers if they are running */
      for (i = 1 ; i < 3; i++) {
        if ((top->pit_timer_mask & (1 << i)) != 0) {
          top->pit_timer[i]-- ;
          if (top->pit_timer[i] == 0) {
#if 0
            ARMul_DebugPrint(top->state, "*** PIT %d expired\n",
                             i); 
#endif
            /* raise the interrupt/fiq */
            irq_int_list |= (0x10 << (i - 1));
            SetClearIRQ(top, (irq_int_list & irq_mask)) ;
            fiq_int_list |= (0x10 << (i - 1));
            SetClearFIQ(top, (fiq_int_list & fiq_mask)) ;

            /* Disable the timer running */
            top->pit_timer_mask &= ~(1 << i) ;
          }
        }
      }
    }
  }
  
  /* 
   * Memory accesses 
   */
  if (acc_MREQ(acc)) {

    /* 
     * check for abort access, there's an area of memory
     * that always raises aborts when read from or written to. 
     */
    if (acc_READ(acc)) {
      if ((address >= 0xC0000000) && (address < 0xE0000000))
        return -1 ;
    } else  {
      if ((address >= 0xD0000000) && (address < 0xE0000000))
        return -1 ;
    }

    /* 
     * It isn't an illegal memory location, so let's go
     * read/write it.
     */
    if (acc_BYTELANE(acc)==BYTELANE_MASK) { /* word */
      if (acc_READ(acc)) 
        *data = EBSA110Read32(top, address) ;
      else
        EBSA110Write32(top, *data, address, debugger) ;
    } else {
      unsigned32 mask;
      static const unsigned32 masks[] = {
        0x00000000, 0x000000ff,
        0x0000ff00, 0x0000ffff,
        0x00ff0000, 0x00ff00ff,
        0x00ffff00, 0x00ffffff,
        0xff000000, 0xff0000ff,
        0xff00ff00, 0xff00ffff,
        0xffff0000, 0xffff00ff,
        0xffffff00, 0xffffffff,
      };
      mask=masks[acc_BYTELANE(acc)];
      if (acc_READ(acc)) 
        *data = EBSA110Read32(top, address) & mask ;
      else  {
        int i ;
        unsigned32 byteMask = 0xff ;
        unsigned32 dataCopy = *data ;

        /* Write every byte in the data that can be written */
        for (i = 0 ; i < 4; i++)  {
          if ((mask & byteMask) != 0) 
            EBSA110Write8(top, dataCopy & 0xff, address, debugger) ;
          dataCopy = dataCopy >> 8 ;
          address++ ;
          byteMask = byteMask << 8 ;
        }
      }
    }                           /* internal cycle */
  }
 
  CheckForInput(top) ;

  return 1;
}

/*
 * Utility functions:
 */

static unsigned long ReadClock(void *handle)
{
  /* returns a us count */
  toplevel_t *top=(toplevel_t *)handle;
  double t=((double)(top->cycles.NumNcycles) +
            (double)(top->cycles.NumScycles) +
            (double)(top->cycles.NumIcycles) +
            (double)(top->cycles.NumCcycles))*top->clk;
  return (unsigned long)t;
}

static const ARMul_Cycles *ReadCycles(void *handle)
{
  toplevel_t *top=(toplevel_t *)handle;
  return &(top->cycles);
}

static unsigned int DataCacheBusy(void *handle)
{
  IGNORE(handle);
  return FALSE;
}

static unsigned long GetCycleLength(void *handle)
{
  /* Returns the cycle length in tenths of a nanosecond */
  toplevel_t *top=(toplevel_t *)handle;
  return top->clk*10000.0;
}
