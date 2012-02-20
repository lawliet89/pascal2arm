;;; h_objmacro.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.13 $
;;; Checkin $Date: 1995/06/11 18:40:07 $
;;; Revising $Author: irickard $

 [ :LNOT: :DEF: THUMB
        GBLL    THUMB
    [ {CONFIG} = 16
THUMB   SETL    {TRUE}
    |
THUMB   SETL    {FALSE}
    ]
 ]
 [ THUMB
        CODE32
 ]
 [ :LNOT: :DEF: INTERWORK
        GBLL    INTERWORK
INTERWORK SETL {FALSE}
 ]

 [ :LNOT: :DEF: LDM_MAX
        GBLA    LDM_MAX
LDM_MAX SETA    16
 ]

 [ :LNOT: :DEF: make
        GBLS    make
make    SETS    "all"
 ]

 [ :LNOT: :DEF: FPIS
        GBLA    FPIS
FPIS    SETA    3
 ]

        GBLL    FPE2
FPE2    SETL    FPIS=2

        GBLS    VBar
        GBLS    UL
        GBLS    XXModuleName

VBar    SETS    "|"
UL      SETS    "_"

        GET     h_la_obj.s

        MACRO
        Module  $name
XXModuleName SETS UL:CC:"$name":CC:UL
        MEND

        MACRO
$Label  Variable $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:XXModuleName:CC:"$Label":CC:VBar
        KEEP    $Temps
        ALIGN
O_$Label *      .-StaticData
$Temps  %       &$Tempa*4
        MEND

        MACRO
$Label  ExportedVariable $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:"$Label":CC:VBar
        EXPORT  $Temps
        ALIGN
O_$Label *      .-StaticData
$Temps  %       &$Tempa*4
        MEND

        MACRO
$Label  ExportedWord $Value
        LCLS    Temps
Temps   SETS    VBar:CC:"$Label":CC:VBar
        EXPORT  $Temps
        ALIGN
O_$Label *      .-StaticData
$Temps   &      $Value
        MEND

        MACRO
$Label  ExportedVariableByte $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:"$Label":CC:VBar
        EXPORT  $Temps
O_$Label *      .-StaticData
$Temps  %       &$Tempa
        MEND

        MACRO
$Label  VariableByte $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:XXModuleName:CC:"$Label":CC:VBar
        KEEP    $Temps
O_$Label *      .-StaticData
$Temps  %       &$Tempa
        MEND

        MACRO
$Label  InitByte $Value
$Label  =        $Value
        MEND

        MACRO
$Label  InitWord $Value
$Label  &        $Value
        MEND

        MACRO
$Label  Keep    $Arg
        LCLS    Temps
$Label  $Arg
Temps   SETS    VBar:CC:XXModuleName:CC:"$Label":CC:VBar
        KEEP    $Temps
$Temps
        MEND


        MACRO
        PopFrame  $Regs,$Caret,$CC,$Base
 [ {CONFIG} = 16
 ! 1, "MACRO PopFrame not implemented for 16 bit code"
 ]
        LCLS    BaseReg
 [ "$Base" = ""
BaseReg SETS    "fp"
 |
BaseReg SETS    "$Base"
 ]
        LCLA    count
        LCLS    s
        LCLS    ch
