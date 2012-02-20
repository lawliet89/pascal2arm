#
# $Revision: 1.1.6.1 $
#   $Author: rivimey $
#     $Date: 1997/12/11 16:10:34 $
#
# Copyright (c) 1996 Advanced RISC Machines Limited.
# All Rights Reserved
#

ARMDBG=         ../../../armdbg
CLX=            ../../../clx

LATE_STARTUP=   0
THUMB_SUPPORT=  0
ASSERT_ENABLED= 1    # See also ASSERT macro, below.
MINIMAL_ANGEL=  0
DEBUG_BUILD=    0

RWADDR=         0x00000000
# 0x70000 is a typical place to run from RAM - uncomment if you want this
ROADDR= 0x00070000
# 0xc0000000 is where the ROM resides - uncomment this to run from ROM
# ROADDR=               0xC0000000
ANGELVSN=       0x0001

#
# Pick one of the following modules to link first:
#
# startrom.o - Used when ROM can be unmapped from 0
#   except.o - Used when ROM is permanently mapped at 0
#
FIRST=          'startrom.o(ROMStartup)'
#FIRST=         'except.o(__Vectors)'

# The appropriate one of these must be left uncommented.
# in order to build for the required endianess.
#
ENDIANFLAG= -li
ENDIANDEFN= -DLITTLE_ENDIAN

# ENDIANFLAG=   -bi
# ENDIANDEFN=   -DBIG_ENDIAN

#
# now make compiler and assembler arguments
#
CCENDIAN=       $(ENDIANFLAG) $(ENDIANDEFN)
ASENDIAN=       $(ENDIANFLAG)

CDEFS=          -DTARGET -DRETRANS \
        -DTHUMB_SUPPORT=$(THUMB_SUPPORT) -DANGELVSN=$(ANGELVSN) \
        -DROADDR=$(ROADDR) -DLOGTERM_DEBUGGING=$(LOGTERM_DEBUGGING) -DRWADDR=$(RWADDR) \
        -DMINIMAL_ANGEL=$(MINIMAL_ANGEL) -DLATE_STARTUP=$(LATE_STARTUP)

# Uncomment this for debugging version
# DEBUG=        -DDEBUG -DNO_INFO_MESSAGES

# Uncomment this for assertions in C code
ASSERT=         -DASSERTIONS_ENABLED

CFLAGS=         -fc -fah -apcs 3/32bit -c \
         -I../..,../../pie -I$(CLX) -I$(ARMDBG) \
         $(CCENDIAN) $(CFL) $(CDEFS) $(DEBUG) $(ASSERT)

AFLAGS=         -32 -apcs 3/swst/32bit $(ASENDIAN) \
        -I../.. -I../../pie -I.. -I../../../500/cl \
        -PD "LOGTERM_DEBUGGING SETA $(LOGTERM_DEBUGGING)" \
        -PD "ANGELVSN SETA $(ANGELVSN)" \
        -PD "DEBUG_BUILD SETA $(DEBUG_BUILD)" \
        -PD "LATE_STARTUP SETA $(LATE_STARTUP)" \
        -PD "ROADDR SETA $(ROADDR)" \
        -PD "THUMB_SUPPORT SETA $(THUMB_SUPPORT)" \
        -PD "ASSERT_ENABLED SETA $(ASSERT_ENABLED)" \
        -PD "MINIMAL_ANGEL SETA $(MINIMAL_ANGEL)"

LFLAGS=         -symbols $@.sym.lst

#
# set ARMTOOLS to point to the tools you wish to
# use (otherwise the default ones will be used)
#
# Note that tasm is used since this means the makefile does not have to
# be modified between enabling and disabling Thumb support
#
ARMTOOLS=
CC=             $(ARMTOOLS)armcc
AS=             $(ARMTOOLS)tasm
LD=             $(ARMTOOLS)armlink
AR=             $(ARMTOOLS)armlib

OBJS=           debug.o debughwi.o info.o ctrl.o prof.o debugos.o boot.o \
        serlock.o serlasm.o devices.o devmisc.o devappl.o devshare.o devraw.o \
        devclnt.o channels.o buffers.o rx.o tx.o crc.o msgbuild.o logging.o \
        params.o except.o startlib.o sys.o serpkt.o serraw.o stacks.o

DRIVEROBJS=     serial.o panicblk.o logadp.o ladpasm.o

# The following are useful during testing ...

all: angel.rom angel.lib

