# Microsoft Developer Studio Project File - Name="petit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=petit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "petit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "petit.mak" CFG="petit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "petit - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "petit - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "petit - Win32 Release"

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

!ELSEIF  "$(CFG)" == "petit - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./include" /I "./../basic/include" /I "./../omega_lib/include" /I "./../uniform/include" /I "./../code_gen/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D BATCH_ONLY_PETIT=1 /D "OMIT_GETRUSAGE" /D "OMIT_QUANTIFY_CALLS" /FR /YX /FD /GZ /c
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

# Name "petit - Win32 Release"
# Name "petit - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\src\add-assert.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\affine.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arrayExpand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\browse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\browsedd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\build_stmt_rel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\compare.cpp
# End Source File
# Begin Source File

SOURCE=.\src\copy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\coverlist.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddcheck.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dddriver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddepsdriver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddepsilon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddInf.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddnest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddodriver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddomega.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ddutil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\depcycles.cpp
# End Source File
# Begin Source File

SOURCE=.\src\depend_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\eps_print.cpp
# End Source File
# Begin Source File

SOURCE=.\src\evaluate.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Exit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\f2p.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hpp.cpp
# End Source File
# Begin Source File

SOURCE=".\src\ivr-lang-interf.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\ivr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\kill.cpp
# End Source File
# Begin Source File

SOURCE=".\src\lang-interf.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\langmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\language.cpp
# End Source File
# Begin Source File

SOURCE=.\src\link.cpp
# End Source File
# Begin Source File

SOURCE=.\src\linkio.cpp
# End Source File
# Begin Source File

SOURCE=.\src\make.cpp
# End Source File
# Begin Source File

SOURCE=.\src\message.cpp
# End Source File
# Begin Source File

SOURCE=.\src\motif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\notedd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\parse_and_analyse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\petit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\petit_args.cpp
# End Source File
# Begin Source File

SOURCE=".\src\pres-interf.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\print.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PT.cpp
# End Source File
# Begin Source File

SOURCE=.\src\q_stubs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ssa.cpp
# End Source File
# Begin Source File

SOURCE=.\src\system.cpp
# End Source File
# Begin Source File

SOURCE=.\src\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\timeTrials.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tree.cpp
# End Source File
# Begin Source File

SOURCE=".\src\uniform-interf.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\vutil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Zima.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\include\petit\add-assert.h"
# End Source File
# Begin Source File

SOURCE=.\include\petit\affine.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\arrayExpand.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\browse.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\browsedd.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\build_stmt_rel.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\classes.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\classify.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\compare.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\copy.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\coverlist.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\dd_misc.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddcheck.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\dddir.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\dddriver.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddepsdriver.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddepsilon.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddInf.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddnest.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddodriver.h
# End Source File
# Begin Source File

SOURCE=".\include\petit\ddomega-use.h"
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddomega.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ddutil.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\debug.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\definitions.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\depcycles.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\depend_filter.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\eps_print.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\evaluate.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\Exit.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\f2p.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\find_affine.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\hpp.h
# End Source File
# Begin Source File

SOURCE=".\include\petit\ivr-lang-interf.h"
# End Source File
# Begin Source File

SOURCE=.\include\petit\ivr.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\kill.h
# End Source File
# Begin Source File

SOURCE=".\include\petit\lang-interf.h"
# End Source File
# Begin Source File

SOURCE=.\include\petit\langmenu.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\language.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\link.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\linkio.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\listdir.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\machdef.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\make.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\makeint.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\message.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\missing.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\motif.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\mouse.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\notedd.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\omega2flags.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ops.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\parse_and_analyse.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\petit_args.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\petity.h
# End Source File
# Begin Source File

SOURCE=".\include\petit\pres-interf.h"
# End Source File
# Begin Source File

SOURCE=.\include\petit\print.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\q_stubs.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\range.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\restart.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\ssa.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\stripMine.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\system.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\timer.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\timeTrials.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\tree.h
# End Source File
# Begin Source File

SOURCE=".\include\petit\uniform-interf.h"
# End Source File
# Begin Source File

SOURCE=.\include\petit\vutil.h
# End Source File
# Begin Source File

SOURCE=.\include\petit\Zima.h
# End Source File
# End Group
# End Target
# End Project
