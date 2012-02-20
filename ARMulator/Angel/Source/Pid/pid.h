/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.7 $
 *   $Author: amerritt $
 *     $Date: 1996/10/29 17:10:46 $
 *
 * pid.h - Definition of the PID card
 */

#ifndef pid_h
#define pid_h

/**********************************************************************/

/*
 * macros describing the Reference Microcontroller
 */

/*
 * Base addresses for standard memory-mapped peripherals
 */
#define ICBASE          (0x0A000000)    /* Interrupt Controller Base */
#define CTBASE          (0x0A800000)    /* Counter/Timer Base */
#define RPCBASE         (0x0B000000)    /* Reset and Pause Controller Base */

/*
 * Counter/timer registers
 */
typedef struct {
  int Load;                     /* RW */
  int Value;                    /* RO */
  int Control;                  /* RW */
  int Clear;                    /* WO */
} CT;

#define CT1 ((volatile CT *)CTBASE)
#define CT2 ((volatile CT *)(CTBASE + 0x20))

#define CT1Load         ((volatile int *)CTBASE)
#define CT1Value        ((volatile int *)(CTBASE + 0x04))
#define CT1Control      ((volatile int *)(CTBASE + 0x08))
#define CT1Clear        ((volatile int *)(CTBASE + 0x0C))

#define CT2Load         ((volatile int *)(CTBASE + 0x020))
#define CT2Value        ((volatile int *)(CTBASE + 0x024))
#define CT2Control      ((volatile int *)(CTBASE + 0x028))
#define CT2Clear        ((volatile int *)(CTBASE + 0x02C))

/*
 * Counter/Timer control register bits
 */
#define CTEnable        (0x80)
#define CTPeriodic      (0x40)
#define CTPrescale0     (0x00)
#define CTPrescale4     (0x04)
#define CTPrescale8     (0x08)
#define CTDisable       (0x00)
#define CTCyclic        (0x00)

/*
 * Interrupt Controller registers
 */
#define IRQStatus       ((volatile int *)ICBASE)
#define IRQRawStatus    ((volatile int *)(ICBASE + 0x04))
#define IRQEnable       ((volatile int *)(ICBASE + 0x08))
#define IRQEnableSet    ((volatile int *)(ICBASE + 0x08))
#define IRQEnableClear  ((volatile int *)(ICBASE + 0x0c))
#define IRQSoft         ((volatile int *)(ICBASE + 0x10))

#define IRQ_UNUSED      (0x0001)
#define IRQ_PROGRAMMED  (0x0002)
#define IRQ_COMMSRX     (0x0004)
#define IRQ_COMMSTX     (0x0008)
#define IRQ_TIMER1      (0x0010)
#define IRQ_TIMER2      (0x0020)
#define IRQ_CARDA       (0x0040)
#define IRQ_CARDB       (0x0080)
#define IRQ_SERIALA     (0x0100)
#define IRQ_SERIALB     (0x0200)
#define IRQ_PARALLEL    (0x0400)
#define IRQ_ASBEX0      (0x0800)
#define IRQ_ASBEX1      (0x1000)
#define IRQ_APBEX0      (0x2000)
#define IRQ_APBEX1      (0x4000)
#define IRQ_APBEX2      (0x8000)

#define FIQStatus       ((volatile int *)(ICBASE + 0x100))
#define FIQRawStatus    ((volatile int *)(ICBASE + 0x104))
#define FIQEnable       ((volatile int *)(ICBASE + 0x108))
#define FIQEnableSet    ((volatile int *)(ICBASE + 0x108))
#define FIQEnableClear  ((volatile int *)(ICBASE + 0x10c))
#define FIQSourceIRQ    ((volatile int *)(ICBASE + 0x114))

#define FIQ_UNUSED      (0x0)
#define FIQ_PROGRAMMED  (0x1)
#define FIQ_COMMSRX     (0x2)
#define FIQ_COMMSTX     (0x3)
#define FIQ_TIMER1      (0x4)
#define FIQ_TIMER2      (0x5)
#define FIQ_CARDA       (0x6)
#define FIQ_CARDB       (0x7)
#define FIQ_SERIALA     (0x8)
#define FIQ_SERIALB     (0x9)
#define FIQ_PARALLEL    (0xA)
#define FIQ_ASBEX0      (0xB)
#define FIQ_ASBEX1      (0xC)
#define FIQ_APBEX0      (0xD)
#define FIQ_APBEX1      (0xE)
#define FIQ_APBEX2      (0xF)

/*
 * Reset and Halt registers
 */
