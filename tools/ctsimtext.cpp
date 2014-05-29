/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          ctsimtext.cpp
**   Purpose:       Text mode shell for CTSim
**   Programmer:    Kevin Rosenberg
**   Date Started:  Jan 2001
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

#ifdef HAVE_READLINE_H
extern "C" {
#include <readline.h>
#include <history.h>
};
#elif defined(HAVE_READLINE_READLINE_H)
extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
};
#endif
#include <exception>

// Master shell for all command-line tools
// If called as ctsimtext, program will look to next token on command-line as the function name
// If linked to ctsimtext, but executed as another name, eg pjrec, then program will use that
// linked name as name of function.

static const char* const g_szIdStr = "$Id$";
static const char* const s_szProgramName = "ctsimtext";
static const char* const s_szProgramName2 = "ctsimtext.exe";
static const char* const s_szProgramName3 = "ctsimtext-lam";

extern int if1_main (int argc, char* const argv[]);
extern int if2_main (int argc, char* const argv[]);
extern int ifexport_main (int argc, char* const argv[]);
extern int ifinfo_main (int argc, char* const argv[]);
extern int phm2if_main (int argc, char* const argv[]);
extern int phm2pj_main (int argc, char* const argv[]);
extern int phm2helix_main (int argc, char* const argv[]);
extern int pjHinterp_main (int argc, char* const argv[]);
extern int pj2if_main (int argc, char* const argv[]);
extern int pjinfo_main (int argc, char* const argv[]);
extern int pjrec_main (int argc, char* const argv[]);
extern int linogram_main (int argc, char* const argv[]);

static int processCommand (int argc, char* const argv[]);
static void convertStringToArgcv (char* szLine, int* piArgc, char*** pppArgv);


void
ctsimtext_usage (const char *program)
{
  std::cout << "usage: " << fileBasename(program) << " ctsim-function-name ctstim-function-parameters...\n";
  std::cout << "CTSim text shell";
#ifdef VERSION
  std::cout << ", Version " <<VERSION;
#endif
  std::cout << "\n\n";
  std::cout << "  if1           Single image file conversion\n";
  std::cout << "  if2           Dual image file conversions\n";
  std::cout << "  ifexport      Export an imagefile to a graphics file\n";
  std::cout << "  ifinfo        Image file information\n";
  std::cout << "  pj2if         Convert an projection file into an imagefile\n";
  std::cout << "  pjinfo        Projection file information\n";
  std::cout << "  pjrec         Projection reconstruction\n";
  std::cout << "  phm2if        Convert a geometric phantom into an imagefile\n";
  std::cout << "  phm2pj        Take projections of a phantom object\n";
  std::cout << "  phm2helix     Take projections of a phantom object\n";
  std::cout << "  pjHinterp     Interpolate helical projections of a phantom object\n";
  std::cout << "  linogram      Print linogram sampling\n";
}

void
interactive_usage ()
{
  std::cout << "usage: function-name parameters...\n";
  std::cout << "Available functions:\n";
  std::cout << "  ifexport      Export an imagefile to a graphics file\n";
  std::cout << "  ifinfo        Image file information\n";
  std::cout << "  if1           Single image file conversion\n";
  std::cout << "  if2           Dual image file conversions\n";
  std::cout << "  phm2if        Convert a geometric phantom into an imagefile\n";
  std::cout << "  phm2pj        Take projections of a phantom object\n";
  std::cout << "  phm2helix     Take projections of a phantom object\n";
  std::cout << "  pjinfo        Projection file information\n";
  std::cout << "  pj2if         Convert an projection file into an imagefile\n";
  std::cout << "  pjHinterp     Interpolate helical projections of a phantom object\n";
  std::cout << "  pjrec         Projection reconstruction\n";
  std::cout << "  quit          Quits shell\n";
  std::cout << "  linogram      Display linogram sampling\n";
  std::cout << "All functions accept --help as parameter for online help\n\n";
}

static bool s_bInteractive = false;

int
ctsimtext_main (int argc, char * argv[])
{
  int iReturn = 0;

  if (argc > 1 && (strcmp(s_szProgramName, fileBasename (argv[0])) == 0 || strcmp(s_szProgramName2, fileBasename (argv[0])) == 0 || strcmp(s_szProgramName3, fileBasename (argv[0])) == 0)) {
    argv++;
    argc--;
    iReturn = processCommand (argc, argv);
  } else if (argc >= 1 && ! (strcmp(s_szProgramName, fileBasename (argv[0])) == 0 || strcmp(s_szProgramName2, fileBasename (argv[0])) == 0 || strcmp(s_szProgramName3, fileBasename (argv[0])) == 0)) {
    iReturn = processCommand (argc, argv);
  } else {
    s_bInteractive = true;
    char szPrompt[] = "CTSim> ";
    std::cout << "CTSim Text Shell";
#ifdef VERSION
    std::cout << ", Version " << VERSION;
#endif
    std::cout << " (Type \"quit\" to end)\n\n";

    while (1) {
#ifdef HAVE_READLINE
      char* pszInputLine = readline (szPrompt);
      if (! pszInputLine)
        break;
      if (*pszInputLine != EOS)
        add_history (pszInputLine);

#else  // DONT_HAVE_READLINE

      static const int s_MaxLineLength = 1024;
      char* pszInputLine = new char [s_MaxLineLength+1];
      std::cout << szPrompt;
      std::cin.getline (pszInputLine, s_MaxLineLength);

#ifdef DEBUG
      std::cout << "#" << pszInputLine << "#\n";
#endif

      std::cout << std::flush;
      std::cout << "\n";
#endif  // DONT_HAVE_READLINE

      if (strncasecmp (pszInputLine, "quit", 4) == 0)
        break;

      convertStringToArgcv (pszInputLine, &argc, &argv);
#ifdef DEBUG
      for (int i = 0; i < argc; i++)
        std::cout << "Token " << i << ": " << argv[i] << "\n";
#endif
      iReturn = processCommand (argc, argv);

      delete pszInputLine;
    }
  }

  return iReturn;
}