angel.rom:      $(OBJS) supprom.o startrom.o drivers.a
        $(LD) $(OBJS) supprom.o startrom.o drivers.a -bin -nozeropad \
        -base $(ROADDR) -data $(RWADDR) -symbols $@.sym.lst \
        -first $(FIRST) -o $@
        $(LD) $(OBJS) supprom.o startrom.o drivers.a -aif -bin -nozeropad \
        -base $(ROADDR) -entry $(ROADDR) -data $(RWADDR) \
        -first $(FIRST) -o angel.sym
        sort -f $@.sym.lst > $@.alpha.lst
        sort -b +1 $@.sym.lst > $@.num.lst

angel.lib:      $(OBJS) supplib.o startrom.o drivers.a
                $(LD) -aof -o $@ $(OBJS) supplib.o drivers.a

drivers.a:      $(DRIVEROBJS)
        $(AR) -c -o drivers.a $(DRIVEROBJS)

#
# devconf.h has sanity checks upon definitions in this Makefile
#
../../pie/devconf.h:    Makefile
        chmod +w $@
        touch $@

###

#
# Rules for linking example applications
#

appldev:        appldev.o suppmsg.o devappl.o
        $(LD) -o $@ appldev.o devappl.o suppmsg.o

appldev.rom:    appldev.o suppmsg.o angel.lib
        $(LD) -o $@ appldev.o angel.lib startrom.o \
                -bin -nozeropad -base $(ROADDR) -data $(RWADDR)         \
                -first 'startrom.o(ROMStartup)'
        $(LD) -o appldev.sym appldev.o angel.lib startrom.o \
                -aif -bin -nozeropad -base $(ROADDR) -data $(RWADDR)    \
                -first 'startrom.o(ROMStartup)' -entry $(ROADDR)

appldev.o:      ../../examples/appldev.c
        $(CC) $(CFLAGS) $(CFLappldev) ../../examples/appldev.c

suppmsg.o:      msgbuild.o      # save repeating all the depends
        $(CC) $(CFLAGS) $(CFLsuppmsg) -DMSG_UTILS_ONLY -o suppmsg.o     \
                 ../../msgbuild.c

latestrt.rom:   latestrt.o angel.lib startrom.o
        $(LD) -o $@ latestrt.o angel.lib startrom.o     \
                -bin -nozeropad -base $(ROADDR) -data $(RWADDR)         \
                -first 'startrom.o(ROMStartup)' -symbols $@.sym.lst
        sort -f $@.sym.lst > $@.alpha.lst
        sort -b +1 $@.sym.lst > $@.num.lst
        $(LD) -o latestrt.sym latestrt.o angel.lib startrom.o   \
                -aif -bin -nozeropad -base $(ROADDR) -data $(RWADDR)    \
                -first 'startrom.o(ROMStartup)'

latestrt.o:     ../../examples/latestrt.s
        $(AS) $(AFLAGS) ../../examples/latestrt.s

clean:
        rm -f *.o ../lolevel.s makelo

###

# Rules for creating dependencies, to standard output, which can then be
#  cut and pasted back into this makefile above.

depends: debug.dep debughwi.dep info.dep ctrl.dep debugos.dep boot.dep
depends: serlock.dep devices.dep devmisc.dep devappl.dep devshare.dep
depends: devraw.dep devclnt.dep channels.dep buffers.dep rx.dep tx.dep
depends: crc.dep logging.dep msgbuild.dep sys.dep params.dep serial.dep
depends: serpkt.dep serraw.dep prof.dep panicblk.dep logadp.dep stacks.dep

debug.dep:      ../../debug.c
        @$(CC) -Mx $(CFLAGS) $(CFLdebug) ../../debug.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

params.dep:     ../../params.c
        @$(CC) -Mx $(CFLAGS) $(CFLparams) ../../params.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

debughwi.dep:   ../../debughwi.c
        @$(CC) -Mx $(CFLAGS) $(CFLdebughwi) ../../debughwi.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

info.dep:   ../../info.c
        @$(CC) -Mx $(CFLAGS) $(CFLinfo) ../../info.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

ctrl.dep:   ../../ctrl.c
        @$(CC) -Mx $(CFLAGS) $(CFLctrl) ../../ctrl.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

debugos.dep:    ../../debugos.c
        @$(CC) -Mx $(CFLAGS) $(CFLdebugos) ../../debugos.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

boot.dep:       ../../boot.c
        @$(CC) -Mx $(CFLAGS) $(CFLboot) ../../boot.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

