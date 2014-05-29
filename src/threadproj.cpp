/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          threadproj.cpp
**   Purpose:       Threaded projection class
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "threadproj.h"
#include "backgroundmgr.h"
#include "backgroundsupr.h"

#ifdef HAVE_WXTHREADS




/////////////////////////////////////////////////////////////////////
//
// Class ProjectorSupervisorThread -- Thread for Background Supervisor
//
/////////////////////////////////////////////////////////////////////

ProjectorSupervisorThread::ProjectorSupervisorThread (PhantomFileView* pProjView, int iNDet, int iNView, int iOffsetView,
   const char* pszGeometry, int iNSample, double dRotation, double dFocalLength, double dCenterDetectorLength,
   double dViewRatio, double dScanRatio, wxChar const* pszLabel)
: SupervisorThread(), m_pPhantomView(pProjView), m_iNDet(iNDet), m_iNView(iNView), m_iOffsetView(iOffsetView), m_strGeometry(pszGeometry),
  m_iNSample(iNSample), m_dRotation(dRotation), m_dFocalLength(dFocalLength), m_dCenterDetectorLength(dCenterDetectorLength),
  m_dViewRatio(dViewRatio), m_dScanRatio(dScanRatio), m_strLabel(pszLabel)
{
}

wxThread::ExitCode
ProjectorSupervisorThread::Entry()
{
  ProjectorSupervisor projSupervisor (this, m_pPhantomView, m_iNDet, m_iNView, m_iOffsetView,
   m_strGeometry.c_str(), m_iNSample, m_dRotation, m_dFocalLength, m_dCenterDetectorLength, m_dViewRatio, m_dScanRatio, m_strLabel);

  projSupervisor.start();
  while (! projSupervisor.workersDone() && ! projSupervisor.fail() && ! projSupervisor.cancelled()) {
    Sleep(100);
  }
  if (projSupervisor.fail())
  {
    wxString msg (_T("Error starting Projector supervisor: "));
    msg += projSupervisor.getFailMessage();
    msg += _T("\n");
    wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
    eventLog.SetString( msg );
    wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event
  }

  if (! projSupervisor.cancelled())
          projSupervisor.onDone();
  projSupervisor.deleteWorkers();

  return static_cast<wxThread::ExitCode>(0);
}

void
ProjectorSupervisorThread::OnExit()
{
}


/////////////////////////////////////////////////////////////////////
//
// Class ProjectorSupervisor -- A Background Supervisor
//
/////////////////////////////////////////////////////////////////////

ProjectorSupervisor::ProjectorSupervisor (SupervisorThread* pThread, PhantomFileView* pPhantomView, int iNDet, int iNView, int iOffsetView,
   const char* pszGeometry, int iNSample, double dRotation, double dFocalLength, double dCenterDetectorLength,
   double dViewRatio, double dScanRatio, wxChar const* pszLabel)
  : BackgroundSupervisor (pThread, pPhantomView->GetFrame(), pPhantomView->GetDocument(), _T("Projecting"), iNView),
      m_pPhantomView(pPhantomView), m_pPhantomDoc(pPhantomView->GetDocument()),
      m_iNDet(iNDet), m_iNView(iNView), m_iOffsetView(iOffsetView), m_pszGeometry(pszGeometry), m_iNSample(iNSample),
      m_dRotation(dRotation), m_dFocalLength(dFocalLength), m_dCenterDetectorLength(dCenterDetectorLength),
      m_dViewRatio(dViewRatio), m_dScanRatio(dScanRatio), m_strLabel(pszLabel)
{
  m_pScanner = new Scanner (m_pPhantomDoc->getPhantom(), m_pszGeometry, m_iNDet,
                  m_iNView, m_iOffsetView, m_iNSample, m_dRotation, m_dFocalLength, m_dCenterDetectorLength, m_dViewRatio, m_dScanRatio);

  m_vecpChildProjections.reserve (getNumWorkers());
  for (int iThread = 0; iThread < getNumWorkers(); iThread++) {
    m_vecpChildProjections[iThread] = new Projections (*m_pScanner);
  }



}

ProjectorSupervisor::~ProjectorSupervisor()
{
  for (int i = 0; i < getNumWorkers(); i++) {
      delete m_vecpChildProjections[i];
      m_vecpChildProjections[i] = NULL;
    }

  delete m_pScanner;
}

BackgroundWorkerThread*
ProjectorSupervisor::createWorker (int iThread, int iStartUnit, int iNumUnits)
{
   ProjectorWorker* pThread = new ProjectorWorker (this, iThread, iStartUnit, iNumUnits);
   m_vecpChildProjections[iThread]->setNView (iNumUnits);
   pThread->SetParameters (m_pPhantomView, m_vecpChildProjections[iThread], m_pScanner, m_iNDet, m_iNView, m_iOffsetView,
     m_pszGeometry, m_iNSample, m_dRotation, m_dFocalLength, m_dCenterDetectorLength, m_dViewRatio, m_dScanRatio);

   return pThread;
}

