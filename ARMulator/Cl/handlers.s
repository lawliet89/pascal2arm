;;; handlers.s: interface of low-level error handling to C signals.
;;;
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.8.16.1 $
;;; Checkin $Date: 1997/06/05 13:51:53 $
;;; Revising $Author: wdijkstr $

        GET     objmacs.s
        GET     h_errors.s
 [ backtrace_enabled
        GET     h_uwb.s
 ]

        CodeArea

fp  RN 11

        IMPORT  |__rt_unwind_32|
        IMPORT  |__rt_unwind|
        IMPORT  |__rt_fpavailable|
        IMPORT  |__rt_fpavailable_32|

        IMPORT  raise


SIGFPE  *       2
SIGILL  *       3
SIGINT  *       4
SIGSEGV *       5
SIGSTAK *       7
SIGOSERROR *    10

        Function _clib_SigInt
        ; Internal to library: entered (in reentrant case)
        ; with sb correctly set up.

        FunctionEntry
        LDR     ip, addr__interrupts_off
        LDR     a1, [ip]
        CMPS    a1, #0
        MOV     a1, #SIGINT
        STRNE   a1, [ip, #4]            ; mark pending
        BLEQ    _handler_raise
        Return

_handler_raise
  [ INTERWORK :LOR: THUMB
        LDR     ip, =raise
    [ INTERWORK
        IMPORT  __call_ip
        B       __call_ip
    |
        BX      ip
    ]
  |
        B       raise
  ]

SignalNumber
        ; nb required not to disturb a4
        MOV     a2, #SIGOSERROR
        CMP     a1, #Error_IllegalInstruction
        CMPNE   a1, #Error_PrefetchAbort
        CMPNE   a1, #Error_BranchThroughZero
        MOVEQ   a2, #SIGILL
        CMP     a1, #Error_DataAbort
        CMPNE   a1, #Error_AddressException
        MOVEQ   a2, #SIGSEGV
        LDR     a3, =Error_FPBase
        CMP     a1, a3
        ADD     a3, a3, #Error_FPLimit-Error_FPBase-1
        CMPHS   a3, a1
        MOVHS   a2, #SIGFPE
        CMP     a1, #Error_DivideByZero
        MOVEQ   a2, #SIGFPE
        CMP     a1, #Error_StackOverflow
        MOVEQ   a2, #SIGSTAK
        LDR     a3, =Error_ReadFail
        SUBS    a3, a1, a3
        CMPNE   a3, #Error_WriteFail-Error_ReadFail
        MOVEQ   a2, #SIGSEGV
        MOV     a1, a2
        Return  , "", LinkNotStacked


        Function _clib_TrapHandler
        EXPORT _clib_TrapHandler_32
_clib_TrapHandler_32
        ; Internal to library: entered (in reentrant case)
        ; with sb correctly set up.

        Push    "a2,r14"
        MOV     a4, a1
        BL      SignalNumber
        Pop     "a2,r14"
RaiseIt
        LDR     ip, addr__inSignalHandler
        MOV     a3, #1
        STRB    a3, [ip, #0]
        Push    "a2,a4"             ; ptr to dumped registers and raw error number
                                    ; left on stack for _postmortem to find
        BL      _handler_raise      ; raise desired signal
Raised
        B       |_postmortem_32|    ; and if user insists on returning from
                                    ; signal handler, give him hell!


        Function _raise_stacked_interrupts
        ; Internal to library: entered (in reentrant case)
        ; with sb correctly set up.

        LDR     ip, addr__interrupts_off
        MOV     a2, #0
        STR     a2, [ip, #0]
        LDR     a1, [ip, #4]
        CMPS    a1, #0
        Return  , "", LinkNotStacked, EQ
        STR     a2, [ip, #4]
        B       _handler_raise

        Function _postmortem
_postmortem_32
        FunctionEntry UsesSb, "v1,v2,v3,v4,v5", makeframe

 [ :LNOT: backtrace_enabled
        IMPORT  |__rt_exit_32|
        MOV     r1, #1
        B       |__rt_exit_32|
 |
        IMPORT  |_backtrace|
        BL      |__rt_fpavailable_32|
        MOV     ip, sp

        ; set up an unwind block on the stack
        CMP     a1, #0
        SUBEQ   sp, sp, #uwb_size-uwb_f4
        BEQ     postmortem_nofp
 [ FPE2
        STFE    f7, [sp, #-12]!
        STFE    f6, [sp, #-12]!
        STFE    f5, [sp, #-12]!
        STFE    f4, [sp, #-12]!
 |
        SFMFD   f4, 4, [sp]
 ]
postmortem_nofp
        Push    "sl"
 [ make = "shared-library"
   [ LDM_MAX >= 4
        LDMDB   fp, {a1-a4}     ; sb, fp, sp, lr
        Push    "a1,a2,a3,a4"
        Push    "v1,v2,v3,v4,v5"; unchanged so far
   |
        LDMDB   fp, {a1-a3}     ; fp, sp, lr
        Push    "a1,a2,a3"
        LDR     lr, [fp, #-16]  ; sb
        Push    "v1,v2,v3,v4,v5,lr"; v1-v5 unchanged so far
   ]
 |
        LDMDB   fp, {a1-a3}     ; fp, sp, lr
        Push    "a1,a2,a3"
        Push    "v1,v2,v3,v4,v5,v6" ; unchanged so far
 ]
        ; unwind a few frames in an attempt to remove ones belonging to the
        ; signal edifice from the traceback.
        ; make a copy of the unwindblock to use in the unwind
        ADD     a3, sp, #uwb_size
        Pop     "a4,v1,v2,v3,v4,v5,ip,lr",,,a3
        Push    "a4,v1,v2,v3,v4,v5,ip,lr"
        Pop     "a4,v1,v2,v3,v4,v5,ip",,,a3
        Push    "a4,v1,v2,v3,v4,v5,ip"
        Pop     "a4,v1,v2,v3,v4,v5,ip",,,a3
        Push    "a4,v1,v2,v3,v4,v5,ip"
        ; reserve a word for __rt_unwind's second argument to point to
        ; (what was this for?  Seems not to be used any more)
        SUB     sp, sp, #4
        MOV     v1, #4
02      SUBS    v1, v1, #1
        BMI     %F01
        ADD     a2, sp, #0
        ADD     a1, sp, #4
        BL      |__rt_unwind_32|
        CMP     a1, #0
        BLE     %F01
        ; we've succeeded if we find a link value pointing at the label
        ; Raised (in _clib_TrapHandler)
        LDR     a1, [sp, #4+uwb_pc]
        BIC     a1, a1, #&FC000003
        ADR     a2, Raised
        CMP     a1, a2
        BNE     %B02

        ; update the unwindblock from the copy (and pop the copy)
        ADD     sp, sp, #4
        ADD     a3, sp, #uwb_size
        LDR     a1, [sp, #uwb_sp]
        LDR     a4, [a1]
        Pop     "a2,v1,v2,v3,v4,v5,ip,lr" ; v1-v6, fp, sp
        PushA   "a2,v1,v2,v3,v4,v5,ip,lr",a3
        Pop     "a2,v1,v2,v3,v4,v5,ip"
        LDR     a2, [a4, #pc*4]
        PushA   "a2,v1,v2,v3,v4,v5,ip",a3 ;  pc, sl, five words of fp regs
        Pop     "a2,v1,v2,v3,v4,v5,ip"    ;) seven words of fp regs
        PushA   "a2,v1,v2,v3,v4,v5,ip",a3 ;)
        LDR     a2, [a4, #r0*4]         ; r0 at time of fault (= faulty address for __rt_xxxchk)
        LDR     a1, [a1, #4]            ; error code
        B       %F02

01
        ; pop the working unwindblock copy
        ADD     sp, sp, #4+uwb_size
        MOV     a1, #-1
        MOV     a2, #0
02
        MOV     a3, sp
        BL      |_backtrace|
        Return  UsesSb, "v1,v2,v3,v4,v5", fpbased
 ]

        AdconTable

        IMPORT  |_interrupts_off|
addr__interrupts_off
        &       |_interrupts_off|

        IMPORT  |_inSignalHandler|
addr__inSignalHandler
        &       |_inSignalHandler|

        END
