        TTL     Angel assembler support for logadp.c                > ladpasm.s
        ; ---------------------------------------------------------------------
        ; This source files holds the assembler routine needed by logadp.c
        ;
        ; $Revision: 1.1 $
        ;   $Author: mgray $
        ;     $Date: 1996/08/22 15:42:30 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1996.
        ; All Rights Reserved
        ;
        ; ---------------------------------------------------------------------

        GET     listopts.s              ; standard listing control
        GET     lolevel.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support
        GET     target.s                ; target specific definitions

        ; ---------------------------------------------------------------------
        AREA    |C$$Code$$LogADPsupport|,CODE,READONLY
        KEEP

        ; ---------------------------------------------------------------------
        ; Return 1 if we are in USR mode, else 0.

        EXPORT logadp_inUSRmode

logadp_inUSRmode
        MRS     r1, CPSR
        AND     r1, r1, #ModeMask
        TEQ     r1, #USRmode
        MOVEQ   r0, #1
        MOVNE   r0, #0
        MOV     pc, lr

        END

        
