; Purpose: ARMulator-hosted C-library kernel.
;
; Copyright (C) Advanced RISC Machines Ltd., 1991.
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; Advanced RISC Machines.

;;; RCS $Revision: 1.28.12.2 $
;;; Checkin $Date: 1997/11/13 09:49:09 $
;;; Revising $Author: wdijkstr $

        GET     objmacs.s
        GET     h_errors.s

        GET     h_uwb.s
        GET     h_stack.s
        GET     h_hw.s
        GET     h_os.s

        Module  kernel

 [ make = "all" :LOR: make = "shared-library"

        DataArea

StaticData
imageDesc               Variable

__rt_heapDesc           ExportedVariable 0
heapBase                Variable
heapTop                 Variable
heapLimit               Variable

allocProc               Variable        0
__rt_malloc             ExportedVariable
freeProc                Variable        0
__rt_free               ExportedVariable

__errno                 ExportedVariable
_interrupts_off         ExportedVariable
_saved_interrupt        Variable
__huge_val              ExportedWord &7FEFFFFF
                        InitWord     &FFFFFFFF

fpPresent               VariableByte    ; zero if not
stack_o_flag            VariableByte
_inSignalHandler        ExportedVariableByte

inTrapHandler           VariableByte
__rt_unwinding          ExportedVariableByte 0
unwinding               VariableByte

__rt_errorBuffer        ExportedVariable 0
errorBuffer             Variable
errorNumber             Variable
errorString             Variable 25

psrMode                 VariableByte

        IMPORT  |__fp_initialise|
        IMPORT  |__fp_initialise_32|
        IMPORT  |__fp_finalise|
        IMPORT  |__fp_finalise_32|

;----------------------------------------------------------------------------;
; Functions to initialise and finalise fp support.  The RISCOS-hosted library;
; always has this (either as an explicitly-linked fpe object, or possibly    ;
; through a pre-loaded FPEmulator module).  For re-hostings where fp support ;
; may not be present, the imports should be WEAK and the calls conditional.  ;
;----------------------------------------------------------------------------;

        EXPORT  |__rt_exit|     ; also known as ...
        EXPORT  |_exit|

        EXPORT  |__rt_alloc|

        IMPORT  |_Stack_Init|
        IMPORT  |_Stack_Init_32|
        IMPORT  |_Stack_GetSafe|
        IMPORT  |_Stack_GetSafe_32|
        IMPORT  |_Stack_Validate|
        IMPORT  |_Stack_Validate_32|

        EXPORT  |__rt_fpavailable|
        EXPORT  |__rt_fpavailable_32|

        EXPORT  |__rt_finalise|

        EXPORT  |__rt_exittraphandler|
        EXPORT  |__rt_exittraphandler_32|
        EXPORT  |__rt_trap|
        EXPORT  |__rt_trap_32|

        IMPORT  |_clib_TrapHandler|
        IMPORT  |_clib_TrapHandler_32|

        EXPORT  |_Heap_GetBounds|
        EXPORT  |__rt_imagebase|

        IMPORT  |__rt_lib_init|

        IMPORT  |__osdep_traphandlers_finalise|
        IMPORT  |__osdep_traphandlers_finalise_32|
        IMPORT  |__osdep_traphandlers_init|
        IMPORT  |__osdep_traphandlers_init_32|

        IMPORT  |__osdep_heapsupport_init|
        IMPORT  |__osdep_heapsupport_init_32|
        IMPORT  |__osdep_heapsupport_finalise|
        IMPORT  |__osdep_heapsupport_finalise_32|
        IMPORT  |__osdep_heapsupport_extend|
        IMPORT  |__osdep_heapsupport_extend_32|

   [ backtrace_enabled
        IMPORT  |__rt_unwind_aborted|
   ]
 ]

        CodeArea

 [ make = "all" :LOR: make = "shared-library-stub"

        IMPORT  |__root_stack_size|, WEAK
        IMPORT  |__heap_limit|, WEAK
        IMPORT  |__heap_base|, WEAK

;----------------------------------------------------------------------------;
; If __root_stack_size, imported WEAKly, exists, the value it addresses      ;
; is used as the initial size of the stack. It can be defined in your C      ;
; program as, e.g. int __root_stack_size = 10000; /* 10KB initial stack */   ;
;----------------------------------------------------------------------------;

   [ make = "shared-library-stub"
        IMPORT  |__rt_init|
        IMPORT  |_exit|
   ]
        EXPORT  |__entry|
        IMPORT  main, WEAK
        IMPORT  |_main|

        IMPORT  |Image$$RO$$Base|
        IMPORT  |Image$$RO$$Limit|
        IMPORT  |Image$$RW$$Base|
        IMPORT  _RW_Limit
        IMPORT  |Image$$ZI$$Base|
        IMPORT  |Image$$ZI$$Limit|
        IMPORT  |__cpp_initialise|, WEAK
        IMPORT  |__cpp_finalise|, WEAK

;----------------------------------------------------------------------------;
; Linker-created symbols describing the image.                               ;
; Image$$RO$$Base  is the base of the image's code (ROM): needed for         ;
;                  __riscos_system (as base of store to copy) and, together  ;
; Image$$RO$$Limit with Image$$RO$$Limit (top of image's code) for           ;
;                  _backtrace()                                              ;
; Image$$RW$$Base  defines the data (RAM) base address                       ;
; Image$$RW$$Limit is the top of the image's static data, used as the heap   ;
;                  base.                                                     ;
; Image$$ZI$$Base  defines the base of a section to be initialised with 0s   ;
; Image$$ZI$$Limit defines the end of the region to be initialised with 0s   ;
;                  (must be the same as Image$$RW$$Limit in this model)      ;
;----------------------------------------------------------------------------;

; We have to establish a stack for C, initialise the library, then call _main
; (which will set up the arguments for main, then call it).
|__entry|
 [ make = "shared-library-stub"
        IMPORT  |sb$$interLUdata$$Base|
        LDR     r9, =|sb$$interLUdata$$Base|
 ]

 [ INTERWORK :LOR: THUMB
        BX      pc
        B       wrong_cpu
 ]
        ; Note that __heap_limit is a bit of a misnomer (!)
        ; in fact it is the top of workspace, or at least is used as such.
        
        SWI     SWI_GetEnv              ; to decide heap limit
        LDR     r3, =__heap_limit
        CMP     r3, #0
        LDRNE   r3, [r3]
        MOVEQ   r3, r1                  ; top of program workspace
        LDR     r1, =__heap_base
        CMP     r1, #0
        LDREQ   r1, =_RW_Limit          ; end of program image
        LDR     r1, [r1]

; If __root_stack_size exists, the value it addresses is the initial size of
; the stack (provided it is sensible).
        LDR     r2, =|__root_stack_size|
        CMP     r2, #0
        LDRNE   r2, [r2]
        MOVEQ   r2, #RootStackSize

        ADR     r0, id_block
        ; ADD     sp, r1, #8*4  ; Commented out as this was overridden anyway

        ; Now set up sp and sl here rather than getting Stack_Init to do it
        ; as the Angel version of kernel.s wants to set up sp and sl itself
        ; in a rather different way.

        MOV     sp, r3

        SUB     r3, sp, r2
  [ :DEF:sl
        ADD     sl, r3, #SC_SLOffset
  ]

        BL      |__rt_init_32|
        LDR     a1, =main
  [ INTERWORK :LOR: THUMB
        LDR     ip, =|_main|
    [ INTERWORK
        BL      __call_ip
    |
        ORR     lr, pc, #1
        BX      ip
        CODE16
        BX      pc
        CODE32
    ]
  |
        BL      _main
  ]
        B       |_exit_32|

id_c_base  * 0
id_c_limit * 4
id_cpp_fns * 8

id_block
        &       |Image$$RO$$Base|
        &       |Image$$RO$$Limit|
        &       |__cpp_initialise|
        &       |__cpp_finalise|

    [ INTERWORK
; Don't use 'Function' macro here.
        EXPORT  __call_ip
__call_ip
        FunctionEntry
        TST     ip, #1          ; Calling Thumb Fn?
        BNE     %FT0
        MOV     lr, pc
        BX      ip
        Return
0
        ORR     lr, pc, #1
        BX      ip
        CODE16
        BX      pc
        CODE32
        Return

      [ THUMB
        CODE16
      ]

        EXPORT  __call_r0
__call_r0
        MOV     r2, r0
        EXPORT  __call_r2
__call_r2
        FunctionEntry
      [ THUMB
        BX      pc
        CODE32
      ]
        TST     r2, #1          ; Calling Thumb Fn?
        BNE     %FT0
        MOV     lr, pc
        BX      r2
        Return
0
        ORR     lr, pc, #1
        BX      r2
        CODE16
        BX      pc
        CODE32
        Return
    ]

    [ INTERWORK :LOR: THUMB
        IMPORT  __rt_registerDump
wrong_cpu
        LDR     ip, addr___rt_registerDump
        ; Don't really care about interrupt latency here
        ; Nothing else seems to save the PSR so I don't dother either
        ; PC ends up pointing to the GenerateError
        STMIA   ip, {r0-r15}
        ADR     r0, wrong_cpu_error
        MOV     r1, ip
        SWI     SWI_GenerateError
addr___rt_registerDump
        DCD     __rt_registerDump
wrong_cpu_error
        DCD     Error_WrongCPU
        DCB     "This code can only run on a Thumb compatible processor", 0
        ALIGN
    ]

; That needs to be linked into the image, whereas all of the following may be in
; a shared library.
 ]

 [ make = "all" :LOR: make = "shared-library"

        Function __rt_init
__rt_init_32
        KEEP    __rt_init_32
        ; r0 points to a kernel init block.
        ; r1 = base of root stack chunk
        ; r2 = required initial stack size
        ; r3 = end of workspace (= heap limit).
        ; In user mode.
        ; If shared library, ip = sb value (and caller's sb is not needed)

   [ make = "shared-library"
        MOV     ip, sb                  ; unwanted intra-LU entry
        MOV     sb, ip
   ]
        LDR     v4, addr_StaticData
        STR     r0, [v4, #O_imageDesc]

    [ THUMB :LAND: {CONFIG} <> 16
        ADRL    r5, |__malloc|+1
    |
        ADRL    r5, |__malloc|
    ]
        STR     r5, [v4, #O_allocProc]  ; default alloc proc
    [ THUMB :LAND: {CONFIG} <> 16
        ADRL    r5, |__free|+1
    |
        ADRL    r5, |__free|
    ]
        STR     r5, [v4, #O_freeProc]   ; and no dealloc proc

        MOV     v5, lr
        ADD     r0, v4, #O_heapBase
        BL      |_Stack_Init_32|
        STMFD   sp!, {v5}

        ; Initialise fp support.  The RISCOS-hosted library always has this present,
        ; either as an explicitly-linked fpe object, or possibly through a pre-loaded
        ; FPEmulator module.  The support code gets called in SWI mode because it may
        ; need to install hardware vector entries, and any sane user will have page 0
        ; write-protected in user mode.  (The support code itself, being OS-independent,
        ; doesn't know how to get into SWI mode).
        ; Return value of r0 is 1 if fp operations are available, else 0.
 [ {CONFIG} <> 26
        MRS     v5, CPSR
        AND     v5, v5, #&1f
        STRB    v5, [v4, #O_psrMode]
        CMP     v5, #PSR32_32
        BNE     %FT0
 ]
        SWI     SWI_EnterOS
0
        BL      |__fp_initialise_32|
        STRB    r0, [v4, #O_fpPresent]
 [ {CONFIG} = 26
        TEQP    pc, #0                  ; back to user mode
 |
        MSR     CPSR_c, v5              ; back to original mode
 ]

        BL      |__osdep_traphandlers_init_32|

        ADD     a1, v4, #O_heapBase
        BL      |__osdep_heapsupport_init_32|

        MOV     v1, #0
; argument for __rt_lib_init:
;                   top of stack,
;                   start of code area,
;                   end of code area,
;                   C++ init block
        ADD     a1, sp, #4
        LDR     a2, [v4, #O_imageDesc]
        MOV     a4, #0
        STRB    a4, [v4, #O__inSignalHandler]
        ADD     a4, a2, #id_cpp_fns
        LDMIA   a2, {a2, a3}
    [ INTERWORK :LOR: THUMB
        LDR     ip, =|__rt_lib_init|
      [ INTERWORK
        BL      __call_ip
      |
        ORR     lr, pc, #1
        BX      ip
        CODE16
        BX      pc
        CODE32
      ]
    |
        BL      __rt_lib_init
    ]
        Return  , ""

        Function _exit
_exit_32
        MOV     v5, #1          ; finalise should call atexit() handlers
        B       %F01

        Function __rt_exit
        EXPORT  __rt_exit_32
__rt_exit_32
        MOV     v5, #0
01      MOV     v4, a1          ; save return code
        BL      |_Stack_GetSafe_32|
        MOV     a1, v5
        BL      |__rt_finalise_1|
        MOV     r2, v4
        CMP     r2, #0
        ADRNE   r0, E_BadReturnCode
        LDRNE   r1, ABEXString
        SWI     SWI_Exit

ABEXString
        =       "ABEX"

        ErrorBlock BadReturnCode, "Return code too large"
        ALIGN

        IMPORT  |__rt_lib_shutdown|
|__rt_finalise|
        MOV     a1, #0                  ; no call of atexit() handlers
|__rt_finalise_1|
        FunctionEntry
    [ INTERWORK :LOR: THUMB
        LDR     ip, =|__rt_lib_shutdown|
      [ INTERWORK
        BL      __call_ip
      |
        ORR     lr, pc, #1
        BX      ip
        CODE16
        BX      pc
        CODE32
      ]
    |
        BL      __rt_lib_shutdown
    ]
        BL      |__osdep_traphandlers_finalise_32|
        BL      |__osdep_heapsupport_finalise_32|
        ; __fp_finalise needs to be called in SWI mode for the same reason as
        ; __fp_initialise.
 [ {CONFIG} <> 26
        STMDB   sp!, {v1}
        MRS     v1, CPSR
        AND     v1, v1, #&1f
        CMP     v1, #PSR32_32
        BNE     %FT0
 ]
        SWI     SWI_EnterOS
0
        BL      |__fp_finalise_32|
 [ {CONFIG} = 26
        TEQP    pc, #0                  ; back to user mode
        NOP
 |
        MSR     CPSR_c, v1              ; back to user32 mode
        NOP                             ; Is this necessary?
        LDMIA   sp!, {v1}
 ]
        Return  , ""

        Function __rt_fpavailable
__rt_fpavailable_32
;
; int __rt_fpavailable(); return non-0 if FP instructions are available.
; (for RISCOS hosting, implies support code linked or pre-loaded FPEmulator
;  module present).

        LDR     ip, addr_StaticData
        LDRB    a1, [ip, #O_fpPresent]
        Return  , "", LinkNotStacked


;
; Return the address of the string used to start the program
;

        Function __rt_command_string
        FunctionEntry
        SWI     SWI_GetEnv
        Return , ""

|__rt_CopyError|
                ; a1 is the address of an error block (may be ours)
                ; we want to copy its contents into our error block,
                ; so _kernel_last_oserror works.
        MOV     a4, a1
        LDR     a2, addr__rt_errorBuffer
        STR     pc, [a2], #+4   ; mark as valid
        LDMIA   a4!, {a3}
        STMIA   a2!, {a3}
CopyErrorString
01      LDRB    a3, [a4], #+1
        STRB    a3, [a2], #+1
        CMP     a3, #0
        BNE     %B01
        MOV     pc, lr

        Function __rt_trap
__rt_trap_32
;
; A fault needs handling, as the debugger didn't want to see it
; Passed an error block in a1, and a register block in a2
;

        MOV     v5, a2
        BL      |__rt_CopyError|
        LDR     v4, addr_StaticData
        LDRB    r2, [v4, #O_inTrapHandler]
        CMP     r2, #0
        BNE     RecursiveTrap
 [ backtrace_enabled
        LDRB    r2, [v4, #O_unwinding]
        CMP     r2, #0
        LDRNE   r12, [v5, #r12*4]  ; abort handling trampled this.
        LDRNE   r14, [v5, #pc*4]
        BNE     |__rt_unwind_aborted|
 ]
        MOV     r2, #1
        STRB    r2, [v4, #O_inTrapHandler]

        BL      |_Heap_GetBounds_32|
        MOV     r1, v5
        BL      |_Stack_Validate_32|

        CMP     r0, #0
        BEQ     Trap_NoStackForHandler

        ADD     r11, v5, #16*4
 [ LDM_MAX >= 8
        LDMDB   r11!, {a1-a4, v2-v5}
        STMDB   r12!, {a1-a4, v2-v5}
        LDMDB   r11!, {a1-a4, v2-v5}
        STMDB   r12!, {a1-a4, v2-v5}
 |
   [ LDM_MAX >= 4
        LDMDB   r11!, {a1-a4}
        STMDB   r12!, {a1-a4}
        LDMDB   r11!, {a1-a4}
        STMDB   r12!, {a1-a4}
        LDMDB   r11!, {a1-a4}
        STMDB   r12!, {a1-a4}
        LDMDB   r11!, {a1-a4}
        STMDB   r12!, {a1-a4}
   |
        LDMDB   r11!, {a1-a3}
        STMDB   r12!, {a1-a3}
        LDMDB   r11!, {a1-a3}
        STMDB   r12!, {a1-a3}
        LDMDB   r11!, {a1-a3}
        STMDB   r12!, {a1-a3}
        LDMDB   r11!, {a1-a3}
        STMDB   r12!, {a1-a3}
        LDMDB   r11!, {a1-a3}
        STMDB   r12!, {a1-a3}
        LDMDB   r11!, {a1}
        STMDB   r12!, {a1}
   ]
 ]
        LDR     v4, addr_StaticData
 [ {CONFIG} = 26
        TEQP    pc, #PSRIBit:OR:PSRVBit         ; user mode, interrupts off (why V?)
        NOP
 |
        LDR     v1, [v4, #O_psrMode]
        ORR     v1, v1, #PSR32_IBit
        MSR     CPSR_c, v1                      ; original mode, interrupts off
 ]

        MOV     sp, r12         ; all as returned by _Stack_Validate
;       MOV     sl, r10         ; ?? sl *is* r10
        MOV     r11, v1         ; fp

        LDR     a1, addr__rt_errorBuffer
        LDR     a1, [a1, #4]                            ; error number
        MOV     a2, sp
        BL      |_clib_TrapHandler_32|

; hm - if traphandler returns, resume.  _clib_traphandler never returns.
; something wants cleaning up here.
        MOV     r0, #0
        STRB    r0, [v4, #O_inTrapHandler]
        MOV     r0, sp

 [ {CONFIG} = 26
        SWI     SWI_EnterOS
        TEQP    pc, #PSRIBit+PSRSVCMode         ; interrupts off, stay in SVC mode
 |
        CMP     v1, #PSR32_32
        BNE     %FT0
        SWI     SWI_EnterOS
        MSR     CPSR_c, #PSR32_IBit+PSR32_32+PSRSVCMode     ; interrupts off, stay in SVC32
0
 ]
        NOP
 [ LDM_MAX >= 15
        ADD     r14, r0, #pc*4
        LDMDB   r14, {r0-r14}^
 |
        MOV     r14, r0
    [ LDM_MAX >= 4
        LDMDB   r14!, {r0-r3}
        LDMDB   r14!, {r4-r7}
        LDMDB   r14!, {r8-r11}
        LDMDB   r14, {r12-r14}^
    |
        LDMDB   r14!, {r0-r2}
        LDMDB   r14!, {r3-r5}
        LDMDB   r14!, {r6-r8}
        LDMDB   r14!, {r9-r11}
        LDMDB   r14, {r12-r14}^
    ]
 ]
        NOP
        LDMIA   r14, {pc}^


Trap_NoStackForHandler
        ADR     r0, NoStackForTrapError
        SWI     SWI_GenerateError

NoStackForTrapError
        DCD     Error_NoStackForTrapHandler
        DCB     "No Stack for Trap Handler",0
        ALIGN

RecursiveTrap
        ADR     r0, RecursiveTrapError
        SWI     SWI_GenerateError

RecursiveTrapError
        DCD     Error_RecursiveTrap
        DCB     "Recursive Trap",0
        ALIGN

        Function __rt_exittraphandler
__rt_exittraphandler_32
; Declare no longer within trap handler.
; (protection against recursive trap handler invocation may be reset).
        LDR     ip, addr_StaticData
        MOV     a1, #0
        STRB    a1, [ip, #O_inTrapHandler]
        Return  , "", LinkNotStacked

 ;*-------------------------------------------------------------------*
 ;* Storage management                                                *
 ;*-------------------------------------------------------------------*

        Function __rt_imagebase
        LDR     a1, addr_StaticData
        LDR     a1, [a1, #O_imageDesc]
        LDR     a1, [a1]
        Return  , "", LinkNotStacked

        Function _Heap_GetBounds
_Heap_GetBounds_32
        LDR     a1, addr_StaticData
        ADD     a1, a1, #O_heapBase
        Return  , "", LinkNotStacked

        Function __rt_alloc
; unsigned _kernel_alloc(unsigned minwords, void **block);
;  Tries to allocate a block of sensible size >= minwords.  Failing that,
;  it allocates the largest possible block of sensible size.  If it can't do
;  that, it returns zero.
;  *block is returned a pointer to the start of the allocated block
;  (NULL if none has been allocated).
        LDR     ip, addr_StaticData

        CMP     r0, #2048
        MOVLO   r0, #2048

        ADD     ip, ip, #O_heapBase
        LDMIB   ip, {r2, r3}

        SUBS    r3, r3, r0, ASL #2      ; room for a block of this size?
        CMPCS   r3, r2                  ; if so, ...
        BCC     alloc_not_enough

        STR     r2, [r1]                ; return it above the previous heapTop
        ADD     r2, r2, r0, ASL #2      ; and update heapTop
        STR     r2, [ip, #4]
        Return  , "", LinkNotStacked

alloc_not_enough
        Push    "r0,r1,ip,lr"
        MOV     r1, ip
        BL      |__osdep_heapsupport_extend_32|
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
        Pop     "r0,r1,ip,lr"
        STR     r0, [r1]
        MOV     r0, r3, ASR #2
        Return  , "", LinkNotStacked

alloc_cant_extend
        Pop     "r0,r1,ip,lr"
        LDMIB   ip, {r2, r3}
        SUB     r0, r3, r2              ; amount remaining (bytes)
        MOVS    r0, r0, ASR #2
        MOVEQ   r2, #0                  ; if none, returned block is null
        STRNE   r3, [ip, #4]            ; otherwise adjust heaptop to be heaplimit
        STR     r2, [r1]
        Return  , "", LinkNotStacked

        Function __malloc
; static void * __malloc(int numbytes);
;  Allocates numbytes bytes (rounded up to number of words), and returns
;  the block allocated.  If it can't, returns NULL.
;  Normally, this will be replaced by the real malloc very early in the
;  startup procedure.
        ADD     r2, r0, #3
        BIC     r2, r2, #3
        LDR     ip, addr_StaticData
        ADD     ip, ip, #O_heapTop
        LDMIA   ip, {r0, r1}

        ADDS    r2, r0, r2              ; new heapTop
; CC if no wrap
; CS if wrap
        CMPCC   r2, r1                  ; compare with heapLimit
; CS if wrap or r2 >= r1 (heaplimit)
; CC if nowrap and r2 < r1
        STRCC   r2, [ip]
        MOVCS   r0, #0
        Return  , "", LinkNotStacked

        Function __free
; static void __free(void *);
;  Frees the argument block.
;  Normally, this will be replaced by the real free very early in the
;  startup procedure.
;  I don't think there's much point in providing a real procedure for this;
;  if I do, it complicates malloc and alloc above.
        Return  , "", LinkNotStacked

        AdconTable

addr__rt_errorBuffer
        &       |__rt_errorBuffer|
addr_StaticData
        &       StaticData

 ]
       END
