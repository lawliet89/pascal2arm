;;; stkcontig.s
;;; Copyright (C) 1991 Advanced RISC Machines Ltd.  All rights reserved.

;;; RCS $Revision: 1.13.12.1 $
;;; Checkin $Date: 1997/06/05 13:52:48 $
;;; Revising $Author: wdijkstr $

        GET     objmacs.s
        GET     h_stack.s
        GET     h_errors.s

        GBLL SWStackChecking
SWStackChecking SETL :DEF:sl

 [ make = "all" :LOR: make = "shared-library"

        Module  StackContig

fp  RN 11

        DataArea

StaticData

  [ SWStackChecking
initSL  Variable
  ]
initSP  Variable

 ]
        CodeArea

 [ make = "all" :LOR: make = "shared-library"

 [ SWStackChecking
        EXPORT  |x$stack_overflow|
        EXPORT  |x$stack_overflow_1|
        EXPORT  |__rt_stkovf_split_small|
        EXPORT  |__rt_stkovf_split_big|
        EXPORT  |__16__rt_stkovf_split_small|
        EXPORT  |__16__rt_stkovf_split_big|

        IMPORT  |__rt_registerDump|
        IMPORT  |__rt_trap|
        IMPORT  |__rt_trap_32|
        IMPORT  |__rt_heapDesc|
 ]

        Function _Stack_Init
        EXPORT _Stack_Init_32
_Stack_Init_32
        ; r0 = address of a heap descriptor
        ; r1 = address of heap base
        ; r2 = required size of initial stack
        ; r3 = heap limit
        ; fp is not yet set up so do it now.
        MOV     fp, #0  ; not necessary if /nofp

        ; initialise the heap description (nothing allocated yet).
        MOV     r2, r1
        STMIA   r0, {r1, r2, r3}

        ; remember initial stack state
        LDR     ip, addr_StaticData
 [ SWStackChecking
        STMIA   ip, {sl, sp}
 |
        STR     sp, [ip]
 ]
        Return  , "", LinkNotStacked

        Function _Stack_Validate
        EXPORT _Stack_Validate_32
_Stack_Validate_32
        ; r0 is a pointer to the heap description (base / hwm / limit)
        ; r1 is a pointer to a register save block.
        ; checks that the saved values for sp, fp and sl appear valid.
        ; if not, returns with r0 = 0:
        ; otherwise, r0 is unchanged
        ;            r12 is the sp value
        ;            r10 is the sl value
        ;            v1 is the fp value.
        LDR     r2, addr_StaticData

        LDR     r2, [r2, #O_initSP]
        LDR     v1, [r1, #fp*4]
        LDR     r12, [r1, #sp*4]
        CMP     r12, r2         ; want sp < workspace limit and ...
 [ SWStackChecking
        LDR     sl, [r0, #8]
        CMPLO   sl, r12         ; > heap hwm
                                ; we could also check sl = heap limit+sloffset
        ADD     sl, sl, #SC_SLOffset
 ]
        MOVHS   r0, #0          ; invalid
        Return  , "", LinkNotStacked

        Function _Stack_GetSafe
        EXPORT _Stack_GetSafe_32
_Stack_GetSafe_32
        LDR     ip, addr_StaticData

 [ SWStackChecking
        LDMIA   ip, {sl, sp}
 |
        LDR     sp, [ip]
 ]
        MOV     fp, #0
        Return  , "", LinkNotStacked

 [ SWStackChecking
 ;*-------------------------------------------------------------------*
 ;* Stack overflow handling                                           *
 ;*-------------------------------------------------------------------*

 [ THUMB
        CODE16
 ]
|x$stack_overflow|
|__16__rt_stkovf_split_small|
 [ THUMB
        BX      pc
        CODE32
 ]
|__rt_stkovf_split_small|
;
; Enter here when a C function with frame size <= 256 bytes underflows
; the stack low-water mark + StackSlop (sl). The stack space required has
; already been claimed by decrementing sp, so we set the proposed sp (ip)
; to the actual sp and fall into the big-frame case.
        MOV     ip, sp
 [ THUMB
        B       |__rt_stkovf_split_big|
 ]

 [ THUMB
        CODE16
 ]
|x$stack_overflow_1|
|__16__rt_stkovf_split_big|
 [ THUMB
        BX      pc
        CODE32
 ]
|__rt_stkovf_split_big|
;
; Enter here when a C function with frame size > 256 bytes would underflow
; the stack low-water mark + StackSlop (sl). No stack space has been claimed
; but the proposed new stack pointer is in ip.

        SUB     ip, sp, ip                      ; frame size required...
        CMP     ip, #MinStackIncrement          ; rounded up to at least
        MOVLT   ip, #MinStackIncrement          ; the default increment

        ADD     ip, ip, #SC_SLOffset            ; add in low-water offset
        SUBS    sl, sl, ip

 [ make = "shared-library"
        Function __rt_stkovf_split_internal
        MOV     ip, sb
        MOV     sb, ip
 ]
        LDR     ip, addr___rt_heapDesc

        LDR     ip, [ip, #4]                    ; check not below heap hwm
        CMPCS   sl, ip
        BCC     overflow
        LDR     ip, addr___rt_heapDesc

        STR     sl, [ip, #8]                    ; adjust heap limit
        ADD     sl, sl, #SC_SLOffset            ; restore safety margin
        Return  , "", LinkNotStacked

overflow
        LDR     ip, addr___rt_registerDump

 [ LDM_MAX >= 15
        STMIA   ip!, {r0-r14}
 |
        STMIA   ip!, {r0,r1,r2}
        STMIA   ip!, {r3,r4,r5}
        STMIA   ip!, {r6,r7,r8}
        STMIA   ip!, {r9,r10,r11}
        STMIA   ip!, {r12,sp,r14}
 ]
        ADR     r0, E_StackOverflow
        SUB     r1, ip,#pc*4
        STR     r14, [ip]
        B       |__rt_trap_32|

        ErrorBlock StackOverflow, "Stack overflow"
 ]

 [ make = "shared-library-stub"

        IMPORT  |__rt_stkovf_split_internal|

|x$stack_overflow|
|__rt_stkovf_split_small|
;
; Enter here when a C function with frame size <= 256 bytes underflows
; the stack low-water mark + StackSlop (sl). The stack space required has
; already been claimed by decrementing sp, so we set the proposed sp (ip)
; to the actual sp and fall into the big-frame case.
        MOV     ip, sp

|x$stack_overflow_1|
|__rt_stkovf_split_big|
;
; Enter here when a C function with frame size > 256 bytes would underflow
; the stack low-water mark + StackSlop (sl). No stack space has been claimed
; but the proposed new stack pointer is in ip.

        SUB     ip, sp, ip                      ; frame size required...
        CMP     ip, #MinStackIncrement          ; rounded up to at least
        MOVLT   ip, #MinStackIncrement          ; the default increment

        SUB     sl, sl, ip
        B       |__rt_stkovf_split_internal|
 ]
 ] ; SWStackChecking

 [ make = "all" :LOR: make = "shared-library"

        AdconTable

addr_StaticData
        &       StaticData

 [ SWStackChecking
        IMPORT  |__rt_heapDesc|
addr___rt_heapDesc
        &       |__rt_heapDesc|

        IMPORT  |__rt_registerDump|
addr___rt_registerDump
        &       |__rt_registerDump|
 ]

 ]
        END
