# RCS $Revision: 1.2.6.1 $
# Checkin $Date: 1998/03/06 14:22:54 $
# Revising $Author: dearlam $
#
# Makefile for NMAKE from VC++ 4.0

#dearlam 3.3.1998
# get the names of products from the install program directory (prodver.h)
# so that the programs can create registry entry for the correct version of the correct
# product, and the uninstaller, uninstalls the correct one.

INSTALL_INCLUDE= ..\windows\setup

!IF "$(CFG)" == ""
CFG=Release
!MESSAGE No configuration specified.  Defaulting to $(CFG)
!ENDIF 

!if [if not exist $(CFG) md $(CFG)]
!endif

all : $(CFG)\custom.dll
$(CFG)\custom.obj : custom.c
               cl -c -I$(INSTALL_INCLUDE) custom.c -DWIN32 /Fo$(CFG)\custom.obj

$(CFG)\custom.res : custom.rc resource.h $(INSTALL_INCLUDE)\prodver.h
               rc /r -I$(INSTALL_INCLUDE)  /fo$(CFG)\custom.res custom.rc 

custom.h: resource.h
               copy resource.h custom.h

$(CFG)\custom.dll : $(CFG)\custom.obj $(CFG)\custom.res custom.def custom.h
        link $(CFG)\custom.obj $(CFG)\custom.res /def:custom.def /out:$(CFG)\custom.dll /DLL
