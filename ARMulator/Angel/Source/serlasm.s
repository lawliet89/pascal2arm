        TTL     Angel serialiser support                 > serlasm.s
        ;
        ; This file provides veneers used by the serialiser module, as
        ; well as some globaly needed functions.
        ;
        ; $Revision: 1.13.4.1 $
        ;   $Author: rivimey $
        ;     $Date: 1997/12/10 18:50:16 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ;

        KEEP

        GET     listopts.s           ; standard listing control
        GET     lolevel.s            ; automatically built manifest definitions
        GET     macros.s             ; standard assembler support
        GET     target.s             ; target specific manifests

        EXPORT  Angel_EnterSVC
        EXPORT  Angel_ExitToUSR
        EXPORT  Angel_DisableInterruptsFromSVC
        EXPORT  Angel_EnableInterruptsFromSVC
        EXPORT  Angel_RestoreInterruptsFromSVC
        EXPORT  angel_ReadBankedRegs

        AREA    |serlasm|,CODE,PIC,READONLY

        ; See serlock.h for the interface to these functions.
        IMPORT  Angel_StackBase
        IMPORT  angel_SVCEntryFlag
        
        IMPORT  __rt_asm_fatalerror  ; error reporting via suppasm.s
        
Angel_EnterSVC
        ; a1-a4 and ip may be corrupted under the APCS
        MRS     a1, CPSR
        AND     a2, a1, #ModeMask
        CMP     a2, #USRmode
        BEQ     WasUSRMode
        CMP     a2, #SVCmode

InappropriateModeError
        BEQ     WasSVCMode
        ADR     a1, imdmsg
        B       __rt_asm_fatalerror
imdmsg
        DCB     "Inappropriate Mode\n"
        ALIGN
        
WasSVCMode      ; Just disable IRQ and FIQ and note SVC entry from SVC
        MOV     a2, #1
        LDR     a3, =angel_SVCEntryFlag
        STR     a2, [a3]
        ORR     a1, a1, #IRQDisable + FIQDisable
        MSR     CPSR, a1
        MOV     pc, lr
                
WasUSRMode
        ; The SWI does everthing for us - except note that entry was from USR
        MOV     a2, #0
        LDR     a3, =angel_SVCEntryFlag
        STR     a2, [a3]
        LDR     a1, =angel_SWIreason_EnterSVC
        DCD     angel_SWI_ARM   ; Get into SVC with IRQ and FIQ disabled
        MOV     pc, lr          ; Return to caller
        
        ; On entry we check to see whether angel_SVCEntryFlag is
        ; 0 => Entry to SVC was from USR mode
        ; 1 => Entry to SVC was from SVC mode
        ; and return in that mode
Angel_ExitToUSR
        ; a1-a4 and ip may be corrupted under the APCS
        LDR     a3, =angel_SVCEntryFlag
        LDR     a2, [a3]
        CMP     a2, #0
        BNE     ReturnInSVCMode

        ; The standard return to USR case follows ...
        MOV     a2, lr
        MOV     a3, sp
        LDR     sp, =Angel_StackBase   ; Reset SVC sp to the empty SVC stack
        LDR     sp, [sp]
        ADD     sp, sp, #Angel_SVCStackOffset
        STMFD   a3, {a3}
        SUB     a3, a3, #4
        LDMFD   a3, {sp}^
        MRS     a1 ,CPSR  
        BIC     a1, a1, #ModeMask + IRQDisable + FIQDisable
        ORR     a1, a1, #USRmode
        MSR     CPSR, a1
        MOV     pc, a2

ReturnInSVCMode
        MRS     a1, CPSR
        BIC     a1, a1, #IRQDisable + FIQDisable
        MSR     CPSR, a1
        MOV     pc, lr
                

        ; On entry a1 = address of the regblock, a2 = the mode
        ; We must be called in USR mode - by debugos.c most likely.
angel_ReadBankedRegs
        CMP     a2, #USRmode
        BNE     PrivModeCase
        STR     r13, [a1, #Angel_RegBlock_R0offset + (13*4)]
        STR     r14, [a1, #Angel_RegBlock_R0offset + (14*4)]
        MOV     pc, lr
PrivModeCase    ; Write r8-r14 + spsr (in case it was FIQ mode
        MOV     a4, a1
        LDR     a1, =angel_SWIreason_EnterSVC
        DCD     angel_SWI_ARM   ; Get into SVC with IRQ and FIQ disabled
        MSR     CPSR, a2
        ADD     a3, a4, #Angel_RegBlock_R0offset + (8*4)
        STMIA   a3, {r8-r14}
        MRS     a3, SPSR
        STR     a3,  [a4, #Angel_RegBlock_SPSRoffset]
        MOV     a2, #USRmode
        MSR     CPSR, a2
        NOP
        MOV     pc, lr

        
Angel_DisableInterruptsFromSVC
        MRS     a1, CPSR
        IF (FIQ_SAFETYLEVEL >= FIQ_NeverUsesSerialiser_DoesNotReschedule)
          ORR   a2, a1, #IRQDisable
        ELSE
          ORR   a2, a1, #IRQDisable + FIQDisable
        ENDIF
        MSR     CPSR, a2
        MOV     pc, lr

Angel_EnableInterruptsFromSVC
        MRS     a1, CPSR
        BIC     a2, a1, #IRQDisable + FIQDisable
        MSR     CPSR, a2
        MOV     pc, lr

Angel_RestoreInterruptsFromSVC
        MSR     CPSR, a1
        MOV     pc, lr

        
        ; ****************************************************************

        IF :DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0
          ; Don't need all the serialiser-specific stuff
        ELSE
        
        IMPORT  Angel_MutexSharedTempRegBlocks
        IMPORT  angel_SerialiseTaskCore
        EXPORT  Angel_SerialiseTask
        
        ; For the interface details of this pseudo function refer to
        ; serlock.h.  For the details of the actual function
        ; angel_SerialiseTaskCore see serlock.c

Angel_SerialiseTask
        ; r0 (a1) = called_by_yield
        ; r1 (a2) = fn
        ; r2 (a3) = state
        ; r3 (a4) = empty_stack
        
        ; Flatten the stack of whatever mode we were called in
        MOV  sp, r3

        ; Get into SVC and disable IRQ and FIQ
        IF :DEF: ASSERT_ENABLED :LAND: ASSERT_ENABLED <> 0
          ; Check we are not in USR mode
          MRS  r4, CPSR
          AND  r4, r4, #ModeMask
          CMP  r4, #USRmode
          BNE  NotUsrMode
        
          ADR  a1, serlusrmsg
          B    __rt_asm_fatalerror
serlusrmsg
          DCB  "Serialise in USR mode\n"
          ALIGN
NotUsrMode
        ENDIF

        IF (FIQ_SAFETYLEVEL >= \
            FIQ_NeverUsesSerialiser_DoesNotReschedule_HasNoBreakpoints)
          MOV r4, #SVCmode + IRQDisable
        ELSE
          MOV r4, #SVCmode + IRQDisable + FIQDisable
        ENDIF
        MSR  cpsr, r4

        ; Set up desired_regblock (pc and r0 only at this point)
        LDR  r4, =Angel_MutexSharedTempRegBlocks
        ADD  r4, r4, #Angel_RegBlockSize
        STR  r1, [r4, #Angel_RegBlock_R0offset + (15*4)]
        STR  r2, [r4, #Angel_RegBlock_R0offset]
        MOV  r1, r4  ; This is 2nd arg to angel_SerialiseTaskCore

        ; Note that at this point the SVC stack may be in any of the
        ; following states:
        ;  * Empty Angel_SVCStack - if no task of WantsLock priority
        ;    is running, and the application is not using the SVC stack
        ;  * Non empty Angel_SVCStack - if a task with WantsLock priority
        ;    is running (this will call of serialise task will be due
        ;    to another packet arriving).
        ;  * Application SVC stack
        ;
        ; However, using the Application SVC stack is not acceptable
        ; since it may not be large enough, or indeed r13 may have been
        ; corrupted.  Therefore we must spot sp outside the range
        ; Angel_SVCStack - Angel_SVCStackLimit and if that is the case
        ; the it must be the Application stack in use, so switch to
        ; the Angel_SVCStack (which is empty in this case).
        
        LDR  r4, =Angel_StackBase
        LDR  r4, [r4]
        ADD  sl, r4, #Angel_SVCStackLimitOffset
        ADD  r4, r4, #Angel_SVCStackOffset
        CMP  sp, sl
        BCC  StackNotSetUp      ; sp < Angel_SVCStackLimit
        CMP  sp, r4
        BLS  StackIsSetUp       ; sp <= Angel_SVCStack
StackNotSetUp
        MOV  sp, r4     

StackIsSetUp        
        ; Call angel_SerialiseTaskCore and set up lr to "return" to
        ; angel_NextTask.

        LDR  lr, =angel_NextTask
        LDR  r4, =angel_SerialiseTaskCore
        MOV  pc, r4

        ; ****************************************************************

        EXPORT angel_StartTask
        EXPORT angel_StartTask_NonSVCEntry

        ; For the interface details of this pseudo function refer to
        ; serlock.c
        ;
        ; What makes this all tricky is that we must completely finish with
        ; regblock before making any mode switch as the modes switch may
        ; enable interrupts, which can potentially corrupt regblock!

angel_StartTask
        ; r0 (a1) = regblock

        ; This code resumes, restoring all regs from the regblock
        ; It works for all modes (!!!) (inc FIQ, and SVC, SYS and USR)
        
        ; Reset the SVC Stack Pointer (if the new task is an SVC task,
        ; the correct sp will be set later)
        LDR   sp, =Angel_StackBase
        LDR   sp, [sp]
        ADD   sp, sp, #Angel_SVCStackOffset
        
angel_StartTask_NonSVCEntry
        ; Put CPSR, r8-r12, r14, (SPSR), r0, r1, PC on the Appl stack
        ; Note that SPSR is only VALID if moving to non USR mode
        LDR   r1, [r0, #Angel_RegBlock_R0offset + (13*4)] ; Appl sp
        LDR   r2, [r0, #Angel_RegBlock_CPSRoffset]

        ADD   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        LDMIA r0, {r3,r4,r5,r6,r7}      ; r8-r12 go in r3-r7
        SUB   r0, r0, #Angel_RegBlock_R0offset + (8*4)
        LDR   r8, [r0, #Angel_RegBlock_R0offset + (14*4)]
        LDR   r9, [r0, #Angel_RegBlock_SPSRoffset]
        LDR   r10, [r0, #Angel_RegBlock_R0offset]
        LDR   r11, [r0, #Angel_RegBlock_R0offset + (1*4)]
        LDR   r12, [r0, #Angel_RegBlock_R0offset + (15*4)]
        STMFD r1!, {r2-r12}

        ; Restore r2-r7 from regblock
        ADD   r0, r0, #Angel_RegBlock_R0offset + (2*4)
        LDMFD r0, {r2-r7}      ; Get r2-r7 from regblock

        ; Now we have lost the ptr to regblock, but still have stuff
        ; on the Appl stack.
        LDMFD r1, {r0}          ; Get CPSR from Application stack
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          BIC   r0, r0, #Tbit   ; Ensure T bit is never set here!
        ENDIF

        ; If we need to start a USR mode task, we cannot disable interrupts
        ; to set up USR sp safely, as we won't be able to reenable interrupts
        ; later.
        ; Instead we must set up USR sp now, before switching to USR mode.
        AND   r14, r0, #ModeMask
        CMP   r14, #USRmode
        BNE   ProtectAndChangeMode

        MOV   r14, r1
        STMFD r1!, {r14}        ; save USR sp
        LDMFD r1, {sp}^         ; get USR sp
        ADD   r1, r1, #4        ; reset r1
        B     ChangeMode
        
ProtectAndChangeMode
        ; We are in a privileged mode.
        ; Disable interrupts until the last minute to give us a chance
        ; to set up the stack pointer safely for our new task
        IF (FIQ_SAFETYLEVEL >= FIQ_NeverUsesSerialiser_DoesNotReschedule)
          ORR r0, r0, #IRQDisable
        ELSE
          ORR r0, r0, #IRQDisable + FIQDisable
        ENDIF
        
ChangeMode
        ; Change mode
        ; Also set up Appl sp and r8-12, r14
        MSR   CPSR, r0
        NOP
        MOV   sp, r1            ; already done if USR mode, but that's okay
        LDMFD sp!, {r0}         ; Get CPSR from Application stack
        LDMFD sp!, {r8-r12,r14} ; restore r8-r12 + r14

        ; Decide whether to restore SPSR (not in USR / SYS mode)
        ; But don't forget this corrupts the flag bits in CPSR!
        AND   r1, r0, #ModeMask
        CMP   r1, #USRmode
        CMPNE r1, #SYSmode
        ADDEQ sp, sp, #4        ; If USR or SYS skip over SPSR on stack
        LDMNEFD sp!, {r1}       ; Do next (NE) instrs only if not SYS or USR
        MSRNE SPSR, r1

        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
          TST   r0, #Tbit
          BEQ   ResumeARM
          BIC   r0, r0, #Tbit
          MSR   CPSR, r0  ; Restore flags & IF bits corrupted since mode change
          ADR   r0, NextInstructionLabel+1
          BX    r0              ; Switch into Thumb state !
NextInstructionLabel
          CODE16
          NOP
          POP  {r0, r1, pc} 
          ; And the application executes in Thumb state !
          CODE32
          ALIGN
        ENDIF

ResumeARM
        AND   r1, r0, #ModeMask
        CMP   r1, #USRmode
        BEQ   UserResumeARM
        CMP   r1, #SYSmode
        BEQ   UserResumeARM

        ; Privileged Mode resume - we must not enable interrupts until
        ; the very last instruction!
        MSR   SPSR, r0
        ADD   sp, sp, #(3 * 4)
        LDMDB sp, {r0, r1, pc}^
        ; and the task resumes ...
        
UserResumeARM
        MSR   CPSR, r0      ; Restore the flags & IF bits corrupted above
        NOP
        LDMFD sp!, {r0, r1, pc}
        ; And the task executes!
        
        ; ****************************************************************

        EXPORT Angel_Yield
        IMPORT Angel_YieldCore

        ; For the interface details of this pseudo function refer to
        ; serlock.h

        ; Note that this an APCS conformant fn from the callers point
        ; of view, so we (only) have to preserve r4-r11, r13 and the
        ; mode we were called in.

Angel_Yield
        ; See what mode we are in and get into SVC and disable interrupts
        MRS   r0, CPSR
        AND   r0, r0, #ModeMask
        CMP   r0, #USRmode
        BNE   NotEnteredInUSR
        STMFD sp!, {r0, r14}         ; Save the original CPSR and r14
        BL    Angel_EnterSVC         ; This keeps the stacks the same
        LDMFD sp!, {r0, r14}
        B     InSVCInterruptsDisabled

NotEnteredInUSR
        CMP   r0, #SVCmode
        BEQ   YieldInUsrOrSvcMode
        
        ; Not USR or SVC - Oh dear!!!
        ADR   a1, serlusrsvcmsg
        B     __rt_asm_fatalerror
serlusrsvcmsg
        DCB  "Yield not USR or SVC mode\n"
        ALIGN
YieldInUsrOrSvcMode

        IF (FIQ_SAFETYLEVEL >= FIQ_NeverUsesSerialiser_DoesNotReschedule)
          ORR r1, r0, #IRQDisable
        ELSE
          ORR r1, r0, #IRQDisable + FIQDisable
        ENDIF
        MSR   cpsr, r1
        
        ; Now in SVC with interrupts disabled, r0 = original CPSR
InSVCInterruptsDisabled
        ; Save the caller's regs into our shared regblock
        ; We only have to save r4-r11, r13, cpsr, pc (says the APCS)
        LDR   r1, =Angel_MutexSharedTempRegBlocks
        STR   r0,  [r1, #Angel_RegBlock_CPSRoffset]        ; cpsr
        STR   r13, [r1, #Angel_RegBlock_R0offset + (13*4)] ; sp
        STR   r14, [r1, #Angel_RegBlock_R0offset + (15*4)] ; pc
        ADD   r1, r1, #Angel_RegBlock_R0offset + (4*4)
        STMIA r1, {r4-r11}
        SUB   r0, r1, #Angel_RegBlock_R0offset + (4*4)
 
        ; Now prepare to call Angel_SerialiseTask
        ; r0 = called_by_yield        ; r1 = fn
        ; r2 = state                  ; r3 = empty_stack
        MOV   r0, #1
        LDR   r1, =Angel_YieldCore
        MOV   r2, #0
        LDR   r3, =Angel_StackBase  ; Empty SVC Stack 
        LDR   r3, [r3]
        ADD   r3, r3, #Angel_SVCStackOffset
        B     Angel_SerialiseTask
        ; and Angel_YieldCore will execute when SerialiseTask allows it to

        ; ****************************************************************

        IMPORT angel_SelectNextTask
        EXPORT angel_NextTask
        
angel_NextTask
        ; This can be "called" from SVC or USR.  It must get into SVC and
        ; disable interrupts and grab the empty SVC stack.
        ; Then it must call angel_SelectNextTask with no arguments.
        ;
        ; See what mode we are in and get into SVC and disable interrupts
        MRS   r0, CPSR
        AND   r0, r0, #ModeMask
        CMP   r0, #USRmode
        BNE   NotEnteredInUSR2
        BL    Angel_EnterSVC         ; This keeps the stacks the same
        B     InSVCInterruptsDisabled2

NotEnteredInUSR2
        CMP   r0, #SVCmode
        BEQ   NextTaskUsrOrSvcMode
        
        ; Not USR or SVC - Oh dear!!!
        ADR  a1, serlusrsvcmsg
        B    __rt_asm_fatalerror
ntusrsvcmsg
        DCB  "NextTask not USR or SVC mode\n"
        ALIGN
        
NextTaskUsrOrSvcMode

        IF (FIQ_SAFETYLEVEL >= FIQ_NeverUsesSerialiser_DoesNotReschedule)
          ORR r1, r0, #IRQDisable
        ELSE
          ORR r1, r0, #IRQDisable + FIQDisable
        ENDIF
        MSR   cpsr, r1
        
        ; Now in SVC with interrupts disabled
InSVCInterruptsDisabled2
        ; Grab an empty SVC stack and call angel_SelectNextTask
        LDR   sl, =Angel_StackBase
        LDR   sl, [sl]
        ADD   sp, sl, #Angel_SVCStackOffset
        ADD   sl, sl, #Angel_SVCStackLimitOffset
        LDR   r0, =angel_SelectNextTask
        MOV   pc, r0

        ; ****************************************************************

        EXPORT angel_IdleLoop

angel_IdleLoop
        ; Reset the SVC stack
        LDR   sp, =Angel_StackBase
        LDR   sp, [sp]
        ADD   sp, sp, #Angel_SVCStackOffset

        ; Reenable interrupts
        MRS   r0, CPSR
        BIC   r0, r0, #IRQDisable + FIQDisable
        MSR   CPSR, r0

        ; Repeatedly call Angel_Yield
IdleLoop
        BL    Angel_Yield
        B     IdleLoop

        ENDIF                   ; ... else not minimal angel ...
                
        END
