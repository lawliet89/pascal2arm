
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
		LDR R1, [R2]			; Load value of heap pointer
		MOV R0, R1				; Give new address to R0
		ADD R1, R1, #4			; Increment heap pointer
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
