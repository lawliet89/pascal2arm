;;; riscos/infpe.s: library interface to fp emulator module
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     h_brazil.s
        GET     objmacs.s

        CodeArea

        Function __fp_initialise
        ; Determine whether FP is available (to decide whether fp regs need
        ; saving over _kernel_system and in setjmp).
        ; The SWI will fail if there isn't an active FPEmulator module.
        MOV     r1, lr
        TEQP    pc, #0
        SWI     FPE_Version
        MOVVC   r0, #&70000     ; IVO, DVZ, OFL cause a trap
        WFSVC   r0
        MOVVC   r0, #1
        MOVVS   r0, #0
        MOV     lr, r1
        Return  , "", LinkNotStacked


        Function __fp_finalise
        ; nothing to do.
        Return  , "", LinkNotStacked


        Function __fp_address_in_emulator
        ; determine whether r0 is an address inside the fp emulator,
        ; (to allow a data abort or address exception in a floating-point
        ; load or store to be reported as occurring at that instruction,
        ; rather than somewhere in the code of the emulator).
        FunctionEntry , "r4,r5,r6"
        MOV     r6, r0
        MOV     r0, #18         ; look up name
        ADR     r1, FPEName
        SWI     Module
        ;  returns r0  unchanged
        ;          r1  module number
        ;          r2  instantiation number
        ;          r3  address of module code
        ;          r4  contents of private word
        ;          r5  pointer to postfix string
        MOV     r0, #0
        BVS     exit
        ; (r3 = code base of FPE; word before is length of FPE code)
        CMP     r6, r3
        LDRGE   r4, [r3, #-4]
        ADDGE   r3, r3, r4
        CMPGE   r3, r6
        MOVGE   r0, #1
exit
        Return  , "r4,r5,r6"

FPEName =       "FPEmulator",0
        ALIGN

        END
