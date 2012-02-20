# Makefile for the Application Note 40 configuration of the FPASC.
#
# Copyright (C) Advanced RISC Machines Limited, 1997. All rights reserved.
#
# RCS $Revision: 1.3 $
# Checkin $Date: 1997/04/22 18:41:16 $
# Revising $Author: dseal $

COREDIR   = ../coresrc
VENDIR    = ../vensrc/AppNote40
COREFILES = arith armdefs defaults fpadefs fpaundef fpeundef interf \
            ldst macros main nans optcheck regnames rounding toplevel \
            transhw wsdefs
VENFILES  = ctxtws end globalws header options start

fpasc.o: $(VENFILES:%=$(VENDIR)/%.s) $(COREFILES:%=$(COREDIR)/%.s)
        armasm -apcs 3/32bit -i $(COREDIR) -i $(VENDIR) -o $@ $(VENDIR)/header.s
