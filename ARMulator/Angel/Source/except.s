        TTL     Angel exception support                 > except.s
        ;
        ; This file provides the default Angel exception vector handlers.
        ;
        ; $Revision: 1.16.4.4 $
        ;   $Author: dbrooke $
        ;     $Date: 1998/02/26 17:35:12 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ;

        KEEP

        GET     listopts.s           ; standard listing control
        GET     lolevel.s            ; automatically built manifest definitions
        GET     macros.s             ; standard assembler support
        GET     target.s             ; target specific manifests


        EXPORT  __VectorStart
        EXPORT  __SoftVectors
        IMPORT  angel_DeviceInterruptHandler

        ;
        ; Default ARM hardware exception vectors
        ;
        ; When building ROM at zero systems, the link should be modified
        ; to force this area to be first.
        IF      ROMonly
          AREA    |__Vectors|,CODE,PIC,READONLY
        ELSE
          ; Masquerade as ReadWrite Code so that this AREA gets placed first
          ; in the Read-Write segment by the linker, and has two underscores
          ; to help it get sorted first in case anyone has any real RW Code !
          AREA    |__Vectors|,CODE,PIC,READWRITE
        ENDIF
        ;
        ; After initialisation the following vectors *MUST* start at
        ; 0x00000000 (an ARM processor requirement).
        ;
        ; The following vectors load the address of the relevent
        ; exception handler from an address table.  This makes it
        ; possible for these handlers to be anywhere in the address space.
        ;
        ; To bootstrap the ARM, some form of ROM is present at
        ; 0x00000000 during reset.  Often the startup code performs
        ; some target specific magic to remap RAM to address
        ; zero. The ROM based Read/Write data and BSS must then be
        ; copied to the relevant RAM address during the Angel ROM
        ; initialisation.
        ;
__VectorStart                   ; Start of ARM processor vectors
        LDR     pc,ResetV       ; 00 - Reset
        LDR     pc,UndefV       ; 04 - Undefined instructions
        LDR     pc,SWIV         ; 08 - SWI instructions
        LDR     pc,PAbortV      ; 0C - Instruction fetch aborts
        LDR     pc,DAbortV      ; 10 - Data access aborts
        LDR     pc,UnusedV      ; 14 - Reserved (was address exception)
        LDR     pc,IRQV         ; 18 - IRQ interrupts
        LDR     pc,FIQV         ; 1C - FIQ interrupts
        ;
        ; NOTE: In a normal optimised ARM system the FIQ vector would
        ; not contain a branch to handler code, but would have an
        ; allocation immediately following address 0x1C, with the FIQ
        ; code being placed directly after the vector table. This
        ; avoids the pipe-line breaks associated with indirecting to a
        ; handler routine.

        ; However Angel is designed to be a simple system so we
        ; treat FIQ like all the other vectors, and this allows the
        ; actual handler addresses to be stored immediately after the
        ; ARM vectors. If optimal FIQ entry is required, then space
        ; could be allocated at this point to hold the direct FIQ
        ; code. The __SoftVectors table would then simply appear
        ; higher in the RAM allocation.
        ;
__SoftVectors
        ; Reset - an error unless ROM always at zero, or a branch
        ; to High ROM on reset has been requested explicitly (Cogent Board)

ResetV  
  IF ROMonly
        IMPORT  __rom
        DCD     __rom           ; 00 - Reset
  ELSE
    IF BRANCH_TO_HIGH_ROM_ON_RESET <> 0
        DCD     ROMBase
    ELSE
        DCD     DefaultExceptionTable + 0x0
    ENDIF
  ENDIF
        
  IF :DEF: ICEMAN_LEVEL_3 :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
UndefV  DCD     DefaultExceptionTable + 0x4
  ELSE
UndefV  DCD     HandlerUndef
  ENDIF
SWIV    DCD     HandlerSWI
PAbortV DCD     DefaultExceptionTable + 0xC
DAbortV DCD     DefaultExceptionTable + 0x10
UnusedV DCD     DefaultExceptionTable + 0x14
  IF HANDLE_INTERRUPTS_ON_IRQ <> 0
IRQV    DCD     angel_DeviceInterruptHandler
  ELSE
IRQV    DCD     DefaultExceptionTable + 0x18
  ENDIF
  IF HANDLE_INTERRUPTS_ON_FIQ <> 0
