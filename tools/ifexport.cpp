/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          ifexport.cpp
**   Purpose:       Convert an image file to a viewable image
**   Programmer:    Kevin Rosenberg
**   Date Started:  April 2000
**
**  This is part of the CTSim program
**  Copyright (C) 1983-2010 Kevin Rosenberg
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


enum { O_SCALE, O_MIN, O_MAX, O_AUTO, O_CENTER, O_STATS, O_FORMAT, O_LABELS,
       O_HELP, O_VERBOSE, O_VERSION, O_DEBUG };

static struct option my_options[] =
{
  {"scale", 1, 0, O_SCALE},
  {"min", 1, 0, O_MIN},
  {"max", 1, 0, O_MAX},
  {"auto", 1, 0, O_AUTO},
  {"center", 1, 0, O_CENTER},
  {"format", 1, 0, O_FORMAT},
  {"labels", 0, 0, O_LABELS},
  {"stats", 0, 0, O_STATS},
  {"verbose", 0, 0, O_VERBOSE},
  {"debug", 0, 0, O_DEBUG},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

enum { O_AUTO_FULL, O_AUTO_STD0_1, O_AUTO_STD0_5, O_AUTO_STD1, O_AUTO_STD2, O_AUTO_STD3 };
static const char O_AUTO_FULL_STR[]="full";
static const char O_AUTO_STD0_1_STR[]="std0.1";
static const char O_AUTO_STD0_5_STR[]="std0.5";
static const char O_AUTO_STD1_STR[]="std1";
static const char O_AUTO_STD2_STR[]="std2";
static const char O_AUTO_STD3_STR[]="std3";

enum { O_CENTER_MEDIAN, O_CENTER_MEAN, O_CENTER_MODE };
static const char O_CENTER_MEAN_STR[]="mean";
static const char O_CENTER_MODE_STR[]="mode";
static const char O_CENTER_MEDIAN_STR[]="median";

enum { O_FORMAT_PNG, O_FORMAT_PNG16, O_FORMAT_PGM, O_FORMAT_PGMASC, O_FORMAT_RAW, O_FORMAT_TEXT };
static const char O_FORMAT_PNG_STR[]="png" ;
static const char O_FORMAT_PNG16_STR[]="png16" ;
static const char O_FORMAT_PGM_STR[]="pgm";
static const char O_FORMAT_PGMASC_STR[]="pgmasc";
static const char O_FORMAT_TEXT_STR[]="text";
static const char O_FORMAT_RAW_STR[]="raw";

void
ifexport_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " ifname outfile [OPTIONS]\n";
  std::cout << "Convert IF file to an image file\n";
  std::cout << std::endl;
  std::cout << "     ifname     Name of input file\n";
  std::cout << "     outfile    Name of output file\n";
  std::cout << "     --format   Output format\n";
  std::cout << "         pgm    PGM (portable graymap) format (default)\n";
  std::cout << "         pgmasc PGM (portable graymap) ASCII format\n";
  std::cout << "         text   Text format\n";
  std::cout << "         raw    Raw format\n";
#ifdef HAVE_PNG
  std::cout << "         png    PNG (8-bit) format\n";
  std::cout << "         png16  PNG (16-bit) format\n";
#endif
  std::cout << "     --center   Center of window\n";
  std::cout << "         median Median is center of window (default)\n";
  std::cout << "         mode   Mode is center of window\n";
  std::cout << "         mean   Mean is center of window\n";
  std::cout << "     --auto     Set auto window\n";
  std::cout << "         full   Use full window (default)\n";
  std::cout << "         std0.1 Use 0.1 standard deviation about center\n";
  std::cout << "         std0.5 Use 0.5 standard deviation about center\n";
  std::cout << "         std1   Use one standard deviation about center\n";
  std::cout << "         std2   Use two standard deviations about center\n";
  std::cout << "         std3   Use three standard deviations about center\n";
  std::cout << "     --scale    Scaling factor for output size\n";
  std::cout << "     --min      Set minimum intensity\n";
  std::cout << "     --max      Set maximum intensity\n";
  std::cout << "     --stats    Print image statistics\n";
  std::cout << "     --labels   Print image labels\n";
  std::cout << "     --debug    Set debug mode\n";
  std::cout << "     --verbose  Set verbose mode\n";
  std::cout << "     --version  Print version\n";
  std::cout << "     --help     Print this help message\n";
}


