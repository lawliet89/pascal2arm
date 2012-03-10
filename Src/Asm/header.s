
;--------------------------------------------------------------------
; Unsigned division macro
; From http://www.keil.com/support/man/docs/armasm/armasm_CEGECDGD.htm
; $Bot	The register that holds the divisor.
; $Top	The register that holds the dividend before the instructions are executed. After the instructions are executed, it holds the remainder.
; $Div	The register where the quotient of the division is placed. It can be NULL ("") if only the remainder is required.
; $Temp	A temporary register used during the calculation.
;
; Usage: 
; ratio  DivMod  R0,R5,R4,R2
;--------------------------------------------------------------------
        MACRO
$Lab    DivMod  $Div,$Top,$Bot,$Temp
        ASSERT  $Top <> $Bot             ; Produce an error message if the
        ASSERT  $Top <> $Temp            ; registers supplied are
        ASSERT  $Bot <> $Temp            ; not all different
        IF      "$Div" <> ""
            ASSERT  $Div <> $Top         ; These three only matter if $Div
            ASSERT  $Div <> $Bot         ; is not null ("")
            ASSERT  $Div <> $Temp        ;
        ENDIF
$Lab
        MOV     $Temp, $Bot              ; Put divisor in $Temp
        CMP     $Temp, $Top, LSR #1      ; double it until
90      MOVLS   $Temp, $Temp, LSL #1     ; 2 * $Temp > $Top
        CMP     $Temp, $Top, LSR #1
        BLS     %b90                     ; The b means search backwards
        IF      "$Div" <> ""             ; Omit next instruction if $Div is null
            MOV     $Div, #0             ; Initialize quotient
        ENDIF
91      CMP     $Top, $Temp              ; Can we subtract $Temp?
        SUBCS   $Top, $Top,$Temp         ; If we can, do so
        IF      "$Div" <> ""             ; Omit next instruction if $Div is null
            ADC     $Div, $Div, $Div     ; Double $Div
        ENDIF
        MOV     $Temp, $Temp, LSR #1     ; Halve $Temp,
        CMP     $Temp, $Bot              ; and loop until
        BHS     %b91                     ; less than divisor
        MEND

; Default header for generated assembly
	AREA ProgramData, DATA, READWRITE, NOINIT

SWI_WriteC	EQU &0 ; output the character in r0 to the screen
SWI_Write0	EQU &2 ; Write a null (0) terminated buffer to the screen
SWI_ReadC	EQU &4 ; input character into r0
SWI_Exit	EQU &11 ; finish program

; Rest of user's Data 
