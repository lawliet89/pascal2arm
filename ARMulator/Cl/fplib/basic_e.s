; basic_e.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.5 $
; Checkin $Date: 1995/05/22 11:50:07 $
; Revising $Author: mwilliam $

; basic_e.s -- basic (comparison) functions for extended precision.
; These variants use the FPE code.

        GET     fpe.s

        MACRO
$label  Compare
$label  VEnter

        LDMIA   a4,{OP2sue,OP2mhi,OP2mlo}
        ASSERT  OP2sue < OP2mhi
        ASSERT  OP2mhi < OP2mlo

        BL      __fp_compare

        TST     Rarith,#Error_bit
        MOVNE   a1,#-1
        VReturn NE

        MEND

;===========================================================================

        [ :DEF: eq_s

        CodeArea |FPL$$eeq|

        IMPORT  __fp_compare
        EXPORT  _eeq

_eeq    Compare

        TEQ     Rarith,#Comp_EQ
        MOVEQ   a1,#1
        MOVNE   a1,#0

        VReturn

        ]

;---------------------------------------------------------------------------

        [ :DEF: neq_s

        CodeArea |FPL$$eneq|

        IMPORT  __fp_compare
        EXPORT  _eneq

_eneq   Compare

        TEQ     Rarith,#Comp_EQ
        MOVEQ   a1,#0
        MOVNE   a1,#1

        VReturn

        ]

;===========================================================================

        [ :DEF: ls_s

        CodeArea |FPL$$els|

        IMPORT  __fp_compare
        EXPORT  _els

_els    Compare

        TEQ     Rarith,#Comp_LT
        MOVEQ   a1,#1
        MOVNE   a1,#0

        VReturn

        ]

;---------------------------------------------------------------------------

        [ :DEF: leq_s

        CodeArea |FPL$$eleq|

        IMPORT  __fp_compare
        EXPORT  _eleq

_eleq   Compare

        TEQ     Rarith,#Comp_EQ
        TEQNE   Rarith,#Comp_LT
        MOVEQ   a1,#1
        MOVNE   a1,#0

        VReturn

        ]

;===========================================================================

        [ :DEF: gr_s

        CodeArea |FPL$$egr|

        IMPORT  __fp_compare
        EXPORT  _egr

_egr    Compare

        TEQ     Rarith,#Comp_GT
        MOVEQ   a1,#1
        MOVNE   a1,#0

        VReturn

        ]

;---------------------------------------------------------------------------

        [ :DEF: geq_s

        CodeArea |FPL$$egeq|

        IMPORT  __fp_compare
        EXPORT  _egeq

_egeq   Compare

        TEQ     Rarith,#Comp_EQ
        TEQNE   Rarith,#Comp_GT
        MOVEQ   a1,#1
        MOVNE   a1,#0

        VReturn

        ]

;===========================================================================

        END
