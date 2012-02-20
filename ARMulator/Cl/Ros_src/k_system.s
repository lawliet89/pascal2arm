;;; riscos/k_system.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     h_brazil.s
        GET     objmacs.s

        Module  k_system

        DataArea

StaticData
languageEnvSave Variable        4       ; (sb, fp, sp, sl over OSCLI)

        CodeArea

        IMPORT  |__riscos_InstallHandlers|
        IMPORT  |__riscos_InstallCallersHandlers|
        IMPORT  |__rt_fpavailable|
        IMPORT  |__rt_imagebase|
        IMPORT  |_Heap_GetBounds|

        IMPORT  |__riscos_heap_extended|
        IMPORT  |__riscos_heap_readarea|
        IMPORT  |__riscos_heap_resetarea|

        IMPORT  |__rt_errorBuffer|

        Function __riscos_system
; Execute the string a1 as a command;  if a2 is zero, as a subprogram,
; otherwise a replacement.
; Not available to client code of shared library: entered with sb set up
        FunctionEntry , "v1,v2,v3,v4,v5"
        MOV     v1, a1                  ; save arguments over calls below
        MOV     v5, a2
        BL      |__rt_fpavailable|
        CMPS    a1, #0
        STFNEE  f7, [sp, #-12]!
        STFNEE  f6, [sp, #-12]!
        STFNEE  f5, [sp, #-12]!
        STFNEE  f4, [sp, #-12]!
        RFSNE   v2

        BL      |_Heap_GetBounds|
        MOV     v4, r0

        BL      |__rt_imagebase|
        MOV     v3, r0

        BL      |__riscos_heap_extended|
        CMP     a1, #0                  ; has heap been extended?

        MOV     a2, v5                  ; recover subr/chain
        LDMIB   v4, {v4, v5}            ; heap top, heap limit
        STMFD   sp!, {v1, v2}           ; stack command string & fppsr to restore

        ; if the heap has been extended, copying the image is futile at best
        ; (and maybe harmful if it has a hole)
        MOVNE   v4, v5                  ; so pretend top = limit.

; Calculate len of image and size of gap.  We will not bother copying at all if gap
; is too small (but can't fault, because the command may not be an application)
        SUB     r0, v4, v3              ; Len = heapTop - imageBase
        ADD     r0, r0, #15
        BIC     v2, r0, #15             ; rounded Len, multiple of 16
        SUB     r14, v5, v3             ; heapLimit - imageBase
        SUB     r14, r14, v2            ; Gap = (heapLimit -imageBase) - Len
        ; if gap is too small, don't bother with copy.  1024 is an arbitrary
        ; small number, but what this is mainly aiming to avoid is the
        ; (otherwise possible) case of v6 < 0.
        CMP     r14, #1024
        MOVLT   r14, #0
        STMFD   sp!, {a2, v2-v5, r14, r15}   ; save them away
                                        ;subr/chain, len, base, top, limit, gap
                                        ; hole for memoryLimit

        BL      |__riscos_InstallCallersHandlers|  ; sets r0 to current memoryLimit
        STR     r0, [sp, #24]
        BL      |__riscos_heap_readarea|
        ADD     r14, sp, #16
        LDMIA   r14, {v5, r14}          ; recover limit, gap

        STMFD   sp!, {r1}               ; remember slot size

; The following loop copies the image up memory. It avoids overwriting
; itself by jumping to its copied copy as soon as it has copied itself,
; unless, of course, it's running in the shared C library...
; The image is copied in DECREASING address order.
CopyUp  CMP     r14, #0
        BEQ     CopyUpDone
        LDR     v2, [sp, #8]            ; image base
        LDR     v3, [sp, #12]           ; Len
        ADD     r1, v3, v2              ; imageBase + Len = initial src
        BIC     v4, pc, #&FC000003      ; where to copy down to before jumping
        CMP     v4, v5                  ; copy code > limit?
        ADDGT   v4, v3, #16             ; yes => in shared lib so fake v4
        MOVGT   v2, #0                  ; and don't jump to non-copied code
        MOVLE   v2, r14                 ; else jump to copied code
01      LDMDB   r1!, {r0,r2-r4}
        STMDB   v5!, {r0,r2-r4}
        CMP     r1, v4                  ; r1 < %B01 ?
        BGT     %B01                    ; no, so keep going...
        ADD     r0, pc, v2              ; ... go to moved image
        MOV     pc, r0                  ; and continue copying up...
01      LDMDB   r1!, {r0,r2-r4}
        STMDB   v5!, {r0,r2-r4}
        CMP     r1, v3                  ; src > imageBase ?
        BGT     %B01                    ; yes, so continue

CopyUpDone
        ; All registers must be preserved whose values are wanted afterwards.
        ; v1 to v6 are already on the stack.
 [ make = "shared-library"
        ADD     sb, sb, r14
 ]
        ADD     sp, sp, r14
        LDR     v5, addr_StaticData
        ADD     v5, v5, r14             ; our static data (now moved)
        STMIA   v5, {r9, fp, sp, sl}    ; save ptr to moved stack

        LDR     r0, [sp, #4]            ; chain/subr
        CMP     r0, #0

        MOVEQ   r0, #Env_MemoryLimit
        LDREQ   r1, [sp, #12]           ; original image base
        ADDEQ   r1, r1, r14
        SWIEQ   ChangeEnv

        MOVEQ   r0, #Env_ErrorHandler
        ADREQ   r1, s_ErrHandler
        MOVEQ   r2, v5
        LDR     r3, addr___rt_errorBuffer
        ADDEQ   r3, r3, r14             ; relocate error buffer alone
        SWIEQ   ChangeEnv

        MOVEQ   r0, #Env_ExitHandler
        ADREQ   r1, s_ExitHandler
        MOVEQ   r2, v5
        SWIEQ   ChangeEnv

        MOVEQ   r0, #Env_UpCallHandler  ; We don't really want one of these, ...
        ADREQ   r1, s_UpCallHandler     ; but RISCOS rules say we must have it
        MOVEQ   r2, v5
        SWIEQ   ChangeEnv

        LDR     r0, [sp, #32]           ; the CLI string to execute
        ADD     r0, r0, r14             ; ... suitably relocated...

        SWI     CLI:AND::NOT:X          ; force non-X variant
        B       s_Exit

s_UpCallHandler
        MOV     pc, r14

s_ErrHandler
        MOV     v5, r0
        MOV     r0, #-2
        B       s_Exit2

s_ExitHandler
        MOV     v5, r12
s_Exit
        MOV     r0, #0
s_Exit2
        LDMIA   v5, {r9, fp, sp, sl}
        LDMFD   sp!, {a2, a3, v1-v5}    ; slotsize,
                                        ;subr/chain, Len, Base, Top, Limit, Gap
        STR     r0, [sp, #4]            ; ... over prev saved r0...
        CMP     a3, #0
        SWINE   Exit
        MOVS    a1, a2
; ???? too early ????
        BLNE    |__riscos_heap_resetarea|   ; set slot size back to value before CLI

; The following loop copies the image down memory. It avoids overwriting
; itself by jumping to its copied copy as soon as it has copied itself,
; unless of course, this code is running in the shared C library...
; The image is copied in ASCENDING address order.
        CMP     v5, #0
        BEQ     CopyDnDone
CopyDn
        SUB     r0, v4, v1              ; limit - L = init src
        BIC     v3, pc, #&FC000003
        ADD     v3, v3, #%F02-.-4       ; where to copy to before jumping
        CMP     v3, v4                  ; copy code > limit?
        SUBGT   v3, v4, #16             ; yes => in shared lib so fake v3
        MOVGT   v1, #0                  ; and don't jump to not copied code...
        MOVLE   v1, v5                  ; else jump...
01      LDMIA   r0!, {r1-r3,ip}
        STMIA   v2!, {r1-r3,ip}
        CMP     r0, v3                  ; copied the copy code?
        BLT     %B01                    ; no, so continue...
        SUB     ip, pc, v1              ; yes => copied this far ...
        MOV     pc, ip                  ; ... so branch to copied copy loop
01      LDMIA   r0!, {r1-r3,ip}
        STMIA   v2!, {r1-r3,ip}
        CMP     r0, v4                  ; finished copying?
        BLT     %B01                    ; no, so continue...
02
CopyDnDone
        SUB     sp, sp, v5              ; and relocate sp...
 [ make = "shared-library"
        SUB     sb, sb, v5
 ]
        LDMFD   sp!, {r0}               ; old memoryLimit
        BL      |__riscos_InstallHandlers|

        BL      |__rt_fpavailable|
        CMP     a1, #0
        LDMFD   sp!, {a1, v5}
        WFSNE   v5
        LDFNEE  f4, [sp], #12
        LDFNEE  f5, [sp], #12
        LDFNEE  f6, [sp], #12
        LDFNEE  f7, [sp], #12
        Return  , "v1,v2,v3,v4,v5"

        AdconTable

        IMPORT  |__rt_errorBuffer|
addr___rt_errorBuffer
        &       |__rt_errorBuffer|

addr_StaticData
        &       StaticData

        END