FIQV    DCD     angel_DeviceInterruptHandler
  ELSE
FIQV    DCD     DefaultExceptionTable + 0x1C
  ENDIF

        
__VectorEnd             ; End of Angel code copied to zero


        AREA    |ExceptionInit|,CODE,PIC,READONLY

        IMPORT  angelOS_ThreadStopped
        IMPORT  angel_ApplDeviceHandler
        IMPORT  angel_LateBootInit

  IF :LNOT: ROMonly     ;This is removed for FLASH at 0 systems where a 
        ;; write to the flash may cause reprogramming - in the case of the
        ;; ATMEL devices on the PID. If ROM is at 0 there is no need to
        ;; initialise the vectors as they are hard coded.
        
        ; This function performs the exception system initialisation:
        
        EXPORT angel_ExceptionInit
angel_ExceptionInit
        ; in:   no arguments
        ; out:  no result
        ;
        ; Here we copy the vector table (__VectorStart to __VectorEnd)
        ; to address 0.
        ; This is clearly necessary if we have a RAM based system, and
        ; is clearly pointless if we have a ROM at 0 based system.

        LDR     a1,=__VectorStart       ; start of data
        MOV     a2,#0x00000000          ; destination address
        LDR     a3,=__VectorEnd         ; end of data
01      LDR     a4,[a1],#4              ; get word from data AREA
        STR     a4,[a2],#4              ; store to RAM at zero
        CMP     a1,a3                   ; check for end condition
        BLO     %BT01                   ; if (a2 < a3) we have more to transfer
        ;
        MOV     pc,lr

  ENDIF ;:LNOT: ROMonly 
        
        ; *****************************************************************

  IF (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)

        AREA    |MinimalRegBlock|,DATA,NOINIT

        EXPORT  Angel_MutexSharedTempRegBlocks

Angel_MutexSharedTempRegBlocks
        %       (2 + 16) * 4

  ELSE
    IF (:DEF: ICEMAN_LEVEL_3)
        IMPORT  Angel_MutexSharedTempRegBlocks
    ENDIF
  ENDIF
        
        ; Exception handlers

        AREA    |DefaultVectorHandlers|,CODE,PIC,READONLY

        ; This is the ARM exception interface to the Angel debug world.
        ;
        ; This code provides the default ARM vector handlers for the
        ; processor exceptions. If the target application (or the
        ; more likely an O/S) does provide vector handlers,
        ; then they should be coded to provide the VectorCatch
        ; checking code performed below, to ensure that a debug agent
        ; can stop on system events. However, this is very system
        ; specific.
        ;
        ; By default the Angel debug world makes use of the
        ; Undefined Instruction exception, SWI's and eother IRQ's or
        ; FIQ's depending on which the board uses.  All of the
        ; other exceptions are unused by Angel, and the default action
        ; is purely to raise a debug event.
        ;
        ; The model used by the serialisation module is discussed
        ; in detail elsewhere.  This module follows the rules and
        ; guidelines laid out by the serialiser.

        ; in: UND mode; IRQs disabled; FIQs undefined
        ;     r13 = FD stack
        ;     r14 = address of undefined instruction + 4
        ;     All other registers must be preserved

  IF :DEF: ICEMAN_LEVEL_3 :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
        ; Do not include an Undef Handler for ICEMan
  ELSE  
        IMPORT Angel_MutexSharedTempRegBlocks
        EXPORT HandlerUndef 

