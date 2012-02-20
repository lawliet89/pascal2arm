;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1996/03/26 00:11:41 $
;;; Revising $Author: enevill $

        GET     objmacs.s

        CodeArea

        EXPORT  __huge_val

__huge_val DATA
        DCD     0x7fefffff
        DCD     0xffffffff

        END
