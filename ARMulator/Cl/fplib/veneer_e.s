; veneer_e.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.4 $
; Checkin $Date: 1995/02/20 16:30:07 $
; Revising $Author: mwilliam $

;===========================================================================
;Veneers onto the arith.s functions.
;
;This block should be assembled multiple times, once for each function.
;The possible functions are:
;
;       addsub_s        shared add and subtract
;       mul_s           shared multiply
;       div_s           shared divide

        GET     fpe.s

        MACRO
        InternalToInternal
        BL      _e2e
;;      TST     a4,#Error_bit
;;      LDMEQFD sp!,veneer_l
;;      B       __fp_veneer_error
        MEND

;===========================================================================
; Veneer functions

        [ :DEF: sub_s

        CodeArea |FPL$$eaddsub|

        IMPORT  __fp_addsub_common
        IMPORT  __fp_addsub_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _esub

_esub   VEnter

        LDMIA   a4,{OP2sue,OP2mhi,OP2mlo}
        ASSERT  OP2sue < OP2mhi
        ASSERT  OP2mhi < OP2mlo

        EOR     OP2sue, OP2sue, #Sign_bit

        TST     OP1sue,#Uncommon_bit
        TSTEQ   OP2sue,#Uncommon_bit
        BNE     sub_uncommon

        BL      __fp_addsub_common

sub_return
        InternalToInternal
        VReturn

sub_uncommon
        ADR     lr,sub_return
        MOV     Rins,#Internal_mask
        B       __fp_addsub_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: add_s

        CodeArea |FPL$$eaddsub|

        IMPORT  __fp_addsub_common
        IMPORT  __fp_addsub_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _eadd

_eadd   VEnter

        LDMIA   a4,{OP2sue,OP2mhi,OP2mlo}
        ASSERT  OP2sue < OP2mhi
        ASSERT  OP2mhi < OP2mlo

        TST     OP1sue,#Uncommon_bit
        TSTEQ   OP2sue,#Uncommon_bit
        BNE     add_uncommon

        BL      __fp_addsub_common

add_return
        InternalToInternal
        VReturn

add_uncommon
        ADR     lr,add_return
        MOV     Rins,#Internal_mask
        B       __fp_addsub_uncommon

        ]

;------------------------------------------------------------------------------

        [ :DEF: mul_s

        CodeArea |FPL$$emul|

        IMPORT  __fp_mult_common
        IMPORT  __fp_mult_fast_common
        IMPORT  __fp_mult_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _emul

_emul   VEnter

        LDMIA   a4,{OP2sue,OP2mhi,OP2mlo}
        ASSERT  OP2sue < OP2mhi
        ASSERT  OP2mhi < OP2mlo

        TST     OP1sue,#Uncommon_bit
        TSTEQ   OP2sue,#Uncommon_bit
        BNE     mul_uncommon

        BL      __fp_mult_common

mul_return
        InternalToInternal
        VReturn

mul_uncommon
        ADR     lr,mul_return
        MOV     Rins,#Internal_mask
        B       __fp_mult_uncommon

        ]

;------------------------------------------------------------------------------

        [ :DEF: square_s

        CodeArea |FPL$$square|

        IMPORT  __fp_mult_common
        IMPORT  __fp_mult_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _esquare

_esquare VEnter

        MOV     OP2sue,OP1sue
        MOV     OP2mhi,OP1mhi
        MOV     OP2mlo,OP1mlo

        TST     OP1sue,#Uncommon_bit
        BNE     square_uncommon

        BL      __fp_mult_common

square_return
        InternalToInternal
        VReturn

square_uncommon
        ADR     lr,square_return
        MOV     Rins,#Internal_mask
        B       __fp_mult_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: div_s

        CodeArea |FPL$$ediv|
        
        IMPORT  __fp_div_common
        IMPORT  __fp_div_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _ediv

_ediv   VEnter

        LDMIA   a4,{OP2sue,OP2mhi,OP2mlo}
        ASSERT  OP2sue < OP2mhi
        ASSERT  OP2mhi < OP2mlo

        MOV     Rins,#Internal_mask
        ASSERT  Uncommon_bit = 1:SHL:(31-1)
        ASSERT  EIUnits_bit = 1:SHL:31
        BICS    tmp,OP1mhi,OP1sue,LSL #1
        BICMIS  tmp,OP2mhi,OP2sue,LSL #1
        BPL     div_special_case

        MOV     Rins,#Internal_mask
        BL      __fp_div_common

div_return
        InternalToInternal
        VReturn

div_special_case
        TST     OP2mhi,#EIUnits_bit
        BEQ     div_op2_zero
        TST     OP1sue,#Uncommon_bit
        TSTEQ   OP2sue,#Uncommon_bit
        ADREQ   lr,div_return
        MOV     Rins,#Internal_mask
        BEQ     __fp_div_uncommon

; Op1 must be zero then! Return an appropiately signed zero.

        EOR     OP1sue,OP2sue,OP1sue
        AND     OP1sue,OP1sue,#Sign_bit
        VReturn

div_op2_zero

;Op2 is zero. Divide by zero iff Op1 isn't zero (invalid op)

        TST     OP1mhi,#EIUnits_bit
        MOVEQ   a4,#DVZ_bits
        MOVNE   a4,#IVO_bits
        B       __fp_veneer_error

        ]

