; veneer_d.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.15.6.1 $
; Checkin $Date: 1997/08/21 12:42:57 $
; Revising $Author: ijohnson $

;===========================================================================
;Veneers onto the arith.s functions.
;
;This block should be assembled multiple times, once for each function.
;The possible functions are:
;
;       addsub_s        double precision add and subtract
;       mul_s           double precision multiply
;       div_s           double precision divide
;       fmod_s          implementation of math.h's fmod() [REM]
;       sqrt_s          implementation of math.h's sqrt() [SQRT]

        GET     fpe.s

;===========================================================================

; When veneering these functions we need to be able to convert from double
; to extended on entry and back again on exit. This macro provides the
; conversion function.

; *WARNING* If no ulabel is set then the next instruction is skipped in
; the case of a number that needs normalizing. This is INTENTIONAL, since
; this macro leaves the Z flag set in the case of an uncommon case, but
; might also leave it set in the case of a denorm, so the following,
; conditional, instruction is skipped. It might be better if ulabel weren't
; there at all, just to make it explicit.
; The __fp_norm_opx functions should also do the skipping, rather than
; hacking lr in the fast path. This should be fixed, but for the moment
; I'd rather not be so disgustingly vile.

        MACRO
$label  DoubleToInternal $op,$zlabel,$ulabel
        ASSERT  ($op = 1) :LOR: ($op = 2)
$label  MOVS    tmp,dOP$op.h,LSL #1        ;C:=sign; Z:=exp & frac.top zero
        TEQEQ   dOP$op.l,#0                     ;C unchanged; Z:=value is a zero
        [ "$zlabel" <> ""
          BEQ   $zlabel                         ;Possible early abort
        ]
        MOV     Rtmp,tmp,LSL #DExp_len-1   ;Frac.top in bits 30:11 of mhi
        MOV     dOP$op.h,tmp,LSR #DExp_pos ;Exponent in bits 11:1
        MOV     OP$op.mlo,dOP$op.l,LSL #DExp_len; OP2mhi and 31:11 of OP2mlo
        ORR     OP$op.mhi,Rtmp,dOP$op.l,LSR #DFhi_len+1
                                                ;Fraction in bits 30:0 of
        ADDNE   dOP$op.h,dOP$op.h,#(EIExp_bias - DExp_bias):SHL:1
        MOV     OP$op.sue,dOP$op.h,RRX          ;Recombine sign and exponent
        ORRNE   OP$op.mhi,OP$op.mhi,#EIUnits_bit

        ; Gets here with the *double precision* exponent in the top 11 bits
        ; of tmp. (Exponent<<1+DExp_pos.) We use a sign extended shift to
        ; spot the "maximum exponent case" - leaves us with -1 in tmp.
        MOVS    tmp,tmp,ASR #1+DExp_pos
        ADDEQ   lr,pc,#8        ;Skip two instructions past normalise call
        BEQ     __fp_norm_op$op
        CMP     tmp,#&ffffffff
        [ "$ulabel" <> ""
        BEQ     $ulabel
        ]
        MEND

        MACRO
        InternalToDouble
        BL      __fp_e2d
        TST     a4,#Error_bit
        VReturn EQ
        ORR     a4,a4,#DoubleErr_bit
        B       __fp_veneer_error
        MEND

        MACRO
        Double $name
        IMPORT  __fp_e2d
        IMPORT  __fp_veneer_error
        IMPORT  __fp_norm_op1
        IMPORT  __fp_norm_op2
        MEND

;===========================================================================
; Veneer functions

        [ :DEF: add_s

        CodeArea |FPL$$dadd|
        Double  add

; This is the veneer onto _dadd, _drsb and _dsub.

        Export  _dadd
        IMPORT  __fp_addsub_common
        IMPORT  __fp_addsub_uncommon

        [ :LNOT: :DEF: thumb

        Export  _drsb
        Export  _dsub

_drsb

;We negate both arguments. The _dsub entry will then unnegate the second.
;For data-slow's sake I negate the second operand first.

        EOR     dOP2h, dOP2h, #Sign_bit
        EOR     dOP1h, dOP1h, #Sign_bit

        KEEP    |$F_dsub|
|$F_dsub|

_dsub

;For sub we just invert the sign of operand 2 and fall through. That is all
;the AddSubFPE code does anyway, so this is no slower.

        EOR     dOP2h, dOP2h, #Sign_bit

;Fall through to Add

        ]

_dadd   VEnter_16                ; Includes extra ARM entry point

        DoubleToInternal 2,,dadd_uncommon1
        DoubleToInternal 1,,dadd_uncommon

        BL      __fp_addsub_common

dadd_return
        InternalToDouble

dadd_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1      ; Skips next instruction if denorm
dadd_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,dadd_return
        MOV     Rins,#Double_mask
        B       __fp_addsub_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: sub_s :LAND: :DEF: thumb

        CodeArea |FPL$$dsub|
        Double  sub

        Export  _dsub

        IMPORT  _dadd

_dsub   VEnter_16

        EOR     dOP2h, dOP2h, #Sign_bit
        ; Just do a tail call to dadd. In the THUMB world, code density is
        ; king. (The addition skips the LDM on the _dadd entry point, and
        ; is dangerous.)
        B       _dadd+4

        [ {FALSE}
        DoubleToInternal 2,,dsub_uncommon1
        DoubleToInternal 1,,dsub_uncommon

        BL      __fp_addsub_common

dsub_return
        InternalToDouble

dsub_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1      ; Skips next instruction if denorm
dsub_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,dsub_return
        MOV     Rins,#Double_mask
        B       __fp_addsub_uncommon

        ]

        ]

