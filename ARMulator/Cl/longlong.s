;;; longlong.s: support library for implementation of 64-bit integers
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.6.4.1 $
;;; Checkin $Date: 1997/05/28 08:10:19 $
;;; Revising $Author: wdijkstr $

;;; Preliminary version: for processors with MULL instruction family only

        GET     objmacs.s
 [ THUMB
        CODE16
 ]

        CodeArea

 [ make = "_ll_neg" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_neg, leaf
        MOV     r3, r1
        MOV     r1, #0
        NEG     r0, r0
        SBC     r1, r3
     |
        Function _ll_neg, leaf
        RSBS    r0, r0, #0
        RSC     r1, r1, #0
   ]
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_add" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_add, leaf
        ADD     r0, r0, r2
        ADC     r1, r3
     |
        Function _ll_add, leaf
        ADDS    r0, r0, r2
        ADC     r1, r1, r3
   ]
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_sub" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_sub, leaf
        SUB     r0, r0, r2
        SBC     r1, r3
     |
        Function _ll_sub, leaf
        SUBS    r0, r0, r2
        SBC     r1, r1, r3
   ]
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_rsb" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_rsb, leaf
        SUB     r0, r2, r0
        SBC     r3, r1
        MOV     r1, r3
     |
        Function _ll_rsb, leaf
        RSBS    r0, r0, r2
        RSC     r1, r1, r3
   ]
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_mul" :LOR: make = "all" :LOR: make="shared-library"

t0      RN      r4
t1      RN      r5
t2      RN      r6
        ; separate a 32 bit value into 2xunsigned 16 bit values
        ; resh and resl must be different
        MACRO
        USplit16 $rl, $rh, $x
        MOV     $rh, $x, LSR #16
        BIC     $rl, $x, $rh, LSL #16
        MEND

        ; unsigned 32x32=64 and dl,dh,x must be distinct
        ; dh.dl = x*y
        MACRO
        _UMULL  $dl, $dh, $x, $y
        ; no hardware multiplier
        ; extract y first - it may equal dl, dh or x
        USplit16        t0, t1, $y
        USplit16        t2, $dh, $x
        MUL               $dl, t0, t2       ; low x * low y
        MUL               t0, $dh, t0       ; high x * low y
        MUL               $dh, t1, $dh          ; high y * high x
        MUL               t1, t2, t1      ; low x * high y
        UAdd16  $dl, $dh, t0            ; add one middle value
        UAdd16  $dl, $dh, t1            ; add other middle value
        MEND

        ; add a 32 bit value to a 64 bit value, shifted up 16 unsigned
        MACRO
        UAdd16  $rl, $rh, $x
        ADDS      $rl, $rl, $x, LSL#16
        ADC         $rh, $rh, $x, LSR#16
        MEND

   [ THUMB
        Function  __16_ll_mul, leaf
   |
        Function  _ll_mul, leaf
   ]
        FunctionEntry , "r4,r5,r6"
        MOV     lr, r0
   [ THUMB
        BX      pc
        CODE32
   ]
        _UMULL  r0, ip, r2, lr
        MLA     r1, r2, r1, ip
        MLA     r1, r3, lr, r1
        Return  , "r4,r5,r6"
   [ THUMB
        CODE16
   ]
 ]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       64/64 DIVISION          ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; unsigned 64/64 = 64 with remainder 64
;
; n=numerator   d=denominator (each split into high and low part)
; q=quotient    r=remainder
;
; ie n/d = q + r/d or n=q*d+r and 0<=r<d
; if d=0 then it returns q=0, r=n (so n=q*d+r !)
; registers must be distinct
; n and d are corrupted
; t is a temporary register
;
; Routine is not unrolled since the speedup isn't great.
; Can unroll if you like.

        MACRO
        UDIV_64d64_64r64 $ql,$qh,$rl,$rh,$nl,$nh,$dl,$dh,$t
        MOV     $ql,#0          ; zero the quotient
        MOV     $qh,#0
        MOV     $rh,$nh         ; set the remainder to the current value
        MOV     $rl,$nl
        TEQ     $dh,#0
        TEQEQ   $dl,#0
        BEQ     %F08            ; divide by 0
        MOVS    $t,#0           ; count number of shifts
        ; first loop gets $d as large as possible
