; Assembler source for FPA support code - simple stand-alone veneer
;
; Copyright (C) Advanced RISC Machines Limited, 1997. All rights reserved.
;
; RCS $Revision: 1.2 $
; Checkin $Date: 1997/04/22 18:46:21 $
; Revising $Author: dseal $

                GBLS    VeneerName      ;The name of the veneer to use
VeneerName      SETS    "simple"

                GBLS    CoreDir
CoreDir         SETS    ""

                GBLS    VeneerDir
VeneerDir       SETS    ""

                GBLS    FileExt
FileExt         SETS    ".s"

        GET     $CoreDir.main$FileExt

        END
