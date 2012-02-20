; veneer.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.4 $
; Checkin $Date: 1995/02/22 10:37:36 $
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

        [ :DEF: thumb
        CODE32
        ]

;===========================================================================
; Veneer functions

        [ :DEF: addsub_s

        CodeArea |FPL$$addsub|

        EXPORT  __fp_addsub_common
        EXPORT  __fp_addsub_uncommon

        ]

;------------------------------------------------------------------------------

        [ :DEF: mul_s

        CodeArea |FPL$$mul|

        EXPORT  __fp_mult_common
        EXPORT  __fp_mult_fast_common
        EXPORT  __fp_mult_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: div_s

        CodeArea |FPL$$div|
        
        EXPORT  __fp_div_common
        EXPORT  __fp_rdv_common
        EXPORT  __fp_div_uncommon
        EXPORT  __fp_rdv_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: sqrt_s

        CodeArea |FPL$$sqrt|

        EXPORT  __fp_sqrt_common
        EXPORT  __fp_sqrt_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: fix_s

        CodeArea |FPL$$fix|

        EXPORT  __fp_fix_common
        EXPORT  __fp_fix_uncommon

        ]

;---------------------------------------------------------------------------

        [ :DEF: fixu_s

        CodeArea |FPL$$fixu|

        EXPORT  __fp_fixu_common
        EXPORT  __fp_fixu_uncommon

        ]

;===========================================================================

        [ :DEF: compare_s

        CodeArea |FPL$$ecompare|

        EXPORT  __fp_compare

        ]

;===========================================================================

        GET     arith.s

        END
