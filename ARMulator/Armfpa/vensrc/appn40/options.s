; Assembler source for FPA support code - simple stand-alone veneer
;
; Copyright (C) Advanced RISC Machines Limited, 1997. All rights reserved.
;
; RCS $Revision: 1.2 $
; Checkin $Date: 1997/04/22 18:46:29 $
; Revising $Author: dseal $

                        GBLL    FPEWanted
FPEWanted               SETL    {FALSE}

                        GBLL    FPASCWanted
FPASCWanted             SETL    {TRUE}

                        GBLS    UndefHandlerConvention
UndefHandlerConvention  SETS    "StandAlone"

                        GBLL    MultipleContexts
MultipleContexts        SETL    {FALSE}

                        GBLL    EnableInterrupts
EnableInterrupts        SETL    {FALSE}

                        GBLS    FPRegInitValue
FPRegInitValue          SETS    "SigNaN"

                        GBLA    FPSRInitValue
FPSRInitValue           SETA    &000200         ;NE=1 for IEEE compliance

TrapsCanReturn          SETL    {FALSE}

NoTranscendentals       SETL    {TRUE}

NoPacked                SETL    {TRUE}

        END
