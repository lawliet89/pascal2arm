;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:28:43 $
;;; Revising $Author: enevill $

        GET     objmacs.s

        CodeArea

        CODE16

        Function        strcpy

        MOV     r2, r0
        ORR     r2, r1
        LSL     r2, #&1e
        BNE     strcpy_u
        MOV     r12, r0
        PUSH    {r4, r5}
        LDR     r4, ones_word
        LSL     r5, r4, #7
        B       cpywords0
cpywords
        STMIA   r0!, {r2}
cpywords0
        LDMIA   r1!, {r2}
        SUB     r3, r2, r4
        BIC     r3, r2
        AND     r3, r5
        BEQ     cpywords
    [ {ENDIAN} = "big"
        B       cpybytes0
cpybytes
        ASL     r2, #8
        ADD     r0, r0, #1
cpybytes0
        LSR     r3, r2, #24
        STRB    r3, [r0]
        BNE     cpybytes
    |
        MOV     r3, #&ff
        B       cpybytes0
cpybytes
        LSR     r2, #8
        ADD     r0, r0, #1
cpybytes0
        STRB    r2, [r0]
        TST     r2, r3
        BNE     cpybytes
    ]
        POP     {r4, r5}
        MOV     r0, r12
        Return  , "", LinkNotStacked
strcpy_u
        MOV     r3, r0
strcpy_u1
        LDRB    r2, [r1]
        STRB    r2, [r3]
        ADD     r1, #1
        ADD     r3, #1
        CMP     r2, #0
        BNE     strcpy_u1
        Return  , "", LinkNotStacked

        ALIGN
ones_word
        DCD     &01010101

        END
