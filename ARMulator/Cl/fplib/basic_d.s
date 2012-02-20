; basic_d.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.8.22.1 $
; Checkin $Date: 1997/08/21 12:42:51 $
; Revising $Author: ijohnson $

; Basic floating point functions

        GET     fpe.s

        CodeArea |FPL$$Basic|

        IMPORT  __fp_nonveneer_error

;==============================================================================
;Equality
;
;This isn't as simple as it could be. The problem is that unordered numbers
;(i.e. a NaN and a number) compare as not equal. Furthermore +0=-0.
;Thankfully a bitwise compare coming out false catches a lot of the NaN cases
;(NaN and number, two different representations of NaN), so in the "true"
;case we only need to check whether one argument is a NaN (and by implication
;so will the other). After discovering that it looks like a NaN a further
;check is needed to make sure that infinities compare equal.
;The code is optimised for the "not equal" path.

        [ :DEF: eq_s
        
        Export _deq

_deq    EnterWithLR_16
        TEQ     dOP1h, dOP2h
        TEQEQ   dOP1l, dOP2l
        BEQ     DEQ_CheckNaN
        ;Check for -0=0
        ORR     tmp, dOP1h, dOP2h
        TEQ     tmp, #SignBit
        ORREQS  tmp, dOP1l, dOP2l
        MOVEQ   a1, #1
        MOVNE   a1, #0
        ReturnToLR

DEQ_CheckNaN
        
;Numbers are the same. If either is a NaN return false

        MOV     tmp, #&ff000000
        ORR     tmp, tmp, #&00e00000
        CMP     tmp, dOP1h, LSL #1
        MOVHI   a1, #1
        ReturnToLR HI
        CMPEQ   dOP1l, #0
        MOVEQ   a1, #1
        MOVNE   a1, #0
        ReturnToLR

        ]

;==============================================================================
;Inequality
;
;AFAIK neq = NOT(eq), so the same code is used as above with an inverted
;response.

        [ :DEF: neq_s

        Export _dneq

_dneq   EnterWithLR_16
        TEQ     dOP1h, dOP2h
        TEQEQ   dOP1l, dOP2l
        BEQ     DNEQ_CheckNaN

;Check for -0=0. The ORR will yield -0 if the top words look like they will.
;If so we need to check that the bottom words are both zero. This is done
;by ORRing them together. If the result is zero, we return zero, so the
;ORR is done straight into the result register. We must return 1 if the
;numbers aren't zero

        ORR     tmp, dOP1h, dOP2h
        TEQ     tmp, #SignBit
        ORREQS  a1, dOP1l, dOP2l
        ;MOVEQ  a1, #0
        MOVNE   a1, #1
        ReturnToLR

DNEQ_CheckNaN
        MOV     tmp, #&ff000000
        ORR     tmp, tmp, #&00e00000
        CMP     tmp, dOP1h, LSL #1
        MOVHI   a1, #0
        ReturnToLR HI

;Again we check for zero (infinities) by setting the flags moving OP1l to
;the result register. This sets the Z flag if the result is zero. If not we
;write the correct value.

        MOVS    a1, dOP1l
        ;MOVEQ  a1, #0  ;hack
        MOVNE   a1, #1
        ReturnToLR

        ]