serlock.dep:    ../../serlock.c
        @$(CC) -Mx $(CFLAGS) $(CFLserlock) ../../serlock.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devices.dep:    ../../pie/devices.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevices) ../../pie/devices.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devmisc.dep:    ../../devmisc.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevmisc) ../../devmisc.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devappl.dep:    ../../devappl.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevappl) ../../devappl.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devshare.dep:   ../../devshare.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevshare) ../../devshare.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devraw.dep:     ../../devraw.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevraw) ../../devraw.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devclnt.dep:    ../../devclnt.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevclnt) ../../devclnt.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

channels.dep:   ../../channels.c
        @$(CC) -Mx $(CFLAGS) $(CFLchannels) ../../channels.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

buffers.dep:    ../../buffers.c
        @$(CC) -Mx $(CFLAGS) $(CFLbuffers) ../../buffers.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

rx.dep: ../../rx.c
        @$(CC) -Mx $(CFLAGS) $(CFLrx) ../../rx.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

tx.dep: ../../tx.c
        @$(CC) -Mx $(CFLAGS) $(CFLtx) ../../tx.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

crc.dep:        ../../crc.c
        @$(CC) -Mx $(CFLAGS) $(CFLcrc) ../../crc.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

msgbuild.dep:   ../../msgbuild.c
        @$(CC) -Mx $(CFLAGS) $(CFLmsgbuild) ../../msgbuild.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

logging.dep:    ../../logging.c
        @$(CC) -Mx $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") -I../../logging $?"

sys.dep:        ../../sys.c
        @$(CC) -Mx $(CFLAGS) $(CFLsys) ../../sys.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

serial.dep:     ../../pie/serial.c
        @$(CC) -Mx $(CFLAGS) $(CFLserial) ../../pie/serial.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

serpkt.dep:       ../../serpkt.c
        @$(CC) -Mx $(CFLAGS) $(CFLserpkt) ../../serpkt.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

serraw.dep:       ../../serraw.c
        @$(CC) -Mx $(CFLAGS) $(CFLserraw) ../../serraw.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

panicblk.dep:     ../../logging/panicblk.c
        @$(CC) -Mx $(CFLAGS) $(CFLpanicblk) ../../logging/panicblk.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

logadp.dep:       ../../logging/logadp.c
        @$(CC) -Mx $(CFLAGS) $(CFLlogadp) ../../logging/logadp.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

prof.dep:       ../../prof.c
        @$(CC) -Mx $(CFLAGS) $(CFLprof) ../../prof.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

stacks.dep:     ../../stacks.c
        @$(CC) -Mx $(CFLAGS) $(CFLstacks) ../../stacks.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

# And here are the assmebler dependencies - probably not 100% correct

startrom.o:     ../../startrom.s
startrom.o:     ../../listopts.s
startrom.o:     ../lolevel.s
startrom.o:     ../../macros.s
startrom.o:     ../../pie/target.s
        $(AS) ../../startrom.s $(AFLAGS)

startlib.o:     ../../startlib.s
startlib.o:     ../../listopts.s
startlib.o:     ../lolevel.s
startlib.o:     ../../macros.s
startlib.o:     ../../pie/target.s
        $(AS) ../../startlib.s $(AFLAGS)

except.o:       ../../except.s
except.o:       ../../listopts.s
except.o:       ../lolevel.s
except.o:       ../../macros.s
except.o:       ../../pie/target.s
        $(AS) ../../except.s $(AFLAGS)

supprom.o:      ../../suppasm.s
supprom.o:      ../../listopts.s
supprom.o:      ../lolevel.s
supprom.o:      ../../macros.s
supprom.o:      ../../pie/target.s
        $(AS) ../../suppasm.s $(AFLAGS) -o supprom.o

supplib.o:      ../../suppasm.s
supplib.o:      ../../listopts.s
supplib.o:      ../lolevel.s
supplib.o:      ../../macros.s
supplib.o:      ../../pie/target.s
        $(AS) ../../suppasm.s $(AFLAGS) -PD 'LINKING_WITH_CLIB SETA 1' -o supplib.o

serlasm.o:      ../../serlasm.s
serlasm.o:      ../../listopts.s
serlasm.o:      ../lolevel.s
serlasm.o:      ../../macros.s
        $(AS)   ../../serlasm.s $(AFLAGS)

ladpasm.o:              ../../logging/ladpasm.s
        $(AS)   ../../logging/ladpasm.s $(AFLAGS)

