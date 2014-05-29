/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          phm2helix.cpp
**   Purpose:       Take projections of a phantom object
**   Programmers:   Ian Kay and Kevin Rosenberg
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
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/

#include "ct.h"
#include "timer.h"


enum { O_PHANTOMPROG, O_PHMFILE,  O_DESC, O_NRAY, O_ROTANGLE, O_GEOMETRY, O_FOCAL_LENGTH, O_CENTER_DETECTOR_LENGTH,
  O_VIEW_RATIO, O_SCAN_RATIO, O_OFFSETVIEW,  O_TRACE, O_VERBOSE, O_HELP, O_DEBUG, O_VERSION };

static struct option phm2helix_options[] =
{
  {"phantom", 1, 0, O_PHANTOMPROG},
  {"desc", 1, 0, O_DESC},
  {"nray", 1, 0, O_NRAY},
  {"rotangle", 1, 0, O_ROTANGLE},
  {"geometry", 1, 0, O_GEOMETRY},
  {"focal-length", 1, 0, O_FOCAL_LENGTH},
  {"center-detector-length", 1, 0, O_CENTER_DETECTOR_LENGTH},
  {"view-ratio", 1, 0, O_VIEW_RATIO},
  {"scan-ratio", 1, 0, O_SCAN_RATIO},
  {"offsetview", 1, 0, O_OFFSETVIEW},
  {"trace", 1, 0, O_TRACE},
  {"verbose", 0, 0, O_VERBOSE},
  {"help", 0, 0, O_HELP},
  {"debug", 0, 0, O_DEBUG},
  {"version", 0, 0, O_VERSION},
  {"phmfile", 1, 0, O_PHMFILE},
  {0, 0, 0, 0}
};

static const char* g_szIdStr = "$Id$";


void
phm2helix_usage (const char *program)
{
          std::cout << "usage: " << fileBasename(program) << " outfile ndet nview phmprog [OPTIONS]\n";
          std::cout << "Calculate (projections) through time varying phantom object  \n\n";
          std::cout << "     outfile          Name of output file for projectsions\n";
          std::cout << "     ndet             Number of detectors\n";
          std::cout << "     nview            Number of rotated views\n";
          std::cout << "     phmprog          Name of phm generation executable\n";
          std::cout << "     --phmfile        Temp phantom file name \n";
          std::cout << "     --desc           Description of raysum\n";
          std::cout << "     --nray           Number of rays per detector (default = 1)\n";
          std::cout << "     --rotangle       Angle to rotate view through (fraction of a circle)\n";
          std::cout << "                      (default = select appropriate for geometry)\n";
          std::cout << "     --geometry       Geometry of scanning\n";
          std::cout << "        parallel      Parallel scan beams (default)\n";
          std::cout << "        equilinear    Equilinear divergent scan beams\n";
          std::cout << "        equiangular   Equiangular divergent scan beams\n";
          std::cout << "     --focal-length   Focal length ratio (ratio to radius of phantom)\n";
          std::cout << "                      (default = 1)\n";
          std::cout << "     --view-ratio     Length to view (view diameter to phantom diameter)\n";
          std::cout << "                      (default = 1)\n";
          std::cout << "     --scan-ratio     Length to scan (scan diameter to view diameter)\n";
          std::cout << "                      (default = 1)\n";
          std::cout << "     --offsetview      Intial gantry offset in  'views' (default = 0)\n";
          std::cout << "     --trace          Trace level to use\n";
          std::cout << "        none          No tracing (default)\n";
          std::cout << "        console       Trace text level\n";
          std::cout << "     --verbose        Verbose mode\n";
          std::cout << "     --debug          Debug mode\n";
          std::cout << "     --version        Print version\n";
          std::cout << "     --help           Print this help message\n";
}


