# Microsoft Developer Studio Project File - Name="libctsim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libctsim - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libctsim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libctsim.mak" CFG="libctsim - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libctsim - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libctsim - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libctsim - Win32 Release"

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
# ADD CPP /nologo /G6 /MT /W3 /GR /GX /O2 /I "..\..\include" /I "..\..\getopt" /I "\fftw-2.1.3\fftw" /I "\fftw-2.1.3\rfftw" /I "\wx2\include" /I "\wx2\src\png" /I "\wx2\src\zlib" /I "\wx2\src\tiff" /I "\wx2\src\xpm" /I "\ctn\include" /D "_LIB" /D "__WIN95__" /D "__WIN32__" /D "_WINDOWS" /D "__WXMSW__" /D "MSVC" /D "NDEBUG" /D VERSION=\"3.5.5\" /D "WIN32" /D "HAVE_CTN_DICOM" /D "HAVE_STRING_H" /D "HAVE_GETOPT_H" /D "HAVE_FFTW" /D "HAVE_PNG" /D "HAVE_SGP" /D "HAVE_WXWINDOWS" /D WINVER=0x0400 /D "STRICT" /YX /FD /D _WI"NDOWS,_MBCS,HAVE_STRING_H,HAVE_GETOPT_H,MSVC,HAVE_FFTW,HAVE_PNG,HAVE_SGP,HAVE_WXWINDOWS,__WXMSW__,__WIN95__,__WIN32__,VERSION=3.5.4,WINVER=0x0400,STRICT /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libctsim - Win32 Debug"

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
# ADD CPP /nologo /G6 /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\..\include" /I "..\..\getopt" /I "\fftw-2.1.3\fftw" /I "\fftw-2.1.3\rfftw" /I "\wx2\include" /I "\wx2\src\png" /I "\wx2\src\zlib" /I "\wx2\src\tiff" /I "\wx2\src\xpm" /I "\ctn\include" /D "_LIB" /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /D "__WXDEBUG__" /D WXDEBUG=1 /D "_MBCS" /D "HAVE_STRING_H" /D "HAVE_GETOPT_H" /D "MSVC" /D "HAVE_FFTW" /D "HAVE_PNG" /D "HAVE_SGP" /D "HAVE_WXWINDOWS" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D VERSION=\"3.5.5\" /D "WIN32" /D "HAVE_CTN_DICOM" /YX /FD /GZ /c
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

# Name "libctsim - Win32 Release"
# Name "libctsim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\libctsim\array2dfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\backprojectors.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\clip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\consoleio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\ctndicom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctgraphics\ezplot.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctgraphics\ezset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctgraphics\ezsupport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\filter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\fnetorderstream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\fourier.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\globalvars.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\hashtable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\imagefile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\interpolator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\mathfuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\msvc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\phantom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\plotfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctgraphics\pol.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\procsignal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\projections.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\reconstruct.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\scanner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctgraphics\sgp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\strfuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\syserror.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsim\trace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctgraphics\transformmatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libctsupport\xform.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\array2d.h
# End Source File
# Begin Source File

SOURCE=..\..\include\array2dfile.h
# End Source File
# Begin Source File

SOURCE=..\..\include\backprojectors.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ct.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ctglobals.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ctndicom.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ctsupport.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ezplot.h
# End Source File
# Begin Source File

SOURCE=..\..\include\filter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\fnetorderstream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\fourier.h
# End Source File
# Begin Source File

SOURCE=..\..\include\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\imagefile.h
# End Source File
# Begin Source File

SOURCE=..\..\include\interpolator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpiworld.h
# End Source File
# Begin Source File

SOURCE=..\..\include\msvc_compat.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nographics.h
# End Source File
# Begin Source File

SOURCE=..\..\include\phantom.h
# End Source File
# Begin Source File

SOURCE=..\..\include\plotfile.h
# End Source File
# Begin Source File

SOURCE=..\..\include\pol.h
# End Source File
# Begin Source File

SOURCE=..\..\include\procsignal.h
# End Source File
# Begin Source File

SOURCE=..\..\include\projections.h
# End Source File
# Begin Source File

SOURCE=..\..\include\reconstruct.h
# End Source File
# Begin Source File

SOURCE=..\..\include\scanner.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sgp.h
# End Source File
# Begin Source File

SOURCE=..\..\include\timer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\trace.h
# End Source File
# Begin Source File

SOURCE=..\..\include\transformmatrix.h
# End Source File
# End Group
# End Target
# End Project
