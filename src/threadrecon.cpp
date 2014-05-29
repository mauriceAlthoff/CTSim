/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          threadrecon.cpp
**   Purpose:       Threaded reconstruction class
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

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "threadrecon.h"
#include "backgroundmgr.h"
#include "backgroundsupr.h"

#ifdef HAVE_WXTHREADS




/////////////////////////////////////////////////////////////////////
//
// Class ReconstructorSupervisorThread -- Thread for Background Supervisor
//
/////////////////////////////////////////////////////////////////////

ReconstructorSupervisorThread::ReconstructorSupervisorThread (ProjectionFileView* pProjView, int iNX, int iNY,
   const char* pszFilterName, double dFilterParam, const char* pszFilterMethod, int iZeropad,
   const char* pszFilterGenerationName, const char* pszInterpName, int iInterpParam,
   const char* pszBackprojectName, wxChar const* pszLabel, ReconstructionROI* pROI, bool bRebinToParallel)
:   SupervisorThread(), m_pProjView(pProjView), m_iNX(iNX), m_iNY(iNY), m_strFilterName(pszFilterName), m_dFilterParam(dFilterParam),
  m_strFilterMethod(pszFilterMethod), m_iZeropad(iZeropad), m_strFilterGenerationName(pszFilterGenerationName),
  m_strInterpName(pszInterpName), m_iInterpParam(iInterpParam), m_strBackprojectName(pszBackprojectName),
  m_strLabel(pszLabel), m_reconROI(*pROI), m_bRebinToParallel(bRebinToParallel)
{
}

wxThread::ExitCode
ReconstructorSupervisorThread::Entry()
{
  Projections* pProj = &m_pProjView->GetDocument()->getProjections();

  if (m_bRebinToParallel)
    pProj = pProj->interpolateToParallel();

  ReconstructorSupervisor reconSupervisor (this, pProj, m_pProjView, m_iNX, m_iNY,
   m_strFilterName.c_str(), m_dFilterParam, m_strFilterMethod.c_str(), m_iZeropad, m_strFilterGenerationName.c_str(),
   m_strInterpName.c_str(), m_iInterpParam, m_strBackprojectName.c_str(), m_strLabel, &m_reconROI);

  reconSupervisor.start();
  while (! reconSupervisor.workersDone() && ! reconSupervisor.fail() && ! reconSupervisor.cancelled()) {
    Sleep(100);
  }
  if (reconSupervisor.fail())
  {
    wxString msg (_T("Error starting reconstructor supervisor: "));
    msg += reconSupervisor.getFailMessage();
    msg += _T("\n");
    wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
    eventLog.SetString( msg );
    wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event
  }
  if (! reconSupervisor.cancelled())
          reconSupervisor.onDone();
  reconSupervisor.deleteWorkers();

  if (m_bRebinToParallel)
    delete pProj;

  return static_cast<wxThread::ExitCode>(0);
}

void
ReconstructorSupervisorThread::OnExit()
{
}


/////////////////////////////////////////////////////////////////////
//
// Class ReconstructorSupervisor -- A Background Supervisor
//
/////////////////////////////////////////////////////////////////////

ReconstructorSupervisor::ReconstructorSupervisor (SupervisorThread* pThread, Projections* pProj,
  ProjectionFileView* pProjView, int iImageNX, int iImageNY, const char* pszFilterName, double dFilterParam,
  const char* pszFilterMethod, int iZeropad, const char* pszFilterGenerationName,
  const char* pszInterpName, int iInterpParam, const char* pszBackprojectName, wxChar const* pszLabel,
  ReconstructionROI* pROI)
    : BackgroundSupervisor (pThread, pProjView->GetFrame(), pProjView->GetDocument(),
                            _T("Reconstructing"), 
                            pProjView->GetDocument()->getProjections().nView()),
      m_pProj(pProj), m_pProjView(pProjView), m_pProjDoc(pProjView->GetDocument()),
      m_iImageNX(iImageNX), m_iImageNY(iImageNY),
      m_pszFilterName(pszFilterName), m_dFilterParam(dFilterParam), m_pszFilterMethod(pszFilterMethod),
      m_iZeropad(iZeropad), m_pszFilterGenerationName(pszFilterGenerationName), m_pszInterpName(pszInterpName),
      m_iInterpParam(iInterpParam), m_pszBackprojectName(pszBackprojectName), m_strLabel(pszLabel),
      m_pReconROI(pROI)
{
  m_vecpChildImageFile.reserve (getNumWorkers());
  for (int iThread = 0; iThread < getNumWorkers(); iThread++) {
    m_vecpChildImageFile[iThread] = new ImageFile (m_iImageNX, m_iImageNY);
  }

}

