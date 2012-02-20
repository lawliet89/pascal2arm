; Purpose: Semi Hosted C-library SWIs and definitions.
;
; Copyright (C) Advanced RISC Machines Ltd., 1991.
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; Advanced RISC Machines.

;;; RCS $Revision: 1.6 $
;;; Checkin $Date: 1993/12/10 15:12:55 $
;;; Revising $Author: irickard $


SWI_WriteC              EQU     &0
SWI_Write0              EQU     &2
SWI_ReadC               EQU     &4
SWI_CLI                 EQU     &5
SWI_GetEnv              EQU     &10
SWI_Exit                EQU     &11
SWI_EnterOS             EQU     &16

SWI_GetErrno            EQU     &60
SWI_Clock               EQU     &61
SWI_Time                EQU     &63
SWI_Remove              EQU     &64
SWI_Rename              EQU     &65
SWI_Open                EQU     &66

SWI_Close               EQU     &68
SWI_Write               EQU     &69
SWI_Read                EQU     &6a
SWI_Seek                EQU     &6b
SWI_Flen                EQU     &6c

SWI_IsTTY               EQU     &6e
SWI_TmpNam              EQU     &6f
SWI_InstallHandler      EQU     &70
SWI_GenerateError       EQU     &71

FPEStart                EQU     &1400
FPEEnd                  EQU     &8000

        END
