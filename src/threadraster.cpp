/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          threadraster.cpp
**   Purpose:       Threaded rasterizer class
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


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"


#ifdef HAVE_WXTHREADS


#include "threadraster.h"
#include "backgroundmgr.h"
#include "backgroundsupr.h"


/////////////////////////////////////////////////////////////////////
//
// Class RasterizerSupervisorThread -- Thread for Background Supervisor
//
/////////////////////////////////////////////////////////////////////

RasterizerSupervisorThread::RasterizerSupervisorThread (PhantomFileView* pProjView, int iNX, int iNY,
                                                        int iNSample, double dViewRatio, wxChar const* pszLabel)
                                                        :   SupervisorThread(), m_pPhantomView(pProjView), m_iNX(iNX), m_iNY(iNY), m_iNSample(iNSample), m_dViewRatio(dViewRatio), m_strLabel(pszLabel)
{
}

wxThread::ExitCode
RasterizerSupervisorThread::Entry()
{
  RasterizerSupervisor rasterSupervisor (this, m_pPhantomView, m_iNX, m_iNY, m_iNSample, m_dViewRatio, m_strLabel);

  rasterSupervisor.start();

  while (! rasterSupervisor.workersDone() && ! rasterSupervisor.fail() && ! rasterSupervisor.cancelled()) {
    Sleep(100);
  }

  if (rasterSupervisor.fail())
  {
    wxString msg (_T("Error starting Rasterizer supervisor: "));
    msg += rasterSupervisor.getFailMessage();
    msg += _T("\n");
    wxCommandEvent eventLog (wxEVT_COMMAND_MENU_SELECTED, MAINMENU_LOG_EVENT );
    eventLog.SetString( msg );
    wxPostEvent( theApp->getMainFrame(), eventLog ); // send log event
  }

  if (! rasterSupervisor.cancelled())
    rasterSupervisor.onDone();
  rasterSupervisor.deleteWorkers();

  return static_cast<wxThread::ExitCode>(0);
}

void
RasterizerSupervisorThread::OnExit()
{
}


/////////////////////////////////////////////////////////////////////
//
// Class RasterizerSupervisor -- A Background Supervisor
//
/////////////////////////////////////////////////////////////////////

RasterizerSupervisor::RasterizerSupervisor (SupervisorThread* pThread, PhantomFileView* pPhantomView, int iNX, int iNY,
                                            int iNSample, double dViewRatio, wxChar const* pszLabel)
  : BackgroundSupervisor (pThread, pPhantomView->GetFrame(), pPhantomView->GetDocument(), _T("Rasterizing"), iNX),
                                            m_pPhantomView(pPhantomView), m_pPhantomDoc(pPhantomView->GetDocument()),
                                            m_iNX(iNX), m_iNY(iNY), m_iNSample(iNSample), m_dViewRatio(dViewRatio), m_strLabel(pszLabel)
{
  m_vecpChildImageFiles.reserve (getNumWorkers());
  for (int iThread = 0; iThread < getNumWorkers(); iThread++) {
    m_vecpChildImageFiles[iThread] = new ImageFile;
  }
}

RasterizerSupervisor::~RasterizerSupervisor()
{
  for (int i = 0; i < getNumWorkers(); i++)
    delete m_vecpChildImageFiles[i];
}

BackgroundWorkerThread*
RasterizerSupervisor::createWorker (int iThread, int iStartUnit, int iNumUnits)
{
  RasterizerWorker* pThread = new RasterizerWorker (this, iThread, iStartUnit, iNumUnits);
  m_vecpChildImageFiles[iThread]->setArraySize (iNumUnits, m_iNY);
  pThread->SetParameters (m_pPhantomView, m_vecpChildImageFiles[iThread], m_iNX, m_iNY, m_iNSample, m_dViewRatio);

  return pThread;
}

void
RasterizerSupervisor::onDone()
{
  wxCriticalSection doneSection;
  wxCriticalSectionLocker critsect (doneSection);

  ImageFile* pImageFile = getImageFile();
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
RasterizerSupervisor::getImageFile()
{
  ImageFile* pImageFile = new ImageFile (m_iNX, m_iNY);

  size_t iColSize = sizeof(ImageFileValue) * m_iNY;

  ImageFileArray globalArray = pImageFile->getArray();
  int iGlobalCol = 0;
  for (int iw = 0; iw < getNumWorkers(); iw++) {
    ImageFileArray childArray = m_vecpChildImageFiles[iw]->getArray();
    for (unsigned int iCol = 0; iCol < m_vecpChildImageFiles[iw]->nx(); iCol++) {
      memcpy (globalArray[iGlobalCol++], childArray[iCol], iColSize);
    }
  }
  return (pImageFile);
}


/////////////////////////////////////////////////////////////////////
//
// Class RasterizerWorker -- A worker thread
//
/////////////////////////////////////////////////////////////////////

void
RasterizerWorker::SetParameters (PhantomFileView* pPhantomView, ImageFile* pImageFile, int iNX, int iNY, int iNSample, double dViewRatio)
{
  m_pImageFile = pImageFile;
  m_iNX = iNX;
  m_iNY = iNY;
  m_pPhantomView = pPhantomView;
  m_iNSample = iNSample;
  m_dViewRatio = dViewRatio;
}

wxThread::ExitCode
RasterizerWorker::Entry ()
{
  const Phantom& rPhantom = m_pPhantomView->GetDocument()->getPhantom();
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
    rPhantom.convertToImagefile (*m_pImageFile, m_iNX, m_dViewRatio, m_iNSample, Trace::TRACE_NONE, iUnit + m_iStartUnit, 1, iUnit);
    m_pSupervisor->onWorkerUnitTick();
  }

  m_pSupervisor->onWorkerDone (m_iThread);

  while (! TestDestroy())
    Sleep(100);

  return static_cast<wxThread::ExitCode>(0);
}

void
RasterizerWorker::OnExit ()
{
}

#endif // HAVE_WXTHREADS
