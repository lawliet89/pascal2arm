;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:29:06 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                Function        strncpy

                MOV     r12, r0
                MOV     r3, r0
                ORR     r3, r1
                LSL     r3, #&1e
                BNE     strncpy_u

                SUB     r2, #4
                BCC     strncpy_tail

                PUSH    {r4, r5, r7}
                LDR     r4, ones_word
                LSL     r5, r4, #7

0               LDMIA   r1!, {r7}
                SUB     r3, r7, r4
                BIC     r3, r7
                AND     r3, r5
                BNE     base_readj
                STMIA   r0!, {r7}
                SUB     r2, #4
                BCS     %BT0

                POP     {r4, r5, r7}

strncpy_tail    ADD     r2, #3
                BCC     exit

0               LDRB    r3, [r1]
                STRB    r3, [r0]
                ADD     r1, #1
                ADD     r0, #1
                CMP     r3, #0
                BEQ     strncpy_pad
strncpy_u       SUB     r2, #1
                BCS     %BT0
                MOV     r0, r12
                Return  , "", LinkNotStacked

base_readj      POP     {r4, r5, r7}
                SUB     r1, #4
                ADD     r2, #3
                B       %BT0

0               STRB    r3, [r0]
                ADD     r0, r0, #1
strncpy_pad     SUB     r2, #1
                BCS     %BT0

exit            MOV     r0, r12
                Return  , "", LinkNotStacked

                ALIGN
ones_word       DCD     &01010101

                END