;==============================================================================
;General comparison instruction flow: (this is less than)
;
;   OP2=NaN  N---->  OP1=0    N->  OP1>0    N->  OP1=NaN  Y->  *exception1*
;      Y                Y            Y              Y
; *exception2*     (OP1_zero)    (OP1_pos)     (both_neg)
;                       |            |
; *exception1*  <-Y  OP2=NaN      OP2=NaN  Y->  *exception1*
;                       N            |
;         TRUE  <-Y  OP2 -ve      OP2 -ve  Y-> TRUE -> (End)
;           |           N            N
;        OP2=0  <---  FALSE     (both_pos)
;         Y  N                       |
;     FALSE  |               abs(OP1)=abs(OP2)  Y->  frac(OP1)=frac(OP2)  Y-.
;         v  v                       N                        N             |
;        (End)               abs(OP1)<abs(OP2)       frac(OP1)<frac(OP2)    |
;                              Y        N               Y         N         |
;                            TRUE     FALSE           TRUE      FALSE     FALSE
;                              |        |               |         |         |
;                              `--------`-----------> (End) <-----'---------'
;
;            (both_neg)  ->  abs(OP1)=abs(OP2)  Y->  frac(OP1)=frac(OP2)  Y-.  
;                                    N                        N             |  
;                            abs(OP1)>abs(OP2)       frac(OP1)>frac(OP2)    |  
;                              Y        N               Y         N         |  
;                            TRUE     FALSE           TRUE      FALSE     FALSE
;                              |        |               |         |         |  
;                              `--------`-----------> (End) <-----'---------'  
;
; *exception1* and *exception2* check if the NaNs are signalling NaN and
; raise an abort/return 0 as appropiate.
;
;==============================================================================

;Since all the compares have the same structure, a macro is used to define
;them. The fast route through is always the comparison with OP1 -ve and
;OP2 +ve. OP2=0 may be a better choice.


        MACRO
$label  DoubleCompare $lt,$eq

        ASSERT  $lt = 1 :LOR: $lt = 0
        ASSERT  $eq = 1 :LOR: $eq = 0

        IMPORT  __fp_compare_divo_check_exception1
        IMPORT  __fp_compare_divo_check_exception2

;tmp is setup to the mask required to test for a NaN. The test is applied to
;the second operand here. The first operand is checked after the sign bit
;has been strtmped from it, as that is an easier test (the flags are more
;advantageous).

$label  EnterWithLR_16
        MOV     tmp, #&ff000000
        ORR     tmp, tmp, #&00e00000

        CMP     tmp, dOP2h, LSL #1
        BLS     $label._check_OP2       ;Out of line check
$label._OP2_okay
        MOVS    dOP1h, dOP1h, LSL #1
        TEQEQ   dOP1l, #0

;Zeroes need to be special cased, since different signed zeroes are treated
;as equal. A single routine deals with both OP1=0 and OP1=-0. This is then
;the only routine that needs to bother with OP2 being zero. All the other
;routines can relax, safe in the knowledge that OP1 isn't zero.

        BEQ     $label._dOP1_zero
        BCC     $label._dOP1_positive

;This code is copied on the entry to each of the above sub-functions

        CMP     dOP1h, tmp
        CMPEQ   dOP1l, #0
        BHI     __fp_compare_divo_check_exception1

        MOVS    dOP2h, dOP2h, LSL #1
        BHI     $label._both_ops_negative

        MOV     a1, #$lt
        ReturnToLR              ;16S+1N

$label._check_OP2

;We get to here if it looks like OP2 may be a NaN. If the Z flag
;is set then the top word matched the mask - i.e. it could be an
;infinity.
;Could take advantage of the knowledge that OP2 is an infinity, but
;for now, I won't

        CMPEQ   dOP2l, #0
        BEQ     $label._OP2_okay
        B       __fp_compare_divo_check_exception2

$label._dOP1_zero
        CMP     dOP1h, tmp
        CMPEQ   dOP1l, #0
        BHI     __fp_compare_divo_check_exception1

;Conditions here are a little messy. If OP2 is a zero then the numbers
;are equal. If positive, OP2>OP1. If negative OP2<OP1.

        MOVS    dOP2h, dOP2h, LSL #1
        [ $lt = $eq
        TEQEQ   dOP2l, #0       ;check for zero (doesn't affect carry)
        MOVHI   a1, #1-$lt
        MOVLS   a1, #$lt        ;$lt=$eq, so EQ/CC are merged
        ;MOVEQ  a1, #$eq
        |
        MOVCC   a1, #$lt        ;dOP2 is +ve (or zero)
        MOVCS   a1, #1-$lt      ;dOP2 is -ve (or zero)
        TEQEQ   dOP2l, #0       ;test for zero
        MOVEQ   a1, #$eq        ;dOP2 is zero (=dOP1). CS/EQ can't be merged
        ]
        ReturnToLR              ;17/18S+2N

