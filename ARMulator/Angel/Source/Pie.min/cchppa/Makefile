#
# $Revision: 1.1.6.1 $
#   $Author: dbrooke $
#     $Date: 1998/03/10 16:03:52 $
#
# Copyright (c) 1996 Advanced RISC Machines Limited.
# All Rights Reserved
#

ANGSD=    ../../../angsd
ARMDBG=  ../../../armdbg

LATE_STARTUP=   0
THUMB_SUPPORT=  0
# See also ASSERT macro, below.
MINIMAL_ANGEL=  1

# Define this only if DEBUG_METHOD is set to logterm in devconf.h
LOGTERM_DEBUGGING=0

RWADDR=  0x00000000
#ROADDR=        0xC0000000
ROADDR=  0x00060000
ANGELVSN=       0x0001

#
# Pick one of the following modules to link first:
#
# startrom.o - Used when ROM can be unmapped from 0
#   except.o - Used when ROM is permanently mapped at 0
#
FIRST=    'startrom.o(ROMStartup)'
#FIRST=  'except.o(__Vectors)'

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

# Uncomment these for debugging version
#DEBUG=  -DSKEPTIC -DDEBUG -DKDBG -DM_HISTORY \
        # -DNO_PIDULATOR -DNO_INFO_MESSAGES

# Set this to 1 for debugging version of Angel, 0 for Release.
DEBUG=    0

# Uncomment this for debugging version
# Extra debugging flags: choose from:
#   NO_RXE_DATA  -- don't print received data bytes.
#   CLEAR_BUFFERS       -- clear Angel buffers on allocation/deallocation
#   NO_LOG_INFO  -- Allow LogWarning/LogError, but kill LogInfo. Saves ~35K code
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
ALL_DEBUG=      $(ANGEL_DEBUG)

# Uncomment this for assertions in C code
ASSERT= -DASSERTIONS_ENABLED

CDEFS=  -DMINIMAL_ANGEL=$(MINIMAL_ANGEL) -DTARGET -DRETRANS \
        -DTHUMB_SUPPORT=$(THUMB_SUPPORT) -DANGELVSN=$(ANGELVSN) \
        -DROADDR=$(ROADDR) -DRWADDR=$(RWADDR) \
        -DDEBUG=$(DEBUG) -DASSERT_ENABLED=$(ASSERT_ENABLED)


CFLAGS=  -fc -fah -apcs 3/32bit -c \
         -I../..,../../pie \
        -I$(ANGSD) -I$(ARMDBG) $(CCENDIAN) $(CFL) $(CDEFS) $(ANGEL_DEBUG) 

AFLAGS=  -32 -apcs 3/swst/32bit $(ASENDIAN) \
        -I../.. -I../../pie -I.. -I../../../500/cl \
        -PD "ANGELVSN SETA $(ANGELVSN)" \
        -PD "ROADDR SETA $(ROADDR)" \
        -PD "DEBUG SETA $(DEBUG)" \
        -PD "LATE_STARTUP SETA $(LATE_STARTUP)" \
        -PD "THUMB_SUPPORT SETA $(THUMB_SUPPORT)" \
        -PD "ASSERT_ENABLED SETA $(ASSERT_ENABLED)" \
        -PD "MINIMAL_ANGEL SETA $(MINIMAL_ANGEL)"

LFLAGS=  -symbols $@.sym.lst

#
# set ARMTOOLS to point to the tools you wish to
# use (otherwise the default ones will be used)
#
# Note that tasm is used since this means the makefile does not have to
# be modified between enabling and disabling Thumb support
#
#ARMTOOLS=
CC=          $(ARMTOOLS)armcc
AS=          $(ARMTOOLS)tasm
LD=          $(ARMTOOLS)armlink
ALIB=      $(ARMTOOLS)armlib

OBJS=   serlasm.o devices.o devraw.o devmisc.o params.o \
        buffers.o logging.o except.o startlib.o serraw.o \
        stacks.o serlock.o

