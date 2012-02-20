; thumb.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.6 $
; Checkin $Date: 1997/01/06 18:51:20 $
; Revising $Author: mwilliam $

        GET     fpe.s

        CodeArea |FPL$$thumb|

        IMPORT  _efix
        IMPORT  _eflt
        IMPORT  _emul
        IMPORT  _esub
        IMPORT  _eadd

; These functions do the same as their ARM equivalents, and a description of
; the code can be found in arm.s

        CODE16

;---------------------------------------------------------------------------

        [ :DEF: range_s 

        EXPORT  __fp_range_red_by_mod

__fp_range_red_by_mod
    [ :DEF: INTERWORK
        PUSH    {v1,v2,v3}
        MOV     v3, lr
    |
        PUSH    {v1,v2,lr}
    ]
        SUB     sp,#12*2+4
        PUSH    {a1-a3}

        MOV     v2,a4
        BL      _emul
        BL      _efix
        LSL     a3,a2,#32-Error_pos
        BNE     range_red_fix_error
        STR     a1,[sp,#12*3]

        BL      _eflt
        MOV     v1,sp   
        ADD     v1,#12*2
        STMIA   v1!,{a1-a3}

        MOV     a4,v2
        ADD     a4,#12*2
        BL      _emul
        SUB     v1,#12*2
        STMIA   v1!,{a1-a3}

        LDMIA   v1!,{a1-a3}
        MOV     a4,v2
        ADD     a4,#12*1
        BL      _emul

        MOV     a4,#1
        LSL     a4,a4,#31
        EOR     a1,a4
        MOV     a4,sp
        BL      _eadd

        MOV     a4,sp
        ADD     a4,#12
        BL      _esub

        ADD     sp,#12*3

    [ :DEF: INTERWORK
        MOV     lr, v3
        POP     {a4,v1,v2,v3}
        BX      lr
    |
        POP     {a4,v1,v2,pc}
    ]

range_red_fix_error
        MOV     a1,a2
        ADD     sp,#12*3+4
    [ :DEF: INTERWORK
        MOV     lr, v3
        POP     {v1,v2,v3}
        BX      lr
    |
        POP     {v1,v2,pc}
    ]

        ]

;---------------------------------------------------------------------------

        [ :DEF: poly_s

        EXPORT  __fp_poly
        
__fp_poly
    [ :DEF: INTERWORK
        PUSH    {a1-a3,v1,v2,v3}
        MOV     v3, lr
    |
        PUSH    {a1-a3,v1,v2,lr}
    ]
        LDR     v1,[sp,#4*6]
        SUB     v2,a4,#1
        LDMIA   v1!,{a1-a3}
poly_loop
        MOV     a4,sp
        BL      _emul
        MOV     a4,v1
        ADD     v1,#12
        BL      _eadd
        SUB     v2,#1
        BNE     poly_loop

        ADD     sp,#12
    [ :DEF: INTERWORK
        MOV     lr, v3
        POP     {v1-v2,v3}
        BX      lr
    |
        POP     {v1-v2,pc}
    ]

        ]

;---------------------------------------------------------------------------

        [ :DEF: poly0_s

        EXPORT  __fp_poly0

__fp_poly0
    [ :DEF: INTERWORK
        PUSH    {a1-a3,v1,v2,v3}
        MOV     v3, lr
    |
        PUSH    {a1-a3,v1,v2,lr}
    ]
        SUB     v2,a4,#1
        LDR     a4,[sp,#4*6]
        MOV     v1,a4
        ADD     v1,#12
        BL      _eadd
poly0_loop
        MOV     a4,sp
        BL      _emul
        MOV     a4,v1
        ADD     v1,#12
        BL      _eadd
        SUB     v2,#1
        BNE     poly0_loop

        ADD     sp,#12
    [ :DEF: INTERWORK
        MOV     lr, v3
        POP     {v1,v2,v3}
        BX      lr
    |
        POP     {v1,v2,pc}
    ]

        ]

;---------------------------------------------------------------------------

        [ :DEF: poly1_s

        EXPORT  __fp_poly1

__fp_poly1
    [ :DEF: INTERWORK
        PUSH    {a1-a3,v1,v2,v3}
        MOV     v3, lr
    |
        PUSH    {a1-a3,v1,v2,lr}
    ]
        SUB     v2,a4,#1
        LDR     a4,[sp,#4*6]
        MOV     v1,a4
        ADD     v1,#12
        BL      _emul
poly1_loop
        MOV     a4,v1
        ADD     v1,#12
        BL      _eadd
        MOV     a4,sp
        BL      _emul
        SUB     v2,#1
        BNE     poly1_loop

        ADD     sp,#12
    [ :DEF: INTERWORK
        MOV     lr, v3
        POP     {v1,v2,v3}
        BX      lr
    |
        POP     {v1,v2,pc}
    ]

        ]

;---------------------------------------------------------------------------

        END
