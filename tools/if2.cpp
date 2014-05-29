/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          if2.cpp
**   Purpose:       Manipulate two image files
**   Programmer:    Kevin Rosenberg
**   Date Started:  May 2000
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
#include "timer.h"

enum {O_ADD, O_SUB, O_MUL, O_DIVIDE, O_COMP, O_ROW_PLOT, O_COLUMN_PLOT, O_VERBOSE, O_HELP, O_VERSION};

static struct option my_options[] =
{
  {"add", 0, 0, O_ADD},
  {"sub", 0, 0, O_SUB},
  {"multiply", 0, 0, O_MUL},
  {"divide", 0, 0, O_DIVIDE},
  {"comp", 0, 0, O_COMP},
  {"column-plot", 1, 0, O_COLUMN_PLOT},
  {"row-plot", 1, 0, O_ROW_PLOT},
  {"verbose", 0, 0, O_VERBOSE},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

void
if2_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " infile1 infile2 outfile [OPTIONS]\n";
  std::cout << "Perform functions on two input image files\n";
  std::cout << std::endl;
  std::cout << "     infile1            Name of first input IF file\n";
  std::cout << "     infile2            Name of second input IF file\n";
  std::cout << "     outfile            Name of output Image or Plot file\n";
  std::cout << "     --add              Add images\n";
  std::cout << "     --sub              Subtract image 2 from image 1\n";
  std::cout << "     --mul              Multiply images\n";
  std::cout << "     --comp             Compare images\n";
  std::cout << "     --column-plot n    Plot column\n";
  std::cout << "     --row-plot n       Plot row\n";
  std::cout << "     --verbose          Verbose modem\n";
  std::cout << "     --version          Print version\n";
  std::cout << "     --help             Print this help message\n";
}

int
if2_main (int argc, char *const argv[])
{
  ImageFile* pim_in1;
  ImageFile* pim_in2;
  ImageFile* pim_out = NULL;
  std::string in_file1;
  std::string in_file2;
  std::string strOutFile;
  int opt_verbose = 0;
  int opt_add = 0;
  int opt_sub = 0;
  int opt_mul = 0;
  bool opt_divide = false;
  int opt_comp = 0;
  bool opt_bImageOutputFile = false;
  bool opt_bPlotOutputFile = false;
  int opt_rowPlot = -1;
  int opt_columnPlot = -1;
  Timer timerProgram;

  while (1) {
    char* endptr;
    int c = getopt_long (argc, argv, "", my_options, NULL);

    if (c == -1)
      break;

    switch (c) {
    case O_ADD:
      opt_add = 1;
      opt_bImageOutputFile = true;
      break;
    case O_SUB :
      opt_sub = 1;
      opt_bImageOutputFile = true;
      break;
    case O_MUL:
      opt_mul = 1;
      opt_bImageOutputFile = true;
      break;
    case O_DIVIDE:
      opt_divide = true;
      opt_bImageOutputFile = true;
      break;
    case O_ROW_PLOT:
      opt_rowPlot = strtol(optarg, &endptr, 10);
      if (endptr != optarg + strlen(optarg)) {
        if2_usage(argv[0]);
      }
      opt_bPlotOutputFile = true;
      break;
    case O_COLUMN_PLOT:
      opt_columnPlot = strtol(optarg, &endptr, 10);
      if (endptr != optarg + strlen(optarg)) {
        if2_usage(argv[0]);
      }
      opt_bPlotOutputFile = true;
      break;
    case O_COMP:
      opt_comp = 1;
      break;
    case O_VERBOSE:
      opt_verbose = 1;
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
      if2_usage(argv[0]);
      return (0);
    default:
      if2_usage(argv[0]);
      return (1);
    }
  }

  if ((opt_bImageOutputFile || opt_bPlotOutputFile) && (optind + 3 != argc)) {
    if2_usage(argv[0]);
    return (1);
  }
  else if (! (opt_bImageOutputFile || opt_bPlotOutputFile) && optind + 2 != argc) {
    if2_usage(argv[0]);
    return (1);
  }

  in_file1 = argv[optind];
  in_file2 = argv[optind + 1];
  if (opt_bImageOutputFile || opt_bPlotOutputFile)
    strOutFile = argv[optind + 2];

  pim_in1 = new ImageFile ();
  pim_in2 = new ImageFile ();
  ImageFile& im_in1 = *pim_in1;
  ImageFile& im_in2 = *pim_in2;

  if (! im_in1.fileRead(in_file1) || ! im_in2.fileRead(in_file2)) {
    sys_error (ERR_WARNING, "Error reading an image");
    return (1);
  }

  if (im_in1.nx() != im_in2.nx() || im_in1.ny() != im_in2.ny()) {
    sys_error (ERR_SEVERE, "Error: Size of image 1 (%d,%d) and image 2 (%d,%d) do not match",
      im_in1.nx(), im_in1.ny(), im_in2.nx(), im_in2.ny());
    return(1);
  }
  if (im_in1.nx() < 0 || im_in1.ny() < 0) {
    sys_error (ERR_SEVERE, "Error: Size of image < 0");
    return(1);
  }

  ImageFileArray v1 = im_in1.getArray();
  ImageFileArray v2 = im_in2.getArray();
  ImageFileArray vout = NULL;

  if (opt_bImageOutputFile && opt_bPlotOutputFile) {
    sys_error (ERR_SEVERE, "Both Image and Plot output files can not be selected simultaneously");
    return (1);
  }
  if (opt_bImageOutputFile) {
    pim_out = new ImageFile (im_in1.nx(), im_in1.ny());
    vout = pim_out->getArray();
  }

  std::string strOperation;
  int nx = im_in1.nx();
  int ny = im_in1.ny();
  int nx2 = im_in2.nx();
  int ny2 = im_in2.ny();

  if (opt_add) {
    strOperation = "Add Images";
    im_in1.addImages (im_in2, *pim_out);
  } else if (opt_sub) {
    strOperation = "Subtract Images";
    im_in1.subtractImages (im_in2, *pim_out);
  } else if (opt_mul) {
    strOperation = "Multiply Images";
    im_in1.multiplyImages (im_in2, *pim_out);
  } else if (opt_divide) {
    strOperation = "Divide Images";
    im_in1.divideImages (im_in2, *pim_out);
  }
  if (opt_comp) {
    double d, r, e;
    im_in1.comparativeStatistics (im_in2, d, r, e);
    std::cout << "d=" << d << ", r=" << r << ", e=" << e << std::endl;
  }

  int i;
  if (opt_columnPlot > 0) {
    if (opt_columnPlot >= nx || opt_columnPlot >= nx2) {
      sys_error (ERR_SEVERE, "column-plot > nx");
      return (1);
    }
    double* plot_xaxis = new double [nx];
    for (i = 0; i < nx; i++)
      plot_xaxis[i] = i;

    PlotFile plotFile (3, nx);

    plotFile.addColumn (0, plot_xaxis);
    plotFile.addColumn (1, v1[opt_columnPlot]);
    plotFile.addColumn (2, v2[opt_columnPlot]);
    std::ostringstream os;
    os << "Column " << opt_columnPlot << " Comparison";
    plotFile.addDescription (os.str().c_str());
    std::string title("title ");
    title += os.str();
    plotFile.addEzsetCommand (title.c_str());
    plotFile.addEzsetCommand ("xlabel Column");
    plotFile.addEzsetCommand ("ylabel Pixel Value");
    plotFile.addEzsetCommand ("box");
    plotFile.addEzsetCommand ("grid");
    plotFile.addEzsetCommand ("xticks major 5");

    plotFile.fileWrite (strOutFile.c_str());

    delete plot_xaxis;
  }

  if (opt_rowPlot > 0) {
    if (opt_rowPlot >= ny || opt_rowPlot >= ny2) {
      sys_error (ERR_SEVERE, "row_plot > ny");
      return (1);
    }
    double* plot_xaxis = new double [ny];
    double* v1Row = new double [ny];
    double* v2Row = new double [ny2];

    for (i = 0; i < ny; i++)
      plot_xaxis[i] = i;
    for (i = 0; i < ny; i++)
      v1Row[i] = v1[i][opt_rowPlot];
    for (i = 0; i < ny2; i++)
      v2Row[i] = v2[i][opt_rowPlot];

    PlotFile plotFile (3, ny);

    plotFile.addColumn (0, plot_xaxis);
    plotFile.addColumn (1, v1Row);
    plotFile.addColumn (2, v2Row);
    std::ostringstream os;
    os << "Row " << opt_rowPlot << " Comparison";
    plotFile.addDescription (os.str().c_str());
    std::string title("title ");
    title += os.str();
    plotFile.addEzsetCommand (title.c_str());
    plotFile.addEzsetCommand ("xlabel Row");
    plotFile.addEzsetCommand ("ylabel Pixel Value");
    plotFile.addEzsetCommand ("box");
    plotFile.addEzsetCommand ("grid");
    plotFile.addEzsetCommand ("xticks major 5");

    plotFile.fileWrite (strOutFile.c_str());

    delete plot_xaxis;
    delete v1Row;
    delete v2Row;
  }

  if (opt_bImageOutputFile) {
    pim_out->labelsCopy (im_in1, "if2 file 1: ");
    pim_out->labelsCopy (im_in2, "if2 file 2: ");
    pim_out->labelAdd (Array2dFileLabel::L_HISTORY, strOperation.c_str(), timerProgram.timerEnd());
    pim_out->fileWrite (strOutFile);
  }

  return (0);
}

#ifndef NO_MAIN
int
main (int argc, char *const argv[])
{
  int retval = 1;

  try {
    retval = if2_main(argc, argv);
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif

