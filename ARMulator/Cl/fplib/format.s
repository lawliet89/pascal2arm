; format.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.9 $
; Checkin $Date: 1995/02/20 16:37:10 $
; Revising $Author: mwilliam $

        GET     fpe.s

        CodeArea |FPL$$Format|

        IMPORT  __fp_nonveneer_error

SNaNInf EQU     NaNInfExp_Single - EIExp_bias + SExp_bias
DNaNInf EQU     NaNInfExp_Double - EIExp_bias + DExp_bias

;==============================================================================
;Format conversions

        [ :DEF: d2f_s

        Export  _d2f

_d2f    EnterWithLR_16

        ;Double in dOPl/h to a float in fOP
        ;fOP=dOPh
        ;Can use utmp1, utmp2 and tmp as temporaries.

        AND     tmp, dOPh, #Sign_bit    ;Save sign bit
        MOV     utmp2, dOPh, LSL #1     ;Extract exponent
        MOVS    utmp2, utmp2, LSR #32-DExp_len
        BEQ     _d2f_ExponentIsZero

;See if exponent is in range. [1024-127 .. 1024+127] or [&380+1..&480-1]

        SUBS    utmp2, utmp2, #(DExp_bias-SExp_bias)    ;Re-bias exponent
        BMI     _d2f_ExpUnderflow

;MI catches most underflow cases, but not the case where the exponent
;was exactly &380. If it was then the Z flag was set. We could be a
;BEQ here to the underflow handler. However instead we take the
;overflow handler and catch it there (on the slow path).

        CMPNE   utmp2, #SNaNInf
        BGE     _d2f_ExpOverflow

;We only need to round if the top bit of the bits we are going to junk
;is set. This is bit 28.

        TST     dOPl, #1:SHL:(32-DExp_len+SExp_len-1)
        BNE     _d2f_NeedsRounding

;Okay, simple enough now then.

        ORR     tmp, tmp, utmp2, LSL #SExp_pos  ;Merge in sign-bit
        MOV     fOP, dOPh, LSL #DExp_len+1      ;Extract upper fraction
        ORR     fOP, tmp, fOP, LSR #SExp_len+1  ;Merge in exponent/sign
        ORR     fOP, fOP, dOPl, LSR #32-DExp_len+SExp_len ;Merge in rest of fraction

        ReturnToLR                              ;16S+1N

;...........................................................................

_d2f_ExponentIsZero

;Sign is in tmp. dOPh and dOPl haven't been changed. We know that the
;exponent is zero - indicating either a zero or a denormalised number.

;Test for zero. If the high word equals the sign bit, and the low word is
;zero, then this is a zero. What is more, if fOP=dOPh then the zero is
;already in the right register.

        ORRS    utmp2, dOPl, dOPh, LSL #1
        ASSERT  fOP=dOPh

;A denormalised number - underflow - return just the sign flag

        MOVNE   fOP, tmp
        ReturnToLR

_d2f_ExpOverflow

;Sign is in tmp. May still need rounding. The exponent (utmp2) is out of range
;for a single precision number, but may be an infinity/NaN. It can't be a zero
;exponent (handled along a different path). dOPl *hasn't* been changed.

;The exponent has been re-biased.

;Might just be the "just underflowing case" (because of a quirk above). In
;that case utmp2 = 0

        TEQ     utmp2, #0
        BEQ     _d2f_ExpUnderflow

;Need to check first for the NaN/infinity case.

        ADD     utmp1, utmp2, #1
        TEQ     utmp1, #DNaNInf+1-(DExp_bias-SExp_bias)

;See if under or overflow

;IEEE says:
;"Trapped overflow on conversion from a binary floating point format shall
;deliver to the trap handler a result in that or a wider format, possibly
;with the exponent bias adjusted, but rounded to the destination's
;precision."

;We say: "Cause an exception on overflow".

        BNE     _d2f_TakeOverflowTrap

_d2f_NaNOrInf

;Need to check whether it is an infinity, in which case the conversion is
;exact

        MOVS    utmp1, dOPh, LSL #DExp_len+1
        BEQ     _d2f_Inf

;If a Quiet NaN (top bit of that shift was set) return a floating Quiet NaN.
;This does not need to preserve the sign bit.

        MOVMI   fOP, #-1
        ReturnToLR MI

        TEQ     dOPl, #0

;This is a signalling NaN.

        MOVNE   a4,#IVO_bit:OR:SingleErr_bit
        BNE     __fp_nonveneer_error

;Otherwise it is an infinity, so fall through to...

_d2f_Inf

;An infinity. The sign of the resulting infinity is in tmp.
        ASSERT  fOP <> tmp
        MOV     fOP, #SNaNInf
        ORR     fOP, tmp, fOP, LSL #32-SExp_len-1
        ReturnToLR

_d2f_ExpUnderflow

;Underflow. If the value can be represented as a denorm in the target
;precision, then it should be. For this to be the case the exponent needs
;to lie in the range [&380-23..&380]. Otherwise the result is a suitably
;signed zero. (A zero is, however, a denorm with zero mantissa.)

;The exponent (utmp2) has been rebiased (by (DExp_bias-SExp_bias))

        ADDS    utmp1, utmp2, #SFrc_len

;If the result is zero then we round according to the top bit of the
;fraction. Branch out of line to do this.

        BEQ     _d2f_ExpJustUnderflow

;If this result is negative, nothing can be done, so return a signed zero.

        MOVMI   fOP, tmp
        ReturnToLR MI

;utmp1 now contains the (exponent - &380 + 22) value.
;Shift the dOPh exponent into the top of the destination, and then prepare
;a number of bits to shift it down by. If the exponent is &380 in the original
;then it needs to be shifted back by 9. In this case utmp1 is 23, so we
;shift by (32-utmp1). A one (the implicit one in a normalised number) needs to
;be included.

        MOV     dOPh, dOPh, LSL #DExp_len
        ORR     dOPh, dOPh, #(1<<31)

;This gives us 1MMMMMMMMMMMMMMMMMMMM00000000000
;and in dOPl   MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

;Before shifting down again, some bits of dOPl may need to be added in. dOPl
;needs to be merged with dOPh to give:
;               <- 20         OPh -><- OPl 11->
;              1MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM (at least 9 'M's will be lost)
        
        ORR     dOPh, dOPh, dOPl, LSR #32-DExp_len

;This code determines the rounding action. If the bit below the lsb of the
;result is clear (resulting in -ve result) then no action is needed (leaves
;tmp alone). Otherwise we go to the rounding code.

        MOVS    utmp2, dOPh, LSL utmp2
        BMI     _d2f_UnderflowNeedsRounding

;Now we calculate the shift amount and do the shift, probably throwing away
;lots of mantissa bits

        RSB     utmp2, utmp1, #32
        MOV     dOPh, dOPh, LSR utmp2

;Merge in the sign bit and return

        ORR     fOP, dOPh, tmp
        ReturnToLR                      ;No need to do rounding

_d2f_UnderflowNeedsRounding

;We need to determine whether to just round upwards, or to round to even.

        MOVS    utmp2, utmp2, LSL #1    ;C=1, Z=possible round to even.
        TSTEQ   dOPl, #(DNaNInf :AND: &ff00)    ;C=1  Z=definite round to even
        TSTEQ   dOPl, #(DNaNInf :AND: &00ff)

;Now we calculate the shift amount and do the shift, probably throwing away
;lots of mantissa bits

        RSB     utmp2, utmp1, #32
        MOV     dOPh, dOPh, LSR utmp2

;Merge in the sign bit and add one (C flag). Then undo the rounding if we
;need to round to even and the result isn't even.

        ADC     fOP, dOPh, tmp
        TSTEQ   fOP, #1                 ;If round to even, test if even
        SUBEQ   fOP, fOP, #1            ;Undo rounding if RTE and have odd result
        ReturnToLR

_d2f_ExpJustUnderflow

;The exponent is just at the limits of underflow - i.e. we may be able
;to represent the number as the float epsilon.

;       MOVS    dOPh, dOPh, LSR #DFhi_len
;       ADC     fOP, tmp, #0

;But that's not how the FP emulator behaves. It does this:

        MOVS    dOPh, dOPh, LSL #DExp_len+1
        TEQEQ   dOPl, #0
        MOVEQ   fOP, tmp
        ORRNE   fOP, tmp, #1
        ReturnToLR

;...........................................................................

_d2f_NeedsRounding                      ;12S+1N

        MOVS    utmp1, dOPl, LSL #DExp_len-SExp_len+1  ;Z<-round to even

        MOV     dOPl, dOPl, LSR #32-DExp_len+SExp_len

;Okay, simple enough now then.

        MOV     fOP, dOPh, LSL #DExp_len+1      ;Extract upper fraction
        ORR     fOP, dOPl, fOP, LSR #SExp_len+1 ;Combine fraction
        TSTEQ   fOP, #1                         ;Z<-don't round
        ADDNE   fOP, fOP, #1                    ;add rounding bit
        TST     fOP, #1:SHL:SExp_pos            ;check for overflow

        ADD     fOP, fOP, utmp2, LSL #SExp_pos  ;add in exponent
        ORREQ   fOP, fOP, tmp                   ;and sign flag

        ReturnToLR EQ                           ;17S+1N

;If there was an overflow in the mantissa, check for an overflow in the
;exponent - i.e. exponent = SNaNInf

        CMP     utmp2, #SNaNInf-1
        ORRNE   fOP, fOP, tmp
        ReturnToLR NE

_d2f_TakeOverflowTrap

        MOV     a4,#OVF_bit:OR:SingleErr_bit
        B       __fp_nonveneer_error

        ]

