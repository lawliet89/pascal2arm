/*
 * disass.h:
 * Copyright (C) Advanced RISC Machines Ltd., 1991.
 */

/*
 * RCS $Revision: 1.10 $
 * Checkin $Date: 1997/05/01 09:40:20 $
 * Revising $Author: hmeeking $
 */

#ifndef __disass_h
#define __disass_h

#include "host.h"

typedef enum {
   D_BORBL,
   D_LOADPCREL,   /* LDR with base = pc */
   D_STOREPCREL,  /* STR with base = pc */
   D_LOAD,        /* LDR with base != pc */
   D_STORE,       /* STR with base != pc */
   D_ADDPCREL,    /* ADD r, pc, #n */
   D_SUBPCREL,    /* SUB r, pc, #n */
   D_SWI
} dis_cb_type;

typedef char *dis_cb(dis_cb_type type, int32 offset, unsigned32 address,
                     int width, void *cb_arg, char *buffer);

/* The type of a function passed to disass, which gets called back before
   printing the operand address for various classes of instruction.  If the
   function is able to generate a better representation than the default
   (<number> or <reg>,<number> or [<reg>,<number>] depending on the type)
   it should place it at buffer, and return a pointer to the next character;
   otherwise, it should return the value of buffer.
   cb_arg is the argument handed to disass.
   address is the operand address except for type = D_SWI (undefined), and
   D_LOAD and D_STORE (the base register).
 */

extern void disass_sethexprefix(char const *);
/* set the string to be prefixed to hexadecimal numbers in the disassembler's
   output.  (No copy of the string is made).
   In the absence of a call to this, a prefix of "0x" is used.
 */

extern void disass_setregnames(char const *regnames[], char const *fregnames[]);
/* set the names to be used for the integer and floating point registers
   respectively in instruction disassembly.  If either argument is 0,
   the default names will be used (R0-R14 and PC for integer registers,
   F0-F7 for floating-point registers).
 */

extern unsigned32 disass_16(unsigned32 instr, unsigned32 instr2,
                     unsigned32 address, char *o,
                     void *cb_arg, dis_cb *p);
extern unsigned32 disass_32or26(unsigned32 instr, unsigned32 address, char *o,
                     void *cb_arg, dis_cb *p, int mode_32bit);
extern unsigned32 disass(unsigned32 instr, unsigned32 address, char *o,
                     void *cb_arg, dis_cb *p);
/* place the disassembly of the instruction instr, treated as occurring at
   address address, in the buffer o.  mode_32bit tells it whether addresses
   are 26 or 32 bit (relevant only in constructing operand address for
   branches): disass is just disass_32or26(... 0)
   Returns: The address of the following instruction
 */

typedef enum { CP_DP, CP_RT, CP_DT } Disass_CPOpType;

typedef char *Disass_CoProProc(int cpno, Disass_CPOpType type,
                               uint32 instr, uint32 address,
                               char *o, char *notes);
/* Type of a function to disassemble coprocessor instructions. o points to
 * a buffer to receive the disassembly. If a disassembly is produced, the
 * return value is a pointer to the next free byte in the buffer. Otherwise,
 * it is NULL.
 */

void disass_addcopro(Disass_CoProProc *f);
/* Adds a function to handle disassembly of a coprocessor's instructions
 * When disass() finds a coprocessor instruction, it hands it to the
 * functions thus registered, in reverse order of registration, until
 * one returns a non-NULL result. Failing that, it disassembles the
 * instruction as a generic coprocessor instruction.
 * A handler for FPA instructions is always registered (first, called last).
 */

void disass_deletecopro(Disass_CoProProc *f);

#endif /* disass.h */
