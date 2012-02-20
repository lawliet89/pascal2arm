        SUBT    PIE board description and Angel support         > pie.s
        ; ---------------------------------------------------------------------
        ; This file defines the PIE manifests and macros required
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
        ;
        ; $Revision: 1.9 $
        ;   $Author: hmeeking $
        ;     $Date: 1996/08/20 08:54:35 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ;
        ; ---------------------------------------------------------------------

        ASSERT  (listopts_s)
old_opt SETA    {OPT}
        OPT     (opt_off)   ; disable listing of include files

        ; ---------------------------------------------------------------------

        [       (:LNOT: :DEF: pie_s)

                GBLL    pie_s
pie_s       SETL    {TRUE}

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

        ; In theory on any PIE card the instruction
        ;STR     $pos,[$pos,-$pos]
        ; should ensure RAM mapped to zero on the PIE. However because of the
        ; timing of the signals on the ARM60, an ARM60 PIE will not switch. It
        ; needs a STMIA with at least two registers to do the switch.
        ; So we do the following:
        MOV     $w1,#0
        MOV     $w2,#0
        STMIA   $w1,{$w1,$w2}
        MEND

        ; STARTUPCODE
        ; -----------
        ; Provide code to deal with ensuring that the
        ; target I/O world is suitably clean before attempting to
        ; initialise the rest of Angel.
        ;
        MACRO
$label  STARTUPCODE     $w1, $w2, $pos, $ramsize

        ;; leave $pos alone - already set - unless we have good reason
        ;; to change it.  Here we don't.

        MOV     $ramsize,#0x00000000    ; RAM size not yet known

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
        ; PIE. However, certain targets may need to perform I/O
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
        ;                  -1 - Ghost Interrupt (no interrupt source active)
        ; DE_NUM_INT_HANDLERS - Interrupt source not recognised
        ;            IH_<xxx> - IntHandlerID (from devconf.h) of Int. source
        ;

        MACRO
$label  GETSOURCE       $w1,$w2
$label  MOV     $w1,#DE_NUM_INT_HANDLERS ; Initally unrecognised interrupt

        ; Now test for specific interrupts. Interrupts tested for
        ; later are given higher priority.

        IF (PARALLEL_SUPPORTED > 0)
          MOV   $w2,#PIObase            ; Reference the PIO
          LDR   $w2,[$w2,#PIO_RSR]      ; Load PIO Rx status
          TST   $w2,#RxDataMask         ; Check for Rx Data
          MOVNE $w1,#IH_PARALLEL        ; Set index accordingly
          BNE   $label.1                ; continue immediately
        ENDIF

        MOV     $w2,#SerialChipBase     ; Reference the I/O hardware
        LDR     $w2,[$w2,#SER_ISR]      ; Load the current Int status
 IF :DEF: PROFILE_SUPPORTED
   IF PROFILE_SUPPORTED <> 0
        TST     $w2,#IMRTimer
        MOVNE   $w1,#IH_TIMER
        BNE     $label.1
   ENDIF
 ENDIF
        TST     $w2,#SerialIntSource    ; Check for serial source
        MOVNE   $w1,#IH_SERIAL          ; Set index accordingly if active

$label.1
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

        ]       ; pie_s

        OPT     (old_opt)   ; restore previous listing options

        ; ---------------------------------------------------------------------
        END     ; EOF pie.s
