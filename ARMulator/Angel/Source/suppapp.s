        TTL     Angel assembler for Sys Libary support routines     > suppapp.s
        ; ---------------------------------------------------------------------
        ; This source files holds the routine needed to glue the low level 
        ; sys library layer together. This routine is used to call into Angel
        ; where the library functions are carried out. 
        ;
        ; $Revision: 1.4.6.3 $
        ;   $Author: dbrooke $
        ;     $Date: 1998/02/25 17:36:12 $
        ;
        ; Copyright (c) 1995, Advanced RISC Machines Ltd (ARM)
        ; All Rights Reserved
        ; ---------------------------------------------------------------------
        GET     objmacs.s              ; C library definitions.
   [ INTERWORK 
        AREA    |SysSupportCode|,CODE, READONLY, INTERWORK
   |
        AREA    |SysSupportCode|,CODE, READONLY
   ]
        GET     listopts.s              ; standard listing control
        GET     cl_lolvl.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support

        EXPORT  _syscall

   [ THUMB
        CODE16
        EXPORT _syscall16
_syscall16
        BX        pc
        NOP
        CODE32
   ]

_syscall
        STMFD   sp!,{lr}                ; Save lr as we are about to write
        ADR     lr,_syscall_return      ; over it. 
        DCD     angel_SWI_ARM
        
_syscall_return
   [ INTERWORK :LOR: THUMB
        LDMFD        sp!,{lr}
        BX        lr
   |
        LDMFD   sp!,{pc}                ; Return to the caller
   ]


        
        ; No THUMB or interworking versions of __sys_exit and
        ; __sys_generate_error as these are called directly from assembler
        ; code for ARM state inside the C library.

        IMPORT  __rt_asm_fatalerror     ; error reporting via suppasm.s
        EXPORT __sys_exit
__sys_exit
        MOV     r1,r0           ; This is the exit() return code; sadly,
                                ; it gets lost in the SWI...  please fix!
        LDR     r0,=angel_SWIreason_ReportException
        LDR     r1,=ADP_Stopped_ApplicationExit
        DCD     angel_SWI_ARM
        
    IF :DEF: ANGEL_VSN          ; if compiling into Angel, do nice error handling
        ADR     a1, retexitmsg
        B       __rt_asm_fatalerror
retexitmsg
        DCB     "sys_exit returned!\n"
    ELSE
1       B       %B01
    ENDIF
        ALIGN


        
        EXPORT __sys_generate_error
__sys_generate_error
        MOV     r1,r0           ; Save the internal error code.
        LDR     r0,=angel_SWIreason_ReportException 
        LDR     r1,=ADP_Stopped_RunTimeErrorUnknown
        DCD     angel_SWI_ARM
        
    IF :DEF: ANGEL_VSN          ; if compiling into Angel, do nice error handling
        ADR     a1, reterrmsg
        B       __rt_asm_fatalerror
reterrmsg
        DCB     "sys_generate_error returned!\n"
    ELSE
2       B       %B02
    ENDIF
        ALIGN

        END
