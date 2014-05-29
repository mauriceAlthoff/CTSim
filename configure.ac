dnl *****************************************************************************
dnl FILE IDENTIFICATION
dnl   Name:          ctsim.cpp
dnl   Purpose:       Top-level routines of CTSim program
dnl   Programmer:    Kevin Rosenberg
dnl   Date Started:  July 2000
dnl
dnl  This is part of the CTSim program
dnl  Copyright (c) 1983-2009 Kevin Rosenberg
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License (version 2) as
dnl  published by the Free Software Foundation.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
dnl****************************************************************************/

dnl Process this file with autoconf to produce a configure script.

dnl Must reset CDPATH so that bash's cd does not print to stdout
dnl CDPATH=

AC_INIT([CTSim], [5.2.0], [ctsim-users@b9.com], [ctsim], [http://www.ctsim.org/])
AC_CONFIG_SRCDIR([src/ctsim.cpp])
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_PROG_CC
AC_PROG_CXX
AC_C_BIGENDIAN

AC_PATH_PROG(wxconfig,wx-config)

dnl Check for C operation
AC_CANONICAL_HOST 
AC_C_INLINE
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 8)

dnl Add define templates

dnl Checks for libraries.
AC_CHECK_LIB(z, deflate, [ zlib="true" ], [ zlib="false" ;  AC_MSG_WARN([zlib missing. Will need zlib for PNG support])])
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(curses, main, [curses=true], [curses=false])
AC_CHECK_LIB(ncurses, main, [ncurses=true], [ncurses=false])
AC_CHECK_LIB(readline, main, [readline=true; 
                 AC_DEFINE([HAVE_READLINE],1,[Readline library])],
		 [readline=false], [-lcurses])	
wxwin=false
AC_CHECK_LIB(gtk-x11-2.0, main, [hasx11gtk2=true], [])
if test "x$hasx11gtk2" = "x" ; then
  AC_MSG_NOTICE([Does not have X11 GTK2])
  AC_DEFUN([AM_PATH_GLIB_2_0], [])
  AC_DEFUN([AM_PATH_GTK_2_0], [])
fi
if test "$hasx11gtk2" = "true" ; then
  AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR(You should get glib 2.0.0 or better.))
  AM_PATH_GTK_2_0(2.0.0,havegtk_am=yes,havegtk_am=no)
  CFLAGS="${CFLAGS} ${g76GTK_CFLAGS} ${GLIB_CFLAGS}"
fi

AC_CHECK_LIB(wx_gtk2u_core-2.8, main, [wxwin=true; wx_gtk=true; AC_DEFINE(HAVE_WXWINDOWS,1,[wxwindows library])], [], [-L/usr/lib64 -L/usr/lib ${GTK_LIBS} ${GLIB_LIBS} ])
AC_CHECK_LIB(wx_mac_core-2.8, main, [wxwin=true; wx_mac=true; AC_DEFINE(HAVE_WXWINDOWS,1,[wxwindows library])])
AC_CHECK_LIB(fftw3, fftw_malloc, [fftw=true; AC_DEFINE(HAVE_FFTW,1,[FFTW library])], [fftw=false], [-L/usr/lib64 -L/usr/lib])
AC_CHECK_LIB(GL, main, [libgl=true], [libgl=false], [-L/usr/X11R6/lib -L/usr/X11R6/lib64])
AC_CHECK_LIB(pthread, main, [pthread=true], [pthread=false])

if test "$zlib" = "true" ; then
  AC_CHECK_LIB(png, main, [png=true ; AC_DEFINE(HAVE_PNG,1,[PNG library])], [png=false], [-L/opt/local/lib -lz -lm])
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h getopt.h sys/fcntl.h setjmp.h stdarg.h stddef.h sys/types.h sys/stat.h string.h ctype.h math.h stdio.h netinet/in.h inttypes.h sys/param.h stdint.h stdlib.h assert.h sys/time.h sys/resource.h sys/time.h readline.h readline/readline.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strtod strtol snprintf htonl usleep vsprintf vsnprintf basename setjmp setpriority time gettimeofday getenv)
AC_CHECK_FUNC(basename)
AC_CHECK_FUNC(setjmp)
AC_CHECK_FUNC(setpriority)
AC_CHECK_FUNC(time)
AC_CHECK_FUNC(gettimeofday)
AC_CHECK_FUNC(getopt, [ getopt=true ], [ getopt=false ])