void
ProjectorSupervisor::onDone()
{
  wxCriticalSection doneSection;
  wxCriticalSectionLocker critsect (doneSection);

  Projections* pProjections = getProjections();
  pProjections->setRemark (m_strLabel.mb_str(wxConvUTF8));
  pProjections->setCalcTime (getTimerEnd());

  wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
  wxString msg (m_strLabel);
  msg += _T("\n");
  eventLog.SetString( msg );
  wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event

  wxCommandEvent newProjEvent (wxEVT_COMMAND_MENU_SELECTED, NEW_PROJECTIONFILE_EVENT);
  newProjEvent.SetClientData (pProjections);
  wxPostEvent (theApp->getMainFrame(), newProjEvent);

  setDone();
}


Projections*
ProjectorSupervisor::getProjections()
{
  Projections* pProjections = new Projections (*m_pScanner);

  int iGlobalView = 0;
  size_t detArraySize = pProjections->nDet() * sizeof (DetectorValue);
  for (int iw = 0; iw < getNumWorkers(); iw++) {
    for (int iView = 0; iView < m_vecpChildProjections[iw]->nView(); iView++) {
      DetectorArray& childDetArray = m_vecpChildProjections[iw]->getDetectorArray(iView);
      DetectorArray& globalDetArray = pProjections->getDetectorArray(iGlobalView++);
      globalDetArray.setViewAngle (childDetArray.viewAngle());
      DetectorValue* childDetval = childDetArray.detValues();
      DetectorValue* globalDetval = globalDetArray.detValues();
      memcpy (globalDetval, childDetval, detArraySize);
    }
  }

  return (pProjections);
}


/////////////////////////////////////////////////////////////////////
//
// Class ProjectorWorker -- A worker thread
//
/////////////////////////////////////////////////////////////////////

void
ProjectorWorker::SetParameters (PhantomFileView* pPhantomView, Projections* pProjections, Scanner* pScanner,
 int iNDet, int iView, int iOffsetView,
 const char* pszGeometry, int iNSample, double dRotation, double dFocalLength, double dCenterDetectorLength,
 double dViewRatio, double dScanRatio)
{
   m_pScanner = pScanner;
   m_pPhantomView = pPhantomView;
   m_pProjections = pProjections;
   m_pszGeometry = pszGeometry;
   m_iNSample = iNSample;
   m_dFocalLength = dFocalLength;
   m_dCenterDetectorLength = dCenterDetectorLength;
   m_dViewRatio = dViewRatio;
   m_dScanRatio = dScanRatio;
}

wxThread::ExitCode
ProjectorWorker::Entry ()
{
  const Phantom& rPhantom = m_pPhantomView->GetDocument()->getPhantom();
  bool bFail = m_pScanner->fail();
  wxString failMsg;
  if (bFail) {
    failMsg = _T("Unable to make Projector: ");
    failMsg += wxConvUTF8.cMB2WX(m_pScanner->failMessage().c_str());
      wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
    event.SetString( failMsg );
      wxPostEvent( theApp->getMainFrame(), event );
  }
  else
  {
    wxCommandEvent eventProgress (wxEVT_COMMAND_MENU_SELECTED, BackgroundSupervisor::MSG_WORKER_THREAD_UNIT_TICK);
    for (int iUnit = 0; iUnit < m_iNumUnits; iUnit++) {
      if (TestDestroy()) {
#ifdef DEBUG
        if (theApp->getVerboseLogging()) {
          wxString msg;
          msg.Printf(_T("Worker thread: Received destroy message at work unit #%d\n"), iUnit);
          wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
          event.SetString( msg );
          wxPostEvent( theApp->getMainFrame(), event );
        }
#endif
        break;
      }
      m_pScanner->collectProjections (*m_pProjections, rPhantom, iUnit + m_iStartUnit, 1,
        m_iOffsetView, iUnit, Trace::TRACE_NONE);
      m_pSupervisor->onWorkerUnitTick();
    }
  }

  if (bFail) {
    m_pSupervisor->onWorkerFail (m_iThread, failMsg);
  } else {
    m_pSupervisor->onWorkerDone (m_iThread);
  }

  while (! TestDestroy())
    Sleep(100);

  return reinterpret_cast<wxThread::ExitCode>(0);
}

void
ProjectorWorker::OnExit ()
{
}

#endif // HAVE_WXTHREADS
