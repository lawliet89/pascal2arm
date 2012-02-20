        ; Copyright (c) 1995, Advanced RISC Machines Ltd
        ; All Rights Reserved

        
        ; $Revision: 1.1 $
        ;   $Author: mgray $
        ;     $Date: 1996/08/22 15:42:28 $

                GET listopts.s
                GET lolevel.s
                
                AREA  BackChannelCode,  CODE, READONLY, ALIGN=4

; --- BCPutByte ---
;
; On entry:     R0 == byte to send
;
; On exit:      --
;
; Use:          Send a byte up to the ev5 terminal

                EXPORT  BCPutByte
BCPutByte       ROUT

                STMFD   R13!,{R0-R5,R12,R14}            ;Stack registers
                LDR     R12,=(_BackChannel - ROADDR + ROMBase) ;Pt. to the ROM

00              MOV     R0,R0,LSL #24                   ;Move byte to top
                MOV     R1,#4                           ;We're sending MSB
                ADR     R3,bc_send00                    ;Address of send 0
                ADR     R4,bc_send01                    ;Address of send 1
                MOV     R5,#8                           ;8 bits to do

                ; --- Wait till PC is ready ---

01              LDR     R14,bc_status                   ;Read the status byte
                AND     R14,R14,#&c2                    ;Get interesting bits
                CMP     R14,#&80                        ;Ready to receive?
                BNE     %b01                            ;No -- keep waiting

                ; --- Do the read (write) ---

                MOVS    R0,R0,LSL #1                    ;Shift bit into carry
                LDRCCB  R14,[R3,R1]                     ;Either send 0...
                LDRCSB  R14,[R4,R1]                     ;...or 1
                MOV     R1,#0                           ;Not longer MSB

                ; --- Do all the bits ---

                SUBS    R5,R5,#1                        ;Reduce the count
                BNE     %b01                            ;Do the rest

                LDMFD   R13!,{R0-R5,R12,PC}             ;Return to caller

                NOP                                     ;Just in case
        
                ; --- Set up the backchannel block ---

                ALIGN   16
                EXPORT  _BackChannel
_BackChannel    DCB     0
                DCB     &40,0,0,0,0,0,0,0
                DCB     0,0
                DCB     0,0
                DCB     0,0
                DCB     0

                ; --- Define it's offsets ---

                ^       0,R12
bc_pcdata       #       1
bc_status       #       8
bc_send00       #       2
bc_send01       #       2
bc_send10       #       2
bc_send11       #       2

                END
