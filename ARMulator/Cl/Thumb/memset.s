;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:28:04 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                Function        memset

                MOV     r12, r0
                LSL     r3, r0, #&1e
                BEQ     dst_aligned

0               SUB     r2, #1
                BCC     exit
                STRB    r1, [r0]
                ADD     r0, #1
                LSL     r3, r0, #&1e
                BNE     %BT0

dst_aligned     SUB     r2, #8
                BCC     dst_tail

                LSL     r3, r1, #8
                ORR     r1, r3
                LSL     r3, r1, #16
                ORR     r1, r3
                MOV     r3, r1

0               STMIA   r0!, {r1, r3}
                SUB     r2, #8
                BCS     %BT0

dst_tail        ADD     r2, #7
                BCC     exit

0               STRB    r1, [r0, r2]
                SUB     r2, #1
                BCS     %BT0

exit            MOV     r0, r12
                Return  , "", LinkNotStacked

                END