../lolevel.s:   ../../pie/makelo.c ../../angel.h ../../arm.h
../lolevel.s:   ../../sys.h ../../pie/devconf.h ../../pie/hw.h
        cc -Ae +O2 -o makelo -I../.. -I../../pie ../../pie/makelo.c \
        $(CDEFS) $(ENDIANDEFN)
        ./makelo > ../lolevel.s

# Extra dependencies for logging.o, for all the optional headers

logging.o:      ../../logging/logadp.h
logging.o:      ../../logging/panicblk.h
logging.o:      ../../logging/pidulate.h

# And now the mechanically built dependencies

debug.o:        ../../debug.c
debug.o:        ../../angel.h
debug.o:        ../../adp.h
debug.o:        ../../chandefs.h
debug.o:        ../../arm.h
debug.o:        ../../angel.h
debug.o:        ../../debug.h
debug.o:        ../../endian.h
debug.o:        ../../debughwi.h
debug.o:        ../../buffers.h
debug.o:        ../../chandefs.h
debug.o:        ../../channels.h
debug.o:        ../../devices.h
debug.o:        ../../angel.h
debug.o:        ../../buffers.h
debug.o:        ../../chandefs.h
debug.o:        ../../adperr.h
debug.o:        ../../msgbuild.h
debug.o:        ../../channels.h
debug.o:        ../../logging.h
debug.o:        ../../serlock.h
debug.o:        ../../angel.h
debug.o:        ../../pie/devconf.h
        $(CC) $(CFLAGS) $(CFLdebug) ../../debug.c
debughwi.o:     ../../debughwi.c
debughwi.o:     ../../angel.h
debughwi.o:     ../../adp.h
debughwi.o:     ../../chandefs.h
debughwi.o:     ../../channels.h
debughwi.o:     ../../devices.h
debughwi.o:     ../../angel.h
debughwi.o:     ../../buffers.h
debughwi.o:     ../../chandefs.h
debughwi.o:     ../../chandefs.h
debughwi.o:     ../../adperr.h
debughwi.o:     ../../debug.h
debughwi.o:     ../../debugos.h
debughwi.o:     ../../arm.h
debughwi.o:     ../../angel.h
debughwi.o:     ../../../armdbg/dbg_cp.h
debughwi.o:     ../../endian.h
debughwi.o:     ../../debughwi.h
debughwi.o:     ../../buffers.h
debughwi.o:     ../../logging.h
debughwi.o:     ../../msgbuild.h
debughwi.o:     ../../channels.h
        $(CC) $(CFLAGS) $(CFLdebughwi) ../../debughwi.c
info.o: ../../info.c
info.o: ../../angel.h
info.o: ../../adp.h
info.o: ../../chandefs.h
info.o: ../../channels.h
info.o: ../../devices.h
info.o: ../../angel.h
info.o: ../../buffers.h
info.o: ../../chandefs.h
info.o: ../../chandefs.h
info.o: ../../adperr.h
info.o: ../../debughwi.h
info.o: ../../buffers.h
info.o: ../../debugos.h
info.o: ../../arm.h
info.o: ../../angel.h
info.o: ../../../armdbg/dbg_cp.h
info.o: ../../endian.h
info.o: ../../logging.h
info.o: ../../msgbuild.h
info.o: ../../channels.h
info.o: ../../../armdbg/dbg_cp.h
        $(CC) $(CFLAGS) $(CFLinfo) ../../info.c
ctrl.o: ../../ctrl.c
ctrl.o: ../../angel.h
ctrl.o: ../../adp.h
ctrl.o: ../../chandefs.h
ctrl.o: ../../debug.h
ctrl.o: ../../debughwi.h
ctrl.o: ../../buffers.h
ctrl.o: ../../chandefs.h
ctrl.o: ../../debugos.h
ctrl.o: ../../arm.h
ctrl.o: ../../angel.h
ctrl.o: ../../../armdbg/dbg_cp.h
ctrl.o: ../../logging.h
ctrl.o: ../../msgbuild.h
ctrl.o: ../../channels.h
ctrl.o: ../../devices.h
ctrl.o: ../../angel.h
ctrl.o: ../../buffers.h
ctrl.o: ../../chandefs.h
ctrl.o: ../../adperr.h
        $(CC) $(CFLAGS) $(CFLctrl) ../../ctrl.c
