        AREA    Jump, CODE, READONLY    ; name this block of code

num     EQU     2               ; Number of entries in jump table

        ENTRY                   ; mark the first instruction to call

start
        MOV     r0, #0  ; set up the three parameters
        MOV     r1, #3
        MOV     r2, #2
        BL      arithfunc       ; call the function

stop
        MOV     r0, #0x18               ; angel_SWIreason_ReportException
        LDR     r1, =0x20026    ; ADP_Stopped_ApplicationExit
        SWI     0x123456                ; Angel semihosting ARM SWI

arithfunc                       ; label the function
        CMP     r0, #num        ; Treat function code as unsigned integer
        BHS     DoAdd           ; If code is >=2 then do operation 0.

        ADR     r3, JumpTable   ; Load address of jump table
        LDR     pc, [r3,r0,LSL#2]       ; Jump to the appropriate routine

JumpTable
        DCD     DoAdd
        DCD     DoSub

DoAdd
        ADD     r0, r1, r2      ; Operation 0, >1
        MOV     pc, lr  ; Return

DoSub
        SUB     r0, r1, r2      ; Operation 1
        MOV     pc,lr           ; Return

        END             ; mark the end of this file

