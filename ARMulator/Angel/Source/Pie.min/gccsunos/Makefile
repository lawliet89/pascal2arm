#
# $Revision: 1.5.6.3 $
#   $Author: dbrooke $
#     $Date: 1998/03/10 16:03:54 $
#
# Copyright (c) 1996, 1997 Advanced RISC Machines Limited.
# All Rights Reserved
#

# The following options on the make command line are supported:
#
# ENDIAN=BIG   build for big endian system (default is little endian)
#
#

ANGSD=		../../../angsd
ARMDBG=		../../../armdbg

LATE_STARTUP=	0
THUMB_SUPPORT=	0
MINIMAL_ANGEL=	1

RWADDR=		0x00000000
# 0x60000 is a typical place to run from RAM - uncomment if you want this
ROADDR=		0x00060000
# 0xC0000000 is where the ROM resides - uncomment this to run from ROM
#ROADDR=	0xC0000000
ANGELVSN=	0x0001

# Define this only if DEBUG_METHOD is set to logterm in devconf.h
# Currently this won't work -- the PIE doesn't have two serial ports.
LOGTERM_DEBUGGING=0

#
# Pick one of the following modules to link first:
#
# startrom.o - Used when ROM can be unmapped from 0
#   except.o - Used when ROM is permanently mapped at 0
#
FIRST=		'startrom.o(ROMStartup)'
#FIRST=		'except.o(__Vectors)'

# This is the default endianess - it can be overwritten on the
# make command line by ENDIAN=BIG
ENDIAN = LITTLE

LITTLE_ENDIANFLAG= -li
LITTLE_ENDIANDEFN= -DLITTLE_ENDIAN
LITTLE_ENDIANSFX= l
LITTLE_ENDIANOBJDIR= little

BIG_ENDIANFLAG=	-bi
BIG_ENDIANDEFN=	-DBIG_ENDIAN
BIG_ENDIANSFX=	b
BIG_ENDIANOBJDIR= big

ENDIANFLAG= $($(ENDIAN)_ENDIANFLAG)
ENDIANDEFN= $($(ENDIAN)_ENDIANDEFN)
ENDIANSFX= $($(ENDIAN)_ENDIANSFX)

TYPEOBJDIR_0= _rel
TYPEOBJDIR_1= _deb

OBJDIR = $($(ENDIAN)_ENDIANOBJDIR)$(TYPEOBJDIR_$(DEBUG))

#
# now make compiler and assembler arguments
#
CCENDIAN=	$(ENDIANFLAG) $(ENDIANDEFN)
ASENDIAN=	$(ENDIANFLAG)

LFLAGS=		-symbols $@.sym.lst

CDEFS=	-DTARGET -DTHUMB_SUPPORT=$(THUMB_SUPPORT) -DANGELVSN=$(ANGELVSN) \
	-DROADDR=$(ROADDR) -DLOGTERM_DEBUGGING=$(LOGTERM_DEBUGGING) \
	-DRWADDR=$(RWADDR) -DMINIMAL_ANGEL=$(MINIMAL_ANGEL) \
	-DLATE_STARTUP=$(LATE_STARTUP) -DDEBUG=$(DEBUG) \
	-DASSERT_ENABLED=$(ASSERT_ENABLED) 

# Set this to 1 for debugging version of Angel, 0 for Release.
DEBUG=          1

# Uncomment this for debugging version
# Extra debugging flags: choose from:
#   NO_RXE_DATA         -- don't print received data bytes.
#   CLEAR_BUFFERS       -- clear Angel buffers on allocation/deallocation
#   NO_LOG_INFO         -- Allow LogWarning/LogError, but kill LogInfo. Saves ~35K code
#   DEBUG_FILE_TOO      -- include file names in debug info available (define format too)
#   ASSERT_ENABLED      -- Allow Assertions with ASSERT(cond, info) macro
#
#      [ also:   -DNO_PIDULATOR -DSKEPTIC -DDEBUG -DKDBG -DM_HISTORY ]
#

# ANGEL_DEBUG = -g+
# ANGEL_DEBUG = -g+ -DCLEAR_BUFFERS
# ANGEL_DEBUG = -g+ -DNO_LOG_INFO
ANGEL_DEBUG= -O

ASSERT_ENABLED= 0

# Leave this one alone - comment, uncomment and edit the two macros above
# as necessary
#
ALL_DEBUG=	$(ANGEL_DEBUG)

CFLAGS= -fc -fah -apcs 3/32bit -c \
	 -I../..,../../pie -I$(CLX) -I$(ARMDBG) \
	 $(CCENDIAN) $(CFL) $(CDEFS) $(ALL_DEBUG)

AFLAGS= -32 -apcs 3/swst/32bit $(ASENDIAN) \
	-I$(OBJDIR) \
	-I../.. -I../../pie -I.. -I../../../500/cl \
	-PD "LOGTERM_DEBUGGING SETA $(LOGTERM_DEBUGGING)" \
	-PD "ANGELVSN SETA $(ANGELVSN)" \
	-PD "DEBUG SETA $(DEBUG)" \
	-PD "LATE_STARTUP SETA $(LATE_STARTUP)" \
	-PD "ROADDR SETA $(ROADDR)" \
	-PD "THUMB_SUPPORT SETA $(THUMB_SUPPORT)" \
	-PD "ASSERT_ENABLED SETA $(ASSERT_ENABLED)" \
	-PD "MINIMAL_ANGEL SETA $(MINIMAL_ANGEL)"
#
# set ARMTOOLS to point to the tools you wish to
# use (otherwise the default ones will be used)
#
# Note that tasm is used since this means the makefile does not have to
# be modified between enabling and disabling Thumb support
#
ARMTOOLS=
CC=		$(ARMTOOLS)armcc
AS=		$(ARMTOOLS)tasm
LD=		$(ARMTOOLS)armlink
AR=		$(ARMTOOLS)armlib

OBJS=	$(OBJDIR)/serlasm.o $(OBJDIR)/devices.o $(OBJDIR)/devraw.o \
	$(OBJDIR)/devmisc.o $(OBJDIR)/params.o $(OBJDIR)/buffers.o \
	$(OBJDIR)/logging.o $(OBJDIR)/except.o $(OBJDIR)/startlib.o \
	$(OBJDIR)/serraw.o $(OBJDIR)/stacks.o $(OBJDIR)/serlock.o

DRIVEROBJS=	$(OBJDIR)/serial.o $(OBJDIR)/panicblk.o $(OBJDIR)/ladpasm.o

all: OBJECT_DIR $(OBJDIR)/appldev

