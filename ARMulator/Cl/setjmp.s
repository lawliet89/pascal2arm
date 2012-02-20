;;; setjmp.s
;;; Copyright (C) 1991 Advanced RISC Machines Ltd.  All rights reserved

;;; RCS $Revision: 1.15 $
;;; Checkin $Date: 1996/11/19 19:03:12 $
;;; Revising $Author: enevill $

        GET     objmacs.s
        GET     h_stack.s

        CodeArea

        IMPORT  |__rt_fpavailable|
        IMPORT  |__rt_fpavailable_32|
        IMPORT  |__rt_exittraphandler|
        IMPORT  |__rt_exittraphandler_32|

        IMPORT  |_Stack_DeallocateChunks|, WEAK

        ^ 0
sj_v1   #       4
sj_v2   #       4
sj_v3   #       4
sj_v4   #       4
sj_v5   #       4
sj_v6   #       4
sj_sl   #       4
sj_fp   #       4
sj_sp   #       4
sj_pc   #       4
sj_f4   #       3*4
sj_f5   #       3*4
sj_f6   #       3*4
sj_f7   #       3*4

        Function setjmp

; save everything that might count as a register variable value.
  [ make = "shared-library"
        MOV     ip, r9          ; intra-link-unit entry
        PushA   "v1,v2,v3,v4,v5,r9,r10,r11,sp,lr",a1
        MOV     r9, ip
  |
        PushA   "v1,v2,v3,v4,v5,v6,r10,r11,sp,lr",a1
  ]
        MOV     v5, a1
 [ :LNOT::DEF:EMBEDDED_CLIB
        BL      |__rt_fpavailable_32|
        CMP     a1, #0
        BEQ     setjmp_return
  [ FPE2
        STFE    f4, [v5, #sj_f4-sj_f4]
        STFE    f5, [v5, #sj_f5-sj_f4]
        STFE    f6, [v5, #sj_f6-sj_f4]
        STFE    f7, [v5, #sj_f7-sj_f4]
  |
        SFM     f4, 4, [v5]
  ]
 ]
        MOV     a1, #0
setjmp_return
 [ THUMB
        PopFrame "v5,r9,r10,r11,sp,lr",,,v5
        Return  , "", LinkNotStacked
 |
  [ {CONFIG} = 26
        PopFrame "v5,r9,r10,r11,sp,pc",^,,v5
  |
        PopFrame "v5,r9,r10,r11,sp,pc",,,v5
  ]
 ]

        Function longjmp

  [ make = "shared-library"
        MOV     ip, r9          ; intra-link-unit entry
        MOV     r9, ip          ; no need to save caller's sb (no return)
  ]
        ADD     v1, a1, #sj_f4
        MOVS    v5, a2
        MOVEQ   v5, #1          ; result of setjmp == 1 on longjmp(env, 0)

    [ :LNOT::DEF:EMBEDDED_CLIB
        LDR     ip, adr__inSignalHandler
        LDRB    a1, [ip, #0]
        CMP     a1, #0
        MOVNE   a1, #0
        STRNEB  a1, [ip, #0]
        BLNE    |__rt_exittraphandler_32|

        BL      |__rt_fpavailable_32|
        CMP     a1, #0
        BEQ     longjmp_return
  [ FPE2
        LDFE    f7, [v1, #sj_f7-sj_f4]
        LDFE    f6, [v1, #sj_f6-sj_f4]
        LDFE    f5, [v1, #sj_f5-sj_f4]
        LDFE    f4, [v1, #sj_f4-sj_f4]
  |
        LFM     f4, 4, [v1]
  ]
longjmp_return
        PopFrame "r10,r11,sp,r14",,,v1
        ; stack now cut back to that of caller of setjmp
        ; (r14 value unwanted here)
        LDR     r0, addr__Stack_DeallocateChunks
        CMP     r0, #0
        BLNE    |_Stack_DeallocateChunks|
    ]
        MOV     a1, v5
 [ THUMB
        PopFrame "v1,v2,v3,v4,v5,r9,r10,r11,sp,lr",,,v1
        Return  , "", LinkNotStacked
 |
  [ {CONFIG} = 26
        PopFrame "v1,v2,v3,v4,v5,r9,r10,r11,sp,pc",^,,v1
  |
        PopFrame "v1,v2,v3,v4,v5,r9,r10,r11,sp,pc",,,v1
  ]
 ]

 [ :LNOT::DEF:EMBEDDED_CLIB
        AdconTable

addr__Stack_DeallocateChunks
        & _Stack_DeallocateChunks

        IMPORT  |_inSignalHandler|
adr__inSignalHandler
        &       |_inSignalHandler|
 ]

        END
