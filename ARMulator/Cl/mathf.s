;;; mathf.s: coding of math function making use of native fp instructions
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.8 $
;;; Checkin $Date: 1995/02/03 19:23:39 $
;;; Revising $Author: irickard $

        GET     objmacs.s

        MACRO
$Label  DisableFPInterrupts
        ; Disables all FP exceptions, remembering the exception mask in ip
        ; for subsequent reinstatement by ReEnableFPInterrupts.  (ip must
        ; therefore be left alone by the FP procedures which call this macro).
$Label  MOV     r1, #0
        RFS     ip
        WFS     r1
        MEND

        MACRO
$Label  ReEnableFPInterrupts
        ; Reinstates the exception mask state which prevailed before the call
        ; to DisableFPInterrupts; sets r1 to the current fp flags.
$Label  RFS     r1
        WFS     ip
        MEND

ERANGE  *       2
EDOM    *       1

        CodeArea

 [ make = "acos" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_m_huge|
        IMPORT  |_edom_m_huge|
   ]
        Function acos

   [ fpregargs
        FunctionEntry UsesSb, ""
        DisableFPInterrupts
   |
        FunctionEntry UsesSb, "r0,r1"
        DisableFPInterrupts
        LDFD    f0, [sp], #8
   ]
        ACSD    f0, f0
        ReEnableFPInterrupts
        ; A range error is not possible; any error must be a domain error.
        ; (And the only plausible error flag is IVO, but I don't check).
        ; Dunno what result is sensible.
        TST     r1, #&07
        Return  UsesSb, "", , EQ
        B       |_edom_m_huge|
 ]

 [ make = "asin" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_edom_m_huge|
   ]
        Function asin

   [ fpregargs
        FunctionEntry UsesSb, ""
        DisableFPInterrupts
   |
        FunctionEntry UsesSb, "r0,r1"
        DisableFPInterrupts
        LDFD    f0, [sp], #8
   ]
        ASND    f0, f0
        ReEnableFPInterrupts
        ; A range error is not possible; any error must be a domain error.
        ; (And the only plausible error flag is IVO, but I don't check).
        ; Dunno what result is sensible.
        TST     r1, #&07
        Return  UsesSb, "", , EQ
        B       |_edom_m_huge|
 ]

 [ make = "atan" :LOR: make = "all" :LOR: make="shared-library"
        Function atan, leaf

   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        ATND    f0, f0
        Return  , "", LinkNotStacked
 ]

 [ make = "atan2" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_edom_m_huge|
   ]
        Function atan2

   [ fpregargs
        FunctionEntry UsesSb, ""
   |
        FunctionEntry UsesSb, "r0, r1, r2, r3"
        LDFD    f0, [sp], #8
        LDFD    f1, [sp], #8
   ]
        CMF     f0, #0
        CMFEQ   f1, #0
        BEQ     |_edom_m_huge|
        POLD    f0, f1, f0
        Return  UsesSb, ""
 ]

 [ make = "ceil" :LOR: make = "all" :LOR: make="shared-library"
        Function ceil, leaf

        ; No exceptions possible for finite arguments
   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        RNDDP   f0, f0
        Return  , "", LinkNotStacked
 ]

 [ make = "cos" :LOR: make = "all" :LOR: make="shared-library"
        Function cos, leaf

; Ought not to be used much since compiler inlines.
   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        COSD    f0, f0
        Return  , "", LinkNotStacked
 ]

 [ make = "exp" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_huge|
        IMPORT  |_erange_0|
   ]
        Function exp

   [ fpregargs
        FunctionEntry UsesSb, ""
        DisableFPInterrupts
   |
        FunctionEntry UsesSb, "r0,r1"
        DisableFPInterrupts
        LDFD    f0, [sp], #8
   ]
        EXPD    f0, f0
        ReEnableFPInterrupts
        TST     r1, #&0F
        Return  UsesSb, "", , EQ
        TST     r1, #8
        BNE     |_erange_0|             ; underflow
        B       |_erange_huge|
 ]

 [ make = "fabs" :LOR: make = "all" :LOR: make="shared-library"
        Function fabs, leaf

   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        ABSD    f0, f0
        Return  , "", LinkNotStacked
 ]

 [ make = "floor" :LOR: make = "all" :LOR: make="shared-library"
        Function floor, leaf

        ; No exceptions possible for finite arguments
   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        RNDDM   f0, f0
        Return  , "", LinkNotStacked
 ]

 [ make = "log" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_m_huge|
        IMPORT  |_edom_m_huge|
   ]
        Function log

   [ fpregargs
        FunctionEntry UsesSb, ""
   |
        FunctionEntry UsesSb, "r0,r1"
        LDFD    f0, [sp], #8
   ]
        CMFE    f0, #0
        BEQ     |_erange_m_huge|
        BMI     |_edom_m_huge|
        LGND    f0, f0
        Return  UsesSb, ""
 ]

 [ make = "log10" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_m_huge|
        IMPORT  |_edom_m_huge|
   ]
        Function log10

   [ fpregargs
        FunctionEntry UsesSb, ""
   |
        FunctionEntry UsesSb, "r0,r1"
        LDFD    f0, [sp], #8
   ]
        CMFE    f0, #0
        BEQ     |_erange_m_huge|
        BMI     |_edom_m_huge|
        LOGD    f0, f0
        Return  UsesSb, ""
 ]

 [ make = "modf" :LOR: make = "all" :LOR: make="shared-library"
        Function modf, leaf

   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        RNDDZ   f1, f0
   [ fpregargs
        STFD    f1, [r0]
   |
        STFD    f1, [r2]
   ]
        SUFD    f0, f0, f1
        Return  , "", LinkNotStacked
 ]

 [ make = "pow" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_m_huge|
        IMPORT  |_edom_m_huge|
        IMPORT  |_erange_huge|
        IMPORT  |_erange_0|
   ]
        Function pow

   [ fpregargs
        FunctionEntry UsesSb, ""
        DisableFPInterrupts
   |
        FunctionEntry UsesSb, "r0,r1,r2,r3"
        DisableFPInterrupts
        LDFD    f0, [sp], #8
        LDFD    f1, [sp], #8
   ]
        CMFE    f0, #0
        BEQ     POWFirstArgZero
        POWD    f0, f0, f1
        ReEnableFPInterrupts
        ; Plausibly, there may have been either an overflow or IVO error.
        ; I assume that the former is always a range error, and the latter
        ; corresponds to one of the possible C domain errors (first arg
        ; negative, second non-integer).
        ; (DVZ assumed impossible).
        TST     r1, #&0F
        Return  UsesSb, "", , EQ
        TST     r1, #1
        BNE     |_edom_m_huge|
        TST     r1, #8
        BNE     |_erange_0|             ; underflow
        B       |_erange_huge|

