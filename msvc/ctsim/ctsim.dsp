# Microsoft Developer Studio Project File - Name="ctsim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ctsim - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ctsim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ctsim.mak" CFG="ctsim - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ctsim - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ctsim - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ctsim - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GR /GX /O2 /I "..\..\getopt" /I "..\..\include" /I "\wx2\include" /I "\wx2\src\png" /I "\fftw-2.1.3\fftw" /I "\fftw-2.1.3\rfftw" /I "\wx2\src\zlib" /I "\ctn\include" /D "NDEBUG" /D VERSION=\"3.5.5\" /D "_WINDOWS" /D "HAVE_STRING_H" /D "HAVE_GETOPT_H" /D "MSVC" /D "HAVE_FFTW" /D "HAVE_PNG" /D "HAVE_SGP" /D "HAVE_WXWINDOWS" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D "WIN32" /D "HAVE_CTN_DICOM" /D CTSIMVERSION=\"3.5.5\" /YX /FD /D _WI"NDOWS,_MBCS,HAVE_STRING_H,HAVE_GETOPT_H,MSVC,HAVE_FFTW,HAVE_PNG,HAVE_SGP,HAVE_WXWINDOWS,__WXMSW__,__WIN95__,__WIN32__,VERSION=3.5.4,WINVER=0x0400,STRICT /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\wx2\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ctn_lib.lib winmm.lib wx.lib png.lib tiff.lib jpeg.lib xpm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ctn_lib.lib comctl32.lib rpcrt4.lib fftw2st.lib rfftw2st.lib opengl32.lib wsock32.lib htmlhelp.lib /nologo /subsystem:windows /machine:I386 /libpath:"\wx2\lib" /libpath:"\ctn\winctn\ctn_lib\Release" /libpath:"\fftw-2.1.3\Win32\FFTW2st\Release" /libpath:"\fftw-2.1.3\Win32\RFFTW2st\Release"

!ELSEIF  "$(CFG)" == "ctsim - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\..\getopt" /I "..\..\include" /I "\wx2\include" /I "\wx2\src\png" /I "\fftw-2.1.3\fftw" /I "\fftw-2.1.3\rfftw" /I "\wx2\src\zlib" /I "\ctn\include" /D "_DEBUG" /D "DEBUG" /D "__WXDEBUG__" /D WXDEBUG=1 /D "_MBCS" /D WINVER=0x0400 /D "STRICT" /D VERSION=3.5.5 /D "_WINDOWS" /D "HAVE_STRING_H" /D "HAVE_GETOPT_H" /D "MSVC" /D "HAVE_FFTW" /D "HAVE_PNG" /D "HAVE_SGP" /D "HAVE_WXWINDOWS" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D "WIN32" /D "HAVE_CTN_DICOM" /D CTSIMVERSION=\"3.5.5\" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\wx2\include" /i "\wx2\contrib\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib comctl32.lib winmm.lib rpcrt4.lib wxd.lib xpmd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib opengl32.lib fftw2st.lib rfftw2st.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ctn_lib.lib htmlhelp.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /nodefaultlib:"msvcrtd.lib" /pdbtype:sept /libpath:"\fftw-2.1.3\win32\fftw2st\Debug" /libpath:"\fftw-2.1.3\win32\rfftw2st\Debug" /libpath:"\wx2\lib" /libpath:"\ctn\winctn\ctn_lib\Debug"

!ENDIF 

# Begin Target

# Name "ctsim - Win32 Release"
# Name "ctsim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\backgroundmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\backgroundsupr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ctsim.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\dialogs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\dlgezplot.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\dlgprojections.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\dlgreconstruct.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\docs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\getopt\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\getopt\getopt1.c
# End Source File
# Begin Source File

SOURCE=..\..\src\graph3dview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\threadproj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\threadraster.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\threadrecon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\tips.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\views.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\backgroundmgr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\backgroundsupr.h
# End Source File
# Begin Source File

SOURCE="..\..\src\ctsim-map.h"
# End Source File
# Begin Source File

SOURCE=..\..\src\ctsim.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dialogs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dlgezplot.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dlgprojections.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dlgreconstruct.h
# End Source File
# Begin Source File

SOURCE=..\..\src\threadraster.h
# End Source File
# Begin Source File

SOURCE=..\..\src\threadrecon.h
# End Source File
# Begin Source File

SOURCE=..\..\src\tips.h
# End Source File
# Begin Source File

SOURCE=..\..\src\views.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\wx\msw\blank.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\bullseye.cur
# End Source File
# Begin Source File

SOURCE=.\ctsim.rc
# End Source File
# Begin Source File

SOURCE=.\wx\msw\error.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\hand.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\info.ico
# End Source File
# Begin Source File

SOURCE=.\logo.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\magnif1.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\noentry.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pbrush.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pencil.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntleft.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntright.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\query.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\question.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\roller.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\size.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\tip.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\warning.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\watch1.cur
# End Source File
# Begin Source File

SOURCE=.\wx\html\msw\wbook.ico
# End Source File
# Begin Source File

SOURCE=.\wx\html\msw\wfolder.ico
# End Source File
# Begin Source File

SOURCE=.\wx\html\msw\whelp.ico
# End Source File
# Begin Source File

SOURCE=.\wx\html\msw\whlproot.ico
# End Source File
# Begin Source File

SOURCE=.\wx\html\msw\wpage.ico
# End Source File
# End Group
# End Target
# End Project
