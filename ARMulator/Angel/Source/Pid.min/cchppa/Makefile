#
# $Revision: 1.1.6.2 $
#   $Author: dbrooke $
#     $Date: 1998/03/10 16:03:49 $
#
# Copyright (c) 1996 Advanced RISC Machines Limited.
# All Rights Reserved
#

ANGSD=          ../../../angsd
ARMDBG=         ../../../armdbg

LATE_STARTUP=   0
THUMB_SUPPORT=  1
MINIMAL_ANGEL=  1

RWADDR=         0x02060000
ROADDR=         0x02040000
ANGELVSN=       0x0001

# Define this only if DEBUG_METHOD is set to logterm in devconf.h
LOGTERM_DEBUGGING=0

# Set this to 1 for debugging version of Angel, 0 for Release.
DEBUG=          0

# Extra debugging flags: choose from:
#   NO_RXE_DATA         -- don't print received data bytes.
#   CLEAR_BUFFERS       -- clear Angel buffers on allocation/deallocation
#   NO_LOG_INFO         -- Allow LogWarning/LogError, but kill LogInfo. Saves ~35K code
#   DEBUG_FILE_TOO      -- include file names in debug info available (define format too)
#   ASSERT_ENABLED      -- Allow Assertions with ASSERT(cond, info) macro
#

# ANGEL_DEBUG = -g+
# ANGEL_DEBUG = -g+ -DCLEAR_BUFFERS
# ANGEL_DEBUG = -g+ -DNO_LOG_INFO
ANGEL_DEBUG= -Ospace

ASSERT_ENABLED= 0

# Leave this one alone - comment, uncomment and edit the two macros above
# as necessary
#
ALL_DEBUG=      $(ANGEL_DEBUG) $(FUSION_DEBUG)
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


CDEFS=  -DMINIMAL_ANGEL=$(MINIMAL_ANGEL) -DTARGET -DRETRANS \
        -DTHUMB_SUPPORT=$(THUMB_SUPPORT) -DANGELVSN=$(ANGELVSN) \
        -DROADDR=$(ROADDR) -DLOGTERM_DEBUGGING=$(LOGTERM_DEBUGGING) -DRWADDR=$(RWADDR)  \
        -DETHERNET_SUPPORTED=0 \
        -DDEBUG=$(DEBUG) -DASSERT_ENABLED=$(ASSERT_ENABLED)


OBJS=   serlasm.o devices.o devraw.o devmisc.o params.o \
        buffers.o logging.o except.o serlock.o  startlib.o \
        serraw.o stacks.o

DRIVEROBJS=     st16c552.o dccdrv.o dccsup.o e5_sendb.o rombox.o \
                panicblk.o ladpasm.o

# Uncomment these for debugging version
#DEBUG=  -DSKEPTIC -DDEBUG -DKDBG -DM_HISTORY -DNO_INFO_MESSAGES

# Uncomment this for assertions in C code
ASSERT= -DASSERTIONS_ENABLED

CFLAGS=         -fc -fah -apcs 3/32bit -c \
        -I../../dcc/,../..,../../pid,../../pid/fusion/incl \
        -I$(ANGSD) -I$(ARMDBG) $(CCENDIAN) $(CFL) $(CDEFS) $(ALL_DEBUG) $(ASSERT)

AFLAGS=         -32 -apcs 3/swst/32bit $(ASENDIAN) \
        -I../.. -I../../pid -I.. -I../../../500/cl \
        -PD "LOGTERM_DEBUGGING SETA $(LOGTERM_DEBUGGING)" \
        -PD "ANGELVSN SETA $(ANGELVSN)" \
        -PD "DEBUG SETA $(DEBUG)" \
        -PD "LATE_STARTUP SETA $(LATE_STARTUP)" \
        -PD "ROADDR SETA $(ROADDR)" \
        -PD "THUMB_SUPPORT SETA $(THUMB_SUPPORT)" \
        -PD "ASSERT_ENABLED SETA $(ASSERT_ENABLED)" \
        -PD "MINIMAL_ANGEL SETA $(MINIMAL_ANGEL)" \
        -PD "ETHERNET_SUPPORTED SETA 0"

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
ALIB=           $(ARMTOOLS)armlib


all:    appldev

angmin.lib:     $(OBJS) supplib.o startrom.o drivers.a
                $(LD) -aof -o $@ $(OBJS) supplib.o drivers.a

drivers.a:      $(DRIVEROBJS)
                $(ALIB) -c -o drivers.a $(DRIVEROBJS)

clean:
        rm -f *.o makelo ../lolevel.s

#
# devconf.h has sanity checks upon definitions in this Makefile
#
../../pid/devconf.h:    Makefile

# Rules for creating dependencies, to standard output, which can then be
#  cut and pasted back into this makefile above.

depends: devices.dep devmisc.dep devraw.dep buffers.dep params.dep
depends: logging.dep msgbuild.dep st16c552.dep serraw.dep
depends: dccdrv.dep rombox.dep panicblk.dep stacks.dep serlock.dep

devices.dep:    ../../pid/devices.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevices) ../../pid/devices.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devmisc.dep:    ../../devmisc.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevmisc) ../../devmisc.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

devraw.dep:     ../../devraw.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevraw) ../../devraw.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

buffers.dep:    ../../buffers.c
        @$(CC) -Mx $(CFLAGS) $(CFLbuffers) ../../buffers.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

params.dep:     ../../params.c
        @$(CC) -Mx $(CFLAGS) $(CFLparams) ../../params.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

stacks.dep:     ../../stacks.c
        @$(CC) -Mx $(CFLAGS) $(CFLstacks) ../../stacks.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

logging.dep:    ../../logging.c
        @$(CC) -Mx $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") -I../../logging $?"

serlock.dep:    ../../serlock.c
        @$(CC) -Mx $(CFLAGS) $(CFLserlock) ../../serlock.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

msgbuild.dep:   ../../msgbuild.c
        @$(CC) -Mx $(CFLAGS) $(CFLmsgbuild) ../../msgbuild.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

st16c552.dep:     ../../pid/st16c552.c
        @$(CC) -Mx $(CFLAGS) $(CFLst16c552) ../../pid/st16c552.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

serraw.dep:       ../../serraw.c
        @$(CC) -Mx $(CFLAGS) $(CFLserraw) ../../serraw.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

dccdrv.dep:       ../../dcc/dccdrv.c
        @$(CC) -Mx $(CFLAGS) $(CFLdccdrv) ../../dcc/dccdrv.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

rombox.dep:       ../../logging/rombox.c
        @$(CC) -Mx $(CFLAGS) $(CFLrombox) ../../logging/rombox.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

panicblk.dep:     ../../logging/panicblk.c
        @$(CC) -Mx $(CFLAGS) $(CFLpanicblk) ../../logging/panicblk.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

# And here are the assmebler dependencies - probably not 100% correct

startrom.o:     ../../startrom.s
startrom.o:     ../../listopts.s
startrom.o:     ../lolevel.s
startrom.o:     ../../macros.s
startrom.o:     ../../pid/target.s
        $(AS) ../../startrom.s $(AFLAGS)

startlib.o:     ../../startlib.s
startlib.o:     ../../listopts.s
startlib.o:     ../lolevel.s
startlib.o:     ../../macros.s
startlib.o:     ../../pid/target.s
        $(AS) ../../startlib.s $(AFLAGS)

except.o:       ../../except.s
except.o:       ../../listopts.s
except.o:       ../lolevel.s
except.o:       ../../macros.s
except.o:       ../../pid/target.s
        $(AS) ../../except.s $(AFLAGS)

supprom.o:      ../../suppasm.s
supprom.o:      ../../listopts.s
supprom.o:      ../lolevel.s
supprom.o:      ../../macros.s
supprom.o:      ../../pid/target.s
        $(AS) ../../suppasm.s $(AFLAGS) -o supprom.o

supplib.o:      ../../suppasm.s
supplib.o:      ../../listopts.s
supplib.o:      ../lolevel.s
supplib.o:      ../../macros.s
supplib.o:      ../../pid/target.s
        $(AS) ../../suppasm.s $(AFLAGS) -PD 'LINKING_WITH_CLIB SETA 1' -o supplib.o

serlasm.o:      ../../serlasm.s
serlasm.o:      ../../listopts.s
serlasm.o:      ../lolevel.s
serlasm.o:      ../../macros.s
        $(AS)   ../../serlasm.s $(AFLAGS)

e5_sendb.o:     ../../logging/e5_sendb.s
e5_sendb.o:     ../../listopts.s
e5_sendb.o:     ../lolevel.s
        $(AS)   ../../logging/e5_sendb.s $(AFLAGS)
dccsup.o:       ../../dcc/dccsup.s
        $(AS)   ../../dcc/dccsup.s $(AFLAGS)
ladpasm.o:            ../../logging/ladpasm.s
        $(AS)   ../../logging/ladpasm.s $(AFLAGS)

