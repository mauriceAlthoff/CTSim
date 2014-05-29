# Microsoft Developer Studio Project File - Name="ctsimtext" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ctsimtext - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ctsimtext.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ctsimtext.mak" CFG="ctsimtext - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ctsimtext - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ctsimtext - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ctsimtext - Win32 Release"

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
# ADD CPP /nologo /G6 /MT /W3 /GR /GX /O2 /I "..\..\include" /I "..\..\getopt" /I "\fftw-2.1.3\fftw" /I "\fftw-2.1.3\rfftw" /I "\wx2\include" /I "\wx2\src\png" /I "\wx2\src\zlib" /I "\wx2\src\tiff" /I "\wx2\src\xpm" /D "_CONSOLE" /D "NOMAIN" /D "__WIN95__" /D "__WIN32__" /D "_WINDOWS" /D "__WXMSW__" /D "MSVC" /D "NDEBUG" /D VERSION=\"3.5.5\" /D "WIN32" /D "HAVE_CTN_DICOM" /YX /FD /D _WI"NDOWS,_MBCS,HAVE_STRING_H,HAVE_GETOPT_H,MSVC,HAVE_FFTW,HAVE_PNG,HAVE_SGP,HAVE_WXWINDOWS,__WXMSW__,__WIN95__,__WIN32__,VERSION=3.5.4,WINVER=0x0400,STRICT /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wx.lib zlib.lib tiff.lib jpeg.lib png.lib libctsim.lib fftw2st.lib rfftw2st.lib wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ctn_lib.lib /nologo /subsystem:console /machine:I386 /libpath:"..\libctsim\Release" /libpath:"\fftw-2.1.3\Win32\RFFTW2st\Release" /libpath:"\fftw-2.1.3\Win32\FFTW2st\Release" /libpath:"\wx2\\" /libpath:"\wx2\lib" /libpath:"\ctn\winctn\ctn_lib\Release"

!ELSEIF  "$(CFG)" == "ctsimtext - Win32 Debug"

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
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\..\include" /I "..\..\getopt" /I "\fftw-2.1.3\fftw" /I "\fftw-2.1.3\rfftw" /I "\wx2\include" /I "\wx2\src\png" /I "\wx2\src\zlib" /I "\wx2\src\tiff" /I "\wx2\src\xpm" /D "_CONSOLE" /D "NOMAIN" /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /D "__WXDEBUG__" /D WXDEBUG=1 /D "_MBCS" /D "HAVE_STRING_H" /D "HAVE_GETOPT_H" /D "MSVC" /D "HAVE_FFTW" /D "HAVE_PNG" /D "HAVE_SGP" /D "HAVE_WXWINDOWS" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D VERSION=3.5.5 /D "WIN32" /D "HAVE_CTN_DICOM" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\wx2\include" /i "\wx2\contrib\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxd.lib libctsim.lib zlibd.lib tiffd.lib jpegd.lib pngd.lib fftw2st.lib rfftw2st.lib wsock32.lib comctl32.lib winmm.lib rpcrt4.lib xpmd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ctn_lib.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\libctsim\Debug" /libpath:"\fftw-2.1.3\Win32\RFFTW2st\Debug" /libpath:"\fftw-2.1.3\Win32\FFTW2st\Debug" /libpath:"\fftw-2.1.3\fftw2st\Debug" /libpath:"\fftw-2.1.3\rfftw2st\Release" /libpath:"\wx2\lib" /libpath:"\ctn\winctn\ctn_lib\Debug"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "ctsimtext - Win32 Release"
# Name "ctsimtext - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\tools\ctsimtext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\getopt\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\getopt\getopt1.c
# End Source File
# Begin Source File

SOURCE=..\..\tools\if1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\if2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\ifexport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\ifinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\nographics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\phm2helix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\phm2if.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\phm2pj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\pj2if.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\pjHinterp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\pjinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\tools\pjrec.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\getopt\getopt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ctsimtext.rc
# End Source File
# End Group
# End Target
# End Project
