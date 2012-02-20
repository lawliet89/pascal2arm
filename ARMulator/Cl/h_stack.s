;;; h_stack.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

RootStackSize   *       4*1024
MinStackIncrement *     RootStackSize
SafeStackSize   *       RootStackSize-SC_SLOffset

; A stack frame, relative to its fp value
frame_entrypc   *       0
frame_link      *       -4
frame_sp        *       -8
frame_prevfp    *       -12

; Stack chunks

SL_Lib_Offset     *      -540
SL_Client_Offset  *      -536

                ^       0
SC_mark         #       4
SC_next         #       4
SC_prev         #       4
SC_size         #       4
SC_deallocate   #       4
SC_LibOffset    #       4
SC_ClientOffset #       4
SC_veneerStaticLink #   4
SC_veneerStkexLink  #   4
SC_DescSize     #       0

SC_SLOffset     *       560
IsAStackChunk   *       &f60690ff

ChunkChange     *       &1              ; marker in FP values in stack

; The size of a private stack chunk, used purely for allocating new
; stack chunks (in GetStackChunk).
ExtendStackSize *       150*4 + SC_DescSize

        END
