        TTL     Generic ARM start-of-day (initialisation) code     > startrom.s
        ; ---------------------------------------------------------------------
        ; This file provides the Angel initialisation and startup code
        ; for ROM systems. After performing all the necessary ARM and
        ; target initialisation the "__main" function is called to
        ; start the Angel library and application code present.
        ;
        ; $Revision: 1.14.6.4 $
        ;   $Author: dbrooke $
        ;     $Date: 1998/03/26 16:47:57 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ;
        ; Notes:
        ; To make it simpler for the developer to understand, the ROM
        ; specific and generic Angel workspace are provided as
        ; standard RW data AREAs.
        ;
        ; However, this means that care must be taken when
        ; constructing ROM based versions of Angel. In fact there are
        ; two types of ROM based Angel: boot ROMs and application
        ; ROMs. For application ROMs we do not need to worry about the
        ; size of RW data, since *NO* programs will be downloaded, and
        ; the application can use as much RAM as required. However,
        ; for boot ROMs we need to ensure that the RAM usage of the
        ; ROM system does not exceed 0x8000. This will ensure that
        ; default AIF applications can be downloaded.
        ;
        ; For non-ROM based applications the linker can be left to
        ; place data AREAs as it sees fit. For ROM based code we force
        ; the linker to reference the data at a fixed address. The
        ; only problem then becomes finding the initialised data in
        ; the ROM image, and ensuring that it is copied to the correct
        ; RAM address. This can be achieved as follows:
        ;
        ; 1) We know that the linker places identical area types
        ;    contiguously.
        ;
        ; 2) We explicitly force the RO (code areas) to be at the
        ;    start of the ROM image.
        ;
        ; 3) We can use Image$$RO$$Base and Limit to find the end of
        ;    the RO section in the ROM image. This will give us the start
        ;    location in the ROM of the RW data.
        ;
        ; 4) We can use the corresponding RW and ZI Base and Limit
        ;    values to find out how big the areas are, and initialise the
        ;    RAM accordingly.
        ;
        ; 5) Since we now know how large the RW data is, we can find
        ;    the end of the ROM image and any post-armlink information
        ;    that has been appended to the binary image (e.g. CRCs, MMU
        ;    tables, etc.).
        ;
        ; 6) We know that the linker places R/W code areas before R/W
        ;    data areas, which in turn appear before the BSS areas.
        ;
        ; We cannot use the "-last" feature of the ARM linker to place
        ; a RO area at the end, since this will break point 1) where
        ; the linker wants to make AREAs of the same type contiguous.
        ;
        ; ---------------------------------------------------------------------

        GET     listopts.s              ; standard listing control
        GET     lolevel.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support
        GET     target.s                ; target specific definitions
    
        ; ---------------------------------------------------------------------

        IMPORT  |__main|                ; Hi-level Angel init entry point
        IMPORT  |__entry|,WEAK          ; main need not exist (no application)
        IMPORT  __rt_asm_fatalerror     ; error reporting via suppasm.s
        
        IMPORT  angel_StartupStatus     ; Variable describing type of reset.

        IMPORT  angel_MMUtype           ; Variable holding ARM MMU type.

        KEEP                            ; Keep local symbols in the object.

        ; ---------------------------------------------------------------------
        ;
        ; This AREA is marked as NOINIT, to ensure that it appears as BSS:
        AREA    |AngelROM$$Data|,DATA,NOINIT
        EXPORT  angel_ROMEnd
        EXPORT  angel_GhostCount

angel_ROMWorkspaceStart %       0       ; start of this workspace section
        ;
angel_ROMEnd            %       4       ; true ROM end address
        ; When constructing the ROM, it may be necessary to append
        ; information to the image produced by the linker. This
        ; variable is initialised to the address after all of the
        ; linker placed AREAs. This allows the run-time code to find
        ; such private, appended, information. e.g. hard-wired MMU
        ; tables, ROM CRC values, etc.
        ;
angel_GhostCount        %       4       ; count of ghost interrupts
angel_ROMWorkspaceEnd   %       0       ; allows size of area to be calculated

        ; ---------------------------------------------------------------------
        ; -- ROM Startup code -------------------------------------------------
        ; ---------------------------------------------------------------------

        AREA    |ROMStartup|,CODE,PIC,READONLY