#define HaltMode                ((volatile int *)RPCBASE)
#define Identification          ((volatile int *)(RPCBASE + 0x010))
#define ClearResetMap           ((volatile int *)(RPCBASE + 0x020))
#define ResetStatus             ((volatile int *)(RPCBASE + 0x030))
#define ResetStatusClear        ((volatile int *)(RPCBASE + 0x034))

#define RESET_STATUS_POR        (0x01)

/**********************************************************************/

/*
 * macros describing the NISA Bus
 */
#define NISA_BASE       0x0C000000              /* NISA Base address */

#define NISA_IO_BASE    (NISA_BASE)
#define NISA_PCMCIA     (NISA_IO_BASE + 0x7C0) /* PCMCIA control registers */
#define NISA_SER_A      (NISA_IO_BASE + 0x01800000) /* Serial port A */
#define NISA_SER_B      (NISA_IO_BASE + 0x01800020) /* Serial port B */
#define NISA_PAR        (NISA_IO_BASE + 0x01800040) /* Parallel port */

#define NISA_PCMEM      0x0E000000              /* PCMCIA memory base */

#define NISA_TOP        0x10000000              /* Top of NISA Space */

/*
 * Note that the system address perceived by the Vadem controller
 * is different to that used by the ARM.  This is because of the
 * way the APB address lines are attached to the controller chip:
 *
 * P_A (24) ---\
 * .            \---> S_A (23)
 * .
 * P_A (3)  ----\
 * P_A (2)  ---\ \--> S_A (2)
 * P_A (1)  --x \---> S_A (1)
 * P_A (0)  --------> S_A (0)
 *
 * This arrangement is used because of the interaction of the ARM
 * and PCMCIA controller's processing of byte and halfword
 * data. It enables byte accesses and word-aligned halfword
 * acceses to the PCMCIA controller; of course, getting a C compiler
 * to use ARM LDH/STH is another problem in itself...
 *
 * For a given card address, the required ARM address is given by:
 *
 * ARM_ADDR = ((CARD_ADDR div 2) * 4) + (CARD_ADDR MOD 2)
 *
 * or
 *
 * ARM_ADDR = ((CARD_ADDR & ~0x1) << 1) + (CARD_ADDR & 0x1)
 *
 * The data read from the controller also needs adjustment,
 * because both the PCMCIA controller and ARM fiddle with byte
 * ordering. The PCMCIA controller will always present byte data
 * on the bottom byte lane, D[7:0]. The ARM will try to extract
 * data from the appropriate byte lane (and 0 extend it) when
 * doing byte accesses.  This means data cannot be read back as
 * bytes. A full word read access must be performed, but this will
 * still need fixing up as when the ARM does non-word-aligned
 * reads, it will rotate the word so that the corresponding byte
 * is at D[7:0]. If A[0] is '1', the read data must be shifted
 * right by 24 bits to get the correct data. e.g.
 *
 * ARM_BYTE = ((ARM_ADDR & 0x1) ? (RAW_DATA >> 24) : (RAW_DATA & 0xFF))
 *
 */
#define PCMCIA_CALC_ADDR(x)     ((((x) & ~0x01) << 1) | ((x) & 0x01))
#define PCMCIA_READ_RAW(x, y)   (*(volatile unsigned int *) \
                                 (PCMCIA_CALC_ADDR(x) | (y)))

#define PCMCIA_WRITE_RAW(x, y, z)       (*(volatile unsigned char *) \
                                 (PCMCIA_CALC_ADDR(x) | (y)) = (z))

#define PCMCIA_MEM_READ(x)      ((((x) & 0x01) == 0) ? \
                                 PCMCIA_READ_RAW(x, NISA_PCMEM) & 0xff : \
                                 PCMCIA_READ_RAW(x, NISA_PCMEM) >> 24)

#define PCMCIA_MEM_WRITE(x, y)  PCMCIA_WRITE_RAW(x, NISA_PCMEM, y)

/*
 * I/O addresses are pre-cooked to take care of the
 * peculiarities of the PCMCIA addressing scheme
 */
#define PCMCIA_IO_READ(x)       ((((x) & 0x01) == 0) ?\
                                 (*(volatile unsigned int *)(x)) & 0xff : \
                                 (*(volatile unsigned int *)(x)) >> 24)
#define PCMCIA_IO_WRITE(x, y)   ((*(volatile unsigned char *)(x)) = (y))

/*
 * Layout of PCMCIA Attribute Memory.  Each socket is given a chunk
 * of attribute memory at a given offset.
 *
 * Note that the controller does not allow memory to be mapped into
 * the bottom 64K of memory space.
 */
