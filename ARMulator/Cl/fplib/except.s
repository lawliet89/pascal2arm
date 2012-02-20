; except.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.16.2.1 $
; Checkin $Date: 1997/08/21 12:42:52 $
; Revising $Author: ijohnson $

;==============================================================================
;Error generation - from ANSI library

        GET     fpe.s
        GET     h_errors.s

        [ :DEF: liberror_s

        CodeArea |FPL$$Exception|

        Export_32  __fp_edom
        Export_32  __fp_erange

;  __fp_edom(ulong sign_bit, boolean huge_val);
;  __fp_erange(ulong sign_bit, boolean huge_val);
;
; set errno to EDOM/ERANGE and return sign_bit | (huge_val ? HUGE_VAL : 0)

__fp_edom EnterWithLR
        MOV     a3, #EDOM
        B       skip
__fp_erange EnterWithLR
        MOV     a3, #ERANGE
skip
        AND     a4, a1, #Sign_bit
    [ :DEF:EMBEDDED_CLIB
        IMPORT  __rt_errno_addr, WEAK
        LDR     a1, =__rt_errno_addr
        CMP     a1, #0
        BEQ     %FT1
        STMDB   sp!, {a2-a4, lr}
        BL      __rt_errno_addr
        LDMIA   sp!, {a2-a4, lr}
        STR     a3, [a1]
1
    |
        LDR     a1, errno
        STR     a3, [a1]
    ]
        TEQ     a2, #0
        MOVEQ   a1, #0          ; generate +/- 0.0 in a1/a2
        LDRNE   a1, huge_val
        LDMNEIA a1, {a1, a2}    ; load HUGE_VAL into a1/a2
        ORR     a1, a1, a4      ; add in sign bit
        ReturnToLR

    [ :LNOT::DEF:EMBEDDED_CLIB
errno
        IMPORT  __errno
        DCD     __errno
    ]
 
huge_val
        IMPORT  __huge_val
        DCD     __huge_val

        ]

;==============================================================================
;Setting/returning status flags.

        [ :DEF: status_s

        CodeArea |FPL$$Exception|

        EXPORT  __fp_status

; extern unsigned int __fp_status(unsigned int mask,unsigned int flags)

__fp_status

        [ :DEF: thumb

        CODE16
        ; mask "mask" to an acceptable value
        MOV     a4,#&e0
        LSL     a3,a1,#32-FPExceptE_pos-Except_len
        LSR     a3,a3,#32-FPExceptE_pos-Except_len
        BIC     a3,a4
        LSR     a4,#8
        BIC     a3,a4
        AND     a4,a1
        ORR     a3,a1
        AND     a2,a3
        ; mask in a3, masked flags in a2
        LDR     a4,status_ptr
        LDR     a1,[a4]
        ; **** *WARNING* is ip spare under the TPCS ????? ****
        MOV     ip,a1           ; store value
        BIC     a1,a3
        EOR     a1,a2
        STR     a1,[a4]
        MOV     a1,ip           ; restore value
        BX      lr
        ALIGN

        |

        ; mask "mask" to an acceptable value
        MOV     a3,a1,LSL #32-FPExceptE_pos-Except_len
        BIC     a3,a3,#&e0:SHL:(32-FPExceptE_pos-Except_len)
        BIC     a3,a3,#&e0:SHL:(32-FPExceptE_pos-Except_len+8)
        AND     a2,a2,a3,LSR #32-FPExceptE_pos-Except_len
        ; mask in a3, masked flags in a2.
        LDR     a4,status_ptr
        LDR     a1,[a4]          ; load old flags
        BIC     a3,a1,a3,LSR #32-FPExceptE_pos-Except_len
        EOR     a3,a3,a2         ; clear/set/toggle flags and
        STR     a3,[a4]          ; write back.
        MOV     pc,lr            ; return 

        ]

status_ptr
        IMPORT  __fp_status_flags
        DCD     __fp_status_flags

        ]

