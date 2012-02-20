;;; riscos/traphandle.s: riscos handlers for abort, error and the like
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     objmacs.s
        GET     h_brazil.s
        GET     h_errors.s
        GET     h_hw.s

        Module  traphandler

        DataArea

StaticData

__rt_registerDump       ExportedVariable        0
registerDump            Variable        16

; Handler values on entry, to be restored on exit
oldAbortHandlers        Variable        4

oldExitHandler          Variable        2       ; handler, r12
oldMemoryLimit          Variable

oldErrorHandler         Variable                ; handler, r0, buffer
oldErrorR0              Variable
oldErrorBuffer          Variable

oldCallBackHandler      Variable        3       ; handler, r12, buffer
oldEscapeHandler        Variable        2       ; handler, r12

oldUpCallHandler        Variable        2       ; handler, r12

callbackInactive        VariableByte
handlerFlags            VariableByte
hf_Debuggee     *       1

hadEscape               VariableByte    ; 0 if not, 1 if so
__rt_escapeSeen         ExportedVariableByte 0
escapeSeen              VariableByte

IIHandlerInData         Variable        3
PAHandlerInData         Variable        3
DAHandlerInData         Variable        3
AEHandlerInData         Variable        3

eventUserR13            Variable
eventRegisters          Variable        12      ; r0-r10 and r13

r9_value                Variable                ; sb for shared library

errorR12                Variable
__rt_errorBuffer        ExportedVariable 0
errorBuffer             Variable
errorNumber             Variable
errorString             Variable        25

        IMPORT  |__rt_finalise|

        IMPORT  |_Stack_Validate|
        IMPORT  |_Stack_GetSafe|
        IMPORT  |_Heap_GetBounds|

        IMPORT  |_clib_SigInt|

        IMPORT  |__rt_trap|

        CodeArea

;;; None of the functions in this file are for client use
;;; None are visible from a shared library.
;;; All have sb correctly set up on entry (in a shared library)

        Function __osdep_traphandlers_init
        FunctionEntry , "v1,v2,v3,v4,v5"
        LDR     ip, addr_StaticData
 [ make = "shared-library"
        STR     sb, [ip, #O_r9_value]
 ]
        MOV     v5, #0
        STR     v5, [ip, #O_hadEscape]
        MOV     r0, #1
        STRB    r0, [ip, #O_callbackInactive]
        ; Find out where the abort handlers currently are to determine whether
        ; they should be set.  If their top byte is non-zero, then we are not
        ; running under a debugger, and should install handlers.
        MOV     r1, #0          ; just read
        MOV     r0, #Env_AEHandler
        SWI     ChangeEnv
        MOVS    v1, r1, LSR #24
        ORREQ   v5, v5, #hf_Debuggee
        BEQ     DoInstallHandlers
AbortHandlersNeeded
        ; nb: CopyHandler preserves r8

        ADD     v1, ip, #O_IIHandlerInData
        ADR     v2, IIHandlerInDataInitValue
        BL      CopyHandler
        ADD     v1, ip, #O_PAHandlerInData
        ADR     v2, PAHandlerInDataInitValue
        BL      CopyHandler
        ADD     v1, ip, #O_DAHandlerInData
        ADR     v2, DAHandlerInDataInitValue
        BL      CopyHandler
        ADD     v1, ip, #O_AEHandlerInData
        ADR     v2, AEHandlerInDataInitValue
        BL      CopyHandler

DoInstallHandlers
        STRB    v5, [ip, #O_handlerFlags]
        MOV     r0, #0
        BL      |__riscos_InstallHandlers|
        Return  , "v1,v2,v3,v4,v5"

CopyHandler
        LDMIA   v2!, {v3, v4}
        STMIA   v1!, {v3, v4}
        SUB     v3, v2, v1
        SUB     v3, v3, #8
        MOV     v3, v3, ASR #2
        BIC     v3, v3, #&ff000000
        ORR     v3, v3, #&ea000000
        STR     v3, [v1]
        Return  , "", LinkNotStacked

 ;*-------------------------------------------------------------------*
 ;* Abort Handlers                                                    *
 ;*-------------------------------------------------------------------*

; The handlers called by the OS are in my static data, written there on
; startup, because that's the only way they can find out where the static data
; is.  They don't do much, other than save some registers and load r12 with
; the static base.  They are all the same length, and all immediately precede
; the real handler; when they are installed, a branch to the real handler is
; tacked on the end.

IIHandlerInDataInitValue
        STMFD   r13!, {r0, r9, r12}
        SUB     r12, pc, #O_IIHandlerInData+12

; Now the bits of the abort handlers which get executed from the code.
; r12 is the address of my static data; the user's values of r0 and r12 are
; on the SVC stack.  (SVC mode, interrupts disabled).

IIHandler Keep
        SUB     r14, r14, #4
        ADR     r0, E_IllegalInstruction
        B       Aborted

PAHandlerInDataInitValue
        STMFD   r13!, {r0, r9, r12}
        SUB     r12, pc, #O_PAHandlerInData+12
PAHandler Keep
        SUB     r14, r14, #4
        ADR     r0, E_PrefetchAbort
        B       Aborted

DAHandlerInDataInitValue
        STMFD   r13!, {r0, r9, r12}
        SUB     r12, pc, #O_DAHandlerInData+12
DAHandler Keep
        SUB     r14, r14, #8
        ADR     r0, E_DataAbort
        B       Aborted2

AEHandlerInDataInitValue
        STMFD   r13!, {r0, r9, r12}
        SUB     r12, pc, #O_AEHandlerInData+12
AEHandler Keep
        SUB     r14, r14, #8
        ADR     r0, E_AddressException
;       B       Aborted2

        IMPORT  |__fp_address_in_emulator|
Aborted2 Keep
; Abort which may be in the FP emulator, and if so should be reported
; as occurring at the instruction being emulated.
; If in user mode, can't be in FPE
        LDR     r9, [r12, #O_r9_value]
        TST     r14, #3
        BEQ     Aborted
        STMFD   sp!, {r0 - r3, r14}
        BIC     r0, r14, #PSRBits
        BL      |__fp_address_in_emulator|
        CMP     r0, #0
        LDMFD   sp!, {r0-r3, r14}
        BEQ     Aborted         ; no, not in emulator

; It was a storage fault in the FP emulator.
; r13 points to the base of a full register save (r13 needs resetting to
; above this point).
; NB - is the pc value right??
        ADD     r13, r13, #12    ; pop the saved values of r0, r9 and r12
        ADD     r1, r12, #O_registerDump
        LDMFD   r13!, {r2-r9}
        STMIA   r1!, {r2-r9}
        LDMFD   r13!, {r2-r9}
        STMIA   r1!, {r2-r9}
        LDR     r9, [r12, #O_r9_value]
        SUB     r1, r1, #16*4
        B       |__rt_trap|

        ErrorBlock  PrefetchAbort,      "Prefetch Abort"
        ErrorBlock  DataAbort,          "Data Abort"
        ErrorBlock  AddressException,   "Address Exception"
        ErrorBlock  IllegalInstruction, "Illegal Instruction"
        ALIGN

Aborted Keep
; entry here in SVC mode,  r0 a pointer to an error block describing
; the abort.
; all user registers except r0, r12 are as at the time of the abort.
; r0, r9 & r12 are on the stack.
; First, save all user registers in registerDump.
        ASSERT  O_registerDump = 0
        LDR     r9, [r12, #O_r9_value]
        STMIB   r12, {r1-r14}^
        LDMFD   r13!, {r1, r2, r3}
        STR     r14, [r12, #pc*4]
        STR     r1, [r12, #r0*4]
        STR     r2, [r12, #r9*4]
        STR     r3, [r12, #r12*4]
        MOV     r1, r12
        B       |__rt_trap|

        Function __riscos_InstallHandlers
        ; r0 is zero for the initial call (previous values for the handlers
        ; to be saved).
        ; If non-zero, it is the value memoryLimit should be set to.

        FunctionEntry , "r0,r4,r5,r6,r7"
        LDR     ip, addr_StaticData
        MOV     lr, r0
        LDRB    r0, [ip, #O_handlerFlags]
        TST     r0, #hf_Debuggee
        MOV     r0, #0
        MOV     r1, #0
        MOV     r2, #0
        MOV     r3, #0
        ADDEQ   r4, ip, #O_IIHandlerInData
        ADDEQ   r5, ip, #O_PAHandlerInData
        ADDEQ   r6, ip, #O_DAHandlerInData
        ADDEQ   r7, ip, #O_AEHandlerInData
        MOVNE   r4, #0
        MOVNE   r5, #0
        MOVNE   r6, #0
        MOVNE   r7, #0
        SWI     SetEnv
        CMP     lr, #0
        ADDEQ   lr, ip, #O_oldAbortHandlers
        STMEQIA lr!, {r4-r7}

        MOV     r0, #Env_ExitHandler
        ADR     r1, ExitHandler
        MOV     r2, ip
        SWI     ChangeEnv
        STMEQIA lr!, {r1, r2}

        MOV     r0, #Env_MemoryLimit
        LDMFD   sp!, {r1}
        MOV     r7, r1
        SWI     ChangeEnv
        STMEQIA lr!, {r1}

        MOV     r0, #Env_ErrorHandler
        ADR     r1, ErrorHandler
        MOV     r2, ip
        ADD     r3, ip, #O_errorBuffer
        SWI     ChangeEnv
        STMEQIA lr!, {r1, r2, r3}

        ; callback, escape and event handlers must be updated atomically
        SWI     IntOff

        LDRB    r6, [ip, #O_handlerFlags]
        MOV     r0, #Env_CallBackHandler
        ADR     r1, CallBackHandler
        MOV     r2, ip
        ADD     r3, ip, #O_registerDump
        SWI     ChangeEnv
        CMP     r7, #0
        STMEQIA lr!, {r1, r2, r3}

        MOV     r0, #Env_EscapeHandler
        ADR     r1, EscapeHandler
        MOV     r2, ip
        SWI     ChangeEnv
        CMP     r7, #0
        STMEQIA lr!, {r1, r2}

        SWI     IntOn

        MOV     r0, #Env_UpCallHandler
        ADR     r1, UpCallHandler
        MOV     r2, ip
        SWI     ChangeEnv
        STMEQIA lr!, {r1, r2}

        Return  , "r4,r5,r6,r7"


        Function __osdep_traphandlers_finalise
        Function __riscos_InstallCallersHandlers

        FunctionEntry , "r4,r5,r6,r7,r8"
        LDR     ip, addr_StaticData
        ADD     r8, ip, #O_oldAbortHandlers
        MOV     r0, #0
        MOV     r1, #0
        MOV     r2, #0
        MOV     r3, #0
        LDMIA   r8!, {r4-r7}
        SWI     SetEnv

        MOV     r0, #Env_ExitHandler
        LDMIA   r8!, {r1, r2}
        SWI     ChangeEnv

        MOV     r0, #Env_MemoryLimit
        LDMIA   r8!, {r1}
        SWI     ChangeEnv
        MOV     r4, r1

        MOV     r0, #Env_ErrorHandler
        LDMIA   r8!, {r1, r2, r3}
        SWI     ChangeEnv

        ; callback, escape and event handlers must be updated atomically
        SWI     IntOff

        MOV     r0, #Env_CallBackHandler
        LDMIA   r8!, {r1, r2, r3}
        SWI     ChangeEnv

        MOV     r0, #Env_EscapeHandler
        LDMIA   r8!, {r1, r2}
        SWI     ChangeEnv

        SWI     IntOn

        MOV     r0, #Env_UpCallHandler
        LDMIA   r8!, {r1, r2}
        SWI     ChangeEnv

        MOV     r4, r0

        Return  , "r4,r5,r6,r7,r8"



; Our own exit handler, which restores our parent's environment then exits.
; Just in case a C program manages to (call something which) does a SWI Exit.
; Necessary otherwise because of a stupidity in Obey.
; The register state prevailing when exit was called is completely undefined;
; all we can rely on is that r12 addresses our static data.  The stack
; description may be junk so we reset the stack to its base.
ExitHandler Keep
        LDR     r9, [r12, #O_r9_value]
        BL      |_Stack_GetSafe|
        STMFD   sp!, {r0-r2}
        BL      |__rt_finalise|
        LDMIA   sp!, {r0-r2}
        SWI     Exit

UpCallHandler
        CMP     r0, #256
        MOVNE   pc, r14
; Entered in SWI mode.  A new application is starting (not started by system,
; for which there's a different UpCall handler).  It has the same MemoryLimit
; as this application, so is free to overwrite it.  We'd better close ourselves
; down.
; The register state is undefined, except that r13 must be the SWI stack
; pointer (and r12 addresses our static data).
        STMFD   sp!, {r0-r9, fp, sl, lr}
        TEQP    pc, #0
        NOP
        LDR     r9, [r12, #O_r9_value]
        BL      |_Stack_GetSafe|
        BL      |__rt_finalise|
        SWI     EnterSVC
        NOP
        LDMFD   sp!, {r0-r9, fp, sl, pc}^


 ;*-------------------------------------------------------------------*
 ;* Error handler                                                     *
 ;*-------------------------------------------------------------------*

ErrorHandler Keep
; Now Brazil compatibility is discarded and all SWI calls in the library
; are X-bit set (other than _kernel_swi if non-X bit set has been explicitly
; asked for), any error is treated as fatal here.
;
; Entry with static data base in r0.  User mode, interrupts on
        ASSERT  O_registerDump = 0
        STMIA   r0, {r0-r14}^
        MOV     r1, r0
        ADD     r0, r0, #O_errorNumber
        LDMDA   r0, {r2, r3}            ; r1 is error pc, r2 error number
        CMP     r3, #Error_BranchThroughZero
        MOVEQ   r2, #0
        STR     r2, [r0, #O_registerDump+pc*4-O_errorNumber]
        LDR     r9, [r0, #O_r9_value-O_errorNumber]
        B       |__rt_trap|

 ;*-------------------------------------------------------------------*
 ;* Escape and event handling                                         *
 ;*-------------------------------------------------------------------*

EscapeHandler Keep
        TSTS    r11, #&40
        MOVEQ   pc, r14         ; ignore flag going away

        ; It is NEVER safe to call a handler now: we always have to
        ; wait for CallBack.
        STMFD   r13!, {r0}
        MOV     r0, #1
        STRB    r0, [r12, #O_hadEscape]
        STRB    r0, [r12, #O_escapeSeen]
        LDRB    r0, [r12, #O_handlerFlags]
        LDRB    r12, [r12, #O_callbackInactive]
        LDMFD   r13!, {r0}
        MOV     pc, r14

; Callback handler - entered in either SWI or IRQ mode, interrupts disabled,
; just before OS return to user mode with interrupts on.
CallBackHandler Keep
        ; Set 'in callback' to prevent callback being reentered before it finishes
        ; when we enable interrupts later on.
        MOV     r0, #0
        STRB    r0, [r12, #O_callbackInactive]

        LDR     r9, [r12, #O_r9_value]  ; sb for shared library
        ; Copy the register set from our static callback buffer, onto the stack
        ; (If we appear to have a valid stack pointer).  Otherwise, we just
        ; ignore the event.
        BL      |_Heap_GetBounds|
        LDR     r1, addr_StaticData
        BL      |_Stack_Validate|
        CMP     r0, #0
        BNE     Esc_StackOK
Esc_BadStack
        MOV     r12, #-1
        B       Esc_NoStackForHandler

Esc_StackOK
        LDR     r1, addr_StaticData
        ADD     r11,r1,#O_registerDump+16*4
        LDMDB   r11!, {a1-a4, v2-v5}
        STMDB   r12!, {a1-a4, v2-v5}
        LDMDB   r11!, {a1-a4, v2-v5}
        STMDB   r12!, {a1-a4, v2-v5}

Esc_NoStackForHandler
        LDR     v5, addr_StaticData
        TEQP    pc, #PSRIBit+PSRSVCMode ; we want the testing for an escape and
        MOV     r1, #1                  ; allowing callbacks to be indivisible
        LDRB    r2, [v5, #O_hadEscape]  ; (otherwise an escape may be lost).
        STRB    r1, [v5, #O_callbackInactive]
        MOV     r1, #0
        STRB    r1, [v5, #O_hadEscape]
        CMP     v1, #0
        MOV     r0, #&7e
        SWI     Byte

        TEQP    pc, #PSRIBit           ; to user mode, with interrupts off
        NOP
        CMP     r12, #0                ; if no stack,
        BLT     Esc_NoStack
        CMP     r2, #0                 ; or wasn't escape, ignore
        BNE     Esc_Handle
Esc_NoStack
        ADD     r0, v5, #O_registerDump
        B       ReloadUserState

Esc_Handle
        ; now find a handler
        MOV     sp, r12
        MOV     sl, r10
        MOV     fp, v1
        SWI     IntOn
        MOV     r0, sp
        BL      |_clib_SigInt|

        MOV     r0, sp
ReloadUserState
        ; Here we must unset the 'callback active' flag and restore our state
        ; from the callback buffer atomically. 3u ARMs unsupported now
        ; User r13, r14 must be reloaded from user mode.
        SWI     EnterSVC
        TEQP    pc, #PSRIBit+PSRSVCMode
        NOP
        ADD     r14, r0, #pc*4
        LDMDB   r14, {r0-r14}^
        NOP
        LDMIA   r14, {pc}^           ;;;;@@@@ 32-bit arm problem

        Function __riscos_passtodebugger
        ; pointer to error block in a1
        ; pointer to register block in a2
        ; If running under a debugger, hands the error to the debugger's error
        ; handler.  Otherwise, just returns.
        LDR     ip, addr_StaticData

        ; if not being debugged, hand to the library's handler
        LDRB    a3, [ip, #O_handlerFlags]
        TST     a3, #hf_Debuggee
        Return  , "", LinkNotStacked, EQ

        ; otherwise, pass on to the debugger's error handler
        SWI     EnterSVC
        LDR     r2, [ip, #O_oldErrorBuffer]
        LDR     r3, [r1, #pc*4]
        STR     r3, [r2], #+4                   ; error pc
        LDR     r3, [r0], #+4
        STR     r3, [r2], #+4                   ; error number
01      LDRB    r3, [r0], #+1
        STRB    r3, [r2], #+1
        CMP     r3, #0
        BNE     %B01
        LDR     r14, [ip, #O_oldErrorHandler-O_registerDump]
        LDMIA   r1, {r0 - r14}^
        NOP
        BICS    pc, r14, #PSRBits    ;;;;@@@@ 32-bit arm problem


        AdconTable

addr_StaticData
        &       StaticData

        END
