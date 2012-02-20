;;; h_uwb.s: format of a __rt_unwindblock
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

                ^       0
uwb_r4          #       4
uwb_r5          #       4
uwb_r6          #       4
uwb_r7          #       4
uwb_r8          #       4
uwb_r9          #       4
uwb_fp          #       4
uwb_sp          #       4
uwb_pc          #       4
uwb_sl          #       4
uwb_f4          #       3*4
uwb_f5          #       3*4
uwb_f6          #       3*4
uwb_f7          #       3*4
uwb_size        #       0

        END
