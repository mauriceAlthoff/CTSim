/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          BackgroundSupr.cpp
**   Purpose:       Background Supervisor classes
**   Programmer:    Kevin Rosenberg
**   Date Started:  February 2001
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// pragma line required for Fedora 4 and wxWin 2.4.2
#pragma implementation "timer.h"

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "backgroundsupr.h"
#include "backgroundmgr.h"

#ifdef HAVE_WXTHREADS

#define USE_BKGMGR 1

////////////////////////////////////////////////////////////////////////////
//
// Class BackgroundSupervisor -- An event handler run by a SupervisorThread
//
////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(BackgroundSupervisor, wxEvtHandler)
BEGIN_EVENT_TABLE(BackgroundSupervisor, BackgroundSupervisor)
END_EVENT_TABLE()



BackgroundSupervisor::BackgroundSupervisor (SupervisorThread* pMyThread, wxWindow* pParentFrame, BackgroundProcessingDocument* pDocument, wxChar const* pszProcessTitle, int iTotalUnits)
    : wxEvtHandler(), m_pMyThread(pMyThread), m_pParentFrame(pParentFrame), m_pDocument(pDocument), m_strProcessTitle(pszProcessTitle),
    m_iTotalUnits(iTotalUnits), m_iNumThreads(0), m_bDone(false), m_bFail(false), m_bCancelled(false), m_iRunning(0),
    m_pTimer(NULL), m_bWorkersDeleted(false), m_bBackgroundManagerAdded(false)
{
  m_iNumThreads = theApp->getNumberCPU();
  //   ++m_iNumThreads;

  m_vecpThreads.resize (m_iNumThreads);
  for (int iThread = 0; iThread < m_iNumThreads; iThread++)
    m_vecpThreads[iThread] = NULL;

}

BackgroundSupervisor::~BackgroundSupervisor()
{
  m_pDocument->removeBackgroundSupervisor (this);

  delete m_pTimer;
}

void
BackgroundSupervisor::deleteWorkers()
{
  wxCriticalSectionLocker lock (m_critsectThreads);
  if (m_bWorkersDeleted)
    return;

  for (int i = 0; i < m_iNumThreads; i++)
    if (m_vecpThreads[i])
      m_vecpThreads[i]->Delete(); // send Destroy message to workers

#ifdef USE_BKGMGR
  wxCommandEvent doneEvent (wxEVT_COMMAND_MENU_SELECTED, MSG_BACKGROUND_SUPERVISOR_REMOVE);
  doneEvent.SetClientData (this);
  wxPostEvent (theApp->getBackgroundManager(), doneEvent);
#endif

  while (m_iRunning > 0 || m_bBackgroundManagerAdded)
    m_pMyThread->Sleep(50);

  m_bWorkersDeleted = true;
}

void
BackgroundSupervisor::ackRemoveBackgroundManager()
{
  m_bBackgroundManagerAdded = false;
}

bool
BackgroundSupervisor::start()
{
  int iBaseUnits = m_iTotalUnits / m_iNumThreads;
  int iExtraUnits = m_iTotalUnits % m_iNumThreads;
  int iStartUnit = 0;
  for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
    int iNumUnits = iBaseUnits;
    if (iThread < iExtraUnits)
      ++iNumUnits;
    m_vecpThreads[iThread] = createWorker (iThread, iStartUnit, iNumUnits);
    if (! m_vecpThreads[iThread]) {
      m_bFail = true;
      m_strFailMessage = _T("createWorker returned NULL [BackgroundSupervisor]");
      break;
    }
    if (m_vecpThreads[iThread]->Create () != wxTHREAD_NO_ERROR) {
      m_bFail = true;
      m_strFailMessage = _T("Thread creation failed [BackgroundSupervisor]");
      break;
    }
   m_vecpThreads[iThread]->SetPriority (40);
   iStartUnit += iNumUnits;
  }
  if (m_bFail)
    return false;

  m_pTimer = new Timer;

  wxString strLabel (m_strProcessTitle);
  strLabel += _T(" ");
  strLabel += dynamic_cast<wxFrame*>(m_pParentFrame)->GetTitle();

#ifdef USE_BKGMGR
  wxCommandEvent addTaskEvent (wxEVT_COMMAND_MENU_SELECTED, MSG_BACKGROUND_SUPERVISOR_ADD);
  addTaskEvent.SetString (strLabel);
  addTaskEvent.SetInt (m_iTotalUnits);
  addTaskEvent.SetClientData (this);
  wxPostEvent (theApp->getBackgroundManager(), addTaskEvent);
#endif

  m_pDocument->addBackgroundSupervisor (this);
  m_bBackgroundManagerAdded = true;

  m_iRunning = m_iNumThreads;
  m_iUnitsDone = 0;

  for (int i = 0; i < m_iNumThreads; i++)
    m_vecpThreads[i]->Run();

  return true;
}

void
BackgroundSupervisor::onCancel()
{
  m_bCancelled = true;
  m_bDone = true;
}


void
BackgroundSupervisor::onWorkerUnitTick ()
{
    ++m_iUnitsDone;

#ifdef USE_BKGMGR
    wxCommandEvent addTaskEvent (wxEVT_COMMAND_MENU_SELECTED, MSG_BACKGROUND_SUPERVISOR_UNIT_TICK);
    addTaskEvent.SetInt (m_iUnitsDone - 1);
    addTaskEvent.SetClientData (this);
    wxPostEvent (theApp->getBackgroundManager(), addTaskEvent);
#endif
}

void
BackgroundSupervisor::onWorkerDone (int iThread)
{
        wxCriticalSection critsectDone;
        critsectDone.Enter();

  m_iRunning--;

#ifdef DEBUG
  if (theApp->getVerboseLogging()) {
    wxString msg;
    msg.Printf(_T("Background Supervisor: Thread finished. Remaining threads: %d\n"), m_iRunning);
    wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
    eventLog.SetString( msg );
    wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event
  }
#endif

  critsectDone.Leave();
}

void
BackgroundSupervisor::onWorkerFail (int iThread, const wxString& strFailMessage)
{
  m_iRunning--;
  wxCommandEvent eventLog( wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
  eventLog.SetString( strFailMessage );
  wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event

  onCancel();
}

#endif // HAVE_WXTHREADS
