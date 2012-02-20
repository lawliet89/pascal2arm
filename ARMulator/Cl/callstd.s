;;; h_callstd.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.4 $
;;; Checkin $Date: 1995/01/11 11:23:46 $
;;; Revising $Author: enevill $

        GBLS    VBar
        GBLS    UL
        GBLS    XXModuleName

VBar    SETS    "|"
UL      SETS    "_"

        MACRO
        LoadStaticAddress $Addr, $Reg, $Ignore
        LDR     $Reg, =$Addr
        MEND

        MACRO
        LoadStaticBase $Reg, $Ignore
        LoadStaticAddress StaticData, $Reg, $Ignore
        MEND

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
        Procedure $Name, $Ignore
        LCLS    Temps
Temps   SETS    VBar:CC:"$Name":CC:VBar
        EXPORT  $Temps
 [ THUMB :LAND: {CONFIG} <> 16
        CODE16
$Temps
        BX      pc
        CODE32
 |
$Temps
 ]
        MEND

        MACRO
        Return $CC, $ReloadList
 [ {CONFIG} = 26
   [ ReloadList = ""
        MOV$CC.S pc, lr
   |
        LDM$CC.FD sp!, {$ReloadList, pc}^
   ]
 |
  [ {CONFIG} = 16
   [ "$CC" <> "" :LAND: "$CC" <> "AL"
        BN$CC   %FT0
   ]
   [ ReloadList = ""
    [ INTERWORK
        BX      lr
    |
        MOV     pc, lr
    ]
   |
        POP     {$ReloadList, pc}
   ]
0
  |
   [ ReloadList = ""
    [ INTERWORK
        BX$CC lr
    |
        MOV$CC pc, lr
    ]
   |
    [ INTERWORK
        LDM$CC.FD sp!, {$ReloadList, lr}
        BX$CC lr
    |
        LDM$CC.FD sp!, {$ReloadList, pc}
    ]
   ]
  ]
 ]
        MEND

        AREA    |C$$code|, CODE, READONLY

        END
