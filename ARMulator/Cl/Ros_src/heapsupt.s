;;; riscos/heapsuppt.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     h_brazil.s
        GET     objmacs.s

        Module  heapsupport

        DataArea

StaticData

knownSlotSize           Variable
initSlotSize            Variable
heapExtender            Variable

underDesktop            VariableByte
dummy                   VariableByte    3

        CodeArea

; only __riscos_register_slotextend is available to client code.
; all others are entered with sb correctly set up.

        Function __osdep_heapsupport_init
        ; a1 points to a heap descriptor block:
        ;    base; hwm; limit
        FunctionEntry
        LDR     ip, addr_StaticData
        MOV     lr, a1
        MOV     r0, #0
        SWI     Wimp_ReadSysInfo
        MOVVS   r0, #0                  ; error - presumably Wimp not present
        CMP     r0, #0
        MOVNE   r0, #1
        STRB    r0, [ip, #O_underDesktop]

        MOVNE   r0, #Env_ApplicationSpace
        MOVNE   r1, #0
        SWINE   ChangeEnv
        LDREQ   r1, [lr, #8]
        STR     r1, [ip, #O_knownSlotSize]
        STR     r1, [ip, #O_initSlotSize]
        Return

        Function __osdep_heapsupport_finalise
        FunctionEntry , "v1,v2"
        LDR     ip, addr_StaticData
        LDRB    r1, [ip, #O_underDesktop]
        CMP     r1, #0
        LDRNE   r1, [ip, #O_knownSlotSize]
        LDRNE   r0, [ip, #O_initSlotSize]
        CMPNE   r1, r0
        BLNE    SetWimpSlot
        Return  , "v1,v2"

SetWimpSlot  Keep
        ; Set the wimp slot to r0 - ApplicationBase.
        ; Destroys r4, r5
        ; May need to set MemoryLimit temporarily to the slot size,
        ; in order that Wimp_SlotSize not refuse.
        ; (Note that preservation of r4 is required anyway because of
        ;  fault in Wimp_SlotSize which corrupts it).
        ; Returns the slot size set.
        MOV     r4, r0
        MOV     r0, #Env_MemoryLimit
        MOV     r1, #0
        SWI     ChangeEnv
        MOV     r5, r1
        MOV     r0, #Env_ApplicationSpace
        MOV     r1, #0
        SWI     ChangeEnv
        CMP     r1, r5
        MOVNE   r0, #Env_MemoryLimit
        SWINE   ChangeEnv
        SUB     r0, r4, #Application_Base
        MOV     r1, #-1
        SWI     Wimp_SlotSize
        MOVNE   r4, r0
        MOVNE   r0, #Env_MemoryLimit
        MOVNE   r1, r5
        SWINE   ChangeEnv
        MOVNE   r0, r4
        Return  , "", LinkNotStacked

        Function __riscos_register_slotextend
        FunctionEntry UsesSb
        LDR     ip, addr_StaticData
        MOVS    a2, a1
        LDR     a1, [ip, #O_heapExtender]
        STRNE   a2, [ip, #O_heapExtender]
        Return  UsesSb

        Function __osdep_heapsupport_extend
        ; r0 is amount wanted
        ; r1 points to a heap descriptor block:
        ; There's not going to be room for the amount required.  See if
        ; we can extend our workspace.
        FunctionEntry , "r4,r5,v5"
        LDR     ip, addr_StaticData
        LDRB    r3, [ip, #O_underDesktop]
        CMP     r3, #0
        BEQ     alloc_cant_extend       ; not under desktop


        LDR     r3, [ip, #O_heapExtender]
        CMP     r3, #0
        BEQ     alloc_no_extender
        MOV     r0, r0, ASL #2          ; ask for what we were asked for
                                        ; (since what we are given may well
                                        ;  not be contiguous with what we had
                                        ;  before).
        SUB     sp, sp, #4
        MOV     r1, sp                  ; place to store returned block
        ; Set to a silly value, guaranteed not to be equal to initSlotSize, to ensure
        ; reset on exit.
        STR     r3, [ip, #O_knownSlotSize]
        MOV     lr, pc
        MOV     pc, r3
        LDR     ip, addr_StaticData     ; restore our static base
        LDR     r1, [sp], #+4           ; base of area acquired
        Return  , "r4,r5,v5"

alloc_no_extender
        ; if current slotsize = heap limit, try to extend the heap by the
        ; amount required (or perhaps by just enough to allow allocation
        ; of the amount required)
        MOV     v5, r1                  ; save heap descriptor over ChangeEnv
        ADD     lr, r2, r0, ASL #2      ; heaptop if request could be granted
        MOV     r0, #Env_ApplicationSpace ; find current slotsize
        MOV     r1, #0
        SWI     ChangeEnv
        LDR     r0, [ip, #O_knownSlotSize]
        CMP     r1, r0
        BNE     alloc_cant_extend

        ; If the extension will be contiguous with the current heap top,
        ; then we need just enough to allow the requested allocation.
        LDMIB   v5, {r2, r3}
        CMP     r3, r0
        BEQ     alloc_extend_slot
        ; Otherwise, we must extend by the amount requested.  If there's still
        ; some space left in the previous area, give that back first.
        SUBS    r3, r3, r2
        BNE     alloc_cant_extend
        SUB     lr, lr, r3
alloc_extend_slot
        ; lr holds the slot size we want.  r1 is the current memory limit.
        ; Now if memory limit is not slot size, we must reset memory limit
        ; temporarily over the call to Wimp_SlotSize (or it will refuse).
        MOV     r0, lr
        BL      SetWimpSlot
        LDR     r1, [ip, #O_knownSlotSize]
        ADD     r2, r0, #Application_Base
        STR     r2, [ip, #O_knownSlotSize]
        SUB     r0, r2, r1              ; amount we acquired
        Return  , "r4,r5,v5"

alloc_cant_extend
        MOV     r0, #0
        Return  , "r4,r5,v5"

        Function __riscos_heap_extended
        FunctionEntry
        LDR     ip, addr_StaticData
        LDMIA   ip, {r0, r1}            ; known, init slot size
        SUB     r0, r0, r1
        Return

        Function __riscos_heap_readarea
        FunctionEntry
        LDR     ip, addr_StaticData
        LDRB    r0, [ip, #O_underDesktop]  ; if under desktop, find what the
        CMP     r0, #0                     ; Wimp slot size currently is, so we
        MOVNE   r0, #Env_ApplicationSpace  ; can reset it later on
        MOV     r1, #0
        SWINE   ChangeEnv
        MOV     r0, r1
        Return

        Function __riscos_heap_resetarea, leaf
        FunctionEntry , "v1,v2"
        BL      SetWimpSlot               ; set slot size back to value before CLI
        Return  , "v1,v2"

        AdconTable

addr_StaticData
        &       StaticData

        END
