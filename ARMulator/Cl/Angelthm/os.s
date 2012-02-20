; Lastedit: 27 Aug 92 9:58:45
;
; Purpose: ARMulator-hosted C-library Monitor.
;
; Copyright (C) Advanced RISC Machines Ltd., 1991.
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; Advanced RISC Machines.

;;; RCS $Revision: 1.2.10.1 $
;;; Checkin $Date: 1998/02/25 17:36:15 $
;;; Revising $Author: dbrooke $

        GET     objmacs.s
        GET     listopts.s
        GET     cl_lolvl.s
        GET     h_os.s
        IMPORT  __errno
        IMPORT  __rt_trap
        IMPORT  __rt_trap_32
        DataArea

StaticData
__rt_registerDump       ExportedVariable 16
        CodeArea

        MACRO
        LoadIPIfReentrant
   [ make = "shared-library"
        MOV     ip, sb          ; intra-link-unit entry
                                ; (sb gets preserved & restored only if needed)
   ]
        MEND


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

        MACRO
        ADPErrorBlock $name, $string
E_$name
        &       ADP_Stopped_$name
        =       "$string", 0
        ALIGN
        MEND
        
        ADPErrorBlock BranchThroughZero, "Branch Through Zero"
        ADPErrorBlock UndefinedInstr, "Undefined Instruction"
        ADPErrorBlock SoftwareInterrupt, "Undefined SWI Instruction"
        ADPErrorBlock PrefetchAbort, "Prefetch Abort"
        ADPErrorBlock DataAbort, "Data Abort"
        ADPErrorBlock AddressException, "Address Exception"
        ADPErrorBlock IRQ, "Unhandled Interrupt"
        ADPErrorBlock FIQ, "Unhandled Fast Interrupt"
ErrorTable
        &  E_BranchThroughZero
        &  E_UndefinedInstr
        &  E_SoftwareInterrupt
        &  E_PrefetchAbort
        &  E_DataAbort
        &  E_AddressException
        &  E_IRQ
        &  E_FIQ
                
__rt_trap_wrap
        ADR     r2,ErrorTable
        LDR     r0,[r2,r0, LSL#2]
        B       __rt_trap_32

trapRegBlock
        % 16 * 4


        END
