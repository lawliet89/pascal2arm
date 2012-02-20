; Assembler source for FPA support code - simple stand-alone veneer
;
; Copyright (C) Advanced RISC Machines Limited, 1997. All rights reserved.
;
; RCS $Revision: 1.2 $
; Checkin $Date: 1997/04/22 18:46:10 $
; Revising $Author: dseal $

; Various exits:

        IMPORT  COREFAULTY

veneer_corefaulty
        B       COREFAULTY

        IMPORT  COREDISABLED

veneer_coredisabled
        B       COREDISABLED

        IMPORT  FP_TRAP

veneer_invalidop1_single
veneer_invalidop2_single
veneer_invalidop1_double
veneer_invalidop2_double
veneer_invalidop1_extended
veneer_invalidop2_extended
veneer_invalidop1_integer
veneer_invalidop2_integer
veneer_invalidop1_packed
veneer_invalidop1_xpacked
veneer_invalidop1_ldfp
veneer_invalidop1_ldfpx
veneer_zerodivide1_single
veneer_zerodivide2_single
veneer_zerodivide1_double
veneer_zerodivide2_double
veneer_zerodivide1_extended
veneer_zerodivide2_extended
veneer_overflow_single
veneer_overflow_double
veneer_overflow_extended
veneer_underflow_single
veneer_underflow_double
veneer_underflow_extended
veneer_inexact_single
veneer_inexact_double
veneer_inexact_extended
veneer_inexact_integer
veneer_inexact_packed
veneer_inexact_xpacked
        B       FP_TRAP

; The global workspace, with its single included context (because
; "MultipleContexts" is {FALSE}).

Workspace       %       WorkspaceLength

        END
