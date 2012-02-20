        TTL     Angel Late Startup code                 > latestrt.s
        ;
        ; $Revision: 1.1.6.1 $
        ;   $Author: rivimey $
        ;     $Date: 1997/12/10 18:51:11 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995-1997.
        ; All Rights Reserved
        ;

        KEEP

        GET     listopts.s              ; standard listing control
        GET     lolevel.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support
        GET     target.s                ; target specific definitions

        AREA    entry,CODE,PIC,READONLY

AL_BLOCK        EQU     1

        IMPORT  Angel_LateStartup
        IMPORT  Angel_Yield
        IMPORT  LogWarning
        IMPORT  LogError
                
        EXPORT  |__entry|
__entry
        ADR     r0, in_entry
        BL      LogWarning
        MOV     r6, #&10000
delay   BL      Angel_Yield
        SUBS    r6, r6, #1
        BNE     delay
        ADR     r0, about_to
        BL      LogWarning
        MOV     r0, #AL_BLOCK
        BL      Angel_LateStartup
        ADR     r0, done_late
        BL      LogWarning
loop    BL      Angel_Yield
        B       loop

in_entry        DCB     "In __entry\n", 0
about_to        DCB     "About to do LateStartup\n", 0
done_late       DCB     "Done LateStartup\n", 0
        
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
        ADR     r0, PrivilegedStackOverflowMessage
        BL      LogError
PrivilegedStackOverflow
        B       PrivilegedStackOverflow

PrivilegedStackOverflowMessage
        DCB     "Stack Overflow in mode %2X\n"
        ALIGN

        IMPORT  Angel_AngelStackLimit
        IMPORT  Angel_AngelStack
USRmodeStackCheck
        ; If this is the AngelStack then sl should
        ; be somewhere within the Angel Stack range
        LDR     r0, =Angel_AngelStackLimit
        LDR     r1, =Angel_AngelStack - 1
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
;; then stop in a tight loop

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

        ADR     a1, stkovfmsg
        BL      LogError

        ENDIF

; NOTE: Since this is the stack overflow handler we need to ensure
; that the code we call does not check the stack-limit, or that we
; setup the handler to execute in a new stack.
FatalError1
        B       FatalError1

stkovfmsg
        DCB     "Stack Overflow\n"

        ; and we should return to this point after the CATCHVector
        ; processing. NOTE: Since we placed our original PSR into the
        ; SPSR register, we should be back in the correct mode.
        LDMFD   sp!,{v1,v2,pc}  ; and return to caller

        END
