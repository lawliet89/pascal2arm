;;; stkchunk.s: stack implementation as linked list of chunks
;;;             allocated from heap
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.12 $
;;; Checkin $Date: 1995/05/04 11:04:03 $
;;; Revising $Author: enevill $

        GET     objmacs.s
        GET     h_stack.s
        GET     h_hw.s
        GET     h_errors.s

 [ :LNOT::DEF:sl
sl      RN      10
 ]

 [ make = "stkchunks" :LOR make = "all" :LOR: make = "shared-library"

        Module  StackChunks

        DataArea

StaticData

rootChunk       Variable
extendChunk     Variable

 ]
        CodeArea

 [ make = "stkchunks" :LOR make = "all" :LOR: make = "shared-library"

        EXPORT  |__StackOverflow_Fault|
        EXPORT  |__StackOverflow_Exit|
        EXPORT  |__StackOverflow_PseudoEntry|

        IMPORT  |__rt_malloc|
        IMPORT  |__rt_free|
        IMPORT  |__rt_trap|

        Function _Stack_Init
        ; r0 = address of a heap descriptor
        ; r1 = address of heap base
        ; r2 = required size of initial stack
        ; r3 = end of application workspace (= heap limit).
        ; stack description registers (sp, fp, sl) are not yet set up.
        ; so do them now.
        ADD     sp, r1, r2
        ADD     sl, r1, #SC_SLOffset
        MOV     fp, #0

        ; initialise the root chunk
        STR     r2, [r1, #SC_size]
        STR     fp, [r1, #SC_next]
        STR     fp, [r1, #SC_prev]
        STR     fp, [r1, #SC_deallocate]
        LDR     r2, =IsAStackChunk
        STR     r2, [r1, #SC_mark]

        ; set up a small stack chunk for use in performing stack extension.
        ; We needn't bother with most of the fields in the description of
        ; this chunk - they won't ever be used.  We must set mark (to be
        ; not IsAStackChunk) and SL_xxx_Offset. Also prev.

        ASSERT  O_rootChunk = 0
        LDR     ip, addr_StaticData
        STMIA   ip, {r1, sp}            ; rootChunk, extendChunk respectively
        ADD     r2, sp, #ExtendStackSize

        ; initialise the heap description (base / hwm / limit)
        STMIA   r0, {r1, r2, r3}

        ADD     r3, sl, #SL_Lib_Offset
        LDMIA   r3, {r1, r2}
        ADD     r3, sp, #SC_SLOffset+SL_Lib_Offset
        STMIA   r3, {r1, r2}
        STR     sp, [sp, #SC_mark]      ; anything other than IsAStackChunk
        STR     fp, [sp, #SC_prev]      ; 0 to mark end of chain
        MOV     r1, #ExtendStackSize
        STR     r1, [sp, #SC_size]
        Return  , "", LinkNotStacked

        Function _Stack_Validate
        ; r0 is a pointer to the heap description (base / hwm / limit)
        ; r1 is a pointer to a register save block.
        ; checks that the saved values for sp, fp and sl appear valid.
        ; if not, returns with r0 = 0:
        ; otherwise, r0 is unchanged
        ;            r12 is the sp value
        ;            r10 is the sl value
        ;            v1 is the fp value.
        LDR     r10, [r1, #sl*4]
        LDR     r12, [r1, #sp*4]
        LDR     v1, [r1, #fp*4]
        LDMIA   r0, {r1, r2}
        CMP     r12, r1
        CMPGT   r2, r12         ; sp within heap and ...
        CMPGT   r10, r1
        CMPGT   r2, r10         ; sl within heap and ...
        ADDGT   r1, r12, #256
        CMPGT   r1, r10         ; sp > sl and ...
        BLE     StackInvalid
        LDR     r1, =IsAStackChunk
        LDR     r2, [r10, #SC_mark-SC_SLOffset]
        EOR     r2, r2, r1
        BICS    r2, r2, #&80000000 ; a chunk marked 'handling extension' will do
        BNE     StackInvalid
        Return  , "", LinkNotStacked

StackInvalid
        MOV     r0, #0
        Return  , "", LinkNotStacked


        Function _Stack_GetSafe
        LDR     sl, addr_StaticData
        LDR     sl, [sl, #O_rootChunk]
        LDR     sp, [sl, #SC_size]
        ADD     sp, sl, sp
        ADD     sl, sl, #SC_SLOffset
        MOV     fp, #0
        Return  , "", LinkNotStacked


 ;*-------------------------------------------------------------------*
 ;* Stack chunk handling                                              *
 ;*-------------------------------------------------------------------*

        Function __rt_current_stack_chunk, leaf
        SUB     a1, sl, #SC_SLOffset
        Return  , "", LinkNotStacked

 ;*-------------------------------------------------------------------*
 ;* Stack overflow handling                                           *
 ;*-------------------------------------------------------------------*

        Function x$stack_overflow
        Function __rt_stkovf_split_small
; In the shared library case, these are only for use from within the library.
; (so sb is alread correctly set up)
; There are separate veneers for other-link-unit use, which end up by
; branching to __rt_stkovf_split_internal.

; Run out of stack.
; Before doing anything else, we need to acquire some work registers
; as only ip is free.
; We can save things on the stack a distance below fp which allows the
; largest possible list of saved work registers (r0-r3, r4-r9 inclusive,
; plus fp, sp, lr, entry pc, = 14 regs in total) plus a minimal stack
; frame for return from StkOvfExit (a further 5 words, giving 19 in total)
; plus 4 extended floating point registers (a further 3*4 words)
        MOV     ip, sp

        Function x$stack_overflow_1
        Function __rt_stkovf_split_big

        SUB     ip, sp, ip              ; size required
        SUB     sp, fp, #30*4
        Push    "a1,a2,v1,v2,v3,v4,v5,r9,lr"
        ADD     v4, ip, #SC_SLOffset    ; required size + safety margin
 [ make = "shared-library"
        Function __rt_stkovf_split_internal
        MOV     ip, sb
        MOV     sb, ip
 ]
        SUBS    v1, fp, #31*4-4         ; save area ptr, clear V flag
        BL      |__Stack_GetChunk|
        CMP     v2, #0
        BEQ     |__StackOverflow_Fault|
; Get here with v2 pointing to a big enough chunk of size v3
; (Not yet marked as a stack chunk)
        ADD     sl, v2, #SC_SLOffset    ; make the new sl
        ADD     sp, v2, v3              ; and initial sp
        LDR     a1, =IsAStackChunk
        STR     a1, [sl, #SC_mark-SC_SLOffset]
; v1 is save area in old frame... will be temp sp in old frame
        ADD     a1, v1, #5*4-4          ; temp fp in old frame
 [ LDM_MAX >= 4
        LDMDA   fp, {v2,v3,v4,v5}       ; old fp, sp,lr, pc
 |
        LDMDA   fp, {v3,v4,v5}          ; old sp,lr, pc
        LDR     v2, [fp, #-12]          ; old fp
 ]
        ADR     v5, |__StackOverflow_PseudoEntry|+12
        MOV     a2, r9                  ; (shared library) save our sb for exit code
 [ LDM_MAX >= 5
        STMDA   a1, {a2,v2,v3,v4,v5}    ; new return frame in old chunk...
 |
        STMDA   a1!, {v3,v4,v5}         ;) new return frame in old chunk...
        STMDA   a1!, {a2,v2}            ;)
        ADD     a1, a1, #5*4
 ]
        ADR     lr, |__StackOverflow_Exit|
        MOV     a2, sp                  ; saved sp in old frame = NEW sp
                                        ; (otherwise exit call is fatal)
        ORR     a1, a1, #ChunkChange
        STMDB   fp, {a1, a2, lr}        ; pervert old frame to return here...
 [ {CONFIG} = 26
        PopFrame "a1,a2,v1,v2,v3,v4,v5,r9,pc",^,,v1
 |
        PopFrame "a1,a2,v1,v2,v3,v4,v5,r9,pc",,,v1
 ]

|__StackOverflow_PseudoEntry|
        Push    "fp,ip,lr,pc"           ; A register save mask

|__StackOverflow_Exit|
; We return here when returning from the procedure which caused the
; stack to be extended. FP is in the old chunk SP and SL are still
; in the new one.
        ; We need to move sp and sl back into the old chunk.  Since this happens
        ; in two operations, we need precautions against events while we're
        ; doing it.
        ADD     sl, sl, #4              ; (an invalid stack-chunk handle)
        BIC     fp, fp, #ChunkChange
        LDR     ip, [fp, #-4*4]         ; our sb if shared library
        SUB     sp, fp, #3*4            ; get a sensible sp in the old chunk
        Push    "a1,v1,v2,v3,v4,r9"     ; Save some work regs
        MOV     r9, ip
; Now see if the new chunk has a next chunk and deallocate it if it has.
        SUB     v1, sl, #4
        LDR     sl, [v1, #SC_prev-SC_SLOffset]
        LDR     a1, [sl, #SC_mark]      ; make not a stack chunk (before making
        EOR     a1, a1, #&40000000      ; sl a proper stackchunk handle).
        STR     a1, [sl, #SC_mark]
        ADD     sl, sl, #SC_SLOffset
        SUB     sp, sp, #4
        BL      |_Stack_DeallocateChunks|
; Clear the chunk change bit, and return to caller by reloading the saved work
; regs and the frame regs that were adjusted at the time the stack was extended
        LDR     a1, =IsAStackChunk
        STR     a1, [sl, #SC_mark-SC_SLOffset]
        BIC     fp, fp, #ChunkChange
 [ {CONFIG} = 26
        PopFrame "a1,v1,v2,v3,v4,r9,fp,sp,pc",^
 |
        PopFrame "a1,v1,v2,v3,v4,r9,fp,sp,pc",
 ]

        Function _Stack_DeallocateChunks
; Exit from overflow, or longjmp.  Stack is back in old chunk: deallocate all
; extension chunks except the first (if they are deallocatable).
        FunctionEntry , "v2,v3,v4"
        LDR     v2, [sl, #SC_next-SC_SLOffset]  ; first extension chunk
        ADD     v4, sl, #SC_next-SC_SLOffset    ; base of extension chain
        CMP     v2, #0
        Return  , "v2,v3,v4", , EQ
deallocate_chunks
        LDR     v3, [v2, #SC_next]              ; chunk after next
        LDR     ip, [v2, #SC_deallocate]
        MOV     a1, v2
        CMPS    ip, #0                          ;) deallocate it if it can be deallocated
        ADDEQ   v4, v2, #SC_next                ; else retain it
        BEQ     %F01
        MOV     lr, pc                          ;) deallocate, and update the
        MOV     pc, ip                          ;) chain.
        STR     v3, [v4]                        ;)

01      MOVS    v2, v3
        BNE     deallocate_chunks
        Return  , "v2,v3,v4"


        Function __Stack_GetChunk
; Naughty procedure with non-standard argument conventions.
; On entry, V1 = save area ptr in case of error return; V4 = needed size;
; On exit, V1, and V4 are preserved, V2 points to the newly inserted chunk
; and V3 is the chunk's size. In case of error, V2 is zero.
; PSR is not preserved.
        TST     r14, #PSRSVCMode        ; in SWI mode, stack overflow is fatal
        ORRNES  pc, r14, #PSRVBit       ; (could have been detected earlier,
                                        ; but deferral to now is simpler).
; Check that the current chunk really is a stack chunk...
        Push    "a3,a4,ip,lr"           ; save args not saved before
        LDR     v2, =IsAStackChunk      ; magic constant...
        LDR     v3, [sl, #SC_mark-SC_SLOffset]
        CMP     v2, v3                  ; matches magic in chunk?
        BNE     StkOvfError             ; No! - die horribly
        EOR     v3, v3, #&80000000      ; make not a stack chunk, so recursive
        STR     v3, [sl, #SC_mark-SC_SLOffset] ; extension faults
; We have a chunk, see if there's a usable next chunk...
        SUB     v2, sl, #SC_SLOffset
02      LDR     v2, [v2, #SC_next]
        CMP     v2, #0
        BEQ     StkOvfGetNewChunk       ; No! - so make one
        LDR     v3, [v2, #SC_size]
        CMP     v4, v3                  ; is it big enough?
        BGT     %B02                    ; No! so try next chunk
; unlink the usable chunk from the chain...
        LDR     a1, [v2, #SC_prev]      ; previous chunk
        LDR     a2, [v2, #SC_next]      ; next chunk
        STR     a2, [a1, #SC_next]      ; prev->next = next
        CMPS    a2, #0                  ; next == NULL ?
        STRNE   a1, [a2, #SC_prev]      ; next->prev = prev
        B       StkOvfInsertChunk
StkOvfGetNewChunk Keep
        ; Now we swap to the special extension chunk (to give a reasonable
        ; stack size to malloc).
        LDR     v2, addr_StaticData
        LDR     a2, [v2, #O_extendChunk]
        LDR     a3, [a2, #SC_size]
        ADD     a3, a2, a3              ; new sp
        Push    "sl,fp,sp", a3          ; save old stack description
        MOV     sp, a3
        ADD     sl, a2, #SC_SLOffset
        MOV     fp, #0

        MOV     a1, #MinStackIncrement  ; new chunk is at least this big
        CMP     a1, v4                  ; but may be bigger if he wants a huge frame
        MOVLT   a1, v4
        LDR     v2, addr___rt_malloc
        LDR     ip, [v2]
        CMPS    ip, #0
        BEQ     %F01                    ; (restore stack chunk, then error)
        LDR     v2, addr___rt_free
        LDR     v2, [v2]
        Push    "a1,v2"           ; chunk size in bytes, dealloc proc
        MOV     lr, pc
        MOV     pc, ip
        MOVS    v2, a1
        Pop     "v3,ip"                 ; size in bytes, dealloc
01
        LDMFD   sp, {sl, fp, sp}        ; back to old chunk
        BEQ     StkOvfError
        STR     v3, [v2, #SC_size]
        STR     ip, [v2, #SC_deallocate]
        LDR     a1, [sl, #SL_Lib_Offset]
        STR     a1, [v2, #SL_Lib_Offset+SC_SLOffset]
        LDR     a1, [sl, #SL_Client_Offset]
        STR     a1, [v2, #SL_Client_Offset+SC_SLOffset]
; and re-link it in its proper place...
StkOvfInsertChunk Keep
        SUB     a1, sl, #SC_SLOffset    ; chunk needing extension...
        LDR     a2, =IsAStackChunk
        STR     a2, [a1, #SC_mark]      ; remark as stack chunk
        LDR     a2, [a1, #SC_next]      ; its next chunk
        STR     a2, [v2, #SC_next]      ; this->next = next
        STR     v2, [a1, #SC_next]      ; prev->next = this
        STR     a1, [v2, #SC_prev]      ; this->prev = prev
        CMPS    a2, #0
        STRNE   v2, [a2, #SC_prev]      ; next->prev = this
        STR     pc, [v2, #SC_mark]      ; Not a stack chunk (for safe non-atomic
                                        ; update of sp and sl).
        Pop     "a3,a4,ip,pc"           ; restore extra saved regs
StkOvfError
        MOV     v2, #0
        Pop     "a3,a4,ip,pc"

        Function __StackOverflow_Fault
        MOV     sp, v1
        PopFrame "a1,a2,v1,v2,v3,v4,v5,r9,lr",,,v1
        LDR     ip, addr___rt_registerDump
        STMIA   ip, {a1 - r14}
        ADR     r0, E_StackOverflow
        MOV     r1, ip
        STR     r14, [ip, #pc*4]
        B       |__rt_trap|

        ErrorBlock StackOverflow, "Stack overflow"

 ]
 [ make = "chunk_copy" :LOR: make = "all" :LOR: make = "shared-library"

   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__StackOverflow_Exit|
        IMPORT  |__StackOverflow_PseudoEntry|

        IMPORT  |__Stack_GetChunk|
        IMPORT  |__StackOverflow_Fault|
   ]

        Function __rt_stkovf_copy_small
; In the shared library case, these are only for use from within the library.
; (so sb is alread correctly set up)
; There are separate veneers for other-link-unit use, which end up by
; branching to __rt_stkovf_copy_internal.

; Run out of stack.
; Before doing anything else, we need to acquire some work registers
; (IP is free in the StkOvf case, but not in the StkOvfN case).
; We can save things on the stack a distance below FP which allows the
; largest possible list of saved work registers (R0-R3, R4-9 inclusive,
; plus FP, SP, LR, entry PC, = 14 regs in total) plus 4 extended floating
; point registers, a further 3*4 words
        MOV     ip, #0                  ; STKOVF not STKOVFN

        Function __rt_stkovf_copy_big

; the (probable) write below sp here is inevitable - there are no registers
; free.  The only way to make this safe against events is to pervert sl
; temporarily.
        ADD     sl, sl, #4
        STR     lr, [fp, #-26*4]        ; save LR
        SUB     lr, fp, #26*4           ; & use as temp SP
        Push    "a1,a2,v1,v2,v3,v4,v5,r9", lr; to save A1-A2, V1-V6
   [ make = "shared-library"
        MOV     a1, ip
        Function __rt_stkovf_copy_internal
        MOV     ip, sb
        MOV     sb, ip
        MOV     ip, a1
   ]
        SUB     v4, fp, sp              ; needed frame size
        ADD     v4, v4, #SC_SLOffset    ; + safety margin
        MOV     sp, lr                  ; got an SP now...
        SUB     sl, sl, #4              ; restore proper SL now stack is in a good state.
        ; We do not drop SL here : the code that gets called to acquire a new
        ; stack chunk had better not check for stack overflow (and also had
        ; better not use more than the minimum that may be available).
        SUBS    v1, fp, #26*4+4         ; save area ptr, clear V flag
        BL      |__Stack_GetChunk|
        CMP     v2, #0
        BEQ     |__StackOverflow_Fault| ; out of stack

; Get here with V2 pointing to a big enough chunk of size V3
        ADD     sl, v2, #SC_SLOffset    ; make the new SL
        ADD     sp, v2, v3              ; and initial SP
        LDR     a1, =IsAStackChunk
        STR     a1, [sl, #SC_mark-SC_SLOffset]
; Copy over 5th and higher arguments, which are expected on the stack...
        CMP     ip, #0
        BLE     DoneArgumentCopy
01      LDR     v5, [fp, ip, ASL #2]    ; copy args in high->low
        STR     v5, [sp, #-4]!          ; address order
        SUBS    ip, ip, #1
        BNE     %B01
DoneArgumentCopy
; Now create a call frame in the new stack chunk by copying
; over stuff saved in the frame in the old stack chunk to the
; new, perverting LR so that, on return, control comes back to
; this code and perverting SP and FP to give us a save area
; containing none of the V registers.
        ADD     v1, fp, #4              ; old chunk's frame pointer
        SUB     ip, v4, #SC_SLOffset    ; needed frame size, no margin
        PopA    "a1,a2,v2,v3,v4,v5,lr",v1 ; 1st 7 of possible 14 saved regs
        LDR     v5, =|__StackOverflow_Exit|; return address...
        MOV     v4, sp                  ; SP in NEW chunk
        ORR     v3, fp, #ChunkChange    ; new FP in old chunk
        SUB     fp, sp, #4              ; FP in new chunk
        Push    "a1,a2,v2,v3,v4,v5,lr"  ; 1st 7 copied frame regs
        PopA    "a1,a2,v2,v3,v4,v5,lr",v1 ; and the 2nd 7 regs
        Push    "a1,a2,v2,v3,v4,v5,lr"  ; copied to the new frame
; Now adjust the PC value saved in the old chunk to say "no registers"
        LDR     v2, =|__StackOverflow_PseudoEntry|+12
        STR     v2, [v1, #26*4]
        STR     r9, [v1, #26*4-4*4]
; Set the SP to be FP - requiredFrameSize and return by reloading regs
; from where they were saved in the old chunk on entry to STKOVF/N
        SUB     sp, fp, ip
   [ {CONFIG} = 26
        PopFrame "a1,a2,v1,v2,v3,v4,v5,r9,pc",^,,v1
   |
        PopFrame "a1,a2,v1,v2,v3,v4,v5,r9,pc",,,v1
   ]
 ]

 [ make = "shared-library-stub"

        IMPORT  |__rt_stkovf_copy_internal|
        IMPORT  |__rt_stkovf_split_internal|

        ; a direct copy of code above
        Function __rt_stkovf_copy_small
        MOV     ip, #0                  ; STKOVF not STKOVFN

        Function __rt_stkovf_copy_big

        ADD     sl, sl, #4
        STR     lr, [fp, #-26*4]        ; save LR
        SUB     lr, fp, #26*4           ; & use as temp SP
        Push    "a1,a2,v1,v2,v3,v4,v5,r9",lr ; to save a1-a2, v1-v5, sb
        MOV     a1, ip
        B       |__rt_stkovf_copy_internal|


        Function x$stack_overflow
        Function __rt_stkovf_split_small
        MOV     ip, sp

        Function x$stack_overflow_1
        Function __rt_stkovf_split_big

        SUB     ip, sp, ip              ; size required
        SUB     sp, fp, #30*4
        Push    "a1,a2,v1,v2,v3,v4,v5,r9,lr"
        ADD     v4, ip, #SC_SLOffset    ; required size + safety margin
        B       |__rt_stkovf_split_internal|
 ]

 [ make = "stkchunks" :LOR make = "all" :LOR: make = "shared-library"

        AdconTable

addr_StaticData
        &       StaticData

        IMPORT  |__rt_registerDump|
addr___rt_registerDump
        &       |__rt_registerDump|

        IMPORT  |__rt_malloc|
addr___rt_malloc
        &       |__rt_malloc|

        IMPORT  |__rt_free|
addr___rt_free
        &       |__rt_free|

 ]
       END
