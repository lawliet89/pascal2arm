; Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UND        EQU     0x1B
Mode_SYS        EQU     0x1F

I_Bit           EQU     0x80            ; when I bit is set, IRQ is disabled
F_Bit           EQU     0x40            ; when F bit is set, FIQ is disabled


;// <h> Stack Configuration (Stack Sizes in Bytes)
;//   <o0> Undefined Mode      <0x0-0xFFFFFFFF:8>
;//   <o1> Supervisor Mode     <0x0-0xFFFFFFFF:8>
;//   <o2> Abort Mode          <0x0-0xFFFFFFFF:8>
;//   <o3> Fast Interrupt Mode <0x0-0xFFFFFFFF:8>
;//   <o4> Interrupt Mode      <0x0-0xFFFFFFFF:8>
;//   <o5> User/System Mode    <0x0-0xFFFFFFFF:8>
;// </h>

UND_Stack_Size  EQU     0x00000000
SVC_Stack_Size  EQU     0x00000080
ABT_Stack_Size  EQU     0x00000000
FIQ_Stack_Size  EQU     0x00000000
IRQ_Stack_Size  EQU     0x00000080
USR_Stack_Size  EQU     0x00000000

ISR_Stack_Size  EQU     (UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
                         FIQ_Stack_Size + IRQ_Stack_Size)

        		AREA     RESET, CODE
				ENTRY
;  Dummy Handlers are implemented as infinite loops which can be modified.

Vectors         LDR     PC, Reset_Addr         
                LDR     PC, Undef_Addr
                LDR     PC, SWI_Addr
                LDR     PC, PAbt_Addr
                LDR     PC, DAbt_Addr
                NOP                            ; Reserved Vector 
                LDR     PC, IRQ_Addr
;               LDR     PC, [PC, #-0x0FF0]     ; Vector from VicVectAddr
                LDR     PC, FIQ_Addr

ACBASE			DCD		P0COUNT
SCONTR			DCD		SIMCONTROL

Reset_Addr      DCD     Reset_Handler
Undef_Addr      DCD     Undef_Handler
SWI_Addr        DCD     SWI_Handler
PAbt_Addr       DCD     PAbt_Handler
DAbt_Addr       DCD     DAbt_Handler
                DCD     0                      ; Reserved Address 
FIQ_Addr        DCD     FIQ_Handler

Undef_Handler   B       Undef_Handler
SWI_Handler     B       SWI_Handler
PAbt_Handler    B       PAbt_Handler
DAbt_Handler    B       DAbt_Handler
FIQ_Handler     B       FIQ_Handler


				AREA 	ARMuser, CODE,READONLY

IRQ_Addr        DCD     ISR_FUNC1
EINT2			EQU 	16
Addr_VicIntEn	DCD		0xFFFFF010	 	; set to (1<<EINT0)
Addr_EXTMODE	DCD 	0xE01FC148   	; set to 1
Addr_PINSEL0	DCD		0xE002C000		; set to 2_1100
Addr_EXTINT		DCD		0xE01FC140

;  addresses of two registers that allow fast input

Addr_IOPIN		DCD		0xE0028000


; Initialise Interrupt System
;  ...
ISR_FUNC1		STMED	R13!, {R0,R1}
				MOV 	R0, #(1 << 2) 	   ; bit 2 of EXTINT
				LDR 	R1,	Addr_EXTINT	   
				STR		R0, [R1]		   ; EINT2 reset interrupt
				LDMED	R13!, {R0,R1}
				B 		ISR_FUNC

Reset_Handler
; PORT0.1 1->0 triggers EINT0 IRQ interrupt
				MOV R0, #(1 << EINT2)
				LDR R1, Addr_VicIntEn
				STR R0, [R1]
				MOV R0, #(1 << 30)
				LDR R1, Addr_PINSEL0
				STR R0, [R1]
				MOV R0, #(1 << 2)
				LDR R1, Addr_EXTMODE
				STR R0, [R1]

;  Setup Stack for each mode

                LDR     R0, =Stack_Top

;  Enter Undefined Instruction Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_UND:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #UND_Stack_Size

;  Enter Abort Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_ABT:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #ABT_Stack_Size

;  Enter FIQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_FIQ:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #FIQ_Stack_Size

;  Enter IRQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #IRQ_Stack_Size

;  Enter Supervisor Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SVC:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #SVC_Stack_Size
				B 		START
;----------------------------DO NOT CHANGE ABOVE THIS COMMENT--------------------------------
;--------------------------------------------------------------------------------------------
;--------------------------------------------------------------------------------------------
				; constant data used in counting loop

START		    ; initialse	 counting loop

				; main couting loop loops forever, interrupted at end of simulation
LOOP		
				B 		LOOP			; For skeleton code only, replace with counting loop which
										; branches to LOOP_END on termination of loop



ISR_FUNC								; Interrupt must set variable to terminate main loop
				B 		LOOP_END		; for skeleton code only		
										; replace this by return from interrupt
										; branch to LOOP_END will be at end of LOOP code

;--------------------------------------------------------------------------------------------
; PARAMETERS TO CONTROL SIMULATION, VALUES MAY BE CHANGED TO IMPLEMENT DIFFERENT TESTS
;--------------------------------------------------------------------------------------------
SIMCONTROL
SIM_TIME 		DCD  	10000	  ; length of simulation in cycles (100MHz clock)
P0_PERIOD		DCD   	40        ; bit 0 input period in cycles
P1_PERIOD		DCD   	34		  ; bit 8 input period in cycles
P2_PERIOD		DCD  	44		  ; bit 16 input period	in cycles
P3_PERIOD		DCD		38		  ; bit 24 input period	in cycles
;---------------------DO NOT CHANGE AFTER THIS COMMENT---------------------------------------
;--------------------------------------------------------------------------------------------
;--------------------------------------------------------------------------------------------
LOOP_END		MOV R0, #0x7f00
				LDR R0, [R0] 	; read memory location 7f00 to stop simulation
STOP			B 	STOP
;-----------------------------------------------------------------------------
 				AREA	DATA, READWRITE

P0COUNT			DCD		0
P1COUNT			DCD		0
P2COUNT			DCD		0
P3COUNT			DCD		0
;------------------------------------------------------------------------------			
                AREA    STACK, NOINIT, READWRITE, ALIGN=3

Stack_Mem       SPACE   USR_Stack_Size
__initial_sp    SPACE   ISR_Stack_Size

Stack_Top


        		END                     ; Mark end of file

