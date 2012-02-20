       TTL     Angel assembler for DCC support     > dccsup.s
        ; ---------------------------------------------------------------------
        ; contains functions to read and write data to the comms channel
        ; $Revision: 1.3 $
        ;   $Author: amerritt $
        ;     $Date: 1996/07/08 13:22:46 $
        ;
        ; Copyright (c) 1995, Advanced RISC Machines Ltd
        ; All Rights Reserved
        ; ---------------------------------------------------------------------

        AREA  DccSupportCode,  CODE, READONLY

C0      CN 0
C1      CN 1
CP14    CP 14
        
        EXPORT  dcc_PutWord
        EXPORT  dcc_GetWord
        EXPORT  dcc_CanWrite
        EXPORT  dcc_CanRead
        EXPORT  dcc_GetFlags

dcc_GetFlags
        MRC     CP14, 0, R0, C0, C0
        MOV     PC, R14
        
dcc_CanWrite                    ; wait until reg is free
        MRC     CP14, 0, R1, C0, C0
        TST     R1, #0x2
        MOVNE   R0, #0
        MOVEQ   R0, #1
        MOV     PC, R14
        
dcc_PutWord                     ; write out data
        MCR     CP14, 0, R0, C1, C0
        MOV     PC, R14
        
dcc_CanRead                     ; wait until there is data waiting
        MRC     CP14, 0, R1, C0, C0
        TST     R1, #0x1
        MOVEQ   R0, #0
        MOVNE   R0, #1
        MOV     PC, R14
dcc_GetWord                     ; read data
        MRC     CP14, 0, R0, C1, C0
        MOV     PC, R14

        END