|__romBase|     ; Symbol for the first location in the ROM.

        EXPORT  |__rom|         ; NOTE: This is not an APCS function
|__rom| ; This is the only symbol exported from this area. It is the
        ; main ENTRY point into a ROM Angel world from an ARM hardware
        ; reset. It can also be used as an emergency re-start when
        ; called directly from software control.
        ;
        ; We treat the call to this code as a branch, and will *NOT*
        ; return to the caller. This code corrupts the r14 value
        ; present at the instance of the entry to this routine. NOTE:
        ; At this point we cannot rely on the state of the ARM
        ; hardware exception vectors, so the code should not generate
        ; aborts or events until handlers have been put in place.

        ; First we must get the actual start address
    IF ROMonly
        SUB     r4, lr, #4              ; 1st instruction is BL __rom
    ELSE
        ADR     r4, __rom               ; We are linked to be the 1st module
    ENDIF

        ;
        ; Ensure the processor is in SVC32 mode, with IRQs and FIQs
        ; disabled before continuing with the initialisation.


        IF :DEF: HAS_ARM600_MMU
          ; On ARM6x0 processors the MMU control register is written as
          ; zero by reset. This means little-endian, early abort, 26bit
          ; data, 26bit program, write buffer off, cache off, alignment
          ; fault off and MMU off. However Angel is a 32bit clean
          ; system, so we need to ensure the correct processor
          ; configuration.
          MRS     r14,CPSR                ; get the current processor status
          TST     r14,#(1 :SHL: 4)        ; 26- or 32-bit mode?
          ; If bit (1 :SHL: 4) is *NOT* set then we are in a 26-bit
          ; mode. This code assumes that if we are started in 26-bit
          ; mode then we *MUST* have an MMU.
          MOVEQ   r14,#DefaultMMUConfig   ; desired state
          MCREQ   MMUCP,0,r14,MMUControlReg,c0
          ; NOTE: The above has *NOT* enabled the MMU, cache or
          ; write-buffer. Until the Angel system has been fully
          ; initialised, we cannot construct any MMU tables. The default
          ; Angel world defines a straight through logical-to-physical
          ; mapping for ARM systems with MMUs. However it is up to a
          ; particular developer whether an Angel ROM system enables the
          ; cache and write-buffer as part of its startup. This code may
          ; need to change when dealing with different ARM processors.
        ENDIF

        
        MRS     r14,CPSR                ; get the processor status
        BIC     r14,r14,#ModeMask
        ORR     r14,r14,#(SVCmode :OR: InterruptMask)
        MSR     CPSR,r14                ; SVC 32 mode with interrupts disabled

        ;
        ; state: SVC mode; IRQs disabled; FIQs disabled.
        ; r14 = current PSR
        ;  r4 = PowerOnStatus [input] (zero - hard; non-zero - soft).
        ;  r5 = RAM size, or 0x00000000 if not yet known.
        ;  r0, r1 used as temporary working space within STARTUPCODE
        ;
        ;        All other registers undefined.
        ;        If present, MMU initialised to default off state.
        ;
        ; The following macro should perform any target specific I/O
        ; disabling and setup. i.e. disable all interrupt sources, set
        ; memory access timings, etc.

        STARTUPCODE     r0,r1,r4,r5

        IF :DEF: HAS_ARM600_MMU
          ; Force 32 bit address & data, little endian
        
          MOVNE   r0,#0x30        ; 32-bit address & data, little-endian
          MCRNE   MMUCP,0,r0,MMUControlReg,c0

        ENDIF        

        ;
        ; The following linker generated symbols are used to control
        ; the ROM-to-RAM copy loop:
        ;
        IMPORT  |Image$$RO$$Base|  ; start of ROM code
        IMPORT  |Image$$RO$$Limit| ; end of ROM code
        IMPORT  |Image$$RW$$Base|  ; destination address of initialised R/W
        IMPORT  |Image$$ZI$$Base|  ; start of the zero initialised (BSS) data.
        IMPORT  |Image$$RW$$Limit| ; used to size initialised R/W section

        ;
        ; These are used to copy the vector table to zero when necessary
        ;
        IMPORT  |__Vectors$$Base|
        IMPORT  |__Vectors$$Limit|

        LDR     r0, =|Image$$RO$$Base| ; get linked location of code

        ; see if we started in ROM mapped at zero or not

        MOVS    r1, r4
        LDREQ   r1, =ROMBase
        SUB     r8, r1, r0

    IF :LNOT: ROMonly
        ;
        ; Z is set if we are running in ROM that has been mapped to
        ; zero - if this is the case, then we need to start executing
        ; from the real ROM and unmap the ROM from zero
        ;
        BNE     RealROMAddress

        LDR     r2, =RealROMAddress
        ADD     pc, r2, r8
RealROMAddress

        UNMAPROM r2, r3                 ; macro to unmap ROM from 0
                                        ; r2, r3 are passed as working space
    ENDIF ; :LNOT: ROMonly

        ; r8 now contains offset from code base to current base (could be 0)
        ; r1 contains current base
        ; r0 contains linked location of code
        
        LDR     r3, =|Image$$RO$$Limit|
        TEQ     r8, #0
        BEQ     NoCodeCopy

        ; copy code
        ; r0 is dest, r0 + r8 is source, r3 is limit

01      LDR     r1, [r0, r8]
        STR     r1, [r0], #+4
        CMP     r0, r3
        BLO     %B01

NoCodeCopy

        ADD     r1, r3, r8              ; source address = RO$$Limit + offset
        LDR     r2,=|Image$$RW$$Base|   ; destination address
        LDR     r3,=|Image$$ZI$$Base|   ; limit
        ;
        ; For ROM based applications we must perform the work of
        ; initialising the RW initialised and BSS data here. For
        ; non-ROM applications the mapping (and initialisation) of RW
        ; data is performed by the AIF header, and such applications
        ; would not be using the "startrom.s" code at all.
        ;
        ; Copy the data to the final RAM address:
        
01      LDR     r0,[r1],#4              ; get from ROM
        STR     r0,[r2],#4              ; store to RAM
        CMP     r2,r3                   ; check for end condition
        BLO     %BT01                   ; if (r2 < r3) we have more to transfer
        ;
        ; Now initialise the BSS area.
        ;
        LDR     r3,=|Image$$RW$$Limit|
        MOV     r0,#0
02
        STR     r0,[r2],#4
        CMP     r2,r3
        BLO     %BT02
        
        ; NOTE: We could optimise the loop above to use more
        ; registers. This would improve the initialisation latency of
        ; ROM systems.

        ; r1 points to top of image, save this to r7 for now
        MOV     r7, r1

        ; Now we have set everything up, we can jump to (possibly)
        ; re-located code and continue running from there.
        ; The following instruction forces the PC to be loaded from
        ; a literal pool. The value loaded is the address of the
        ; gotoRelocated symbol, suitably relocated. This means when
        ; we "armlink" the object created we can specify the image
        ; address, and the code will then be correct.
        
        LDR     pc, =gotoRelocated
gotoRelocated
        
        ; We have just initialised the RAM, so we can now start using
        ; Angel variables:

        ; The value in "r7" is the true ROM end address. This can be
        ; used by ROM systems that have appended data after that
        ; constructed by the linker.

        LDR     r0,=angel_ROMEnd        ; variable to hold true ROM end address
        STR     r7,[r0,#0x00]
        ;
        ; The MMU identification word is held in r6 from above. A
        ; value of zero is used when no MMU is present.
        
        LDR     r0,=angel_MMUtype
        STR     r6,[r0,#0x00]

        ;
        ; We can support the following three system configurations:
        ;
        ; 1)  ROM at zero, (dataseg, bss) elsewhere
        ;
        ; 2)  ROM elsewhere, (dataseg, bss) at zero
        ;
        ; 3)  Vectors at zero, (ROM, dataseg, bss) elsewhere
        ;
        ; In the first two configurations, the Vectors will automatically
        ; be at zero (in (1) because the linker was told to put them there,
        ; in (2) because they are linked at the start of the "dataseg");
        ; in the third configuration we need to copy them from their
        ; current position to their proper home at zero.
        ;
        LDR     r0, =|__Vectors$$Base|
        CMP     r0, #0
        BEQ     VectorsInPlace

        ;
        ; we really do need to move the vectors
        ;
        LDR     r3, =|__Vectors$$Limit|
        SUBS    r3, r3, r0

        IF :DEF: ASSERT_ENABLED :LAND: ASSERT_ENABLED <> 0
