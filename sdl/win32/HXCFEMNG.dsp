# Microsoft Developer Studio Project File - Name="HXCFEMNG" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=HXCFEMNG - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HXCFEMNG.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HXCFEMNG.mak" CFG="HXCFEMNG - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HXCFEMNG - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HXCFEMNG - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HXCFEMNG - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I ".." /I "../../fat32" /I "./sdl" /I "../../" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D FATFS_IS_LITTLE_ENDIAN=1 /D "CORTEX_FW_SUPPORT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sdl.lib /nologo /subsystem:console /machine:I386 /libpath:"./sdl/lib"

!ELSEIF  "$(CFG)" == "HXCFEMNG - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I ".." /I "../../fat32" /I "./sdl" /I "../../" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D FATFS_IS_LITTLE_ENDIAN=1 /D "DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sdl.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"./sdl/lib"

!ENDIF 

# Begin Target

# Name "HXCFEMNG - Win32 Release"
# Name "HXCFEMNG - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "fat32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\fat32\fat_access.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_access.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_cache.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_cache.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_filelib.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_filelib.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_format.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_format.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_opts.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_string.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_string.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_table.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_table.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_types.h
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_write.c
# End Source File
# Begin Source File

SOURCE=..\..\fat32\fat_write.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\config_file.c
# End Source File
# Begin Source File

SOURCE=..\..\errors_def.c
# End Source File
# Begin Source File

SOURCE=..\..\fectrl.c
# End Source File
# Begin Source File

SOURCE=..\..\gui_utils.c
# End Source File
# Begin Source File

SOURCE=..\..\media_access.c
# End Source File
# Begin Source File

SOURCE=..\..\menu.c
# End Source File
# Begin Source File

SOURCE=..\..\menu_selectdrive.c
# End Source File
# Begin Source File

SOURCE=..\..\menu_settings.c
# End Source File
# Begin Source File

SOURCE=..\..\msg_txt.c
# End Source File
# Begin Source File

SOURCE=..\sdl_hal.c
# End Source File
# Begin Source File

SOURCE=..\slot_list_gen.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "graphx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\graphx\data_bmp_font8x8_bmp.h
# End Source File
# Begin Source File

SOURCE=..\..\graphx\data_bmp_hxc2001_smalllogo_bmp.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\cfg_file.h
# End Source File
# Begin Source File

SOURCE=..\..\config_file.h
# End Source File
# Begin Source File

SOURCE=..\..\cortex_cfg_file.h
# End Source File
# Begin Source File

SOURCE=..\..\errors_def.h
# End Source File
# Begin Source File

SOURCE=..\..\fectrl.h
# End Source File
# Begin Source File

SOURCE=..\..\gui_utils.h
# End Source File
# Begin Source File

SOURCE=..\..\hal.h
# End Source File
# Begin Source File

SOURCE=..\..\hxcfeda.h
# End Source File
# Begin Source File

SOURCE=..\..\keys_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\keysfunc_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\media_access.h
# End Source File
# Begin Source File

SOURCE=..\..\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\menu_commands.c
# End Source File
# Begin Source File

SOURCE=..\..\menu_commands.h
# End Source File
# Begin Source File

SOURCE=..\..\menu_selectdrive.h
# End Source File
# Begin Source File

SOURCE=..\..\menu_settings.h
# End Source File
# Begin Source File

SOURCE=..\..\msg_txt.h
# End Source File
# Begin Source File

SOURCE=..\..\ui_context.h
# End Source File
# Begin Source File

SOURCE=..\..\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
