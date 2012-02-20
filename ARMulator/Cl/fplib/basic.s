; basic.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.7 $
; Checkin $Date: 1995/02/21 17:41:06 $
; Revising $Author: mwilliam $

; Basic floating point functions

        GET     fpe.s

        CodeArea |FPL$$Basic|

;==============================================================================
;Negate
;
;We just invert the sign bit.
;Since the sign bit is in *the same place* for both floats and doubles we
;use the same code.
;
;CANDIDATE FOR COMPILER INLINING

        [ :DEF: thumb
        EXPORT __16_dneg
        EXPORT __16_fneg

__16_dneg
__16_fneg
        MOV     a3, #1
        LSL     a3, #31
        EOR     a1, a3
        BX      lr

        CODE32
        ]

        EXPORT _dneg
        EXPORT _fneg

_dneg
_fneg
        ASSERT  dOPh = a1
        ASSERT  fOP = a1
        EOR     a1, a1, #SignBit
    [ :DEF: INTERWORK
        BX      lr
    |
        [ {CONFIG} = 32
        MOV     pc, lr
        ]
        [ {CONFIG} = 26
        MOVS    pc, lr
        ]
    ]

;==============================================================================
;Absolute
;
;Just clear the sign bit. Only ever called with double - as if it matters.
;
;CANDIDATE FOR COMPILER INLINING.

        [ :DEF: thumb
        CODE16
        ]

        EXPORT fabs
        EXPORT _fabs

fabs
_fabs
        [ :DEF: thumb
        LSL     dOPh, #1
        LSR     dOPh, #1
        BX      lr
        |
        BIC     dOPh, dOPh, #SignBit
    [ :DEF: INTERWORK
        BX      lr
    |
        [ {CONFIG} = 32
        MOV     pc, lr
        ]
        [ {CONFIG} = 26
        MOVS    pc, lr
        ]
    ]
        ]

;==============================================================================

        END