if test "${OSTYPE}" = "cygwin" ; then
  getopt_long=false
else
  AC_CHECK_FUNC(getopt_long, [ getopt_long=true ], [ getopt_long=false ])
fi

if test "${getopt_long}" = "false" ; then
  my_includes="$my_includes -I../getopt"
  AC_DEFINE(HAVE_GETOPT_LONG,1,[getopt_long library])
  ctlibs_base="$ctlibs_base -lgetopt"
  LDFLAGS="$LDFLAGS -L../getopt"
  GETOPTLONG=1
fi
AM_CONDITIONAL(INCLUDED_GETOPT_LONG, test "$GETOPTLONG"="1")

if test "${readline}" = "true" ; then
  ctlibs_base="$ctlibs_base -lreadline"
  if test "${ncurses}" = "true"; then
    ctlibs_base="$ctlibs_base -lncurses"
  elif test "${curses}" = "true"; then
    ctlibs_base="$ctlibs_base -lcurses"
  fi
fi

dnl Check for debug mode
AC_MSG_CHECKING([debug])
AC_ARG_ENABLE(debug,
[  --enable-debug          Turn on debugging],
[case "${enableval}" in
  yes) debug="true" 
       AC_MSG_RESULT(yes)
       ;;
  no)  debug="false" 
       AC_MSG_RESULT(no)
       ;;
  *) AC_MSG_RESULT([bad value ${enableval} for --enable-debug]) 
       ;;
esac],
[debug=false; AC_MSG_RESULT(no)])
AM_CONDITIONAL(DEBUG, test "$debug" = "true")

if test "$datadir" != "" ; then
  CPPFLAGS="$CPPFLAGS -DDATADIR=\"\\\"$datadir\\\"\""
fi
dnl CFLAGS="$CFLAGS -I/usr/local/include"
dnl LDFLAGS="$LDFLAGS -L/usr/local/lib"

if test "$debug" = "true" ; then
  CFLAGS="$CFLAGS -g -O0 -DDEBUG"
  AC_DEFINE(DEBUG,1,[turn on debugging])
else
  CFLAGS="$CFLAGS -O2 -DNDEBUG -fomit-frame-pointer"
  AC_DEFINE(NDEBUG,1,[no debugging])
fi

if test "${pthread}" = "true" ; then
dnl  CFLAGS="$CFLAGS -D_REENTRANT"
  AC_DEFINE(HAVE_WXTHREADS,1,[have wxthreads library])
fi

AC_MSG_CHECKING(sstream)
 if [ test -f /usr/include/sstream || test -f /usr/include/g++/sstream ||
     test -f /usr/include/g++-2/sstream || test -f /usr/include/g++-3/sstream ||
     test -f /usr/include/gcc/darwin/3.1/g++-v3/sstream ]; then
   AC_DEFINE(HAVE_SSTREAM,1,[sstream header])
   AC_MSG_RESULT(yes)
  else
   AC_MSG_RESULT(no)
 fi


CFLAGS="$CFLAGS -Wall"
AC_MSG_CHECKING(whether to enable verbose warnings)
AC_ARG_ENABLE(verbose-warnings,
[  --enable-verbose-warnings
                          Enable verbose compiler warnings.],
[ case "$enableval" in
  yes)
    AC_MSG_RESULT(yes)
    CFLAGS="$CFLAGS -W -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Waggregate-return -Wmissing-prototypes -Wstrict-prototypes -pedantic -Wredundant-decls"
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ], AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(static executables)
AC_ARG_ENABLE(static,
[  --enable-static
                            Enable static executables.],
[ case "$enableval" in
  yes)
    AC_MSG_RESULT(yes)
    CFLAGS="$CFLAGS --static"
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ], AC_MSG_RESULT(no)
)

dnl Set LAM path
AC_ARG_WITH(lam,
[  --with-lam[=PATH]         Set path of LAM MPI ],
[    if test "$withval" != "no" ; then
         trylamdir=$withval
     fi ]
)

if test "$withval" != "no" ; then
  if test "$prefix" != "NONE" ; then
     trylamdir="$trylamdir $prefix"
  fi

  AC_MSG_CHECKING([for LAM MPI installation])
  for testlamdir in "." $trylamdir /usr/local /usr/local/lam /usr /usr/lam /opt /opt/lam ; do
    if test -x "$testlamdir/bin/hcp" ; then
      if test "$testlamdir" != "/usr"; then 	
        LDFLAGS="$LDFLAGS -L$testlamdir/lib"
        CFLAGS="$CFLAGS -I$testlamdir/include"
      fi
      CFLAGS="$CFLAGS -I$testlamdir/include/mpi2c++"
      lamdir="$testlamdir"
      AC_SUBST(lamdir)
      mpienable="true"
      AC_SUBST(mpienable)
      break
    fi
  done
  AC_MSG_RESULT($lamdir)
else
  AC_MSG_RESULT([no])
fi


dnl Set CTN path
AC_ARG_WITH(ctn,
[  --with-ctn[=PATH]         Set path of CTN DICOM library ],
[    if test "$withval" != "no" ; then
         tryctndir=$withval
     fi ]
)

if test "$withval" = "no" -o "$withval" = "NONE" ; then
  AC_MSG_CHECKING([for CTN DICOM installation])
  AC_MSG_RESULT([disabled])
else
  AC_MSG_CHECKING([for CTN DICOM installation])
  for testctndir in $tryctndir/lib /usr/local/lib /usr/local/ctn/lib /usr/lib /usr/ctn/lib /usr/lib/ctn /opt/ctn/lib ; do
    if test -f "$testctndir/libctn.a" ; then
      LDFLAGS="$LDFLAGS -L$testctndir"
      ctnlib="true"
      break
    fi
  done
  for testctndir in $tryctndir/include /usr/local/include /usr/local/ctn/include /usr/include /usr/include/ctn /usr/ctn/include /opt/ctn/include ; do
    if test -f "$testctndir/ctn_os.h" ; then
      CFLAGS="$CFLAGS -I$testctndir"
      ctninclude="true"
      break
    fi
  done
  if test "$ctnlib" = "true" -a "$ctninclude" = "true" ; then
      ctn="true"
      AC_DEFINE(HAVE_CTN_DICOM,1,[ctn dicom library])
      AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
fi

dnl Set CPU
AC_MSG_CHECKING([for CPU specification])
AC_ARG_WITH(mcpu,
[  --with-mcpu[=CPU]         Set name of CPU for gcc -mcpu])

if test "$withval" = "no" -o "$withval" = "yes" -o "$withval" = ""; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([$withval])
  CFLAGS="$CFLAGS -mcpu=$withval"
fi

AC_MSG_CHECKING([for web access])
dnl Set cgi-bin directory
AC_ARG_WITH(cgibin-dir, [  --with-cgibin-dir=PATH  Set path of CGI binaries directory ], 
[ cgibindir=$withval ; AC_SUBST(cgibindir) ] )

AC_ARG_WITH(cgibin-url, [  --with-cgibin-url=PATH  Set URL path of CGI binaries ], 
[ cgibinurl=$withval ; AC_SUBST(cgibinurl) ] )

AC_ARG_WITH(webdata-dir, [  --with-webdata-dir=PATH Set path of webdata], 
[ webdatadir=$withval ; AC_SUBST(webdatadir)  ] )

AC_ARG_WITH(webdata-url, [  --with-webdata-url=PATH Set URL path of webdata], 
[ webdataurl=$withval ; AC_SUBST(webdataurl) ] )

