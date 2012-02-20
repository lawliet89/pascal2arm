	AREA ifThenForElseFor,CODE,READWRITE

	ENTRY

	MOV R0, #0x0
	ADD R0, R0, #0x3
	LDR R12, =0xe001c020
	STR R0, [R12]
	LDR R2, [R12]
	MOV R0, #0x0
	ADD R0, R0, #0x6
	CMP R2, R0
	BLT else1
	MOV R0, #0x0
	ADD R0, R0, #0x1
	LDR R12, =0xe001c024
	STR R0, [R12]
	SUB R0, R0, #0x2
	STR R0, [R12]
for1
	LDR R12, =0xe001c024
	LDR R10, [R12]
	ADD R10, R10, #1
	STR R10, [R12]
	MOV R11, #0x5
	CMP R10, R11
	BEQ forend1
	MOV R0, #0x0
	LDR R12, =0xe001c028
	LDR R1, [R12]
	ADD R0, R0, R1
	ADD R0, R0, #0x1
	STR R0, [R12]
	B for1
forend1
	B then1
else1
	MOV R0, #0x0
	ADD R0, R0, #0x5
	LDR R12, =0xe001c024
	STR R0, [R12]
	SUB R0, R0, #0x2
	STR R0, [R12]
for2
	LDR R12, =0xe001c024
	LDR R10, [R12]
	ADD R10, R10, #1
	STR R10, [R12]
	MOV R11, #0x9
	CMP R10, R11
	BEQ forend2
	MOV R0, #0x0
	LDR R12, =0xe001c028
	LDR R1, [R12]
	ADD R0, R0, R1
	ADD R0, R0, #0x2
	STR R0, [R12]
	B for2
forend2
then1

	END