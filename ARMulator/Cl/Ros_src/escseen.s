;;; riscos/k_escseen.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     h_brazil.s
        GET     objmacs.s
        GET     h_hw.s

        CodeArea

        Function _kernel_escape_seen

        FunctionEntry UsesSb
        MOV     a4, pc                  ; read psr
        LDR     a3, addr___rt_escapeSeen
        MOV     a2, #0
        TST     a4, #PSRSVCMode
        SWIEQ   EnterSVC
        TEQP    pc, #PSRIBit+PSRSVCMode ; interrupts off, for atomic read and update
        LDRB    a1, [a3, #0]
        STRB    a2, [a3, #0]
        TEQP    pc, a4                  ; restore original psr
        NOP
        Return  UsesSb

        AdconTable

        IMPORT  |__rt_escapeSeen|
addr___rt_escapeSeen
        &       |__rt_escapeSeen|
        END