debugos.o:      ../../debugos.c
debugos.o:      ../../angel.h
debugos.o:      ../../adp.h
debugos.o:      ../../chandefs.h
debugos.o:      ../../debug.h
debugos.o:      ../../debugos.h
debugos.o:      ../../arm.h
debugos.o:      ../../angel.h
debugos.o:      ../../../armdbg/dbg_cp.h
debugos.o:      ../../endian.h
debugos.o:      ../../debughwi.h
debugos.o:      ../../buffers.h
debugos.o:      ../../chandefs.h
debugos.o:      ../../logging.h
debugos.o:      ../../serlock.h
debugos.o:      ../../angel.h
debugos.o:      ../../devclnt.h
debugos.o:      ../../devices.h
debugos.o:      ../../angel.h
debugos.o:      ../../buffers.h
debugos.o:      ../../pie/devconf.h
debugos.o:      ../../support.h
        $(CC) $(CFLAGS) $(CFLdebugos) ../../debugos.c
boot.o: ../../boot.c
boot.o: ../../angel.h
boot.o: ../../adp.h
boot.o: ../../chandefs.h
boot.o: ../../channels.h
boot.o: ../../devices.h
boot.o: ../../angel.h
boot.o: ../../buffers.h
boot.o: ../../chandefs.h
boot.o: ../../chandefs.h
boot.o: ../../adperr.h
boot.o: ../../debugos.h
boot.o: ../../arm.h
boot.o: ../../angel.h
boot.o: ../../../armdbg/dbg_cp.h
boot.o: ../../devclnt.h
boot.o: ../../devices.h
boot.o: ../../endian.h
boot.o: ../../params.h
boot.o: ../../angel.h
boot.o: ../../adp.h
boot.o: ../../devdriv.h
boot.o: ../../pie/devconf.h
boot.o: ../../devclnt.h
boot.o: ../../serlock.h
boot.o: ../../angel.h
boot.o: ../../params.h
boot.o: ../../logging.h
boot.o: ../../msgbuild.h
boot.o: ../../channels.h
boot.o: ../../support.h
boot.o: ../../serlock.h
boot.o: ../../pie/banner.h
boot.o: ../../../clx/toolver.h
        $(CC) $(CFLAGS) $(CFLboot) ../../boot.c
serlock.o:      ../../serlock.c
serlock.o:      ../../serlock.h
serlock.o:      ../../angel.h
serlock.o:      ../../logging.h
serlock.o:      ../../support.h
serlock.o:      ../../arm.h
serlock.o:      ../../angel.h
serlock.o:      ../../devdriv.h
serlock.o:      ../../pie/devconf.h
serlock.o:      ../../devclnt.h
serlock.o:      ../../devices.h
serlock.o:      ../../angel.h
serlock.o:      ../../buffers.h
serlock.o:      ../../chandefs.h
serlock.o:      ../../serlock.h
serlock.o:      ../../params.h
serlock.o:      ../../angel.h
serlock.o:      ../../adp.h
serlock.o:      ../../chandefs.h
serlock.o:      ../../pie/devconf.h
        $(CC) $(CFLAGS) $(CFLserlock) ../../serlock.c
devices.o:      ../../pie/devices.c
devices.o:      ../../devdriv.h
devices.o:      ../../pie/devconf.h
devices.o:      ../../devclnt.h
devices.o:      ../../devices.h
devices.o:      ../../angel.h
devices.o:      ../../buffers.h
devices.o:      ../../chandefs.h
devices.o:      ../../serlock.h
devices.o:      ../../angel.h
devices.o:      ../../params.h
devices.o:      ../../angel.h
devices.o:      ../../adp.h
devices.o:      ../../chandefs.h
devices.o:      ../../prof.h
devices.o:      ../../serial.h
devices.o:      ../../devdriv.h
        $(CC) $(CFLAGS) $(CFLdevices) ../../pie/devices.c
devmisc.o:      ../../devmisc.c
devmisc.o:      ../../devdriv.h
devmisc.o:      ../../pie/devconf.h
devmisc.o:      ../../devclnt.h
devmisc.o:      ../../devices.h
devmisc.o:      ../../angel.h
devmisc.o:      ../../buffers.h
devmisc.o:      ../../chandefs.h
devmisc.o:      ../../serlock.h
devmisc.o:      ../../angel.h
devmisc.o:      ../../params.h
devmisc.o:      ../../angel.h
devmisc.o:      ../../adp.h
devmisc.o:      ../../chandefs.h
devmisc.o:      ../../logging.h
        $(CC) $(CFLAGS) $(CFLdevmisc) ../../devmisc.c
