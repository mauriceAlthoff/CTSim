/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          linogram.cpp
**   Purpose:       Display linogram sampling
**   Programmer:    Kevin Rosenberg
**   Date Started:  April 2003
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

#include "ct.h"

enum { O_XY, O_POLAR_RT, O_VERBOSE, O_HELP, O_VERSION, O_DEBUG };

static struct option my_options[] =
{
  {"xy", 0, 0, O_XY},
  {"polar-rt", 0, 0, O_POLAR_RT},
  {"debug", 0, 0, O_DEBUG},
  {"verbose", 0, 0, O_VERBOSE},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";


void
linogram_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " n d [OPTIONS]\n";
  std::cout << "Imagefile information\n";
  std::cout << std::endl;
  std::cout << "     n            Linogram N\n";
  std::cout << "     d            Max detector spacing\n";
  std::cout << "     --xy         Output x,y pairs\n";
  std::cout << "     --polar-rt   Output r,t pairs\n";
  std::cout << "     --debug      Debug mode\n";
  std::cout << "     --verbose    Verbose mode\n";
  std::cout << "     --version    Print version\n";
  std::cout << "     --help       Print this help message\n";
}

int
linogram_main (int argc, char *const argv[])
{
  int opt_polar_rt = 0;
  int opt_xy = 0;
  int opt_verbose = 0;
  int opt_debug = 0;

  while (1)
    {
      int c = getopt_long (argc, argv, "", my_options, NULL);

      if (c == -1)
        break;

      switch (c)
        {
        case O_XY:
          opt_xy = 1;
          break;
        case O_POLAR_RT:
          opt_polar_rt = 1;
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
          linogram_usage(argv[0]);
          return (0);
        default:
          linogram_usage(argv[0]);
          return (1);
        }
    }

  if (optind + 2 != argc) {
    linogram_usage (argv[0]);
    return (1);
  }

  const char* in_n = argv[optind];
  const char* in_d = argv[optind+1];

  int n = atol (in_n);
  double d = atof (in_d);
  int size = 4 * n + 3;
  int max = 2 * n + 1;
  int min = -max;
  double theta_base = PI/4;
  //  theta_base = 0;

  double theta_vec [size];
  for (int i = 0; i < size; i++) {
    int m = i - (2 * n + 1);
    theta_vec[i] = atan (static_cast<double>(2 * m) / size);
  }

  if (opt_xy) {
    int m;
    for (m = 0; m < size; m++) {
      double step = d * cos(theta_vec[m]);
      for (int id = min; id <= max; id++) {
        double r = id * step;
        double x = r * cos(theta_vec[m] + theta_base);
        double y = r * sin(theta_vec[m] + theta_base);
        printf ("%lf,%lf ", x, y);
      }
      printf ("\n");
    }

    for (m = 0; m < size; m++) {
      double step = d * cos(theta_vec[m]);
      for (int id = min; id <= max; id++) {
        double r = id * step;
        double x = r * cos(theta_vec[m] + PI/2 + theta_base);
        double y = r * sin(theta_vec[m] + PI/2 + theta_base);
        printf ("%lf,%lf ", x, y);
      }
      printf ("\n");
    }
  } else {
    int m;
    for (m = 0; m < size; m++) {
      if (! opt_polar_rt)
        printf ("%lf: ", theta_vec[m] + theta_base);
      double step = d * cos(theta_vec[m]);
      for (int id = min; id <= max; id++) {
        if (opt_polar_rt)
          printf ("%lf,", theta_vec[m] + theta_base);
        printf ("%lf ", id * step);
      }
      printf ("\n");
    }

    for (m = 0; m < size; m++) {
      if (! opt_polar_rt)
        printf ("%lf: ", theta_vec[m] + PI/2 + theta_base);
      double step = d * cos(theta_vec[m]);
      for (int id = min; id <= max; id++) {
        if (opt_polar_rt)
          printf ("%lf,", theta_vec[m] + PI/2 + theta_base);
        printf ("%lf ", id * step);
      }
      printf ("\n");
    }
  }

  return (0);
}

#ifndef NO_MAIN
int
main (int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = linogram_main(argc, argv);
  } catch (exception e) {
          std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
          std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif
