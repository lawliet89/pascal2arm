;;; riscos/osveneer.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     h_brazil.s
        GET     objmacs.s

        CodeArea

 [ make = "osargs" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osargs   ; compatibility
        Function __riscos_osargs

        FunctionEntry UsesSb
        MOV     ip, a1
        ORR     ip, ip, a2
        SWI     Args
        BVS     |__riscos_SWIErrorExit|
        CMP     ip, #0
        MOVNE   a1, r2
        Return  UsesSb, ""
 ]

 [ make = "osbget" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osbget  ; compatibility
        Function __riscos_osbget

        FunctionEntry UsesSb
        MOV     r1, a1
        SWI     BGet
        BVS     |__riscos_SWIErrorExit|
        MOVCS   a1, #-1
        Return  UsesSb, ""
 ]

 [ make = "osbput" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osbput  ; compatibility
        Function __riscos_osbput

        FunctionEntry UsesSb
        SWI     BPut
        Return  UsesSb, "",, VC
        BVS     |__riscos_SWIErrorExit|
 ]

 [ make = "osbyte" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osbyte  ; compatibility
        Function __riscos_osbyte

        FunctionEntry UsesSb
        SWI     Byte
        BVS     |__riscos_SWIErrorExit|
        AND     a1, a2, #&ff
        ORR     a1, a1, a3, ASL #8
        MOV     a1, a1, ASL #16
        ADC     a1, a1, #0
        MOV     a1, a1, ROR #16
        Return  UsesSb, ""
 ]

 [ make = "oscli" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_oscli  ; compatibility
        Function __riscos_oscli

        FunctionEntry UsesSb
        SWI     CLI
        BVS     |__riscos_SWIErrorExit|
        MOV     a1, #1      ; return 1 if OK
        Return  UsesSb, ""
 ]

 [ make = "osfile" :LOR: make = "all" :LOR: make = "shared-library"
        IMPORT  |__rt_CopyError|

        Function _kernel_osfile  ; compatibility
        Function __riscos_osfile

; typedef struct {
;         int load, exec;
;         int start, end;
; } __riscos_osfile_block;
; int __riscos_osfile(int op, const char *name, __riscos_osfile_block *inout);
        FunctionEntry UsesSb, "r4, r5, r6"
        MOV     r6, a3
        LDMIA   a3, {r2 - r5}
        SWI     File
        STMIA   r6, {r2 - r5}
        Return  UsesSb, "r4, r5, r6",, VC

        BL      |__rt_CopyError|
        MOV     a1, #-2
        Return  UsesSb, "r4, r5, r6"
 ]

 [ make = "osfind" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osfind_open  ; compatibility
        Function __riscos_osfind_open
        Function _kernel_osfind_close  ; compatibility
        Function __riscos_osfind_close

        FunctionEntry UsesSb
        SWI     Open
        Return  UsesSb, "",, VC
        BVS     |__riscos_SWIErrorExit|
 ]

 [ make = "osgbpb" :LOR: make = "all" :LOR: make = "shared-library"
        IMPORT  |__rt_CopyError|

        Function _kernel_osgbpb  ; compatibility
        Function __riscos_osgbpb

; typedef struct {
;         void * dataptr;
;         int nbytes, fileptr;
;         int buf_len;
;         char * wild_fld;
; } __riscos_osgbpb_block;
; int __riscos_osgbpb(int op, unsigned handle, __riscos_osgbpb_block *inout);
        FunctionEntry UsesSb, "r4, r5, r6, r7"
        MOV     r7, a3
        LDMIA   a3, {r2 - r6}
        SWI     Multiple
        STMIA   r7, {r2 - r6}
        BVS     %F01
        MOVCS   a1, #-1
        Return  UsesSb, "r4, r5, r6, r7"

01
        BL      |__rt_CopyError|
        MOV     a1, #-2
        Return  UsesSb, "r4, r5, r6, r7"
 ]

 [ make = "osrdch" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osrdch  ; compatibility
        Function __riscos_osrdch

        FunctionEntry UsesSb
        SWI     ReadC
        BVS     |__riscos_SWIErrorExit|
        Return  UsesSb, "",, CC
        CMPS    a1, #27         ; escape
        MOVEQ   a1, #-27
        MOVNE   a1, #-1         ; other error, EOF etc
        Return  UsesSb, ""
 ]

 [ make = "osword" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_osword  ; compatibility
        Function __riscos_osword

        FunctionEntry UsesSb
        SWI     Word
        BVS     |__riscos_SWIErrorExit|
        MOV     a1, r1
        Return  UsesSb, ""
 ]

 [ make = "oswrch" :LOR: make = "all" :LOR: make = "shared-library"
  [ make <> "all" :LAND: make <> "shared-library"
        IMPORT  |__riscos_SWIErrorExit|
  ]

        Function _kernel_oswrch  ; compatibility
        Function __riscos_oswrch

        FunctionEntry UsesSb
        SWI     WriteC
        BVS     |__riscos_SWIErrorExit|
        Return  UsesSb, ""
 ]

 [ make = "swi" :LOR: make = "all" :LOR: make = "shared-library"
        IMPORT  |__rt_CopyError|

        Function _kernel_swi_c  ; compatibility
        Function __riscos_swi_c

        ; Set up a proper frame here, so if an error happens (and not X)
        ; a sensible traceback can be given.

        FunctionEntry UsesSb, "a3,a4,v1,v2,v3,v4,v5", MakeFrame
        ; be kind to fault handler if there is an error.
        ADR     a4, AfterSWI
        SUB     a4, a4, sp
        MOV     a4, a4, LSR #2
        BIC     a4, a4, #&ff000000
        ADD     a4, a4, #&ea000000      ; B always
        TST     a1, #&80000000          ; non-X bit requested?
        ORR     a1, a1, #&EF000000      ; SWI + Always
        ORREQ   a1, a1, #X
        STMFD   sp!, {a1, a4, r9}
        LDMIA   a2, {r0 - r9}
        MOV     pc, sp
AfterSWI
        ADD     sp, sp, #8
        LDMFD   sp!, {ip, lr}
        STMIA   lr, {r0 - r9}
        MOV     r9, ip
        LDMFD   sp!, {lr}
        MOV     a2, #0
        MOVCS   a2, #1
        MOVVS   a2, #0
        STR     a2, [lr]
        MOVVC   a1, #0
        BLVS    |__rt_CopyError|
        Return  UsesSb, "v1,v2,v3,v4,v5", fpbased

        Function _kernel_swi  ; compatibility
        Function __riscos_swi

        SUB     a4, sp, #11*4           ; write state of carry flag one word
        B       |__riscos_swi_c|        ; below sp at time of writing.
 ]

 [ make = "k_getenv" :LOR: make = "all" :LOR: make = "shared-library"
        IMPORT  |__rt_CopyError|

        Function _kernel_getenv  ; compatibility
        Function __riscos_getenv

; __riscos_error *__riscos_getenv(const char *name, char *buffer, unsigned size);
        FunctionEntry UsesSb, "v1,v2", MakeFrame
        MOV     r3, #0
        MOV     r4, #3
        SWI     X:OR:ReadVarVal
        MOVVC   a1, #0
        STRVCB  a1, [r1, r2]
        BLVS    |__rt_CopyError|
        Return  UsesSb, "v1, v2", fpbased
 ]

 [ make = "k_setenv" :LOR: make = "all" :LOR: make = "shared-library"
        IMPORT  |__rt_CopyError|

        Function _kernel_setenv  ; compatibility
        Function __riscos_setenv

; __riscos_error *__riscos_setenv(const char *name, const char *value);
        FunctionEntry UsesSb, v1
        ; Apparently, we need to say how long the value string is as well
        ; as terminating it.
        MOV     a3, #0
01      LDRB    ip, [a2, a3]
        CMP     ip, #0
        ADDNE   a3, a3, #1
        BNE     %B01
        MOV     r3, #0
        MOV     r4, #0
        SWI     X:OR:SetVarVal
        MOVVC   a1, #0
        BLVS    |__rt_CopyError|
        Return  UsesSb, "v1"
 ]

 [ make = "k_hostos" :LOR: make = "all" :LOR: make = "shared-library"
        Function _kernel_hostos, "leaf"  ; compatibility, defunct?

        FunctionEntry
        MOV     r0, #0
        MOV     r1, #1
        SWI     Byte
        MOV     a1, r1
        Return
 ]

 [ make = "lasterr" :LOR: make = "all" :LOR: make = "shared-library"
        Function _kernel_last_oserror  ; compatibility
        Function __riscos_last_oserror

        FunctionEntry UsesSb
        LDR     ip, addr___rt_errorBuffer
        LDR     a1, [ip, #0]
        CMP     a1, #0
        ADDNE   a1, ip, #4
        MOVNE   a2, #0
        STRNE   a2, [ip, #0]
        Return  UsesSb, ""
 ]

 [ make = "k_errexit" :LOR: make = "all" :LOR: make = "shared-library"
  [ make = "k_errexit"
        EXPORT  |__riscos_SWIErrorExit|
  ]
        IMPORT  |__rt_CopyError|

|__riscos_SWIErrorExit|
        BL      |__rt_CopyError|
        MOV     a1, #-2
        Return  UsesSb, ""
 ]

        AdconTable
 [ make = "lasterr" :LOR: make = "all" :LOR: make = "shared-library"
        IMPORT  |__rt_errorBuffer|
addr___rt_errorBuffer
        &       |__rt_errorBuffer|
 ]

        END
