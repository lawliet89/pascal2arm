;;; unhosted.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.12 $
;;; Checkin $Date: 1994/12/22 17:11:19 $
;;; Revising $Author: enevill $

        GET     objmacs.s
        GET     h_errors.s

        MACRO
        LoadIPIfReentrant
   [ make = "shared-library"
        MOV     ip, sb          ; intra-link-unit entry
                                ; (sb gets preserved & restored only if needed)
   ]
        MEND

        CodeArea

    [ THUMB
        CODE16
    ]

; Support for compiler profiling options.

 [ make = "count" :LOR: make = "all" :LOR: make="shared-library"
        Function _count, leaf       ; used when profile option is enabled
        Function __rt_count, leaf

   [ THUMB
        !       1, "_count not implemented in 16 bit code"
   ]
   [ {CONFIG} = 26
        BIC     lr, lr, #&FC000003  ; remove condition code bits
   ]
        LDR     ip, [lr, #0]
        ADD     ip, ip, #1
        STR     ip, [lr, #0]
        ADD     pc, lr, #4          ; condition codes are preserved because
                                    ; nothing in this code changes them!

 ]

 [ make = "count1" :LOR: make = "all" :LOR: make="shared-library"
        Function _count1, leaf
        Function __rt_count1, leaf

   [ THUMB
        !       1, "_count1 not implemented in 16 bit code"
   ]
   [ {CONFIG} = 26
        BIC     lr, lr, #&FC000003  ; remove condition code bits
   ]
        LDR     ip, [lr, #0]
        ADD     ip, ip, #1
        STR     ip, [lr, #0]
        ADD     pc, lr, #8          ; condition codes are preserved because
                                    ; nothing in this code changes them!
 ]

 [ make = "rwcheck" :LOR: make = "ALL" :LOR: make="shared-library"
; Support for compiler option to check pointers before dereferencing.

    [ THUMB
        !       1, "rwcheck not implemented in 16 bit code"
    ]

        IMPORT  |__rt_trap|

        Function _rd1chk
        Function __rt_rd1chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     readfail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       readfail

        Function _rd2chk
        Function __rt_rd2chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     readfail
        TST     a1, #1
        BNE     readfail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       readfail

        Function _rd4chk
        Function __rt_rd4chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     readfail
        TST     a1, #3
        BNE     readfail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       readfail

        Function _wr1chk
        Function __rt_wr1chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     writefail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       writefail

        Function _wr2chk
        Function __rt_wr2chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     writefail
        TST     a1, #1
        BNE     writefail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       writefail

        Function _wr4chk
        Function __rt_wr4chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     writefail
        TST     a1, #3
        BNE     writefail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC

writefail
        STMFD   sp!, {r0, r1}
        ADR     r0, E_WriteFail
        B       fault

readfail
        STMFD   sp!, {r0, r1}
        ADR     r0, E_ReadFail
fault
        ; original r0 and r1 are on the stack.
        ; r14 is the place to pretend the fault happened
   [ make = "shared-library"
        ; ip is our sb value
        Push    "sb,ip"                 ; caller's and our sb
        MOV     sb, ip                  ; can't access sb-relative data from ip
        LDR     r1, addr___rt_registerDump
        Pop     "sb"
   |
        LDR     r1, addr___rt_registerDump
   ]
   [ LDM_MAX >= 15
        ADD     r1,r1,#2*4
        STMIA   r1, {r2-r14}
   |
        ; blocks of three registers only here for improved latency
        ; (performance fairly immaterial)
        ADD     r1, r1, #15*4
        STMDB   r1!, {r12,sp,r14}
        STMDB   r1!, {r9,r10,r11}
        STMDB   r1!, {r6,r7,r8}
        STMDB   r1!, {r3,r4,r5}
        STMDB   r1!, {r2}
   ]
   [ make = "shared-library"
        Pop     "sb"
   ]
        Pop     "r2,r3"                 ; original r0, r1
        STMDB   r1!, {r2, r3}
        STR     r13, [r1,#r13*4]
        SUB     r14, r14, #4
        STR     r14, [r1, #pc*4]
        B       |__rt_trap|

        ErrorBlock ReadFail, "Illegal read"
        ErrorBlock WriteFail, "Illegal write"

 ]

 [ make = "div0" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        EXPORT  |__rt_div0|
        [ THUMB
        EXPORT  |__16__rt_div0|
        ]
   ]
        IMPORT  |__rt_trap|
        IMPORT  |__rt_trap_32|

        [ THUMB
        CODE16
|__16__rt_div0|
        BX      pc
        CODE32
        ]
|__rt_div0|
        ; Dump all registers, then enter the abort code.
        ; r14 is a valid link.
   [ make = "shared-library"
        ; ip is our sb value
        Push    "sb,ip"                 ; caller's and our sb
        MOV     sb, ip                  ; can't access sb-relative data from ip
        LDR     ip, addr___rt_registerDump
        Pop     "sb"
   |
        LDR     ip, addr___rt_registerDump
   ]
        ; blocks of three registers only here for improved latency
        ; (performance fairly immaterial)
   [ LDM_MAX >= 15
        STMIA   ip!, {r0-r14}
   |
        STMIA   ip!, {r0,r1,r2}
        STMIA   ip!, {r3,r4,r5}
        STMIA   ip!, {r6,r7,r8}
        STMIA   ip!, {r9,r10,r11}
        STMIA   ip!, {r12,sp,r14}
   ]
   [ make = "shared-library"
        Pop     "sb"
   ]
        ADR     r0, E_DivideByZero
        SUB     r1, ip, #pc*4
        SUB     r14, r14, #4
        STR     r14, [ip]
        B       __rt_trap_32

        ErrorBlock DivideByZero, "Divide by zero"
 ]
        [ THUMB
        CODE16
        ]

 [ make = "divtest" :LOR: make = "all" :LOR: make="shared-library"
   [ :LNOT: (make = "all" :LOR: make="shared-library")
    [ THUMB
        IMPORT  |__16__rt_div0|
    |
        IMPORT  |__rt_div0|
    ]
   ]
        [ THUMB
        Function __16x$divtest
        Function __16__rt_divtest
        |
        Function x$divtest
        Function __rt_divtest
        ]
; test for division by zero (used when division is voided)

        LoadIPIfReentrant
        CMPS    a1, #0
        Return  , "", LinkNotStacked, NE
        [ THUMB
        BL       |__16__rt_div0|
        |
        BL       |__rt_div0|
        ]
 ]

 [ make = "udiv10" :LOR: make = "all" :LOR: make="shared-library"
; Fast unsigned divide by 10: dividend in a1
; Returns quotient in a1, remainder in a2
;
; Calculate x / 10 as (x * 2**32/10) / 2**32.
; That is, we calculate the most significant word of the double-length
; product. In fact, we calculate an approximation which may be 1 off
; because we've ignored a carry from the least significant word we didn't
; calculate. We correct for this by insisting that the remainder < 10
; and by incrementing the quotient if it isn't.

        [ THUMB
        Function __16_kernel_udiv10, leaf
        Function __16__rt_udiv10, leaf
        |
        Function _kernel_udiv10, leaf
        Function __rt_udiv10, leaf
        ]

    [ THUMB
        MOV     a2, a1
        LSR     a1, #1
        LSR     a3, a1, #1
        ADD     a1, a3
        LSR     a3, a1, #4
        ADD     a1, a3
        LSR     a3, a1, #8
        ADD     a1, a3
        LSR     a3, a1, #16
        ADD     a1, a3
        LSR     a1, #3
        ASL     a3, a1, #2
        ADD     a3, a1
        ASL     a3, #1
        SUB     a2, a3
        CMP     a2, #10
        BLT     %FT0
        ADD     a1, #1
        SUB     a2, #10
0
    |
        SUB     a2, a1, #10
        SUB     a1, a1, a1, lsr #2
        ADD     a1, a1, a1, lsr #4
        ADD     a1, a1, a1, lsr #8
        ADD     a1, a1, a1, lsr #16
        MOV     a1, a1, lsr #3
        ADD     a3, a1, a1, asl #2
        SUBS    a2, a2, a3, asl #1
        ADDPL   a1, a1, #1
        ADDMI   a2, a2, #10
    ]
        Return  , "", LinkNotStacked
 ]

 [ make = "sdiv10" :LOR: make = "all" :LOR: make="shared-library"
; Fast signed divide by 10: dividend in a1
; Returns quotient in a1, remainder in a2
; Quotient is truncated (rounded towards zero).

        [ THUMB
        Function __16_kernel_sdiv10, leaf
        Function __16__rt_sdiv10, leaf
        |
        Function _kernel_sdiv10, leaf
        Function __rt_sdiv10, leaf
        ]

        [ THUMB
        ASR     a4, a1, #31
        EOR     a1, a4
        SUB     a1, a4

        MOV     a2, a1
        LSR     a1, #1
        LSR     a3, a1, #1
        ADD     a1, a3
        LSR     a3, a1, #4
        ADD     a1, a3
        LSR     a3, a1, #8
        ADD     a1, a3
        LSR     a3, a1, #16
        ADD     a1, a3
        LSR     a1, #3
        ASL     a3, a1, #2
        ADD     a3, a1
        ASL     a3, #1
        SUB     a2, a3
        CMP     a2, #10
        BLT     %FT0
        ADD     a1, #1
        SUB     a2, #10
0
        EOR     a1, a4
        SUB     a1, a4
        EOR     a2, a4
        SUB     a2, a4
        |
        MOVS    a4, a1
        RSBMI   a1, a1, #0

        SUB     a2, a1, #10         ; start of udiv10 code (verbatim)
        SUB     a1, a1, a1, lsr #2
        ADD     a1, a1, a1, lsr #4
        ADD     a1, a1, a1, lsr #8
        ADD     a1, a1, a1, lsr #16
        MOV     a1, a1, lsr #3
        ADD     a3, a1, a1, asl #2
        SUBS    a2, a2, a3, asl #1
        ADDPL   a1, a1, #1
        ADDMI   a2, a2, #10

        MOVS    a4, a4
        RSBMI   a1, a1, #0
        RSBMI   a2, a2, #0
        ]
        Return  , "", LinkNotStacked

 ]

 [ make = "sdiv_rolled"
        [ THUMB
        Function __16x$divide
        Function __16__rt_sdiv
        |
        Function x$divide
        Function __rt_sdiv
        ]

        [ THUMB
        IMPORT  |__16__rt_div0|
        |
        IMPORT  |__rt_div0|
        ]

        LoadIPIfReentrant
        [ THUMB
        ASR     a4, a2, #31
        EOR     a2, a4
        SUB     a2, a4

        ASR     a3, a1, #31
        EOR     a1, a3
        SUB     a1, a3

        BNE     %FT0
        BL      |__16__rt_div0| ; Divide by zero
0

        PUSH    {a3, a4}        ; Save so we can look at signs later on

        LSR     a4, a2, #1
        MOV     a3, a1

s_loop  CMP     a3, a4
        BNLS    %FT0
        LSL     a3, #1
0       BLO     s_loop

        MOV     a4, #0
        B       %FT0
s_loop2 LSR     a3, #1
0       CMP     a2, a3
        ADC     a4, a4
        CMP     a2, a3
        BCC     %FT0
        SUB     a2, a3
0
        CMP     a3, a1
        BNE     s_loop2
        MOV     a1, a4

        POP     {a3, a4}

        EOR     a3, a4
        EOR     a1, a3
        SUB     a1, a3

        EOR     a2, a4
        SUB     a2, a4
        |
; all-new signed divide entry sequence
; effectively zero a4 as top bit will be shifted out later
        ANDS    a4, a1, #&80000000
        RSBMI   a1, a1, #0
        EORS    ip, a4, a2, ASR #32
; ip bit 31 = sign of result
; ip bit 30 = sign of a2
        RSBCS   a2, a2, #0

; central part is identical code to udiv
; (without MOV a4, #0 which comes for free as part of signed entry sequence)
        MOVS    a3, a1
        BEQ     |__rt_div0|

s_loop
; justification stage shifts 1 bit at a time
        CMP     a3, a2, LSR #1
        MOVLS   a3, a3, LSL #1
; NB: LSL #1 is always OK if LS succeeds
        BLO     s_loop

s_loop2
        CMP     a2, a3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3

        TEQ     a3, a1
        MOVNE   a3, a3, LSR #1
        BNE     s_loop2
        MOV     a1, a4

        MOVS    ip, ip, ASL #1
        RSBCS   a1, a1, #0
        RSBMI   a2, a2, #0
        ]

        Return  , "", LinkNotStacked
 ]

 [ make = "udiv_rolled"
        [ THUMB
        IMPORT  |__16__rt_div0|
        |
        IMPORT  |__rt_div0|
        ]

        [ THUMB
        Function __16x$udivide
        Function __16__rt_udiv
        |
        Function x$udivide
        Function __rt_udiv
        ]

        LoadIPIfReentrant
        [ THUMB
        LSR     a4, a2, #1
        MOV     a3, a1
        BNE     u_loop
        BL      |__16__rt_div0|

u_loop  CMP     a3, a4
        BNLS    %FT0
        LSL     a3, #1
0       BLO     u_loop

        MOV     a4, #0
        B       %FT0
u_loop2 LSR     a3, #1
0       CMP     a2, a3
        ADC     a4, a4
        CMP     a2, a3
        BCC     %FT0
        SUB     a2, a3
0
        CMP     a3, a1
        BNE     u_loop2
        MOV     a1, a4
        |
; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
; Destroys a3, a4

        MOV     a4, #0
        MOVS    a3, a1
        BEQ     |__rt_div0|

u_loop
; justification stage shifts 1 bit at a time
        CMP     a3, a2, LSR #1
        MOVLS   a3, a3, LSL #1
; NB: LSL #1 is always OK if LS succeeds
        BLO     u_loop

u_loop2
        CMP     a2, a3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3

; CMP sets carry so that loop-step MOV is executed if BNE u_loop2 taken
        TEQ     a3, a1
        MOVNE   a3, a3, LSR #1
        BNE     u_loop2
        MOV     a1, a4
        ]

        Return  , "", LinkNotStacked
 ]

 [ make = "sdiv_unrolled8" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

        Function x$divide
        Function __rt_sdiv

        [ THUMB
        !       1, "sdiv_unrolled8 not implemented for 16 bit code"
        ]
        LoadIPIfReentrant
; Signed divide of a2 by a1: returns quotient in a1, remainder in a2
; Quotient is truncated (rounded towards zero).
; Sign of remainder = sign of dividend.
; Destroys a3, a4 and ip
; Negates dividend and divisor, then does an unsigned divide; signs
; get sorted out again at the end.
; Core code almost identical to udiv

; all-new signed divide entry sequence
; effectively zero a4 as top bit will be shifted out later
        ANDS    a4, a1, #&80000000
        RSBMI   a1, a1, #0
        EORS    ip, a4, a2, ASR #32
; ip bit 31 = sign of result
; ip bit 30 = sign of a2
        RSBCS   a2, a2, #0

; central part is identical code to udiv
; (without MOV a4, #0 which comes for free as part of signed entry sequence)
        MOVS    a3, a1
        BEQ     |__rt_div0|

; so only need to prove independently that udiv & signed entry/exit are OK
s_loop
        CMP     a3, a2, LSR #8
        MOVLS   a3, a3, LSL #8
        BLO     s_loop

        CMP     a3, a2, LSR #1
        BHI     s_jump7
        CMP     a3, a2, LSR #2
        BHI     s_jump6
        CMP     a3, a2, LSR #3
        BHI     s_jump5
        CMP     a3, a2, LSR #4
        BHI     s_jump4
        CMP     a3, a2, LSR #5
        BHI     s_jump3
        CMP     a3, a2, LSR #6
        BHI     s_jump2
        CMP     a3, a2, LSR #7
        BHI     s_jump1

s_loop2
; not executed when falling into s_loop2
        MOVHI   a3, a3, LSR #8

        CMP     a2, a3, LSL #7
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #7
        CMP     a2, a3, LSL #6
s_jump1
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #6
        CMP     a2, a3, LSL #5
s_jump2
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #5
        CMP     a2, a3, LSL #4
s_jump3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #4
        CMP     a2, a3, LSL #3
s_jump4
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #3
        CMP     a2, a3, LSL #2
s_jump5
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #2
        CMP     a2, a3, LSL #1
s_jump6
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #1
s_jump7
        CMP     a2, a3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3

        CMP     a3, a1
        BNE     s_loop2
        MOV     a1, a4

        MOVS    ip, ip, ASL #1
        RSBCS   a1, a1, #0
        RSBMI   a2, a2, #0

        Return  , "", LinkNotStacked
 ]

 [ make = "udiv_unrolled8" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]
        Function x$udivide
        Function __rt_udiv

        [ THUMB
        !       1, "udiv_unrolled8 not implemented for 16 bit code"
        ]
        LoadIPIfReentrant
; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
; Destroys a3, a4

        MOV     a4, #0
        MOVS    a3, a1
        BEQ     |__rt_div0|

u_loop
; justification stage shifts 8 bits at a time
        CMP     a3, a2, LSR #8
        MOVLS   a3, a3, LSL #8
; NB: LSL #8 is always OK if LS succeeds
; NB: jump-in branches moved OUTSIDE u_loop - saves cycles for big divides
        BLO     u_loop

        CMP     a3, a2, LSR #1
; branch if a2 < (a3 LSL #1)
        BHI     u_jump7
        CMP     a3, a2, LSR #2
; branch if a2 < (a3 LSL #2)
; already know a2 >= (a3 LSL #1) as above test failed
; CS if taken, so can miss out CMP
        BHI     u_jump6
        CMP     a3, a2, LSR #3
        BHI     u_jump5
        CMP     a3, a2, LSR #4
        BHI     u_jump4
        CMP     a3, a2, LSR #5
        BHI     u_jump3
        CMP     a3, a2, LSR #6
        BHI     u_jump2
        CMP     a3, a2, LSR #7
        BHI     u_jump1
; NB: CC here so MOV below is not executed
u_loop2
; not executed when falling into u_loop2
; put MOV here so it is not executed for simpler divisions (eg. Dhrystone!)
        MOVHI   a3, a3, LSR #8

        CMP     a2, a3, LSL #7
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #7
        CMP     a2, a3, LSL #6
u_jump1
; don't bother with CMP as we know there is a subtraction
; NB: CS if u_jump1 taken so it all works
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #6
        CMP     a2, a3, LSL #5
u_jump2
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #5
        CMP     a2, a3, LSL #4
u_jump3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #4
        CMP     a2, a3, LSL #3
u_jump4
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #3
        CMP     a2, a3, LSL #2
u_jump5
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #2
        CMP     a2, a3, LSL #1
u_jump6
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3, LSL #1
u_jump7
; need to perform CMP this time as we don't know if a2 >= a3 if u_jump7 taken
        CMP     a2, a3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3

; CMP sets carry so that loop-step MOV is executed if BNE u_loop2 taken
        CMP     a3, a1
        BNE     u_loop2
        MOV     a1, a4

        Return  , "", LinkNotStacked
 ]

 [ make = "dspdiv64" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

        Function __rt_sdiv64by32

        [ THUMB
        !       1, "dspdiv64 not implemented for 16 bit code"
        ]
        LoadIPIfReentrant
; Constant time divide
; (64-bit) / (32-bit) = (32-bit) quotient and (32-bit) remainder
; 108 cycles + call and return

; Entry: r0 = MSW of dividend, r1 = LSW of dividend, r2 = divisor.
; Exit:  r0 = Remainder, r1 = Quotient, r2 unchanged.

        MOVS        r3, r2
        BEQ         |__rt_div0|               ; divide by zero handler
        RSBPL       r2, r2, #0                ; negate absolute value of divisor
        MOV         r3, r3, LSR #1            ; shift r1 sign down one bit
        EORS        r3, r3, r0, ASR #1        ; insert dividend sign and
        ; r1 bit 31 sign of dividend (= sign of remainder)
        ;    bit 30 sign of dividend EOR sign of divisor (= sign of quotient)
        BPL         %F01
        RSBS        r1, r1, #0                ; absolute value of dividend
        RSC         r0, r0, #0                ; absolute value of dividend
01
        ADDS        r1, r1, r1

        ADCS        r0, r2, r0, LSL #1        ; 31
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 30
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 29
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 28
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 27
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 26
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 25
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 24
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 23
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 22
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 21
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 20
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 19
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 18
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 17
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 16
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 15
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 14
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 13
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 12
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 11
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 10
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 9
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 8
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 7
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 6
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 5
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 4
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 3
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 2
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 1
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 0
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1

        MOVS        r3, r3, ASL #1
        RSBMI       r1, r1, #0
        RSBCS       r0, r0, #0

        Return  , "", LinkNotStacked
 ]

 [ make = "dspdiv32" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

        Function __rt_sdiv32by16

        [ THUMB
        !       1, "dspdiv32 not implemented for 16 bit code"
        ]
        LoadIPIfReentrant
; Constant time divide
; (32-bit) / (16-bit) = (16-bit) quotient and (16-bit) remainder
; 44 cycles + call and return

; Entry: r0 = dividend, r1 = divisor.
; Exit:  r0 = remainder, r1 = quotient, r2 destroyed

        MOVS        r1, r1, LSL #16           ; shift the divisor to the top
        BEQ         |__rt_div0|               ; divide by zero handler
        MOV         r2, r1, LSR #1            ; shift r1 sign down one bit
        RSBPL       r1, r1, #0                ; negate absolute value of divisor
        EORS        r2, r2, r0, ASR #1        ; insert dividend sign and
        ; r1 bit 31 sign of dividend (= sign of remainder)
        ;    bit 30 sign of dividend EOR sign of divisor (= sign of quotient)
        RSBMI       r0, r0, #0                ; absolute value of dividend

        ADDS        r0, r1, r0, LSL #1        ; 15
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 14
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 13
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 12
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 11
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 10
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 9
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 8
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 7
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 6
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 5
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 4
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 3
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 2
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 1
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 0
        RSBCC       r0, r1, r0

        MOV         r1, r0, LSR #16           ; extract the remainder
        BIC         r0, r0, r1, LSL #16       ; and the partial quotient
        ADC         r0, r0, r0                ; insert the final quotient bit

        MOVS        r2, r2, ASL #1            ; put the signs back
        RSBMI       r0, r0, #0                ; quotient
        RSBCS       r1, r1, #0                ; remainder

        Return  , "", LinkNotStacked
 ]

        AdconTable

 [ make = "rwcheck" :LOR: make = "div0" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT  |__rt_registerDump|
addr___rt_registerDump
        &       |__rt_registerDump|
 ]
        END
