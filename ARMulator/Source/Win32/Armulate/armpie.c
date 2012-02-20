/*
 * armpie.c
 * Platform Independant Evaluation Card Model.
 * Copyright (C) 1991,96 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.13 $
 * Checkin $Date: 1997/03/20 17:36:49 $
 * Revising $Author: mwilliam $
 */

#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>

#include <unistd.h>
#include "armdefs.h"
#include "armcnf.h"

#define ModelName (tag_t)"PIE"

#define LINEBUFFERSIZE 1024
#define CHECKSERIALSOON 0
#define CHECKSERIALEVERY 1000
#define CLOCKSPERSECOND 1000000

struct MemInfo {
  char *rom;
  char *ram;

  int HaveReset; /* a write to the RAMAREA has happened */
  
  /* first the EXP data */
  
  int Flash;
  int UDCaddr;
  int UDCdata[16];
  int Control;
  int Display[8];
  
  /* and then the IO data */
  
  int SerialLine;
  int BaudRate;
  unsigned CheckLineRate;
  char LineBuffer[LINEBUFFERSIZE];
  unsigned CharsInBuffer;
  unsigned BufferPos;
  long LastClock;
  unsigned ClockTicks;
  int ReceiveInput;

  ARMul_State *state;

  unsigned int bigendSig;
  unsigned int priv_mode;
};

static ARMword ExpRead(struct MemInfo *info, ARMword address);
static void ExpWrite(struct MemInfo *info, ARMword address, ARMword data);
     
static ARMword IORead(struct MemInfo *info, ARMword address);
static void IOWrite(struct MemInfo *info, ARMword address, ARMword data);
     
static int OpenSerial(struct MemInfo *info);
static void ConfigSerial(struct MemInfo *info, int d, unsigned speed);
static unsigned ReadSerial(struct MemInfo *info, int d, char *block, unsigned len);
static void WriteSerial(struct MemInfo *info, int d, char *block, unsigned len);
static void CloseSerial(struct MemInfo *info, int d);
     
     
#ifdef SMALLADDR
#define AREAMASK        0x03000000
#define RAMAREA         0x00000000
#define EXPAREA         0x01000000
#define IOAREA          0x02000000
#define ROMAREA         0x03000000
#else
#define AREAMASK        0xc0000000
#define RAMAREA         0x00000000
#define EXPAREA         0x40000000
#define IOAREA          0x80000000
#define ROMAREA         0xc0000000
#endif
     
#define DEFRAMSIZE (512 * 1024) /* default memory size, must be a power of 2 */
#define RAMWORDWRAP (DEFRAMSIZE - 4)
#define RAMBYTEWRAP (DEFRAMSIZE - 1)
#define RAMWORDMASK(addr) (addr & RAMWORDWRAP)
#define RAMBYTEMASK(addr) (addr & RAMBYTEWRAP)
     
#define DEFROMSIZE (512 * 1024) /* default memory size, must be a power of 2 */
#define ROMWORDWRAP (DEFROMSIZE - 4)
#define ROMBYTEWRAP (DEFROMSIZE - 1)
#define ROMWORDMASK(addr) (addr & ROMWORDWRAP)
#define ROMBYTEMASK(addr) (addr & ROMBYTEWRAP)
     
#define ENDSWAP(addr) (addr ^ 3)
     
#ifndef HostEndian
static unsigned HostEndian;
#endif
     
#define RAM (info->rom)
#define ROM (info->ram)
#define MEMINFO (info)

/*
 * Remove the memory interface
 */

static void MemoryExit(void *handle)
{
  struct MemInfo *info=(struct MemInfo *)handle;
  close(MEMINFO->SerialLine);
  free((char *)RAM);
  free((char *)ROM);
  free((char *)MEMINFO);
  return;
}

