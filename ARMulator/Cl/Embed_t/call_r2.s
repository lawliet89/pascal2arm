                GET     objmacs.s

                CodeArea

                CODE16

                EXPORT  __call_r2
__call_r2
                BX      pc
                CODE32
                STMDB   sp!, {lr}
                TST     r2, #1
                ADREQ   lr, %FT0
                ADRNE   lr, %FT1+1
                BX      r2
                CODE16
                ALIGN
1
                BX      pc
                CODE32
0
                LDMIA   sp!, {lr}
                BX      lr

                END
