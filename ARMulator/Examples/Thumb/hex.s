; All assembler files must have at least one AREA directive at
; the start. The AREA directive passes information to the linker
; telling it where to place the code in memory (for example,
; READONLY areas would be placed in a ROM segment).
;
        AREA    hex, CODE, READONLY

; Some definitions for system calls or SWIs supported by
; 'armos'
;
OS_WriteC       EQU     0x00    ; Write a character
OS_Exit         EQU     0x11    ; Exit program

; ENTRY tells the linker where the entry point of an image is.
; The linker in turn encodes this information in the image
; header so the armulator knows where the application image
; start.
;
        ENTRY

; All applications are entered initially in ARM state so we
; must write a couple of lines of ARM assembler to switch to
; Thumb state. We tell the assembler we are going to do this
; with the 'CODE32' directive.
;
        CODE32

; Use the ARM BX instruction to switch into Thumb state. The
; BX instruction will branch to the Thumb entry point. Add
; one to the entry point to force the switch into Thumb state.
;
        ADR     R0, Thumb_Entry+1
        BX      R0

; The next section of code is Thumb code so tell the assembler
; to assemble it as Thumb.
;
        CODE16

Thumb_Entry
; Now load several values in turn into R0 and call the Hex
; Print routine.
;
        MOV     R0, #0xED
        BL      Hex_Print

; The number 0xAA55AA55 is too big to load into a register
; using an immediate constant so we tell the assembler to
; load it from memory by placing an '=' symbol before the
; number. We do not need to tell the assembler where to
; store the number in memory, it will decide that for us.
;
        LDR     R0, =0xAA55AA55
        BL      Hex_Print

; To load a negative number it is often easier to load the
; postive value and then negate it as negative numbers cannot
; be loaded using immediate constants.
;
        MOV     R0, #10
        NEG     R0, R0
        BL      Hex_Print

; Now exit the program cleanly
;
        SWI     OS_Exit

Hex_Print
; Save registers used by this routine. Also save the link
; register which contains the return address as this may be
; destroyed by the call to SWI OS_WriteC if the code is
; executing in supervisor mode.
        PUSH    {R0, R1, R2, LR}

        MOV     R1, R0          ; Enter with value to print in R0
                                ; Mov it to R1 as R0 needed for call
                                ; to OS_WriteC.
        MOV     R2, #8          ; Loop counter for 8 digits
Loop1   LSR     R0, R1, #28     ; Extract digits from R1 starting with
                                ; the most significant digit.
        LSL     R1, #4          ; Shift up the digit to be used next
                                ; time round the loop.
        CMP     R0, #10         ; Convert R0 to a hex digit
        BLT     Loop2
        ADD     R0, #'A'-'0'-10 ; Value >10 => add in extra to convert
                                ; to range 'A' to 'F'.
Loop2   ADD     R0, #'0'        ; Add in ASCII value for '0'.
        SWI     OS_WriteC       ; Write the hex digit
        SUB     R2, #1          ; Loop for each digit
        BNE     Loop1           ; (SUB implicitly sets the condition codes)
        MOV     R0, #0x0D
        SWI     OS_WriteC       ; Write a CR
        MOV     R0, #0x0A
        SWI     OS_WriteC       ; Followed by a newline
        POP     {R0, R1, R2, PC}; Recover registers and return

; An END directive is always needed.
;
        END
