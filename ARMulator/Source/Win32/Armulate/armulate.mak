# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (ALPHA) Dynamic-Link Library" 0x0602

!IF "$(CFG)" == ""
CFG=armulate - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to armulate - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "armulate - Win32 Release" && "$(CFG)" !=\
 "armulate - Win32 Debug" && "$(CFG)" != "armulate - Win32 (ALPHA) Release" &&\
 "$(CFG)" != "armulate - Win32 (ALPHA) Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "armulate.mak" CFG="armulate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "armulate - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "armulate - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "armulate - Win32 (ALPHA) Release" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "armulate - Win32 (ALPHA) Debug" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "armulate - Win32 Debug"

!IF  "$(CFG)" == "armulate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "IntelRel"
# PROP Intermediate_Dir "IntelRel"
OUTDIR=.\IntelRel
INTDIR=.\IntelRel

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : "$(OUTDIR)\armulate.dll" "$(OUTDIR)\armulate.bsc"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191 /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2\
 /D "NDEBUG" /D "WIN32" /D\
"_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191\
/FR"$(INTDIR)/" /Fp"$(INTDIR)/armulate.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\IntelRel/
CPP_SBRS=.\IntelRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/ARMU8DLL.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/armulate.bsc" 
BSC32_SBRS= \
        "$(INTDIR)\armflat.sbr" \
        "$(INTDIR)\armmap.sbr" \
        "$(INTDIR)\ARMU8DLL.SBR" \
        "$(INTDIR)\bytelane.sbr" \
        "$(INTDIR)\ANGEL.SBR" \
        "$(INTDIR)\DUMMYMMU.SBR" \
        "$(INTDIR)\errors.sbr" \
        "$(INTDIR)\MODELS.SBR" \
        "$(INTDIR)\pagetab.sbr" \
        "$(INTDIR)\profiler.sbr" \
        "$(INTDIR)\TRACER.SBR" \
        "$(INTDIR)\trickbox.sbr" \
        "$(INTDIR)\VALIDATE.SBR" \
        "$(INTDIR)\watchpnt.sbr" \
        "$(INTDIR)\winglass.sbr"

"$(OUTDIR)\armulate.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ..\Lib\clx.lib ..\Lib\armulib.lib ..\Lib\iarm.lib ..\Lib\sarmul.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt.lib" /verbose:lib
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=..\Lib\clx.lib ..\Lib\armulib.lib\
..\Lib\iarm.lib ..\Lib\sarmul.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/armulate.pdb"\
 /machine:I386 /nodefaultlib:"libcmt.lib" /def:".\armu8dll.def"\
 /out:"$(OUTDIR)/armulate.dll" /implib:"$(OUTDIR)/armulate.lib" /verbose:lib 
DEF_FILE= \
        ".\armu8dll.def"
LINK32_OBJS= \
        "$(INTDIR)\armflat.obj" \
        "$(INTDIR)\armmap.obj" \
        "$(INTDIR)\ARMU8DLL.OBJ" \
        "$(INTDIR)\ARMU8DLL.res" \
        "$(INTDIR)\bytelane.obj" \
        "$(INTDIR)\ANGEL.OBJ" \
        "$(INTDIR)\DUMMYMMU.OBJ" \
        "$(INTDIR)\errors.obj" \
        "$(INTDIR)\MODELS.OBJ" \
        "$(INTDIR)\pagetab.obj" \
        "$(INTDIR)\profiler.obj" \
        "$(INTDIR)\TRACER.OBJ" \
        "$(INTDIR)\trickbox.obj" \
        "$(INTDIR)\VALIDATE.OBJ" \
        "$(INTDIR)\watchpnt.obj" \
        "$(INTDIR)\winglass.obj"