;---------------------------------------------------------------------------

        [ :DEF: rsb_s :LAND: :DEF: thumb

        CodeArea |FPL$$drsb|
        Double  rsb

        Export  _drsb
        IMPORT  _dadd

_drsb   VEnter_16

        EOR     dOP1h, dOP1h, #Sign_bit
        ; Same as above - branch to add code.
        B       _dadd+4

        [ {FALSE}

        DoubleToInternal 2,,drsb_uncommon1
        DoubleToInternal 1,,drsb_uncommon

        BL      __fp_addsub_common

drsb_return
        InternalToDouble

drsb_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1      ; Skips next instruction if denorm
drsb_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,drsb_return
        MOV     Rins,#Double_mask
        B       __fp_addsub_uncommon

        ]

        ]

;---------------------------------------------------------------------------

        [ :DEF: mul_s

        CodeArea |FPL$$dmul|
        Double  mul

        Export  _dmul
        IMPORT  __fp_mult_common
        IMPORT  __fp_mult_uncommon

_dmul   VEnter_16

        DoubleToInternal 2,,dmul_uncommon1
        DoubleToInternal 1,,dmul_uncommon
        
        BL      __fp_mult_common

dmul_return
        InternalToDouble

dmul_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1      ; Skips next instruction if denorm
dmul_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,dmul_return
        MOV     Rins,#Double_mask
        B       __fp_mult_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: div_s

        CodeArea |FPL$$ddiv|
        Double  div

        Export  _ddiv
        IMPORT  __fp_veneer_error
        IMPORT  __fp_div_common
        IMPORT  __fp_div_uncommon

_ddiv   VEnter_16

        DoubleToInternal 2,ddiv_zero2,ddiv_uncommon1
        DoubleToInternal 1,ddiv_zero1,ddiv_uncommon

        MOV     Rins,#Double_mask
        BL      __fp_div_common

ddiv_return
        InternalToDouble
        
ddiv_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1,ddiv_zero3
ddiv_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,ddiv_return
        MOV     Rins,#Double_mask
        B       __fp_div_uncommon

ddiv_zero3
; Op1 is a zero, Op2 is an uncommon non-zero. Op2 is in the converted form.
; Op2 is an infinity if all bits are zero (result is a signed zero). Otherwise
; a quiet NaN/exception.

        ORRS    tmp,OP2mlo,OP2mhi,LSL #1
        BEQ     ddiv_zero1
        MOVS    OP2mhi,OP2mhi,LSL #1
        BPL     ddiv_ivo

; return any old quiet NaN

ddiv_return_qnan
        MOV     dOPh,#-1
        VReturn

ddiv_zero2
; Op2 is a zero. If operand 1 is a zero or a SNaN, this is an invalid
; operation, otherwise it is a divide by zero.

        MOVS    tmp, dOP1h, LSL #1
        TEQEQ   dOP1l, #0                       ; Z <- zero
        BEQ     ddiv_ivo

        MVNS    tmp, tmp, ASR #32-DExp_len-1    ; Z <- QNaN
        VReturn EQ                              ; Return Op1 (QNaN)
        ; tmp==1 and mantissa==0 => Inf (Inf)
        ; tmp==1 and mantissa!=0 => SNaN (IVO)
        TEQ     tmp, #1
        BNE     ddiv_dvz
        ORRS    tmp, dOP1l, dOP1h, LSL #DExp_len+1 ; Z <- zero mantissa (Inf)
        VReturn EQ                              ; Return Op1 (Inf)

ddiv_ivo
        MOV     a4, #IVO_bit:OR:DoubleErr_bit
        B       __fp_veneer_error

ddiv_dvz
        MOV     a4, #DVZ_bit:OR:DoubleErr_bit
        B       __fp_veneer_error

ddiv_zero1
; Op1 is a zero, Op2 is in the extended form, and can't be an "uncommon".

        EOR     dOP1h, dOP1h, OP2sue
        AND     dOP1h, dOP1h, #Sign_bit
        VReturn

        ]

