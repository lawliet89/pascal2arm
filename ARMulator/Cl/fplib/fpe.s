; fpe.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.12.2.1 $
; Checkin $Date: 1997/08/21 12:42:56 $
; Revising $Author: ijohnson $

        GET     defaults.s

        GBLL    FPEWanted
        GBLL    FPASCWanted
        GBLL    EnableInterrupts
        GBLA    CoreDebugging

FPEWanted       SETL    {FALSE}
FPASCWanted     SETL    {FALSE}
EnableInterrupts        SETL    {FALSE}
CoreDebugging   SETA    0


;==============================================================================

; 
; Allow some control over the code/speed of code produced.
; 0 = fastest -> 2 = smallest (overall)
; 

        GBLA    CodeSize
CodeSize        SETA    0

        MACRO
        ImportCodeSize $name
        [ CodeSize <> 0
        IMPORT  $name
        ]
        MEND

        MACRO
        ExportCodeSize $name
        [ CodeSize <> 0
        EXPORT  $name
        ]
        MEND

;==============================================================================

        MACRO
$label  CDebug4 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug3 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug2 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug1 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug0 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND

; File for setting up THUMB macros for entry and exit from the THUMB
; versions of the functions.

        GBLA    V_N

        [ :DEF: thumb

        MACRO
$label  VEnter_16               ; Like VEnter, but declare __16$label as the
        CODE16                  ; THUMB entry point
__16$label
        BX      pc
        CODE32
V_N     SETA    V_N+1
        LCLS    f_lab
f_lab   SETS    "|":CC:"$F":CC::STR:V_N:CC:"|"
        KEEP    $f_lab
$f_lab
$label  STMFD   r13!,veneer_s   ; ARM is the declared entry point
        MEND

        MACRO
$label  VEnter                  ; Declare the THUMB entry point as the main
        CODE16                  ; entry point
$label  BX      pc
        CODE32
V_N     SETA    V_N+1
        LCLS    f_lab
f_lab   SETS    "|":CC:"$F":CC::STR:V_N:CC:"|"
        KEEP    $f_lab
$f_lab
__32$label                      ; Declare a __32$label entry point for ARM
        STMFD   r13!,veneer_s
        MEND

        MACRO
$label  VReturn $cc
$label  LDM$cc.FD r13!,veneer_s
        BX$cc   lr
        MEND

        MACRO
$label  VPull $cc
$label  LDM$cc.FD r13!,veneer_s
        MEND

        MACRO
$label  ReturnToLR $cc
$label  BX$cc   lr
        MEND

        MACRO
$label  ReturnToStack $cc
$label  LDM$cc.FD sp!,{lr}
        BX$cc   lr
        MEND

        MACRO
$label  PullFromStack $cc
$label  LDM$cc.FD sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR_16
        CODE16
__16$label
        BX      pc
        CODE32
V_N     SETA    V_N+1
        LCLS    f_lab
f_lab   SETS    "|":CC:"$F":CC::STR:V_N:CC:"|"
        KEEP    $f_lab
$f_lab
$label
        MEND

        MACRO
$label  EnterWithStack_16
        CODE16
__16$label
        BX      pc
        CODE32
V_N     SETA    V_N+1
        LCLS    f_lab
f_lab   SETS    "|":CC:"$F":CC::STR:V_N:CC:"|"
        KEEP    $f_lab
$f_lab
$label  STMFD   sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR
        CODE16
$label  BX      pc
        CODE32
V_N     SETA    V_N+1
        LCLS    f_lab
f_lab   SETS    "|":CC:"$F":CC::STR:V_N:CC:"|"
        KEEP    $f_lab
$f_lab
__32$label
        MEND

        MACRO
$label  EnterWithStack
        CODE16
$label  BX      pc
        CODE32
V_N     SETA    V_N+1
        LCLS    f_lab
f_lab   SETS    "|":CC:"$F":CC::STR:V_N:CC:"|"
        KEEP    $f_lab
$f_lab
__32$label
        STMFD   sp!,{lr}
        MEND
        
        MACRO
        Export $name
        EXPORT  $name
        EXPORT  __16$name
        MEND

        MACRO
        Export_32 $name
        EXPORT  __32$name
        EXPORT  $name
        MEND

        MACRO
        Import_32 $name
        IMPORT __32$name
        MEND

        MACRO
$label  B_32 $name
$label  B       __32$name
        MEND

        |

;ARM 32 and 26-bit mode

        MACRO
$label  VEnter_16
$label  STMFD   r13!,veneer_s
        MEND

        MACRO
$label  VEnter
$label  STMFD   r13!,veneer_s
        MEND

        MACRO
$label  VReturn $cc
$label  [ :DEF: INTERWORK
        LDM$cc.FD r13!,veneer_s
        BX$cc   lr
        |
        [ {CONFIG} = 32 
        LDM$cc.FD r13!,veneer_l
        ]
        [ {CONFIG} = 26
        LDM$cc.FD r13!,veneer_l^
        ]
        ]
        MEND    

        MACRO
$label  VPull $cc
$label  LDM$cc.FD r13!,veneer_s
        MEND

        MACRO
$label  ReturnToLR $cc
$label  [ :DEF: INTERWORK
        BX      lr
        |
        [ {CONFIG} = 32
        MOV$cc  pc,lr
        ]
        [ {CONFIG} = 26
        MOV$cc.S pc,lr
        ]
        ]
        MEND

        MACRO
$label  ReturnToStack $cc
$label  [ :DEF: INTERWORK
        LDM$cc.FD sp!,{lr}
        BX      lr
        |
        [ {CONFIG} = 32
        LDM$cc.FD sp!,{pc}
        ]
        [ {CONFIG} = 26
        LDM$cc.FD sp!,{pc}^
        ]
        ]
        MEND

        MACRO
$label  PullFromStack $cc
$label  LDM$cc.FD sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR_16
$label
        MEND

        MACRO
$label  EnterWithStack_16
$label  STMFD   sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR
$label
        MEND

        MACRO
$label  EnterWithStack
$label  STMFD   sp!,{lr}
        MEND

        MACRO
        Export $name
        EXPORT  $name
        MEND

        MACRO
        Export_32 $name
        EXPORT  $name
        MEND

        MACRO
        Import_32 $name
        IMPORT $name
        MEND

        MACRO
$label  B_32 $name
$label  B       $name
        MEND

        ]

        MACRO
        CodeArea $name