MissingVectors
        BNE     VectorsOK
        ADR     a1, novecmsg
        B       __rt_asm_fatalerror
novecmsg
        DCB     "Missing Vectors\n"
        ALIGN
VectorsOK
        ENDIF

        MOV     r1, #0
VectorCopy
        LDR     r2, [r0], #4
        STR     r2, [r1], #4
        SUBS    r3, r3, #4
        BNE     VectorCopy

        ;
        ; We now have the processor vectors, and the indirection table
        ; at address zero. Also, depending on the target, we will have
        ; mapped RAM to zero.
        ;
        ; Initialise the default stack pointers for the various ARM
        ; processor modes. We do this by switching into all the modes
        ; (ensuring interrupts stay disabled), setting up the default
        ; stack top addresses. These addresses are hard-wired into the
        ; target workspace. Normally an O/S would perform some more
        ; memory management, and have the concept of dynamic exception
        ; stacks depending on the task/thread active. However for
        ; Angel we just have some pre-allocated stacks at fixed
        ; addresses in the work-space.
        ;

        IMPORT  Angel_StackBase
VectorsInPlace
        LDR     r5, =Angel_StackBase
        LDR     r5, [r5]
        ADD     sp, r5, #Angel_SVCStackOffset      ; SVC mode stack
        ADD     sl, r5, #Angel_SVCStackLimitOffset ; No APCS_STACKGUARD space

        ; Define a default APCS-3 register state to ensure we can call
        ; suitably conforming functions:
        MOV     fp,#0x00000000          ; no frame-pointer chain

        ; We now call the memory sizer if there is one, and adjust
        ; things if the memory size is not the default.

        IMPORT  angel_FindTopOfMemory, WEAK
        IMPORT  angel_RelocateWRTTopOfMemory, WEAK
        
        LDR     r1, =angel_FindTopOfMemory
        CMP     r1, #0
        BEQ     MemorySizeAlreadyCorrect
        BL      angel_FindTopOfMemory
        BL      angel_RelocateWRTTopOfMemory
        ; If we have just done relocations then go back and set up the SVC
        ; stack again (and repeat this).
        CMP     r0, #0
        BNE     VectorsInPlace
        ; Now everything is all ok, and set up correctly
        