"$(OUTDIR)\armulate.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "IntelDbg"
# PROP Intermediate_Dir "IntelDbg"
OUTDIR=.\IntelDbg
INTDIR=.\IntelDbg

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : "$(OUTDIR)\armulate.dll" "$(OUTDIR)\armulate.bsc"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "ARM_DEBUGGING" /D "WIN32" /D "_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191 /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od\
 /D "_DEBUG" /D\
"ARM_DEBUGGING" /D "WIN32" /D "_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D\
HOURGLASS_RATE=8191 /FR"$(INTDIR)/" /Fp"$(INTDIR)/armulate.pch" /YX\
/Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c
CPP_OBJS=.\IntelDbg/
CPP_SBRS=.\IntelDbg/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/ARMU8DLL.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/armulate.bsc" 
BSC32_SBRS= \
        "$(INTDIR)\armflat.sbr" \
        "$(INTDIR)\armmap.sbr" \
        "$(INTDIR)\ARMU8DLL.SBR" \
        "$(INTDIR)\bytelane.sbr" \
        "$(INTDIR)\ANGEL.SBR" \
        "$(INTDIR)\DUMMYMMU.SBR" \
        "$(INTDIR)\errors.sbr" \
        "$(INTDIR)\MODELS.SBR" \
        "$(INTDIR)\pagetab.sbr" \
        "$(INTDIR)\profiler.sbr" \
        "$(INTDIR)\TRACER.SBR" \
        "$(INTDIR)\trickbox.sbr" \
        "$(INTDIR)\VALIDATE.SBR" \
        "$(INTDIR)\watchpnt.sbr" \
        "$(INTDIR)\winglass.sbr"

"$(OUTDIR)\armulate.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 ..\Lib\clx.lib ..\Lib\armulib.lib ..\Lib\iarm.lib ..\Lib\sarmul.lib ..\Lib\windebug.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt.lib"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=..\Lib\clx.lib ..\Lib\armulib.lib\
..\Lib\iarm.lib ..\Lib\sarmul.lib\
..\Lib\windebug.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/armulate.pdb" /debug /machine:I386\
 /nodefaultlib:"libcmt.lib" /def:".\armu8dll.def" /out:"$(OUTDIR)/armulate.dll"\
 /implib:"$(OUTDIR)/armulate.lib" 
DEF_FILE= \
        ".\armu8dll.def"
LINK32_OBJS= \
        "$(INTDIR)\armflat.obj" \
        "$(INTDIR)\armmap.obj" \
        "$(INTDIR)\ARMU8DLL.OBJ" \
        "$(INTDIR)\ARMU8DLL.res" \
        "$(INTDIR)\bytelane.obj" \
        "$(INTDIR)\ANGEL.OBJ" \
        "$(INTDIR)\DUMMYMMU.OBJ" \
        "$(INTDIR)\errors.obj" \
        "$(INTDIR)\MODELS.OBJ" \
        "$(INTDIR)\pagetab.obj" \
        "$(INTDIR)\profiler.obj" \
        "$(INTDIR)\TRACER.OBJ" \
        "$(INTDIR)\trickbox.obj" \
        "$(INTDIR)\VALIDATE.OBJ" \
        "$(INTDIR)\watchpnt.obj" \
        "$(INTDIR)\winglass.obj"

"$(OUTDIR)\armulate.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ARMULATE"
# PROP BASE Intermediate_Dir "ARMULATE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "AlphaRel"
# PROP Intermediate_Dir "AlphaRel"
# PROP Target_Dir ""
OUTDIR=.\AlphaRel
INTDIR=.\AlphaRel

ALL : "$(OUTDIR)\armulate.dll" "$(OUTDIR)\armulate.bsc"