HandlerUndef
        STMFD sp!, {r0}

        ; Disable interrupts
        MRS   r0, CPSR
        IF (FIQ_SAFETYLEVEL >= \
            FIQ_NeverUsesSerialiser_DoesNotReschedule_HasNoBreakpoints)
          ORR r0, r0, #IRQDisable
        ELSE
          ORR r0, r0, #IRQDisable + FIQDisable
        ENDIF
        MSR   CPSR, r0

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
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          TST   r1, #Tbit
          SUBEQ r14, r14, #4      ; Adjust for ARM instruction
          SUBNE r14, r14, #2      ; Adjust for Thumb instruction
        ELSE
          SUB   r14, r14, #4      ; Adjust to point to the undef
        ENDIF
        STR   r14, [r0, #Angel_RegBlock_R0offset + (15*4)] ; pc

        ; Accessing other modes depends on the mode
        AND   r1, r1, #ModeMask
        CMP   r1, #USRmode
        BEQ   UndefWasInUSRMode
        CMP   r1, #SYSmode
        BEQ   UndefWasInUSRMode

        ; Deal with non USR mode case
        IF (FIQ_SAFETYLEVEL >= \
            FIQ_NeverUsesSerialiser_DoesNotReschedule_HasNoBreakpoints)
          ORR r1, r1, #IRQDisable
        ELSE
          ORR r1, r1, #IRQDisable + FIQDisable
        ENDIF
        MSR   CPSR, r1  

        ; Now we are in the appropriate mode
        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        STMIA r0, {r8-r14}      ; r8-r12 may be banked - now saved
        SUB   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        MRS   r2, SPSR
        STR   r2, [r0, #Angel_RegBlock_SPSRoffset]

        ; Get back into UND
        BIC   r1, r1, #ModeMask
        ORR   r1, r1, #UNDmode
        MSR   CPSR, r1
        B     RegsNowSaved

UndefWasInUSRMode
        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        STMIA r0, {r8-r12}      ; r8-r12 are not banked between USR and UND
        ADD   r0, r0, #(13*4) - (8*4)
      IF :DEF: STRONGARM_REV1OR2
        MCR   MMUCP, 0, r0, c0, c0, 0
      ENDIF
        STMIA r0, {r13,r14}^    ; Store out the USR r13 and r14
        SUB   r0, r0, #Angel_RegBlock_R0offset + (13*4)

RegsNowSaved
        ; We have to check that the undefined instruction we hit
        ; was indeed the Angel Undefined instruction
        ;
        ; Clearly this uses different code for Thumb and ARM states
        ; but make the Thumb support code removable
        ;
        ; Note that r14_und has been adjusted to point to the undef
        ; (ARM or Thumb).

        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          MRS   r0, SPSR
          TST   r0, #Tbit
          BEQ   CompareWithARMBreakPoint   
        
          LDRH  r0, [r14]       ; load the Thumb instruction
          MOV   r1, #angel_BreakPointInstruction_THUMB :AND: 0xff00
          ORR   r1, r1, #angel_BreakPointInstruction_THUMB :AND: 0xff
          CMP   r0, r1
          BNE   UnrecognisedInstruction
          BEQ   NormalBreakpoint ; Skip over the check for Vector Hit case
        ENDIF

CompareWithARMBreakPoint
        LDR     r0, [r14]        ; load the ARM instruction
        LDR     r1, =angel_BreakPointInstruction_ARM
        CMP     r0, r1
        BNE     UnrecognisedInstruction
        
        ; We now know it is Angel's Undefined instruction

        ; r0 = r1 = The undefined instruction
        ; lr = address of the undefined instruction
        ; spsr = unchanged
        ; sp  = FD UND stack (currently empty)

BreakPointInstruction
        ; We must check to see if it is a special case - a breakpoint
        ; in DefaultExceptionTable, which actually means it is an
        ; exception (there are no real handlers for the other exceptions).
        ;
        ; This little piece of code only every has to deal with ARM undefs

        LDR     r0, =DefaultExceptionTable
        SUB     r1, r14, r0     ; r1 = addr of Breakpoint - start of table
        MOV     r1, r1, ASR #2  ; r1 is now the index into DefaultExceptionTable
        CMP     r1, #7
        BGT     NormalBreakpoint ; Not in this table
        CMP     r1, #0
        BLT     NormalBreakpoint ; Not in this table
        B       VectorHit

NormalBreakpoint
        ; This is executed for both an ARM and Thumb breakpoint
        LDR    r2, =ADP_Stopped_BreakPoint

CallSerialiseTask
        ; Now prepare to call Angel_SerialiseTask
        ; r0 and r2 are set up by caller
        ;
        ; SerialiseTask expects:
        ; r0 = called_by_yield        ; r1 = fn
        ; r2 = type (already set up)  ; r3 = empty_stack
        MOV   r0, #0
        LDR   r1, =angelOS_ThreadStopped
        MOV   r3, sp
        B     Angel_SerialiseTask
        ; and angelOS_ThreadStopped will execute when SerialiseTask
        ; allows it to


UnrecognisedInstruction
        LDR   r2, =ADP_Stopped_UndefinedInstr
        B     CallSerialiseTask

VectorHit
        ; r1 is the index into the DefaultExceptionTable.
        ; sp = UND sp - currently empty
        ;
        ; If this is a data abort, and Angel was in the process of
        ; performing a data access, which aborted then we must
        ; unwind the abort and return to that code, setting a flag
        ; so it knows what has happened.
        ;
        ; In all other cases just report the problem to the debugger.

        CMP   r1, #4            ; Data Abort vector
        BNE   ReportToDebugger

        IMPORT  memory_is_being_accessed
        IMPORT  memory_access_aborted
        IMPORT  angel_StartTask_NonSVCEntry
        
        LDR   r0, =memory_is_being_accessed
        LDR   r0, [r0]
        CMP   r0, #0
        BEQ   ReportToDebugger  ; Not due to Angel Aborting!

        ; Now we know that it is either angelOS_MemRead or angelOS_MemWrite
        ; that has aborted.  These both run in User32 mode, and if aborted
        ; then we went into Abort32 mode, storing the real pc, cpsr
        ; in the abort regs.  This will in turn have gone through the
        ; table of undefined instructions and hence got into under32 mode,
        ; which is where we are now.  Thus we have to read the Abort regs
        ; and put them into the saved register block and then restart
        ; using that block.

        ; Indicate an abort has happened
        LDR   r0, =memory_access_aborted
        MOV   r1, #1
        STR   r1, [r0]
        
        ; Get into abort mode
        MRS   r1, CPSR
        BIC   r1, r1, #ModeMask
        ORR   r1, r1, #ABTmode
        MSR   CPSR, r1

        LDR   r0, =Angel_MutexSharedTempRegBlocks

        ; We have to get r13 and r14 from the USR bank !
        ADD   r1, r0, #Angel_RegBlock_R0offset + (4*13)
      IF :DEF: STRONGARM_REV1OR2
        MCR   MMUCP, 0, r0, c0, c0, 0
      ENDIF
        STMIA r1, {r13,r14}^    ; USR bank version
        
        ; Put SPSR, and pc into the saved registers block
        ; Adjust PC by 4 - This relies on MemRead / MemWrite being written
        ; in ARM code not Thumb code.
        SUB   r2, r14, #4 
        STR   r2, [r0, #Angel_RegBlock_R0offset + (4*15)] ; original pc
        MRS   r2, SPSR
        STR   r2, [r0, #Angel_RegBlock_CPSRoffset] ; original cpsr

        B     angel_StartTask_NonSVCEntry

                
ReportToDebugger        
        LDR   r2, =ADP_Stopped_BranchThroughZero
        ADD   r2, r2, r1
        B     CallSerialiseTask

  ENDIF ; End of ICEMAN_LEVEL_3


        IMPORT Angel_SerialiseTask
        

                
DefaultExceptionTable
        DCD     angel_BreakPointInstruction_ARM ; 00 - Reset
        DCD     angel_BreakPointInstruction_ARM ; 04 - Undefined instructions
        DCD     angel_BreakPointInstruction_ARM ; 08 - SWI instructions
        DCD     angel_BreakPointInstruction_ARM ; 0C - Instruction fetch aborts
        DCD     angel_BreakPointInstruction_ARM ; 10 - Data access aborts
        DCD     angel_BreakPointInstruction_ARM ; 14 - Reserved
        DCD     angel_BreakPointInstruction_ARM ; 18 - IRQ interrupts
        DCD     angel_BreakPointInstruction_ARM ; 1C - FIQ interrupts

        ; *************************************************************

        ; See discussion above HandlerUndef for limitations which also
        ; apply to this handler.

        ; in: SVC mode; IRQs disabled; FIQs undefined
        ;     r13 = FD stack
        ;     r14 = address of SWI instruction + 4
        ;     All other registers must be preserved
        
        IMPORT  angelOS_SemiHostingEnabled
        
        EXPORT HandlerSWI
HandlerSWI
        STMFD sp!, {r0}

        ; Disable interrupts
        MRS   r0, CPSR
        IF (FIQ_SAFETYLEVEL >= FIQ_NeverUsesSerialiser_DoesNotReschedule)
          ORR r0, r0, #IRQDisable
        ELSE
          ORR r0, r0, #IRQDisable + FIQDisable
        ENDIF
        MSR   CPSR, r0
        NOP
        
        ; Save the state of the callee (can be any mode) into a regblock
        ; This means r0-r15, and cpsr 

        ; Can do r0-r7, pc, cpsr for all modes
        LDR   r0, =Angel_MutexSharedTempRegBlocks
        ADD   r0, r0, #Angel_RegBlock_R0offset + (1*4)
        STMIA r0, {r1-r7}       ; r1-r7 are never banked
        SUB   r0, r0, #Angel_RegBlock_R0offset + (1*4)
        LDMFD sp!, {r1}
        STR   r1, [r0, #Angel_RegBlock_R0offset] ; original r0
        MRS   r1, SPSR
        STR   r1, [r0, #Angel_RegBlock_CPSRoffset] ; original cpsr
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          TST   r1, #Tbit
          SUBEQ r14, r14, #4      ; Adjust for ARM instruction
          SUBNE r14, r14, #2      ; Adjust for Thumb instruction
        ELSE
          SUB   r14, r14, #4      ; Adjust to point to the SWI
        ENDIF
        STR   r14, [r0, #Angel_RegBlock_R0offset + (15*4)] ; pc

        ; Accessing other modes depends on the mode
        AND   r1, r1, #ModeMask
        CMP   r1, #USRmode
        BEQ   SWIWasInUSRMode
        CMP   r1, #SYSmode
        BEQ   SWIWasInUSRMode

        ; Deal with non USR mode case
        IF (FIQ_SAFETYLEVEL >= \
            FIQ_NeverUsesSerialiser_DoesNotReschedule_HasNoBreakpoints)
          ORR r1, r1, #IRQDisable
        ELSE
          ORR r1, r1, #IRQDisable + FIQDisable
        ENDIF
        MSR   CPSR, r1  

        ; Now we are in the appropriate mode
        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        STMIA r0, {r8-r14}      ; r8-r12 may be banked - now saved
        SUB   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        MRS   r2, SPSR
        STR   r2, [r0, #Angel_RegBlock_SPSRoffset]

        ; Get back into SVC
        BIC   r1, r1, #ModeMask
        ORR   r1, r1, #SVCmode
        MSR   CPSR, r1
        B     RegsNowSaved2

SWIWasInUSRMode
        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        STMIA r0, {r8-r12}      ; r8-r12 are not banked between USR and SVC
        ADD   r0, r0, #(13*4) - (8*4)
      IF :DEF: STRONGARM_REV1OR2
        MCR   MMUCP, 0, r0, c0, c0, 0
      ENDIF
        STMIA r0, {r13,r14}^    ; Store out the USR r13 and r14
        SUB   r0, r0, #Angel_RegBlock_R0offset + (13*4)

RegsNowSaved2
        ; r0 = &Angel_MutexSharedTempRegBlocks[0]
        ; r1,r2, = corrupt
        ; r3-r12 unmodified
        ; r13 = SVC stack
        ; r14 = adjusted to point to SWI

        ; We have to check that the SWI we hit was indeed the Angel SWI
        ;
        ; Clearly this uses different code for Thumb and ARM states
        ; but make the Thumb support code removable

        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          MRS   r2, SPSR
          TST   r2, #Tbit
          BEQ   CompareWithARMSWI
        
          LDRH  r2, [r14]       ; load the Thumb instruction
          MOV   r1, #angel_SWI_THUMB :AND: 0xff00
          ORR   r1, r1, #angel_SWI_THUMB :AND: 0xff
          CMP   r2, r1
          BNE   UnrecognisedSWI
          BEQ   AngelSWI
        ENDIF

CompareWithARMSWI
        LDR     r2, [r14]        ; load the ARM instruction
        BIC     r2, r2, #0xFF000000 ; Only compare the SWI numbers
        LDR     r1, =angel_SWI_ARM
        BIC     r1, r1, #0xFF000000 ; Only compare the SWI numbers
        CMP     r2, r1
        BNE     UnrecognisedSWI
        
AngelSWI
        ; We now know it is our SWI, so we have to interpret the reason code
        ; which was in r0.
        ;
        ; Here we assume that r0 still addresses regblock
        ;
        LDR     r1, [r0, #Angel_RegBlock_R0offset] ; original r0

  IF (:LNOT: :DEF: MINIMAL_ANGEL) :LOR: MINIMAL_ANGEL = 0

        ; Only recognise the C Library SWIs if Semihosting is enabled
        LDR     r2, =angelOS_SemiHostingEnabled
        LDR     r2, [r2]
        CMP     r2, #0

  IF :DEF: ICEMAN_LEVEL_3
        ; We quietly ignore CLib codes, so first carry on as if semihosting...
  ELSE
        BEQ     NonCLibReasonCode
  ENDIF
        
        ; Now we know semihosting is switched on
        LDR     r2, =angel_SWIreason_CLibBase
        CMP     r1, r2
        BCC     NonCLibReasonCode     ; orig r0 < angel_SWIreason_CLibBase
        LDR     r2, =angel_SWIreason_CLibLimit
        CMP     r1, r2
        BHI     NonCLibReasonCode     ; orig r0 > angel_SWIreason_CLibLimit

  IF :DEF: ICEMAN_LEVEL_3
        ; ..and now we know it is a CLib code, return immediately
        MOV     r0, #1          ; r0 = 1 as a dummy return code
        ADD     r14, r14, #4    ; readjust r14, iceman is never in Thumb mode
        MOVS    pc, r14
  ELSE
        ; carry on with CLib processing...
    
        
        ; Now we know it is a CLib SWI
        IMPORT  SysLibraryHandler
SysLibraryCall
        LDR     r2, =SysLibraryHandler
        ; Fall through to DoIndirectCall ...

  ENDIF

  ELSE                          ; minimal angel
        B       NonCLibReasonCode
  ENDIF
        
        ; We expect the SVC stack to be empty
        ; r2 contains the address of the function to be called indirectly
        ;
        ; We have to leave the SVC stack empty and also finish with
        ; regblock before switching mode, as this will (almost definitely)
        ; enable interrupts which can blow away the SVC stack.
        ; However, we can put stuff on the callers stack temporarily!
        ;
        ; So we put the caller's CPSR, caller's r1-r3, caller's lr
        ; and the address of the instruction after the SWI on the
        ; application stack.
        ; We can then restore all the potentially unbanked registers
        ; and execute the indirectly called function
DoIndirectCall
        IMPORT Angel_StackBase
        
        ; Set up caller's stack
        LDR     r3, [r0, #Angel_RegBlock_R0offset + (13*4)]
        MRS     r6, SPSR        ; original CPSR
        ADD     r0, r0, #Angel_RegBlock_R0offset + (1*4)
        LDMIA   r0, {r7-r9}     ; original r1-r3
        SUB     r0, r0, #Angel_RegBlock_R0offset + (1*4)
        LDR     r10, [r0, #Angel_RegBlock_R0offset + (14*4)] ; caller's lr
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          TST   r6, #Tbit
          ADDEQ r11, r14, #4      ; RE-Adjust for ARM instruction
          ADDNE r11, r14, #2      ; RE-Adjust for Thumb instruction
        ELSE
          ADD   r11, r14, #4      ; RE-Adjust to point after SWI
        ENDIF
        STMFD   r3!, {r6-r11}     ; Put them on callers Stack

      IF :DEF: STRONGARM_REV1OR2
        MCR     MMUCP, 0, r0, c0, c0, 0
      ENDIF
        STMFD   r3, {r13,r14}^    ; Put USR r13 and r14 on Stack
        MOV     r6, #USRmode      ; We call the fn in USR mode (32 bit code)
        SUB     r3, r3, #8
        MSR     SPSR, r6
                
        ; Set up USR r13 and r14 for the indirectly called function
        STR     r3, [r0, #Angel_RegBlock_R0offset + (13*4)] ; callers r13
        LDR     r3, =ApplicationModeIndirectReturnVeneer
        STR     r3, [r0, #Angel_RegBlock_R0offset + (14*4)] ; callers r14
        ADD     r0, r0, #Angel_RegBlock_R0offset + (13*4)
        
        LDMIA   r0, {r13,r14}^  ; Load the USR r13 and r14 (!!!)
        SUB     r0, r0, #(13*4) ; wind back to r0

        ; Force the SVC stack to use the Angel SVC stack, no matter
        ; what it was before.
        LDR     r13, =Angel_StackBase
        LDR     r13, [r13]
        ADD     r13, r13, #Angel_SVCStackOffset
        
        ; The problem here is that all angel SWI C code is compiled as
        ; swst so it needs a valid r10 as a stack limit check.
        ; If someone has used r10 as a general purpose register they will
        ; have destroyed this functionality and will cause Angel to fail
        ; We must put a valid sl in r10 at this point
        ; The first thing we must do is take a local copy of the user r10
        ; from the stored copy of the user stack (in r0)

        LDR     r10, [r0, #(10*4)] ; r10 from the stack
        ADR     r11, SavedR10   ; get the address of the local variable
        STR     r10, [r11]      ; and store it there
        
        ; Restore r0-r12 (unbanked regs) - keeping fn to call in r14
        MOV     r14, r2
        LDMIA   r0, {r0-r12}

        STMFD   sp, {r13}^      ; bring in the User Stack pointer
        LDR     r10, [sp,#-4]
        SUB     r10, r10, #Angel_SVCStackSize ; put the limit at the size

        ; We now have a valid sl for Angel to use - The user info is
        ; reinstated in ReturnSVC
        
        ; And leap into the indirectly called routine in USR mode
        MOVS    pc, lr

       
        ; This gets called when the indirectly called fn returns
        ; we will have eight values on the stack (nearest first): 
        ;  * r13,r14 for USR Mode prior to the indirectly called
        ;    fn.  This is only needed if the CPSR at the time
        ;    of the SWI was NOT USR.
        ;  * CPSR at time of SWI
        ;  * r1-r3 values to reinstate
        ;  * LR value to reinstate
        ;  * address of instruction after the SWI (return there!)
        ; This veneer has to return to either ARM or Thumb code
        ; depending on what the CPSR on the stack indicates.  It also
        ; needs to be able to return to USR or SVC mode code, but is
        ; entered always in USR mode.
        ; Note that we want to restore the PSR flags on return too.
ApplicationModeIndirectReturnVeneer   
        ; See if SWI was not from USR mode
        ; If so restore USR mode r13, r14 after getting into proper mode
        LDR   r1, [sp, #8]      ; Get saved CPSR
        AND   r1, r1, #ModeMask
        CMP   r1, #USRmode
        CMPNE r1, #SYSmode
        ADDEQ sp, sp, #8
        BEQ   AnyModeReturn

        ; Non USR mode case - we need to gain privileged mode again
        LDR   r1, [sp, #8]      ; Get saved CPSR
        MOV   r2, r0
        MOV   r3, sp
        LDR   r0, =angel_SWIreason_EnterSVC
        DCD   angel_SWI_ARM

        ;; Response to MLS 1965 to ensure that thumb SWI calls return
        ;; correctly when not called from user mode
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT <> 0
          BIC   r1, r1, #Tbit   ; Ensure we don't switch in Thumb Mode
        ENDIF
        
        MSR   CPSR, r1          ; Switch into desired mode
        MOV   r0, r2
        MOV   sp, r3            ; Restore stack
        LDMFD sp, {r13, r14}^   ; Restore USR mode r13 and r14
        NOP
        ADD   sp, sp, #8

AnyModeReturn
        LDR     r14, [sp, #0]    ; Get saved CPSR
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          TST   r14, #Tbit
        ENDIF

        ADD     sp, sp, #4      ; Finished with the cpsr on the stack
        LDMFD   sp!, {r1-r3}    ; Restore old r1-r3

        ; Here we must retain the applications r10 if sl wasn't being used
        ; so we must get it back from the saved location
        
        ADR     r10, SavedR10   ; we must reinstate the app r10
        LDR     r10, [r10]      ; get it from the saved location
        
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          BEQ   ARMCodeReturn
          MSR   cpsr_f, lr      ; Restore flags
          LDMFD sp!, {lr}       ; Restore old lr
          STMFD sp!, {r0}       ; Need a work register (must be r0-r7 - not r14)
          ADR   r0, SwitchToThumbLabel+1
          BX    r0
SwitchToThumbLabel
          CODE16
          POP   {r0, pc}
          CODE32
          ALIGN
        ENDIF

ARMCodeReturn        
        MSR   cpsr_f, lr        ; Restore flags
        LDMFD   sp!, {lr, pc}   ; Return to ARM code
        
        
        ; **********************************************************

        ; Deal with cases other than the semi-hosted CLib SWI
        ; r0 = our regblock with the saved regs stored
        ; r1 = the original r0 when we get here
NonCLibReasonCode
        LDR     r2, =angel_SWIreason_EnterSVC
        CMP     r1, r2
        BEQ     ReturnSVC

        IF (:LNOT: :DEF: MINIMAL_ANGEL) :LOR: MINIMAL_ANGEL = 0
          IF :LNOT: (:DEF: JTAG_ADP_SUPPORTED :LOR: :DEF: ICEMAN_LEVEL_3)

            LDR     r2, =angel_SWIreason_ApplDevice
            CMP     r1, r2
            BEQ     ApplDeviceSWI

          ENDIF
        ENDIF

        IF :DEF: LATE_STARTUP :LAND: LATE_STARTUP <> 0

          LDR     r2, =angel_SWIreason_LateStartup
          CMP     r1, r2
          BEQ     LateStartupSWI

        ENDIF

        LDR     r2, =angel_SWIreason_ReportException
        CMP     r1, r2
        BNE     UnrecognisedSWI
        ; Fall through to ReportException
        
ReportException
  IF :DEF: ICEMAN_LEVEL_3 :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
        ; We cannot report an exception within ICEMan
        B       ReportException
  ELSE
        ; The ADP_Stopped code was in r1 before the SWI happened
        LDR     r2, [r0, #Angel_RegBlock_R0offset + (1*4)]
        B       CallSerialiseTask
  ENDIF


ReturnSVC
        ; The SVC stack is empty.  We have to return in SVC mode with
        ; IRQ and FIQ disabled. r0-r13 and r14 should be reinstated from
        ; regblock (ie. the stack remains the caller's stack)
        ; Execution will resume from the instruction after the SWI
        ;
        ; We will return with r0 set to the address of Angel_ExitToUser
        ; 
        LDR     r2, [r0, #Angel_RegBlock_R0offset + (13*4)] ; get callers sp
        LDR     r3, [r0, #Angel_RegBlock_R0offset + (15*4)] ; PC (pts to SWI)
        MRS     r4, SPSR
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          TST   r4, #Tbit
          ADDEQ r3, r14, #4      ; RE-Adjust for ARM instruction
          ADDNE r3, r14, #2      ; RE-Adjust for Thumb instruction
        ELSE
          ADD   r3, r14, #4      ; RE-Adjust to point after UNDEF 
        ENDIF
        STMFD   sp!, {r2-r3}    ; Push Caller sp and pc onto SVC stack

        ; Restore r1-12 and r14
        ADD     r0, r0, #Angel_RegBlock_R0offset + (1*4)
        LDMIA   r0, {r1-r12}
        SUB     r0, r0, #Angel_RegBlock_R0offset + (1*4)
        LDR     r14, [r0, #Angel_RegBlock_R0offset + (14*4)]
        LDR     r0, [r0, #Angel_RegBlock_CPSRoffset]
        MSR     CPSR_f, r0    ; Only reinstate the flag values !

        ; Set r0 to the address of Angel_ExitToUser
        IMPORT  Angel_ExitToUSR
        LDR     r0, =Angel_ExitToUSR
        
        ; Restore sp and resume
        LDMFD   sp, {sp, pc}

        ; And now the application executes in SVC mode with its own stack
        ; and interrupts disabled.
        
        IF (:LNOT: :DEF: MINIMAL_ANGEL) :LOR: MINIMAL_ANGEL = 0
          IF :LNOT: (:DEF: JTAG_ADP_SUPPORTED :LOR: :DEF: ICEMAN_LEVEL_3)
        
ApplDeviceSWI
            LDR     r2, =angel_ApplDeviceHandler
            B       DoIndirectCall

          ENDIF
        ENDIF
        
        IF :DEF: LATE_STARTUP :LAND: LATE_STARTUP <> 0

LateStartupSWI
          LDR     r2, =angel_LateBootInit
          B       DoIndirectCall

        ENDIF
        
UnrecognisedSWI
  IF :DEF: ICEMAN_LEVEL_3 :LOR: (:DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0)
        ; We cannot report an unrecognised SWI within ICEMan
        B       UnrecognisedSWI 
  ELSE
        LDR     r2, =ADP_Stopped_SoftwareInterrupt
        B       CallSerialiseTask

  ENDIF

SavedR10 DCD 0
        
        END
