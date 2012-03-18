
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

HeapPtr	DCD 0	;Heap pointer
; Rest of user's Data 
GLOBAL_mypointer	DCD 0

HEAP		SPACE 1000
	ALIGN
STACK_TOP	SPACE 1000
STACK_BASE
	ALIGN
STACK_PADDING	SPACE 100	;Just in case
	ALIGN
;--------------------------------------------------------------------------------
; Program Code
;--------------------------------------------------------------------------------
	AREA RESET, CODE
	ENTRY
	LDR R13, =STACK_BASE-4 ; Set Stack Pointer to Stack_Base
	LDR R12, =HEAP	; Load start of Heap address
	LDR R11, =HeapPtr	; //Load address of HeapPtr
	STR R12, [R11]		; Initialise HeapPtr
; User Code
;		 R0 is now forced to be GLOBAL_mypointer
	BL NEW ; NEW procedure call
;		 R1 is now GLOBAL_mypointer dereferenced
	MOV R1, #10 ;Line 9
	LDR R12, =GLOBAL_mypointer; Force storage of variable
	STR R0, [R12]
	MOV R0, R1; Moving register temporarily
	BL PRINTR0_ ;Print integer
;		 R0 is now forced to be GLOBAL_mypointer
	BL DISPOSE ; DISPOSE procedure call
	SWI SWI_Exit
;--------------------------------------------------------------------------------
; Function and Procedures
;--------------------------------------------------------------------------------
	AREA Functions, CODE
;"Library functions"

;--------------------------------------------------------------------------------
; Subroutine to print contents of register 0 in decimal
; By Dr. Tom Clarke
;--------------------------------------------------------------------------------
; ** REGISTER DESCRIPTION ** 
; R0 byte to print, carry count
; R1 number to print
; R2 ... ,thousands, hundreds, tens, units.
; R3 addresses of constants, automatically incremented
; R4 holds the address of units
; Allocate 10^9, 10^8, ... 1000, 100, 10, 1

CMP1_	DCD 1000000000
CMP2_	DCD 100000000
CMP3_	DCD 10000000
CMP4_	DCD 1000000
CMP5_	DCD 100000
CMP6_	DCD 10000
CMP7_	DCD 1000
CMP8_	DCD 100
CMP9_	DCD 10
CMP10_	DCD 1
; Entry point

PRINTR0_	STMED R13!,{r0-r4,r14}
		CMP R0, #0x0
		MOVEQ R0, #0x30
		SWIEQ SWI_WriteC
		BEQ PrintNewL
		MOV R1, R0
		; Is R1 negative?
		CMP R1,#0
		BPL LDCONST_
		RSB R1, R1, #0 ;Get 0-R1, ie positive version of r1
		MOV R0, #'-'
		SWI SWI_WriteC

LDCONST_ 	ADR R3, CMP1_ ;Used for comparison at the end of printing

		ADD R4, R3, #40 ;Determine final address (10 word addresses +4 because of post-indexing)
		; Take as many right-0's as you can...

NEXT0_ 		LDR R2, [R3], #4 
		CMP R2, R1
		BHI NEXT0_
		;Print all significant characters

NXTCHAR_ 	MOV R0, #0
SUBTRACT_	CMP R1, R2
		SUBPL R1, R1, R2
		ADDPL R0,R0, #1
		BPL SUBTRACT_

		;Output number of Carries
		ADD R0, R0, #'0'
		SWI SWI_WriteC
		; Get next constant, ie divide R2/10 

		LDR R2, [R3], #4 

		;If we have gone past L10, exit function; else take next character 
		CMP R3, R4;

		BLE NXTCHAR_;
		; Print a line break
PrintNewL	MOV R0, #'\n'
		SWI SWI_WriteC
		LDMED R13!,{r0-r4,r15} ;Return

;-------------------------------------------------------------------
; New Procedure 
; R0 - Pointer to initialise - Only supports one word pointers
; NOTE: Does not reuse disposed addresses. Simple allocator
;-------------------------------------------------------------------

NEW		STMED R13!, {R1,R2,R14}
		LDR R2, =HeapPtr		; Address of heap pointer
		LDR R1, [R2], #4		; Load value of heap pointer and increment
		MOV R0, R1				; Give new address to R0
		STR R1, [R2]			; Save heap pointer
		LDMED R13!, {R1,R2,R15}	; Return
		
;-------------------------------------------------------------------
; Dispose Procedure 
; R0 - Pointer to dispose - Only supports one word pointers
; NOTE: Does not reuse disposed addresses. Simple deallocator
;-------------------------------------------------------------------

DISPOSE	MOV R0, #0		; Simply resets the pointer
		MOV R15, R14	; Return

;-------------------------------------------------------------------
; User functions and procedures
;-------------------------------------------------------------------

	END