00
        ADDS    $dl, $dl, $dl
        ADCS    $dh, $dh, $dh   ; double d
        BCS     %F01            ; overflowed
        CMP     $dh, $rh
        CMPEQ   $dl, $rl
        ADDLS   $t, $t, #1      ; done an extra shift
        BLS     %B00
        ADDS    $t, $t, #0      ; clear carry
01                              ; carry the overflow here
        MOVS    $dh, $dh, RRX   ; colour
        MOV     $dl, $dl, RRX   ; shift back down again
02                              ; now main loop
        SUBS    $nl, $rl, $dl
        SBCS    $nh, $rh, $dh   ; n = r - d and C set if r>=d
        MOVCS   $rh, $nh
        MOVCS   $rl, $nl        ; r=r-d if this goes
        ADCS    $ql, $ql, $ql
        ADC     $qh, $qh, $qh   ; shift next bit into the answer
        MOVS    $dh, $dh, LSR#1
        MOV     $dl, $dl, RRX   ; shift down d
        SUBS    $t, $t, #1
        BGE     %B02            ; do next loop (t+1) loops
08
        MEND

; signed 64/64 with remainder 64
;
; n=numerator   d=denominator (each has a high and low part)
; q=quotient    r=remainder
; sign = an extra scratch register to store the signs in.
;
; ie n/d = q + r/d or n=q*d+r
; q is rounded towards zero and r has the same sign as n
; hence -3/2 = -1 remainder -1.
;       3/-2 = -1 remainder 1
;       -3/-2 = 1 remainder -1.
; if d=0 then it returns q=0, r=n (so n=q*d+r !)
; registers must be distinct

        MACRO
        SDIV_64d64_64r64 $ql,$qh,$rl,$rh,$nl,$nh,$dl,$dh,$t,$sign
        ANDS    $sign, $dh, #1<<31              ; get sign of d
        BPL     %F00
        RSBS    $dl, $dl, #0                    ; ensure d +ve
        RSC     $dh, $dh, #0
00
        EORS    $sign, $sign, $nh, ASR#32       ; b31=result b30=sign of n
        BCC     %F01
        RSBS    $nl, $nl, #0                    ; ensure n +ve
        RSC     $nh, $nh, #0
01
        UDIV_64d64_64r64 $ql,$qh,$rl,$rh,$nl,$nh,$dl,$dh,$t ; do the divide
        MOVS    $sign, $sign, LSL#1             ; get out sign bits
        BCC %F02
        RSBS    $ql, $ql, #0
        RSC     $qh, $qh, #0
02
        MOVS    $sign, $sign, LSL#1
        BCC %F03
        RSBS    $rl, $rl, #0                    ; negate remainder
        RSC     $rh, $rh, #0
