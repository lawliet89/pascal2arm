        SUBT    Run-time support macros for assembler source         > macros.s
        ; ---------------------------------------------------------------------
        ;
        ; $Revision: 1.3 $
        ;   $Author: amerritt $
        ;     $Date: 1996/07/08 13:22:20 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ; ---------------------------------------------------------------------

        ASSERT  (listopts_s)
old_opt SETA    {OPT}
        OPT     (opt_off)   ; disable listing of include files

        ; ---------------------------------------------------------------------

        [       (:LNOT: :DEF: macros_s)

                GBLL    macros_s
macros_s        SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; bit
        ; ---
        ; This provides a shorthand method of describing individual bit
        ; positions.
        ;
        MACRO
$label  bit     $index
$label  *       (1 :SHL: $index)
        MEND

        ; ---------------------------------------------------------------------
        ; NPOW2
        ; -----
        ; Calculate the next-power-of-2 number above the value given.
        ;
        MACRO
$label  NPOW2   $value
        LCLA    newval
newval  SETA    1
        WHILE   (newval < $value)
newval  SETA    (newval :SHL: 1)
        WEND
$label  EQU     (newval)
        ; Allow the user to see how much "wasted" space is being
        ; generated within the object:
        !       0,"NPOW2: original &" :CC: (:STR: $value) :CC: " new &" :CC: (:STR: newval) :CC: " wasted &" :CC: (:STR: (newval - $value))
        MEND

        ; ---------------------------------------------------------------------

        ]       ; macros_s

        OPT     (old_opt)   ; restore previous listing options

        ; ---------------------------------------------------------------------
        END     ; EOF macros.s
