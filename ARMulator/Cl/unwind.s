;;; unwind.s: call stack unwind
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.6 $
;;; Checkin $Date: 1994/12/22 17:24:11 $
;;; Revising $Author: enevill $

        GET     h_uwb.s
        GET     h_stack.s
        GET     h_hw.s
        GET     objmacs.s

        EXPORT  |__rt_unwind_aborted|

        CodeArea

; int __rt_unwind(__rt_unwindblock *inout);
        Function __rt_unwind
        EXPORT  __rt_unwind_32
__rt_unwind_32
        FunctionEntry UsesSb, "v1,v2,v3,v4,v5"
        LDR     ip, addr___rt_unwinding
        MOV     v2, #1
        STRB    v2, [ip]
        LDR     a4, [a1, #uwb_fp]
        BICS    a4, a4, #ChunkChange
        MOVEQ   v5, #0
        BEQ     duh_exit

        LDR     a3, [a4, #frame_entrypc]
        BIC     a3, a3, #PSRBits
        LDR     v1, [a3, #-12]

        ; check that the save mask instruction is indeed the right sort of STM
        ; If not, return indicating stack corruption.
        MOV     ip, v1, LSR #16
        EOR     ip, ip, #&e900
        EOR     ip, ip, #&002c
        BICS    ip, ip, #1              ; STMFD sp!, ... (sp = r12 or r13)
        BNE     duh_corrupt

        ; update register values in the unwindblock which the save mask says
        ; were saved in this frame.
        MOV     ip, #1
        SUB     v2, a4, #frame_prevfp
        MOV     v3, #r9
        ADD     v4, a1, #uwb_r4-r4*4
01      TST     v1, ip, ASL v3
        LDRNE   r14, [v2, #-4]!
        STRNE   r14, [v4, v3, ASL #2]
        SUB     v3, v3, #1
        CMP     v3, #v1
        BGE     %B01

        ; now look for floating point stores immediately after the savemask
        ; instruction, updating values in the saveblock if they are there.
        SUB     a3, a3, #8
        LDR     v4, =&ed6c0103
02      LDR     v1, [a3], #+4
        BIC     r14, v1, #&7000
        CMP     r14, v4
        BNE     UnwindEndFP
        MOV     v1, v1, LSR #10
        AND     v1, v1, #&1c
        ADD     v1, v1, v1, ASL #1
        ADD     v1, a1, v1
        LDR     r14, [v2, #-4]!
        STR     r14, [v1, #uwb_r4-r4*4*3+8]
        LDR     r14, [v2, #-4]!
        STR     r14, [v1, #uwb_r4-r4*4*3+4]
        LDR     r14, [v2, #-4]!
        STR     r14, [v1, #uwb_r4-r4*4*3]
        B       %B02

UnwindEndFP
        LDMDB   a4, {a3, a4, v1}        ; saved fp, sp, link
        ; if the new fp is in a different stack chunk, must amend sl
        ; in the unwind block.
        TST     a3, #ChunkChange
        BIC     a3, a3, #ChunkChange
        LDR     v3, [a1, #uwb_sl]
        LDRNE   v3, [v3, #SC_prev-SC_SLOffset]
        ADDNE   v3, v3, #SC_SLOffset
        ADD     ip, a1, #uwb_fp
        STMIA   ip!, {a3, a4, v1}
        STMIA   ip, {v3}
        MOV     v3, a2
        MOV     v5, #1
duh_exit
        MOV     a1, #0
        LDR     ip, addr___rt_unwinding
        STRB    a1, [ip]
        MOV     a1, v5
        Return  UsesSb, "v1,v2,v3,v4,v5"

        Function __rt_unwind_aborted

 [ {CONFIG} = 26
        TEQP    r14, #PSRBits           ; Back to the mode and interrupt
 |                                      ; status before we got the abort
        MOVS    pc, pc
 ]
        NOP
duh_corrupt
        MOV     v5, #-1
        B       duh_exit

        AdconTable

        IMPORT  |__rt_unwinding|
addr___rt_unwinding
        &       |__rt_unwinding|

        END