int
ifexport_main (int argc, char *const argv[])
{
  ImageFile* pim = NULL;
  double densmin = HUGE_VAL, densmax = -HUGE_VAL;
  char *in_file, *out_file;
  int opt_verbose = 0;
  int opt_scale = 1;
  int opt_auto = O_AUTO_FULL;
  int opt_set_max = 0;
  int opt_set_min = 0;
  int opt_stats = 0;
  int opt_debug = 0;
  int opt_center = O_CENTER_MEDIAN;
  int opt_format = O_FORMAT_PGM;
  int opt_labels = 0;

  while (1)
    {
      int c = getopt_long (argc, argv, "", my_options, NULL);
      char *endptr = NULL;
      char *endstr;

      if (c == -1)
        break;

      switch (c)
        {
        case O_MIN:
          opt_set_min = 1;
          densmin = strtod(optarg, &endptr);
          endstr = optarg + strlen(optarg);
          if (endptr != endstr)
            {
              sys_error (ERR_SEVERE, "Error setting --min to %s", optarg);
              ifexport_usage(argv[0]);
              return (1);
            }
          break;
        case O_MAX:
          opt_set_max = 1;
          densmax = strtod(optarg, &endptr);
          endstr = optarg + strlen(optarg);
          if (endptr != endstr)
            {
              sys_error (ERR_SEVERE, "Error setting --max to %s", optarg);
              ifexport_usage(argv[0]);
              return (1);
            }
          break;
        case O_SCALE:
          opt_scale = strtol(optarg, &endptr, 10);
          endstr = optarg + strlen(optarg);
          if (endptr != endstr)
            {
              sys_error (ERR_SEVERE, "Error setting --scale to %s", optarg);
              ifexport_usage(argv[0]);
              return (1);
            }
          break;
        case O_AUTO:
          if (strcmp(optarg, O_AUTO_FULL_STR) == 0)
            opt_auto = O_AUTO_FULL;
          else if (strcmp(optarg, O_AUTO_STD1_STR) == 0)
            opt_auto = O_AUTO_STD1;
          else if (strcmp(optarg, O_AUTO_STD0_5_STR) == 0)
            opt_auto = O_AUTO_STD0_5;
          else if (strcmp(optarg, O_AUTO_STD0_1_STR) == 0)
            opt_auto = O_AUTO_STD0_1;
          else if (strcmp(optarg, O_AUTO_STD2_STR) == 0)
            opt_auto = O_AUTO_STD2;
          else if (strcmp(optarg, O_AUTO_STD3_STR) == 0)
            opt_auto = O_AUTO_STD3;
          else
            {
              sys_error (ERR_SEVERE, "Invalid auto mode %s", optarg);
              ifexport_usage(argv[0]);
              return (1);
            }
                break;
        case O_CENTER:
          if (strcmp(optarg, O_CENTER_MEDIAN_STR) == 0)
            opt_center = O_CENTER_MEDIAN;
          else if (strcmp(optarg, O_CENTER_MEAN_STR) == 0)
            opt_center = O_CENTER_MEAN;
          else if (strcmp(optarg, O_CENTER_MODE_STR) == 0)
            opt_center = O_CENTER_MODE;
          else
            {
              sys_error (ERR_SEVERE, "Invalid center mode %s", optarg);
              ifexport_usage(argv[0]);
              return (1);
            }
          break;
        case O_FORMAT:
          if (strcmp(optarg, O_FORMAT_PGM_STR) == 0)
            opt_format = O_FORMAT_PGM;
          else if (strcmp(optarg, O_FORMAT_PGMASC_STR) == 0)
            opt_format = O_FORMAT_PGMASC;
#if HAVE_PNG
          else if (strcmp(optarg, O_FORMAT_PNG_STR) == 0)
            opt_format = O_FORMAT_PNG;
          else if (strcmp(optarg, O_FORMAT_PNG16_STR) == 0)
            opt_format = O_FORMAT_PNG16;
#endif
          else if (strcmp(optarg, O_FORMAT_TEXT_STR) == 0)
                opt_format = O_FORMAT_TEXT;
          else if (strcmp(optarg, O_FORMAT_RAW_STR) == 0)
                opt_format = O_FORMAT_RAW;
#if HAVE_GIF
          else if (strcmp(optarg, O_FORMAT_GIF_STR) == 0)
            opt_format = O_FORMAT_GIF;
#endif
  else {
              sys_error (ERR_SEVERE, "Invalid format mode %s", optarg);
              ifexport_usage(argv[0]);
              return (1);
            }
          break;
        case O_LABELS:
          opt_labels = 1;
          break;
        case O_VERBOSE:
          opt_verbose = 1;
          break;
        case O_DEBUG:
          opt_debug = 1;
          break;
        case O_STATS:
          opt_stats = 1;
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
          ifexport_usage(argv[0]);
          return (0);
        default:
          ifexport_usage(argv[0]);
          return (1);
        }
    }

  if (optind + 2 != argc) {
    ifexport_usage(argv[0]);
    return (1);
  }

  in_file = argv[optind];
  out_file = argv[optind+1];

  pim = new ImageFile ();
  ImageFile& im = *pim;
  if (! im.fileRead(in_file)) {
    sys_error (ERR_FATAL, "File %s does not exist", in_file);
    return (1);
  }

  if (opt_labels)
    im.printLabels(std::cout);

  if (opt_stats || (! (opt_set_max && opt_set_min))) {
      double min, max, mean, mode, median, stddev;
      double window = 0;
      im.statistics(min, max, mean, mode, median, stddev);

      if (opt_auto == O_AUTO_FULL) {
        if (! opt_set_max)
            densmax = max;
        if (! opt_set_min)
            densmin = min;
      }
      if (opt_stats || opt_auto != O_AUTO_FULL) {

          if (opt_auto == O_AUTO_FULL)
              ;
          else if (opt_auto == O_AUTO_STD1)
              window = stddev;
          else if (opt_auto == O_AUTO_STD0_1)
              window = stddev * 0.1;
          else if (opt_auto == O_AUTO_STD0_5)
              window = stddev * 0.5;
          else if (opt_auto == O_AUTO_STD2)
              window = stddev * 2;
          else if (opt_auto == O_AUTO_STD3)
              window = stddev * 3;
          else {
              sys_error (ERR_SEVERE, "Internal Error: Invalid auto mode %d", opt_auto);
              return (1);
          }
      }
      if (opt_stats) {
          std::cout <<"nx: " << im.nx() << std::endl;
          std::cout <<"ny: " << im.ny() << std::endl;
          std::cout <<"min: " << min << std::endl;
          std::cout <<"max: " << max << std::endl;
          std::cout <<"mean: " << mean << std::endl;
          std::cout <<"mode: " << mode << std::endl;
          std::cout <<"stddev: " << stddev << std::endl;
      }
      if (opt_auto != O_AUTO_FULL) {
          double center;

          if (opt_center == O_CENTER_MEDIAN)
              center = median;
          else if (opt_center == O_CENTER_MODE)
              center = mode;
          else if (opt_center == O_CENTER_MEAN)
              center = mean;
          else {
              sys_error (ERR_SEVERE, "Internal Error: Invalid center mode %d", opt_center);
              return (1);
          }
          if (! opt_set_max)
              densmax = center + window;
          if (! opt_set_min)
              densmin = center - window;
      }
  }

  if (opt_stats) {
    std::cout << "min display: " << densmin << std::endl;
    std::cout << "max display: " << densmax << std::endl;
  }

  if (opt_format == O_FORMAT_PGM)
    im.writeImagePGM (out_file, opt_scale, opt_scale, densmin, densmax);
  else if (opt_format == O_FORMAT_PGMASC)
    im.writeImagePGMASCII (out_file, opt_scale, opt_scale, densmin, densmax);
#if HAVE_PNG
  else if (opt_format == O_FORMAT_PNG)
    im.writeImagePNG (out_file, 8, opt_scale, opt_scale, densmin, densmax);
  else if (opt_format == O_FORMAT_PNG16)
    im.writeImagePNG (out_file, 16, opt_scale, opt_scale, densmin, densmax);
#endif
#if HAVE_GIF
  else if (opt_format == O_FORMAT_GIF)
    im.writeImageGIF (out_file, opt_scale, opt_scale, densmin, densmax);
#endif
  else if (opt_format == O_FORMAT_TEXT)
        im.writeImageText (out_file);
  else if (opt_format == O_FORMAT_RAW)
        im.writeImageRaw (out_file, opt_scale, opt_scale);
  else
    {
      sys_error (ERR_SEVERE, "Internal Error: Invalid format mode %d", opt_format);
      return (1);
    }
  return (0);
}


#ifndef NO_MAIN
int
main (int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = ifexport_main(argc, argv);
  } catch (exception e) {
          std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
          std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif
