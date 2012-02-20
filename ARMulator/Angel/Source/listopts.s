        SUBT    armasm OPT directive values                        > listopts.s
        ; ---------------------------------------------------------------------
        ; Assembler support header file.
        ;
        ; $Revision: 1.2 $
        ;   $Author: amerritt $
        ;     $Date: 1996/07/08 13:22:18 $
        ;
        ; listopts.s
        ; ----------
        ; This file should be included before including any other armasm
        ; "include" files. This file defines the OPT directive parameters
        ; that control listing when the "-print" command line option is
        ; specified.
        ;
        ; Copyright Advanced RISC Machines Limited, 1995. 
        ; All Rights Reserved
        ; ---------------------------------------------------------------------

        [       (:LNOT: :DEF: listopts_s)

                GBLL         listopts_s
listopts_s      SETL         {TRUE}

        ; ---------------------------------------------------------------------
        ; Printing options

opt_on          *       (1 :SHL: 0)     ; listing on
opt_off         *       (1 :SHL: 1)     ; listing off
opt_ff          *       (1 :SHL: 2)     ; form-feed
opt_reset       *       (1 :SHL: 3)     ; reset line number to zero
opt_von         *       (1 :SHL: 4)     ; variable listing on
opt_voff        *       (1 :SHL: 5)     ; variable listing off
opt_mon         *       (1 :SHL: 6)     ; macro expansion on
opt_moff        *       (1 :SHL: 7)     ; macro expansion off
opt_mcon        *       (1 :SHL: 8)     ; macro calls on
opt_mcoff       *       (1 :SHL: 9)     ; macro calls off
opt_p1on        *       (1 :SHL: 10)    ; pass 1 listing on
opt_p1off       *       (1 :SHL: 11)    ; pass 1 listing off
opt_con         *       (1 :SHL: 12)    ; conditional directives on
opt_coff        *       (1 :SHL: 13)    ; conditional directives off
opt_mendon      *       (1 :SHL: 14)    ; mend directives on
opt_mendoff     *       (1 :SHL: 15)    ; mend directives off

        OPT     (opt_on :OR: opt_voff :OR: opt_moff :OR: opt_mcon :OR: opt_p1off :OR: opt_coff :OR: opt_mendoff)
        GBLA    old_opt
old_opt SETA    {OPT}
        ; The "old_opt" variable should be used as a holder for the
        ; user specified options over future header files. This gives
        ; control over hiding the contents of include files in
        ; listings.
        ; ---------------------------------------------------------------------

        NOFP    ; assembler option to ensure no FP instructions generated

        [       ({CONFIG} <> 32)
        ASSERT  (1 = 0) ; This code is designed for 32bit ARM systems only
        ]

FALSE           EQU     0       ; standard boolean control values
TRUE            EQU     1

        ; ---------------------------------------------------------------------

        ]       ; listopt_s

        ; ---------------------------------------------------------------------
        END     ; listopts.s