POWFirstArgZero
        CMFE    f1, #0
        ReEnableFPInterrupts
        Return  UsesSb, "", , GT
        B       |_edom_m_huge|
 ]

 [ make = "sin" :LOR: make = "all" :LOR: make="shared-library"
        Function sin, leaf

; Ought not to be used much since compiler inlines.
   [ fpregargs
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
   ]
        SIND    f0, f0
        Return  , "", LinkNotStacked
 ]

 [ make = "sqrt" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_edom_m_huge|
   ]
        Function sqrt

   [ fpregargs
        FunctionEntry UsesSb, ""
   |
        FunctionEntry UsesSb, "r0,r1"
        LDFD    f0, [sp], #8
   ]
        CMFE    f0, #0
        BMI     |_edom_m_huge|
        SQTD    f0, f0
        Return  UsesSb, ""
 ]

 [ make = "tan" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_huge|
   ]
        Function tan

   [ fpregargs
        FunctionEntry UsesSb, ""
        DisableFPInterrupts
   |
        FunctionEntry UsesSb, "r0,r1"
        DisableFPInterrupts
   ]
        LDFD    f0, [sp], #8
        TAND    f0, f0
        ReEnableFPInterrupts
        TST     r1, #&07
        Return  UsesSb, "", , EQ
        B       |_erange_huge|
 ]

 [ make = "_edom_mh" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_set_errno|
        EXPORT  |_edom_m_huge|
   ]
