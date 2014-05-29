/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          ifinfo.cpp
**   Purpose:       Display information about an image file
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
 *   ifinfo.c             Display info on sdf files
 */

#include "ct.h"

enum { O_LABELS, O_STATS, O_NO_STATS, O_NO_LABELS, O_VERBOSE, O_HELP, O_VERSION, O_DEBUG };

static struct option my_options[] =
{
  {"labels", 0, 0, O_LABELS},
  {"no-labels", 0, 0, O_NO_LABELS},
  {"stats", 0, 0, O_STATS},
  {"no-stats", 0, 0, O_NO_STATS},
  {"debug", 0, 0, O_DEBUG},
  {"verbose", 0, 0, O_VERBOSE},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";


void
ifinfo_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " image-filename [OPTIONS]\n";
  std::cout << "Imagefile information\n";
  std::cout << std::endl;
  std::cout << "     infile       Name of input IF file\n";
  std::cout << "     --display    Display image\n";
  std::cout << "     --labels     Print image labels (default)\n";
  std::cout << "     --no-labels  Do not print image labels\n";
  std::cout << "     --stats      Print image statistics (default)\n";
  std::cout << "     --no-stats   Do not print image statistics\n";
  std::cout << "     --debug      Debug mode\n";
  std::cout << "     --verbose    Verbose mode\n";
  std::cout << "     --version    Print version\n";
  std::cout << "     --help       Print this help message\n";
}

int
ifinfo_main (int argc, char *const argv[])
{
  ImageFile *im = NULL;
  std::string in_file;
  int opt_verbose = 0;
  int opt_stats = 1;
  int opt_labels = 1;
  int opt_debug = 0;

  while (1)
    {
      int c = getopt_long (argc, argv, "", my_options, NULL);

      if (c == -1)
        break;

      switch (c)
        {
        case O_LABELS:
          opt_labels = 1;
          break;
        case O_STATS:
          opt_stats = 1;
          break;
        case O_NO_LABELS:
          opt_labels = 0;
          break;
        case O_NO_STATS:
          opt_stats = 0;
          break;
        case O_VERBOSE:
          opt_verbose = 1;
          break;
        case O_DEBUG:
          opt_debug = 0;
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
          ifinfo_usage(argv[0]);
          return (0);
        default:
          ifinfo_usage(argv[0]);
          return (1);
        }
    }

  if (optind + 1 != argc) {
    ifinfo_usage (argv[0]);
    return (1);
  }

  in_file = argv[optind];

  im = new ImageFile ();
  if (! im->fileRead (in_file)) {
    sys_error (ERR_WARNING, "Unable to read file %s", in_file.c_str());
    return (1);
  }

  if (opt_labels)
    im->printLabels (std::cout);

  if (opt_stats) {
    std::cout << "Size: (" << im->nx() << "," << im->ny() << ")\n";
    std::cout << "Data type: ";
    if (im->dataType() == Array2dFile::DATA_TYPE_COMPLEX)
      std::cout << "Complex\n";
    else
      std::cout << "Real\n";

    im->printStatistics (std::cout);
  }

  return (0);
}

#ifndef NO_MAIN
int
main (int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = ifinfo_main(argc, argv);
  } catch (exception e) {
          std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
          std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif
