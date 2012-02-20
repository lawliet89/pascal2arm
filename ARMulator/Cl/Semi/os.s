; Lastedit: 27 Aug 92 9:58:45
;
; Purpose: ARMulator-hosted C-library Monitor.
;
; Copyright (C) Advanced RISC Machines Ltd., 1991.
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; Advanced RISC Machines.

;;; RCS $Revision: 1.19 $
;;; Checkin $Date: 1995/01/25 17:21:58 $
;;; Revising $Author: enevill $

        GET     objmacs.s
        GET     h_errors.s
        GET     h_os.s
        IMPORT  __errno
        IMPORT  __rt_trap
        IMPORT  __rt_trap_32
        DataArea

StaticData
__rt_registerDump       ExportedVariable 16
__ClockInit             Variable
        CodeArea

        MACRO
        LoadIPIfReentrant
   [ make = "shared-library"
        MOV     ip, sb          ; intra-link-unit entry
                                ; (sb gets preserved & restored only if needed)
   ]
        MEND

;
; Return, maybe setting errno first
;

        MACRO
        ErrorNEReturn
        BNE     seterrno
        Return
        MEND

        MACRO
        ErrorEQReturn
        BEQ     seterrno
        Return
        MEND

seterrno
        Push    "a1"
        SWI     SWI_GetErrno
        LDR     a2, addr___errno
        STR     a1, [a2]
        Return ,"a1"

;
; I/O Support, via SWI's to the host's OS
;

        Function clock
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Clock
        LDR     a2, addr___ClockInit
        LDR     a2, [a2]
        SUB     a1, a1, a2
        Return

        Function _clock_init
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Clock
        LDR     a2, addr___ClockInit
        STR     a1, [a2]
        Return

        Function time, leaf
        FunctionEntry
        MOVS    a2, a1 ; store to memory too ?
        SWI     SWI_Time ; get the time
        STRNE   a1, [a2] ; store it to memory
        Return

        Function system, leaf
        FunctionEntry
        CMP     a1, #0 ; is there a system
        MOVEQ   a1, #1
        SWINE   SWI_CLI
        Return

        Function getenv, leaf
        MOV     r0, #0
        Return ,, LinkNotStacked

        Function _sys_open
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Open
        CMP     a1, #0
        ErrorEQReturn

        Function _sys_iserror, leaf
        CMN     a1, #1
        MOVEQ   a1, #1
        MOVNE   a1, #0
        Return ,, LinkNotStacked

        Function _sys_close
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Close
        CMP     a1, #0
        ErrorNEReturn

        Function _sys_write
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Write
        CMP     a1, #0
        ErrorNEReturn

        Function _sys_read
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Read
        CMP     a1, #0
        ErrorNEReturn

        Function _sys_seek
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Seek
        CMN     a1, #1
        ErrorEQReturn

        Function _sys_flen
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Flen
        CMN     a1, #1
        ErrorEQReturn

        Function remove
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Remove
        CMP     a1, #0
        ErrorNEReturn

        Function rename
        LoadIPIfReentrant
        FunctionEntry
        SWI     SWI_Rename
        CMP     a1, #0
        ErrorNEReturn

        Function _ttywrch, leaf
        FunctionEntry
        SWI     SWI_WriteC
        Return

; this function is called from _sys_istty, and takes a FILEHANDLE rather
; than a FILE *. _sys_istty is in os.c

        Function __sys_istty, leaf
        FunctionEntry
        SWI     SWI_IsTTY
        Return

        Function _sys_tmpnam, leaf
        FunctionEntry
        SWI     SWI_TmpNam
        Return

;
; Floating Point Support
;
 [ :LNOT: linked_fpe

        Function __fp_initialise, leaf
        EXPORT __fp_initialise_32
__fp_initialise_32
        ; Determine whether FP is available (to decide whether fp regs need
        ; saving over _kernel_system and in setjmp).

        MOV     r0, #4
        LDR     r0, [r0]
        AND     r0, r0, #&ff000000      ; see if B somewhere
        CMP     r0, #&ea000000
        MOVEQ   r0, #1
        MOVNE   r0, #0
        Return  , "", LinkNotStacked


        Function __fp_finalise, leaf
        EXPORT  __fp_finalise_32
__fp_finalise_32
        ; nothing to do.
        Return  , "", LinkNotStacked


        Function __fp_address_in_emulator, leaf
        ; determine whether r0 is an address inside the fp emulator,
        ; (to allow a data abort or address exception in a floating-point
        ; load or store to be reported as occurring at that instruction,
        ; rather than somewhere in the code of the emulator).

        SUB     r0, r0, #FPEStart
        CMP     r0, #(FPEEnd-FPEStart)
        MOVLO   r0, #1
        MOVHS   r0, #0
        Return  , "", LinkNotStacked
 ]
;
; Miscellaneous Support
;

        Function __osdep_traphandlers_init, leaf
        EXPORT __osdep_traphandlers_init_32
__osdep_traphandlers_init_32
        FunctionEntry
        MOV     r0, #0
        ADR     r3, newVectors
01      ADR     r1, trapRegBlock
        LDR     r2, [r3, r0, LSL #2]
        TEQ     r2, #0
        ADDNE   r2, r2, r3
        SWINE   SWI_InstallHandler      ; no SWI handler
        ADD     r0, r0, #1
        CMP     r0, #9
        BNE     %B01
        Return

        Function __osdep_traphandlers_finalise, leaf
        EXPORT __osdep_traphandlers_finalise_32
__osdep_traphandlers_finalise_32
        Return ,, LinkNotStacked

newVectors
        & ResetHandler-newVectors
        & UndefInstrHandler-newVectors
        & 0
        & PrefAbortHandler-newVectors
        & DataAbortHandler-newVectors
        & AddrExceptHandler-newVectors
        & IRQHandler-newVectors
        & FIQHandler-newVectors
        & ErrorHandler-newVectors

        Function __osdep_heapsupport_init, leaf
        EXPORT __osdep_heapsupport_init_32
__osdep_heapsupport_init_32
        Return ,, LinkNotStacked

        Function __osdep_heapsupport_finalise, leaf
        EXPORT __osdep_heapsupport_finalise_32
__osdep_heapsupport_finalise_32
        Return ,, LinkNotStacked

        Function __osdep_heapsupport_extend, leaf
        EXPORT __osdep_heapsupport_extend_32
__osdep_heapsupport_extend_32
        MOV     r0,#0
        Return ,, LinkNotStacked

        Function _hostos_error_string, leaf
        ADR     a1, errorstring
        Return ,, LinkNotStacked

errorstring     = "Unknown Error",0
        ALIGN

ResetHandler
        STMIA   r11, {r0-r2}
        ADR     a1, E_BranchThroughZero
        B       CallTrap

UndefInstrHandler
        STMIA   r11, {r0-r2}
        ADR     a1, E_IllegalInstruction
        B       CallTrap

;SWIHandler
;        STMIA   r11, {r0-r2}
;        ADR     a1, E_UnknownSWI
;        B       CallTrap

PrefAbortHandler
        STMIA   r11, {r0-r2}
        ADR     a1, E_PrefetchAbort
        B       CallTrap

DataAbortHandler
        STMIA   r11, {r0-r2}
        ADR     a1, E_DataAbort
        B       CallTrap

AddrExceptHandler
        STMIA   r11, {r0-r2}
        ADR     a1, E_AddressException
        B       CallTrap

IRQHandler
FIQHandler
        STMIA   r11, {r0-r2}
        ADR     a1, E_UnknownIRQ
        B       CallTrap

ErrorHandler
        STMIA   r11, {r0-r2}
        MOV     a1, r10 ; ; get the error pointer
        B       CallTrap

CallTrap
        ; Max 3 registers at a time in the following LDM/STMs for
        ; lowest allowed interrupt latency (IRQs but not FIQs are
        ; disabled for most faults).
        MOV     r1, r11
        ADD     r2, r1, #3*4
 [ LDM_MAX >= 12
        LDMFD   sp!, {r10 - r12, r14}
        STMIA   r2, {r3-r14}^
 |
        LDMFD   sp!, {r10 - r12}
        LDMFD   sp!, {r14}
        STMIA   r2!, {r3,r4,r5}
        STMIA   r2!, {r6,r7}
        STMIA   r2, {r8,r9,r10}^
        ADD     r2,r2,#12
        STMIA   r2, {r11,r12,r13}^
        ADD     r2,r2,#12
        STMIA   r2, {r14}^
 ]
        STR     r14, [r1, #15*4]
        B       __rt_trap_32

;        ErrorBlock UnknownSWI, "Undefined SWI Instruction"
        ErrorBlock BranchThroughZero, "Branch Through Zero"
        ErrorBlock IllegalInstruction, "Undefined Instruction"
        ErrorBlock PrefetchAbort, "Prefetch Abort"
        ErrorBlock DataAbort, "Data Abort"
        ErrorBlock AddressException, "Address Exception"
        ErrorBlock UnknownIRQ, "Unhandled Interrupt"

trapRegBlock
        % 16 * 4

        AdconTable

addr___errno
        & __errno

addr___ClockInit
        & __ClockInit

        END
