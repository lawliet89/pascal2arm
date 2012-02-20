#
# $Revision: 1.2.2.1 $
#   $Author: ijohnson $
#     $Date: 1997/08/14 11:37:22 $
#
# Copyright (c) 1996 Advanced RISC Machines Limited.
# All Rights Reserved
#

CC=armcc -bi -c
LD=armlink

.c.o:
	$(CC) -APCS 3/32/noswst $< 

all: app/root

app/root: main.o uue.o getdata.o overmgrs.o initapp.o maininit.o
	$(LD) -bin -scatter scatdes -o app  maininit.o main.o overmgrs.o uue.o \
              getdata.o initapp.o -map -symbols - -first maininit.o \
              -list tS.map -v

maininit.o: maininit.s
	armasm -bi -APCS 3/32/noswst  maininit.s 

initapp.o: initapp.s
	armasm -bi -APCS 3/32/noswst  initapp.s 

overmgrs.o: overmgrs.s
	armasm -bi -APCS 3/32/noswst  overmgrs.s 

main.o: main.c

uue.o: uue.c

getdata.o: getdata.c

