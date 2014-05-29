/*****************************************************************************
 ** FILE IDENTIFICATION
 **
 **   Name:          pjHinterp.cpp
 **   Purpose:       Interpolate helical data in projection space
 **   Programmer:    Ian Kay and Kevin Rosenberg
 **   Date Started:  Aug 2001
 **
 **  This is part of the CTSim program
 **  Copyright (C) 1983-2009 Kevin Rosenberg
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
 **  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 ******************************************************************************/

#include "ct.h"
#include "timer.h"


enum { O_INTERPVIEW, O_VERBOSE, O_TRACE, O_HELP, O_DEBUG, O_VERSION};

static struct option my_options[] =
{
        {"interpview", 1, 0, O_INTERPVIEW},
        {"trace", 1, 0, O_TRACE},
        {"debug", 0, 0, O_DEBUG},
        {"verbose", 0, 0, O_VERBOSE},
        {"help", 0, 0, O_HELP},
        {"version", 0, 0, O_VERSION},
        {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

void pjHinterp_usage ( const char *program )
{
  std::cout << "usage: " << fileBasename(program) << " projection-file interp-projection-file [OPTIONS]" << std::endl;
  std::cout << "Interpolation of helical data in raw data space" << std::endl;
  std::cout << "  projection-file Input projection file" << std::endl;
  std::cout << "  interp-file     Output interpolated projection file " << std::endl;
  std::cout << "  --trace         Set tracing to level" << std::endl;
  std::cout << "     none         No tracing (default)" << std::endl;
  std::cout << "     console      Text level tracing" << std::endl;
  std::cout << "  --verbose       Turn on verbose mode" << std::endl;
  std::cout << "  --debug         Turn on debug mode" << std::endl;
  std::cout << "  --version       Print version" << std::endl;
  std::cout << "  --help          Print this help message" << std::endl;
}


int
pjHinterp_main(int argc, char * const argv[])
{
  Projections projGlobal;
  char* pszProjFilename = NULL;
  char* pszInterpFilename = NULL;
  bool bOptVerbose = false;
  bool bOptDebug = 1;
  int optTrace = Trace::TRACE_NONE;
  char *endptr = NULL;
  char *endstr;
  int opt_interpview=-1;

  while (1) {
    int c = getopt_long(argc, argv, "", my_options, NULL);

    if (c == -1)
      break;

    switch (c) {
    case O_INTERPVIEW:
      opt_interpview = strtol(optarg, &endptr, 10);
      endstr = optarg + strlen(optarg);
      if (endptr != endstr) {
        std::cerr << "Error setting --interpview to %s" << optarg << std::endl;
        pjHinterp_usage(argv[0]);
        return(1);
      }
      break;
    case O_VERBOSE:
      bOptVerbose = true;
      break;
    case O_DEBUG:
      bOptDebug = true;
      break;
    case O_TRACE:
      if ((optTrace = Trace::convertTraceNameToID(optarg))
          == Trace::TRACE_INVALID) {
        pjHinterp_usage(argv[0]);
        return (1);
      }
      break;
    case O_VERSION:
#ifdef VERSION
      std::cout <<  "Version " <<  VERSION << std::endl <<
        g_szIdStr << std::endl;
#else
      std::cout << "Unknown version number" << std::endl;
#endif
      return (0);

    case O_HELP:
    case '?':
      pjHinterp_usage(argv[0]);
      return (0);
    default:
      pjHinterp_usage(argv[0]);
      return (1);
    } // end switch
  } // end while

  if (optind + 2 != argc) {
    pjHinterp_usage(argv[0]);
    return (1);
  }

  pszProjFilename = argv[optind];

  pszInterpFilename = argv[optind + 1];

  Projections projections;
  if ( projections.read(pszProjFilename) != true ){
    std::cerr << "Error reading input file " << pszProjFilename << std::endl;
    return (1);
  }
  if (bOptVerbose) {
          std::ostringstream os;
    projections.printScanInfo(os);
    std::cout << os.str();
  }

  int status = projections.Helical180LI(opt_interpview);
  if ( status != 0 )  return (1);
  status = projections.HalfScanFeather();
  if ( status != 0 )  return (1);
  projections.write( pszInterpFilename  );

  return (0);
}


#ifndef NO_MAIN
int
main (int argc, char* argv[])
{
  int retval = 1;

  try {
    retval = pjHinterp_main(argc, argv);
  } catch (exception e) {
      std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
      std::cerr << "Unknown exception" << std::endl;
  }

  return (retval);
}
#endif