;------------------------------------------------------------------------------

        [ :DEF: f2d_s

        Export  _f2d

_f2d    EnterWithLR_16
        AND     tmp, fOP, #Sign_bit             ;Save sign bit
        MOV     dOPl, fOP, LSL #1               ;Extract exponent
        MOVS    dOPl, dOPl, LSR #31-SExp_len+1
        BEQ     _f2d_denorm

        CMP     dOPl, #SNaNInf                  ;Test for NaN/Inf
        BEQ     _f2d_NaN_or_Inf
        ADD     dOPl, dOPl, #DExp_bias-SExp_bias;Convert exponent range
        ORR     tmp, tmp, dOPl, LSL #DExp_pos   ;Add sign bit back
        MOV     dOPl, fOP, LSL #32-DExp_len+SExp_len    ;Move bits of fraction
        MOV     fOP, fOP, LSL #SExp_len+1       ;Throw away old exponent
        ORR     dOPh, tmp, fOP, LSR #DExp_len+1 ;Recombine top word
        ReturnToLR                              ;12S+1N

_f2d_denorm

;The exponent turned out to be zero. If the mantissa is also zero then this
;is a signed zero, and that is all we need to return. Otherwise we need to
;renormalise the number.

        TEQ     tmp, fOP
        ReturnToLR EQ           ;dOPl is already zero

;Renormalise the number. We need to build up an exponent (in dOPl) whilst
;searching for the msb in the mantissa (fOP). A mantissa of just one
;corresponds to an exponent of &380-22. This will require 23 shifts to find.
;Conversely, a mantissa of (1<<22) (top bit set) takes one shift and
;corresponds to &380. We build the new exponent up alongside the sign bit.

        ORR     tmp, tmp, #(DExp_bias-SExp_bias)<<DExp_pos
        MOVS    dOPh, dOPh, LSL #SExp_len+1+1   ;first shift is implicit

_f2d_DenormLoop
        SUBCC   ip, ip, #1<<DExp_pos
        MOVCCS  dOPh, dOPh, LSL #1      ;conditional for implicit shift
        BCC     _f2d_DenormLoop

;Now the normalised mantissa is in dOPh, and the exponent/sign is in tmp.
;Separate the mantissa into two halves. 20bits are to go in dOPh, the other
;12 in dOPl

        MOV     dOPl, dOPh, LSL #DFhi_len
        ORR     dOPh, tmp, dOPh, LSR #32-DFhi_len

        ReturnToLR

_f2d_NaN_or_Inf

;Here if fOP looks like a NaN or an infinity. Sign bit is in dOP2l

        MOVS    fOP, fOP, LSL #SExp_len+1
        ORREQ   dOPl, dOPl, #(DNaNInf:AND:&ff00)
        ORREQ   dOPh, tmp, dOPl, LSL #32-DExp_len-1     ;Add sign bit
        MOVEQ   dOPl, #0                                ;Must contain zero
        ReturnToLR EQ

;Definitely is a NaN. MOVS will have set N bit to the "signalling" bit

        MOVPL   a4,#IVO_bit:OR:DoubleErr_bit
        BPL     __fp_nonveneer_error

;Return any old quiet NaN (no need to preserve the sign bit)

        MOV     dOPh, #-1
        ReturnToLR

        ]

;==============================================================================

        END