devappl.o:      ../../devappl.c
devappl.o:      ../../devappl.h
devappl.o:      ../../devices.h
devappl.o:      ../../angel.h
devappl.o:      ../../buffers.h
devappl.o:      ../../chandefs.h
devappl.o:      ../../arm.h
devappl.o:      ../../angel.h
        $(CC) $(CFLAGS) $(CFLdevappl) ../../devappl.c
devshare.o:     ../../devshare.c
devshare.o:     ../../arm.h
devshare.o:     ../../angel.h
devshare.o:     ../../devclnt.h
devshare.o:     ../../devices.h
devshare.o:     ../../angel.h
devshare.o:     ../../buffers.h
devshare.o:     ../../chandefs.h
devshare.o:     ../../devdriv.h
devshare.o:     ../../pie/devconf.h
devshare.o:     ../../devclnt.h
devshare.o:     ../../serlock.h
devshare.o:     ../../angel.h
devshare.o:     ../../params.h
devshare.o:     ../../angel.h
devshare.o:     ../../adp.h
devshare.o:     ../../chandefs.h
devshare.o:     ../../devraw.h
devshare.o:     ../../devices.h
devshare.o:     ../../serlock.h
devshare.o:     ../../logging.h
devshare.o:     ../../support.h
        $(CC) $(CFLAGS) $(CFLdevshare) ../../devshare.c
devraw.o:       ../../devraw.c
devraw.o:       ../../devraw.h
devraw.o:       ../../devices.h
devraw.o:       ../../angel.h
devraw.o:       ../../buffers.h
devraw.o:       ../../chandefs.h
devraw.o:       ../../devdriv.h
devraw.o:       ../../pie/devconf.h
devraw.o:       ../../devclnt.h
devraw.o:       ../../devices.h
devraw.o:       ../../serlock.h
devraw.o:       ../../angel.h
devraw.o:       ../../params.h
devraw.o:       ../../angel.h
devraw.o:       ../../adp.h
devraw.o:       ../../chandefs.h
devraw.o:       ../../logging.h
        $(CC) $(CFLAGS) $(CFLdevraw) ../../devraw.c
devclnt.o:      ../../devclnt.c
devclnt.o:      ../../devclnt.h
devclnt.o:      ../../devices.h
devclnt.o:      ../../angel.h
devclnt.o:      ../../buffers.h
devclnt.o:      ../../chandefs.h
devclnt.o:      ../../devdriv.h
devclnt.o:      ../../pie/devconf.h
devclnt.o:      ../../devclnt.h
devclnt.o:      ../../serlock.h
devclnt.o:      ../../angel.h
devclnt.o:      ../../params.h
devclnt.o:      ../../angel.h
devclnt.o:      ../../adp.h
devclnt.o:      ../../chandefs.h
devclnt.o:      ../../logging.h
        $(CC) $(CFLAGS) $(CFLdevclnt) ../../devclnt.c
channels.o:     ../../channels.c
channels.o:     ../../channels.h
channels.o:     ../../devices.h
channels.o:     ../../angel.h
channels.o:     ../../buffers.h
channels.o:     ../../chandefs.h
channels.o:     ../../chandefs.h
channels.o:     ../../adperr.h
channels.o:     ../../chandefs.h
channels.o:     ../../chanpriv.h
channels.o:     ../../devclnt.h
channels.o:     ../../devices.h
channels.o:     ../../pie/devconf.h
channels.o:     ../../logging.h
channels.o:     ../../serlock.h
channels.o:     ../../angel.h
channels.o:     ../../adperr.h
        $(CC) $(CFLAGS) $(CFLchannels) ../../channels.c
buffers.o:      ../../buffers.c
buffers.o:      ../../adp.h
buffers.o:      ../../chandefs.h
buffers.o:      ../../buffers.h
buffers.o:      ../../chandefs.h
buffers.o:      ../../chandefs.h
buffers.o:      ../../devices.h
buffers.o:      ../../angel.h
buffers.o:      ../../buffers.h
buffers.o:      ../../pie/devconf.h
buffers.o:      ../../logging.h
        $(CC) $(CFLAGS) $(CFLbuffers) ../../buffers.c