03
        MEND

 [ make = "_ll_udiv" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_udiv, leaf
   |
        Function _ll_udiv, leaf
   ]
        FunctionEntry ,"r4,r5,r6"
        MOV     r4, r0
        MOV     r5, r1
        MOV     r6, r2
        MOV     lr, r3
   [ THUMB
        BX      pc
        CODE32
   ]
        UDIV_64d64_64r64 r0,r1,r2,r3,r4,r5,r6,lr,ip
        Return  , "r4,r5,r6"
   [ THUMB
        CODE16
   ]
 ]

 [ make = "_ll_sdiv" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_sdiv, leaf
   |
        Function _ll_sdiv, leaf
   ]
        FunctionEntry ,"r4,r5,r6,r7"
        MOV     r4, r0
        MOV     r5, r1
        MOV     r6, r2
        MOV     r7, r3
   [ THUMB
        BX      pc
        CODE32
   ]
        SDIV_64d64_64r64 r0,r1,r2,r3,r4,r5,r6,r7,lr,ip
        Return  , "r4,r5,r6,r7"
   [ THUMB
        CODE16
   ]
 ]

 [ make = "_ll_urdv" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_urdv, leaf
   |
        Function _ll_urdv, leaf
   ]
        FunctionEntry ,"r4,r5,r6"
        MOV     r4, r2
        MOV     r5, r3
        MOV     r6, r0
        MOV     lr, r1
   [ THUMB
        BX      pc
        CODE32
   ]
        UDIV_64d64_64r64 r0,r1,r2,r3,r4,r5,r6,lr,ip
        Return  , "r4,r5,r6"
   [ THUMB
        CODE16
   ]
 ]

 [ make = "_ll_srdv" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_srdv, leaf
   |
        Function _ll_srdv, leaf
   ]
        FunctionEntry ,"r4,r5,r6,r7"
        MOV     r4, r2
        MOV     r5, r3
        MOV     r6, r0
        MOV     r7, r1
   [ THUMB
        BX      pc
        CODE32
   ]
        SDIV_64d64_64r64 r0,r1,r2,r3,r4,r5,r6,r7,lr,ip
        Return  , "r4,r5,r6,r7"
   [ THUMB
        CODE16
   ]
 ]

 [ make = "_ll_not" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_not, leaf
   |
        Function _ll_not, leaf
   ]
        MVN     r0, r0
        MVN     r1, r1
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_and" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_and, leaf
   |
        Function _ll_and, leaf
   ]
        AND     r0, r0, r2
        AND     r1, r1, r3
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_or" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_or, leaf
   |
        Function _ll_or, leaf
   ]
        ORR     r0, r0, r2
        ORR     r1, r1, r3
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_eor" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_eor, leaf
   |
        Function _ll_eor, leaf
   ]
        EOR     r0, r0, r2
        EOR     r1, r1, r3
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_shift_l" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_shift_l, leaf
        SUB     r2, #32
        BPL     %F01
        NEG     r3, r2
        ADD     r2, #32
        ASL     r1, r2
        MOV     ip, r2
        MOV     r2, r0
        LSR     r2, r3
        ORR     r1, r2
        MOV     r2, ip
        ASL     r0, r2
        Return  , "", LinkNotStacked

01      MOV     r1, r0
        LSL     r1, r2
        MOV     r0, #0
        Return  , "", LinkNotStacked
  |
        Function _ll_shift_l, leaf
        SUBS    r3, r2, #32
        BPL     %F01
        RSB     r3, r2, #32
        MOV     r1, r1, ASL r2
        ORR     r1, r1, r0, LSR r3
        MOV     r0, r0, ASL r2
        Return  , "", LinkNotStacked

01      MOV     r1, r0, ASL r3
        MOV     r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_ushift_r" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_ushift_r, leaf
        SUB     r2, #32
        BPL     %F01
        NEG     r3, r2
        ADD     r2, #32
        LSR     r0, r2
        MOV     ip, r2
        MOV     r2, r1
        LSL     r2, r3
        ORR     r0, r2
        MOV     r2, ip
        LSR     r1, r2
        Return  , "", LinkNotStacked

01      MOV     r0, r1
        LSR     r0, r2
        MOV     r1, #0
        Return  , "", LinkNotStacked
  |
        Function _ll_ushift_r, leaf
        SUBS    r3, r2, #32
        BPL     %F01
        RSB     r3, r2, #32
        MOV     r0, r0, LSR r2
        ORR     r0, r0, r1, ASL r3
        MOV     r1, r1, LSR r2
        Return  , "", LinkNotStacked

01      MOV     r0, r1, LSR r3
        MOV     r1, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_sshift_r" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_sshift_r, leaf
        SUB     r2, #32
        BPL     %F01
        NEG     r3, r2
        ADD     r2, #32
        LSR     r0, r2
        MOV     ip, r2
        MOV     r2, r1
        ASL     r2, r3
        ORR     r0, r2
        MOV     r2, ip
        ASR     r1, r2
        Return  , "", LinkNotStacked

01      MOV     r0, r1
        ASR     r1, r1, #31
        ASR     r0, r2
        Return  , "", LinkNotStacked
  |
        Function _ll_sshift_r, leaf
        SUBS    r3, r2, #32
        BPL     %F01
        RSB     r3, r2, #32
        MOV     r0, r0, LSR r2
        ORR     r0, r0, r1, ASL r3
        MOV     r1, r1, ASR r2
        Return  , "", LinkNotStacked

01      MOV     r0, r1, ASR r3
        MOV     r1, r1, ASR #31
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_cmpeq" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_cmpeq, leaf
        CMP     r1, r3
        BNE     %F01
        CMP     r0, r2
        BNE     %F01
        MOV     r0, #1
        Return  , "", LinkNotStacked
