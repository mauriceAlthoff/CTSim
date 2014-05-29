/*****************************************************************************
**  This is part of the CTSim program
**  Copyright (c) 1983-2009 Kevin Rosenberg
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

#include <iostream>
#include <exception>
#include <stdexcept>
#include <stdarg.h>
#include <ctype.h>
#include <string>
#include "ct.h"

#ifdef HAVE_WXWINDOWS
#include "../src/ctsim.h"
#include <wx/log.h>
#endif

/* NAME
*   sys_error                   System error handler
*
* SYNOPSIS
*   sys_error (severity, msg, args . . .)
*   int severity                Severity of error
*   char *msg                   Error message
*   args                        Argument list, direct transfer to printf stack
*/

static int s_reportErrorLevel = ERR_TRACE;      // Set error reporting level


void sys_error (int severity, const char *msg, ...)
{
  va_list arg;

  va_start(arg, msg);

  std::string strOutput;
  sys_verror (strOutput, severity, msg, arg);

#ifdef HAVE_WXWINDOWS
  if (g_bRunningWXWindows) {
    if (theApp) {
      wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
      wxString msg (wxConvCurrent->cMB2WX(strOutput.c_str()));
      if (msg.length() > 0) {
        msg += wxChar('\n');
        eventLog.SetString( msg );
        wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event, thread safe
      }
    } else {
      wxMutexGuiEnter();
      wxLog::OnLog (wxLOG_Message, wxConvCurrent->cMB2WX(strOutput.c_str()), time(NULL));
      wxMutexGuiLeave();
    }
  }
  else
    std::cout << strOutput << "\n";
#else
    std::cout << strOutput << "\n";
#endif

  va_end(arg);
}

static unsigned long s_nErrorCount = 0;
unsigned long int g_lSysErrorMaxCount = 2000;


void sys_verror (std::string& strOutput, int severity, const char *msg, va_list arg)
{
  if (severity < s_reportErrorLevel)
    return;     // ignore error if less than reporting level

  std::ostringstream os;

  if (severity > ERR_TRACE)
    s_nErrorCount++;

  if (severity != ERR_FATAL) {
    if (s_nErrorCount > g_lSysErrorMaxCount)
      return;
    else if (s_nErrorCount == g_lSysErrorMaxCount) {
      os << "*****************************************************************\n";
      os << "***   M A X I M U M   E R R O R   C O U N T   R E A C H E D   ***\n";
      os << "***                                                           ***\n";
      os << "***           No further errors will be reported              ***\n";
      os << "*****************************************************************\n";
      strOutput = os.str();
      return;
    }
  }

  switch (severity) {
  case ERR_FATAL:
    os << "FATAL ERROR: ";
    break;
  case ERR_SEVERE:
    os << "SEVERE ERROR: ";
    break;
  case ERR_WARNING:
    os << "WARNING ERROR: ";
    break;
  case ERR_TRACE:
    os << "Trace: ";
    break;
  default:
    os << "Illegal error severity #" << severity << ": ";
  }

  char errStr[2000];

#if HAVE_VSNPRINTF
  vsnprintf (errStr, sizeof(errStr), msg, arg);
#elif HAVE_VSPRINTF
  vsprintf (errStr, msg, arg);
#else
  strncpy (errStr, sizeof(errStr), "Error message not available on this platform.");
#endif

  os << errStr;
  strOutput = os.str();

  if (severity == ERR_FATAL) {
    std::cerr << strOutput << "\n";
    throw std::runtime_error (strOutput);
  }

#if INTERACTIVE_ERROR_DISPLAY
  std::cout << "A - Abort  C - Continue  W - Turn off warnings? ";
  //  fflush(stderr);
  do
  {
    int c = cio_kb_waitc("AaBbCcWw", TRUE);       /* get code from keyboard */
    c = tolower (c);
    fputc (c, stderr);
    fputc (NEWLINE, stderr);

    if (c == 'a')
      exit (1);
    else if (c == 'c')
      return;
    else if (c == 'w')
    {
      sys_error_level (ERR_SEVERE);     /* report severe & fatal errors */
      break;
    }
  } while (TRUE);
#endif
}


/* NAME
*   sys_error_level                     Set error reporting level
*
* SYNOPSIS
*   sys_error_level (severity)
*   int severity                Report all error as serious as severity and beyond
*
* DESCRIPTION
*   Causes the system to ignore all error below the level of severity
*   For example, if severity == ERR_SEVERE, then report severe & fatal
*   error and ignore warnings
*/

void
sys_error_level (int severity)
{
  if (severity == ERR_FATAL ||
    severity == ERR_SEVERE ||
    severity == ERR_WARNING ||
    severity == ERR_TRACE)
    s_reportErrorLevel = severity;
  else
    s_reportErrorLevel = ERR_WARNING;
}