AC_ARG_WITH(html-dir, [  --with-html-dir=PATH    Set directory of html files], 
[ html_%dir=$withval ; AC_SUBST(htmldir) ] )

if test "x$cgibindir" != "x" -o "x$cgibinurl" != "x" ; then
  cgiprograms="ctsim.cgi ctsim.conf"
  AC_SUBST(cgiprograms)
fi

if test "x$html_dir" = "x" ; then
  htmldata=simulate.html
  AC_SUBST(htmldata)
fi
AM_CONDITIONAL(USE_HTML, test "x$html_dir" = "x")

if test "x$cgibindir" != "x" -o "x$cgibinurl" != "x" -o "x$webdatadir" != "x" -o "x$webdataurl" != "x" -o "x$html_dir" != "x" ; then
  webenabled=true
  AC_SUBST(webenabled)
  AC_MSG_RESULT([yes])
  if test "x$cgibindir" = "x" ; then
    AC_MSG_ERROR([--with-cgibin-dir not set for web access])
  elif test "x$cgibinurl" = "x" ; then
    AC_MSG_ERROR([--with-cgibin-url not set for web access])
  elif test "x$webdatadir" = "x" ; then
    AC_MSG_ERROR([--with-webdata-dir not set for web access])
  elif test "x$webdataurl" = "x" ; then
    AC_MSG_ERROR([--with-webdata-url not set for web access])
  elif test "x$html_dir" = "x" ; then
    AC_MSG_ERROR([--with-html-dir not set for web access])
  fi
else
  AC_MSG_RESULT([no])
fi

AM_CONDITIONAL(USE_LAM, test "x$lamdir" != "x")

AC_PATH_XTRA
AC_SUBST(X_CFLAGS)
dnl Following five are concatenated together into a linker specification.
AC_SUBST(X_LIBS)
AC_SUBST(X_TOOLKIT_LIBS)
AC_SUBST(X_PRE_LIBS)
AC_SUBST(X_BASIC_LIBS)
AC_SUBST(X_EXTRA_LIBS)

dnl Our Makefile.am files test the automake variable NO_X to determine
dnl whether X should be supported.
AM_CONDITIONAL(NO_X, test "x$with_x" = "xno")

my_includes="$my_includes -I../include -I.. -I/usr/local/include"
AC_SUBST(my_includes)

LDFLAGS="$LDFLAGS -L/usr/local/lib"

AC_MSG_CHECKING([for X])
if test "$with_x" != "no" && test "$wxwin" = "true"; then
  AC_DEFINE(HAVE_X11,1,[X11 system])
  LDFLAGS="$LDFLAGS -L../libctgraphics -L/usr/X11R6/lib -L/usr/X11R6/lib64"
  my_includes="$my_includes -I/usr/X11R6/include"
  ctlib_graphics="$ctlibs_base -lctgraphics"
  AC_MSG_RESULT(yes)
  ctlib_graphics="$ctlib_graphics $X_BASIC_LIBS $X_TOOLKIT_LIBS"
  if test "$libgl" = "true" ; then
    # Removed "-lglut" for Mac OS X compilation
    ctlib_graphics="$ctlib_graphics -lGL -lGLU"
    if test "$wxwin" = "true" ; then
       wxdebug=""
       if test "$debug" = "true"; then
         wxdebug="--debug"
       fi  
      if test "x$wx_gtk" != "x" ; then
       ctlib_graphics="$ctlib_graphics `$wxconfig $wxdebug --version=2.8  --libs std,gl` ${GTK_LIBS} ${GLIB_LIBS}"
      elif test "x$wx_mac" != "x" ; then
        ctlib_graphics="$ctlib_graphics `$wxconfig $wxdebug --version=2.8 --libs std,gl`"
      fi
    fi
  fi
  if test "$wxwin" = "true" ; then
    AC_DEFINE(HAVE_SGP,1,[use sgp library])
  fi
else
  AC_MSG_RESULT(no)
fi

AM_CONDITIONAL(HAVE_SGP, test "$wxwin" = "true")

if test "$png" = "true" ; then
  ctlibs_tools="$ctlibs_tools -lpng"