static int MemAccess(void *handle,ARMword address,ARMword *word,
                     ARMul_acc acc)
{
  struct MemInfo *info = (struct MemInfo *)handle;
  int account=acc_ACCOUNT(acc);
  
  if (acc==acc_LoadInstrS) {
    if (info->CheckLineRate == 0) {
      /* get a character from the serial line */
      long temp;
      
      if (info->ReceiveInput) {
        if (info->CharsInBuffer == 0) {
          info->CharsInBuffer = ReadSerial(handle,info->SerialLine,
                                           info->LineBuffer,LINEBUFFERSIZE);
          info->BufferPos = 0;
        }
        if (info->CharsInBuffer > 0) {
          ARMul_SetNirq(info->state,LOW); /* raise the interrupt */
        }
      }
      temp = (clock()/CLOCKSPERSECOND) - info->LastClock;
      if (temp > 0) {
        info->LastClock += temp;
        /* Angel doesn't handle timer interrupts so don't raise. */
        /*       state->Exception = TRUE;*/ /* raise the interrupt */
        /*       state->IRQPin = LOW;*/
        info->ClockTicks += temp;
      }
      
      info->CheckLineRate = CHECKSERIALEVERY;
    } else
      info->CheckLineRate--;
  }
  
  /* access memory */
  if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc_WIDTH(acc)) {
      case BITS_32:
        switch (address & AREAMASK) {
        case RAMAREA :
          if (info->HaveReset || !account) {
            *word=*(ARMword *)(RAM + RAMWORDMASK(address));
            break;
          }
          /* falls through */
        case ROMAREA :
          *word = *(ARMword *)(ROM + ROMWORDMASK(address));
          break;
        case EXPAREA :
          *word = ExpRead(info,address);
          break;
        case IOAREA :
          if (account && acc_OPC(acc) && !info->priv_mode) {
            return -1;
          }
          else
            *word = IORead(info,address);
          break;
        default:
          *word = 0;
          break;
        }
        break;
      case BITS_8:
        switch (address & AREAMASK) {
        case RAMAREA :
          if (info->HaveReset || !account) {
            if (HostEndian == info->bigendSig)
              *word = (ARMword)*(RAM + RAMBYTEMASK(address));
            else
              *word = (ARMword)*(RAM + ENDSWAP(RAMBYTEMASK(address)));
            break;
          }
          /* falls through */
        case ROMAREA :
          if (HostEndian == info->bigendSig)
            *word = (ARMword)*(ROM + ROMBYTEMASK(address));
          else
            *word = (ARMword)*(ROM + ENDSWAP(ROMBYTEMASK(address)));
          break;
        case EXPAREA:
          *word = ExpRead(info,address) & 0xff;
          break;
        case IOAREA :
          *word = IORead(info,address) & 0xff;
          break;
        }
        break;
      default:
        return -1;              /* 16-bit not supported */
      }
    } else {                    /* write */
      switch (acc_WIDTH(acc)) {
      case BITS_32:
        switch (address & AREAMASK) {
        case RAMAREA :
          if (info->HaveReset || !account) {
            *(ARMword *)(RAM + RAMWORDMASK(address)) = *word;
            break;
          } else if (!acc_SEQ(acc)) {
            info->HaveReset = TRUE;
            break;
          }        
          /* fall through */
        case ROMAREA :
          /* only let the debugger write to ROM ! */
          if (!account)
            *(ARMword *)(ROM + ROMWORDMASK(address)) = *word;
          break;
        case EXPAREA :
          ExpWrite(info,address,*word);
          break;
        case IOAREA :
          IOWrite(info,address,*word);
          break;
        }
        break;
      case BITS_8:
        switch (address & AREAMASK) {
        case RAMAREA :
          if (info->HaveReset || !account) {
            if (HostEndian == info->bigendSig)
              *(RAM + RAMBYTEMASK(address)) = (unsigned char)*word;
            else
              *(RAM + ENDSWAP(RAMBYTEMASK(address))) = (unsigned char)*word;
            break;
          }
        case ROMAREA :
          /* let the debugger write to ROM ! */
          if (!account) {
            if (HostEndian == info->bigendSig)
              *(ROM + ROMBYTEMASK(address)) = (unsigned char)*word;
            else
              *(ROM + ENDSWAP(ROMBYTEMASK(address))) = (unsigned char)*word;
          }
          break;
        case EXPAREA :
          ExpWrite(info,address,*word);
          break;
        case IOAREA :
          IOWrite(info,address,*word);
          break;
        }
        break;
      default:
        return -1;              /* 16-bit not supported */
      }
    }
  }
  
/* 
  if (acc_ACCOUNT(acc)) {
    if (acc_SEQ(acc)) {
      if (acc_MREQ(acc)) state->NumScycles++;
      else state->NumIcycles++;
    } else if (acc_MREQ(acc)) state->NumNcycles++;
    else state->NumCcycles++;
  }
*/
  
  return 1;
}

