        TTL     Angel assembler support routines                    > suppasm.s
        ; ---------------------------------------------------------------------
        ; This source files holds the general assembler routines
        ; needed by Angel.
        ;
        ; $Revision: 1.21.6.3 $
        ;   $Author: rivimey $
        ;     $Date: 1997/12/19 15:54:54 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ;
        ; ---------------------------------------------------------------------

        GET     listopts.s              ; standard listing control
        GET     lolevel.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support
        GET     target.s                ; target specific definitions

        ; ---------------------------------------------------------------------

        AREA    |C$$Code$$InterruptSupport|,CODE,READONLY
        KEEP

        [       ({CONFIG} = 26)
        ASSERT  (1 = 0) ; This code has been written for 32bit mode only
        ]


        IF CACHE_SUPPORTED <> 0
          EXPORT Cache_IBR
        ENDIF

        ; ---------------------------------------------------------------------
        ; The following is a word which gets set to either ADP_CPU_LE or
        ; ADP_CPU_BE at assemble time.  It can then be used by Angel to
        ; tell the host what the endianess of the system is.

        EXPORT  angel_Endianess

angel_Endianess
        IF :DEF: ICEMAN2
          DCD  ADP_CPU_LE | ADP_CPU_BE ;
          DCD  0
        ELSE
          IF {ENDIAN} = "big"
            DCD ADP_CPU_BE
            DCD ADP_CPU_BigEndian
          ELSE
            IF {ENDIAN} = "little"
              DCD ADP_CPU_LE
              DCD 0
            ELSE
              ! ERROR - could not detect endianess
            ENDIF
          ENDIF
        ENDIF
        
        ; This is the veneer for the late startup SWI
        EXPORT  Angel_LateStartup
Angel_LateStartup
        MOV     a2, a1          ; preserve type
        LDR     a1, =angel_SWIreason_LateStartup
        DCD     angel_SWI_ARM
        MOV     pc, lr


        IMPORT  angel_IntHandler        ; Table of Interrupt handlers
        IMPORT  Angel_MutexSharedTempRegBlocks
        IMPORT  angel_GhostCount
        IMPORT  HandlerSWI
        IMPORT  Angel_StackBase
        
  IF :DEF: ICEMAN_LEVEL_3 :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
        ; Do not include an Undef Handler for ICEMan
  ELSE
        IMPORT  HandlerUndef
  ENDIF
        ; This is the lowest level interrupt handler which is installed
        ; statically. It can be put on either the IRQ or FIQ vector as
        ; required for a particular board (or both if you really want !)
        EXPORT  angel_DeviceInterruptHandler