#define MappedMemoryBase        0x10000         /* 64K */
#define AttributeMemorySlot     0x01000         /* 4K */

#define AttribMemory_A          MappedMemoryBase
#define AttribMemory_B          AttribMemory_A + AttributeMemorySlot

/*
 * Serial Port A
 */
#define SerA_RHR ((volatile int *)(NISA_SER_A + 0x00))  /* Rx holding data */
#define SerA_THR ((volatile int *)(NISA_SER_A + 0x00))  /* Tx holding data */
#define SerA_IER ((volatile int *)(NISA_SER_A + 0x04))  /* Interrupt enable */
#define SerA_FCR ((volatile int *)(NISA_SER_A + 0x08))  /* Fifo control */
#define SerA_ISR ((volatile int *)(NISA_SER_A + 0x08))  /* Interrupt status */
#define SerA_LCR ((volatile int *)(NISA_SER_A + 0x0C))  /* Line control */
#define SerA_MCR ((volatile int *)(NISA_SER_A + 0x10))  /* Modem control */
#define SerA_LSR ((volatile int *)(NISA_SER_A + 0x14))  /* Line status */
#define SerA_MSR ((volatile int *)(NISA_SER_A + 0x18))  /* Modem status */
#define SerA_SPR ((volatile int *)(NISA_SER_A + 0x1C))  /* Scratchpad */
#define SerA_DLL ((volatile int *)(NISA_SER_A + 0x00))  /* LSB divisor latch */
#define SerA_DLM ((volatile int *)(NISA_SER_A + 0x04))  /* MSB divisor latch */

/*
 * Serial Port B
 */
#define SerB_RHR ((volatile int *)(NISA_SER_B + 0x00))  /* Rx holding data */
#define SerB_THR ((volatile int *)(NISA_SER_B + 0x00))  /* Tx holding data */
#define SerB_IER ((volatile int *)(NISA_SER_B + 0x04))  /* Interrupt enable */
#define SerB_FCR ((volatile int *)(NISA_SER_B + 0x08))  /* Fifo control */
#define SerB_ISR ((volatile int *)(NISA_SER_B + 0x08))  /* Interrupt status */
#define SerB_LCR ((volatile int *)(NISA_SER_B + 0x0C))  /* Line control */
#define SerB_MCR ((volatile int *)(NISA_SER_B + 0x10))  /* Modem control */
#define SerB_LSR ((volatile int *)(NISA_SER_B + 0x14))  /* Line status */
#define SerB_MSR ((volatile int *)(NISA_SER_B + 0x18))  /* Modem status */
#define SerB_SPR ((volatile int *)(NISA_SER_B + 0x1C))  /* Scratchpad */
#define SerB_DLL ((volatile int *)(NISA_SER_B + 0x00))  /* LSB divisor latch */
#define SerB_DLM ((volatile int *)(NISA_SER_B + 0x04))  /* MSB divisor latch */

/*
 * Parallel Port
 */
#define ParPR    ((volatile int *)(NISA_PAR + 0x00))    /* Data */
#define ParIO    ((volatile int *)(NISA_PAR + 0x04))    /* I/O Select */
#define ParStat  ((volatile int *)(NISA_PAR + 0x04))    /* Status */
#define ParCon   ((volatile int *)(NISA_PAR + 0x08))    /* Control */
#define ParCom   ((volatile int *)(NISA_PAR + 0x08))    /* Command */

/*
 * The PCMCIA Card interface has several registers,
 * accessed by an indirect addressing mechanism
 */
#define PCIndex  ((volatile char *)(NISA_PCMCIA + 0x0)) /* Index */

#define PCDataW  ((volatile char *)(NISA_PCMCIA + 0x1)) /* Data (writes) */
#define PCDataR  ((volatile int *)(NISA_PCMCIA + 0x1))  /* Data (reads) */

/*
 * Socket offsets
 */
#define PCSocketA       0x0
#define PCSocketB       0x40

/* General Setup Registers */
#define PCIdRev         0x0     /* Identification & Revision */
#define PCIfStatus      0x1     /* Interface Status */
#define PCPwrRstCtl     0x2     /* Power and RESETDRV Control */
#define PCCStatChng     0x4     /* Card Status Change */
#define PCAddWinEn      0x6     /* Address Window Enable */
#define PCCDGCR         0x16    /* Card Detect and General Control Register */
#define PCGlbCtl        0x1E    /* Global Control Register */

/* Interrupt Registers */
#define PCIntGCtl       0x3     /* Interrupt & General Control */
#define PCStatChngInt   0x5     /* Card Status Change Interrupt Configuration */

