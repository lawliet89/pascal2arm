;;; h_signal.s: Definitions of default signal-handler values
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.2 $
;;; Checkin $Date: 1995/04/12 08:42:02 $
;;; Revising $Author: enevill $

        EXPORT  |__SIG_DFL|
        EXPORT  |__SIG_ERR|
        EXPORT  |__SIG_IGN|

|__SIG_DFL|     *       -1
|__SIG_ERR|     *       -2
|__SIG_IGN|     *       -3

        AREA    |C$$code|, CODE, READONLY

        END
