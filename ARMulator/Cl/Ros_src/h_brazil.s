;;; riscos/heapsuppt.s: OS interface for Clibrary / shared kernel
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

; The version number below bears no resemblance to any release version
; numbers.  It must be incremented by one each time a non-downwards compatible
; version of the library is produced (ie one where the new stubs will not
; function correctly with an older library).

LibraryVersionNumber            *       4

OSBase          *       &1800000

X               EQU     1:SHL:17

; SWIs common to Brazil and Arthur
WriteC          EQU     X+0
WriteS          EQU     X+1
Write0          EQU     X+2
NewLine         EQU     X+3
ReadC           EQU     X+4
CLI             EQU     X+5
Byte            EQU     X+6
Word            EQU     X+7
File            EQU     X+8
Args            EQU     X+9
BGet            EQU     X+&a
BPut            EQU     X+&b
Multiple        EQU     X+&c
Open            EQU     X+&d
ReadLine        EQU     X+&e
Control         EQU     X+&f
GetEnv          EQU     X+&10
Exit            EQU     X+&11
SetEnv          EQU     X+&12
IntOn           EQU     X+&13
IntOff          EQU     X+&14
CallBack        EQU     X+&15
EnterSVC        EQU     X+&16
BreakPt         EQU     X+&17
BreakCtrl       EQU     X+&18
UnusedSWI       EQU     X+&19
KUpdateMEMC     EQU     X+&1A
SetCallBack     EQU     X+&1B
Mouse           EQU     X+&1C

WriteI          EQU     X+&100

; Arthur only SWIs
Module          EQU     X+&1E
ChangeEnv       EQU     X+&40
GenerateError   EQU     &2B               ; X form not sensible
ReadVarVal      EQU     X+&23
SetVarVal       EQU     X+&24
ExitAndDie      EQU     X+&4D

Lib_Init        EQU     &80680            ; shared library initialise

FPE_Version     EQU     X+&40480

Module_Claim    EQU     6                 ; Module reason codes
Module_Free     EQU     7
Module_Extend   EQU     13

Wimp_ReadSysInfo EQU X+&400f2
Wimp_SlotSize    EQU X+&400ec

; r0 values for swi ChangeEnv
Env_MemoryLimit         EQU     0
Env_UIHandler           EQU     1
Env_PAHandler           EQU     2
Env_DAHandler           EQU     3
Env_AEHandler           EQU     4
Env_ErrorHandler        EQU     6
Env_CallBackHandler     EQU     7
Env_EscapeHandler       EQU     9
Env_EventHandler        EQU     10
Env_ExitHandler         EQU     11
Env_ApplicationSpace    EQU     14
Env_UpCallHandler       EQU     16

Application_Base EQU    &8000

        END
