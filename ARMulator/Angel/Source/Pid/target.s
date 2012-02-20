        SUBT    PID board description and Angel support         > target.s
;
; $Revision: 1.9.6.1 $
;   $Author: rivimey $
;     $Date: 1997/12/10 18:51:29 $
;
; Copyright (c) 1996 Advanced RISC Machines Limited
; All Rights Reserved
;
; ---------------------------------------------------------------------
; This file defines the PID manifests and macros required
; to support the Angel world. It is not a complete description
; of the board, only the information necessary for Angel. It
; is envisaged that individual device drivers or application
; code will hold the necessary definitions for the other board
; features.
;
; NOTE: To help keep the main Angel source as generic as
; possible, use is made of macros. However, it is likely
; that the use of macros will hide some very obvious and
; simple optimisations. If optimised Angel source is required,
; then a fixed target version should be created by the porting
; developer. The aim of Angel is to be as simple and quick to
; port as possible.
; ---------------------------------------------------------------------

        ASSERT  (listopts_s)
old_opt SETA    {OPT}
        OPT     (opt_off)   ; disable listing of include files

; ---------------------------------------------------------------------

    IF  (:LNOT: :DEF: target_s)

                GBLL    target_s
target_s        SETL    {TRUE}

; ---------------------------------------------------------------------
; The "ROMonly" build variable controls whether the system
; being built supports a system where only ROM is mapped at
; zero, and RAM cannot be mapped there. i.e. where the ARM
; vectors are hard-wired in ROM. We create this variable
; un-conditionally, to ensure that if an attempt is made to
; define it elsewhere then a build error will occur.

                GBLL    ROMonly         ; depends on target
ROMonly         SETL    {FALSE}         ; {TRUE} if no RAM at zero

; ---------------------------------------------------------------------
; MMU initialisation manifests.
; For the default boot ROM operation we want the cache and
; write-buffer to be disabled, so that any external state
; monitoring gets a clearer picture of what is happening
; within the processor. When a commercial ROM system is being
; constructed, i.e. when the final application is being
; ROMmed, we should ensure that the cache and writebuffer are
; enabled. This can be achieved within the ROM startup code
; (INITMMU macro), or later by the actual application.

EnableMMU32             EQU     (Config32 :OR: MMUOn)
EnableMMUCW32           EQU     (Config32 :OR: MMUOn :OR: CacheOn :OR: WriteBufferOn)

; ---------------------------------------------------------------------
; -- Support macros ---------------------------------------------------
; ---------------------------------------------------------------------

;; UNMAPROM
;; --------
;; Provide code to deal with mapping the reset ROM away from zero
;; (if required).

        MACRO
$label  UNMAPROM        $w1,$w2

; The PID maps ROM away from 0 by writing a random word to a specific
; address within the Reset and Wait control area.

