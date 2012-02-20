;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:28:51 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                Function        strlen

                NEG     r1, r0
                MOV     r12, r1

                LSL     r3, r0, #&1e
                BEQ     aligned

0               LDRB    r3, [r0]
                CMP     r3, #0
                BEQ     exit
                ADD     r0, r0, #1
                LSL     r3, r0, #&1e
                BNE     %BT0

aligned         LDR     r1, eights_word

0               LDMIA   r0!, {r2}
                LSR     r1, #7
                SUB     r3, r2, r1
                BIC     r3, r2
                LSL     r1, #7
                AND     r3, r1
                BEQ     %BT0

            [ {ENDIAN} = "big"
                SUB     r0, #4
                LSR     r3, r2, #24
                BEQ     exit
                ADD     r0, #1
                LSL     r3, r2, #8
                LSR     r3, #24
                BEQ     exit
                ADD     r0, #1
                LSL     r3, r2, #16
                LSR     r3, #24
                BEQ     exit
                ADD     r0, #1
            |
                SUB     r0, #4
                LSL     r3, r2, #24
                BEQ     exit
                ADD     r0, #1
                LSR     r3, r2, #8
                LSL     r3, #24
                BEQ     exit
                ADD     r0, #1
                LSR     r3, r2, #16
                LSL     r3, #24
                BEQ     exit
                ADD     r0, #1
            ]

exit            ADD     r0, r12
                Return  , "", LinkNotStacked

                ALIGN
eights_word     DCD     &80808080

                END
