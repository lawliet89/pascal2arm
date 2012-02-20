;;; RISCOS-hosted C-library kernel : ros_src/kernel.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991.

;;; RCS $Revision: 1.8 $
;;; Checkin $Date: 1996/01/25 16:00:17 $
;;; Revising $Author: hmeeking $

        GET     h_brazil.s
        GET     objmacs.s
        GET     h_errors.s

        GET     h_uwb.s
        GET     h_stack.s
        GET     h_hw.s

        Module  kernel

 [ make = "all" :LOR: make = "shared-library"

        DataArea

StaticData
imageDesc       Variable

__rt_heapDesc   ExportedVariable 0
heapBase        Variable
heapTop         Variable
heapLimit       Variable

allocProc       Variable        0
__rt_malloc     ExportedVariable
freeProc        Variable        0
__rt_free       ExportedVariable

ArgString       Variable

__errno         ExportedVariable
_interrupts_off ExportedVariable
_saved_interrupt Variable
__huge_val      ExportedWord &7FEFFFFF
                InitWord     &FFFFFFFF

fpPresent       VariableByte    ; zero if not
stack_o_flag    VariableByte
_inSignalHandler ExportedVariableByte

inTrapHandler   VariableByte
__rt_unwinding  ExportedVariableByte 0
unwinding       VariableByte

        IMPORT  |__fp_initialise|
        IMPORT  |__fp_finalise|

;----------------------------------------------------------------------------;
; Functions to initialise and finalise fp support.  The RISCOS-hosted library;
; always has this (either as an explicitly-linked fpe object, or possibly    ;
; through a pre-loaded FPEmulator module).  For re-hostings where fp support ;
; may not be present, the imports should be WEAK and the calls conditional.  ;
;----------------------------------------------------------------------------;

        EXPORT  |__rt_exit|     ; also known as ...
        EXPORT  |__rt_exit_32|  ; also known as ...
        EXPORT  |_exit|

        EXPORT  |__rt_command_string|

        EXPORT  |__rt_alloc|

        IMPORT  |_Stack_Init|
        IMPORT  |_Stack_GetSafe|
        IMPORT  |_Stack_Validate|

        EXPORT  |__rt_fpavailable|
        EXPORT  |__rt_fpavailable_32|

        EXPORT  |__rt_CopyError|

        EXPORT  |__rt_finalise|

        EXPORT  |__rt_exittraphandler|
        EXPORT  |__rt_exittraphandler_32|
        EXPORT  |__rt_trap|
        EXPORT  |__rt_trap_32|

        IMPORT  |__riscos_passtodebugger|
        EXPORT  |__riscos_raise_error|
        IMPORT  |_clib_TrapHandler|

        EXPORT  |_Heap_GetBounds|
        EXPORT  |__rt_imagebase|

        IMPORT  |__rt_lib_init|

        IMPORT  |__osdep_traphandlers_finalise|
        IMPORT  |__osdep_traphandlers_init|

        IMPORT  |__osdep_heapsupport_init|
        IMPORT  |__osdep_heapsupport_finalise|
        IMPORT  |__osdep_heapsupport_extend|

        IMPORT  |__rt_unwind_aborted|
 ]

        CodeArea

 [ make = "all" :LOR: make = "shared-library-stub"

        IMPORT  |__root_stack_size|, WEAK

;----------------------------------------------------------------------------;
; If __root_stack_size, imported WEAKly, exists, the value it addresses      ;
; is used as the initial size of the stack. It can be defined in your C      ;
; program as, e.g. int __root_stack_size = 10000; /* 10KB initial stack */   ;
;----------------------------------------------------------------------------;

   [ make = "shared-library-stub"
        IMPORT  |__rt_init|
        IMPORT  |_exit|
   ]
        EXPORT  |__main|
        IMPORT  main
        IMPORT  |_main|

        IMPORT  |Image$$RO$$Base|
        IMPORT  |Image$$RO$$Limit|
        IMPORT  |Image$$RW$$Limit|
        IMPORT  |__cpp_initialise|, WEAK
        IMPORT  |__cpp_finalise|, WEAK

;----------------------------------------------------------------------------;
; Linker-created symbols describing the image.                               ;
; Image$$RO$$Base  is the base of the image's code: needed for               ;
;                  __riscos_system (as base of store to copy) and, together  ;
; Image$$RO$$Limit with Image$$RO$$Limit (top of image's code) for           ;
;                  _backtrace()                                              ;
; Image$$RW$$Limit is the top of the image's static data, used as the heap   ;
;                  base.                                                     ;
;----------------------------------------------------------------------------;

;
; This is the initial entry point to the image.
; (The compiler ensures it is linked in to the image by generating a reference
;  to __main from the object module generated from compiling a file containing
;  an  extern main()).
        ENTRY

; We have to establish a stack for C, initialise the library, then call _main
; (which will set up the arguments for main, then call it).
|__main|
 [ make = "shared-library-stub"
        IMPORT  |sb$$interLUdata$$Base|
        LDR     r9, =|sb$$interLUdata$$Base|
 ]
        SWI     GetEnv                  ; to decide heap limit
        MOV     r3, r1                  ; top of program workspace
        LDR     r1, =|Image$$RW$$Limit| ; end of program image

; If __root_stack_size exists, the value it addresses is the initial size of
; the stack (provided it is sensible).
        LDR     r2, =|__root_stack_size|
        CMP     r2, #0
        LDRNE   r2, [r2]
        MOVEQ   r2, #RootStackSize

        ADR     r0, id_block
        ADD     sp, r1, #8*4
        BL      |__rt_init|
        LDR     a1, =main
        BL      |_main|
        B       |_exit|

id_c_base  * 0
id_c_limit * 4
id_cpp_fns * 8

id_block
        &       |Image$$RO$$Base|
        &       |Image$$RO$$Limit|
        &       |__cpp_initialise|
        &       |__cpp_finalise|

; That needs to be linked into the image, whereas all of the following may be in
; a shared library.
 ]

 [ make = "all" :LOR: make = "shared-library"

   [ make = "shared-library"
        EXPORT  |__rt_init|
   ]
|__rt_init|
        ; r0 points to a kernel init block.
        ; r1 = base of root stack chunk
        ; r2 = required initial stack size
        ; r3 = end of workspace (= heap limit).
        ; In user mode.
        ; If shared library, ip = sb value (and caller's sb is not needed)

   [ make = "shared-library"
        MOV     ip, sb                  ; unwnated intra-LU entry
        MOV     sb, ip
   ]
        LDR     v4, addr_StaticData
        STR     r0, [v4, #O_imageDesc]

        ADRL    r5, |__malloc|
        STR     r5, [v4, #O_allocProc]  ; default alloc proc
        ADRL    r5, |__free|
        STR     r5, [v4, #O_freeProc]   ; and no dealloc proc

        ; save over argument string copying
        MOV     r5, r1
        MOV     r4, r2
        STR     r5, [v4, #O_ArgString]

        ; Copy the argument string (in SWI mode), so we can access it
        ; (assumed in page 0), while all user access to page 0 is disabled
        SWI     EnterSVC
        SWI     GetEnv
        MOV     r6, #&01
        ORR     r6, r6, #&0100
        ORR     r6, r6, r6, ASL #16
01      LDR     r1, [r0], #+4
        STR     r1, [r5], #+4
        SUB     ip, r1, r6
        BIC     ip, ip, r1
        TST     ip, r6, ASL #7
        BEQ     %B01

        TEQP    pc, #0                  ; back to user mode
        NOP

        MOV     v5, lr
        ADD     r0, v4, #O_heapBase
        MOV     r1, r5
        MOV     r2, r4
        BL      |_Stack_Init|
        STMFD   sp!, {v5}

        ; Initialise fp support.  The RISCOS-hosted library always has this present,
        ; either as an explicitly-linked fpe object, or possibly through a pre-loaded
        ; FPEmulator module.  The support code gets called in SWI mode because it may
        ; need to install hardware vector entries, and any sane user will have page 0
        ; write-protected in user mode.  (The support code itself, being OS-independent,
        ; doesn't know how to get into SWI mode).
        ; Return value of r0 is 1 if fp operations are available, else 0.
        SWI     EnterSVC
      [ :DEF: No_FP
        MOV     r0,#0
      |
        BL      |__fp_initialise|
      ]
        STRB    r0, [v4, #O_fpPresent]
        TEQP    pc, #0                  ; back to user mode

        BL      |__osdep_traphandlers_init|

        ADD     a1, v4, #O_heapBase
        BL      |__osdep_heapsupport_init|

        MOV     v1, #0
; argument for __rt_lib_init:
;                   top of stack,
;                   start of code area,
;                   end of code area,
        ADD     a1, sp, #4
        LDR     a2, [v4, #O_imageDesc]
        MOV     a4, #0
        STRB    a4, [v4, #O__inSignalHandler]
        ADD     a4, a2, #id_cpp_fns
        LDMIA   a2, {a2, a3}
        BL      |__rt_lib_init|
        Return  , ""

|_exit|
        MOV     v5, #1          ; finalise should call atexit() handlers
        B       %F01
|__rt_exit|
|__rt_exit_32|
        MOV     v5, #0
01      MOV     v4, a1          ; save return code
        BL      |_Stack_GetSafe|
        MOV     a1, v5
        BL      |__rt_finalise_1|
        MOV     r2, v4
        CMP     r2, #0
        ADRNE   r0, E_BadReturnCode
        LDRNE   r1, ABEXString
        SWI     Exit

ABEXString
        =       "ABEX"

        ErrorBlock BadReturnCode, "Return code too large"
        ALIGN

        IMPORT  |__rt_lib_shutdown|
|__rt_finalise|
        MOV     a1, #0                  ; no call of atexit() handlers
|__rt_finalise_1|
        FunctionEntry
        BL      |__rt_lib_shutdown|
        BL      |__osdep_traphandlers_finalise|
        BL      |__osdep_heapsupport_finalise|
        ; __fp_finalise needs to be called in SWI mode for the same reason as
        ; __fp_initialise.
        SWI     EnterSVC
      [ :LNOT: :DEF: No_FP
        BL      |__fp_finalise|
      ]
        TEQP    pc, #0
        NOP
        Return  , ""

|__rt_fpavailable|
|__rt_fpavailable_32|
;
; int __rt_fpavailable(); return non-0 if FP instructions are available.
; (for RISCOS hosting, implies support code linked or pre-loaded FPEmulator
;  module present).

        LDR     ip, addr_StaticData
        LDRB    a1, [ip, #O_fpPresent]
        Return  , "", LinkNotStacked

 ;*-------------------------------------------------------------------*
 ;* SWI interfaces                                                    *
 ;*-------------------------------------------------------------------*


|__rt_command_string|
        LDR     a1, addr_StaticData
        LDR     a1, [a1, #O_ArgString]
        Return  , "", LinkNotStacked

        IMPORT  |__rt_errorBuffer|
|__rt_CopyError|
                ; a1 is the address of an error block (may be ours)
                ; we want to copy its contents into our error block,
                ; so _kernel_last_oserror works.
        MOV     a4, a1
        LDR     a2, addr___rt_errorBuffer
        STR     pc, [a2], #+4   ; mark as valid
        LDMIA   a4!, {a3}
        STMIA   a2!, {a3}
CopyErrorString
01      LDRB    a3, [a4], #+1
        STRB    a3, [a2], #+1
        CMP     a3, #0
        BNE     %B01
        Return  , "", LinkNotStacked

|__rt_exittraphandler|
|__rt_exittraphandler_32|
; Declare no longer within trap handler.
; (protection against recursive trap handler invocation may be reset).
        LDR     ip, addr_StaticData
        MOV     a1, #0
        STRB    a1, [ip, #O_inTrapHandler]
        Return  , "", LinkNotStacked

|__rt_trap|
|__rt_trap_32|
        ; pointer to error block in a1
        ; pointer to register block in a2
        MOV     v5, a2

        ; see whether running under a debugger, and if so pass the error to its
        ; error handler
        BL      |__riscos_passtodebugger|

; We can only call an abort handler if we had a stack at the
; time of the abort.  If not, we have to say 'uncaught trap'.
; There is a problem as soon as interrupts are enabled, that an event may
; arrive and trash the register dump.  If there's a stack, this is solved
; by copying the dump onto it.  Otherwise, we protect ourselves while
; constructing the error by pretending there's a callback going on.
; Entry may be in SWI mode (faults) or user mode (stack overflow,
; divide by zero).

        BL      |__rt_CopyError|
        LDR     v4, addr_StaticData
        LDRB    r2, [v4, #O_inTrapHandler]
        CMP     r2, #0
        BNE     RecursiveTrap
        LDRB    r2, [v4, #O_unwinding]
        CMP     r2, #0
        LDRNE   r12, [v5, #r12*4]  ; abort handling trampled this.
        LDRNE   r14, [v5, #pc*4]
        BNE     |__rt_unwind_aborted|
        MOV     r2, #1
        STRB    r2, [v4, #O_inTrapHandler]

        BL      |_Heap_GetBounds|
        MOV     r1, v5
        BL      |_Stack_Validate|

        CMP     r0, #0
        BEQ     Trap_NoStackForHandler

        ADD     r11, v5, #16*4
        LDMDB   r11!, {a1-a4, v2-v5}
        STMDB   r12!, {a1-a4, v2-v5}
        LDMDB   r11!, {a1-a4, v2-v5}
        STMDB   r12!, {a1-a4, v2-v5}
        TEQP    pc, #PSRIBit:OR:PSRVBit                 ; user mode

        NOP
        MOV     sp, r12
        MOV     sl, r10
        MOV     fp, v1
        SWIVS   IntOn

        LDR     a1, addr___rt_errorBuffer
        LDR     a1, [a1, #4]                            ; error number
        MOV     a2, sp
        BL      |_clib_TrapHandler|

; hm - if traphandler returns, resume.  _clib_traphandler never returns.
; something wants cleaning up here.
        LDR     r1, addr_StaticData
        MOV     r0, #0
        STRB    r0, [r1, #O_inTrapHandler]
        MOV     r0, sp

        SWI     EnterSVC
        TEQP    pc, #PSRIBit+PSRSVCMode
        NOP
        ADD     r14, r0, #pc*4
        LDMDB   r14, {r0-r14}^
        NOP
        LDMIA   r14, {pc}^

Trap_NoStackForHandler
        ADR     v1, E_NoStackForTrapHandler
        B       FatalErrorY

        ErrorBlock NoStackForTrapHandler, " (no stack for trap handler)"
        ErrorBlock RecursiveTrap, " (while already in trap handler)"

RecursiveTrap Keep
        ADR     v1, E_RecursiveTrap
FatalErrorY
; RISCOS desperation method: raise a RISCOS error
        TEQP    pc, #0
        BL      |_Stack_GetSafe|
        ; find end of error string in library's buffer
        LDR     a2, addr___rt_errorBuffer
        LDR     a1, [v1]
        STR     a1, [a2, #4]    ; overwrite old error number
        ADD     a2, a2, #8
01      LDRB    a1, [a2], #+1
        CMP     a1, #0
        BNE     %B01

        ADD     a4, v1, #4
        SUB     a2, a2, #1
        BL      CopyErrorString
        SUB     a2, a2, #1
        ADR     a4, str2
        BL      CopyErrorString
        SUB     a2, a2, #1
        LDR     a1, [v5, #pc*4]
        BL      HexOut
        ADR     a4, str3
        BL      CopyErrorString
        SUB     a2, a2, #1
        MOV     a1, v5
        BL      HexOut
        MOV     a1, #0
        STRB    a1, [a2]
        LDR     a1, addr___rt_errorBuffer
        ADD     a1, a1, #4

|__riscos_raise_error|
        STMFD   sp!, {a1}
        BL      |__rt_finalise|
        LDMFD   sp!, {a1}
        SWI     GenerateError


str2    = ", pc = ", 0
str3    = ": registers at ", 0
        ALIGN

HexOut  MOV     a4, #8
01      MOV     a1, a1, ROR #28
        AND     a3, a1, #15
        CMP     a3, #10
        ADDLT   a3, a3, #"0"
        ADDGE   a3, a3, #"A"-10
        STRB    a3, [a2], #+1
        SUBS    a4, a4, #1
        BNE     %B01
        Return  , "", LinkNotStacked


 ;*-------------------------------------------------------------------*
 ;* Storage management                                                *
 ;*-------------------------------------------------------------------*

|__rt_imagebase|
        LDR     a1, addr_StaticData
        LDR     a1, [a1, #O_imageDesc]
        LDR     a1, [a1]
        Return  , "", LinkNotStacked

|_Heap_GetBounds|
        LDR     a1, addr_StaticData
        ADD     a1, a1, #O_heapBase
        Return  , "", LinkNotStacked

|__rt_alloc|
; unsigned _kernel_alloc(unsigned minwords, void **block);
;  Tries to allocate a block of sensible size >= minwords.  Failing that,
;  it allocates the largest possible block of sensible size.  If it can't do
;  that, it returns zero.
;  *block is returned a pointer to the start of the allocated block
;  (NULL if none has been allocated).
        LDR     ip, addr_StaticData

        CMP     r0, #2048
        MOVLT   r0, #2048

        ADD     ip, ip, #O_heapBase
        LDMIB   ip, {r2, r3}

        SUB     r3, r3, r0, ASL #2      ; room for a block of this size?
        CMP     r3, r2                  ; if so, ...
        BLT     alloc_not_enough

        STR     r2, [r1]                ; return it above the previous heapTop
        ADD     r2, r2, r0, ASL #2      ; and update heapTop
        STR     r2, [ip, #4]
        Return  , "", LinkNotStacked

alloc_not_enough
        STMFD   sp!, {r0, r1, ip, lr}
        MOV     r1, ip
        BL      |__osdep_heapsupport_extend|
        ; arguments  size wanted (bytes); pointer to heap descriptor
        ; returns    size acquired (bytes); base of new.

        CMP     r0, #0                  ; size (should be 0 or big enough)
        BEQ     alloc_cant_extend

        LDR     ip, [sp, #8]
        LDMIB   ip, {r2, lr}
        CMP     lr, r1
        SUBNE   r3, lr, r2              ; if not contiguous with old area, amount free in old
        ADD     lr, r1, r0              ; adjust heapLimit
        MOVNE   r0, r2                  ; if not contiguous, remember old heapTop
        MOVNE   r2, r1                  ; and adjust
        STMIB   ip, {r2, lr}
        CMPNE   r3, #0                  ; if contiguous, or old area had no free space,
        BEQ     alloc_cant_extend       ; return from new area

        ; otherwise, return block from top of old area first
        ; (malloc will try again and get from new area)
        LDMFD   sp!, {r0, r1, ip, lr}
        STR     r0, [r1]
        MOV     r0, r3, ASR #2
        Return  , "", LinkNotStacked

alloc_cant_extend
        LDMFD   sp!, {r0, r1, ip, lr}
        LDMIB   ip, {r2, r3}
        SUB     r0, r3, r2              ; amount remaining (bytes)
        MOVS    r0, r0, ASR #2
        MOVEQ   r2, #0                  ; if none, returned block is null
        STRNE   r3, [ip, #4]            ; otherwise adjust heaptop to be heaplimit
        STR     r2, [r1]
        Return  , "", LinkNotStacked

|__malloc|
; static void * __malloc(int numbytes);
;  Allocates numbytes bytes (rounded up to number of words), and returns
;  the block allocated.  If it can't, returns NULL.
;  Normally, this will be replaced by the real malloc very early in the
;  startup procedure.
        ADD     r0, r0, #3
        BIC     r0, r0, #3
        LDR     ip, addr_StaticData
        ADD     ip, ip, #O_heapTop
        LDMIA   ip, {r2, r3}

        SUB     r3, r3, r0              ; room for a block of this size?
        CMP     r3, r2                  ; if so, ...
        ADDGE   r3, r2, r0
        MOVGE   r0, r2                  ; return it above heapTop
        STRGE   r3, [ip]
        MOVLT   r0, #0
        Return  , "", LinkNotStacked

|__free|
; static void __free(void *);
;  Frees the argument block.
;  Normally, this will be replaced by the real free very early in the
;  startup procedure.
;  I don't think there's much point in providing a real procedure for this;
;  if I do, it complicates malloc and alloc above.
        Return  , "", LinkNotStacked

        AdconTable

addr_StaticData
        &       StaticData

        IMPORT  |__rt_errorBuffer|
addr___rt_errorBuffer
        &       |__rt_errorBuffer|

 ]
       END