/*
 * Two routines follow that implement the 8 digit 5 x 7 LED display
 * model, and model the two switches. It's fairly simple stuff, see the
 * datasheet for details. The first routine implements Exp reads, the
 * second Exp writes.
 */

static ARMword ExpRead(struct MemInfo *info, ARMword address)
{
  address = address & ~IOAREA;
  switch(address >> 5) {
  case 0x0 : /* Flash Attributes */
  case 0x1 :
  case 0x2 :
  case 0x3 :
    return(info->Flash);
  case 0x4 : /* UDC addr pointer */
    return(info->UDCaddr);
  case 0x5 : /* UDC data pointer */
    return(info->UDCdata[info->UDCaddr]);
  case 0x6 : /* Control word */
    return(info->Control);
  case 0x7 : /* Display pointer */
    return(info->Display[(address >> 2) & 7]);
    default :
      ARMul_DebugPrint(info->state,"Exp Read from address %08lx\n",address);
    return(0);
  }
}

static void ExpWrite(struct MemInfo *info, ARMword address, ARMword data)
{
  address = address & ~EXPAREA;
  switch(address >> 5) {
  case 0x0 : /* Flash Attributes */
  case 0x1 :
  case 0x2 :
  case 0x3 :
    info->Flash = data & 0xff;
  case 0x4 : /* UDC addr pointer */
    info->UDCaddr = data & 0xf;
  case 0x5 : /* UDC data pointer */
    info->UDCdata[info->UDCaddr] = data & 0x1f;
  case 0x6 : /* Control word */
    info->Control = data & 0xff;
  case 0x7 : /* Display pointer */
    info->Display[(address >> 2) & 7] = data;
    default :
      ARMul_DebugPrint(info->state,"Exp Write to address %08lx\n",address);
  }
}

/*
 * Two routines follow that implement the host independant SCC2691 UART
 * model. It's fairly simple stuff, see the datasheet for details. The
 * first routine implements IO reads, the second IO writes.
 */

static ARMword IORead(struct MemInfo *info, ARMword address)
{
  address = address & ~IOAREA;
  switch(address) {
  case 0x00 : /* MR1 and MR2 */
  case 0x08 : /* Reserved */
  case 0x10 : /* Reserved */
  case 0x18 : /* CTU */
  case 0x1c : /* CTL */
    return(0);
  case 0x04 : /* SR */
    if (info->CharsInBuffer)
      return(13);
    else
      return(12);
  case 0x0c : /* RHR */
    if (info->CharsInBuffer) {
      info->CheckLineRate = CHECKSERIALEVERY;
      ARMul_SetNirq(info->state,HIGH);
      info->CharsInBuffer--;
#ifdef RDI_VERBOSE
      ARMul_DebugPrint(info->state, "<%02x ", (info->LineBuffer[info->BufferPos])&0xff);
#endif
      return((ARMword)info->LineBuffer[info->BufferPos++]);
    }
    else
      return(0xdeadbeef);
  case 0x14 : { /* ISR */
    ARMword temp;
    
    if (info->CharsInBuffer)
      temp = 6;
    else
      temp = 2;
    if (info->ClockTicks) {
      info->ClockTicks--;
      return(16 | temp);
    }
    else
      return(temp);
  }
    default :
      ARMul_DebugPrint(info->state,"IO Read from address %08lx\n",address);
    return(0);
  }
}