angel_DeviceInterruptHandler
        STMFD sp!, {r0}

        ; Disable FIQ's if necessary
        IF (FIQ_SAFETYLEVEL < FIQ_NeverUsesSerialiser_DoesNotReschedule)
          MRS   r0, CPSR
          ORR   r0, r0, #IRQDisable + FIQDisable
          MSR   CPSR, r0
        ENDIF

        ; Save the state of the callee into a regblock
        ; This means r0-r15, cpsr and if appropriate spsr
        ;
        ; Can do r0-r7, pc, cpsr for all modes
        LDR   r0, =Angel_MutexSharedTempRegBlocks
        ADD   r0, r0, #Angel_RegBlock_R0offset + (1*4)
        STMIA r0, {r1-r7}       ; r1-r7 are never banked
        SUB   r0, r0, #Angel_RegBlock_R0offset + (1*4)
        LDMFD sp!, {r1}
        STR   r1, [r0, #Angel_RegBlock_R0offset] ; original r0
        MRS   r1, SPSR
        STR   r1, [r0, #Angel_RegBlock_CPSRoffset] ; original cpsr
        SUB   r14, r14, #4      ; Adjust lr (works for ARM and Thumb)
        STR   r14, [r0, #Angel_RegBlock_R0offset + (15*4)] ; pc

        ; Accessing other modes depends on the mode
        AND   r1, r1, #ModeMask
        CMP   r1, #USRmode
        BEQ   InterruptedInUSRMode
        CMP   r1, #SYSmode
        BEQ   InterruptedInUSRMode

        ; Deal with non USR mode case
        MRS   r3, cpsr
        IF (FIQ_SAFETYLEVEL >= FIQ_NeverUsesSerialiser_DoesNotReschedule)
          ORR r1, r1, #IRQDisable
          ORR r3, r3, #IRQDisable
        ELSE
          ORR r1, r1, #IRQDisable + FIQDisable
          ORR r3, r3, #IRQDisable + FIQDisable
        ENDIF
        MSR   CPSR, r1

        ; Now we are in the appropriate mode
        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        STMIA r0, {r8-r14}      ; r8-r14 may be banked - now saved
        SUB   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        MRS   r2, SPSR
        STR   r2, [r0, #Angel_RegBlock_SPSRoffset]

        ; Get back into original interrupt mode (IRQ or FIQ)
        MSR   cpsr, r3

        IF HANDLE_INTERRUPTS_ON_FIQ <> 0
          ; Nasty case code - check to see if we have interrupted the first
          ; few instructions of the SWI handler or UND handler!!!
          ; which includes the vectors themselves !!!  This is necessary
          ; because on entry to SWI and UNDEF handlers FIQ is still enabled.

          ; It is therefore highly recommendsed that FIQ is not used to
          ; handle Angel Device interrupts, as this is a significant overhead
          ; and is also really quite unpleasant!
          CMP   r14, #0x1c
          BCC   ReallyNastyCase

          LDR   r3, =HandlerSWI
          CMP   r14, r3
          BCC   CheckUNDHandler ; interrupted pc < HandlerSWI
          ADD   r3, r3, #60
          CMP   r14, r3
          BHI   CheckUNDHandler ; interrupted pc >> HandlerSWI
          B     ReallyNastyCase

CheckUNDHandler
  IF :DEF: ICEMAN_LEVEL_3 :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
          B     RegsNowSaved3   ; skip this bit
  ELSE
          LDR   r3, =HandlerUndef
          CMP   r14, r3
          BCC   RegsNowSaved3   ; interrupted pc < HandlerSWI
          ADD   r3, r3, #60
          CMP   r14, r3
          BHI   RegsNowSaved3   ; interrupted pc >> HandlerSWI
  ENDIF

          ; This is the really really nasty case - ignore the interrupt
          ; and resume the SWI Handler with IRQ and FIQ disabled !!!
          ; Why oh Why isn't FIQ disabled on entry to SWI's ??
ReallyNastyCase
          MRS   r3, spsr
          ORR   r3, r3, #IRQDisable + FIQDisable
          MSR   spsr, r3
          B     ReturnFromIntHandler

        ELSE    ; HANDLE_INTERRUPTS_ON_FIQ <> 0

          B     RegsNowSaved3

        ENDIF   ; HANDLE_INTERRUPTS_ON_FIQ <> 0

InterruptedInUSRMode
        ; We need to do this slightly differently depending on whether
        ; this is an IRQ or FIQ based interrupt handler ...
        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        MRS   r3, cpsr
        AND   r3, r3, #ModeMask
        CMP   r3, #IRQmode
        BNE   FIQModeCase

IRQModeCase
        STMIA r0, {r8-r12}      ; r8-r12 are not banked between USR and IRQ
        ADD   r0, r0, #(13*4) - (8*4)
      IF :DEF: STRONGARM_REV1OR2
        MCR   MMUCP, 0, r0, c0, c0, 0
      ENDIF
        STMIA r0, {r13,r14}^    ; Store out the USR r13 and r14
        SUB   r0, r0, #Angel_RegBlock_R0offset + (13*4)
        B     RegsNowSaved3

FIQModeCase
      IF :DEF: STRONGARM_REV1OR2
        MCR   MMUCP, 0, r0, c0, c0, 0
      ENDIF
        STMIA r0, {r8-r14}^     ; r8-r14 are banked between USR and FIQ
        SUB   r0, r0, #Angel_RegBlock_R0offset + (8*4)

RegsNowSaved3
        LDR   r1, =angel_GhostCount
GetSrc  GETSOURCE       r0, lr        ; Get the interrupt vector index in r0
        CMN   r0, #1                  ; Check for Ghost Interrupt
        BEQ   GhostInterrupt          ; It's a Ghost
        CMP   r0, #DE_NUM_INT_HANDLERS; Check if a valid source index
        BGE   UnrecognisedSource      ; Unrecognised source

        LDR   lr, =angel_IntHandler   ; Reference the handlers vector
        ADD   lr, lr, r0, LSL #3      ; lr = angel_IntHandler + r0 * 8
        LDMIA lr, {r4, r5}            ; and load the required func pointer
                                      ; and data into r4 and r5

        MOV   lr, #0
        STR   lr, [r1]                ; clear count of ghost interrupts

        MOV   r1, r5
        TEQ   r4, #0
        BEQ   UnrecognisedSource      ; No handler attached for source

        ; r0 = interrupt vector index, set up above
        ; r1 = data from table, set up above
        ; r2 = empty_stack (stack is currently empty)
        ; r3 not used
        ;
        ; r4 = address of Interrupt handler, which may return but
        ;      WON'T return if it calls the serialiser.

        ; We can now call the APCS-3 handler for this source:
        MOV   r2, sp
        MOV   r3, #0
        MOV   fp,#0x00000000          ; start of call-frame

        ; Set up sl for the appropriate stack (depends on mode)
        MRS   r5, cpsr
        AND   r5, r5, #ModeMask
        LDR   sl, =Angel_StackBase
        LDR   sl, [sl]
        CMP   r5, #IRQmode
        ADDEQ sl, sl, #Angel_IRQStackLimitOffset ; No APCS_STACKGUARD space
        ADDNE sl, sl, #Angel_FIQStackLimitOffset ; No APCS_STACKGUARD space

        ; It is assumed that the handler function called will execute
        ; within the interrupt handler stack allocation, and the code
        ; will not manipulate the interrupt mask status, or the SPSR
        ; register:
        LDR   lr, =ReturnFromIntHandler  ; generate return address
        MOV   pc, r4                     ; call handler function

UnrecognisedSource
ReturnFromIntHandler
        ; We will only get here if the interrupt handler did not need
        ; to grab the serialiser lock.

        ; We won't have interrupted an FIQ, so r0-r12 will not be banked
        ; so we have to reinstate these before returning.

        ; Assume that SPSR has not been corrupted.

        LDR   r14, =Angel_MutexSharedTempRegBlocks
        ADD   r14, r14, #Angel_RegBlock_R0offset
        LDMIA r14, {r0-r12}
        ADD   r14, r14, #4*15   ; Where the PC is stored
        LDMIA r14, {pc}^

        ; And now the interuptee executes in whatever mode it was in ..

        ;
        ; it is possible to get ghost interrupts - ignore these, unless
        ; too many of them stack up
        ;
        ; r1 = &angel_GhostInterrupt
        ;
GhostInterrupt
        LDR     r0, [r1]
        ADD     r0, r0, #1
        CMP     r0, #5
        STR     r0, [r1]
        BLT     ReturnFromIntHandler

TooManyGhosts
        ADR     a1, ghosterrmsg
        B       __rt_asm_fatalerror
ghosterrmsg
        DCB     "Too Many Ghost Interrupts\n"
        ALIGN

        ; ---------------------------------------------------------------------

        AREA    |C$$Code$$LibrarySupport|,CODE,READONLY
        KEEP

        ; Angel is designed *NOT* to require any of the standard ANSI
        ; 'C' library, since it lives beneath the library.  This means
        ; that we need to provide our own versions of certain standard
        ; routines:

        EXPORT __rt_memcpy
        GBLS  SLA                       ; shift towards low address end
        GBLS  SHA                       ; shift towards high address end
 [ {ENDIAN} = "big"
SLA     SETS "LSL"
SHA     SETS "LSR"
 |                                      ; assume little-endian
SLA     SETS "LSR"
SHA     SETS "LSL"
 ]

 [ {TRUE}       ; new, fast memcpy

        GET objmacs.s

src     RN    a2
dst     RN    a1
n       RN    a3
tmp1    RN    a4
tmp3    RN    ip

        Function __rt_memcpy, leaf

        CMP     src, dst
        BLO     CopyDown
        Return  , "", LinkNotStacked, EQ ; dst == src, no move required

        FunctionEntry , "dst"           ; Must return original dst.
        SUBS    n, n, #4                ; need at least 4 bytes
        BLT     Up_TrailingBytes        ; < 4 bytes to go

        ; word align the dst - first find out how many bytes
        ; must be stored to do this.  If the number is 0
        ; check the src too.

        ANDS    tmp3, dst, #3           ; eq means aligned!
        BNE     Up_AlignDst
        ANDS    tmp3, src, #3
        BNE     Up_SrcUnaligned         ; more difficult!

        ; here when source and destination are both aligned.
        ; number of bytes to transfer is (n+4), n is >= 0.

Up_SrcDstAligned
        SUBS    n, n, #12-4             ; 12 bytes or more?
        BLT     Up_TrailingWords
        ; We only have three registers to play with.  It is
        ; worth gaining more only if the number of bytes to
        ; transfer is greater than 12+8*<registers stacked>
        ; We need to stack 8 (4+4) registers to gain 8 temporaries,
        ; so look for >=44 bytes.  Since we would save 8*4 = 32
        ; bytes at a time we actually compare with 64.

        SUBS    n, n, #32-12            ; n+32 to go.
        BLT     %F1

        STMFD   sp!, {v1}

        ; loop loading 4 registers per time, twice (32 bytes)

0       LDMIA   src!, {tmp1, v1, tmp3, lr}
        STMIA   dst!, {tmp1, v1, tmp3, lr}
        LDMIA   src!, {tmp1, v1, tmp3, lr}
        STMIA   dst!, {tmp1, v1, tmp3, lr}
        SUBS    n, n, #32
        BGE     %B0
        ; see if we can handle another 8

        CMN     n, #16
        LDMGEIA src!, {tmp1, v1, tmp3, lr}
        STMGEIA dst!, {tmp1, v1, tmp3, lr}
        SUBGE   n, n, #16

        ; Reload the registers - note that we still have (n+32)
        ; bytes to go, and that this is <16.

        LDMFD   sp!, {v1}

        ; Here when there are fewer than 16 bytes to go.

1       ADDS    n, n, #32-12               ; (n-12) to go

        ; Ok - do three words at a time.

2       LDMGEIA src!, {tmp1, tmp3, lr}
        STMGEIA dst!, {tmp1, tmp3, lr}
        SUBGES  n, n, #12
        BGE     %B2
        ; (n-12) bytes to go - 0, 1 or 2 words.  Check
        ; which.

Up_TrailingWords
        ADDS    n, n, #12-4             ; (n-4) to go
        BLT     Up_TrailingBytes        ; < 4 bytes to go
        SUBS    n, n, #4
        LDRLT   tmp1, [src], #4
        STRLT   tmp1, [dst], #4
        LDMGEIA src!, {tmp1, tmp3}
        STMGEIA dst!, {tmp1, tmp3}
        SUBGE   n, n, #4

        ; Here with less than 4 bytes to go

Up_TrailingBytes
        ADDS    n, n, #4
        Return  , "a1", , EQ            ; 0 bytes
        CMP     n, #2                   ; 1, 2 or 3 bytes
        LDRB    tmp1, [src], #1
        STRB    tmp1, [dst], #1
        LDRGEB  tmp1, [src], #1
        STRGEB  tmp1, [dst], #1
        LDRGTB  tmp1, [src], #1
        STRGTB  tmp1, [dst], #1
        Return  , "a1"                  ; recover old dst value

;------------------------------------------------------------

; word align dst - tmp3 contains current destination
; alignment.  We can store at least 4 bytes here.

Up_AlignDst
        RSB     tmp3, tmp3, #4          ; 1-3 bytes to go
        CMP     tmp3, #2
        LDRB    tmp1, [src], #1
        STRB    tmp1, [dst], #1
        LDRGEB  tmp1, [src], #1
        STRGEB  tmp1, [dst], #1
        LDRGTB  tmp1, [src], #1
        STRGTB  tmp1, [dst], #1
        SUBS    n, n, tmp3              ; check number to go
        BLT     Up_TrailingBytes        ; less than 4 bytes
        ANDS    tmp3, src, #3
        BEQ     Up_SrcDstAligned        ; coaligned case

        ; The source is not coaligned with the destination,
        ; the destination IS currently word aligned.

Up_SrcUnaligned
        BIC     src, src, #3            ; tmp3 holds extra!
        LDR     lr, [src], #4           ; 1-3 useful bytes
        CMP     tmp3, #2
        BGT     Up_OneByte              ; one byte in tmp1
        BEQ     Up_TwoBytes             ; two bytes in tmp1

; The next three source bytes are in tmp1, one byte must
; come from the next source word.  At least four bytes
; more must be stored.  Check first to see if there are a
; sufficient number of bytes to go to justify using stm/ldm
; instructions.

Up_ThreeBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no                    ; 1
        SUB     n, n, #16-4             ; (n+16) bytes to go    ; 1

        ; save some work registers.  The point at which this
        ; is done is based on the ldm/stm time being = (n+3)+(n/4)S

        STMFD   sp!, {v1, v2}                                  ; 14   ????

        ; loop doing 16 bytes at a time.  There are currently
        ; three useful bytes in lr.

0       MOV     tmp1, lr, $SLA #8        ; first three bytes     ; 1
        LDMIA   src!, {v1, v2, tmp3, lr}                         ; 12/13
        ORR     tmp1, tmp1, v1, $SHA #24         ; word 1        ; 1
        MOV     v1, v1, $SLA #8                                  ; ...
        ORR     v1, v1, v2, $SHA #24             ; word 2        ; 2 (1+1)
        MOV     v2, v2, $SLA #8
        ORR     v2, v2, tmp3, $SHA #24           ; word 3        ; 2
        MOV     tmp3, tmp3, $SLA #8
        ORR     tmp3, tmp3, lr, $SHA #24         ; word 4        ; 2
        STMIA   dst!, {tmp1, v1, v2, tmp3}                       ; 12/13
        SUBS    n, n, #16                                        ; 1
        BGE     %B0                                              ; 4 / 1

        ; loop timing (depends on alignment) for n loops:-

        ;       pre:    17
        ;               ((45/46/47)n - 3) for 32n bytes
        ;       post:   13/14
        ;       total:  (45/46/47)n+(27/28)
        ;       32 bytes:       72-75
        ;       64 bytes:       117-122
        ;       96 bytes:       162-169

        ; Reload registers

        LDMFD   sp!, {v1, v2}                                   ; 12/13 ????

        ADDS    n, n, #16-4              ; check for at least 4
        BLT     %F2                      ; < 4 bytes

1       MOV     tmp3, lr, $SLA #8        ; first three bytes     ; 1
        LDR     lr, [src], #4            ; next four bytes       ; 4
        ORR     tmp3, tmp3, lr, $SHA #24                         ; 1
        STR     tmp3, [dst], #4                                  ; 4
        SUBS    n, n, #4                                         ; 1
        BGE     %B1                      ; tmp1 contains three bytes 1 / 4

        ; Loop timing:

        ;               15n-3   for 4n bytes
        ;       32:     117
        ;       64:     237

        ; Less than four bytes to go - readjust the src
        ; address.

2       SUB     src, src, #3
        B       Up_TrailingBytes

; The next two source bytes are in tmp1, two bytes must
; come from the next source word.  At least four bytes
; more must be stored.

Up_TwoBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2}

        ; loop doing 32 bytes at a time.  There are currently
        ; two useful bytes in lr.

0       MOV     tmp1, lr, $SLA #16       ; first two bytes
        LDMIA   src!, {v1, v2, tmp3, lr}
        ORR     tmp1, tmp1, v1, $SHA #16 ; word 1
        MOV     v1, v1, $SLA #16
        ORR     v1, v1, v2, $SHA #16     ; word 2
        MOV     v2, v2, $SLA #16
        ORR     v2, v2, tmp3, $SHA #16   ; word 3
        MOV     tmp3, tmp3, $SLA #16
        ORR     tmp3, tmp3, lr, $SHA #16 ; word 4
        STMIA   dst!, {tmp1, v1, v2, tmp3}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2}

        ADDS    n, n, #16-4              ; check number of bytes
        BLT     %F2
1       MOV     tmp3, lr, $SLA #16       ; first two bytes
        LDR     lr, [src], #4            ; next four bytes
        ORR     tmp3, tmp3, lr, $SHA #16
        STR     tmp3, [dst], #4
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains two bytes

        ; Less than four bytes to go - readjust the src
        ; address.

2       SUB     src, src, #2
        B       Up_TrailingBytes

; The next source byte is in tmp1, three bytes must
; come from the next source word.  At least four bytes
; more must be stored.

Up_OneByte
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2}

        ; loop doing 32 bytes at a time.  There is currently
        ; one useful byte in lr

0       MOV     tmp1, lr, $SLA #24       ; first byte
        LDMIA   src!, {v1, v2, tmp3, lr}
        ORR     tmp1, tmp1, v1, $SHA #8  ; word 1
        MOV     v1, v1, $SLA #24
        ORR     v1, v1, v2, $SHA #8      ; word 2
        MOV     v2, v2, $SLA #24
        ORR     v2, v2, tmp3, $SHA #8    ; word 3
        MOV     tmp3, tmp3, $SLA #24
        ORR     tmp3, tmp3, lr, $SHA #8  ; word 4
        STMIA   dst!, {tmp1, v1, v2, tmp3}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2}

        ADDS    n, n, #16-4              ; check number of bytes
        BLT     %F2
1       MOV     tmp3, lr, $SLA #24       ; first byte
        LDR     lr, [src], #4            ; next four bytes
        ORR     tmp3, tmp3, lr, $SHA #8
        STR     tmp3, [dst], #4
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains one byte

        ; Less than four bytes to go - one already in tmp3.

2       SUB     src, src, #1
        B       Up_TrailingBytes

;======================================================================
; Copy down code
; ==============

;       This is exactly the same as the copy up code -
;       but it copies in the opposite direction.

CopyDown
        ADD     src, src, n             ; points beyond end
        ADD     dst, dst, n

        SUBS    n, n, #4                ; need at least 4 bytes
        BLT     Down_TrailingBytes      ; < 4 bytes to go

        ; word align the dst - first find out how many bytes
        ; must be stored to do this.  If the number is 0
        ; check the src too.

        ANDS    tmp3, dst, #3           ; eq means aligned!
        BNE     Down_AlignDst
        ANDS    tmp3, src, #3
        BNE     Down_SrcUnaligned       ; more difficult!

        ; here when source and destination are both aligned.
        ; number of bytes to transfer is (n+4), n is >= 0.

Down_SrcDstAligned
        SUBS    n, n, #12-4             ; 12 bytes or more?
        BLT     Down_TrailingWords
        ; We only have three registers to play with.  It is
        ; worth gaining more only if the number of bytes to
        ; transfer is greater than 12+8*<registers stacked>
        ; We need to stack 8 (4+4) registers to gain 8 temporaries,
        ; so look for >=44 bytes.  Since we would save 8*4 = 32
        ; bytes at a time we actually compare with 64.

        STMFD   sp!, {v1, lr}
        SUBS    n, n, #32-12            ; n+32 to go.
        BLT     %F1

        ; loop loading 4 registers per time, twice (32 bytes)

0       LDMDB   src!, {tmp1, v1, tmp3, lr}
        STMDB   dst!, {tmp1, v1, tmp3, lr}
        LDMDB   src!, {tmp1, v1, tmp3, lr}
        STMDB   dst!, {tmp1, v1, tmp3, lr}
        SUBS    n, n, #32
        BGE     %B0
        ; see if we can handle another 8

1       CMN     n, #16
        LDMGEDB src!, {tmp1, v1, tmp3, lr}
        STMGEDB dst!, {tmp1, v1, tmp3, lr}
        SUBGE   n, n, #16

        ; Here when there are fewer than 16 bytes to go.

        ADDS    n, n, #32-12            ; (n-12) to go

        ; Ok - do three words at a time.

        LDMGEDB src!, {tmp1, tmp3, lr}
        STMGEDB dst!, {tmp1, tmp3, lr}
        SUBGE   n, n, #12
        LDMFD   sp!, {v1, lr}
        ; (n-12) bytes to go - 0, 1 or 2 words.  Check
        ; which.

Down_TrailingWords
        ADDS    n, n, #12-4             ; (n-4) to go
        BLT     Down_TrailingBytes      ; < 4 bytes to go
        SUBS    n, n, #4
        LDRLT   tmp1, [src, #-4]!
        STRLT   tmp1, [dst, #-4]!
        LDMGEDB src!, {tmp1, tmp3}
        STMGEDB dst!, {tmp1, tmp3}
        SUBGE   n, n, #4

        ; Here with less than 4 bytes to go

Down_TrailingBytes
        ADDS    n, n, #4
        Return  , "", LinkNotStacked, EQ ; 0 bytes
        CMP     n, #2                    ; 1, 2 or 3 bytes
        LDRB    tmp1, [src, #-1]!
        STRB    tmp1, [dst, #-1]!
        LDRGEB  tmp1, [src, #-1]!
        STRGEB  tmp1, [dst, #-1]!
        LDRGTB  tmp1, [src, #-1]!        ; dst is now original dst
        STRGTB  tmp1, [dst, #-1]!
        Return  , "", LinkNotStacked

;------------------------------------------------------------

; word align dst - tmp3 contains current destination
; alignment.  We can store at least 4 bytes here.  We are
; going downwards - so tmp3 is the actual number of bytes
; to store.

Down_AlignDst
        CMP     tmp3, #2
        LDRB    tmp1, [src, #-1]!
        STRB    tmp1, [dst, #-1]!
        LDRGEB  tmp1, [src, #-1]!
        STRGEB  tmp1, [dst, #-1]!
        LDRGTB  tmp1, [src, #-1]!
        STRGTB  tmp1, [dst, #-1]!
        SUBS    n, n, tmp3              ; check number to go
        BLT     Down_TrailingBytes      ; less than 4 bytes
        ANDS    tmp3, src, #3
        BEQ     Down_SrcDstAligned      ; coaligned case

        ; The source is not coaligned with the destination,
        ; the destination IS currently word aligned.

Down_SrcUnaligned
        BIC     src, src, #3            ; tmp3 holds extra!
        LDR     tmp1, [src]             ; 1-3 useful bytes
        CMP     tmp3, #2
        BLT     Down_OneByte            ; one byte in tmp1
        BEQ     Down_TwoBytes           ; two bytes in tmp1

; The last three source bytes are in tmp1, one byte must
; come from the previous source word.  At least four bytes
; more must be stored.  Check first to see if there are a
; sufficient number of bytes to go to justify using stm/ldm
; instructions.

Down_ThreeBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2, lr}

        ; loop doing 32 bytes at a time.  There are currently
        ; three useful bytes in tmp1 (a4).

0       MOV     lr, tmp1, $SHA #8        ; last three bytes
        LDMDB   src!, {tmp1, v1, v2, tmp3}
        ORR     lr, lr, tmp3, $SLA #24   ; word 4
        MOV     tmp3, tmp3, $SHA #8
        ORR     tmp3, tmp3, v2, $SLA #24 ; word 3
        MOV     v2, v2, $SHA #8
        ORR     v2, v2, v1, $SLA #24     ; word 2
        MOV     v1, v1, $SHA #8
        ORR     v1, v1, tmp1, $SLA #24   ; word 1
        STMDB   dst!, {v1, v2, tmp3, lr}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2, lr}

        ADDS    n, n, #16-4              ; check for at least 4
        BLT     %F2                      ; < 4 bytes

1       MOV     tmp3, tmp1, $SHA #8      ; last three bytes
        LDR     tmp1, [src, #-4]!        ; previous four bytes
        ORR     tmp3, tmp3, tmp1, $SLA #24
        STR     tmp3, [dst, #-4]!
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains three bytes

        ; Less than four bytes to go - readjust the src
        ; address.

2       ADD     src, src, #3
        B       Down_TrailingBytes

; The last two source bytes are in tmp1, two bytes must
; come from the previous source word.  At least four bytes
; more must be stored.

Down_TwoBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2, lr}

        ; loop doing 32 bytes at a time.  There are currently
        ; two useful bytes in tmp1 (a4).

0       MOV     lr, tmp1, $SHA #16       ; last two bytes
        LDMDB   src!, {tmp1, v1, v2, tmp3}
        ORR     lr, lr, tmp3, $SLA #16   ; word 4
        MOV     tmp3, tmp3, $SHA #16
        ORR     tmp3, tmp3, v2, $SLA #16 ; word 3
        MOV     v2, v2, $SHA #16
        ORR     v2, v2, v1, $SLA #16     ; word 2
        MOV     v1, v1, $SHA #16
        ORR     v1, v1, tmp1, $SLA #16   ; word 1
        STMDB   dst!, {v1, v2, tmp3, lr}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2, lr}

        ADDS    n, n, #16-4              ; check for at least 4
        BLT     %F2                      ; < 4 bytes

1       MOV     tmp3, tmp1, $SHA #16     ; last two bytes
        LDR     tmp1, [src, #-4]!        ; previous four bytes
        ORR     tmp3, tmp3, tmp1, $SLA #16
        STR     tmp3, [dst, #-4]!
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains two bytes

        ; Less than four bytes to go - readjust the src
        ; address.

2       ADD     src, src, #2
        B       Down_TrailingBytes

; The last source byte is in tmp1, three bytes must
; come from the previous source word.  At least four bytes
; more must be stored.

Down_OneByte
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2, lr}

        ; loop doing 32 bytes at a time.  There is currently
        ; one useful byte in tmp1 (a4).

0       MOV     lr, tmp1, $SHA #24       ; last byte
        LDMDB   src!, {tmp1, v1, v2, tmp3}
        ORR     lr, lr, tmp3, $SLA #8    ; word 4
        MOV     tmp3, tmp3, $SHA #24
        ORR     tmp3, tmp3, v2, $SLA #8  ; word 3
        MOV     v2, v2, $SHA #24
        ORR     v2, v2, v1, $SLA #8      ; word 2
        MOV     v1, v1, $SHA #24
        ORR     v1, v1, tmp1, $SLA #8    ; word 1
        STMDB   dst!, {v1, v2, tmp3, lr}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2, lr}

        ADDS    n, n, #16-4               ; check for at least 4
        BLT     %F2                       ; < 4 bytes

1       MOV     tmp3, tmp1, $SHA #24      ; last byte
        LDR     tmp1, [src, #-4]!         ; previous four bytes
        ORR     tmp3, tmp3, tmp1, $SLA #8
        STR     tmp3, [dst, #-4]!
        SUBS    n, n, #4
        BGE     %B1                       ; tmp1 contains one byte

        ; Less than four bytes to go - one already in tmp3.

2       ADD     src, src, #1
        B       Down_TrailingBytes

 |      ; old, slow memcpy

__rt_memcpy
        ; a1 = dst address
        ; a2 = src address
        ; a3 = byte count.
        ;
        ; A very slow version of memcpy. Its only merit is that it is quite
        ; small.
        ;
        CMP     a3,#0
        MOVEQ   pc,lr
        CMP     a2,a1
        BLO     CopyDown
        MOVEQ   pc,lr

        MOV     ip,a1
CopyUpLoop
        LDRB    a4,[a2],#1
        STRB    a4,[ip],#1
        SUBS    a3,a3,#1
        BNE     CopyUpLoop

memcpyExit
        MOV     pc,lr

CopyDown
        ADD     a2,a2,a3
        ADD     a1,a1,a3
CopyDownLoop
        LDRB    a4,[a2,#-1]!
        STRB    a4,[a1,#-1]!
        SUBS    a3,a3,#1
        BNE     CopyDownLoop
        BAL     memcpyExit

 ]

        EXPORT __rt_udiv
__rt_udiv
        ; Signed divide of a2 by a1, returns quotient in a1, remainder in a2
        ; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
        ; Destroys a3, a4 and ip
        MOVS    a3,a1
        BEQ     __division_by_zero
        MOV     a4,#0
        MOV     ip,#0x80000000
        CMP     a2,# ip
        MOVLO   ip,a2
01
        CMP     ip,a3,ASL #0
        BLS     u_shifted0mod8
        CMP     ip,a3,ASL #1
        BLS     u_shifted1mod8
        CMP     ip,a3,ASL #2
        BLS     u_shifted2mod8
        CMP     ip,a3,ASL #3
        BLS     u_shifted3mod8
        CMP     ip,a3,ASL #4
        BLS     u_shifted4mod8
        CMP     ip,a3,ASL #5
        BLS     u_shifted5mod8
        CMP     ip,a3,ASL #6
        BLS     u_shifted6mod8
        CMP     ip,a3,ASL #7
        MOVHI   a3,a3,ASL #8
        BHI     %BT01
02
u_shifted7mod8
        CMP     a2,a3,ASL #7
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #7
u_shifted6mod8
        CMP     a2,a3,ASL #6
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #6
u_shifted5mod8
        CMP     a2,a3,ASL #5
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #5
u_shifted4mod8
        CMP     a2,a3,ASL #4
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #4
u_shifted3mod8
        CMP     a2,a3,ASL #3
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #3
u_shifted2mod8
        CMP     a2,a3,ASL #2
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #2
u_shifted1mod8
        CMP     a2,a3,ASL #1
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #1
u_shifted0mod8
        CMP     a2,a3,ASL #0
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #0
        CMP     a1,a3,LSR #1
        MOVLS   a3,a3,LSR #8
        BLS     %BT02
        MOV     a1,a4
        MOV     pc,lr

__division_by_zero
        ; Currently we do not deal with division by zero in a nice way
        MOV     a1,#0
        MOV     a2,#0
        MOV     pc,lr

        EXPORT  __rt_udiv10
__rt_udiv10     ROUT
        SUB     a2, a1, #10
        SUB     a1, a1, a1, lsr #2
        ADD     a1, a1, a1, lsr #4
        ADD     a1, a1, a1, lsr #8
        ADD     a1, a1, a1, lsr #16
        MOV     a1, a1, lsr #3
        ADD     a3, a1, a1, asl #2
        SUBS    a2, a2, a3, asl #1
        ADDPL   a1, a1, #1
        ADDMI   a2, a2, #10
        MOV     pc, lr

        EXPORT  __rt_memcmp
__rt_memcmp     ROUT
        MOV     a4, a1
        MOVS    a1, a3
        MOVEQ   pc, lr

0       LDRB    a1, [a2], #1
        LDRB    ip, [a4], #1
        SUBS    a1, a1, ip
        MOVNE   pc, lr

        SUBS    a3, a3, #1
        BNE     %0
        MOV     pc, lr

        EXPORT  __rt_strlen
__rt_strlen     ROUT
        MOV     a2, a1
        MOV     a1, #0

0       LDRB    a3, [a2], #1
        CMP     a3, #0
        MOVEQ   pc, lr

        ADD     a1, a1, #1
        B       %0
        
;;      The following routines are currently only needed for the Fusion
;;      TCP/IP stack and Ethernet drivers, so don't include them if we
;;      don't have to

  IF    ETHERNET_SUPPORTED /= 0

        EXPORT  __rt_divtest
__rt_divtest    ROUT
        CMPS    a1, #0
        MOVNE   pc, lr

;;
;;      Fall through to...
;;

__rt_div0       ROUT
        MOV     a1, #angel_SWIreason_ReportException
        LDR     a2, =ADP_Stopped_DivisionByZero
        DCD     angel_SWI_ARM

        ; if we ever continue from this SWI, then report this and die
        ADR     a1, div0msg
        B       __rt_asm_fatalerror
div0msg
        DCB     "Divide by zero\n"
        ALIGN

  ENDIF ; ETHERNET_SUPPORTED /= 0
  
;;
;; __rt_memXXX routines - small but inefficient
;;
        EXPORT  __rt_memset
__rt_memset     ROUT
        ADD     a3, a3, #1
        MOV     a4, a1

0       SUBS    a3, a3, #1
        MOVEQ   pc, lr
        STRB    a2, [a4], #1
        B       %0

        EXPORT  __rt_strcmp
__rt_strcmp     ROUT
        MOV     ip, a1

0       LDRB    a3, [ip], #1
        LDRB    a4, [a2], #1

        SUBS    a1, a3, a4
        MOVNE   pc, lr

        CMP     a3, #0
        CMPNE   a4, #0
        BNE     %0
        MOV     pc, lr

        EXPORT  __rt_strncmp
__rt_strncmp    ROUT
        MOV     ip, a1
        MOVS    a1, a3
        MOVEQ   pc, lr

        STR     lr, [sp, #-4]!

0       LDRB    lr, [ip], #1
        LDRB    a4, [a2], #1

        SUBS    a1, lr, a4
        LDRNE   pc, [sp], #4

        CMP     lr, #0
        CMPNE   a4, #0
        SUBNES  a3, a3, #1
        BNE     %0
        LDR     pc, [sp], #4

        EXPORT  __rt_strcpy
__rt_strcpy     ROUT
        MOV     ip, a1

0       LDRB    a3, [a2], #1
        STRB    a3, [ip], #1
        CMP     a3, #0
        BNE     %0

        MOV     pc, lr

        EXPORT  __rt_strcat
__rt_strcat     ROUT
        MOV     ip, a1          ; so we can return a1
0
        LDRB    a3, [ip], #1
        CMP     a3, #0
        BNE     %0
        SUB     ip, ip, #1      ; want to overwrite NUL
        
1       LDRB    a3, [a2], #1
        STRB    a3, [ip], #1
        CMP     a3, #0
        BNE     %1
        
        MOV     pc, lr


  IF :DEF: LINKING_WITH_CLIB
        ; stack checking support is in the C lib
  ELSE
        ; ---------------------------------------------------------------------
        ; -- Software stack checking support ----------------------------------
        ; ---------------------------------------------------------------------

        ; The following functions are provided for software stack
        ; checking. If hardware stack-checking is being used then the
        ; code can be compiled without the PCS entry checks, and
        ; simply rely on VM management to extend the stack for a
        ; thread.
        ;
        ; The stack extension event occurs when the PCS function
        ; entry code would result in a stack-pointer beneath the
        ; stack-limit register value. The system relies on the
        ; following map:
        ;
        ; +-----------------------------------+ <-- end of stack block
        ; | ...                               |
        ; | ...                               |
        ; | active stack                      |
        ; | ...                               | <-- sp (stack-pointer) somewhere in here
        ; | ...                               |
        ; +-----------------------------------+ <-- sl (stack-limit)
        ; | stack-extension handler workspace |
        ; +-----------------------------------+ <-- base of stack block
        ;
        ; The "stack-extension handler workspace" is an amount of
        ; memory in which the stack overflow support code must
        ; execute. It must be large enough to deal with the worst
        ; case through the stack overflow handler code.
        ;
        ; At the moment the compiler expects this to be AT LEAST
        ; 256bytes. It uses this fact to code functions with small
        ; local data usage within the overflow space.
        ;
        ; NOTE: We may need to increase the space between sl and the
        ; true limit to allow for the stack extension code and any
        ; other system software that may temporarily use the stack of
        ; the current foreground thread.
        ;
        ; The following example stack overflow handling code requires
        ; the following run-time functions:
        ;       __rt_allocmem   ; Kernel memory allocator "malloc"
        ;       __rt_freemem    ; Kernel memory allocator "free"
        ; We need to ensure that the MAXIMUM stack usage possible by the
        ; above routines, fits into the stack overflow handler space
        ; left beneath the "sl" value. The above routines also need to
        ; be compiled with stack-overflow checking disabled.
        ;
        ; If the "__rt_" routines needed by this code are not
        ; interrupt safe (i.e. if they cannot be called from within an
        ; interrupt handler) then this code must ensure that any code
        ; executing as part of an interrupt handler does NOT generate
        ; a stack extension event.

        AREA    |StackOverflow$$Code|,CODE,READONLY
        KEEP

        ; NOTE: This code assumes that it is entered with a valid
        ; frame-pointer. If the system is being constructed without
        ; frame-pointer support, then the following code will not
        ; work, and an alternative means of providing soft
        ; stack-extension will need to be coded.

        EXPORT __rt_stkovf_split_small
__rt_stkovf_split_small
        ; Called when we have a standard register saving only
        ; stack-limit check failure.
        MOV     ip,sp   ; ensure we can calculate the amount of stack required
        ; and then fall through to...

        IMPORT Angel_StackBase
        EXPORT __rt_stkovf_split_big
__rt_stkovf_split_big
        ; If we are in a privileged mode then stack overflow is fatal, so
        ; we should do nothing more than output a debugging message and
        ; give up!

        MRS     r0, CPSR
        AND     r0, r0, #ModeMask
        CMP     r0, #USRmode
        BEQ     USRmodeStackCheck

        MOV     r1, r0
        ADR     r0, PrivilegedStackOverflowMessage ;  r0 == a1
        B       __rt_asm_fatalerror
        
PrivilegedStackOverflowMessage
        DCB     "Stack Overflow in mode %2X\n"
        ALIGN

USRmodeStackCheck
        ; If this is the AngelStack then sl should
        ; be somewhere within the Angel Stack range
        LDR     r1, =Angel_StackBase
        LDR     r1, [r1]
        ADD     r0, r1, #Angel_AngelStackLimitOffset
        ADD     r1, r1, #Angel_AngelStackOffset
        SUB     r1, r1, #1
        CMP     sl, r0
        CMPHS   r1, sl

AngelStackOverflow
        BHS     AngelStackOverflow

RealStackCheckCode
        ; Called when we have a large local stack allocation (>
        ; 256bytes with the current C compiler) which would cause an
        ; overflow. This version is called when the compiler generated
        ; PCS code has checked its requirement against the stack-limit
        ; register.
        ;
        ; in:   sp = current stack-pointer (beneath stack-limit)
        ;       sl = current stack-limit
        ;       ip = low stack point we require for the current function
        ;       lr = return address back to the function requiring more stack
        ;       fp = frame-pointer
        ;
        ;               original sp --> +----------------------------------+
        ;                               | pc (12 ahead of PCS entry store) |
        ;               current fp ---> +----------------------------------+
        ;                               | lr (on entry) pc (on exit)       |
        ;                               +----------------------------------+
        ;                               | sp ("original sp" on entry)      |
        ;                               +----------------------------------+
        ;                               | fp (on entry to function)        |
        ;                               +----------------------------------+
        ;                               |                                  |
        ;                               | ..argument and work registers..  |
        ;                               |                                  |
        ;               current sp ---> +----------------------------------+
        ;
        ; The "current sl" is somewhere between "original sp" and
        ; "current sp" but above "true sl". The "current sl" should be
        ; "APCS_STACKGUARD" bytes above the "true sl". The value
        ; "APCS_STACKGUARD" should be large enough to deal with the
        ; worst case function entry stacking (160bytes) plus the stack
        ; overflow handler stacking requirements, plus the stack
        ; required for the memory allocation routines, and the
        ; exception handling code.
        ;
        ; THINGS TO NOTE:
        ; We should ensure that every function that calls
        ; "__rt_stkovf_split_small" or "__rt_stkovf_split_big" does so
        ; with a valid PCS. ie. they should use "fp" to de-stack on
        ; exit (see notes above).
        ;
        ; Code should never poke values beneath sp. The sp register
        ; should always be "dropped" first to cover the data. This
        ; protects the data against any events that may try and use
        ; the stack. This is a requirement of APCS-3.
        ;
        SUB     ip,sp,ip        ; extra stack required for the function
        STMFD   sp!,{v1,v2,lr}  ; temporary work registers
        ;

        ; For simplicity never attempt to extend the stack just report
        ; an overflow.
RaiseStackOverflow
        ; in:  v1 = undefined
        ;      v2 = undefined
        ;      sl = stack-limit
        ;      ip = amount of stack required
        ;      sp = FD stack containing {v1,v2,lr}
        ;      lr = undefined
        ;

        ;; 960506 KWelton       
        ;;      
        ;; I'm not sure what this is trying to do, but it all seems redundant
        ;; to me - what we *really* want to do is report the overflow, and
        ;; then stop in a tight loop.
        
        ;; others disagree... this kills Angel, and without angel debug
        ;; code, this is not useful. IJ has requested that such errors
        ;; cause a restart of angel (i.e. jump to __rt_angel_restart)
        ;; while debug builds can jump to Deadloop. -- 22/10/97 RIC
        
        IF {FALSE}

        MRS     v1,CPSR         ; get current PSR
        TST     v1,#ModeMaskUFIS

        MOV     v2, a1
        LDR     a1, =angel_SWIreason_EnterSVC
        DCD     angel_SWI_ARM
        MOV     a1, v2
        ; We are now in a suitably priviledged mode.
        MSR     SPSR,v1         ; get original PSR into SPSR
        STMFD   sp!,{r10-r12}   ; work registers
        LDR     r10,=ADP_Stopped_RunTimeError
        MOV     r11,#ADP_RunTime_Error_StackOverflow
        MOV     lr,pc           ; return address

        ELSE

        MOV     a1, #angel_SWIreason_ReportException
        LDR     a2, =ADP_Stopped_StackOverflow
        DCD     angel_SWI_ARM

        ENDIF
        
        ;; If we get here, print a message and die...
        ADR     a1, stkovfmsg
        B       __rt_asm_fatalerror
stkovfmsg
        DCB     "Stack Overflow\n"
        ALIGN
        
        ;; This instruction matched a STM above, but I can't see any way
        ;;  of actualy executing it!! It should probably be removed. -- ric
        LDMFD   sp!,{v1,v2,pc}  ; and return to caller

  ENDIF ; :DEF: LINKING_WITH_CLIB

        ; ---------------------------------------------------------------------

        IF CACHE_SUPPORTED <> 0
          ; Added for EBSARM, Instruction barrier range function. r0
          ; contains start of range, r1 end.
Cache_IBR
          ; Simply call the board dependent macro
          CACHE_IBR r0,r1,r2,r3
          ; And return
          MOV   pc,lr
        ENDIF

        IMPORT  Angel_EnterSVC
        EXPORT  __rt_uninterruptable_loop
__rt_uninterruptable_loop
        MRS     a1, CPSR
        AND     a2, a1, #ModeMask
        CMP     a2, #USRmode
        BLEQ    Angel_EnterSVC
        ORRNE   a1, a1, #IRQDisable + FIQDisable
        MSRNE   CPSR, a1
DeadLoop
        B       DeadLoop

        ; This code simulates the LogFatalError macro used in the C parts of
        ; Angel. Enter it with a1 pointing at the error message.
        EXPORT  __rt_asm_fatalerror
        IMPORT  __rt_logerror, WEAK
        IMPORT  __rt_logmsginfo, WEAK
__rt_asm_fatalerror

        ;; if the logerror routine exists, use it to print the message
        LDR     a2, =__rt_logerror
        CMP     a2, #0
        BEQ     no_logerror
        
        ;; these variables are used in C code to say where the call came from;
        ;;  we don't have that info to hand for asm.
        STMFD   sp!, {r1,r2,r3}
        MOV     r1, #0
        MOV     r2, #0
        MOV     r3, #0
        BL      __rt_logmsginfo
        LDMFD   sp!, {r1,r2,r3}
        BL      __rt_logerror
no_logerror
        ; debug build jumps into DeadLoop, release build restarts Angel.
        IF      DEBUG = 1
        BL      __rt_uninterruptable_loop
        ELSE
        BL      __rt_angel_restart
        ENDIF


        ;; This routine exists as the action used when Angel needs to be
        ;; restarted. There are two occasions; when the debugger asks for
        ;; it, and when Angel detects a fatal error. The routine should
        ;; reload Angel from ROM, if possible, and restart as if a
        ;; power-on-reset had just occurred, as NOTHING can be assumed about
        ;; the current state of anything in RAM.
        ;; 
        ;; THIS ROUTINE NEVER RETURNS!
        
__rt_angel_restart

        IMPORT __rom
        EXPORT __rt_angel_restart

        ;; for the moment, just start at the beginning again...
        B __rom
        
                                
        END     ; EOF suppasm.s
