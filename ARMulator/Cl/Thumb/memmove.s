;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:27:37 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                Function memcpy
                Function memmove

                SUB     r3, r0, r1
                CMP     r3, r2          ; Check for overlapping objects
                BCC     up              ; Unsigned pun so dest < src always
                                        ; copied down.

; dest < src or non overlapping objects => copy down
; -----------------------------------------------
dn              MOV     r12, r0
                LSL     r3, r1, #&1e
                BEQ     src_aligned

dn_alignsrc     SUB     r2, #1
                BCC     exit
                LDRB    r3, [r1]
                STRB    r3, [r0]
                ADD     r0, #1
                ADD     r1, #1
                LSL     r3, r1, #&1e
                BNE     dn_alignsrc

src_aligned     LSL     r3, r0, #&1e
                BNE     dn_unaligned

dn_aligned      SUB     r2, #16
                BCC     dn_aligned4
                PUSH    {r4, r5, r7}
0               LDMIA   r1!, {r3, r4, r5, r7}
                STMIA   r0!, {r3, r4, r5, r7}
                SUB     r2, #16
                BCS     %BT0
                POP     {r4, r5, r7}

dn_aligned4     ADD     r2, #12
                BCC     dn_tail
0               LDMIA   r1!, {r3}
                STMIA   r0!, {r3}
                SUB     r2, #4
                BCS     %BT0
                B       dn_tail

dn_unaligned4   LDMIA   r1!, {r3}
            [ {ENDIAN} = "big"
                STRB    r3, [r0, #3]
                LSR     r3, #8
                STRB    r3, [r0, #2]
                LSR     r3, #8
                STRB    r3, [r0, #1]
                LSR     r3, #8
                STRB    r3, [r0, #0]
            |
                STRB    r3, [r0, #0]
                LSR     r3, #8
                STRB    r3, [r0, #1]
                LSR     r3, #8
                STRB    r3, [r0, #2]
                LSR     r3, #8
                STRB    r3, [r0, #3]
            ]
                ADD     r0, #4
dn_unaligned    SUB     r2, #4
                BCS     dn_unaligned4

dn_tail         ADD     r2, #3
                BCC     exit
0               LDRB    r3, [r1]
                STRB    r3, [r0]
                ADD     r1, #1
                ADD     r0, #1
                SUB     r2, #1
                BCS     %BT0

exit            MOV     r0, r12
                Return  , "", LinkNotStacked

; dest > src and objects overlap => copy up
; (only copy up for objects that actually overlap as since there
; is no LDMDx instruction in Thumb copy up is inherently less
; effecient)
; -----------------------------------------------
; ensure we do at least as well as C version
up              MOV     r3, r0
                ORR     r3, r1
                ORR     r3, r2
                LSL     r3, #&1e
                BNE     up_unaligned

; We know R2 > 0 as otherwise we would have not have taken the BCC up
up_aligned      SUB     r2, #4
                LDR     r3, [r1, r2]
                STR     r3, [r0, r2]
                BNE     up_aligned
                Return  , "", LinkNotStacked

up_unaligned    SUB     r2, #1
                LDRB    r3, [r1, r2]
                STRB    r3, [r0, r2]
                BNE     up_unaligned
                Return  , "", LinkNotStacked

                END
