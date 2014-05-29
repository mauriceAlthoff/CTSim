/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          pjinfo.cpp
**   Purpose:       Convert an projection data file to an image file
**   Programmer:    Kevin Rosenberg
**   Date Started:  April 2000
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
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/

/* FILE
*   pjinfo.c                    Convert Raysum to image
*
* DATE
*   August 2000
*/

#include "ct.h"
#include "timer.h"


enum { O_BINARYHEADER, O_BINARYVIEWS, O_STARTVIEW, O_ENDVIEW, O_DUMP, O_HELP, O_VERSION };

static struct option my_options[] =
{
  {"binaryheader", 0, 0, O_BINARYHEADER},
  {"binaryviews", 0, 0, O_BINARYVIEWS},
  {"startview", 1, 0, O_STARTVIEW},
  {"endview", 1, 0, O_ENDVIEW},
  {"dump", 0, 0, O_DUMP},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

void
pjinfo_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " proj-file [OPTIONS]\n";
  std::cout << "Display projection file information\n";
  std::cout << "\n";
  std::cout << "   --binaryheader  Dump binary header data\n";
  std::cout << "   --binaryviews   Dump binary view data\n";
  std::cout << "   --startview n   Beginning view number to display (default=0)\n";
  std::cout << "   --endview n     Ending view number to display (default=last view)\n";
  std::cout << "   --dump          Print all scan data ASCII format\n";
  std::cout << "   --version       Print version\n";
  std::cout << "   --help          Print this help message\n";
}



int
pjinfo_main (const int argc, char *const argv[])
{
  std::string pj_name;
  bool optDump = false;
  bool optBinaryHeader = false;
  bool optBinaryViews = false;
  int optStartView = 0;
  int optEndView = -1;  // tells copyViewData to use default last view
  extern int optind;

  while (1)
  {
    char *endptr, *endstr;
    int c = getopt_long (argc, argv, "", my_options, NULL);
    if (c == -1)
      break;

    switch (c)
    {
    case O_DUMP:
      optDump = true;
      break;
    case O_BINARYHEADER:
      optBinaryHeader = true;
      break;
    case O_BINARYVIEWS:
      optBinaryViews = true;
      break;
    case O_STARTVIEW:
      optStartView = strtol(optarg, &endptr, 10);
      endstr = optarg + strlen(optarg);
      if (endptr != endstr) {
        std::cerr << "Error setting --startview to %s" << optarg << std::endl;
        pjinfo_usage(argv[0]);
        return (1);
      }
      break;
    case O_ENDVIEW:
      optEndView = strtol(optarg, &endptr, 10);
      endstr = optarg + strlen(optarg);
      if (endptr != endstr) {
        std::cerr << "Error setting --endview to %s" << optarg << std::endl;
        pjinfo_usage(argv[0]);
        return (1);
      }
      break;
    case O_VERSION:
#ifdef VERSION
      std::cout << "Version " << VERSION << std::endl << g_szIdStr << std::endl;
#else
      std::cout << "Unknown version number\n";
#endif
      return (0);
    case O_HELP:
    case '?':
      pjinfo_usage(argv[0]);
      return (0);
    default:
      pjinfo_usage(argv[0]);
      return (1);
    }
  }

  if (argc - optind != 1) {
    pjinfo_usage(argv[0]);
    return (1);
  }

  pj_name = argv[optind];

  if (optBinaryHeader)
    Projections::copyHeader (pj_name, std::cout);
  else if (optBinaryViews)
    Projections::copyViewData (pj_name, std::cout, optStartView, optEndView);
  else {
    Projections pj;
    if (! pj.read (pj_name)) {
      sys_error (ERR_SEVERE, "Can not open projection file %s", pj_name.c_str());
      return (1);
    }

    if (optDump) {
      pj.printProjectionData (optStartView, optEndView);
    } else {
      std::ostringstream os;
      pj.printScanInfo (os);
      std::cout << os.str();
    }
  }

  return(0);
}


#ifndef NO_MAIN
int
main (const int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = pjinfo_main(argc, argv);
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif
