        AREA    mul, CODE, READONLY, INTERWORK

        EXPORT  smull
        EXPORT  smlal
        EXPORT  umull
        EXPORT  umlal

smull   MOV     R2, R0          ; Ensure RdLo != RdHi != Rm
        SMULL   R0, R1, R2, R1
        BX      LR              ; Return using BX for interworking

smlal   SMLAL   R0, R1, R2, R3
        BX      LR

umull   MOV     R2, R0          ; Ensure RdLo != RdHi != Rm
        UMULL   R0, R1, R2, R1
        BX      LR

umlal   UMLAL   R0, R1, R2, R3
        BX      LR

        END
