; Default header for generated assembly
	AREA ProgramData, DATA, READWRITE

SWI_WriteC	EQU &0 ; output the character in r0 to the screen
SWI_Write0	EQU &2 ; Write a null (0) terminated buffer to the screen
SWI_ReadC	EQU &4 ; input character into r0
SWI_Exit	EQU &11 ; finish program

; Rest of user's Data 