int
phm2helix_main (int argc, char* const argv[])
{
        Phantom phm;
        std::string optGeometryName = Scanner::convertGeometryIDToName(Scanner::GEOMETRY_PARALLEL);
        char *opt_outfile = NULL;
        std::string opt_desc;
        std::string opt_PhmProg;
        std::string opt_PhmFileName = "tmpphmfile";
        int opt_ndet;
        int opt_nview;
        int opt_offsetview = 0;
        int opt_nray = 1;
        double dOptFocalLength = 2.;
        double dOptCenterDetectorLength = 2;
        double dOptViewRatio = 1.;
        double dOptScanRatio = 1.;
        int opt_trace = Trace::TRACE_NONE;
        int opt_verbose = 0;
        int opt_debug = 0;
        double opt_rotangle = -1;
        char* endptr = NULL;
        char* endstr;

        Timer timerProgram;

        while (1) {
                int c = getopt_long(argc, argv, "", phm2helix_options, NULL);

                if (c == -1)
                break;

        switch (c) {
        case O_VERBOSE:
                opt_verbose = 1;
                break;
        case O_DEBUG:
                opt_debug = 1;
                break;
        case O_TRACE:
                if ((opt_trace = Trace::convertTraceNameToID(optarg))
                                                                == Trace::TRACE_INVALID) {
                        phm2helix_usage(argv[0]);
                        return (1);
                }
                break;
                          case O_PHMFILE:
                                opt_PhmFileName = optarg;
                                break;
                          case O_DESC:
                                opt_desc = optarg;
                                break;
                          case O_ROTANGLE:
                                opt_rotangle = strtod(optarg, &endptr);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                        std::cerr << "Error setting --rotangle to " << optarg << std::endl;
                                        phm2helix_usage(argv[0]);
                                        return (1);
                                }
                                break;
                          case O_GEOMETRY:
                                optGeometryName = optarg;
                                break;
                          case O_FOCAL_LENGTH:
                                dOptFocalLength = strtod(optarg, &endptr);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                        std::cerr << "Error setting --focal-length to " << optarg << std::endl;
                                        phm2helix_usage(argv[0]);
                                        return (1);
                                }
                                break;
                          case  O_CENTER_DETECTOR_LENGTH:
                                dOptCenterDetectorLength = strtod(optarg, &endptr);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                        std::cerr << "Error setting --center-detector-length to " << optarg << std::endl;
                                        phm2helix_usage(argv[0]);
                                        return (1);
                                }
                          break;
                          case O_VIEW_RATIO:
                                dOptViewRatio = strtod(optarg, &endptr);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                        std::cerr << "Error setting --view-ratio to " << optarg << std::endl;
                                        phm2helix_usage(argv[0]);
                                        return (1);
                                }
                                break;
                          case O_SCAN_RATIO:
                                dOptScanRatio = strtod(optarg, &endptr);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                        std::cerr << "Error setting --scan-ratio to " << optarg << std::endl;
                                        phm2helix_usage(argv[0]);
                                        return (1);
                                }
                                break;
                          case O_NRAY:
                                opt_nray = strtol(optarg, &endptr, 10);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                  std::cerr << "Error setting --nray to %s" << optarg << std::endl;
                                  phm2helix_usage(argv[0]);
                                  return (1);
                                }
                                break;
                          case O_OFFSETVIEW:
                                opt_offsetview = strtol(optarg, &endptr, 10);
                                endstr = optarg + strlen(optarg);
                                if (endptr != endstr) {
                                  std::cerr << "Error setting --offsetview to %s" << optarg << std::endl;
                                  phm2helix_usage(argv[0]);
                                  return (1);
                                }
                                break;
                          case O_VERSION:
#ifdef VERSION
                                  std::cout << "Version: " << VERSION << std::endl << g_szIdStr << std::endl;
#else
                                          std::cout << "Unknown version number\n";