CLEAN : 
        -@erase ".\AlphaRel\armulate.bsc"
        -@erase ".\AlphaRel\trickbox.sbr"
        -@erase ".\AlphaRel\winglass.sbr"
        -@erase ".\AlphaRel\profiler.sbr"
        -@erase ".\AlphaRel\TRACER.SBR"
        -@erase ".\AlphaRel\pagetab.sbr"
        -@erase ".\AlphaRel\errors.sbr"
        -@erase ".\AlphaRel\watchpnt.sbr"
        -@erase ".\AlphaRel\DUMMYMMU.SBR"
        -@erase ".\AlphaRel\armmap.sbr"
        -@erase ".\AlphaRel\ARMU8DLL.SBR"
        -@erase ".\AlphaRel\VALIDATE.SBR"
        -@erase ".\AlphaRel\armflat.sbr"
        -@erase ".\AlphaRel\ANGEL.SBR"
        -@erase ".\AlphaRel\bytelane.sbr"
        -@erase ".\AlphaRel\MODELS.SBR"
        -@erase ".\AlphaRel\armulate.dll"
        -@erase ".\AlphaRel\ANGEL.OBJ"
        -@erase ".\AlphaRel\bytelane.obj"
        -@erase ".\AlphaRel\MODELS.OBJ"
        -@erase ".\AlphaRel\trickbox.obj"
        -@erase ".\AlphaRel\winglass.obj"
        -@erase ".\AlphaRel\profiler.obj"
        -@erase ".\AlphaRel\TRACER.OBJ"
        -@erase ".\AlphaRel\pagetab.obj"
        -@erase ".\AlphaRel\errors.obj"
        -@erase ".\AlphaRel\watchpnt.obj"
        -@erase ".\AlphaRel\DUMMYMMU.OBJ"
        -@erase ".\AlphaRel\armmap.obj"
        -@erase ".\AlphaRel\ARMU8DLL.OBJ"
        -@erase ".\AlphaRel\VALIDATE.OBJ"
        -@erase ".\AlphaRel\armflat.obj"
        -@erase ".\AlphaRel\ARMU8DLL.res"
        -@erase ".\AlphaRel\armulate.lib"
        -@erase ".\AlphaRel\armulate.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /Gt0 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191 /FR /YX /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /GX /O2\
 /D "NDEBUG" /D "WIN32" /D\
"_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191\
/FR"$(INTDIR)/" /Fp"$(INTDIR)/armulate.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\AlphaRel/
CPP_SBRS=.\AlphaRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /alpha
# ADD MTL /nologo /D "NDEBUG" /alpha
MTL_PROJ=/nologo /D "NDEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/ARMU8DLL.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/armulate.bsc" 
BSC32_SBRS= \
        "$(INTDIR)/trickbox.sbr" \
        "$(INTDIR)/winglass.sbr" \
        "$(INTDIR)/profiler.sbr" \
        "$(INTDIR)/TRACER.SBR" \
        "$(INTDIR)/pagetab.sbr" \
        "$(INTDIR)/errors.sbr" \
        "$(INTDIR)/watchpnt.sbr" \
        "$(INTDIR)/DUMMYMMU.SBR" \
        "$(INTDIR)/armmap.sbr" \
        "$(INTDIR)/ARMU8DLL.SBR" \
        "$(INTDIR)/VALIDATE.SBR" \
        "$(INTDIR)/armflat.sbr" \
        "$(INTDIR)/ANGEL.SBR" \
        "$(INTDIR)/bytelane.sbr" \
        "$(INTDIR)/MODELS.SBR"

"$(OUTDIR)\armulate.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:ALPHA
# ADD LINK32 ..\Lib\clx.lib ..\Lib\iarm.lib ..\Lib\armulib.lib ..\Lib\sarmul.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:ALPHA
LINK32_FLAGS=..\Lib\clx.lib ..\Lib\iarm.lib\
..\Lib\armulib.lib ..\Lib\sarmul.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/armulate.pdb" /machine:ALPHA\
 /def:".\armu8dll.def" /out:"$(OUTDIR)/armulate.dll"\
 /implib:"$(OUTDIR)/armulate.lib" 
DEF_FILE= \
        ".\armu8dll.def"
LINK32_OBJS= \
        "$(INTDIR)/ANGEL.OBJ" \
        "$(INTDIR)/bytelane.obj" \
        "$(INTDIR)/MODELS.OBJ" \
        "$(INTDIR)/trickbox.obj" \
        "$(INTDIR)/winglass.obj" \
        "$(INTDIR)/profiler.obj" \
        "$(INTDIR)/TRACER.OBJ" \
        "$(INTDIR)/pagetab.obj" \
        "$(INTDIR)/errors.obj" \
        "$(INTDIR)/watchpnt.obj" \
        "$(INTDIR)/DUMMYMMU.OBJ" \
        "$(INTDIR)/armmap.obj" \
        "$(INTDIR)/ARMU8DLL.OBJ" \
        "$(INTDIR)/VALIDATE.OBJ" \
        "$(INTDIR)/armflat.obj" \
        "$(INTDIR)/ARMU8DLL.res"

"$(OUTDIR)\armulate.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ARMULAT0"
# PROP BASE Intermediate_Dir "ARMULAT0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AlphaDbg"
# PROP Intermediate_Dir "AlphaDbg"
# PROP Target_Dir ""
OUTDIR=.\AlphaDbg
INTDIR=.\AlphaDbg

ALL : "$(OUTDIR)\armulate.dll" "$(OUTDIR)\armulate.bsc"

CLEAN : 
        -@erase ".\AlphaDbg\vc40.pdb"
        -@erase ".\AlphaDbg\armulate.bsc"
        -@erase ".\AlphaDbg\pagetab.sbr"
        -@erase ".\AlphaDbg\armflat.sbr"
        -@erase ".\AlphaDbg\MODELS.SBR"
        -@erase ".\AlphaDbg\winglass.sbr"
        -@erase ".\AlphaDbg\ARMU8DLL.SBR"
        -@erase ".\AlphaDbg\ANGEL.SBR"
        -@erase ".\AlphaDbg\TRACER.SBR"
        -@erase ".\AlphaDbg\VALIDATE.SBR"
        -@erase ".\AlphaDbg\errors.sbr"
        -@erase ".\AlphaDbg\bytelane.sbr"
        -@erase ".\AlphaDbg\watchpnt.sbr"
        -@erase ".\AlphaDbg\DUMMYMMU.SBR"
        -@erase ".\AlphaDbg\trickbox.sbr"
        -@erase ".\AlphaDbg\armmap.sbr"
        -@erase ".\AlphaDbg\profiler.sbr"
        -@erase ".\AlphaDbg\armulate.dll"
        -@erase ".\AlphaDbg\trickbox.obj"
        -@erase ".\AlphaDbg\armmap.obj"
        -@erase ".\AlphaDbg\profiler.obj"
        -@erase ".\AlphaDbg\pagetab.obj"
        -@erase ".\AlphaDbg\armflat.obj"
        -@erase ".\AlphaDbg\MODELS.OBJ"
        -@erase ".\AlphaDbg\winglass.obj"
        -@erase ".\AlphaDbg\ARMU8DLL.OBJ"
        -@erase ".\AlphaDbg\ANGEL.OBJ"
        -@erase ".\AlphaDbg\TRACER.OBJ"
        -@erase ".\AlphaDbg\VALIDATE.OBJ"
        -@erase ".\AlphaDbg\errors.obj"
        -@erase ".\AlphaDbg\bytelane.obj"
        -@erase ".\AlphaDbg\watchpnt.obj"
        -@erase ".\AlphaDbg\DUMMYMMU.OBJ"
        -@erase ".\AlphaDbg\ARMU8DLL.res"
        -@erase ".\AlphaDbg\armulate.ilk"
        -@erase ".\AlphaDbg\armulate.lib"
        -@erase ".\AlphaDbg\armulate.exp"
        -@erase ".\AlphaDbg\armulate.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MDd /Gt0 /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191 /FR /YX /c
CPP_PROJ=/nologo /MDd /Gt0 /W3 /GX /Zi /Od\
 /D "_DEBUG" /D "WIN32"\
/D "_WINDOWS" /D "__STDC__" /D "RDI_VERBOSE" /D HOURGLASS_RATE=8191\
/FR"$(INTDIR)/" /Fp"$(INTDIR)/armulate.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
/c
CPP_OBJS=.\AlphaDbg/
CPP_SBRS=.\AlphaDbg/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/ARMU8DLL.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/armulate.bsc" 
BSC32_SBRS= \
        "$(INTDIR)/pagetab.sbr" \
        "$(INTDIR)/armflat.sbr" \
        "$(INTDIR)/MODELS.SBR" \
        "$(INTDIR)/winglass.sbr" \
        "$(INTDIR)/ARMU8DLL.SBR" \
        "$(INTDIR)/ANGEL.SBR" \
        "$(INTDIR)/TRACER.SBR" \
        "$(INTDIR)/VALIDATE.SBR" \
        "$(INTDIR)/errors.sbr" \
        "$(INTDIR)/bytelane.sbr" \
        "$(INTDIR)/watchpnt.sbr" \
        "$(INTDIR)/DUMMYMMU.SBR" \
        "$(INTDIR)/trickbox.sbr" \
        "$(INTDIR)/armmap.sbr" \
        "$(INTDIR)/profiler.sbr"

"$(OUTDIR)\armulate.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 ..\Lib\clx.lib ..\Lib\iarm.lib ..\Lib\armulib.lib ..\Lib\sarmul.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT LINK32 /incremental:no
LINK32_FLAGS=..\Lib\clx.lib ..\Lib\iarm.lib\
..\Lib\armulib.lib ..\Lib\sarmul.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/armulate.pdb" /debug /machine:ALPHA\
 /def:".\armu8dll.def" /out:"$(OUTDIR)/armulate.dll"\
 /implib:"$(OUTDIR)/armulate.lib" 
DEF_FILE= \
        ".\armu8dll.def"
LINK32_OBJS= \
        "$(INTDIR)/trickbox.obj" \
        "$(INTDIR)/armmap.obj" \
        "$(INTDIR)/profiler.obj" \
        "$(INTDIR)/pagetab.obj" \
        "$(INTDIR)/armflat.obj" \
        "$(INTDIR)/MODELS.OBJ" \
        "$(INTDIR)/winglass.obj" \
        "$(INTDIR)/ARMU8DLL.OBJ" \
        "$(INTDIR)/ANGEL.OBJ" \
        "$(INTDIR)/TRACER.OBJ" \
        "$(INTDIR)/VALIDATE.OBJ" \
        "$(INTDIR)/errors.obj" \
        "$(INTDIR)/bytelane.obj" \
        "$(INTDIR)/watchpnt.obj" \
        "$(INTDIR)/DUMMYMMU.OBJ" \
        "$(INTDIR)/ARMU8DLL.res"

"$(OUTDIR)\armulate.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "armulate - Win32 Release"
# Name "armulate - Win32 Debug"
# Name "armulate - Win32 (ALPHA) Release"
# Name "armulate - Win32 (ALPHA) Debug"

!IF  "$(CFG)" == "armulate - Win32 Release"

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ARMU8DLL.RC
!IF  "$(CFG)" == "armulate - Win32 Release"

"$(INTDIR)\ARMU8DLL.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

"$(INTDIR)\ARMU8DLL.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"


"$(INTDIR)\ARMU8DLL.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x809 /fo"$(INTDIR)/ARMU8DLL.res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"


"$(INTDIR)\ARMU8DLL.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x809 /fo"$(INTDIR)/ARMU8DLL.res" /d "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\armu8dll.def
!IF  "$(CFG)" == "armulate - Win32 Release"

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ARMU8DLL.C
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_ARMU8=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\DBG_CP.H"\
	".\debug.h"\
	".\windebug.h"\
	".\winrdi.h"\
	".\armu8dll.h"\
        

"$(INTDIR)\ARMU8DLL.OBJ" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"

"$(INTDIR)\ARMU8DLL.SBR" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_ARMU8=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\DBG_CP.H"\
	".\debug.h"\
	".\windebug.h"\
	".\winrdi.h"\
	".\armu8dll.h"\
        

"$(INTDIR)\ARMU8DLL.OBJ" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"

"$(INTDIR)\ARMU8DLL.SBR" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_ARMU8=\
	".\armu8dll.h"\
	".\winrdi.h"\
	".\windebug.h"\
	".\multirdi.h"\
	".\armdbg.h"\
	".\dbg_rdi.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\toolconf.h"\
	".\dbg_conf.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\debug.h"\
        

"$(INTDIR)\ARMU8DLL.OBJ" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"

"$(INTDIR)\ARMU8DLL.SBR" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_ARMU8=\
	".\armu8dll.h"\
	".\winrdi.h"\
	".\windebug.h"\
	".\multirdi.h"\
	".\armdbg.h"\
	".\dbg_rdi.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\toolconf.h"\
	".\dbg_conf.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\debug.h"\
        