rx.o:   ../../rx.c
rx.o:   ../../angel.h
rx.o:   ../../endian.h
rx.o:   ../../crc.h
rx.o:   ../../rxtx.h
rx.o:   ../../angel.h
rx.o:   ../../devclnt.h
rx.o:   ../../devices.h
rx.o:   ../../angel.h
rx.o:   ../../buffers.h
rx.o:   ../../chandefs.h
rx.o:   ../../channels.h
rx.o:   ../../devices.h
rx.o:   ../../chandefs.h
rx.o:   ../../adperr.h
rx.o:   ../../buffers.h
rx.o:   ../../devdriv.h
rx.o:   ../../pie/devconf.h
rx.o:   ../../devclnt.h
rx.o:   ../../serlock.h
rx.o:   ../../angel.h
rx.o:   ../../params.h
rx.o:   ../../angel.h
rx.o:   ../../adp.h
rx.o:   ../../chandefs.h
rx.o:   ../../logging.h
        $(CC) $(CFLAGS) $(CFLrx) ../../rx.c
tx.o:   ../../tx.c
tx.o:   ../../angel.h
tx.o:   ../../endian.h
tx.o:   ../../crc.h
tx.o:   ../../rxtx.h
tx.o:   ../../angel.h
tx.o:   ../../devclnt.h
tx.o:   ../../devices.h
tx.o:   ../../angel.h
tx.o:   ../../buffers.h
tx.o:   ../../chandefs.h
tx.o:   ../../channels.h
tx.o:   ../../devices.h
tx.o:   ../../chandefs.h
tx.o:   ../../adperr.h
tx.o:   ../../buffers.h
tx.o:   ../../logging.h
        $(CC) $(CFLAGS) $(CFLtx) ../../tx.c
crc.o:  ../../crc.c
crc.o:  ../../crc.h
        $(CC) $(CFLAGS) $(CFLcrc) ../../crc.c
logging.o:      ../../logging.c
logging.o:      ../../angel.h
logging.o:      ../../pie/devconf.h
logging.o:      ../../logging.h
        $(CC) $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c
msgbuild.o:     ../../msgbuild.c
msgbuild.o:     ../../angel.h
msgbuild.o:     ../../pie/devconf.h
msgbuild.o:     ../../channels.h
msgbuild.o:     ../../devices.h
msgbuild.o:     ../../angel.h
msgbuild.o:     ../../buffers.h
msgbuild.o:     ../../chandefs.h
msgbuild.o:     ../../chandefs.h
msgbuild.o:     ../../adperr.h
msgbuild.o:     ../../buffers.h
msgbuild.o:     ../../endian.h
msgbuild.o:     ../../msgbuild.h
msgbuild.o:     ../../channels.h
        $(CC) $(CFLAGS) $(CFLmsgbuild) ../../msgbuild.c
sys.o:  ../../sys.c
sys.o:  ../../angel.h
sys.o:  ../../adp.h
sys.o:  ../../chandefs.h
sys.o:  ../../arm.h
sys.o:  ../../angel.h
sys.o:  ../../logging.h
sys.o:  ../../msgbuild.h
sys.o:  ../../channels.h
sys.o:  ../../devices.h
sys.o:  ../../angel.h
sys.o:  ../../buffers.h
sys.o:  ../../chandefs.h
sys.o:  ../../chandefs.h
sys.o:  ../../adperr.h
sys.o:  ../../support.h
sys.o:  ../../sys.h
sys.o:  ../../endian.h
sys.o:  ../../debug.h
sys.o:  ../../pie/devconf.h
        $(CC) $(CFLAGS) $(CFLsys) ../../sys.c
params.o:       ../../params.c
params.o:       ../../params.h
params.o:       ../../angel.h
params.o:       ../../adp.h
params.o:       ../../chandefs.h
params.o:       ../../endian.h
params.o:       ../../logging.h
        $(CC) $(CFLAGS) $(CFLparams) ../../params.c
serial.o:       ../../pie/serial.c
serial.o:       ../../serial.h
serial.o:       ../../devdriv.h
serial.o:       ../../pie/devconf.h
serial.o:       ../../devclnt.h
serial.o:       ../../devices.h
serial.o:       ../../angel.h
serial.o:       ../../buffers.h
serial.o:       ../../chandefs.h
serial.o:       ../../serlock.h
serial.o:       ../../angel.h
serial.o:       ../../params.h
serial.o:       ../../angel.h
serial.o:       ../../adp.h
serial.o:       ../../chandefs.h
serial.o:       ../../devdriv.h
serial.o:       ../../pie/hw.h
serial.o:       ../../pie/devconf.h
serial.o:       ../../params.h
serial.o:       ../../rxtx.h
serial.o:       ../../angel.h
serial.o:       ../../devclnt.h
serial.o:       ../../logging.h
serial.o:       ../../serlock.h
serial.o:       ../../serring.h
serial.o:       ../../ringbuff.h
serial.o:       ../../pie/ringsize.h
serial.o:       ../../rxtx.h
serial.o:       ../../prof.h
        $(CC) $(CFLAGS) $(CFLserial) ../../pie/serial.c