01
        MOV     r0, #0
        Return  , "", LinkNotStacked
   |
        Function _ll_cmpeq, leaf
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVEQ   r0, #1
        MOVNE   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_cmpne" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_cmpne, leaf
        CMP     r1, r3
        BNE     %F01
        CMP     r0, r2
        BNE     %F01
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_cmpne, leaf
        SUBS    r0, r0, r2
        CMPEQ   r1, r3
        MOVNE   r0, #1
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_ucmpgt" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_ucmpgt, leaf
        CMP     r1, r3
        BHI     %F01
        BLO     %F02
        CMP     r0, r2
        BHI     %F01
02
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_ucmpgt, leaf
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVHI   r0, #1
        MOVLS   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_ucmpge" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_ucmpge, leaf
        CMP     r1, r3
        BHI     %F01
        BLO     %F02
        CMP     r0, r2
        BHS     %F01
02
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_ucmpge, leaf
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVHS   r0, #1
        MOVLO   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_ucmplt" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_ucmplt, leaf
        CMP     r1, r3
        BHI     %F01
        BLO     %F02
        CMP     r0, r2
        BHS     %F01
02
        MOV     r0, #1
        Return  , "", LinkNotStacked
01
        MOV     r0, #0
        Return  , "", LinkNotStacked
   |
        Function _ll_ucmplt, leaf
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVLO   r0, #1
        MOVHS   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_ucmple" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_ucmple, leaf
        CMP     r1, r3
        BHI     %F01
        BLO     %F02
        CMP     r0, r2
        BHI     %F01
02
        MOV     r0, #1
        Return  , "", LinkNotStacked
01
        MOV     r0, #0
        Return  , "", LinkNotStacked
   |
        Function _ll_ucmple, leaf
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVLS   r0, #1
        MOVHI   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_scmpgt" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_scmpgt, leaf
        CMP     r1, r3
        BGT     %F01
        BLT     %F02
        CMP     r0, r2
        BHI     %F01
02
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_scmpgt, leaf
        CMP     r1, r3
        MOVGT   r0, #1
        MOVLT   r0, #0
        Return  , "", LinkNotStacked, NE
        CMP     r0, r2
        MOVHI   r0, #1
        MOVLS   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_scmpge" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_scmpge, leaf
        CMP     r1, r3
        BGT     %F01
        BLT     %F02
        CMP     r0, r2
        BHS     %F01
02
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_scmpge, leaf
        CMP     r1, r3
        MOVGT   r0, #1
        MOVLT   r0, #0
        Return  , "", LinkNotStacked, NE
        CMP     r0, r2
        MOVHS   r0, #1
        MOVLO   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_scmplt" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_scmplt, leaf
        CMP     r1, r3
        BLT     %F01
        BGT     %F02
        CMP     r0, r2
        BLO     %F01
02
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_scmplt, leaf
        CMP     r1, r3
        MOVLT   r0, #1
        MOVGT   r0, #0
        Return  , "", LinkNotStacked, NE
        CMP     r0, r2
        MOVLO   r0, #1
        MOVHS   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_scmple" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_scmple, leaf
        CMP     r1, r3
        BLT     %F01
        BGT     %F02
        CMP     r0, r2
        BLS     %F01
02
        MOV     r0, #0
        Return  , "", LinkNotStacked
01
        MOV     r0, #1
        Return  , "", LinkNotStacked
   |
        Function _ll_scmple, leaf
        CMP     r1, r3
        MOVLT   r0, #1
        MOVGT   r0, #0
        Return  , "", LinkNotStacked, NE
        CMP     r0, r2
        MOVLS   r0, #1
        MOVHI   r0, #0
        Return  , "", LinkNotStacked
   ]
 ]

 [ make = "_ll_from_l" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_from_l, leaf
        ASR     r1, r0, #31
   |
        Function _ll_from_l, leaf
        MOV     r1, r0, ASR #31
   ]
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_from_u" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_from_u, leaf
   |
        Function _ll_from_u, leaf
   ]
        MOV     r1, #0
        Return  , "", LinkNotStacked
 ]

 [ make = "_ll_to_l" :LOR: make = "all" :LOR: make="shared-library"
   [ THUMB
        Function __16_ll_to_l, leaf
   |
        Function _ll_to_l, leaf
   ]
        Return  , "", LinkNotStacked
 ]

        END