"$(INTDIR)\ARMU8DLL.OBJ" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"

"$(INTDIR)\ARMU8DLL.SBR" : $(SOURCE) $(DEP_CPP_ARMU8) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VALIDATE.C
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_VALID=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\dbg_hif.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\VALIDATE.OBJ" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\VALIDATE.SBR" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_VALID=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\dbg_hif.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\VALIDATE.OBJ" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\VALIDATE.SBR" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_VALID=\
	".\armdefs.h"\
	".\dbg_hif.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\VALIDATE.OBJ" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\VALIDATE.SBR" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_VALID=\
	".\armdefs.h"\
	".\dbg_hif.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\VALIDATE.OBJ" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\VALIDATE.SBR" : $(SOURCE) $(DEP_CPP_VALID) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TRACER.C
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_TRACE=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\TRACER.H"\
        

"$(INTDIR)\TRACER.OBJ" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TRACER.SBR" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_TRACE=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\TRACER.H"\
        

"$(INTDIR)\TRACER.OBJ" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TRACER.SBR" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_TRACE=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\TRACER.H"\
	".\disass.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\TRACER.OBJ" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TRACER.SBR" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_TRACE=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\TRACER.H"\
	".\disass.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\TRACER.OBJ" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TRACER.SBR" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ANGEL.C
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_ANGEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\adp.h"\
	".\angel.h"\
	".\arm.h"\
	".\CHANDEFS.H"\
	".\DBG_CP.H"\
	".\dbg_hif.h"\
	".\aif.h"\
	".\armerrs.h"\
	".\armfpe.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\ANGEL.OBJ" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ANGEL.SBR" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_ANGEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\adp.h"\
	".\angel.h"\
	".\arm.h"\
	".\CHANDEFS.H"\
	".\DBG_CP.H"\
	".\dbg_hif.h"\
	".\aif.h"\
	".\armerrs.h"\
	".\armfpe.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\ANGEL.OBJ" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ANGEL.SBR" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_ANGEL=\
	".\armdefs.h"\
	".\arm.h"\
	".\dbg_hif.h"\
	".\armcnf.h"\
	".\adp.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
	".\angel.h"\
	".\CHANDEFS.H"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\ANGEL.OBJ" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ANGEL.SBR" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_ANGEL=\
	".\armdefs.h"\
	".\arm.h"\
	".\dbg_hif.h"\
	".\armcnf.h"\
	".\adp.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
	".\angel.h"\
	".\CHANDEFS.H"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\ANGEL.OBJ" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ANGEL.SBR" : $(SOURCE) $(DEP_CPP_ANGEL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DUMMYMMU.C
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_DUMMY=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\DUMMYMMU.OBJ" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DUMMYMMU.SBR" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_DUMMY=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\DUMMYMMU.OBJ" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DUMMYMMU.SBR" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_DUMMY=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\DUMMYMMU.OBJ" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DUMMYMMU.SBR" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_DUMMY=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\DUMMYMMU.OBJ" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DUMMYMMU.SBR" : $(SOURCE) $(DEP_CPP_DUMMY) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MODELS.C
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_MODEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\MODELS.H"\
        

"$(INTDIR)\MODELS.OBJ" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MODELS.SBR" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_MODEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\MODELS.H"\
        

"$(INTDIR)\MODELS.OBJ" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MODELS.SBR" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_MODEL=\
	".\armdefs.h"\
	".\MODELS.H"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\MODELS.OBJ" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MODELS.SBR" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_MODEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\MODELS.H"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\MODELS.OBJ" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MODELS.SBR" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\trickbox.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_TRICK=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\trickbox.obj" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\trickbox.sbr" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_TRICK=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\trickbox.obj" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\trickbox.sbr" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_TRICK=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\trickbox.obj" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\trickbox.sbr" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_TRICK=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\trickbox.obj" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\trickbox.sbr" : $(SOURCE) $(DEP_CPP_TRICK) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bytelane.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_BYTEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\bytelane.obj" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\bytelane.sbr" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_BYTEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\bytelane.obj" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\bytelane.sbr" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_BYTEL=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\bytelane.obj" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\bytelane.sbr" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_BYTEL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\bytelane.obj" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\bytelane.sbr" : $(SOURCE) $(DEP_CPP_BYTEL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winglass.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_WINGL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\winglass.obj" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winglass.sbr" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_WINGL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\winglass.obj" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winglass.sbr" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_WINGL=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\winglass.obj" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winglass.sbr" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_WINGL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\winglass.obj" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winglass.sbr" : $(SOURCE) $(DEP_CPP_WINGL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\profiler.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_PROFI=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\profiler.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\profiler.sbr" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_PROFI=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\profiler.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\profiler.sbr" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_PROFI=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\profiler.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\profiler.sbr" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_PROFI=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\profiler.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\profiler.sbr" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\errors.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_ERROR=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\errors.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\errors.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_ERROR=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\errors.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\errors.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_ERROR=\
	".\armdefs.h"\
	".\errors.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\errors.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\errors.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_ERROR=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\errors.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\errors.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\armmap.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_ARMMA=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\linklist.h"\
        

"$(INTDIR)\armmap.obj" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armmap.sbr" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_ARMMA=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\linklist.h"\
        

"$(INTDIR)\armmap.obj" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armmap.sbr" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_ARMMA=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\dbg_stat.h"\
	".\linklist.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\armmap.obj" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armmap.sbr" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_ARMMA=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\linklist.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\armmap.obj" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armmap.sbr" : $(SOURCE) $(DEP_CPP_ARMMA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\watchpnt.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_WATCH=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\linklist.h"\
        

"$(INTDIR)\watchpnt.obj" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\watchpnt.sbr" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_WATCH=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\linklist.h"\
        

"$(INTDIR)\watchpnt.obj" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\watchpnt.sbr" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_WATCH=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\linklist.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\watchpnt.obj" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\watchpnt.sbr" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_WATCH=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
	".\linklist.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\watchpnt.obj" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\watchpnt.sbr" : $(SOURCE) $(DEP_CPP_WATCH) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\armflat.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_ARMFL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\armflat.obj" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armflat.sbr" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_ARMFL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\armflat.obj" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armflat.sbr" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_ARMFL=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\armflat.obj" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armflat.sbr" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"

DEP_CPP_ARMFL=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\armflat.obj" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\armflat.sbr" : $(SOURCE) $(DEP_CPP_ARMFL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pagetab.c
!IF  "$(CFG)" == "armulate - Win32 Release"

DEP_CPP_PAGET=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\armtypes.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armcnf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\pagetab.obj" : $(SOURCE) $(DEP_CPP_PAGET) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pagetab.sbr" : $(SOURCE) $(DEP_CPP_PAGET) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 Debug"

DEP_CPP_PAGET=\
	".\armdbg.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\dbg_stat.h"\
	".\armtypes.h"\
	".\host.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\toolconf.h"\
	".\armcnf.h"\
	".\armdefs.h"\
	".\DBG_CP.H"\
	".\armerrs.h"\
	".\armmem.h"\
	".\errors.h"\
        

"$(INTDIR)\pagetab.obj" : $(SOURCE) $(DEP_CPP_PAGET) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pagetab.sbr" : $(SOURCE) $(DEP_CPP_PAGET) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Release"

DEP_CPP_PAGET=\
	".\armdefs.h"\
	".\armcnf.h"\
	".\host.h"\
	".\toolconf.h"\
	".\armdbg.h"\
	".\armerrs.h"\
	".\armmem.h"\
	".\ieeeflt.h"\
	".\msg.h"\
	".\armtypes.h"\
	".\dbg_conf.h"\
	".\dbg_rdi.h"\
	".\DBG_CP.H"\
	".\dbg_stat.h"\
	".\errors.h"\
        

BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\pagetab.obj" : $(SOURCE) $(DEP_CPP_PAGET) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pagetab.sbr" : $(SOURCE) $(DEP_CPP_PAGET) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "armulate - Win32 (ALPHA) Debug"


BuildCmds= \
        $(CPP) $(CPP_PROJ) $(SOURCE) \
        

"$(INTDIR)\pagetab.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pagetab.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