static void
convertStringToArgcv (char* pszLine, int* piArgc, char*** pppArgv)
{
  char* pCurrentPos = pszLine;
  int nTokens = 0;
  std::vector<char*> vecpszToken;

  // Process line
  bool bInDoubleQuote = false;
  bool bInSingleQuote = false;
  bool bInToken = false;

  while (*pCurrentPos) {
    if (isspace (*pCurrentPos)) {
      if (! bInToken)
        *pCurrentPos = 0;
      else if (bInSingleQuote || bInDoubleQuote)
        ;
      else { // in non-quote token
        *pCurrentPos = 0;
        bInToken = false;
      }
    }
    else if (*pCurrentPos == '\"') {
      if (bInSingleQuote) {
        bInSingleQuote = false;
        *pCurrentPos = 0;
      } else {
        bInSingleQuote = true;
        if (! bInToken) {
          bInToken = true;
          nTokens++;
          vecpszToken.push_back (pCurrentPos+1);
        }
      }
    } else if (*pCurrentPos == '\'') {
      if (bInDoubleQuote) {
        bInDoubleQuote = false;
        *pCurrentPos = 0;
      } else {
        bInDoubleQuote = true;
        if (! bInToken) {
          bInToken = true;
          nTokens++;
          vecpszToken.push_back (pCurrentPos+1);
        }
      }
    } else if (! bInToken) {   // nonwhite, non-quote character
      bInToken = true;
      nTokens++;
      vecpszToken.push_back (pCurrentPos);
    }

    pCurrentPos++;
  }

  *piArgc = nTokens;
  if (nTokens > 0) {
    *pppArgv = new char* [nTokens];
    for (unsigned int iToken = 0; iToken < vecpszToken.size(); iToken++)
      (*pppArgv)[iToken] = vecpszToken[iToken];
  } else
    *pppArgv = NULL;
}

static int
processCommand (int argc, char* const argv[])
{
  if (argc < 1)
    return 1;

  const char* const pszFunction = fileBasename (argv[0]);

  try {
    if (strcasecmp (pszFunction, "if1") == 0)
      return if1_main (argc, argv);
    else if (strcasecmp (pszFunction, "if2") == 0)
      return if2_main (argc, argv);
    else if (strcasecmp (pszFunction, "ifexport") == 0)
      return ifexport_main (argc, argv);
    else if (strcasecmp (pszFunction, "ifinfo") == 0)
      return ifinfo_main (argc, argv);
    else if (strcasecmp (pszFunction, "phm2if") == 0)
      return phm2if_main (argc, argv);
    else if (strcasecmp (pszFunction, "phm2pj") == 0)
      return phm2pj_main (argc, argv);
    else if (strcasecmp (pszFunction, "phm2helix") == 0)
      return phm2helix_main (argc, argv);
    else if (strcasecmp (pszFunction, "pjHinterp") == 0)
      return pjHinterp_main (argc, argv);
    else if (strcasecmp (pszFunction, "pj2if") == 0)
      return pj2if_main (argc, argv);
    else if (strcasecmp (pszFunction, "pjinfo") == 0)
      return pjinfo_main (argc, argv);
    else if (strcasecmp (pszFunction, "pjrec") == 0)
      return pjrec_main (argc, argv);
    else if (strcasecmp (pszFunction, "linogram") == 0)
      return linogram_main (argc, argv);
    else {
      std::cout << "Unknown function name: " << pszFunction << "\n";
      if (s_bInteractive)
        interactive_usage();
      else
        ctsimtext_usage (s_szProgramName);
      return 1;
    }
  } catch (std::exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception caught\n";
  }

  return 1;
}

int
main (int argc, char* argv[])
{
#if defined(HAVE_FFTW) && defined(HAVE_GETENV)
  const char* const pszHome = getenv("HOME");
  char* pszWisdom = NULL;

  if (pszHome) {
    const char szFileBase[] = ".fftw3-wisdom";
    int nHome = strlen(pszHome);
    int nBase = strlen(szFileBase);
    int len = nHome + nBase + 1;
    pszWisdom = new char [ len + 1 ];
    strcpy (pszWisdom, pszHome);
    pszWisdom[nHome] = '/';
    strcpy(pszWisdom+nHome+1,szFileBase);
    pszWisdom[nHome+nBase+2] = 0;

    FILE *wisdom = fopen(pszWisdom,"r");
    if (wisdom) {
      fftw_import_wisdom_from_file(wisdom);
      fclose(wisdom);
    }
  }
#endif

  int retval = ctsimtext_main(argc, argv);

#if defined(HAVE_FFTW) && defined(HAVE_GETENV)
  if (pszWisdom) {
    FILE* wisdom = fopen(pszWisdom,"w+");
    if (wisdom) {
      fftw_export_wisdom_to_file(wisdom);
      fclose(wisdom);
      delete [] pszWisdom;
    }
  }
#endif

  return retval;
}
