# Microsoft Developer Studio Project File - Name="mount" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=mount - Win32 Checked
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mount.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mount.mak" CFG="mount - Win32 Checked"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mount - Win32 Free" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mount - Win32 Checked" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mount - Win32 Free"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Free"
# PROP BASE Intermediate_Dir "Free"
# PROP BASE Target_Dir ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Free"
# PROP Intermediate_Dir "Free"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /WX /Gy /Gz /Oxs /Oy /D "WIN32" /D "_WINDOWS" /c
# ADD CPP /nologo /W3 /WX /Gy /Gz /Oxs /Oy /D "WIN32=100" /D "_WINDOWS" /c /I "$(BASEDIR)\inc" 
# ADD CPP /I "$(CPU)\" /I "." /D "STD_CALL" /D "CONDITION_HANDLING=1" /D "NT_UP=1" /D "NT_INST=0" /D "_NT1X_=100"
# ADD CPP /D "WINNT=1" /D "_WIN32_WINNT=0x0400" /D "WIN32_LEAN_AND_MEAN=1" /D "DEVL=1" /D "FPO=1" /D "_IDWBUILD"
# ADD CPP /D "NDEBUG" /D "_DLL=1" /Zel /Zp8 /Gy -cbstring /QIfdiv- /QIf /GF /FI"$(BASEDIR)\inc\warning.h"
# ADD CPP /D "_X86_=1" /D "$(CPU)=1" 
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /I "$(BASEDIR)\inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo
# ADD LINK32 /nologo /driver /debug /debug:notmapped,MINIMAL /debugtype:coff  /pdb:none /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096
# ADD LINK32 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /entry:"DriverEntry" /incremental:no
# ADD LINK32 /nodefaultlib /out:build\i386\free\mount.sys /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA
# ADD LINK32 /libpath:"$(BASEDIR)\lib\i386\free"
# ADD LINK32 /align:0x20 /base:0x10000
# ADD LINK32 /version:4.00 /osversion:4.00
# ADD LINK32 /subsystem:native


!ELSEIF  "$(CFG)" == "mount - Win32 Checked"

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
# ADD BASE CPP /nologo /W3 /Gz /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gz /Zi /Od /D "WIN32=100" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c /I "$(BASEDIR)\inc" 
# ADD CPP /I "$(CPU)\" /I "." /D "STD_CALL" /D "CONDITION_HANDLING=1" /D "NT_UP=1" /D "NT_INST=0" /D "_NT1X_=100"
# ADD CPP /D "WINNT=1" /D "_WIN32_WINNT=0x0400" /D "WIN32_LEAN_AND_MEAN=1" /D "DBG=1" /D "DEVL=1" /D "FPO=0"
# ADD CPP /D "NDEBUG" /D "_DLL=1" /Zel /Zp8 /Gy -cbstring /QIfdiv- /QIf /GF /Z7 /Oi /FI"$(BASEDIR)\inc\warning.h"
# ADD CPP /D "_X86_=1" 
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /I "$(BASEDIR)\inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo 
# ADD LINK32 /nologo /driver /debug /debug:notmapped,FULL /debugtype:both /pdb:none /IGNORE:4001,4037,4039,4065,4078,4087,4089,4096
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /entry:"DriverEntry" /incremental:no
# ADD LINK32 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text
# ADD LINK32 /nodefaultlib /out:build\i386\checked\mount.sys /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA
# ADD LINK32 /libpath:"$(BASEDIR)\lib\i386\checked"
# ADD LINK32 /align:0x20 /base:0x10000
# ADD LINK32 /version:4.00 /osversion:4.00
# ADD LINK32 /subsystem:native

!ENDIF 

# Begin Target

# Name "mount - Win32 Free"
# Name "mount - Win32 Checked"
# Begin Group "Source Files"

# PROP Default_Filter ".c;.cpp"

# Begin Source File

SOURCE=.\mount.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"

# Begin Source File

SOURCE=.\mount.h
# End Source File
# Begin Source File

SOURCE=.\ioctlcode.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ".rc;.mc"


# End Group
# End Target
# End Project