#endif
                                  return (0);
                          case O_HELP:
                          case '?':
                                phm2helix_usage(argv[0]);
                                return (0);
                          default:
                                phm2helix_usage(argv[0]);
                                return (1);
                        } // end of switch
          } // end of while loop

          if (optind + 4 != argc) {
                phm2helix_usage(argv[0]);
                return (1);
          }

          opt_outfile = argv[optind];
          opt_ndet = strtol(argv[optind+1], &endptr, 10);
          endstr = argv[optind+1] + strlen(argv[optind+1]);
          if (endptr != endstr) {
                std::cerr << "Error setting --ndet to " << argv[optind+1] << std::endl;
                phm2helix_usage(argv[0]);
                return (1);
          }
          opt_nview = strtol(argv[optind+2], &endptr, 10);
          endstr = argv[optind+2] + strlen(argv[optind+2]);
          if (endptr != endstr) {
                std::cerr << "Error setting --nview to " << argv[optind+2] << std::endl;
                phm2helix_usage(argv[0]);
                return (1);
          }
          opt_PhmProg = argv[optind+3];

          if (opt_rotangle < 0) {
                if (optGeometryName.compare ("parallel") == 0)
                  opt_rotangle = 0.5;
                else
                  opt_rotangle = 1.0;
          }

          std::ostringstream desc;
          desc << "phm2helix: NDet=" << opt_ndet
                   << ", Nview=" << opt_nview
                   << ", NRay=" << opt_nray
                   << ", RotAngle=" << opt_rotangle
                   << ", OffsetView =" << opt_offsetview
                   << ", Geometry=" << optGeometryName
                   << ", PhantomProg=" << opt_PhmProg
                   << ", PhmFileName=" << opt_PhmFileName;
          if (opt_desc.length()) {
                desc << ": " << opt_desc;
          }
          opt_desc = desc.str();

          opt_rotangle *= TWOPI;

          int stat;
          char extcommand[100];
          if(opt_debug != 0)
                        std::cout  <<  opt_PhmProg  <<  " " << 0 << " " <<  opt_nview << " " << opt_PhmFileName  << std::endl;
           //extcommand <<  opt_PhmProg  <<  " " << 0 << " " <<  opt_nview << " " << opt_PhmFileName ;

          sprintf(extcommand, "%s %d %d %s",    opt_PhmProg.c_str(), 0, opt_nview, opt_PhmFileName.c_str() );

           stat = system( extcommand );
          if (stat != 0 )
                        std::cerr << "Error executing external phantom program " << opt_PhmProg << " with command " << extcommand << std::endl;

          phm.createFromFile (opt_PhmFileName.c_str());
          remove(opt_PhmFileName.c_str());

          Scanner scanner (phm, optGeometryName.c_str(), opt_ndet, opt_nview,
                                opt_offsetview, opt_nray, opt_rotangle, dOptFocalLength,
                                dOptCenterDetectorLength, dOptViewRatio, dOptScanRatio);
          if (scanner.fail()) {
                 std::cout << "Scanner Creation Error: " << scanner.failMessage()
                                                        << std::endl;
                 return (1);
          }

          Projections pjGlobal(scanner);


          for( int iView = 0; iView < opt_nview; iView++ ){
                if(opt_debug != 0)
                        std::cout  <<  opt_PhmProg  <<  " " << iView << " " <<  opt_nview << " " << opt_PhmFileName  << std::endl;
           //extcommand <<  opt_PhmProg  <<  " " << iView << " " <<  opt_nview << " " << opt_PhmFileName ;

                sprintf(extcommand, "%s %d %d %s",      
                        opt_PhmProg.c_str(), iView, opt_nview, 
                        opt_PhmFileName.c_str() );
                stat = system( extcommand );

                if (stat != 0 )
                        std::cerr << "Error executing external phantom program " << opt_PhmProg << " with command " << extcommand << std::endl;
                Phantom phmtmp;
                phmtmp.createFromFile (opt_PhmFileName.c_str());

                scanner.collectProjections (pjGlobal, phmtmp, iView,
                          1, scanner.offsetView(), true, opt_trace);
                remove(opt_PhmFileName.c_str());
          }


          pjGlobal.setCalcTime (timerProgram.timerEnd());
          pjGlobal.setRemark (opt_desc);
          pjGlobal.write (opt_outfile);
          if (opt_verbose) {
            phm.print (std::cout);
            std::cout << std::endl;
            std::ostringstream os;
            pjGlobal.printScanInfo (os);
            std::cout << os.str() << std::endl;
            std::cout << "  Remark: " << pjGlobal.remark() << std::endl;
            std::cout << "Run time: " << pjGlobal.calcTime() << " seconds\n";
          }
          
          return (0);
}


#ifndef NO_MAIN
int
main (int argc, char* argv[])
{
  int retval = 1;

  try {
    retval = phm2helix_main(argc, argv);
#if HAVE_DMALLOC
    //    dmalloc_shutdown();
#endif
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception\n";
  }

  return (retval);
}
#endif