ReconstructorSupervisor::~ReconstructorSupervisor()
{
  for (int i = 0; i < getNumWorkers(); i++) {
      delete m_vecpChildImageFile[i];
      m_vecpChildImageFile[i] = NULL;
    }
}

BackgroundWorkerThread*
ReconstructorSupervisor::createWorker (int iThread, int iStartUnit, int iNumUnits)
{
   ReconstructorWorker* pThread = new ReconstructorWorker (this, iThread, iStartUnit, iNumUnits);
   pThread->SetParameters (m_pProj, m_pProjView, m_vecpChildImageFile[iThread], m_pszFilterName,
     m_dFilterParam, m_pszFilterMethod, m_iZeropad, m_pszFilterGenerationName, m_pszInterpName,
     m_iInterpParam, m_pszBackprojectName, m_pReconROI);

   return pThread;
}

void
ReconstructorSupervisor::onDone()
{
  wxCriticalSection doneSection;
  wxCriticalSectionLocker critsect (doneSection);

  ImageFile* pImageFile = getImageFile();
  pImageFile->labelAdd (m_pProj->getLabel());
  pImageFile->labelAdd (m_strLabel.mb_str(wxConvUTF8), getTimerEnd());

  wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
  wxString msg (m_strLabel);
  msg += _T("\n");
  eventLog.SetString( msg );
  wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event

  wxCommandEvent newImageEvent (wxEVT_COMMAND_MENU_SELECTED, NEW_IMAGEFILE_EVENT);
  newImageEvent.SetClientData (pImageFile);
  wxPostEvent (theApp->getMainFrame(), newImageEvent);

  setDone();
}


ImageFile*
ReconstructorSupervisor::getImageFile()
{
  ImageFile* pImageFile = new ImageFile (m_iImageNX, m_iImageNY);
  pImageFile->arrayDataClear();
  ImageFileArray pArray = pImageFile->getArray();

  int i;
  for (i = 0; i < getNumWorkers(); i++) {
    ImageFileArrayConst pChildArray = m_vecpChildImageFile[i]->getArray();
    for (int ix = 0; ix < m_iImageNX; ix++)
      for (int iy = 0; iy < m_iImageNY; iy++)
        pArray[ix][iy] += pChildArray[ix][iy];
  }

  return (pImageFile);
}


/////////////////////////////////////////////////////////////////////
//
// Class ReconstructorWorker -- A worker thread
//
/////////////////////////////////////////////////////////////////////

void
ReconstructorWorker::SetParameters (const Projections* pProj, ProjectionFileView* pProjView, ImageFile* pImageFile,
 const char* pszFilterName, double dFilterParam, const char* pszFilterMethod, int iZeropad,
 const char* pszFilterGenerationName, const char* pszInterpName, int iInterpParam,
 const char* pszBackprojectName, ReconstructionROI* pROI)
{
   m_pProj = pProj;
   m_pProjView = pProjView;
   m_pImageFile = pImageFile;
   m_pszFilterName = pszFilterName;
   m_dFilterParam = dFilterParam;
   m_pszFilterMethod = pszFilterMethod;
   m_iZeropad = iZeropad;
   m_pszFilterGenerationName = pszFilterGenerationName;
   m_pszInterpName = pszInterpName;
   m_iInterpParam = iInterpParam;
   m_pszBackprojectName = pszBackprojectName;
   m_pReconROI = pROI;
}

wxThread::ExitCode
ReconstructorWorker::Entry ()
{
  Reconstructor* pReconstructor = new Reconstructor (*m_pProj, *m_pImageFile, m_pszFilterName,
    m_dFilterParam, m_pszFilterMethod, m_iZeropad, m_pszFilterGenerationName, m_pszInterpName,
    m_iInterpParam, m_pszBackprojectName, Trace::TRACE_NONE, m_pReconROI, false);

  bool bFail = pReconstructor->fail();
  wxString failMsg;
  if (bFail) {
    failMsg = _T("Unable to make reconstructor: ");
    failMsg += wxConvUTF8.cMB2WX(pReconstructor->failMessage().c_str());
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
      pReconstructor->reconstructView (iUnit + m_iStartUnit, 1);
      m_pSupervisor->onWorkerUnitTick();
    }
    pReconstructor->postProcessing();
  }
  delete pReconstructor;

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
ReconstructorWorker::OnExit ()
{
}

#endif // HAVE_WXTHREADS
