# Microsoft Developer Studio Project File - Name="ext2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ext2 - Win32 Checked
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ext2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ext2.mak" CFG="ext2 - Win32 Checked"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ext2 - Win32 Free" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ext2 - Win32 Checked" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ext2 - Win32 Free"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Free"
# PROP BASE Intermediate_Dir "Free"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Free"
# PROP Intermediate_Dir "Free"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /WX /Oy /Gy /D "WIN32" /D "_WINDOWS" /Oxs /c
# ADD CPP /nologo /Gz /W3 /WX /Oy /Gy /I "$(BASEDIR)\inc" /I "$(CPU)\\" /I "." /FI"$(BASEDIR)\inc\warning.h" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D $(CPU)=1 /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(BASEDIR)\inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /machine:IX86
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:"build\i386\free\ext2r.sys" /libpath:"$(BASEDIR)\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Checked"
# PROP BASE Intermediate_Dir "Checked"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Checked"
# PROP Intermediate_Dir "Checked"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gz /W3 /Z7 /Oi /Gy /I "$(BASEDIR)\inc" /I "$(CPU)\\" /I "." /FI"$(BASEDIR)\inc\warning.h" /D WIN32=100 /D "_DEBUG" /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D _X86_=1 /FR /YX /FD /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(BASEDIR)\inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /machine:IX86
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:both /machine:IX86 /nodefaultlib /out:"build\i386\checked\ext2r.sys" /libpath:"$(BASEDIR)\lib\i386\checked" /driver /debug:notmapped,FULL /IGNORE:4001,4037,4039,4065,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=ice.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ext2 - Win32 Free"
# Name "ext2 - Win32 Checked"
# Begin Group "Source Files"

# PROP Default_Filter ".c;.cpp"
# Begin Source File

SOURCE=.\cachesup.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cleanup.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\close.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cmcallbacks.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\create.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\devcontrol.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dirctrl.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\disk.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dispatch.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\except.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ext2.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ext2alg.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ext2api.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fastio.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fileinfo.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fscontrol.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\helpers.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\options.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\read.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\struct.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\versup.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\volumeinfo.cpp

!IF  "$(CFG)" == "ext2 - Win32 Free"

!ELSEIF  "$(CFG)" == "ext2 - Win32 Checked"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cachesup.h
# End Source File
# Begin Source File

SOURCE=.\cleanup.h
# End Source File
# Begin Source File

SOURCE=.\close.h
# End Source File
# Begin Source File

SOURCE=.\cmcallbacks.h
# End Source File
# Begin Source File

SOURCE=.\codetables.h
# End Source File
# Begin Source File

SOURCE=.\create.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\devcontrol.h
# End Source File
# Begin Source File

SOURCE=.\dirctrl.h
# End Source File
# Begin Source File

SOURCE=.\disk.h
# End Source File
# Begin Source File

SOURCE=.\dispatch.h
# End Source File
# Begin Source File

SOURCE=.\except.h
# End Source File
# Begin Source File

SOURCE=.\ext2.h
# End Source File
# Begin Source File

SOURCE=.\ext2alg.h
# End Source File
# Begin Source File

SOURCE=.\ext2api.h
# End Source File
# Begin Source File

SOURCE=.\ext2fs.h
# End Source File
# Begin Source File

SOURCE=.\fastio.h
# End Source File
# Begin Source File

SOURCE=.\fileinfo.h
# End Source File
# Begin Source File

SOURCE=.\fscontrol.h
# End Source File
# Begin Source File

SOURCE=.\helpers.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\read.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\struct.h
# End Source File
# Begin Source File

SOURCE=.\versup.h
# End Source File
# Begin Source File

SOURCE=.\volumeinfo.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ".rc;.mc"
# End Group
# End Target
# End Project
