/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          ct.h
**   Purpose:       Master header file for CTSim
**   Programmer:    Kevin Rosenberg
**   Date Started:  Aug 1984
**
**  This is part of the CTSim program
**  Copyright (c) 1983-2009 Kevin Rosenberg
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License (version 2) as
**  published by the Free Software Foundation.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/

#ifndef CT_H
#define CT_H

#define NO_MAIN 1    // filter out all old main blocks

#ifdef MSVC
#include "msvc_compat.h"
#endif

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#ifdef _DEBUG
#undef DEBUG
#define DEBUG 1
#endif

#define HAVE_ANSI_CPP 1
#ifdef HAVE_ANSI_CPP
#include <complex>
#include <cmath>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstddef>
#include <cstdarg>
#include <cstdlib>

#if defined(MSVC) || HAVE_SSTREAM
#include <sstream>
#else
#include <sstream_subst>
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <memory>

#else

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#endif


#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PNG
  #include "png.h"
#endif
#ifdef HAVE_G2_H
extern "C" {
#include "g2.h"
}
#ifdef HAVE_X11
extern "C" {
#include "g2_X11.h"
}
#endif
#endif


#ifdef  HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if defined(HAVE_GETOPT_H) || defined(HAVE_GETOPT_LONG)
#include <getopt.h>
#endif
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>          /* for htonl on FreeBSD */
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>         /* for htonl on Linux/Solaris */
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>           /* for htonl on Solaris */
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>             /* Standard ints on Linux */
#endif
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif


#ifdef HAVE_FFTW
#include <fftw3.h>
#define HAVE_FFT 1
#endif

#ifdef HAVE_MPI
#include "mpi++.h"
#include "mpiworld.h"
#endif

#include "ctsupport.h"
#include "fnetorderstream.h"

#ifdef HAVE_SGP
  #include "ezplot.h"
  #include "sgp.h"
#endif

#include "array2d.h"
#include "array2dfile.h"
#include "fnetorderstream.h"
#include "imagefile.h"
#include "phantom.h"
#include "scanner.h"
#include "backprojectors.h"
#include "filter.h"
#include "fourier.h"
#include "procsignal.h"
#include "projections.h"
#include "reconstruct.h"
#include "plotfile.h"
#include "trace.h"

#include "ctglobals.h"

#endif