fi
if test "$zlib" = "true" ; then
  ctlibs_tools="$ctlibs_tools -lz"
fi
if test "$fftw" = "true" ; then
  ctlibs_tools="$ctlibs_tools -lfftw3"
fi
if test "$ctn" = "true"; then
  ctlibs_tools="$ctlibs_tools -lctn"
fi

dnl Check for dmalloc
AC_CHECK_LIB(dmallocxx, main, [dmallocxx=true], [dmallocxx=false])
AC_MSG_CHECKING([for enable-dmalloc])
AC_ARG_ENABLE(dmalloc,
[  --enable-dmalloc        Use dmalloc memory allocation],
[case "${enableval}" in
  yes) usedmalloc=true 
       AC_MSG_RESULT(yes)
       ;;
  no)  usedmalloc=false 
       AC_MSG_RESULT(no)
       ;;
  *) AC_MSG_RESULT([bad value ${enableval} for --enable-dmalloc]) 
       ;;
esac],
[usedmalloc=false; AC_MSG_RESULT(no)])

if test "$dmallocxx" = "true" -a "$usedmalloc" = "true" ; then
  ctlibs_tools="$ctlibs_tools -ldmallocxx"
  AC_DEFINE(HAVE_DMALLOC,1,[dmalloc library])
  AC_MSG_RESULT(Using dmalloc)
fi

dnl Check for efence
AC_CHECK_LIB(efence, main, [efence=true], [efence=false])
AC_MSG_CHECKING([for enable-efence])
AC_ARG_ENABLE(efence,
[  --enable-efence        Use ElectricFence memory allocation],
[case "${enableval}" in
  yes) useefence=true 
       AC_MSG_RESULT(yes)
       ;;
  no)  useefence=false 
       AC_MSG_RESULT(no)
       ;;
  *) AC_MSG_RESULT([bad value ${enableval} for --enable-efence]) 
       ;;
esac],
[useefence=false; AC_MSG_RESULT(no)])

if test "$efence" = "true" -a "$useefence" = "true" ; then
  ctlibs_tools="$ctlibs_tools -lefence"
  AC_DEFINE(HAVE_EFENCE,1,[efence library])
  AC_MSG_RESULT(Using efence)
fi

if test "$wxwin" = "true" ; then
  if test "$wx_gtk" = "true"  -o "$wx_mac" = "true" ; then
    wxcflags=`$wxconfig $wxdebug --cxxflags --version=2.8`
    #wxlibs=`$wxconfig --libs`
  else
	wxcflags="-D__WXMSW__ -D__WIN32__ -D__GNUWIN32__"
        wxlibs="-lwx -lglui -ljpeg -lxpm -lzlib -ltiff"
  fi
  CFLAGS="$CFLAGS -I../include $wxcflags"
  ctlib_graphics="$ctlib_graphics $wxlibs"
fi
AC_SUBST(wxcflags)
AC_SUBST(wxlibs)
AM_CONDITIONAL(HAVE_WXWINDOWS, test "$wxwin"="true")

dnl Setting projet libraries and includes
LDFLAGS="$LDFLAGS -L../libctsupport -L../libctsim" 
ctlibs="$ctlibs_base -lctsim $ctlib_graphics -lctsupport $ctlibs_tools"
AC_SUBST(ctlibs)

if test "x$lamdir" != "x" ; then
  lamprograms="ctsimtext-lam"
  AC_SUBST(lamprograms)
  lamdefs="$CFLAGS"
  AC_SUBST(lamdefs)
fi

CXXFLAGS="$CFLAGS"

AC_CONFIG_FILES([Makefile libctgraphics/Makefile libctsupport/Makefile libctsim/Makefile man/Makefile cgi-bin/ctsim.cgi cgi-bin/Makefile html/simulate.html html/Makefile include/Makefile getopt/Makefile tools/sample-ctsim.sh cgi-bin/ctsim.conf tools/Makefile src/Makefile helical/Makefile helical/sample-helical.sh])

AC_OUTPUT