static void IOWrite(struct MemInfo *info, ARMword address, ARMword data)
{
  address = address & ~IOAREA;
  switch(address) {
  case 0x00 : /* MR1 and MR2 */
  case 0x14 : /* IMR */
  case 0x18 : /* CTUR */
  case 0x1c : /* CTLR */
    break;
  case 0x04 : /* CSR */
  case 0x10 : /* ACR */
    if (address == 0x4)
      info->BaudRate = (info->BaudRate & 0x100) | (data & 0xff);
    else
      info->BaudRate = (info->BaudRate & 0xff) | ((data & 0x80) << 1);
    switch (info->BaudRate) {
      default    :
      case 0x0bb :
      case 0x1bb :
        ConfigSerial(info, info->SerialLine,1);
      break;
    case 0x0cc :
      ConfigSerial(info, info->SerialLine,3);
      break;
    case 0x1cc :
      ConfigSerial(info, info->SerialLine,2);
      break;
    }
    info->CheckLineRate = 1;
    info->CharsInBuffer = 0;
    info->BufferPos = 0;
    break;
  case 0x08 : /* CR */
    if ((data & 0xf0) == 0xa0)
    {  info->ReceiveInput=0;
       /*     ARMul_DebugPrint(state,"=[) ");*/
     }
    else if ((data & 0xf0) == 0xb0)
    {  info->ReceiveInput=1;
       /*          ARMul_DebugPrint(state,"=[# ");*/
     }
    else if ((data & 0xf0) == 0x90 && !info->ClockTicks && info->CharsInBuffer == 0) { /* Reset Timer */
      ARMul_SetNirq(info->state,HIGH);
    }
    break;
  case 0x0c : { /* THR */
    unsigned char mess;
    
    mess = (unsigned char)(data & 0xff);
    WriteSerial(info, info->SerialLine,&mess,1);
    break;
  }
  case 0x200:
    ARMul_DebugPrint(info->state, "%c", data);
    break;
    default :
      ARMul_DebugPrint(info->state,"IO Write to address %08lx\n",address);
  }
}

/*
 * Here is the code that does the host specific work for IO locations
 * We model the SCC2691 UART on top of the host's serial device
 * There is code here to: Open the serial line
 *                        Set the baud rate of the serial line
 *                        Read bytes from the serial line
 *                        Write bytes to the serial line
 *                        Close the serial line
 */

/*
 * Open the serial line.
 */


extern char *angelDevice;

static int OpenSerial(struct MemInfo *info)
{
  int d = -1;
  char *name=angelDevice;
  
  if (name!=NULL) {
#ifdef sun
    if (strcmp(name,"1")==0) name="/dev/ttya";
    if (strcmp(name,"2")==0) name="/dev/ttyb";
#endif
#ifdef __hpux
    if (strcmp(name,"1")==0) name="/dev/tty00";
    if (strcmp(name,"2")==0) name="/dev/tty01";
#endif
    
    d = open(name,O_RDWR | O_NDELAY,0);
  }
  
  if (d < 0) {
    ARMul_DebugPrint(info->state,"Couldn't open serial device %s.\n",
                     name==NULL ? "as none was specified" : name);
    perror("PIE model");
    exit(2);
  }
  return(d);
}

/*
 * Set the serial line speed, and set up its initial state.
 */

static void ConfigSerial(struct MemInfo *info, int d, unsigned speed)
{
  int baud, r;
  struct termios serdev;
  
  r = 1;
  /*
    r = ioctl(d,TCFLSH,TCIOFLUSH);
    if (r < 0) {
    ARMul_DebugPrint(info->state,"Couldn't flush serial device.\n");
    perror("PIE model");
    exit(2);
    }
    */
  r = tcgetattr(d,&serdev);
  if (r < 0) {
    ARMul_DebugPrint(info->state,"Couldn't get serial parameters.\n");
    perror("PIE model");
    exit(2);
  }
  
  if (speed == 2)
    baud = B19200;
  else if (speed == 3)
    baud = B38400;
  else
    baud = B9600;
  
  serdev.c_iflag = IGNBRK | IGNPAR | IXON |IXOFF;
  serdev.c_oflag = 0;
  serdev.c_cflag = baud | CS8 | CREAD | HUPCL | CLOCAL;
  serdev.c_lflag = 0;
  serdev.c_cc[VMIN] = 0;
  serdev.c_cc[VTIME] = 1;
  
  r = tcsetattr(d, TCSAFLUSH,& serdev);
  if (r < 0) {
    ARMul_DebugPrint(info->state,"Couldn't write serial parameters.\n");
    perror("PIE model");
    exit(2);
  }
  
  r = tcflow(d, TCION);
}

/*
 * Read a block of len bytes from the serial line.
 */

