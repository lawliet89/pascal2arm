; arm.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.5 $
; Checkin $Date: 1997/01/06 18:51:18 $
; Revising $Author: mwilliam $

; Functions to help mathlib a bit.

        GET     fpe.s

        CodeArea |FPL$$MathLib|

        MACRO
$label  PopAndReturn $cc
$label  [ :DEF: INTERWORK
        LDM$cc.FD sp!,{v1-v2,lr}
        BX      lr
        |
        [ {CONFIG} = 32
        LDM$cc.FD sp!,{v1-v2,pc}
        ]
        [ {CONFIG} = 26
        LDM$cc.FD sp!,{v1-v2,pc}^
        ]
        ]
        MEND

        IMPORT  _efix
        IMPORT  _eflt
        IMPORT  _emul
        IMPORT  _esub
        IMPORT  _eadd

; static __value_in_regs __fp_range_red range_red_by_mod(ip_number x,ip_number *parms)

; Takes an internal precision value in OP1sue, OP1mhi, OP1mlo and a pointer
; to a pointer to a block of three numbers (R, M1, M2), and returns a
; range reduced number and N.

; (From the floating point emulator:

;   Let X be the value to be range-reduced, and M be the modulus of the
;   range-reduction. Represent M as the sum of two floating point values
;   M1 and M2, such that M1 is the more significant of the two and M1 times
;   any integer is exact. We also require an extended precision
;   approximation R of the reciprocal of M. Then the following sequence of
;   calculations is performed:
;
;     IF (X*R overflows an integer) THEN
;       Return error;
;     ELSE
;       N := X*R, rounded to nearest integer;
;       XN := FLOAT(N);
;       X := X - XN * M1;
;       X := X - XN * M2;
;       Return (X,N);
;     ENDIF

;  )

        [ :DEF: range_s

        EXPORT  __fp_range_red_by_mod

__fp_range_red_by_mod

; Passed in the value to reduce in a1-a3. a4 points two a 12 word block
;containing the values R, M1 and M2 (see transhw for details of the
;algorithm).
; Returns a floating value in a1-a3 and the N value in a4.
; Uses the following structure on the stack.

;             |_______________|     <-- sp on entry
;             |     Saved     |
;             |   Registers   |
;             |_______________|
;             |_______n_______|
;             |               | MLO
;             |      x_n      | MHI
;             |_______________| SUE
;             |               | MLO
;             |      tmp      | MHI
;             |_______________| SUE
;             |               | MLO
;             |       x       | MHI
;             |_______________| SUE   <--  sp points here

        STMFD   sp!,{v1-v2,lr}
        SUB     sp, sp, #12*2+4         ; Gap for n, xn and tmp
        STMFD   sp!,{a1-a3}
        MOV     v2, a4                  ; save ptr to p[]
        BL      _emul                   ; _emul(x,&p[0])
        BL      _efix                   ; n=_efix(_emul(x,&p[0]));
        TST     a2, #Error_bit
        BNE     range_red_fix_error
        STR     a1, [sp,#12*3]          ; store away n
        BL      _eflt                   ; x_n=_eflt(n);
        ADD     v1, sp, #12*2
        STMIA   v1, {a1-a3}             ; store x_n

        ADD     a4, v2, #12*2           ; ptr to M2 (p[2])
        BL      _emul                   ; tmp1=_emul(xn,&p[2]);
        ADD     v1, sp, #12
        STMIA   v1!, {a1-a3}            ; store out tmp1 - leaves v1 pointing to x_n

        LDMIA   v1, {a1-a3}             ; load in x_n
        ADD     a4, v2, #12*1           ; ptr to M1 (p[1])
        BL      _emul                   ; tmp2=_emul(xn,&p[1]);

        MOV     a4, sp                  ; point a4 at x
        EOR     a1, a1, #Sign_bit       ; negate(tmp2);
        BL      _eadd                   ; tmp3=_eadd(tmp2,&x);

        ADD     a4, sp, #12             ; point a4 at tmp1
        BL      _esub                   ; result_x=_esub(tmp3,&tmp1);

        ADD     sp, sp, #12*3           ; unwind stack
    [ :DEF: INTERWORK
        LDMFD   sp!,{a4,v1-v2,lr}       ; result_n=n and return
        BX      lr
    |
        [ {CONFIG} = 32
        LDMFD   sp!,{a4,v1-v2,pc}       ; result_n=n and return
        ]
        [ {CONFIG} = 26
        LDMFD   sp!,{a4,v1-v2,pc}^      ; result_n=n and return
        ]
    ]

range_red_fix_error
        MOV     a1, a2                  ; return the error flags
        ADD     sp, sp, #12*3+4         ; fix up the stack pointer
        PopAndReturn

        ]

;---------------------------------------------------------------------------

; extern __value_in_regs ip_number __fp_poly(ip_number,int,ip_number *)

; Function to evaluate an n term polynomial

        [ :DEF: poly_s

        EXPORT  __fp_poly

; Passed in a value in OP1sue, OP1mhi, OP1mlo, the number of co-efficients
; and the co-efficients on the stack.

; Returns ((...((((p0*x)+p1)*x)+p2)*x+...))

; Uses a very simple stack structure:

;             |_______________|     <-- sp on entry
;             |               |
;             |               |
;             |     saved     |
;             |   registers   |
;             |               |
;             |               |
;             |_______________|
;             |               | MLO
;             |       x       | MHI
;             |_______________| SUE <-- sp on entry to _emul(s,x)


__fp_poly
        STMFD   sp!, {a1-a3,v1-v2,lr}
        LDR     v1, [sp,#4*6]           ; load pointer to arguments
        SUB     v2, a4, #1              ; counter
        LDMIA   v1!, {a1-a3}            ; s=p0;
poly_loop
        MOV     a4, sp                  ; ptr to x
        BL      _emul                   ; s=_emul(s,&x);
        MOV     a4, v1                  ; ptr to next argument
        ADD     v1, v1, #12
        BL      _eadd                   ; s=_eadd(s,&p);
        SUBS    v2, v2, #1
        BNE     poly_loop

        ADD     sp, sp, #12             ; skip past x on stack
        PopAndReturn
        ]

;---------------------------------------------------------------------------

; extern __value_in_regs ip_number __fp_poly0(ip_number,int,ip_number *)

; Function to evaluate an n term polynomial

        [ :DEF: poly0_s

        EXPORT  __fp_poly0

; Passed in a value in OP1sue, OP1mhi, OP1mlo, the number of co-efficients
; and the co-efficients on the stack.

; Returns ((...(((((x+p0)*x)+p1)*x)+p2)*x+...))

__fp_poly0
        STMFD   sp!, {a1-a3,v1-v2,lr}
        SUB     v2, a4, #1              ; counter
        LDR     a4, [sp,#4*6]           ; load pointer to arguments
        ADD     v1, a4, #12
        BL      _eadd                   ; s=x+&p0
poly0_loop
        MOV     a4, sp
        BL      _emul                   ; s=_emul(s,&x);
        MOV     a4, v1
        ADD     v1, v1, #12
        BL      _eadd                   ; s=_eadd(s,&p);
        SUBS    v2, v2, #1
        BNE     poly0_loop

        ADD     sp, sp, #12             ; skip past x on stack
        PopAndReturn
        ]

;---------------------------------------------------------------------------

; extern __value_in_regs ip_number __fp_poly1(ip_number,int,ip_number *)

; Function to evaluate an n term polynomial

        [ :DEF: poly1_s

        EXPORT  __fp_poly1

; Passed in a value in OP1sue, OP1mhi, OP1mlo, the number of co-efficients
; and the co-efficients on the stack.

; Returns ((...(((((p0*x)+p1)*x)+p2)*x...))

__fp_poly1
        STMFD   sp!, {a1-a3,v1-v2,lr}
        SUB     v2, a4, #1              ; counter
        LDR     a4, [sp,#4*6]           ; load pointer to arguments
        ADD     v1, a4, #12
        BL      _emul                   ; s=_emul(x,&p0)
poly1_loop
        MOV     a4, v1
        ADD     v1, v1, #12
        BL      _eadd                   ; s=_eadd(s,&p);
        MOV     a4, sp
        BL      _emul                   ; s=_emul(s,&x);
        SUBS    v2, v2, #1
        BNE     poly1_loop

        ADD     sp, sp, #12             ; skip past x on stack
        PopAndReturn
        ]

;---------------------------------------------------------------------------

        END
