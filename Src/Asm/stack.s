
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