../lolevel.s:   ../../pid/makelo.c ../../angel.h ../../arm.h
../lolevel.s:   ../../sys.h ../../pid/devconf.h
        gcc -o makelo -I../.. -I../../pid ../../pid/makelo.c \
        $(CDEFS) $(ENDIANDEFN)
        ./makelo > ../lolevel.s

# And now the mechanically built dependencies

# gnumake -k ARMTOOLS=/xdevt/trunk/gccsunos/ depends
devices.o:      ../../pid/devices.c
devices.o:      ../../devdriv.h
devices.o:      ../../pid/devconf.h
devices.o:      ../../pid/pid.h
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
devices.o:      ../../pid/devconf.h
devices.o:      ../../pid/st16c552.h
devices.o:      ../../devdriv.h
devices.o:      ../../serring.h
devices.o:      ../../ringbuff.h
devices.o:      ../../rxtx.h
devices.o:      ../../angel.h
devices.o:      ../../devclnt.h
devices.o:      ../../prof.h
        $(CC) $(CFLAGS) $(CFLdevices) ../../pid/devices.c
devmisc.o:      ../../devmisc.c
devmisc.o:      ../../devdriv.h
devmisc.o:      ../../pid/devconf.h
devmisc.o:      ../../pid/pid.h
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
devraw.o:       ../../devraw.c
devraw.o:       ../../devraw.h
devraw.o:       ../../devices.h
devraw.o:       ../../angel.h
devraw.o:       ../../buffers.h
devraw.o:       ../../chandefs.h
devraw.o:       ../../devdriv.h
devraw.o:       ../../pid/devconf.h
devraw.o:       ../../pid/pid.h
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
buffers.o:      ../../buffers.c
buffers.o:      ../../adp.h
buffers.o:      ../../chandefs.h
buffers.o:      ../../buffers.h
buffers.o:      ../../chandefs.h
buffers.o:      ../../chandefs.h
buffers.o:      ../../devices.h
buffers.o:      ../../angel.h
buffers.o:      ../../buffers.h
buffers.o:      ../../pid/devconf.h
buffers.o:      ../../pid/pid.h
buffers.o:      ../../logging.h
        $(CC) $(CFLAGS) $(CFLbuffers) ../../buffers.c
params.o:       ../../params.c
params.o:       ../../params.h
params.o:       ../../angel.h
params.o:       ../../adp.h
params.o:       ../../chandefs.h
params.o:       ../../endian.h
params.o:       ../../logging.h
        $(CC) $(CFLAGS) $(CFLparams) ../../params.c
logging.o:      ../../logging.c
logging.o:      ../../angel.h
logging.o:      ../../pid/devconf.h
logging.o:      ../../pid/pid.h
logging.o:      ../../logging.h
logging.o:      ../../logging/panicblk.h
        $(CC) $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c
msgbuild.o:     ../../msgbuild.c
msgbuild.o:     ../../angel.h
msgbuild.o:     ../../pid/devconf.h
msgbuild.o:     ../../pid/pid.h
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
st16c552.o:     ../../pid/st16c552.c
st16c552.o:     ../../devdriv.h
st16c552.o:     ../../pid/devconf.h
st16c552.o:     ../../pid/pid.h
st16c552.o:     ../../devclnt.h
st16c552.o:     ../../devices.h
st16c552.o:     ../../angel.h
st16c552.o:     ../../buffers.h
st16c552.o:     ../../chandefs.h
st16c552.o:     ../../serlock.h
st16c552.o:     ../../angel.h
st16c552.o:     ../../params.h
st16c552.o:     ../../angel.h
st16c552.o:     ../../adp.h
st16c552.o:     ../../chandefs.h
st16c552.o:     ../../logging.h
st16c552.o:     ../../serlock.h
st16c552.o:     ../../params.h
st16c552.o:     ../../pid/st16c552.h
st16c552.o:     ../../devdriv.h
st16c552.o:     ../../serring.h
st16c552.o:     ../../ringbuff.h
st16c552.o:     ../../rxtx.h
st16c552.o:     ../../angel.h
st16c552.o:     ../../devclnt.h
st16c552.o:     ../../pid/pid.h
st16c552.o:     ../../serring.h
st16c552.o:     ../../serial.h
st16c552.o:     ../../devdriv.h
        $(CC) $(CFLAGS) $(CFLst16c552) ../../pid/st16c552.c
