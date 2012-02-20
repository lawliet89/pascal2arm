;;; riscos/_tmpnam.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     h_brazil.s
        GET     objmacs.s

        CodeArea

        ; temporary files in <wimp$scrapdir> if that is defined,
        ; otherwise in $.tmp
        ; implements (but without the dependencies on other library fns)
        ;
        ; void _sys_tmpnam_(char *name, int sig, int buflen)
        ; {
        ;   if (_kernel_getenv("wimp$scrapdir", name, buflen-10) != NULL)
        ;     strcpy(name, "$.tmp");
        ;   name += strlen(name);
        ;   sprintf(name, ".x%.8x", sig);
        ; }

        Function _sys_tmpnam
        FunctionEntry ,"r1,r4"
        MOV     r1, r0
        ADR     r0, scrapdir
        SUB     r2, r2, #11
        MOV     r3, #0          ; first call for this name
        MOV     r4, #3          ; expand if a macro
        SWI     X:OR:ReadVarVal
        ADDVC   r1, r1, r2      ; r2 = length of value if read succeeds
        ADRVS   r2, tmp
        LDMVSIA r2, {r2, r3}
        STMVSIA r1, {r2, r3}
        ADDVS   r1, r1, #5

        MOV     r0, #"."
        STRB    r0, [r1], #+1
        MOV     r0, #"x"
        STRB    r0, [r1], #+1

        MOV     r2, #9          ; buffer size
        LDMFD   sp!, {r0}
        SWI     X:OR:&d4        ; convert to 8 hex digits in buffer r1

        Return ,"r4"

tmp
        =    "$.tmp"
scrapdir
        =    "wimp$scrapdir", 0

        ALIGN

        END