;---------------------------------------------------------------------------

        [ :DEF: rdv_s

        CodeArea |FPL$$erdv|
        
        IMPORT  __fp_rdv_common
        IMPORT  __fp_rdv_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _erdv

_erdv   VEnter

        LDMIA   a4,{OP2sue,OP2mhi,OP2mlo}
        ASSERT  OP2sue < OP2mhi
        ASSERT  OP2mhi < OP2mlo

        MOV     Rins,#Internal_mask
        ASSERT  Uncommon_bit = 1:SHL:(31-1)
        ASSERT  EIUnits_bit = 1:SHL:31
        BICS    tmp,OP1mhi,OP1sue,LSL #1
        BICMIS  tmp,OP2mhi,OP2sue,LSL #1
        BPL     rdv_special_case

        MOV     Rins,#Internal_mask :OR: Reverse
        BL      __fp_rdv_common

rdv_return
        InternalToInternal
        VReturn

rdv_special_case
        TST     OP1mhi,#EIUnits_bit
        BEQ     rdv_op1_zero
        TST     OP1sue,#Uncommon_bit
        TSTEQ   OP2sue,#Uncommon_bit
        ADREQ   lr,rdv_return
        MOV     Rins,#Internal_mask :OR: Reverse
        BEQ     __fp_rdv_uncommon

; Op2 must be zero then! (And Op1 is not.) Return an appropiately signed zero.

        EOR     OP1sue,OP2sue,OP1sue
        AND     OP1sue,OP1sue,#Sign_bit
        VReturn

rdv_op1_zero

; Op1 is zero. Divde by zero iff Op2 isn't zero (invalid op)

        TST     OP2mhi, #EIUnits_bit
        MOVEQ   a4, #DVZ_bits
        MOVNE   a4, #IVO_bits
        B       __fp_veneer_error

        ]

;---------------------------------------------------------------------------

        [ :DEF: sqrt_s

        CodeArea |FPL$$esqrt|

        IMPORT  __fp_sqrt_common
        IMPORT  __fp_sqrt_uncommon
        IMPORT  __fp_veneer_error
        IMPORT  _e2e
        EXPORT  _esqrt

_esqrt  VEnter

        MOV     Rins,#Internal_mask
        ASSERT  Uncommon_bit = 1:SHL:(31-1)
        ASSERT  EIUnits_bit = 1:SHL:31
        ASSERT  Sign_bit = 1:SHL:31
        BICS    tmp,OP1mhi,OP1sue,LSL #1        ; PL if uncommon or zero
        MVNMI   tmp,OP1sue                      ; PL if negative
        BPL     sqrt_special_case

        BL      __fp_sqrt_common

sqrt_return
        InternalToInternal
        VReturn

sqrt_special_case
        TST     OP1sue,#Uncommon_bit
        ADRNE   lr,sqrt_return
        MOVNE   Rins,#Internal_mask
        BNE     __fp_sqrt_uncommon

        TST     OP1mhi,#EIUnits_bit

        VReturn NE

        MOV     a4,#IVO_bits
        B       __fp_veneer_error

        ]

;---------------------------------------------------------------------------

        [ :DEF: fix_s

        CodeArea |FPL$$efix|

        EXPORT  _efix
        IMPORT  __fp_fix_common
        IMPORT  __fp_fix_uncommon
        IMPORT  __fp_veneer_error

_efix   VEnter


        ASSERT  Uncommon_bit = 1:SHL:(31-1)
        ASSERT  EIUnits_bit = 1:SHL:31
        BICS    tmp,OP1mhi,OP1sue,LSL #1
        BPL     efix_special_case

        BL      __fp_fix_common

efix_return
        MOV     a2,a1
        MOV     a1,a3
        VReturn

;;      TST     OP1sue,#Error_bit
;;      MOVEQ   a1,OP1mlo
;;      LDMEQFD sp!,veneer_l

;;      MOV     a4,OP1sue
;;      B       __fp_veneer_error

efix_special_case
        TST     OP1sue,#Uncommon_bit
        BNE     efix_uncommon

efix_zero
        MOV     a1,#0
        VReturn

efix_uncommon
        ORR     OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,efix_return
        B       __fp_fix_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: fixu_s

        CodeArea |FPL$$efixu|

        EXPORT  _efixu
        IMPORT  __fp_fixu_common
        IMPORT  __fp_fixu_uncommon
        IMPORT  __fp_veneer_error

_efixu  VEnter

        ASSERT  Uncommon_bit = 1:SHL:(31-1)
        ASSERT  EIUnits_bit = 1:SHL:31
        BICS    tmp,OP1mhi,OP1sue,LSL #1
        BPL     efixu_special_case

        BL      __fp_fixu_common

efixu_return
        MOV     a2,a1
        MOV     a1,a3
        VReturn

;;      TST     OP1sue,#Error_bit
;;      MOVEQ   a1,OP1mlo
;;      LDMEQFD sp!,veneer_l

;;      MOV     a4,OP1sue
;;      B       __fp_veneer_error

efixu_special_case
        TST     OP1sue,#Uncommon_bit
        BNE     efixu_uncommon

efixu_zero
        MOV     a1,#0
        VReturn

efixu_uncommon
        ORR     OP1sue,OP1sue,#Uncommon_bit
        ADR     lr,efixu_return
        B       __fp_fixu_uncommon

        ]

;===========================================================================

        END
