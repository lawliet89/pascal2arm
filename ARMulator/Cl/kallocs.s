;;; reg_alloc.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        GET     objmacs.s

        CodeArea

        Function __rt_register_allocs
; void __rt_register_allocs(__rt_allocproc *malloc, __rt_freeproc *free);

        FunctionEntry UsesSb
        LDR     ip, addr___rt_malloc
        STMIA   ip, {a1, a2}
        Return  UsesSb

        AdconTable

        IMPORT |__rt_malloc|
addr___rt_malloc
        &       |__rt_malloc|

        END
