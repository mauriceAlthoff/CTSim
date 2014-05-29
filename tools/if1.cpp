/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          if1.cpp
**   Purpose:       Manipulate a single image file
**   Programmer:    Kevin Rosenberg
**   Date Started:  Aug 1984
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
 *   if1.c             Filter a single IF file
 */

#include "ct.h"

enum {O_LOG, O_EXP, O_SQRT, O_SQR, O_INVERT, O_VERBOSE, O_HELP, O_VERSION};

static struct option my_options[] =
{
  {"invert", 0, 0, O_INVERT},
  {"verbose", 0, 0, O_VERBOSE},
  {"log", 0, 0, O_LOG},
  {"exp", 0, 0, O_EXP},
  {"sqr", 0, 0, O_SQR},
  {"sqrt", 0, 0, O_SQRT},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

void
if1_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " infile outfile [OPTIONS]" << std::endl;
  std::cout << "Generate a IF file from a IF file" << std::endl;
  std::cout << std::endl;
  std::cout << "     --invert   Invert image" << std::endl;
  std::cout << "     --log      Natural logrithm of image" << std::endl;
  std::cout << "     --exp      Natural exponential of image" << std::endl;
  std::cout << "     --sqr      Square of image" << std::endl;
  std::cout << "     --sqrt     Square root of image" << std::endl;
  std::cout << "     --verbose  Verbose mode" << std::endl;
  std::cout << "     --version  Print version" << std::endl;
  std::cout << "     --help     Print this help message" << std::endl;
}

int
if1_main (int argc, char *const argv[])
{
  char *in_file;
  char *out_file;
  int opt_verbose = 0;
  int opt_invert = 0;
  int opt_log = 0;
  int opt_exp = 0;
  int opt_sqr = 0;
  int opt_sqrt = 0;

  while (1)
    {
      int c = getopt_long (argc, argv, "", my_options, NULL);

      if (c == -1)
        break;

      switch (c)
        {
        case O_INVERT:
          opt_invert = 1;
          break;
        case O_LOG:
          opt_log = 1;
          break;
        case O_SQR:
          opt_sqr = 1;
          break;
        case O_SQRT:
          opt_sqrt = 1;
          break;
        case O_EXP:
          opt_exp = 1;
          break;
        case O_VERBOSE:
          opt_verbose = 1;
          break;
        case O_VERSION:
#ifdef VERSION
          std::cout << "Version " << VERSION << std::endl << g_szIdStr << std::endl;
#else
          std::cout << "Unknown version number" << std::endl;
#endif
          return (0);
        case O_HELP:
        case '?':
          if1_usage(argv[0]);
          return (0);
        default:
          if1_usage(argv[0]);
          return (1);
        }
    }

  if (optind + 2 != argc)
    {
      if1_usage(argv[0]);
      return (1);
    }

  in_file = argv[optind];
  out_file = argv[optind + 1];

  std::string histString;

  if (opt_invert || opt_log || opt_exp || opt_sqr || opt_sqrt) {
    ImageFile im_in;
    im_in.fileRead (in_file);
    int nx = im_in.nx();
    int ny = im_in.ny();
    ImageFile im_out (nx, ny);

    if (opt_invert) {
      im_in.invertPixelValues (im_out);
      histString = "Invert transformation";
    }
    if (opt_log) {
      im_in.log (im_out);
      histString = "Logrithmic transformation";
    }
    if (opt_exp) {
      im_in.exp (im_out);
      histString = "Exponential transformation";
    }
    if (opt_sqr) {
      im_in.square (im_out);
      histString = "Square transformation";
    }
    if (opt_sqrt) {
      im_in.sqrt (im_out);
      histString = "Square root transformation";
    }

    im_out.labelsCopy (im_in);
    im_out.labelAdd (Array2dFileLabel::L_HISTORY, histString.c_str());
    im_out.fileWrite (out_file);
  }

  return (0);
}

#ifndef NO_MAIN
int
main (int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = if1_main(argc, argv);
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
  }

  return (retval);
}
#endif

