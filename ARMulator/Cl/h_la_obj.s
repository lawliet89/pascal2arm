;;; h_la_obj.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.5 $
;;; Checkin $Date: 1995/02/02 15:37:46 $
;;; Revising $Author: enevill $

        MACRO
        DataArea
        AREA    |C$$data|, DATA
        MEND

 [ make <> "shared-library"

        MACRO
        CodeArea
    [ INTERWORK
        AREA    |C$$code|, CODE, READONLY, INTERWORK
    |
        AREA    |C$$code|, CODE, READONLY
    ]
        MEND

        MACRO
        LoadStaticAddress $Addr, $Reg, $Ignore
        LDR     $Reg, =$Addr
        MEND

        MACRO
        LoadStaticBase $Reg, $Ignore
        LoadStaticAddress StaticData, $Reg, $Ignore
        MEND

        MACRO
        AdconTable
        MEND

        MACRO
        Function $name, $type
        LCLS    Temps
Temps   SETS    VBar:CC:"$name":CC:VBar
        EXPORT  $Temps
  [ THUMB :LAND: {CONFIG} <> 16
        CODE16
$Temps
        BX      pc
        CODE32
; If we are *NOT* building an interworking library we must set bit 0 of lr
; as the caller will not have done it for us.
    [ :LNOT:INTERWORK
        ORR     lr, lr, #1
    ]
  |
$Temps
  ]
        MEND

 |

        MACRO
        CodeArea
        AREA    |C$$code|, CODE, READONLY, REENTRANT, PIC
        MEND

        MACRO
        AdconTable
        AREA    |sb$$adcons|, DATA, READONLY, BASED sb
        MEND

        MACRO
        Function $name, $type
        LCLS    Temps
Temps   SETS    VBar:CC:"$name":CC:VBar
 [ "$type" = ""
        EXPORT  $Temps
 |
        EXPORT  $Temps[$type]
 ]
$Temps
        MEND

 ]
        END
