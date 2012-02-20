;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:28:15 $
;;; Revising $Author: enevill $

        GET     objmacs.s

        CodeArea

        CODE16

        Function        strcmp

        MOV     r2, r0
        ORR     r2, r1
        LSL     r2, #&1e
        BNE     strcmp_u
        MOV     r12, r7
        PUSH    {r4, r5}
        LDR     r4, ones_word
        LSL     r5, r4, #7
cmpwords
        LDMIA   r0!, {r2}
        LDMIA   r1!, {r3}
        SUB     r7, r2, r3
        BNE     cmpbytes
        SUB     r7, r2, r4
        BIC     r7, r2
        AND     r7, r5
        BEQ     cmpwords
        MOV     r0, #0
        POP     {r4, r5}
        MOV     r7, r12
        Return  , "", LinkNotStacked
    [ {ENDIAN} = "big"
cmpbytes1
        LSL     r2, #8
        LSL     r3, #8
cmpbytes
        LSR     r1, r2, #&18
        LSR     r0, r3, #&18
        SUB     r0, r1, r0
        BNE     exit_r0
        CMP     r1, #0
        BNE     cmpbytes1
exit_r0
        POP     {r4, r5}
        MOV     r7, r12
        Return  , "", LinkNotStacked
    |
cmpbytes
        LSL     r0, r7, #24
        BNE     exit_r0
        LSL     r1, r2, #24
        BEQ     exit_r0
        LSL     r0, r7, #16
        BNE     exit_r0
        LSR     r1, r2, #8
        LSL     r1, #24
        BEQ     exit_r0
        LSL     r0, r7, #8
        BNE     exit_r0
        LSR     r1, r2, #16
        LSL     r1, #24
        BEQ     exit_r0
        MOV     r0, r7
exit_r0
        POP     {r4, r5}
        MOV     r7, r12
        Return  , "", LinkNotStacked
    ]
strcmp_u
        LDRB    r2, [r0]
        LDRB    r3, [r1]
        SUB     r2, r3
        BNE     exit_r2
        ADD     r0, #1
        ADD     r1, #1
        CMP     r3, #0
        BNE     strcmp_u
exit_r2
        MOV     r0, r2
        Return  , "", LinkNotStacked

        ALIGN
ones_word
        DCD     &01010101

        END