;==============================================================================
;Error generation - from fplib code

        [ :DEF: except_s

        CodeArea |FPL$$Exception|

; SWI Names

        [ :DEF: thumb
        CODE32
        ]

        EXPORT  __fp_veneer_error
        EXPORT  __fp_nonveneer_error

;Called when the error flag is spotted before returning from a veneered
;function. The exact nature of the error will be in a4. The stack contains
;"veneer_s" - a list of {r4-r9,r11,lr} registers is on the stack. r0-r3
;have been corrupted.

__fp_veneer_error
        VPull
__fp_nonveneer_error

        ; a1 contains the prospective sign
        ; a4 contains the error flags (see fpe.s for a list)
        ; leaves a2, a3 and ip spare.

    [ :DEF:EMBEDDED_CLIB
        IMPORT  __rt_fp_status_addr, WEAK
        LDR     a3, =__rt_fp_status_addr
        CMP     a3, #0
        MOVEQ   a2, #IOE_bit+OFE_bit+DZE_bit
        BEQ     %FT1
        STMDB   sp!, {a1, a4, lr}
        BL      __rt_fp_status_addr
        MOV     a3, a1
        LDMIA   sp!, {a1, a4, lr}
    |
        LDR     a3,status_ptr
    ]
        LDR     a2,[a3]
        MOV     ip,a4,LSL #32-Except_pos-Except_len
        MOV     ip,ip,LSR #32-Except_len
        ORR     a2,a2,ip,LSL #FPExceptC_pos
        STR     a2,[a3]                         ; write back new flags.

1
        TST     a4,#OVF_bit
        BNE     overflow
        TST     a4,#DVZ_bit
        BNE     divide_by_zero
        ;TST     a4,#IVO_bit
        ;BNE     invalid_operation

invalid_operation
        TST     a2,#IOE_bit                     ; check enabled
        BEQ     return_NaN
        ADR     a1,E_FP_IVO
        B       GenerateError
overflow
        TST     a2,#OFE_bit                     ; check enabled
        BEQ     return_Inf
        ADR     a1,E_FP_OFL
        B       GenerateError
divide_by_zero
        TST     a2,#DZE_bit                     ; check enabled
        BEQ     return_Inf
        ADR     a1,E_FP_DVZ
        ;B      GenerateError

GenerateError
    [ :DEF:EMBEDDED_CLIB
        STMDB   sp!, {r0-r15}
        SUB     lr, lr, #4
        STR     lr, [sp, #15*4]
        MOV     r1, sp
    |
        LDR     r1,ErrBlock
        SUB     lr,lr,#4
        STR     lr,[r1,#15*4]
        MOV     lr,#&de00
        ORR     lr,lr,#&00ad
        ORR     lr,lr,lr,LSL #16
        STMIA   r1,{r0-r14}

        B       trap

status_ptr
        IMPORT  __fp_status_flags
        DCD     __fp_status_flags

ErrBlock
        IMPORT  __fp_errblock
        DCD     __fp_errblock

trap
    ]

        IMPORT  __rt_trap, WEAK
        LDR     ip, =__rt_trap
        CMP     ip, #0
    [ (:DEF:INTERWORK):LOR:(:DEF:thumb)
        BXNE    ip
    |
        MOVNE   pc, ip
    ]
        DCD     0xe6000010

        ErrorBlock FP_IVO, "Floating Point Exception: Invalid Operation"
        ErrorBlock FP_OFL, "Floating Point Exception: Overflow"
        ErrorBlock FP_DVZ, "Floating Point Exception: Divide By Zero"

return_Inf
        AND     a3,a1,#Sign_bit
        ; return a MAX_INT like value
        TST     a4,#CardinalErr_bits
        BNE     return_MaxInt
        TST     a4,#DoubleErr_bit
        ADRNE   a1,prototype_double_Inf
        LDMNEIA a1,{a1,a2}
        LDREQ   a1,prototype_single_Inf
        ORR     a1,a1,a3
        ReturnToLR

prototype_double_Inf
        DCD     &7ff00000,&00000000
prototype_single_Inf
        DCD     &7f800000

return_NaN
        TST     a4,#CardinalErr_bits
        ANDNE   a3,a1,#Sign_bit     ; sign bit - from result
        BNE     return_MaxInt
        TST     a4,#DoubleErr_bit
        ADRNE   a1,prototype_double_NaN
        LDMNEIA a1,{a1,a2}
        LDREQ   a1,prototype_single_NaN
        ReturnToLR

prototype_double_NaN
        DCD     &7ff80000,&00000000
prototype_single_NaN
        DCD     &7fc00000

return_MaxInt
        ; return an integer error value
        ; could be zero ...
        TST     a4, #ZeroErr_bit
        MOVNE   a1, #0
        MOVNE   a2, #0
        ReturnToLR NE
        ; ... unsigned (size doesn't matter) ...
        TST     a4, #UnsignedErr_bit
        MOVNE   a1, #0xffffffff
        MOVNE   a2, #0xffffffff
        ReturnToLR NE
        ; ... signed - look at a3 for sign bit, and be careful about
        ; endian-ness of long long.
        TST     a4, #LongLongErr_bit
        LDRNE   a1, =0x00000000
        LDRNE   a2, =0x80000000
        ; else must be LongErr_bit
        LDREQ   a1, =0x80000000
        TEQ     a3, #Sign_bit   ; test for -ve result
        MVNNE   a1, a1
        MVNNE   a2, a2
        ReturnToLR

        ]

;------------------------------------------------------------------------------

        [ :DEF: fpdata_s

        AREA    |C$$data|,DATA

        EXPORT  __fp_status_flags
__fp_status_flags
        ; default - all flags enabled.
        DCD     (&40:SHL:SysID_pos):OR:(((1:SHL:Except_len)-1):SHL:FPExceptE_pos)

        EXPORT  __fp_errblock
__fp_errblock
        DCD     0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0

        ]

;==============================================================================

        END
