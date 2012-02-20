;;; image.s:  make symbols describing the shape of the image
;;;           available to C code.
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.2 $
;;; Checkin $Date: 1995/03/16 12:53:03 $
;;; Revising $Author: enevill $

        AREA |C$$code|, CODE, READONLY

        IMPORT  |Image$$RO$$Base|
        IMPORT  |Image$$RO$$Limit|
        IMPORT  |Image$$RW$$Limit|

        EXPORT |_RO_Base|
        EXPORT |_RO_Limit|
        EXPORT |_RW_Limit|

|_RO_Base|      DATA
                DCD     |Image$$RO$$Base|
|_RO_Limit|     DATA
                DCD     |Image$$RO$$Limit|
|_RW_Limit|     DATA
                DCD     |Image$$RW$$Limit|

        END