;; $name will be a name for the area. However for a release we'll use
;; C$$code instead
        [ :DEF: INTERWORK
        AREA    |C$$code|,CODE,READONLY,INTERWORK
        |
        AREA    |C$$code|,CODE,READONLY
        ]
        MEND

        GET     regnames.s
        GET     armdefs.s
        GET     fpadefs.s
        GET     macros.s

sp      RN      R13
lr      RN      R14
pc      RN      R15

dOP1h   RN      R0      ;Double OP1 hi-reg ("First word") - sign,expn,etc.
dOP1l   RN      R1      ;Double OP1 lo-reg ("Second word")
dOPh    RN      R0      ;Double OP hi-reg (unary ops)
dOPl    RN      R1      ;Double OP lo-reg
dOP2h   RN      R2      ;Double OP2 hi-reg ("First word")
dOP2l   RN      R3      ;Double OP2 lo-reg ("Second word")

fOP1    RN      R0      ;Float OP1
fOP     RN      R0      ;Float OP for unary ops
fOP2    RN      R1      ;Float OP2

utmp1   RN      R2      ;Temporary register fo unary operations
utmp2   RN      R3      ;    "

ip      RN      R12
tmp     RN      R12     ;A temporary register

SignBit         EQU     &80000000
fSignalBit      EQU     &00400000
dSignalBit      EQU     &00080000
Internal_mask   EQU     &00000000
Single_pos      EQU     0
Double_pos      EQU     1
Single_mask     EQU     1:SHL:Single_pos
Double_mask     EQU     1:SHL:Double_pos
Reverse         EQU     0x4     ; Used to signal a reverse divide

;;Error flags - an extension to the normal internal format

Error_pos       EQU     29
Error_bit       EQU     1:SHL:Error_pos

Except_len      EQU     5
Except_pos      EQU     Error_pos-Except_len

        ASSERT  IOC_pos < DZC_pos
        ASSERT  DZC_pos < OFC_pos
        ASSERT  OFC_pos < UFC_pos
        ASSERT  UFC_pos < IXC_pos
FPExceptC_pos   EQU     IOC_pos
        ASSERT  IOE_pos < DZE_pos
        ASSERT  DZE_pos < OFE_pos
        ASSERT  OFE_pos < UFE_pos
        ASSERT  UFE_pos < IXE_pos
FPExceptE_pos   EQU     IOE_pos

INX_pos         EQU     Except_pos-FPExceptC_pos+IXC_pos
INX_bit         EQU     1:SHL:INX_pos
INX_bits        EQU     INX_bit :OR: Error_bit :OR: Uncommon_bit

UNF_pos         EQU     Except_pos-FPExceptC_pos+UFC_pos
UNF_bit         EQU     1:SHL:UNF_pos
UNF_bits        EQU     UNF_bit :OR: Error_bit :OR: Uncommon_bit

OVF_pos         EQU     Except_pos-FPExceptC_pos+OFC_pos
OVF_bit         EQU     1:SHL:OVF_pos
OVF_bits        EQU     OVF_bit :OR: Error_bit :OR: Uncommon_bit

DVZ_pos         EQU     Except_pos-FPExceptC_pos+DZC_pos
DVZ_bit         EQU     1:SHL:DVZ_pos
DVZ_bits        EQU     DVZ_bit :OR: Error_bit :OR: Uncommon_bit

IVO_pos         EQU     Except_pos-FPExceptC_pos+IOC_pos
IVO_bit         EQU     1:SHL:IVO_pos
IVO_bits        EQU     IVO_bit :OR: Error_bit :OR: Uncommon_bit

; Used to identify the width of the error to the error handlers.

DoubleErr_pos   EQU     23
DoubleErr_bit   EQU     1:SHL:DoubleErr_pos

SingleErr_pos   EQU     22
SingleErr_bit   EQU     1:SHL:SingleErr_pos

LongErr_pos     EQU     21
LongErr_bit     EQU     1:SHL:LongErr_pos

LongLongErr_pos EQU     20
LongLongErr_bit EQU     1:SHL:LongLongErr_pos

ZeroErr_pos     EQU     19
ZeroErr_bit     EQU     1:SHL:ZeroErr_pos

UnsignedErr_pos EQU     18
UnsignedErr_bit EQU     1:SHL:UnsignedErr_pos

CardinalErr_bits EQU    LongErr_bit:OR:LongLongErr_bit:OR:ZeroErr_bit:OR:UnsignedErr_bit

EDOM            EQU     1
ERANGE          EQU     2
ESIGNUM         EQU     3

veneer_s        RLIST   {r4-r9,r11,lr}
veneer_l        RLIST   {r4-r9,r11,pc}

        END