s       SETS    "$Regs"
count   SETA    1
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
count       SETA    count+1
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            ]
          ]
        WEND
 [ LDM_MAX < count
        LCLA    r
        LCLS    subs
s       SETS    "$Regs"
subs    SETS    ""
r       SETA    0
        SUB     ip, $BaseReg, #4*&$count
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
              LDM$CC.FD ip!,{$subs}
subs          SETS    ""
r             SETA    0
            |
subs          SETS    subs:CC:ch
            ]
          |
            [ ch = "-"
              ! 1, "Push can't handle register list including '-'"
            |
subs          SETS    subs:CC:ch
            ]
          ]
        WEND
        LDM$CC.FD   ip,{$subs}$Caret
 |
        LDM$CC.DB   $BaseReg, {$Regs}$Caret
 ]
        MEND

        MACRO
        Pop     $Regs,$Caret,$CC,$Base
 [ {CONFIG} = 16
  [ "$Caret" <> ""
  ! 1, "Cannot use writeback in 16 bit code"
  ]
  [ "$Base" <> ""
  ! 1, "Cannot specify a base register in 16 bit code"
  ]
 ]
        LCLS    BaseReg
 [ "$Base" = ""
BaseReg SETS    "sp"
 |
BaseReg SETS    "$Base"
 ]
 [ {CONFIG} = 16
  [ "$CC" <> "" :LAND: "$CC" <> "AL"
    BN$CC       %FT0
  ]
 ]
 [ LDM_MAX < 16
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
subs    SETS    ""
r       SETA    0
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
             [ {CONFIG} = 16
              POP       {$subs}
             |
              LDM$CC.FD   $BaseReg!,{$subs}
             ]
subs          SETS    ""
r             SETA    0
            |
subs          SETS    subs:CC:ch
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    subs:CC:ch
            ]
          ]
        WEND
  [ {CONFIG} = 16
        POP         {$subs}
0
  |
        LDM$CC.FD   $BaseReg!,{$subs}$Caret
  ]
 |
  [ {CONFIG} = 16
        POP         {$Regs}
0
  |
        LDM$CC.FD   $BaseReg!, {$Regs}$Caret
  ]
 ]
        MEND

        MACRO
        Return  $UsesSb, $ReloadList, $Base, $CC
 [ {CONFIG} = 16
  [ "$UsesSb" <> ""
        !       1, "Cannot use SB in 16 bit code"
  ]
  [ "$Base" = "fpbased"
        !       1, "Cannot use fpbased in 16 bit code"
  ]
 ]
        LCLS    Temps
Temps   SETS    "$ReloadList"
 [ "$UsesSb" <> "" :LAND: make="shared-library"
   [ Temps = ""
Temps   SETS    "sb"
   |
Temps   SETS    Temps:CC:", sb"
   ]
 ]

 [ {CONFIG} = 26
   [ "$Base" = "LinkNotStacked" :LAND: "$ReloadList"=""
          MOV$CC.S pc, lr
   |
     [ Temps <> ""
Temps   SETS    Temps :CC: ","
     ]
     [ "$Base" = "fpbased"
       PopFrame "$Temps.fp,sp,pc",^,$CC
     |
       Pop      "$Temps.pc",^,$CC
     ]
   ]
 |
  [ {CONFIG} = 16
      [ "$CC" <> "" :LAND: "$CC" <> "AL"
        BN$CC   %FT0
      ]
   [ "$Base" = "LinkNotStacked" :LAND: "$ReloadList"=""
     [ INTERWORK
        BX      lr
     |
        MOV     pc, lr
     ]
   |
     [ Temps <> ""
Temps   SETS    Temps :CC: ","
     ]
     [ INTERWORK
       BX       pc
       CODE32
       Pop      "$Temps.lr",,
       BX       lr
       CODE16
     |
       Pop      "$Temps.pc",,
     ]
   ]
0
  |
   [ "$Base" = "LinkNotStacked" :LAND: "$ReloadList"=""
    [ INTERWORK :LOR: THUMB
        BX$CC lr
    |
        MOV$CC pc, lr
    ]
   |
     [ Temps <> ""
Temps   SETS    Temps :CC: ","
     ]
    [ INTERWORK :LOR: THUMB
     [ "$Base" = "fpbased"
       PopFrame "$Temps.fp,sp,lr",,$CC
     |
       Pop      "$Temps.lr",,$CC
     ]
     BX$CC lr
    |
     [ "$Base" = "fpbased"
       PopFrame "$Temps.fp,sp,pc",,$CC
     |
       Pop      "$Temps.pc",,$CC
     ]
    ]
   ]
  ]
 ]
        MEND

        MACRO
        PushA   $Regs,$Base
 [ {CONFIG} = 16
        !       1, "MACRO PushA not implemented for 16 bit code"
 ]
        ; Push registers on an empty ascending stack, with stack pointer $Base
        ; (no default for $Base, won't be sp)
 [ LDM_MAX < 16
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
subs    SETS    ""
r       SETA    0
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
              STMIA   $Base!,{$subs}
subs          SETS    ""
r             SETA    0
            |
subs          SETS    subs:CC:ch
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    subs:CC:ch
            ]
          ]
        WEND
        STMIA   $Base!,{$subs}
 |
        STMIA   $Base!, {$Regs}
 ]
        MEND

        MACRO
        PopA    $Regs,$Base
  [ {CONFIG} = 16
        !       1, "MACRO PopA not implemented for 16 bit code"
  ]
        ; Pop registers from an empty ascending stack, with stack pointer $Base
        ; (no default for $Base, won't be sp)
 [ LDM_MAX < 16
        LCLA    n
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
n       SETA    :LEN:s
subs    SETS    ""
r       SETA    0
        WHILE   n<>0
n         SETA    n-1
ch        SETS    s:RIGHT:1
s         SETS    s:LEFT:n
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
              LDMDB   $Base!,{$subs}
subs          SETS    ""
r             SETA    0
            |
subs          SETS    ch:CC:subs
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    ch:CC:subs
            ]
          ]
        WEND
        LDMDB   $Base!,{$subs}
 |
        LDMDB   $Base!, {$Regs}
 ]
        MEND

        MACRO
        Push    $Regs, $Base
        LCLS    BaseReg
 [ "$Base" = ""
BaseReg SETS    "sp"
 |
BaseReg SETS    "$Base"
 ]
 [ LDM_MAX < 16
        LCLA    n
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
n       SETA    :LEN:s
subs    SETS    ""
r       SETA    0
        WHILE   n<>0
n         SETA    n-1
ch        SETS    s:RIGHT:1
s         SETS    s:LEFT:n
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
             [ {CONFIG} = 16
              PUSH    {$subs}
             |
              STMFD   $BaseReg!,{$subs}
             ]
subs          SETS    ""
r             SETA    0
            |
subs          SETS    ch:CC:subs
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    ch:CC:subs
            ]
          ]
        WEND
  [ {CONFIG} = 16
        PUSH    {$subs}
  |
        STMFD   $BaseReg!,{$subs}
  ]
 |
  [ {CONFIG} = 16
        PUSH    {$Regs}
  |
        STMFD   $BaseReg!, {$Regs}
  ]
 ]
        MEND

        MACRO
        FunctionEntry $UsesSb, $SaveList, $MakeFrame
  [ {CONFIG} = 16
    [ "$UsesSb" <> ""
        !       1, "Cannot use SB in 16 bit code"
    ]
    [ "$MakeFrame" <> ""
        !       1, "Cannot make frame in 16 bit code"
    ]
  ]
        LCLS    Temps
        LCLS    TempsC
Temps   SETS    "$SaveList"
TempsC  SETS    ""
 [ Temps <> ""
TempsC SETS Temps :CC: ","
 ]

 [ "$UsesSb" <> "" :LAND: make="shared-library"
   [ "$MakeFrame" = ""
        MOV     ip, sb  ; intra-link-unit entry
        Push    "$TempsC.sb,lr"
        MOV     sb, ip
   |
        MOV     ip, sb  ; intra-link-unit entry
        Push    "sp,lr,pc"
        ADD     lr, sp, #8
        Push    "$TempsC.sb,fp"
        MOV     fp, lr
        MOV     sb, ip
   ]
 |
   [ "$MakeFrame" = ""
        Push    "$TempsC.lr"
   |
        MOV     ip, sp
        Push    "$TempsC.fp,ip,lr,pc"
        SUB     fp, ip, #4
   ]
 ]
        MEND

        END