|_edom_m_huge|
        MOV     r0, #EDOM
        LDFD    f0, negative_huge_val   ; @@@@!!!!
        B       |_set_errno|
negative_huge_val
        DCD     &FFEFFFFF, &FFFFFFFF
 ]

 [ make = "_ernge_0" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_set_errno|
        EXPORT  |_erange_0|
   ]
|_erange_0|
        MOV     r0, #ERANGE
        MVFD    f0, #0
        B       |_set_errno|
 ]

 [ make = "_ernge_h" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_set_errno|
        EXPORT  |_erange_huge|
   ]
|_erange_huge|
        MOV     r0, #ERANGE
        LDFD    f0, huge_val            ; @@@@!!!!
        B       |_set_errno|
huge_val
        DCD     &7FEFFFFF, &FFFFFFFF
 ]

 [ make = "_ernge_mh" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_set_errno|
        EXPORT  |_erange_m_huge|
   ]
|_erange_m_huge|
        MOV     r0, #ERANGE
        LDFD    f0, negative_huge_val   ; @@@@!!!!
        B       |_set_errno|
   [ :LNOT: (make = "all" :LOR: make="shared-library")
negative_huge_val
        DCD     &FFEFFFFF, &FFFFFFFF
   ]
 ]

 [ make = "_set_errno" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        EXPORT  |_set_errno|
   ]
|_set_errno|
        LDR     r1, addr___errno
        STR     r0, [r1, #0]
        Return  UsesSb, ""
 ]

 [ make = "_ldfp" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
        IMPORT  |_erange_m_huge|
        IMPORT  |_erange_huge|
        IMPORT  |_erange_0|
   ]
        Function _ldfp
        FunctionEntry UsesSb
; double _ldfp(void *x) converts packed decimal at x to a double
        DisableFPInterrupts
        LDFP    f0, [r0, #0]
        ADFD    f0, f0, #0   ; (round to D format)
        ReEnableFPInterrupts
        TST     r1, #&F
        Return  UsesSb, "", EQ
        TST     r1, #&7
        BEQ     |_erange_0|

        LDR     r0, [r0, #0]
        CMPS    r0, #0
        BPL     |_erange_huge|
        B       |_erange_m_huge|
 ]

 [ make = "_stfp" :LOR: make = "all" :LOR: make="shared-library"
        Function  _stfp, leaf

; void _stfp(double d, void *x) stores packed decimal at x
   [ fpregargs
        STFP    f0, [r0, #0]
   |
        STMFD   sp!, {r0, r1}
        LDFD    f0, [sp], #8
        STFP    f0, [r2, #0]
   ]
        Return  , "", LinkNotStacked
 ]

 [ make = "status" :LOR: make = "all" :LOR: make="shared-library"
        Function  __fp_status, leaf

FPExceptC_pos   EQU     0
FPExceptE_pos   EQU     16
Except_len      EQU     5

; unsigned int __fp_status(unsigned int mask,unsigned int flags)
; changes the FPSR and returns the old value.

        MOV     ip, a1, LSL #32-FPExceptE_pos-Except_len
        BIC     ip, ip, #&e0:SHL:(32-FPExceptE_pos-Except_len)
        BIC     ip, ip, #&e0:SHL:(32-FPExceptE_pos-Except_len+8)
        AND     a2, a2, ip, LSR #32-FPExceptE_pos-Except_len
        RFS     a1               ; load old flags
        BIC     ip, a1, ip, LSR #32-FPExceptE_pos-Except_len
        EOR     ip, ip, a2       ; toggle/clear/set flags and
        WFS     ip               ; write back.

        Return  , "", LinkNotStacked
 ]

 [ make = "_set_errno" :LOR: make = "all" :LOR: make="shared-library"
        AdconTable

        IMPORT  |__errno|
addr___errno
        &       |__errno|
 ]

        END
