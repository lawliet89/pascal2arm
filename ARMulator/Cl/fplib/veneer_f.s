; veneer_f.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.12 $
; Checkin $Date: 1995/05/11 13:18:34 $
; Revising $Author: enevill $

;===========================================================================
;Veneers onto the arith.s functions.
;
;This block should be assembled multiple times, once for each function.
;The possible functions are:
;
;       addsub_s        single precision add and subtract
;       mul_s           single precision multiply
;       div_s           single precision divide

        GET     fpe.s

;===========================================================================

; When veneering these functions we need to be able to convert from double
; to extended on entry and back again on exit. This macro provides the
; conversion function.

; **** See caveat/warning in veneer_d.s about treatment of denorms and
; uncommon cases. ****

        MACRO
$label  SingleToInternal $op,$zlabel,$ulabel
        ASSERT  ($op = 1) :LOR: ($op = 2)
$label  MOVS    tmp,fOP$op,LSL #1             ;C:=sign; Z:=exp & frac.top zero
        [ "$zlabel" <> ""
          BEQ   $zlabel                       ;Possible early abort
        ]
        MOV     OP$op.mhi,tmp,LSL #SExp_len-1 ;Frac.top in bits 30:11 of mhi
        MOV     fOP$op,tmp,LSR #SExp_pos      ;Exponent in bits 11:1
        ADDNE   fOP$op,fOP$op,#(EIExp_bias-SExp_bias):SHL:1
        MOV     OP$op.sue,fOP$op,RRX            ;Recombine sign and exponent
        ORRNE   OP$op.mhi,OP$op.mhi,#EIUnits_bit
        MOV     OP$op.mlo,#0

        ; Single precision exponent<<1+SExp_pos is in tmp.
        ; If 0 then this is a denormalised number.
        ; If 1fe then this is an uncommon number.
        MOVS    tmp,tmp,LSR #1+SExp_pos
        ADDEQ   lr,pc,#8        ;Skip two instructions past normalise call
        BEQ     __fp_norm_op$op
        TEQ     tmp,#&ff
        [ "$ulabel" <> ""
        BEQ     $ulabel
        ]

        MEND

        MACRO
        InternalToSingle
        BL      __fp_e2f
        TST     a4,#Error_bit
        VReturn EQ
        ORR     a4,a4,#SingleErr_bit
        B       __fp_veneer_error
        MEND

        MACRO
        Single $name
        IMPORT  __fp_e2f
        IMPORT  __fp_veneer_error
        IMPORT  __fp_norm_op1
        IMPORT  __fp_norm_op2
        MEND

;===========================================================================
; Veneer functions

        [ :DEF: add_s

        CodeArea |FPL$$fadd|
        Single  add

; This is the veneer onto _fadd and _fsub.

        Export  _fadd
        IMPORT  __fp_addsub_common
        IMPORT  __fp_addsub_uncommon

        [ :LNOT: :DEF: thumb
        Export  _fsub
        Export  _frsb

_frsb

;We negate both arguments. The _fsub entry will then unnegate the second.

        EOR     fOP2, fOP2, #Sign_bit
        EOR     fOP1, fOP1, #Sign_bit

        KEEP    |$F_fsub|
|$F_fsub|
_fsub

;For sub we just invert the sign of operand 2 and fall through. That is all
;the AddSubFPE code does anyway, so this is no slower.

        EOR     fOP2, fOP2, #SignBit

;Fall through to Add

        ]

_fadd   VEnter_16

        SingleToInternal 2,,faddsub_uncommon1
        SingleToInternal 1,,faddsub_uncommon

        BL      __fp_addsub_common

faddsub_return
        InternalToSingle

faddsub_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        SingleToInternal 1
faddsub_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,faddsub_return
        MOV     Rins,#Single_mask
        B       __fp_addsub_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: sub_s :LAND: :DEF: thumb

        CodeArea |FPL$$fsub|
        Single  sub

        Export  _fsub
        IMPORT  _fadd
        ;IMPORT  __fp_addsub_common
        ;IMPORT  __fp_addsub_uncommon

_fsub   VEnter_16

        EOR     fOP2, fOP2, #Sign_bit
        ; Code density is king. (+4 is dangerous.)
        B       _fadd+4

        [ {FALSE}

        SingleToInternal 2,,fsub_uncommon1
        SingleToInternal 1,,fsub_uncommon

        BL      __fp_addsub_common

fsub_return
        InternalToSingle

fsub_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        SingleToInternal 1
fsub_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,fsub_return
        MOV     Rins,#Double_mask
        B       __fp_addsub_uncommon

        ]

        ]

;---------------------------------------------------------------------------

        [ :DEF: rsb_s :LAND: :DEF: thumb

        CodeArea |FPL$$frsb|
        Single  rsb

        Export  _frsb
        IMPORT  _fadd
        ;IMPORT  __fp_addsub_common
        ;IMPORT  __fp_addsub_uncommon

_frsb   VEnter_16

        EOR     fOP1,fOP1,#Sign_bit
        ; Code density is king.
        B       _fadd+4

        [ {FALSE}

        SingleToInternal 2,,frsb_uncommon1
        SingleToInternal 1,,frsb_uncommon

        BL      __fp_addsub_common

frsb_return
        InternalToSingle

frsb_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        SingleToInternal 1
frsb_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,frsb_return
        MOV     Rins,#Double_mask
        B       __fp_addsub_uncommon

        ]

        ]

;------------------------------------------------------------------------------

        [ :DEF: mul_s

        CodeArea |FPL$$fmul|
        Single  mul

        Export  _fmul
        IMPORT  __fp_mult_fast_common
        IMPORT  __fp_mult_uncommon

_fmul   VEnter_16

        SingleToInternal 2,,fmul_uncommon1
        SingleToInternal 1,,fmul_uncommon

        BL      __fp_mult_fast_common

fmul_return
        InternalToSingle

fmul_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        SingleToInternal 1
fmul_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,fmul_return
        MOV     Rins,#Single_mask
        B       __fp_mult_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: div_s

        CodeArea |FPL$$fdiv|
        Single  div

        Export  _fdiv
        IMPORT  __fp_div_common
        IMPORT  __fp_div_uncommon
        
_fdiv   VEnter_16

        SingleToInternal 2,fdiv_zero2,fdiv_uncommon1
        SingleToInternal 1,fdiv_zero1,fdiv_uncommon

        MOV     Rins,#Single_mask
        BL      __fp_div_common

fdiv_return
        InternalToSingle

fdiv_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        SingleToInternal 1,fdiv_zero3
fdiv_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,fdiv_return
        MOV     Rins,#Single_mask
        B       __fp_div_uncommon

fdiv_zero3
; Op1 is a zero, Op2 is an uncommon non-zero. Op2 is in the converted form.
; Op2 is an infinity if all bits are zero (result is a signed zero). Otherwise
; a quiet NaN/exception.

        ORRS    tmp,OP2mlo,OP2mhi,LSL #1
        BEQ     fdiv_zero1
        MOVS    OP2mhi,OP2mhi,LSL #1
        BPL     fdiv_ivo

; return any old quiet NaN

fdiv_return_qnan
        MOV     fOP,#-1
        VReturn

fdiv_zero2
; Op2 is a zero. If Op1 is a zero or a SNaN, this is an invalid operation,
; otherwise if Op1 is a QNaN then the result is a QNaN, otherwise it is a
; divide by zero.

        MOVS    tmp, fOP1, LSL #1               ; Z <- zero
        BEQ     fdiv_ivo
        MVNS    tmp, tmp, ASR #32-SExp_len-1    ; Z <- QNaN
        VReturn EQ                              ; Return Op1 (QNaN)
        ; tmp==1 and mantissa==0 => Inf (Inf)
        ; tmp==1 and mantissa!=0 => SNaN (IVO)
        TEQ     tmp, #1
        BNE     fdiv_dvz
        MOVS    a2, fOP1, LSL #SExp_len+1       ; Z <- zero mantissa (Inf)
        VReturn EQ                              ; Return Op1 (Inf)

fdiv_ivo
        MOV     a4, #IVO_bit:OR:SingleErr_bit
        B       __fp_veneer_error

fdiv_dvz
        MOV     a4, #DVZ_bit:OR:SingleErr_bit
        B       __fp_veneer_error

fdiv_zero1
; Op1 is a zero, Op2 is in the extended form, and can't be an "uncommon".

        EOR     fOP1, fOP1, OP2sue
        AND     fOP1, fOP1, #Sign_bit
        VReturn

        ]

;---------------------------------------------------------------------------

        [ :DEF: rdv_s

        CodeArea |FPL$$frdv|
        Single  rdv

        ;Export  _frdv
        Export  _frdiv
        IMPORT  __fp_rdv_common
        IMPORT  __fp_rdv_uncommon
        
;_frdv
_frdiv  VEnter_16

        SingleToInternal 2,frdv_zero2,frdv_uncommon1
        SingleToInternal 1,frdv_dvz,frdv_uncommon

        MOV     Rins,#Single_mask :OR: Reverse
        BL      __fp_rdv_common

frdv_return
        InternalToSingle

frdv_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        SingleToInternal 1,frdv_zero1
frdv_uncommon
        ORREQ   OP1sue, OP1sue, #Uncommon_bit
        ADR     lr, frdv_return
        MOV     Rins, #Single_mask :OR: Reverse
        B       __fp_rdv_uncommon

frdv_zero1
; Op2 is uncommon, but Op1 is a zero. Return Inf for Op2=Inf, IVO for
; Op2=SNaN or a QNaN for Op2=QNaN

        MOVS    tmp, fOP2, LSL #SExp_len+1      ; N <- QNaN  Z <- Inf
        MOVMIS  tmp, #0                         ; Z <- N
        BNE     frdv_ivo
        MOV     fOP1, fOP2                      ; Return a QNaN/Inf
        VReturn

frdv_zero2
; Op2 is a zero. If Op1 is a zero or SNaN, this is an invalid operation,
; otherwise it is an appropiately signed zero unless Op1=QNaN

        MOVS    tmp, fOP1, LSL #1
        BEQ     frdv_ivo
        MVNS    tmp, tmp, ASR #32-SExp_len-1    ; Z <- Op1=QNaN
        VReturn EQ                              ; Return QNaN
        MOVS    a3, fOP1, LSL #SExp_len+1       ; Z <- zero mantissa
        BEQ     frdv_return_zero
        TEQ     tmp, #1                         ; Z <- SNaN
        BEQ     frdv_ivo

frdv_return_zero
        EOR     fOP1, fOP1, fOP2
        AND     fOP1, fOP1, #Sign_bit
        VReturn

frdv_dvz
        MOV     a4, #DVZ_bit:OR:SingleErr_bit
        B       __fp_veneer_error

frdv_ivo
        MOV     a4, #IVO_bit:OR:SingleErr_bit
        B       __fp_veneer_error

        ]

;===========================================================================

        END