;---------------------------------------------------------------------------

        [ :DEF: rdv_s

        CodeArea |FPL$$drdv|
        Double  rdv

        IMPORT  __fp_rdv_common
        IMPORT  __fp_rdv_uncommon
        IMPORT  __fp_veneer_error
        ;Export  _drdv
        Export  _drdiv

;_drdv
_drdiv  VEnter_16

        DoubleToInternal 2,drdv_zero2,drdv_uncommon1
        DoubleToInternal 1,drdv_dvz,drdv_uncommon

        MOV     Rins,#Double_mask :OR: Reverse
        BL      __fp_rdv_common

drdv_return
        InternalToDouble
        
drdv_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1,drdv_zero1
drdv_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,drdv_return
        MOV     Rins,#Double_mask:OR:Reverse
        B       __fp_rdv_uncommon

drdv_zero1
; Op2 is uncommon, but Op1 is a zero. Return Inf for Op2=Inf, IVO for
; Op2=SNaN or a QNaN for Op2=QNaN

        MOVS    tmp, dOP2h, LSL #DExp_len+1     ; N <- QNaN
        TEQEQ   dOP1l, #0                       ; Z <- Inf
        MOVMIS  tmp, #0                         ; Z <- N
        BNE     drdv_ivo
        MOV     dOP1h, dOP2h                    ; Return a QNaN/Inf
        MOV     dOP1l, dOP2l
        VReturn

drdv_zero2
; Op2 is a zero. If Op1 is a zero or SNaN, this is an invalid operation,
; otherwise it is an appropiately signed zero unless Op1=QNaN

        MOVS    tmp, dOP1h, LSL #1
        TEQEQ   dOP1l, #0                       ; Z <- Op1=0
        BEQ     drdv_ivo
        MVNS    tmp, tmp, ASR #32-DExp_len-1    ; Z <- Op1=QNaN
        VReturn EQ                              ; Return QNaN
        ORRS    dOP1l, dOP1l, dOP1h, LSL #DExp_len+1
                                                ; Z <- zero mantissa
        BEQ     drdv_return_zero
        TEQ     tmp, #1                         ; Z <- SNaN
        BEQ     drdv_ivo

drdv_return_zero
        EOR     dOP1h, dOP1h, dOP2h
        AND     dOP1h, dOP1h, #Sign_bit
        MOV     dOP1l, #0
        VReturn

drdv_dvz
        MOV     a4,#DVZ_bit:OR:DoubleErr_bit
        B       __fp_veneer_error

drdv_ivo
        MOV     a4,#IVO_bit:OR:DoubleErr_bit
        B       __fp_veneer_error

        ]

;---------------------------------------------------------------------------

        [ :DEF: fmod_s

        CodeArea |FPL$$dfmod|
        Double  fmod

        EXPORT  fmod
        Import_32 __fp_edom

fmod    VEnter

        DoubleToInternal 2,fmod_divide_by_zero,fmod_uncommon1
        DoubleToInternal 1,fmod_Op1Zero,fmod_uncommon

        BL      Rem_Common

fmod_return
        InternalToDouble

fmod_Op1Zero

; Op1 is zero => result is Op1. Op1h/Op1l hasn't been changed.

        VReturn

fmod_uncommon1
        ORR     OP2sue,OP2sue,#Uncommon_bit
        DoubleToInternal 1
fmod_uncommon
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,fmod_return
        MOV     Rins,#Double_mask
        B       Rem_Uncommon

fmod_divide_by_zero
        ; We return -HUGE_VAL and set errno=EDOM

        VPull
        MOV     a1, #Sign_bit
        MOV     a2, #1          ; true
        B_32    __fp_edom

        GET     arith.s

        ]
        
;---------------------------------------------------------------------------

        [ :DEF: sqrt_s

        CodeArea |FPL$$dsqrt|
        Double  sqrt

        EXPORT  sqrt
        IMPORT  __fp_sqrt_common
        IMPORT  __fp_sqrt_uncommon

sqrt    VEnter

        DoubleToInternal 1,sqrt_Zero,sqrt_uncommon

        MOV     Rins,#Double_mask
        BL      __fp_sqrt_common

sqrt_return
        BL      __fp_e2d

        TST     a4, #Error_bit
        VReturn EQ

; error - set errno to EDOM and return -HUGE_VAL

        MOV     a1, #Sign_bit
        MOV     a2, #1          ; something non-zero
sqrt_edom
        Import_32 __fp_edom
; tail call
        VPull
        B_32    __fp_edom

sqrt_Zero
        ; C contains the sign bit - if set, record a domain error,
        ; but return -0.0 (which is what's in a1/a2 already)
      [ :DEF: SqrtMinusZeroGivesEDOM
        VReturn CC
        B       sqrt_edom       ; save a few bytes in error case
      |
        ; Otherwise, just return the zero passed in
        VReturn
      ]

sqrt_uncommon
        ORR     OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,sqrt_return
        MOV     Rins,#Double_mask
        B       __fp_sqrt_uncommon

        ]

;===========================================================================

        END
