/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          backgroundsupr.h
**   Purpose:       Header file for background supervisors
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

#ifndef _BACKGROUNDSUPR_H
#define _BACKGROUNDSUPR_H

#include <vector>
#include <wx/thread.h>
#include <wx/progdlg.h>
#include "timer.h"

// This thread creates a SupervisorTask event handler object
// The thread is detached and terminates when SupervisorTask terminates
class SupervisorThread : public wxThread {
private:

public:
  SupervisorThread()
    : wxThread(wxTHREAD_DETACHED)
  {}

};


// Pure virtual class for BackgroundSupervisor that can communication with BackgroundManager via messages
class BackgroundWorkerThread;
class BackgroundProcessingDocument;

class BackgroundSupervisor : public wxEvtHandler {
private:
  DECLARE_DYNAMIC_CLASS(BackgroundSupervisor)

  SupervisorThread* m_pMyThread;
  wxWindow* m_pParentFrame;
  BackgroundProcessingDocument* m_pDocument;
  const wxString m_strProcessTitle;

  const unsigned int m_iTotalUnits;
  int m_iNumThreads;
  volatile bool m_bDone;
  volatile bool m_bFail;
  wxString m_strFailMessage;
  volatile bool m_bCancelled;
  volatile int m_iRunning;
  volatile unsigned int m_iUnitsDone;
  Timer* m_pTimer;
  volatile bool m_bWorkersDeleted;
  volatile bool m_bBackgroundManagerAdded;

  typedef std::vector<BackgroundWorkerThread*> ThreadContainer;
  ThreadContainer m_vecpThreads;
  wxCriticalSection m_critsectThreads;

public:
  enum {
    MSG_BACKGROUND_SUPERVISOR_ADD = 7500, // sends to BackgroundManager and Document
    MSG_BACKGROUND_SUPERVISOR_REMOVE = 7501, // sends to BackgroundManager and Document
    MSG_BACKGROUND_SUPERVISOR_UNIT_TICK = 7502,  // sends to BackgroundManager for progress bars
    MSG_BACKGROUND_SUPERVISOR_CANCEL = 7503,   // *sent* to Supervisor to cancel process
    MSG_DOCUMENT_ACK_REMOVE = 7504,

    MSG_WORKER_THREAD_UNIT_TICK = 7505,
    MSG_WORKER_THREAD_DONE = 7506,
    MSG_WORKER_THREAD_FAIL = 7507,   // sent by workers when they fail
  };

  BackgroundSupervisor (SupervisorThread* pMyThread, wxWindow* pParentFrame, BackgroundProcessingDocument* pDocument, wxChar const* pszProcessTitle,
    int iTotalUnits);

  BackgroundSupervisor ()
    : wxEvtHandler(), m_iTotalUnits(0)
  {}

  virtual ~BackgroundSupervisor();

  virtual BackgroundWorkerThread* createWorker (int iThread, int iStartUnit, int iNumUnits)
  { return NULL; }

  bool start();
  virtual void onDone() {};

  virtual void onCancel();

  virtual void onWorkerFail(int iThread, const wxString& strFailMessage);
  virtual void onWorkerUnitTick();
  virtual void onWorkerDone(int iThread);

  void deleteWorkers();
  void ackRemoveBackgroundManager();
  bool workersDone() const { return m_iRunning == 0; }
  bool workersDeleted() const { return m_bWorkersDeleted; }
  bool isDone() const {return m_bDone;}
  void setDone() { m_bDone = true; }
  bool fail() const {return m_bFail;}
  const wxString& getFailMessage() const { return m_strFailMessage; }
  bool cancelled() const {return m_bCancelled;}

  int getNumWorkers() const { return m_iNumThreads; }
  double getTimerEnd() { return m_pTimer->timerEnd(); }

  DECLARE_EVENT_TABLE()
};


class BackgroundWorkerThread : public wxThread {
protected:
  BackgroundSupervisor* m_pSupervisor;
  const int m_iThread;
  const int m_iStartUnit;
  const int m_iNumUnits;

public:
  BackgroundWorkerThread (BackgroundSupervisor* pSupervisor, int iThread, int iStartUnit, int iNumUnits)
    : wxThread (wxTHREAD_DETACHED), m_pSupervisor(pSupervisor), m_iThread(iThread), m_iStartUnit(iStartUnit), m_iNumUnits(iNumUnits)
  {}
};

#endif  // _BACKGROUNDSUPR_H_
