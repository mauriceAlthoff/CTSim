/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          phm2if.cpp
**   Purpose:       Convert an phantom object to an image file
**   Programmer:    Kevin Rosenberg
**   Date Started:  1984
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


enum { O_PHANTOM, O_DESC, O_NSAMPLE, O_FILTER, O_VIEW_RATIO, O_TRACE, O_VERBOSE, O_HELP,
O_PHMFILE, O_FILTER_DOMAIN, O_FILTER_BW, O_FILTER_PARAM, O_DEBUG, O_VERSION };

static struct option my_options[] =
{
  {"phantom", 1, 0, O_PHANTOM},
  {"phmfile", 1, 0, O_PHMFILE},
  {"desc", 1, 0, O_DESC},
  {"nsample", 1, 0, O_NSAMPLE},
  {"filter", 1, 0, O_FILTER},
  {"filter-domain", 1, 0, O_FILTER_DOMAIN},
  {"filter-bw", 1, 0, O_FILTER_BW},
  {"filter-param", 1, 0, O_FILTER_PARAM},
  {"trace", 1, 0, O_TRACE},
  {"view-ratio", 1, 0, O_VIEW_RATIO},
  {"verbose", 0, 0, O_VERBOSE},
  {"debug", 0, 0, O_DEBUG},
  {"help", 0, 0, O_HELP},
  {"version", 0, 0, O_VERSION},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";

void
phm2if_usage (const char *program)
{
  std::cout << "phm2if_usage: " << fileBasename(program) << " outfile nx ny [--phantom phantom-name] [--phmfile filename] [--filter filter-name] [OPTIONS]\n";
  std::cout << "Generate phantom image from a predefined --phantom or a --phmfile\n";
  std::cout << std::endl;
  std::cout << "     outfile         Name of output file for image\n";
  std::cout << "     nx              Number of pixels X-axis\n";
  std::cout << "     ny              Number of pixels Y-axis\n";
  std::cout << "     --view-ratio    View diameter to phantom diameter ratio (default = 1)\n";
  std::cout << "     --phantom       Phantom to use for projection\n";
  std::cout << "        herman       Herman head phantom\n";
  std::cout << "        shepp-logan  Shepp-Logan head phantom\n";
  std::cout << "        unit-pulse    Unit pulse phantom\n";
  std::cout << "     --phmfile       Generate Phantom from phantom file\n";
  std::cout << "     --filter        Generate Phantom from a filter function\n";
  std::cout << "       abs_bandlimit Abs * Bandlimiting\n";
  std::cout << "       abs_sinc      Abs * Sinc\n";
  std::cout << "       abs_cos       Abs * Cosine\n";
  std::cout << "       abs_hamming   Abs * Hamming\n";
  std::cout << "       shepp         Shepp-Logan\n";
  std::cout << "       bandlimit     Bandlimiting\n";
  std::cout << "       sinc          Sinc\n";
  std::cout << "       cos           Cosine\n";
  std::cout << "       triangle      Triangle\n";
  std::cout << "       hamming       Hamming\n";
  std::cout << "     --filter-param  Alpha level for Hamming filter\n";
  std::cout << "     --filter-domain Set domain of filter\n";
  std::cout << "         spatial     Spatial domain (default)\n";
  std::cout << "         freq        Frequency domain\n";
  std::cout << "     --filter-bw     Filter bandwidth (default = 1)\n";
  std::cout << "     --desc          Description of raysum\n";
  std::cout << "     --nsample       Number of samples per axis per pixel (default = 1)\n";
  std::cout << "     --trace         Trace level to use\n";
  std::cout << "        none         No tracing (default)\n";
  std::cout << "        console      Trace text level\n";
  std::cout << "     --debug         Debug mode\n";
  std::cout << "     --verbose       Verbose mode\n";
  std::cout << "     --version       Print version\n";
  std::cout << "     --help          Print this help message\n";
}

#ifdef HAVE_MPI
void mpi_gather_image (MPIWorld& mpiWorld, ImageFile* pImGlobal, ImageFile* pImLocal, const int optDebug);
#endif

int
phm2if_main (int argc, char* const argv[])
{
  ImageFile* pImGlobal = NULL;
  Phantom phm;
  std::string optPhmName;
  std::string optFilterName;
  std::string optDomainName (SignalFilter::convertDomainIDToName (SignalFilter::DOMAIN_SPATIAL));
  std::string optOutFilename;
  std::string optDesc;
  std::string optPhmFilename;
  int opt_nx = 0;
  int opt_ny = 0;
  int opt_nsample = 1;
  double optViewRatio = 1.;
  double optFilterParam = 1.;
  double optFilterBW = 1.;
  int optTrace = Trace::TRACE_NONE;
  bool optVerbose = false;
  bool optDebug = false;
  char *endptr = NULL;
  char *endstr;
  Timer timerProgram;

#ifdef HAVE_MPI
  ImageFile* pImLocal = NULL;
  MPIWorld mpiWorld (argc, argv);
  if (mpiWorld.getRank() == 0) {
#endif
    while (1) {
      int c = getopt_long(argc, argv, "", my_options, NULL);
      if (c == -1)
        break;

      switch (c) {
      case O_PHANTOM:
        optPhmName = optarg;
        break;
      case O_PHMFILE:
        optPhmFilename = optarg;
        break;
      case O_VERBOSE:
        optVerbose = true;
        break;
      case O_DEBUG:
        optDebug = true;
        break;
      case O_TRACE:
        if ((optTrace = Trace::convertTraceNameToID(optarg)) == Trace::TRACE_INVALID) {
          phm2if_usage(argv[0]);
          return (1);
        }
        break;
      case O_FILTER:
        optFilterName = optarg;
        break;
      case O_FILTER_DOMAIN:
        optDomainName = optarg;
        break;
      case O_DESC:
        optDesc =  optarg;
        break;
      case O_FILTER_BW:
        optFilterBW = strtod(optarg, &endptr);
        endstr = optarg + strlen(optarg);
        if (endptr != endstr) {
          sys_error(ERR_SEVERE,"Error setting --filter-bw to %s\n", optarg);
          phm2if_usage(argv[0]);
          return (1);
        }
        break;
      case O_VIEW_RATIO:
        optViewRatio = strtod(optarg, &endptr);
        endstr = optarg + strlen(optarg);
        if (endptr != endstr) {
          sys_error(ERR_SEVERE,"Error setting --view-ratio to %s\n", optarg);
          phm2if_usage(argv[0]);
          return (1);
        }
        break;
      case O_FILTER_PARAM:
        optFilterParam = strtod(optarg, &endptr);
        endstr = optarg + strlen(optarg);
        if (endptr != endstr) {
          sys_error(ERR_SEVERE,"Error setting --filter-param to %s\n", optarg);
          phm2if_usage(argv[0]);
          return (1);
        }
        break;
      case O_NSAMPLE:
        opt_nsample = strtol(optarg, &endptr, 10);
        endstr = optarg + strlen(optarg);
        if (endptr != endstr) {
          sys_error(ERR_SEVERE,"Error setting --nsample to %s\n", optarg);
          phm2if_usage(argv[0]);
          return (1);
        }
        break;
      case O_VERSION:
#ifdef VERSION
        std::cout << "Version " << VERSION << std::endl << g_szIdStr << std::endl;
#else
        std::cerr << "Unknown version number\n";
#endif
      case O_HELP:
      case '?':
        phm2if_usage(argv[0]);
        return (0);
      default:
        phm2if_usage(argv[0]);
        return (1);
      }
    }

    if (optPhmName == "" && optFilterName == "" && optPhmFilename == "") {
      std::cerr << "No phantom defined\n" << std::endl;
      phm2if_usage(argv[0]);
      return (1);
    }

    if (optind + 3 != argc) {
      phm2if_usage(argv[0]);
      return (1);
    }
    optOutFilename = argv[optind];
    opt_nx = strtol(argv[optind+1], &endptr, 10);
    endstr = argv[optind+1] + strlen(argv[optind+1]);
    if (endptr != endstr) {
      sys_error(ERR_SEVERE,"Error setting nx to %s\n", argv[optind+1]);
      phm2if_usage(argv[0]);
      return (1);
    }
    opt_ny = strtol(argv[optind+2], &endptr, 10);
    endstr = argv[optind+2] + strlen(argv[optind+2]);
    if (endptr != endstr) {
      sys_error(ERR_SEVERE,"Error setting ny to %s\n", argv[optind+2]);
      phm2if_usage(argv[0]);
      return (1);
    }

    std::ostringstream oss;
    oss << "phm2if: nx=" << opt_nx << ", ny=" << opt_ny << ", viewRatio=" << optViewRatio << ", nsample=" << opt_nsample << ", ";
    if (optPhmFilename != "")
      oss << "phantomFile=" << optPhmFilename;
    else if (optPhmName != "")
      oss << "phantom=" << optPhmName;
    else if (optFilterName != "") {
      oss << "filter=" << optFilterName << " - " << optDomainName;
    }
    if (optDesc != "")
      oss << ": " << optDesc;
    optDesc = oss.str();

    if (optPhmName != "") {
      phm.createFromPhantom (optPhmName.c_str());
      if (phm.fail()) {
        std::cout << phm.failMessage() << std::endl << std::endl;
        phm2if_usage(argv[0]);
        return (1);
      }
    }

    if (optPhmFilename != "") {
      phm.createFromFile(optPhmFilename.c_str());
#ifdef HAVE_MPI
      if (mpiWorld.getRank() == 0)
        std::cerr << "Can't use phantom from file in MPI mode\n";
      return (1);
#endif
    }

    if (optVerbose)
      std::cout << "Rasterize Phantom to Image\n" << std::endl;
#ifdef HAVE_MPI
  }
#endif

#ifdef HAVE_MPI
  TimerCollectiveMPI timerBcast (mpiWorld.getComm());
  mpiWorld.BcastString (optPhmName);
  mpiWorld.getComm().Bcast (&optVerbose, 1, MPI::INT, 0);
  mpiWorld.getComm().Bcast (&optDebug, 1, MPI::INT, 0);
  mpiWorld.getComm().Bcast (&optTrace, 1, MPI::INT, 0);
  mpiWorld.getComm().Bcast (&opt_nx, 1, MPI::INT, 0);
  mpiWorld.getComm().Bcast (&opt_ny, 1, MPI::INT, 0);
  mpiWorld.getComm().Bcast (&opt_nsample, 1, MPI::INT, 0);
  mpiWorld.getComm().Bcast (&optViewRatio, 1, MPI::DOUBLE, 0);
  mpiWorld.getComm().Bcast (&optFilterParam, 1, MPI::DOUBLE, 0);
  mpiWorld.getComm().Bcast (&optFilterBW, 1, MPI::DOUBLE, 0);

  mpiWorld.BcastString (optFilterName);
  mpiWorld.BcastString (optDomainName);

  if (optVerbose)
    timerBcast.timerEndAndReport ("Time to broadcast variables");

  mpiWorld.setTotalWorkUnits (opt_nx);

  if (mpiWorld.getRank() > 0 && optPhmName != "")
    phm.createFromPhantom (optPhmName.c_str());

  if (mpiWorld.getRank() == 0) {
    pImGlobal = new ImageFile (opt_nx, opt_ny);
  }
  pImLocal = new ImageFile (opt_nx, opt_ny);
#else
  pImGlobal = new ImageFile (opt_nx, opt_ny);
#endif

  ImageFileArray v = NULL;
#ifdef HAVE_MPI
  if (mpiWorld.getRank() == 0)
    v = pImGlobal->getArray ();

  if (phm.getComposition() == P_UNIT_PULSE) {
    if (mpiWorld.getRank() == 0) {
      v[opt_nx/2][opt_ny/2] = 1.;
    }
  } else if (optFilterName != "") {
    if (mpiWorld.getRank() == 0) {
      pImGlobal->filterResponse (optDomainName.c_str(), optFilterBW, optFilterName.c_str(), optFilterParam);
    }
  } else {
    TimerCollectiveMPI timerRasterize (mpiWorld.getComm());
    phm.convertToImagefile (*pImLocal, optViewRatio, opt_nsample, optTrace, mpiWorld.getMyStartWorkUnit(), mpiWorld.getMyLocalWorkUnits(), false);
    if (optVerbose)
      timerRasterize.timerEndAndReport ("Time to rasterize phantom");

    TimerCollectiveMPI timerGather (mpiWorld.getComm());
    mpi_gather_image (mpiWorld, pImGlobal, pImLocal, optDebug);
    if (optVerbose)
      timerGather.timerEndAndReport ("Time to gather image");
  }
#else
  v = pImGlobal->getArray ();
  if (phm.getComposition() == P_UNIT_PULSE) {
    v[opt_nx/2][opt_ny/2] = 1.;
  } else if (optFilterName != "") {
    pImGlobal->filterResponse (optDomainName.c_str(), optFilterBW, optFilterName.c_str(), optFilterParam);
  } else {
    phm.convertToImagefile (*pImGlobal, optViewRatio, opt_nsample, optTrace);
  }
#endif

#ifdef HAVE_MPI
  if (mpiWorld.getRank() == 0)
#endif
  {
    double calctime = timerProgram.timerEnd ();
    pImGlobal->labelAdd (Array2dFileLabel::L_HISTORY, optDesc.c_str(), calctime);
    pImGlobal->fileWrite (optOutFilename.c_str());
    if (optVerbose)
      std::cout << "Time to rasterize phantom: " << calctime << " seconds\n";
  }

  delete pImGlobal;
#ifdef HAVE_MPI
  delete pImLocal;
#endif

  return (0);
}



#ifdef HAVE_MPI
void mpi_gather_image (MPIWorld& mpiWorld, ImageFile* pImGlobal, ImageFile* pImLocal, const int optDebug)
{
  ImageFileArray vLocal = pImLocal->getArray();
  ImageFileArray vGlobal = NULL;
  int nyLocal = pImLocal->ny();

  if (mpiWorld.getRank() == 0)
    vGlobal = pImGlobal->getArray();

  for (int iw = 0; iw < mpiWorld.getMyLocalWorkUnits(); iw++)
    mpiWorld.getComm().Send(vLocal[iw], nyLocal, pImLocal->getMPIDataType(), 0, 0);

  if (mpiWorld.getRank() == 0) {
    for (int iProc = 0; iProc < mpiWorld.getNumProcessors(); iProc++) {
      for (int iw = mpiWorld.getStartWorkUnit(iProc); iw <= mpiWorld.getEndWorkUnit(iProc); iw++) {
        MPI::Status status;
        mpiWorld.getComm().Recv(vGlobal[iw], nyLocal, pImLocal->getMPIDataType(), iProc, 0, status);
      }
    }
  }

}
#endif

#ifndef NO_MAIN
int
main (int argc, char* argv[])
{
  int retval = 1;

  try {
    retval = phm2if_main(argc, argv);
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif
