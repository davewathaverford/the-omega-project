# Microsoft Developer Studio Project File - Name="uniform" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=uniform - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uniform.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uniform.mak" CFG="uniform - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uniform - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "uniform - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "uniform - Win32 Release"

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

!ELSEIF  "$(CFG)" == "uniform - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./include" /I "./../basic/include" /I "./../omega_lib/include" /I "../petit/include" /I "../petit/obj" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /TP /c
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

# Name "uniform - Win32 Release"
# Name "uniform - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\communication.c
# End Source File
# Begin Source File

SOURCE=.\src\depend_dir.c
# End Source File
# Begin Source File

SOURCE=.\src\depend_rel2.c
# End Source File
# Begin Source File

SOURCE=.\src\parallelism.c
# End Source File
# Begin Source File

SOURCE=.\src\search.c
# End Source File
# Begin Source File

SOURCE=.\src\select.c
# End Source File
# Begin Source File

SOURCE=.\src\simple_codegen.c
# End Source File
# Begin Source File

SOURCE=.\src\space_constants.c
# End Source File
# Begin Source File

SOURCE=.\src\space_mappings.c
# End Source File
# Begin Source File

SOURCE=.\src\time_mappings.c
# End Source File
# Begin Source File

SOURCE=.\src\uniform.c
# End Source File
# Begin Source File

SOURCE=.\src\uniform_args.c
# End Source File
# Begin Source File

SOURCE=.\src\uniform_misc.c
# End Source File
# Begin Source File

SOURCE=.\src\wak_codegen.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\uniform\communication.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\depend_dir.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\depend_rel.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\parallelism.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\search.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\select.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\simple_codegen.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\space_constants.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\space_mappings.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\time_mappings.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\uniform.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\uniform_args.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\uniform_misc.h
# End Source File
# Begin Source File

SOURCE=.\include\uniform\wak_codegen.h
# End Source File
# End Group
# End Target
# End Project