static unsigned ReadSerial(struct MemInfo *info, int d, char *block, unsigned len)
{
  int r;
  struct timeval timeout;
  fd_set readfd;
  int status;
  
  timerclear(&timeout);
  FD_ZERO(&readfd);
  FD_SET(d,&readfd);
  status = select(d+1,&readfd,NULL,NULL,&timeout);
  if (status==0) return 0;
  if (status<0) {perror("Error with select:"); return 0;}
  
  if (!FD_ISSET(d,&readfd)) return 0;
  r = read(d,block,len);
  if (r < 0)  {
    ARMul_DebugPrint(info->state,"Couldn't read from the serial line.\n");
    perror("PIE model");
    exit(3);
  }
  
#ifdef RDI_VERBOSE
  { int i;
  for (i = 0; i < r; i++)
    ARMul_DebugPrint(info->state,">%02x ",(unsigned)(*(block + i) & 0xff) );
  }
#endif
  
  return(r);
}

/*
 * Write a block of len bytes to the serial line.
 */

static void WriteSerial(struct MemInfo *info, int d, char *block, unsigned len)
{
  int r;
  
  r = write(d,block,len);
  if (r != (int)len) {
    ARMul_DebugPrint(info->state,"Couldn't write enough to the serial line.\n");
    perror("PIE model");
    exit(3);
  }
  
#ifdef RDI_VERBOSE
  for (r = 0; r < len; r++)
    ARMul_DebugPrint(info->state,">%02x ",(unsigned)(*(block + r) & 0xff) );
#endif
}

/*
 * Close the serial line.
 */

static void CloseSerial(struct MemInfo *info, int d)
{
  IGNORE(info);
  close(d);
}

/*
 * ARMulator callbacks
 */

static void ModeChange(void *handle, ARMword old, ARMword new)
{
  struct MemInfo *info=(struct MemInfo *)handle;
  IGNORE(old);
  info->priv_mode=(new < USER32MODE);
}

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  struct MemInfo *info=(struct MemInfo *)handle;
  IGNORE(old);
  info->bigendSig=((new & MMU_B) != 0);
}

/*
 * Initialise the memory interface
 */
     
static ARMul_Error MemoryInit(ARMul_State *state,ARMul_MemInterface *interf,
                              ARMul_MemType type,toolconf config);

const ARMul_MemStub ARMul_PIE = {
  MemoryInit,
  ModelName
  };

static ARMul_Error MemoryInit(ARMul_State *state,ARMul_MemInterface *interf,
                              ARMul_MemType type,toolconf config)
{
  struct MemInfo *info;

  IGNORE(config);

  if (type!=ARMul_MemType_Basic &&
      type!=ARMul_MemType_BasicCached) {
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }

  interf->read_clock=NULL;
  interf->x.basic.access=MemAccess;
  
  ARMul_PrettyPrint(state,", PIE Model 1.0");
  ARMul_SetMemSize(state,DEFRAMSIZE);

  MEMINFO = (struct MemInfo*)malloc(sizeof(struct MemInfo));
  RAM = (char *)malloc(DEFRAMSIZE);
  ROM = (char *)malloc(DEFROMSIZE);

  if (RAM == NULL || ROM == NULL || MEMINFO == NULL)
    return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  ARMul_InstallModeChangeHandler(state,ModeChange,info);
  ARMul_InstallConfigChangeHandler(state,ConfigChange,info);

#ifndef HostEndian
  *(ARMword *)RAM = 1;
  HostEndian = (*RAM != 1); /* 1 for big endian, 0 for little */
#endif
#ifdef BIGEND
  ARMul_ConfigChange(state,MMU_B,MMU_B); /* set bigendian */
#endif
#ifdef LITTLEEND
  ARMul_ConfigChange(state,MMU_B,0); /* set littleendian */
#endif

  MEMINFO->HaveReset = FALSE;
  
  MEMINFO->SerialLine = OpenSerial(info);
  ConfigSerial(info, MEMINFO->SerialLine,1);
  MEMINFO->CheckLineRate = 1;
  MEMINFO->CharsInBuffer = 0;
  MEMINFO->BufferPos = 0;
  MEMINFO->BaudRate = 0xbb;
  MEMINFO->LastClock = clock()/CLOCKSPERSECOND;
  MEMINFO->ReceiveInput=1;

  MEMINFO->state=state;

  ARMul_PrettyPrint(state, ", %d Kb RAM",DEFRAMSIZE);
  ARMul_InstallExitHandler(state,MemoryExit,MEMINFO);

  interf->handle=MEMINFO;

  return ARMulErr_NoError;
}