$label  MOV     $w1, #RPCBASE
        STR     $w1, [$w1, #ClearResetMap - RPCBASE]
        MEND

;; STARTUPCODE
;; -----------
;; Provide code to deal with ensuring that the
;; target I/O world is suitably clean before attempting to
;; initialise the rest of Angel.

        MACRO
$label  STARTUPCODE     $w1, $w2, $pos, $ramsize

;; leave $pos alone - already set - unless we have good reason
;; to change it.  Here we don't.

$label  MOV     $ramsize,#0x00000000    ; RAM size not yet known

;; We are expected to disable all IRQ and FIQ sources, so do so here

        MOV     $w1, #ICBASE
        MVN     $w2, #0
        STR     $w2, [$w1, #IRQEnableClear - ICBASE]!
        STR     $w2, [$w1, #FIQEnableClear - IRQEnableClear]

        MEND

; ---------------------------------------------------------------------
; INITMMU
; -------
; Setup the memory map world for the target. On certain system
; where the target does not have a MMU, then this macro can be
; a NOP.
;
        MACRO
$label  INITMMU         $tmp,$tmp2,$tmp3,$tmp4,$tmp5,$tmp6
        ROUT
$label
        IF :DEF: USE_ARM600_MMU
  ; Setup the ARM600 style MMU with a simple flat page table:
          LDR     $tmp,=angel_MMUtype
          LDR     $tmp,[$tmp]
          CMP     $tmp,#0
          BEQ     %F10
          MCR     MMUCP,0,$tmp,MMUFlushTLBReg,c0  ; Flush the TLB
          MCR     MMUCP,0,$tmp,MMUFlushIDCReg,c0  ; Flush the cache
          LDR     $tmp,=0xFFFFFFFF
          MCR     MMUCP,0,$tmp,MMUDomainAccessReg,c0 ; set domains
  ; The level 1 page table can be found on the next 16K boundary
  ; after the end of the ROM code:
          IMPORT  |Image$$RO$$Limit| ; only valid when building ROM images
          LDR     $tmp,=|Image$$RO$$Limit|        ;
          BIC     $tmp,$tmp,#0x3F00
          BIC     $tmp,$tmp,#0x00FF       ; mask off at 16K boundary
          ADD     $tmp,$tmp,#0x4000       ; bump up to next 16K border
  ; Reference the page table, and initialise the MMU:
          MCR     MMUCP,0,$tmp,MMUPageTableBaseReg,c0 ; Level 1 page table
  ;
  ; See the comments above relating to enabling the cache and
  ; write-buffer. By default this code does not enable either at
  ; the moment.
          MOV     $tmp,#EnableMMU32              ; Default MMU configuration
          MCR     MMUCP,0,$tmp,MMUControlReg,c0  ; Ensure MMU enabled
10

        ELSE
  ; No MMU in the system - do nothing
        ENDIF

        MEND

; ---------------------------------------------------------------------
; INITTIMER
; ---------
; This macro is provided purely as a holder for any code that
; may be required to initialise a hardware timer as part of
; the reset sequence. Angel does *NOT* make use of any timer
; resources, so by default this macro should not be required
; to do anything. However, under exceptional circumstances
; special code may be required to ensure that Angel starts
; cleanly and that later application specific code can have
; full control of the timer.
;
        MACRO
$label  INITTIMER       $w1,$w2
$label  ; No special timer initialisation required for the
        ; PID. However, certain targets may need to perform I/O
        ; register setup, and start the timer running.
        MEND

; ---------------------------------------------------------------------
; GETSOURCE
; ---------
; This macro is used to read the current interrupt source
; activity status for the Angel device driver interrupts.
;
; It can return:
;
;                  -1 - Ghost Interrupt (no Interrupt source active)
; DE_NUM_INT_HANDLERS - Int. source not recognised
;            IH_<xxx> - IntHandlerID (from devconf.h) of Interrupt source
;
        MACRO
$label  GETSOURCE       $re, $w1

        IF HANDLE_INTERRUPTS_ON_IRQ <> 0
$label    LDR     $w1, =IRQStatus
          LDR     $w1, [$w1]

          ; mask of upper 16 bits, and check for ghost interrupts
          MOV     $w1, $w1, LSL #16
          MOVS    $w1, $w1, LSR #16
          MVNEQ   $re, #0
          MOVNE   $re, #DE_NUM_INT_HANDLERS

          ; Now test for specific interrupts. Interrupts tested for
          ; later are given higher priority.
          IF PROFILE_SUPPORTED <> 0
            TST     $w1, #IRQ_TIMER1
            MOVNE   $re, #IH_PROFILETIMER
          ENDIF
          IF  (PARALLEL_SUPPORTED > 0)
            TST     $w1, #IRQ_PARALLEL
            MOVNE   $re, #IH_PARALLEL
          ENDIF
          IF  (PCMCIA_SUPPORTED > 0)
            TST     $w1, #IRQ_CARDA
            MOVNE   $re, #IH_PCMCIA_A

            TST     $w1, #IRQ_CARDB
            MOVNE   $re, #IH_PCMCIA_B
          ENDIF
          IF (SERIAL_INTERRUPTS_ON_FIQ = 0)
            TST     $w1, #IRQ_SERIALA
            MOVNE   $re, #IH_ST16C552_A
          ENDIF
          IF  ((ST16C552_NUM_PORTS > 1) :LOR: (LOGTERM_DEBUGGING <> 0))
            TST     $w1, #IRQ_SERIALB
            MOVNE   $re, #IH_ST16C552_B
          ENDIF

        ENDIF   ; HANDLE_INTERRUPTS_ON_IRQ

        IF HANDLE_INTERRUPTS_ON_FIQ <> 0
          ; This is hardcoding the FIQ interrupt as Serial port A if
          ; we are receving FIQ interrupts at all - this will
          ; need to be changed on a system that really uses FIQ's
          LDR     $w1, =FIQStatus
          LDR     $w1, [$w1]
          TST     $w1, #1
          MOVNE   $re, #IH_ST16C552_A
        ENDIF   ; HANDLE_INTERRUPTS_ON_FIQ <> 0

        MEND

; ---------------------------------------------------------------------

; CACHE_IBR
; ---------
; This macro implements an instruction barrier for a range of addresses
; (i.e. it makes instruction and data memory coherent for this range)
; w1 contains the start of the range , w2 the next address after the end
; Note that w1 will be corrupted

; This will do nothing except for StrongARM systems, where it should
; be implemented.
;
        MACRO
$label  CACHE_IBR       $w1,$w2
        MEND

; ---------------------------------------------------------------------

        ENDIF                   ; target_s

        OPT     (old_opt)   ; restore previous listing options

        END

; EOF target.s
