;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1994/12/22 18:27:18 $
;;; Revising $Author: enevill $

                GET     objmacs.s

                CodeArea

                CODE16

                EXPORT  __call_via_r0
                EXPORT  __call_via_r1
                EXPORT  __call_via_r2
                EXPORT  __call_via_r3
                EXPORT  __call_via_r4
                EXPORT  __call_via_r5
                EXPORT  __call_via_r6
                EXPORT  __call_via_r7

__call_via_r0   BX      r0
                ALIGN

__call_via_r1   BX      r1
                ALIGN

__call_via_r2   BX      r2
                ALIGN

__call_via_r3   BX      r3
                ALIGN

__call_via_r4   BX      r4
                ALIGN

__call_via_r5   BX      r5
                ALIGN

__call_via_r6   BX      r6
                ALIGN

__call_via_r7   BX      r7
                ALIGN

                END
