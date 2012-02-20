; Assembler source for FPA support code and emulator
; ==================================================
; Definitions and default values of optional optimisations. Also used by
; "fplib".
;
; Copyright (C) Advanced RISC Machines Limited, 1992-7. All rights reserved.
;
; RCS $Revision: 1.4 $
; Checkin $Date: 1997/04/22 17:01:08 $
; Revising $Author: dseal $

;===========================================================================

; The "traps never return" code size optimisation.

                GBLL    TrapsCanReturn
TrapsCanReturn  SETL    {TRUE}

; The "FPE context uses 4 words per register" speed optimisation.

                GBLL    FPE4WordsPerReg
FPE4WordsPerReg SETL    {FALSE}

; The "do integer powers" optimisation.

                GBLL    DoIntegerPowers
DoIntegerPowers SETL    {TRUE}

; The value of 0^0.

                GBLS    ZeroToTheZero
ZeroToTheZero   SETS    "One"

; The "FPE checks whether next instruction is floating point" optimisation.

                GBLL    FPEChecksNextInstr
FPEChecksNextInstr SETL {TRUE}

; The "no transcendentals" optimisation.

                GBLL    NoTranscendentals
NoTranscendentals SETL  {FALSE}

; The "no packed precision" optimisation.

                GBLL    NoPacked
NoPacked        SETL    {FALSE}

;===========================================================================

        END
