; Assembler source for FPA support code - simple stand-alone veneer
;
; Copyright (C) Advanced RISC Machines Limited, 1997. All rights reserved.
;
; RCS $Revision: 1.2 $
; Checkin $Date: 1997/04/22 18:46:36 $
; Revising $Author: dseal $

        AREA    Simple_FPASC, CODE

                        GBLA    ExpectsCoreVersion
ExpectsCoreVersion      SETA    1

        MACRO
$label  AdrWS   $reg
$label  ADRL    $reg,Workspace
        MEND

        MACRO
$label  FPInstrDone $opt
        [ "$opt" = ""
$label
        |
$label    TEQ     R9,#0         ;Allow the optimisation, assuming a normal
                                ;  instruction address
        ]
        MEND

        IMPORT  veneer_newhandlers
        IMPORT  veneer_preservehandlers
        IMPORT  veneer_restorehandlers
        IMPORT  FPA_ABSENT
        IMPORT  FPASC_UNHANDLED

; The stack areas.

UndefStack
        %       512
UndefStackEnd

SvcStack
        %       128
SvcStackEnd

; The initialisation routine.

FPASC_STARTUP
        EXPORT  FPASC_STARTUP

; First set up supervisor mode stack and undefined instruction mode stack.

        ADR     R13,SvcStackEnd
        MRS     R0,CPSR
        BIC     R1,R0,#&1F
        ORR     R1,R1,#&1B
        MSR     CPSR,R1
        NOP
        ADR     R13,UndefStackEnd
        MSR     CPSR,R0
        NOP

; Preserve the return link.

        STMFD   R13!,{R14}

; Get hold of the workspace pointer, in R10.

        AdrWS   R10

; Preserve the current undefined instruction handler.

        BL      veneer_preservehandlers

; Initialise the FPASC core, using our own handler as the "next handler in
; the chain".

        LDR     R1,=FPASC_UNHANDLED
        BL      core_initws

; Check we've got the FPA.

        TEQ     R0,#SysID_FPA
        BNE     FPA_ABSENT

; Activate the one and only floating point context.

        BL      core_activatecontext

; Return.

        LDMFD   R13!,{PC}

        LTORG

        END
