
STACK_TOP	SPACE 0x1000
STACK_BASE
	ALIGN
;--------------------------------------------------------------------------------
; Program Code
;--------------------------------------------------------------------------------
	AREA RESET, CODE
	ENTRY
	LDR SP, =STACK_BASE ; Set Stack Pointer to Stack_Base