MemorySizeAlreadyCorrect        
        IMPORT  angel_InitialiseOneOff   ; serialiser initialisation
        IMPORT  angelOS_InitialiseApplication
        IMPORT  angelOS_CPUWrite

        ; We have to set up the other privileged mode regs right now.
        
        MOV     r0, #(InterruptMask :OR: IRQmode)
        MSR     CPSR,r0
        ADD     sp, r5, #Angel_IRQStackOffset      ; IRQ mode stack

        MOV     r0, #(InterruptMask :OR: FIQmode)
        MSR     CPSR,r0
        ADD     sp, r5, #Angel_FIQStackOffset      ; FIQ mode stack

        MOV     r0,#(InterruptMask :OR: UNDmode)
        MSR     CPSR,r0
        ADD     sp, r5, #Angel_UNDStackOffset      ; Undefined instruction mode stack

        MOV     r0,#(InterruptMask :OR: SVCmode)
        MSR     CPSR,r0

        ; We are now back in SVC mode with IRQs and FIQs disabled.

        INITMMU         r0,r1,r2,r3,r5,r6  ; perform MMU initialisation if required
        ;
        INITTIMER       r0,r1  ; perform any special timer initialisation
        ;
        ; For systems that cannot tell the type of reset that
        ; occurred, the STARTUPCODE macro will have set "r4" to
        ; pretend that it is a PowerOnReset.
        LDR     r0,=angel_StartupStatus
        STR     r4,[r0,#0x00]                   ; store PowerOnReset status

        ; NOTE: At this point we are still in SVC mode, using the
        ; SVC stack. Processor interrupts are *STILL* disabled.

  IF (:LNOT: :DEF: MINIMAL_ANGEL) :LOR: MINIMAL_ANGEL = 0
        ; initialise the logterm debugging module, if present.
        
        ;; Note: it probably doesn't make sense in minimal angel anyway;
        ;; and certainly would need small changes to work there...
    IF DEBUG = 1 :LAND: LOGTERM_DEBUGGING = 1
        IMPORT  |logterm_Initialise|,WEAK   ; debug initialisation
        BL      logterm_Initialise
    ENDIF

        ;; Initialise the serialiser
        BL      angel_InitialiseOneOff          ; init serialiser world

  ENDIF

  IF :DEF: JTAG_ADP_SUPPORTED :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
  ELSE
        
    IF :DEF: ICEMAN_LEVEL_3
        ; set up iceman and library as the application
        IMPORT  Iceman_InitialiseApplication

        LDR     r0, =__entry    ; Application entry point
        BL      Iceman_InitialiseApplication
        
    ELSE
        ; Now we have to set up the application registers so that
        ; the world is ready just for the user to type 'go' from armsd.
                        
        ; angelOS_InitialiseApplication sets up some stuff ...
        LDR     r0, =ADP_HandleUnknown
        MOV     r1, r0
        BL      angelOS_InitialiseApplication

        ; Set up arguments to angelOS_CPUWrite to set PC and CPSR

        ADR     r0, regsetuparea1

    [ {ENDIAN} = "big"
        ;
        ; 960603 KWelton
        ;
        ; This call to angelOS_CPUWrite is mimicking the data received
        ; in an ADP packet which is always sent little-endian; we need
        ; to swap the order of the words from the native big-endian
        ; before making the call
        ;
        SUB     sp, sp, #8
        MOV     r1, sp
        MOV     r2, #2          ; XXX We are angelOS_CPUWrite'ing 2 words

swapit  MOV     lr, #0
        LDR     r3, [r0], #4
        STRB    r3, [r1], #4
        MOV     r3, r3, LSR #8
        STRB    r3, [r1, #-3]
        MOV     r3, r3, LSR #8
        STRB    r3, [r1, #-2]
        MOV     r3, r3, LSR #8
        STRB    r3, [r1, #-1]
        SUBS    r2, r2, #1
        BNE     swapit
        MOV     r0, sp
    ]

        STMFD   sp!, {r0}
        LDR     r0, =ADP_HandleUnknown
        MOV     r1, r0
        LDR     r2, =ADP_CPUmode_Current
        LDR     r3, =ADP_CPUread_PCmode + ADP_CPUread_CPSR
        BL      angelOS_CPUWrite

    ENDIF ; :DEF: ICEMAN_LEVEL_3

  ENDIF   ; NOT :DEF: JTAG_ADP_SUPPORTED :LOR: :DEF: MINIMAL_ANGEL

        ; XXX
        ;
        ; We *should* restore sp here after passing args to angel_OSCPUWrite,
        ; but we don't bother as the first thing __main does is reset sp.
                        
        B       |__main|       ; start higher-level Angel initialisation


regsetuparea1
        DCD     __entry, USRmode

        LTORG

        ; ---------------------------------------------------------------------
        ; The following code fragment is the simple undefined
        ; instruction handler we use when checking for the presence of
        ; a MMU co-processor:
DummyMMUCode
        MOV     r6,#0x00            ; no MMU present if we have aborted
        MOVS    pc,lr               ; return to just after faulting instruction

        ; ---------------------------------------------------------------------

        ADR     a1, fatalmsg
        B       __rt_asm_fatalerror
fatalmsg
        DCB     "Fatal Error\n"
        ALIGN

        ; ---------------------------------------------------------------------

        LTORG

        ; ---------------------------------------------------------------------
        END     ; EOF startrom.s


