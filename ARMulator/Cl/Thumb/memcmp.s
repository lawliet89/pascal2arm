;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:27:28 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                Function memcmp

                MOV     r12, r7
                MOV     r3, r0
                ORR     r0, r1
                LSL     r0, #&1e
                BNE     memcmp_u

                SUB     r2, #4
                BCC     memcmp_tail

0               LDMIA   r3!, {r0}
                LDMIA   r1!, {r7}
                SUB     r0, r7
                BNE     base_readj
                SUB     r2, #4
                BCS     %BT0

memcmp_tail     ADD     r2, #3
                BCC     exit
                B       %FT0

base_readj      SUB     r3, #4
                SUB     r1, #4
                ADD     r2, #3

0               LDRB    r0, [r3]
                LDRB    r7, [r1]
                SUB     r0, r7
                BNE     exit
                ADD     r3, #1
                ADD     r1, #1
memcmp_u        SUB     r2, #1
                BCS     %BT0
                MOV     r0, #0

exit            MOV     r7, r12
                Return  , "", LinkNotStacked

                END
