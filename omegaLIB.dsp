# Microsoft Developer Studio Project File - Name="omegaLIB" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=omegaLIB - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "omegaLIB.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "omegaLIB.mak" CFG="omegaLIB - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "omegaLIB - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "omegaLIB - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "omegaLIB - Win32 Release"

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

!ELSEIF  "$(CFG)" == "omegaLIB - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "omegaLIB___Win32_Debug"
# PROP BASE Intermediate_Dir "omegaLIB___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugLIB"
# PROP Intermediate_Dir "DebugLIB"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I ".\basic\include" /I ".\omega_lib\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /TP /c
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

# Name "omegaLIB - Win32 Release"
# Name "omegaLIB - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Omega Files"

# PROP Default_Filter ""
# Begin Group "Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_eq.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_exp_kill.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_global.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_print.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_problems.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_query.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_quick_kill.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_simple.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_solve.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\omega_core\oc_util.c
# End Source File
# End Group
# Begin Group "Basic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\basic\src\ConstString.c
# End Source File
# Begin Source File

SOURCE=.\basic\src\Exit.c
# End Source File
# Begin Source File

SOURCE=.\basic\src\Link.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\omega_lib\src\AST.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\closure.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\evac.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\farkas.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\hull.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\lib_hack.c

!IF  "$(CFG)" == "omegaLIB - Win32 Release"

!ELSEIF  "$(CFG)" == "omegaLIB - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_beaut.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_cnstr.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_col.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_conj.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_decl.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_dnf.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_form.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_gen.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_logic.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_print.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_quant.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_rear.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_subs.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\pres_var.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\reach.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\Relation.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\Relations.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\RelBody.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\RelVar.c
# End Source File
# Begin Source File

SOURCE=.\omega_lib\src\ubiquitous.c

!IF  "$(CFG)" == "omegaLIB - Win32 Release"

!ELSEIF  "$(CFG)" == "omegaLIB - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Omega Headers"

# PROP Default_Filter ""
# Begin Group "Basic Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\basic\include\basic\assert.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Bag.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\bool.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Collection.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Collections.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\ConstString.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Dynamic_Array.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_Bag.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_Collection.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_Iterator.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_List.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_Map.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_Section.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_String.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\enter_Tuple.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Exit.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Iterator.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_Bag.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_Collection.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_Iterator.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_List.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_Map.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_Section.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_String.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\leave_Tuple.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Link.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\List.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Map.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\NonCoercible.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Section.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\SimpleList.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\String.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\Tuple.h
# End Source File
# Begin Source File

SOURCE=.\basic\include\basic\util.h
# End Source File
# End Group
# Begin Group "Core Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\omega_lib\include\omega\AST.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\calc_debug.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\closure.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\omega_core\debugging.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\enter_AST.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\enter_omega.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\evac.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\farkas.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\hull.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\leave_AST.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\leave_omega.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\lib_hack.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\omega_core\oc.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\omega_core\oc_i.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\omega_i.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_cmpr.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_cnstr.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_conj.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_decl.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_dnf.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_form.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_gen.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_logic.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_quant.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_subs.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_tree.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\pres_var.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\reach.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\Rel_map.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\Relation.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\Relations.h
# End Source File
# Begin Source File

SOURCE=.\omega_lib\include\omega\RelBody.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\omega_lib\include\omega.h
# End Source File
# End Group
# End Group
# End Target
# End Project
