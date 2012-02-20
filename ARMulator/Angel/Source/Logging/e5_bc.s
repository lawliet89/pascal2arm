        ; Copyright (c) 1995, Advanced RISC Machines Limited. 
        ; All Rights Reserved.
        
        ; Back channel support for E5 ROM emulator
        
        ; $Revision: 1.1 $
        ;   $Author: mgray $
        ;     $Date: 1996/08/22 15:42:26 $

        GBLL    DBG
DBG     SETL    {FALSE}

; Back channel area definitions

bc_base EQU     0x0101ffc0

bc_pcdata       EQU     0
bc_status       EQU     32
bc_send00       EQU     48
bc_send01       EQU     52
bc_send10       EQU     56
bc_send11       EQU     60

        AREA |backchan|, CODE, READONLY


        [ DBG
        IMPORT  debug_putbyte_hex
        IMPORT  debug_putchar
        ]



; Wait for the number of centiseconds in r0
; r0, r1 and r2 are corrupted


Wait
        MOV     r0, r0, LSL #12
loop
        SUBS    r0, r0, #1
        BPL     loop
        MOV     pc, lr



        EXPORT  bc_state
bc_state
        STMFD   sp!, {r0-r3,lr}
        BL      SetLEDs
        LDMFD   sp!, {r0-r3,pc}


        [ DBG
got_byte
        STMFD   sp!, {r1,lr}    
        MOV     r1, r0
        MOV     r0, #32
        BL      debug_putchar
        MOV     r0, #91
        BL      debug_putchar
        MOV     r0, r1
        BL      debug_putbyte_hex
        MOV     r0, #93
        BL      debug_putchar
        MOV     r0, #32
        BL      debug_putchar
        MOV     r0, r1
        LDMFD   sp!, {r1,pc}    
        ]

; int bc_get (void)
; waits for & reads a byte from the romulator backchannel
; returns byte in r0
; preserves all other regs

        EXPORT  bc_get
        IMPORT  timer_poll
bc_get
        STMFD   sp!, {r1-r2,r4,lr}

        [ DBG
        MOV     r0, #33
        BL      debug_putchar
        ]

        ; wait for a byte to appear while channel is 'ok'
wait_for_byte
;       BL      timer_poll      ; corrupts r4

        LDR     r1, =bc_base
        LDRB    r2, [r1, #bc_status]
        AND     r2, r2, #&c1
        CMP     r2, #&81
        BNE     wait_for_byte

        ; get byte
        LDRB    r0, [r1, #bc_pcdata]
        [ DBG
        BL      got_byte
        ]
        LDMFD   sp!, {r1-r2,r4,pc}


; get byte from romulator backchannel without waiting
; returns byte in r0 if found, otherwise returns -1
; all other regs preserved

        EXPORT  bc_get_nowait
bc_get_nowait
        STMFD   sp!, {r1-r2,lr}

        ; wait for a byte to appear while channel is 'ok'
        LDR     r1, =bc_base
        LDRB    r2, [r1, #bc_status]
        AND     r2, r2, #&c1
        CMP     r2, #&81
        MOVNE   r0, #-1

        ; get byte
        LDREQB  r0, [r1, #bc_pcdata]
        [ DBG
        BNE     %FT01
        MOV     r1, r0
        MOV     r0, #35         ; #
        BL      debug_putchar
        MOV     r0, r1
        BL      got_byte
01
        ]
        LDMFD   sp!, {r1-r2,pc}


; void bc_put (int byte)
; writes byte r0 to the romulator backchannel
; preserves all regs
        EXPORT  bc_put
bc_put
        STMFD   sp!, {r1,lr}
        MOV     r1, #0
        BL      bc_doput
        LDMFD   sp!, {r1,pc}


; void bc_put_clear_reads (int byte)
; writes byte r0 to the romulator backchannel
; preserves all regs
        EXPORT  bc_put_clear_reads
bc_put_clear_reads
        STMFD   sp!, {r1,lr}
        MOV     r1, #1
        BL      bc_doput
        LDMFD   sp!, {r1,pc}


; void bc_doput (int byte)
; writes byte r0 to the romulator backchannel
; if r1 is <> 0 then any data incoming on the backchannel is cleared
; preserves all regs

        EXPORT  bc_doput
bc_doput
        STMFD   sp!, {r2-r5,lr}
        MOV     r5, r1

        [ DBG
        MOV     r1, r0
        MOV     r0, #32
        BL      debug_putchar
        MOV     r0, #&7b
        BL      debug_putchar
        MOV     r0, r1
        BL      debug_putbyte_hex
        ]

        ; wait until the channel is 'ok'
wait_for_ok
        ; poll the timer
;       MOV     r2, r0
;       BL      timer_poll       ; corrupts r0, r1, r4
;       MOV     r0, r2

        LDR     r1, =bc_base
        LDRB    r2, [r1, #bc_status]

       [ {TRUE}
        MOVS    r5, r5
        BEQ     %FT01
        AND     r3, r2, #&c1            ; check for a character to read
        CMP     r3, #&81
        LDREQB  r3, [r1, #bc_pcdata]    ; throw it away
01
       ]

        AND     r3, r2, #&c2
        CMP     r3, #&80
        BNE     wait_for_ok

        ; send bit 7
        TST     r0, #&80
        LDREQB  r2, [r1, #bc_send10]
        LDRNEB  r2, [r1, #bc_send11]

        ; get mask for next bit to send
        MOV     r4, #&40

putloop
        LDR     r1, =bc_base

        ; wait for 'ok' status
        LDRB    r2, [r1, #bc_status]

       [ {TRUE}
        MOVS    r5, r5
        BEQ     %FT01
        AND     r3, r2, #&c1            ; check for a character to read
        CMP     r3, #&81
        LDREQB  r3, [r1, #bc_pcdata]    ; throw it away
01
       ]

        AND     r3, r2, #&c2
        CMP     r3, #&80
        BNE     putloop

        ; send this bit as a 0 or a 1
        TST     r0, r4
        LDREQB  r2, [r1, #bc_send00]
        LDRNEB  r2, [r1, #bc_send01]

        ; move to next bit to send - return if finished
        MOVS    r4, r4, LSR #1
        BNE     putloop

        [ DBG
        MOV     r1, r0
        MOV     r0, #&7d
        BL      debug_putchar
        MOV     r0, #32
        BL      debug_putchar
        MOV     r0, r1
        ]

        ; & return
        MOV     r1, r5
        LDMFD   sp!, {r2-r5,pc}
        


        END

