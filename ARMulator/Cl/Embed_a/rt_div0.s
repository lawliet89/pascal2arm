;;; unhosted.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1996/03/26 00:11:45 $
;;; Revising $Author: enevill $

        GET     objmacs.s
        GET     h_errors.s

        CodeArea

    [ THUMB
        CODE16
    ]

 [ make = "div0" :LOR: make = "all" :LOR: make="shared-library"
        EXPORT  |__rt_div0|
        [ THUMB
        EXPORT  |__16__rt_div0|
        ]
        IMPORT  |__rt_trap|, WEAK

        [ THUMB
        CODE16
|__16__rt_div0|
        BX      pc
        CODE32
        ]
|__rt_div0|
        STMDB   sp!, {r0-r15}
        SUB     r14, r14, #4
        STR     r14, [sp, #15*4]
        ADR     r0, E_DivideByZero
        MOV     r1, sp
        LDR     ip, =__rt_trap
        CMP     ip, #0
    [ INTERWORK:LOR:THUMB
        BXNE    ip
    |
        MOVNE   pc, ip
    ]
        DCD     0xe6000010
        
        ErrorBlock DivideByZero, "Divide by zero"
 ]

        END