$label._dOP1_positive
        CMP     dOP1h, tmp
        CMPEQ   dOP1l, #0
        BHI     __fp_compare_divo_check_exception1

        MOVS    dOP2h, dOP2h, LSL #1

;If dOP2 is negative then dOP1>dOP2

        MOVCS   a1, #1-$lt
        ReturnToLR CS           ;16S+2N

$label._both_ops_positive
        CMP     dOP1h, dOP2h

;If the exponents/high fractions are equal then we need to compare the
;bottom fractions.

        ;MOVEQ  tmp, dOP1h, LSL #8
        ;CMPEQ  tmp, dOP2h, LSL #8
        CMPEQ   dOP1l, dOP2l

;This conditional assembly looks hairy, but in fact it is just the four-way
;case (0,0, 0,1, 1,0, 1,1) compressed into two cases

        [ $lt <> $eq
        MOVCC   a1, #$lt
        MOVCS   a1, #$eq        ;1-$lt=$eq, so EQ/HI cases merged
        ;MOVEQ  a1, #$eq
        ;MOVHI  a1, #1-$lt
        |
        MOVLS   a1, #$lt        ;$lt=$eq, so EQ/CC cases merged
        ;MOVCC  a1, #$lt
        ;MOVEQ  a1, #$eq
        MOVHI   a1, #1-$lt
        ]
        ReturnToLR              ;24S+2N

$label._both_ops_negative
        CMP     dOP1h, dOP2h
        ;MOVEQ  tmp, dOP1h, LSL #8
        ;CMPEQ  tmp, dOP2h, LSL #8
        CMPEQ   dOP1l, dOP2l

;Since both operands are -ve, the result of a comparison is the opposite
;of that were they +ve. Also the "same" cases need to be merged differently

        [ $lt <> $eq
        MOVHI   a1, #$lt
        MOVLS   a1, #$eq
        |
        MOVCC   a1, #1-$lt
        MOVCS   a1, #$lt
        ]
        ReturnToLR              ;22S+2N

        MEND

;==============================================================================
;Less Than

        [ :DEF: ls_s

        Export _dls
_dls    DoubleCompare 1,0

        ]

;==============================================================================
;Less Than or Equal

        [ :DEF: leq_s

        Export _dleq
_dleq   DoubleCompare 1,1

        ]

;==============================================================================
;Greater Than

        [ :DEF: gr_s

        Export _dgr
_dgr    DoubleCompare 0,0

        ]

;==============================================================================
;Greater Than or Equal

        [ :DEF: geq_s

        Export _dgeq
_dgeq   DoubleCompare 0,1

        ]

;==============================================================================
;Invalid Operation checking (NaN on compares)

        [ :DEF: compare_s

        EXPORT  __fp_compare_divo_check_exception1
        EXPORT  __fp_compare_divo_check_exception2

; Always called from ARM Code

        [ :DEF: thumb
        CODE32
        ]

__fp_compare_divo_check_exception1
        ;If we get here then we know dOP2 isn't a NaN, but dOP1 is adjusted
        ;and may be a signalling NaN.
        TST     dOP1h, #dSignalBit<<1
        MOVNE   a1, #0
        ReturnToLR NE
        B       compare_ivo
 
__fp_compare_divo_check_exception2
        ;If we get here it is because the exception test at the top of the
        ;function failed. Hence neither dOP hasn't been adjusted yet
        TST     dOP2h, #dSignalBit
        BEQ     compare_ivo
        ;Test that dOP1 isn't a NaN
        CMP     tmp, dOP1h, LSL #1
        MOVHI   a1, #0
        ReturnToLR HI
        CMPEQ   dOP1l, #0
        MOVEQ   a1, #0
        ReturnToLR EQ
        TST     dOP1h, #dSignalBit
        MOVNE   a1, #0
        ReturnToLR NE
compare_ivo
        MOV     a4,#IVO_bit:OR:DoubleErr_bit
        B       __fp_nonveneer_error

        ]

;==============================================================================

        END