clean:
	if [ -d $(OBJDIR) ]; then \
	  $(RM) $(DRIVEROBJS) $(OBJS) $(OBJDIR)/makelo $(OBJDIR)/drivers.a $(OBJDIR)/lolevel.s \
	  		$(OBJDIR)/startrom.o $(OBJDIR)/rtstand.o $(OBJDIR)/angmin.lib \
			$(OBJDIR)/supplib.o $(OBJDIR)/suppmsg.o $(OBJDIR)/appldev \
			$(OBJDIR)/appldev.o $(OBJDIR)/*.sym $(OBJDIR)/*.lst ; \
	  $(RMDIR) $(OBJDIR); \
	fi

OBJECT_DIR:
	@if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi


CFLsuppmsg = -DMSG_UTILS_ONLY

CLIB = ../../../500/cl/angelarm/armlib.o
CLSTAND = ../../../clstand
RTSTAND_O =

$(OBJDIR)/appldev:	$(OBJDIR)/appldev.o $(OBJDIR)/suppmsg.o \
			$(OBJDIR)/angmin.lib $(OBJDIR)/rtstand.o
	$(LD) -o $@ $(OBJDIR)/appldev.o $(OBJDIR)/suppmsg.o $(OBJDIR)/angmin.lib \
		$(OBJDIR)/startrom.o $(OBJDIR)/rtstand.o	\
		-bin -nozeropad -base $(ROADDR) -data $(RWADDR)		\
		-first 'startrom.o(ROMStartup)' -symbols $@.sym.lst
	$(LD) -o $@.sym $(OBJDIR)/appldev.o $(OBJDIR)/suppmsg.o $(OBJDIR)/angmin.lib \
		$(OBJDIR)/startrom.o $(OBJDIR)/rtstand.o \
		-aif -bin -nozeropad -base $(ROADDR) -data $(RWADDR)	\
		-first 'startrom.o(ROMStartup)'  -entry $(ROADDR)

$(OBJDIR)/appldev.o:	../../examples/appldev.c
	$(CC) $(CFLAGS) $(CFLappldev) -o $@ ../../examples/appldev.c

$(OBJDIR)/rtstand.o:	../../examples/rtstand.s
	$(AS) ../../examples/rtstand.s $(AFLAGS) -o $@

$(OBJDIR)/angmin.lib:	$(OBJS) $(OBJDIR)/supplib.o $(OBJDIR)/startrom.o \
			$(OBJDIR)/drivers.a
	$(LD) -aof -o $@ $(OBJS) $(OBJDIR)/supplib.o $(OBJDIR)/drivers.a

$(OBJDIR)/drivers.a:	$(DRIVEROBJS)
	$(AR) -c -o $@ $(DRIVEROBJS)

#
# devconf.h has sanity checks upon definitions in this Makefile
#
../../pie/devconf.h:	Makefile

# Rules for creating dependencies, to standard output, which can then be
#  cut and pasted back into this makefile above.

depends: devices.dep devmisc.dep devraw.dep buffers.dep params.dep
depends: logging.dep suppmsg.dep serial.dep serraw.dep
depends: panicblk.dep stacks.dep serlock.dep

SORTSED=sort -u | sed -e 's/^/$$(OBJDIR)\//'

devices.dep:	../../pie/devices.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevices) ../../pie/devices.c | $(SORTSED)
	@echo '$$(OBJDIR)/devices.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevices) -o $$(OBJDIR)/devices.o $?'
	@echo

devmisc.dep:	../../devmisc.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevmisc) ../../devmisc.c | $(SORTSED)
	@echo '$$(OBJDIR)/devmisc.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevmisc) -o $$(OBJDIR)/devmisc.o $?'
	@echo

devraw.dep:	../../devraw.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevraw) ../../devraw.c | $(SORTSED)
	@echo '$$(OBJDIR)/devraw.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevraw) -o $$(OBJDIR)/devraw.o $?'
	@echo

buffers.dep:	../../buffers.c
	@$(CC) -Mx $(CFLAGS) $(CFLbuffers) ../../buffers.c | $(SORTSED)
	@echo '$$(OBJDIR)/buffers.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLbuffers) -o $$(OBJDIR)/buffers.o $?'
	@echo

params.dep:	../../params.c
	@$(CC) -Mx $(CFLAGS) $(CFLparams) ../../params.c | $(SORTSED)
	@echo '$$(OBJDIR)/params.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLparams) -o $$(OBJDIR)/params.o $?'
	@echo

logging.dep:	../../logging.c
	@$(CC) -Mx $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c | $(SORTSED)
	@echo '$$(OBJDIR)/logging.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLlogging) -I../../logging -o $$(OBJDIR)/logging.o $?'
	@echo

suppmsg.dep:	../../msgbuild.c
	@$(CC) -Mx $(CFLAGS) $(CFLsuppmsg) ../../msgbuild.c | $(SORTSED)
	@echo '$$(OBJDIR)/suppmsg.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLsuppmsg) -o $$(OBJDIR)/suppmsg.o $?'
	@echo

serial.dep:	../../pie/serial.c
	@$(CC) -Mx $(CFLAGS) $(CFLserial) ../../pie/serial.c | $(SORTSED)
	@echo '$$(OBJDIR)/serial.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLserial) -o $$(OBJDIR)/serial.o $?'
	@echo

serraw.dep:	  ../../serraw.c
	@$(CC) -Mx $(CFLAGS) $(CFLserraw) ../../serraw.c | $(SORTSED)
	@echo '$$(OBJDIR)/serraw.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLserraw) -o $$(OBJDIR)/serraw.o $?'
	@echo

panicblk.dep:	  ../../logging/panicblk.c
	@$(CC) -Mx $(CFLAGS) $(CFLpanicblk) ../../logging/panicblk.c | $(SORTSED)
	@echo '$$(OBJDIR)/panicblk.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLpanicblk) -o $$(OBJDIR)/panicblk.o $?'
	@echo

serlock.dep:	../../serlock.c
	@$(CC) -Mx $(CFLAGS) $(CFLserlock) ../../serlock.c | $(SORTSED)
	@echo '$$(OBJDIR)/serlock.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLserlock) -o $$(OBJDIR)/serlock.o $?'
	@echo

stacks.dep:	../../stacks.c
	@$(CC) -Mx $(CFLAGS) $(CFLstacks) ../../stacks.c | $(SORTSED)
	@echo '$$(OBJDIR)/stacks.o:	../../pie/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLstacks) -o $$(OBJDIR)/stacks.o $?'
	@echo

# And here are the assmebler dependencies - probably not 100% correct

$(OBJDIR)/startrom.o:	../../startrom.s
$(OBJDIR)/startrom.o:	../../listopts.s
$(OBJDIR)/startrom.o:	$(OBJDIR)/lolevel.s
$(OBJDIR)/startrom.o:	../../macros.s
$(OBJDIR)/startrom.o:	../../pie/target.s
	$(AS) ../../startrom.s $(AFLAGS) -o $@

$(OBJDIR)/startlib.o:	../../startlib.s
$(OBJDIR)/startlib.o:	../../listopts.s
$(OBJDIR)/startlib.o:	$(OBJDIR)/lolevel.s
$(OBJDIR)/startlib.o:	../../macros.s
$(OBJDIR)/startlib.o:	../../pie/target.s
	$(AS) ../../startlib.s $(AFLAGS) -o $@

$(OBJDIR)/except.o:	../../except.s
$(OBJDIR)/except.o:	../../listopts.s
$(OBJDIR)/except.o:	$(OBJDIR)/lolevel.s
$(OBJDIR)/except.o:	../../macros.s
$(OBJDIR)/except.o:	../../pie/target.s
	$(AS) ../../except.s $(AFLAGS) -o $@

$(OBJDIR)/supprom.o:	../../suppasm.s
$(OBJDIR)/supprom.o:	../../listopts.s
$(OBJDIR)/supprom.o:	$(OBJDIR)/lolevel.s
$(OBJDIR)/supprom.o:	../../macros.s
$(OBJDIR)/supprom.o:	../../pie/target.s
	$(AS) ../../suppasm.s $(AFLAGS) -o $(OBJDIR)/supprom.o

$(OBJDIR)/supplib.o:	../../suppasm.s
$(OBJDIR)/supplib.o:	../../listopts.s
$(OBJDIR)/supplib.o:	$(OBJDIR)/lolevel.s
$(OBJDIR)/supplib.o:	../../macros.s
$(OBJDIR)/supplib.o:	../../pie/target.s
	$(AS) ../../suppasm.s $(AFLAGS) -PD 'LINKING_WITH_CLIB SETA 1' -o $(OBJDIR)/supplib.o

$(OBJDIR)/serlasm.o:	../../serlasm.s
$(OBJDIR)/serlasm.o:	../../listopts.s
$(OBJDIR)/serlasm.o:	$(OBJDIR)/lolevel.s
$(OBJDIR)/serlasm.o:	../../macros.s
	$(AS)	../../serlasm.s $(AFLAGS) -o $@

$(OBJDIR)/ladpasm.o:		../../logging/ladpasm.s
	$(AS)	../../logging/ladpasm.s $(AFLAGS) -o $@

$(OBJDIR)/lolevel.s:	../../pie/makelo.c ../../angel.h ../../arm.h \
			../../sys.h ../../pie/devconf.h
	gcc -o $(OBJDIR)/makelo -I../.. -I../../pie ../../pie/makelo.c \
			$(CDEFS) $(ENDIANDEFN)
	$(OBJDIR)/makelo > $(OBJDIR)/lolevel.s

# And now the mechanically built dependencies

$(OBJDIR)/devices.o:	../../adp.h
$(OBJDIR)/devices.o:	../../angel.h
$(OBJDIR)/devices.o:	../../buffers.h
$(OBJDIR)/devices.o:	../../chandefs.h
$(OBJDIR)/devices.o:	../../devclnt.h
$(OBJDIR)/devices.o:	../../devdriv.h
$(OBJDIR)/devices.o:	../../devices.h
$(OBJDIR)/devices.o:	../../params.h
$(OBJDIR)/devices.o:	../../pie/devconf.h
$(OBJDIR)/devices.o:	../../pie/devices.c
$(OBJDIR)/devices.o:	../../prof.h
$(OBJDIR)/devices.o:	../../serial.h
$(OBJDIR)/devices.o:	../../serlock.h
$(OBJDIR)/devices.o:	../../stacks.h
$(OBJDIR)/devices.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLdevices) -o $(OBJDIR)/devices.o ../../pie/devices.c

$(OBJDIR)/devmisc.o:	../../adp.h
$(OBJDIR)/devmisc.o:	../../angel.h
$(OBJDIR)/devmisc.o:	../../buffers.h
$(OBJDIR)/devmisc.o:	../../chandefs.h
$(OBJDIR)/devmisc.o:	../../devclnt.h
$(OBJDIR)/devmisc.o:	../../devdriv.h
$(OBJDIR)/devmisc.o:	../../devices.h
$(OBJDIR)/devmisc.o:	../../devmisc.c
$(OBJDIR)/devmisc.o:	../../logging.h
$(OBJDIR)/devmisc.o:	../../logtarget.h
$(OBJDIR)/devmisc.o:	../../params.h
$(OBJDIR)/devmisc.o:	../../pie/devconf.h
$(OBJDIR)/devmisc.o:	../../serlock.h
$(OBJDIR)/devmisc.o:	../../stacks.h
$(OBJDIR)/devmisc.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLdevmisc) -o $(OBJDIR)/devmisc.o ../../devmisc.c

$(OBJDIR)/devraw.o:	../../adp.h
$(OBJDIR)/devraw.o:	../../angel.h
$(OBJDIR)/devraw.o:	../../buffers.h
$(OBJDIR)/devraw.o:	../../chandefs.h
$(OBJDIR)/devraw.o:	../../devclnt.h
$(OBJDIR)/devraw.o:	../../devdriv.h
$(OBJDIR)/devraw.o:	../../devices.h
$(OBJDIR)/devraw.o:	../../devraw.c
$(OBJDIR)/devraw.o:	../../devraw.h
$(OBJDIR)/devraw.o:	../../logging.h
$(OBJDIR)/devraw.o:	../../logtarget.h
$(OBJDIR)/devraw.o:	../../params.h
$(OBJDIR)/devraw.o:	../../pie/devconf.h
$(OBJDIR)/devraw.o:	../../serlock.h
$(OBJDIR)/devraw.o:	../../stacks.h
$(OBJDIR)/devraw.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLdevraw) -o $(OBJDIR)/devraw.o ../../devraw.c

$(OBJDIR)/buffers.o:	../../adp.h
$(OBJDIR)/buffers.o:	../../angel.h
$(OBJDIR)/buffers.o:	../../buffers.c
$(OBJDIR)/buffers.o:	../../buffers.h
$(OBJDIR)/buffers.o:	../../chandefs.h
$(OBJDIR)/buffers.o:	../../devices.h
$(OBJDIR)/buffers.o:	../../logging.h
$(OBJDIR)/buffers.o:	../../logtarget.h
$(OBJDIR)/buffers.o:	../../pie/devconf.h
$(OBJDIR)/buffers.o:	../../serlock.h
$(OBJDIR)/buffers.o:	../../stacks.h
$(OBJDIR)/buffers.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLbuffers) -o $(OBJDIR)/buffers.o ../../buffers.c

$(OBJDIR)/params.o:	../../adp.h
$(OBJDIR)/params.o:	../../angel.h
$(OBJDIR)/params.o:	../../chandefs.h
$(OBJDIR)/params.o:	../../endian.h
$(OBJDIR)/params.o:	../../logging.h
$(OBJDIR)/params.o:	../../logtarget.h
$(OBJDIR)/params.o:	../../params.c
$(OBJDIR)/params.o:	../../params.h
$(OBJDIR)/params.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLparams) -o $(OBJDIR)/params.o ../../params.c

$(OBJDIR)/logging.o:	../../adp.h
$(OBJDIR)/logging.o:	../../angel.h
$(OBJDIR)/logging.o:	../../chandefs.h
$(OBJDIR)/logging.o:	../../logging.c
$(OBJDIR)/logging.o:	../../logging.h
$(OBJDIR)/logging.o:	../../logpriv.h
$(OBJDIR)/logging.o:	../../logtarget.h
$(OBJDIR)/logging.o:	../../pie/devconf.h
$(OBJDIR)/logging.o:	../../stacks.h
$(OBJDIR)/logging.o:	../../support.h
$(OBJDIR)/logging.o:	../../sys.h
$(OBJDIR)/logging.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLlogging) -I../../logging -o $(OBJDIR)/logging.o ../../logging.c

$(OBJDIR)/suppmsg.o:	../../adperr.h
$(OBJDIR)/suppmsg.o:	../../angel.h
$(OBJDIR)/suppmsg.o:	../../buffers.h
$(OBJDIR)/suppmsg.o:	../../chandefs.h
$(OBJDIR)/suppmsg.o:	../../channels.h
$(OBJDIR)/suppmsg.o:	../../devices.h
$(OBJDIR)/suppmsg.o:	../../endian.h
$(OBJDIR)/suppmsg.o:	../../msgbuild.c
$(OBJDIR)/suppmsg.o:	../../msgbuild.h
$(OBJDIR)/suppmsg.o:	../../pie/devconf.h
$(OBJDIR)/suppmsg.o:	../../stacks.h
$(OBJDIR)/suppmsg.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLsuppmsg) -o $(OBJDIR)/suppmsg.o ../../msgbuild.c

$(OBJDIR)/serial.o:	../../adp.h
$(OBJDIR)/serial.o:	../../angel.h
$(OBJDIR)/serial.o:	../../buffers.h
$(OBJDIR)/serial.o:	../../chandefs.h
$(OBJDIR)/serial.o:	../../devclnt.h
$(OBJDIR)/serial.o:	../../devdriv.h
$(OBJDIR)/serial.o:	../../devices.h
$(OBJDIR)/serial.o:	../../logging.h
$(OBJDIR)/serial.o:	../../logtarget.h
$(OBJDIR)/serial.o:	../../params.h
$(OBJDIR)/serial.o:	../../pie/devconf.h
$(OBJDIR)/serial.o:	../../pie/hw.h
$(OBJDIR)/serial.o:	../../pie/serial.c
$(OBJDIR)/serial.o:	../../prof.h
$(OBJDIR)/serial.o:	../../ringbuff.h
$(OBJDIR)/serial.o:	../../rxtx.h
$(OBJDIR)/serial.o:	../../serial.h
$(OBJDIR)/serial.o:	../../serlock.h
$(OBJDIR)/serial.o:	../../serring.h
$(OBJDIR)/serial.o:	../../stacks.h
$(OBJDIR)/serial.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLserial) -o $(OBJDIR)/serial.o ../../pie/serial.c

$(OBJDIR)/serraw.o:	../../adp.h
$(OBJDIR)/serraw.o:	../../angel.h
$(OBJDIR)/serraw.o:	../../buffers.h
$(OBJDIR)/serraw.o:	../../chandefs.h
$(OBJDIR)/serraw.o:	../../devappl.h
$(OBJDIR)/serraw.o:	../../devclnt.h
$(OBJDIR)/serraw.o:	../../devdriv.h
$(OBJDIR)/serraw.o:	../../devices.h
$(OBJDIR)/serraw.o:	../../devraw.h
$(OBJDIR)/serraw.o:	../../logging.h
$(OBJDIR)/serraw.o:	../../logtarget.h
$(OBJDIR)/serraw.o:	../../params.h
$(OBJDIR)/serraw.o:	../../pie/devconf.h
$(OBJDIR)/serraw.o:	../../ringbuff.h
$(OBJDIR)/serraw.o:	../../rxtx.h
$(OBJDIR)/serraw.o:	../../serlock.h
$(OBJDIR)/serraw.o:	../../serraw.c
$(OBJDIR)/serraw.o:	../../serring.h
$(OBJDIR)/serraw.o:	../../stacks.h
$(OBJDIR)/serraw.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLserraw) -o $(OBJDIR)/serraw.o ../../serraw.c

$(OBJDIR)/panicblk.o:	../../logging/panicblk.c
$(OBJDIR)/panicblk.o:	../../logging/panicblk.h
$(OBJDIR)/panicblk.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLpanicblk) -o $(OBJDIR)/panicblk.o ../../logging/panicblk.c

$(OBJDIR)/stacks.o:	../../pie/devconf.h
$(OBJDIR)/stacks.o:	../../stacks.c
$(OBJDIR)/stacks.o:	../../stacks.h
$(OBJDIR)/stacks.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLstacks) -o $(OBJDIR)/stacks.o ../../stacks.c

$(OBJDIR)/serlock.o:	../../adp.h
$(OBJDIR)/serlock.o:	../../angel.h
$(OBJDIR)/serlock.o:	../../arm.h
$(OBJDIR)/serlock.o:	../../buffers.h
$(OBJDIR)/serlock.o:	../../chandefs.h
$(OBJDIR)/serlock.o:	../../devclnt.h
$(OBJDIR)/serlock.o:	../../devdriv.h
$(OBJDIR)/serlock.o:	../../devices.h
$(OBJDIR)/serlock.o:	../../logging.h
$(OBJDIR)/serlock.o:	../../logtarget.h
$(OBJDIR)/serlock.o:	../../params.h
$(OBJDIR)/serlock.o:	../../pie/devconf.h
$(OBJDIR)/serlock.o:	../../serlock.c
$(OBJDIR)/serlock.o:	../../serlock.h
$(OBJDIR)/serlock.o:	../../stacks.h
$(OBJDIR)/serlock.o:	../../support.h
$(OBJDIR)/serlock.o:	../../pie/devconf.h
	$(CC) $(CFLAGS) $(CFLserlock) -o $(OBJDIR)/serlock.o ../../serlock.c

###
