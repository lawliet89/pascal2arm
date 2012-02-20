;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:28:58 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                Function strncmp

                MOV     r12, r7
                PUSH    {r5, r6}
                MOV     r3, r0
                ORR     r0, r1
                LSL     r0, #&1e
                BNE     strncmp_u

                SUB     r2, #4
                BCC     strncmp_tail

                LDR     r6, ones_word

0               LDMIA   r3!, {r0}
                LDMIA   r1!, {r7}
                SUB     r0, r7
                BNE     base_readj
                SUB     r5, r7, r6
                BIC     r5, r7
                LSL     r7, r6, #7
                AND     r5, r7
                BNE     base_readj
                SUB     r2, #4
                BCS     %BT0

strncmp_tail    ADD     r2, #3
                BCC     exit
                B       %FT0

base_readj      SUB     r3, #4
                SUB     r1, #4
                ADD     r2, #3

0               LDRB    r0, [r3]
                LDRB    r7, [r1]
                SUB     r0, r7
                BNE     exit
                CMP     r7, #0
                BEQ     exit
                ADD     r3, #1
                ADD     r1, #1
strncmp_u       SUB     r2, #1
                BCS     %BT0
                MOV     r0, #0

exit            POP     {r5, r6}
                MOV     r7, r12
                Return  , "", LinkNotStacked

                ALIGN
ones_word       DCD     &01010101

                END