serpkt.o:       ../../serpkt.c
serpkt.o:       ../../devdriv.h
serpkt.o:       ../../pie/devconf.h
serpkt.o:       ../../devclnt.h
serpkt.o:       ../../devices.h
serpkt.o:       ../../angel.h
serpkt.o:       ../../buffers.h
serpkt.o:       ../../chandefs.h
serpkt.o:       ../../serlock.h
serpkt.o:       ../../angel.h
serpkt.o:       ../../params.h
serpkt.o:       ../../angel.h
serpkt.o:       ../../adp.h
serpkt.o:       ../../chandefs.h
serpkt.o:       ../../rxtx.h
serpkt.o:       ../../angel.h
serpkt.o:       ../../devclnt.h
serpkt.o:       ../../logging.h
serpkt.o:       ../../serring.h
serpkt.o:       ../../ringbuff.h
serpkt.o:       ../../pie/ringsize.h
serpkt.o:       ../../rxtx.h
        $(CC) $(CFLAGS) $(CFLserpkt) ../../serpkt.c
serraw.o:       ../../serraw.c
serraw.o:       ../../devappl.h
serraw.o:       ../../devices.h
serraw.o:       ../../angel.h
serraw.o:       ../../buffers.h
serraw.o:       ../../chandefs.h
serraw.o:       ../../devdriv.h
serraw.o:       ../../pie/devconf.h
serraw.o:       ../../devclnt.h
serraw.o:       ../../devices.h
serraw.o:       ../../serlock.h
serraw.o:       ../../angel.h
serraw.o:       ../../params.h
serraw.o:       ../../angel.h
serraw.o:       ../../adp.h
serraw.o:       ../../chandefs.h
serraw.o:       ../../logging.h
serraw.o:       ../../serring.h
serraw.o:       ../../ringbuff.h
serraw.o:       ../../pie/ringsize.h
serraw.o:       ../../rxtx.h
serraw.o:       ../../angel.h
serraw.o:       ../../devclnt.h
        $(CC) $(CFLAGS) $(CFLserraw) ../../serraw.c
prof.o: ../../prof.c
prof.o: ../../angel.h
prof.o: ../../adp.h
prof.o: ../../chandefs.h
prof.o: ../../channels.h
prof.o: ../../devices.h
prof.o: ../../angel.h
prof.o: ../../buffers.h
prof.o: ../../chandefs.h
prof.o: ../../chandefs.h
prof.o: ../../adperr.h
prof.o: ../../debug.h
prof.o: ../../debugos.h
prof.o: ../../arm.h
prof.o: ../../angel.h
prof.o: ../../../armdbg/dbg_cp.h
prof.o: ../../debughwi.h
prof.o: ../../buffers.h
prof.o: ../../logging.h
prof.o: ../../pie/devconf.h
prof.o: ../../prof.h
        $(CC) $(CFLAGS) $(CFLdebugos) ../../prof.c
panicblk.o:     ../../logging/panicblk.c
panicblk.o:     ../../logging/panicblk.h
        $(CC) $(CFLAGS) $(CFLpanicblk) ../../logging/panicblk.c
logadp.o:       ../../logging/logadp.c
logadp.o:       ../../channels.h
logadp.o:       ../../devices.h
logadp.o:       ../../angel.h
logadp.o:       ../../buffers.h
logadp.o:       ../../chandefs.h
logadp.o:       ../../chandefs.h
logadp.o:       ../../adperr.h
logadp.o:       ../../logging/logadp.h
logadp.o:       ../../logging.h
logadp.o:       ../../debug.h
        $(CC) $(CFLAGS) $(CFLlogadp) ../../logging/logadp.c
stacks.o:       ../../stacks.c
stacks.o:       ../../stacks.h
stacks.o:       ../../pid/pid.h
        $(CC) $(CFLAGS) $(CFLstacks) ../../stacks.c

### EOF Makefile ###