DRIVEROBJS=     serial.o panicblk.o logadp.o ladpasm.o

all: appldev

angmin.lib:     $(OBJS) supplib.o startrom.o drivers.a
                $(LD) -aof -o $@ $(OBJS) supplib.o drivers.a

drivers.a:      $(DRIVEROBJS)
        $(ALIB) -c -o drivers.a $(DRIVEROBJS)

#
# devconf.h has sanity checks upon definitions in this Makefile
#
../../pie/devconf.h:    Makefile
        chmod +w $@
        touch $@

# Rules for creating dependencies, to standard output, which can then be
#  cut and pasted back into this makefile above.

depends: devices.dep devmisc.dep devraw.dep buffers.dep params.dep
depends: logging.dep msgbuild.dep serial.dep serraw.dep
depends: panicblk.dep logadp.dep stacks.dep serlock.dep

devices.dep:    ../../pie/devices.c
        @$(CC) -Mx $(CFLAGS) $(CFLdevices) ../../pie/devices.c
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

logging.dep:    ../../logging.c
        @$(CC) -Mx $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") -I../../logging $?"

msgbuild.dep:   ../../msgbuild.c
        @$(CC) -Mx $(CFLAGS) $(CFLmsgbuild) ../../msgbuild.c
        @echo " $$(CC) $$(CFLAGS) $$(CFL"`basename $? .c`") $?"

serial.dep:       ../../pie/serial.c
        @$(CC) -Mx $(CFLAGS) $(CFLserial) ../../pie/serial.c
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

serlock.dep:    ../../serlock.c
        @$(CC) -Mx $(CFLAGS) $(CFLserlock) ../../serlock.c
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

ladpasm.o:            ../../logging/ladpasm.s
        $(AS)   ../../logging/ladpasm.s $(AFLAGS)

../lolevel.s:      ../../pie/makelo.c ../../angel.h ../../arm.h \
        ../../sys.h ../../pie/devconf.h
        cc -Ae +O2 -o makelo -I../.. -I../../pie ../../pie/makelo.c \
        $(CDEFS) $(ENDIANDEFN)
        ./makelo > ../lolevel.s

# And now the mechanically built dependencies

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
serial.o:       ../../rxtx.h
serial.o:       ../../prof.h
        $(CC) $(CFLAGS) $(CFLserial) ../../pie/serial.c
serraw.o:       ../../serraw.c
serraw.o:       ../../devappl.h
serraw.o:       ../../devraw.h
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
serraw.o:       ../../rxtx.h
serraw.o:       ../../angel.h
serraw.o:       ../../devclnt.h
        $(CC) $(CFLAGS) $(CFLserraw) ../../serraw.c
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

appldev:        appldev.o suppmsg.o angmin.lib rtstand.o
        $(LD) -o $@ appldev.o suppmsg.o angmin.lib startrom.o rtstand.o \
                -bin -nozeropad -base $(ROADDR) -data $(RWADDR)  \
                -first 'startrom.o(ROMStartup)' -symbols $@.sym.lst
        $(LD) -o $@.sym appldev.o suppmsg.o angmin.lib startrom.o rtstand.o \
                -aif -bin -nozeropad -base $(ROADDR) -data $(RWADDR)    \
                -first 'startrom.o(ROMStartup)'  -entry $(ROADDR)

appldev.o:      ../../examples/appldev.c
        $(CC) $(CFLAGS) $(CFLappldev) ../../examples/appldev.c

suppmsg.o:      msgbuild.o      # save repeating all the depends
        $(CC) $(CFLAGS) $(CFLsuppmsg) -DMSG_UTILS_ONLY -o suppmsg.o     \
                 ../../msgbuild.c

rtstand.o:      ../../examples/rtstand.s
        $(AS) ../../examples/rtstand.s $(AFLAGS)

# EOF
