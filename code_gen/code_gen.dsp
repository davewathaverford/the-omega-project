# Microsoft Developer Studio Project File - Name="code_gen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=code_gen - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "code_gen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "code_gen.mak" CFG="code_gen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "code_gen - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "code_gen - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "code_gen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "code_gen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./include" /I "./../basic/include" /I "./../omega_lib/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "code_gen - Win32 Release"
# Name "code_gen - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\CG_stringBuilder.c
# End Source File
# Begin Source File

SOURCE=.\src\CG_stringRepr.c
# End Source File
# Begin Source File

SOURCE=.\src\code_gen.c
# End Source File
# Begin Source File

SOURCE=.\src\elim.c
# End Source File
# Begin Source File

SOURCE=".\src\mmap-checks.c"
# End Source File
# Begin Source File

SOURCE=".\src\mmap-codegen.c"
# End Source File
# Begin Source File

SOURCE=".\src\mmap-sub.c"
# End Source File
# Begin Source File

SOURCE=".\src\mmap-test.c"
# End Source File
# Begin Source File

SOURCE=".\src\mmap-util.c"
# End Source File
# Begin Source File

SOURCE=.\src\spmd.c
# End Source File
# Begin Source File

SOURCE=.\src\stmt_builder.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\code_gen\CG.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\CG_outputBuilder.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\CG_outputRepr.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\CG_stringBuilder.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\CG_stringRepr.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\code_gen.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\elim.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\spmd.h
# End Source File
# Begin Source File

SOURCE=.\include\code_gen\stmt_builder.h
# End Source File
# End Group
# End Target
# End Project
