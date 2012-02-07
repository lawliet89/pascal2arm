
        AREA     ARMex, CODE, READWRITE
                                ; Name this block of code ARMex
        ENTRY                   ; Mark first instruction to execute
		; do not change text before this comment, which is a fixed template

			ADR 	r2, DATA 
CHECKSUM64
  			MOV 	r3, #0   		; bits 31:0 of sum
  			MOV 	r4, #0       	; bits 63:32 of sum
  			ADD	r6, r2, #32			; address of first word not added (4 words added)
LOOP
  			LDR 	r0, [r2]      	; load 31:0 of current 64 bit word
  			ADD 	r2, r2, #4		; move r2 to 63:32 of current word
			LDR 	r1, [r2]  		; load it
  			ADD 	r2, r2, #4		; move r2 to 31:0 of next 64 bit word
  			ADDS 	r3, r3, r0 		; 31:0 of 64 bit addition, set C
  			ADCS 	r4, r4, r1 		; add bits 63:32, with C
			CMP 	r2, r6			; see if next 64 bits should be added
  			BNE LOOP 				; if not finished add next 64 bits
STOP		B STOP					; loop forever

			; the data below is defined as a sequence of 32 bit words (DCD)
DATA	    DCD		0x11111111,0xffffffff, 0x22222222, 0xeeeeeeee, 0xffffffff, 0x55555555
			DCD		0x66666666, 0xcccccccc

		; so not change text after this comment, which is a fixed template
        END                     ; Mark end of file