serraw.o:       ../../serraw.c
serraw.o:       ../../devappl.h
serraw.o:       ../../devraw.h
serraw.o:       ../../devices.h
serraw.o:       ../../angel.h
serraw.o:       ../../buffers.h
serraw.o:       ../../chandefs.h
serraw.o:       ../../devdriv.h
serraw.o:       ../../pid/devconf.h
serraw.o:       ../../pid/pid.h
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
serraw.o:       ../../rxtx.h
serraw.o:       ../../angel.h
serraw.o:       ../../devclnt.h
        $(CC) $(CFLAGS) $(CFLserraw) ../../serraw.c
dccdrv.o:       ../../dcc/dccdrv.c
dccdrv.o:       ../../devdriv.h
dccdrv.o:       ../../pid/devconf.h
dccdrv.o:       ../../pid/pid.h
dccdrv.o:       ../../devclnt.h
dccdrv.o:       ../../devices.h
dccdrv.o:       ../../angel.h
dccdrv.o:       ../../buffers.h
dccdrv.o:       ../../chandefs.h
dccdrv.o:       ../../serlock.h
dccdrv.o:       ../../angel.h
dccdrv.o:       ../../params.h
dccdrv.o:       ../../angel.h
dccdrv.o:       ../../adp.h
dccdrv.o:       ../../chandefs.h
dccdrv.o:       ../../logging.h
dccdrv.o:       ../../support.h
dccdrv.o:       ../../serlock.h
dccdrv.o:       ../../params.h
dccdrv.o:       ../../serring.h
dccdrv.o:       ../../ringbuff.h
dccdrv.o:       ../../rxtx.h
dccdrv.o:       ../../angel.h
dccdrv.o:       ../../devclnt.h
dccdrv.o:       ../../dcc/dccdrv.h
dccdrv.o:       ../../devdriv.h
        $(CC) $(CFLAGS) $(CFLdccdrv) ../../dcc/dccdrv.c
rombox.o:       ../../logging/rombox.c
rombox.o:       ../../pid/devconf.h
rombox.o:       ../../pid/pid.h
rombox.o:       ../../logging/rombox.h
        $(CC) $(CFLAGS) $(CFLrombox) ../../logging/rombox.c
panicblk.o:     ../../logging/panicblk.c
panicblk.o:     ../../logging/panicblk.h
        $(CC) $(CFLAGS) $(CFLpanicblk) ../../logging/panicblk.c
stacks.o:       ../../stacks.c
stacks.o:       ../../stacks.h
stacks.o:       ../../pid/devconf.h
stacks.o:       ../../pid/pid.h
stacks.o:       ../../stacks.h
        $(CC) $(CFLAGS) $(CFLstacks) ../../stacks.c
serlock.o:      ../../serlock.c
serlock.o:      ../../serlock.h
serlock.o:      ../../angel.h
serlock.o:      ../../logging.h
serlock.o:      ../../support.h
serlock.o:      ../../arm.h
serlock.o:      ../../angel.h
serlock.o:      ../../devdriv.h
serlock.o:      ../../pid/devconf.h
serlock.o:      ../../pid/pid.h
serlock.o:      ../../stacks.h
serlock.o:      ../../pid/devconf.h
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
serlock.o:      ../../pid/devconf.h
serlock.o:      ../../stacks.h
        $(CC) $(CFLAGS) $(CFLserlock) ../../serlock.c

###

CLIB = ../../../500/cl/angelarm/armlib.o
CLSTAND = ../../../clstand
RTSTAND_O =

appldev:  appldev.o angmin.lib rtstand.o suppmsg.o
        $(LD) -o $@ appldev.o angmin.lib startrom.o rtstand.o suppmsg.o \
            -bin -nozeropad -base $(ROADDR) -data $(RWADDR)          \
            -first 'startrom.o(ROMStartup)' -symbols $@.sym.lst
        $(LD) -o $@.sym appldev.o angmin.lib startrom.o rtstand.o suppmsg.o \
                -aif -bin -nozeropad -base $(ROADDR) -data $(RWADDR)    \
                -first 'startrom.o(ROMStartup)' -entry $(ROADDR)

appldev.o:      ../../examples/appldev.c
        $(CC) $(CFLAGS) $(CFLappldev) ../../examples/appldev.c

suppmsg.o:      msgbuild.o      # save repeating all the depends
        $(CC) $(CFLAGS) $(CFLsuppmsg) -DMSG_UTILS_ONLY -o suppmsg.o     \
                 ../../msgbuild.c

rtstand.o:      ../../examples/rtstand.s
        $(AS) ../../examples/rtstand.s $(AFLAGS)

### EOF Makefile ###