/* I/O Registers */
#define PCIOCtl         0x7     /* I/O Control */
#define PCIOA0StartL    0x8     /* I/O Addr 0 Start Low */
#define PCIOA0StartH    0x9     /* I/O Addr 0 Start High */
#define PCIOA0StopL     0xA     /* I/O Addr 0 Stop Low */
#define PCIOA0StopH     0xB     /* I/O Addr 0 Stop High */
#define PCIOA1StartL    0xC     /* I/O Addr 1 Start Low */
#define PCIOA1StartH    0xD     /* I/O Addr 1 Start High */
#define PCIOA1StopL     0xE     /* I/O Addr 1 Stop Low */
#define PCIOA1StopH     0xF     /* I/O Addr 1 Stop High */

/* Memory Registers */
#define PCSysA0MStartL  0x10    /* System Memory Addr 0 Mapping Start Low */
#define PCSysA0MStartH  0x11    /* System Memory Addr 0 Mapping Start High */
#define PCSysA0MStopL   0x12    /* System Memory Addr 0 Mapping Stop Low */
#define PCSysA0MStopH   0x13    /* System Memory Addr 0 Mapping Stop High */
#define PCCMemA0OffL    0x14    /* Card Memory Offset Addr 0 Low */
#define PCCMemA0OffH    0x15    /* Card Memory Offset Addr 0 High */

#define PCSysA1MStartL  0x18    /* System Memory Addr 1 Mapping Start Low */
#define PCSysA1MStartH  0x19    /* System Memory Addr 1 Mapping Start High */
#define PCSysA1MStopL   0x1A    /* System Memory Addr 1 Mapping Stop Low */
#define PCSysA1MStopH   0x1B    /* System Memory Addr 1 Mapping Stop High */
#define PCCMemA1OffL    0x1C    /* Card Memory Offset Addr 1 Low */
#define PCCMemA1OffH    0x1D    /* Card Memory Offset Addr 1 High */

#define PCSysA2MStartL  0x20    /* System Memory Addr 2 Mapping Start Low */
#define PCSysA2MStartH  0x21    /* System Memory Addr 2 Mapping Start High */
#define PCSysA2MStopL   0x22    /* System Memory Addr 2 Mapping Stop Low */
#define PCSysA2MStopH   0x23    /* System Memory Addr 2 Mapping Stop High */
#define PCCMemA2OffL    0x24    /* Card Memory Offset Addr 2 Low */
#define PCCMemA2OffH    0x25    /* Card Memory Offset Addr 2 High */

#define PCSysA3MStartL  0x28    /* System Memory Addr 3 Mapping Start Low */
#define PCSysA3MStartH  0x29    /* System Memory Addr 3 Mapping Start High */
#define PCSysA3MStopL   0x2A    /* System Memory Addr 3 Mapping Stop Low */
#define PCSysA3MStopH   0x2B    /* System Memory Addr 3 Mapping Stop High */
#define PCCMemA3OffL    0x2C    /* Card Memory Offset Addr 3 Low */
#define PCCMemA3OffH    0x2D    /* Card Memory Offset Addr 3 High */

#define PCSysA4MStartL  0x30    /* System Memory Addr 4 Mapping Start Low */
#define PCSysA4MStartH  0x31    /* System Memory Addr 4 Mapping Start High */
#define PCSysA4MStopL   0x32    /* System Memory Addr 4 Mapping Stop Low */
#define PCSysA4MStopH   0x33    /* System Memory Addr 4 Mapping Stop High */
#define PCCMemA4OffL    0x34    /* Card Memory Offset Addr 4 Low */
#define PCCMemA4OffH    0x35    /* Card Memory Offset Addr 4 High */

/* Unique Registers */
#define PCControl       0x38    /* Control */
#define PCActTim        0x39    /* Activity Timer */
#define PCMisc          0x3A    /* Miscellaneous */
#define PCGPIOCon       0x3B    /* GPIO Configuration */
#define PCProgCS        0x3D    /* Programmable Chip Select */
#define PCProgCSCon     0x3E    /* Programmable Chip Select
                                 * Configuration Register */
#define PCATA           0x3F    /* ATA */


/* Define what an ethernet configuration block in the flash looks like */
typedef struct angel_EthernetConfigBlock {
  char block_identifier[4];
  char ip_address[4];
  char netmask[4];
} angel_EthernetConfigBlock;

#define MIN_FLASH_SIZE (128 * 1024)
#define MAX_FLASH_SIZE (2048 * 1024)
#define MIN_SECTOR_SIZE (256)
#define MAX_SECTOR_SIZE (64 * 1024)

#endif /* ndef pid_h */

/* EOF pid.h */
