/* models.h - declare all the user-definable extensions to ARMulator.
 * Copyright (c) 1996 Advanced RISC Machines. All Rights Reserved.
 *
 * RCS $Revision: 1.30 $
 * Checkin $Date: 1997/05/09 09:59:45 $
 * Revising $Author: mwilliam $
 */

/* These are the defaults - must be declared first! */
MEMORY(ARMul_Flat)
COPROCESSOR(ARMul_NoCopro)

/* ARMulator memory interfaces */
#ifndef NO_MMULATOR
MEMORY(ARMul_MMUlator)
#endif
MEMORY(ARMul_TracerMem)         /* Instruction/Memory tracer */
#ifdef unix
MEMORY(ARMul_PIE)               /* PIE card model */
MEMORY(ARMul_EBSA110)           /* Model of the EBSA-110 validation card */
#endif
MEMORY(ARMul_MapFile)           /* armsd.map file support */
#ifndef NO_SARMSD
MEMORY(StrongMMU)               /* StrongARM MMU model */
#endif
MEMORY(ARMul_BytelaneVeneer)    /* Bytelane ASIC */
MEMORY(ARMul_TrickBox)          /* Trickbox (validation) ASIC */
MEMORY(ARMul_WatchPoints)       /* Memory model that does watchpoints */

#ifdef ARM9
MEMORY(ARM9Cache)               /* 940 PU model */
#endif

/* Co-Processor bus models */
COPROCESSOR(ARMul_CPBus)
/* Co-Processor models */
COPROCESSOR(ARMul_DummyMMU)

/* Basic models (extensions) */
/*
 * Basic models are initialised in the order in which they appear in
 * this file.
 */
#ifdef NEW_OS_INTERFACE
/* The "WinGlass" module has to be initialised before other models. Until
 * the O/S becomes a model, this has to be done explicitly from inside the
 * ARMulator, so this model does not appear in this header.
 */
#ifdef HOURGLASS_RATE
MODEL(ARMul_WinGlass)
#endif
#endif /* NEW_OS_INTERFACE */
MODEL(ARMul_Profiler)           /* Instruction profiler */
MODEL(ARMul_TracerModel)        /* Instruction tracer */
MODEL(ARMul_Pagetable)          /* Provides page-tables */
MODEL(ARMul_ValidateCP)         /* Vaidation CoProcessor (not OS model too) */

/* Operating System/Monitors */
#ifdef NEW_OS_INTERFACE         /* in the new world, OS's are plain models */
#ifndef NO_ANGEL
MODEL(ARMul_Angel)              /* An operating-system model */
#endif
MODEL(ARMul_Demon)              /* The old Demon debug-monitor */
MODEL(ARMul_ValidateOS)           /* Used for Validation only */
/* MODEL(PIDprint)               * this is only a new-style model */
#else
#ifndef NO_ANGEL
OSMODEL(ARMul_Angel)
#endif
OSMODEL(ARMul_Demon)
OSMODEL(ARMul_ValidateOS)
#endif /* NEW_OS_INTERFACE */

#if defined(PICCOLO) || defined(PicAlpha)
MEMORY(Piccolo)
MEMORY(Piccolo2)
#endif

#if defined(PERIPHERAL_LIB)
MEMORY(ARMul_APB)
#endif
