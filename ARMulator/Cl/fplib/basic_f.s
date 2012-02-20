; basic_f.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.8 $
; Checkin $Date: 1995/02/20 16:40:09 $
; Revising $Author: mwilliam $

; Basic floating point functions

        GET     fpe.s

        CodeArea |FPL$$Basic|

        IMPORT  __fp_nonveneer_error

;This code is very similar to the "double" versions. The documentation isn't
;extensively repeated. Refer to basic_d.s for further documentation.

;==============================================================================
;Equality

        [ :DEF: eq_s

        Export _feq

_feq    EnterWithLR_16
        TEQ     fOP1, fOP2
        BEQ     FEQ_CheckNaN
        ORR     tmp, fOP1, fOP2 ;Check for -0=0 - yields -0 if this is the
        TEQ     tmp, #SignBit   ;case. If so then the result should be TRUE
        MOVEQ   a1, #1
        MOVNE   a1, #0
        ReturnToLR

FEQ_CheckNaN
        MOV     tmp, #&ff000000
        CMP     tmp, fOP1, LSL #1
        MOVCS   a1, #1
        MOVCC   a1, #0
        ReturnToLR

        ]

;==============================================================================
;Inequality

        [ :DEF: neq_s

        Export _fneq

_fneq   EnterWithLR_16
        TEQ     fOP1, fOP2
        BEQ     FNEQ_CheckNaN
        ORR     tmp, fOP1, fOP2
        EORS    a1, tmp, #SignBit       ;Same trick as above, only simpler
        ;MOVEQ  a1, #0  ;hack           ;because of the single register
        MOVNE   a1, #1
        ReturnToLR

FNEQ_CheckNaN
        MOV     tmp, #&ff000000
        CMP     tmp, fOP1, LSL #1
        MOVCS   a1, #0
        MOVCC   a1, #1
        ReturnToLR

        ]

;==============================================================================

        MACRO
$label  FloatCompare $lt,$eq
        ;Arguments are what to return if less than (0/1), what to return if
        ;equal. So "a<b" is 1,0 "a>=b" is 0,1

        ASSERT  $lt = 1 :LOR: $lt = 0
        ASSERT  $eq = 1 :LOR: $eq = 0

        IMPORT  __fp_compare_fivo_check_exception

$label  EnterWithLR_16
        MOV     tmp, #&ff000000
        CMP     tmp, fOP1, LSL #1
        CMPCS   tmp, fOP2, LSL #1
        BCC     __fp_compare_fivo_check_exception
        MOVS    fOP1, fOP1, LSL #1
        BEQ     $label._OP1_zero
        BCC     $label._OP1_positive
        MOVS    fOP2, fOP2, LSL #1
        BHI     $label._both_ops_negative
        MOV     a1, #$lt
        ReturnToLR

$label._OP1_zero
        MOVS    fOP2, fOP2, LSL #1
        [ $lt = $eq
        MOVLS   a1,#$lt
        MOVHI   a1,#1-$lt
        |
        MOVCS   a1, #1-$lt
        MOVCC   a1, #$lt
        MOVEQ   a1, #$eq
        ]
        ReturnToLR              ;12/13S + 2N

$label._OP1_positive
        MOVS    fOP2, fOP2, LSL #1
        ;If negative (CS) then less than
        MOVCS   a1, #1-$lt
        ReturnToLR CS           ;12S + 2N

$label._both_ops_positive
        CMP     fOP1, fOP2
        MOVEQ   tmp, fOP1, LSL #8
        CMPEQ   tmp, fOP2, LSL #8
        [ $lt <> $eq
        MOVCC   a1, #$lt
        MOVCS   a1, #1-$lt
        |
        MOVLS   a1, #$lt
        MOVHI   a1, #1-$lt
        ]
        ReturnToLR              ;18S + 2N

$label._both_ops_negative
        CMP     fOP1, fOP2
        MOVEQ   tmp, fOP1, LSL #8
        CMPEQ   tmp, fOP2, LSL #8
        [ $lt = $eq
        MOVCC   a1, #1-$lt
        MOVCS   a1, #$lt
        |
        MOVLS   a1, #1-$lt
        MOVHI   a1, #$lt
        ]
        ReturnToLR              ;16S+2N

        MEND

;==============================================================================
;Less Than

        [ :DEF: ls_s

        Export _fls
_fls    FloatCompare 1,0

        ]

;==============================================================================
;Less Than or Equal

        [ :DEF: leq_s

        Export _fleq
_fleq   FloatCompare 1,1

        ]

;==============================================================================
;Greater Than

        [ :DEF: gr_s

        Export _fgr
_fgr    FloatCompare 0,0

        ]

;==============================================================================
;Greater Than or Equal

        [ :DEF: geq_s

        Export _fgeq
_fgeq   FloatCompare 0,1

        ]

;==============================================================================
;Invalid Operation checking (NaN on compares)
;Called from 32-bit (ARM) code

        [ :DEF: compare_s

        [ :DEF: thumb
        CODE32
        ]
        EXPORT  __fp_compare_fivo_check_exception

__fp_compare_fivo_check_exception
        ;If we get here then one of the two fOPs is a NaN. We know not which.
        ;tmp contains the 0xff000000 constant
        CMP     tmp, fOP1, LSL #1
        BCS     fivo_check_exception2
        TST     fOP1, #fSignalBit
        BEQ     compare_ivo
        CMP     tmp, fOP2, LSL #1
        MOVCS   a1, #0
        ReturnToLR CS
fivo_check_exception2
        ;We know it must be fOP2 by now
        TST     fOP2, #fSignalBit
        MOVNE   a1, #0
        ReturnToLR NE
compare_ivo
        MOV     a4,#IVO_bit:OR:SingleErr_bit
        B       __fp_nonveneer_error

        ]

;==============================================================================

        END
