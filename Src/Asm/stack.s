
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
