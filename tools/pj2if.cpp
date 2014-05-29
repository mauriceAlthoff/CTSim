/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          pj2if.cpp
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
 *   pj2if.c                    Convert Raysum to image
 *
 * DATE
 *   Apr 1999
 */

#include "ct.h"
#include "timer.h"


enum { O_VERBOSE, O_HELP, O_VERSION, O_POLAR };

static struct option my_options[] =
{
  {"polar", 0, 0, O_POLAR},
  {"verbose", 0, 0, O_VERBOSE},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

void
pj2if_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " in-proj-file out-if-file [OPTIONS]\n";
  std::cout << "Converts a projection file to a imagefile\n";
  std::cout << std::endl;
  std::cout << "   --polar     Convert to polar format\n";
  std::cout << "   --verbose   Verbose mode\n";
  std::cout << "   --version   Print version\n";
  std::cout << "   --help      Print this help message\n";
}

int
pj2if_main (const int argc, char *const argv[])
{
  char *pj_name, *im_name;
  bool optVerbose = false;
  extern int optind;
  Timer timerProgram;

  while (1)
    {
      int c = getopt_long (argc, argv, "", my_options, NULL);
      if (c == -1)
        break;

      switch (c)
        {
        case O_VERBOSE:
          optVerbose = true;
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
          pj2if_usage(argv[0]);
          return (0);
        default:
          pj2if_usage(argv[0]);
          return (1);
        }
    }

  if (argc - optind != 2) {
    pj2if_usage(argv[0]);
    return (1);
  }

  pj_name = argv[optind];
  im_name = argv[optind + 1];

  Projections pj;
  if (! pj.read (pj_name)) {
    sys_error (ERR_SEVERE, "Can not open projection file %s", pj_name);
    return (1);
  }

  if (optVerbose) {
    std::ostringstream os;
    pj.printScanInfo (os);
    std::cout << os.str();
  }

  ImageFile im (pj.nDet(), pj.nView());

  ImageFileArray v = im.getArray();

  for (int iy = 0; iy < pj.nView(); iy++) {
    const DetectorArray& detarray = pj.getDetectorArray (pj.nView() - iy - 1);
    const DetectorValue* detval = detarray.detValues();
    for (int ix = 0; ix < pj.nDet(); ix++) {
      v[ix][iy] = detval[ix];
    }
  }

  im.labelAdd (pj.getLabel());
  im.labelAdd (Array2dFileLabel::L_HISTORY, "Conversion from .pj to .if", timerProgram.timerEnd());
  im.fileWrite (im_name);

  return(0);
}


#ifndef NO_MAIN
int
main (const int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = pj2if_main(argc, argv);
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif
