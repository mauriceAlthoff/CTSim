/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          backgroundmgr.cpp
**   Purpose:       Background manager class
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
#include "backgroundsupr.h"
#include "backgroundmgr.h"

#ifdef HAVE_WXTHREADS

int BackgroundManager::s_iNextButtonID = 0;

IMPLEMENT_DYNAMIC_CLASS(BackgroundManager, wxMiniFrame)
BEGIN_EVENT_TABLE(BackgroundManager, wxMiniFrame)
EVT_MENU(BackgroundSupervisor::MSG_BACKGROUND_SUPERVISOR_ADD, BackgroundManager::OnAddTask)
EVT_MENU(BackgroundSupervisor::MSG_BACKGROUND_SUPERVISOR_REMOVE, BackgroundManager::OnRemoveTask)
EVT_MENU(BackgroundSupervisor::MSG_BACKGROUND_SUPERVISOR_UNIT_TICK, BackgroundManager::OnUnitTick)
EVT_CLOSE(BackgroundManager::OnCloseWindow)
EVT_COMMAND_RANGE(0, 30000, wxEVT_COMMAND_BUTTON_CLICKED, BackgroundManager::OnCancelButton)
END_EVENT_TABLE()


BackgroundManager::BackgroundManager ()
  : wxMiniFrame (theApp->getMainFrame(), -1, _T("Background Tasks"), wxPoint(0,0), wxSize(210, 50))
{
  m_iNumTasks = 0;
  m_pCanvas = new BackgroundManagerCanvas (this);
  theApp->setIconForFrame (this);

  m_sizeGauge.Set (70, 20);
  m_sizeLabel.Set (140, 20);
  m_sizeBorder.Set (4, 4);
  m_sizeCellSpacing.Set (3, 3);
  m_sizeButton.Set (70, 20);

  m_sizeCell.Set (m_sizeGauge.x + m_sizeLabel.x + m_sizeCellSpacing.x + m_sizeButton.x, 25);

  theApp->getMainFrame()->SetFocus();
  Show(false);
}


BackgroundManager::~BackgroundManager()
{
}

void
BackgroundManager::OnCloseWindow (wxCloseEvent& event)
{
  if (theApp->getMainFrame()->getShuttingDown())
    wxMiniFrame::OnCloseWindow (event);
  else
    event.Veto();
}

void
BackgroundManager::OnUnitTick (wxCommandEvent& event)
{
  int iUnits = event.GetInt();

  BackgroundSupervisor* pSupervisor = reinterpret_cast<BackgroundSupervisor*>(event.GetClientData());
  if (pSupervisor == NULL) {
    sys_error (ERR_SEVERE, "Received NULL task [BackgroundManager::OnUnitTick]");
    return;
  }

  BackgroundManagerTask* pTask = lookupTask (pSupervisor);
  if (pTask == NULL) {
          sys_error (ERR_SEVERE, "Error looking up task [BackgroundManager::OnUnitTick]");
          return;
  }
  pTask->gauge()->SetValue (iUnits);
}

void
BackgroundManager::OnAddTask (wxCommandEvent& event)
{
  int iNumUnits = event.GetInt();
  const char* const pszTaskName = event.GetString().mb_str(wxConvUTF8);
  BackgroundSupervisor* pSupervisor = reinterpret_cast<BackgroundSupervisor*>(event.GetClientData());
  if (pSupervisor == NULL) {
    sys_error (ERR_SEVERE, "Received NULL supervisor [BackgroundManager::OnAddTask]");
    return;
  }

  wxCriticalSectionLocker locker (m_criticalSection);

  int iNumTasks = m_vecpTasks.size();
  std::vector<bool> vecPositionUsed (iNumTasks);  //vector of used table positions
  for (int iP = 0; iP < iNumTasks; iP++)
    vecPositionUsed[iP] = false;

  for (TaskContainer::iterator iT = m_vecpTasks.begin(); iT != m_vecpTasks.end(); iT++) {
    int iPosUsed = (*iT)->position();
    if (iPosUsed < iNumTasks)
      vecPositionUsed[iPosUsed] = true;
  }

  int iFirstUnusedPos = iNumTasks;  // default is just past current number of tasks
  for (int i = 0; i < iNumTasks; i++)
    if (! vecPositionUsed[i]) {
      iFirstUnusedPos = i;
      break;
    }

  wxPoint posGauge (m_sizeBorder.x, m_sizeBorder.y + iFirstUnusedPos * m_sizeCell.y);
  wxPoint posLabel (m_sizeBorder.x + m_sizeGauge.x, m_sizeBorder.y + iFirstUnusedPos * m_sizeCell.y);
  wxPoint posButton (m_sizeBorder.x + m_sizeGauge.x + m_sizeLabel.x, m_sizeBorder.y + iFirstUnusedPos * m_sizeCell.y);
  wxGauge* pGauge = new wxGauge (m_pCanvas, -1, iNumUnits, posGauge, m_sizeGauge);
  wxStaticText* pLabel = new wxStaticText (m_pCanvas, -1, wxConvUTF8.cMB2WX(pszTaskName), posLabel, m_sizeLabel);
  wxButton* pButton = new wxButton (m_pCanvas, s_iNextButtonID, _T("Cancel"), posButton, m_sizeButton, wxBU_LEFT);

  BackgroundManagerTask* pTask = new BackgroundManagerTask (pSupervisor, pszTaskName,
    iFirstUnusedPos, pGauge, pLabel, pButton, s_iNextButtonID);

  m_vecpTasks.push_back (pTask);
  m_iNumTasks++;
  s_iNextButtonID++;

  resizeWindow();
  if (m_iNumTasks == 1) {
    m_pCanvas->SetFocus();
    Show(true);
  }
}

void
BackgroundManager::OnRemoveTask (wxCommandEvent& event)
{
  BackgroundSupervisor* pSupervisor = reinterpret_cast<BackgroundSupervisor*>(event.GetClientData());
  if (pSupervisor == NULL) {
    sys_error (ERR_SEVERE, "Received NULL task [BackgroundManager::OnRemoveTask]");
    return;
  }

  wxCriticalSectionLocker locker (m_criticalSection);

  bool bFound = false;
  for (TaskContainer::iterator iTask = m_vecpTasks.begin(); iTask != m_vecpTasks.end(); iTask++) {
    if ((*iTask)->supervisor() == pSupervisor) {
          delete (*iTask)->gauge();
            delete (*iTask)->label();
            delete (*iTask)->button();
      delete *iTask;
      m_vecpTasks.erase (iTask);
      m_iNumTasks--;
            bFound = true;
      break;
    }
  }
  if (! bFound)  {
          sys_error (ERR_SEVERE, "Unable to find supervisor [BackgroundManager::OnRemoveTask]");
    return;
  }
  pSupervisor->ackRemoveBackgroundManager();
  resizeWindow();
  if (m_iNumTasks <= 0) {
    m_pCanvas->SetFocus();
    Show(false);
  }
}

void
BackgroundManager::OnCancelButton (wxCommandEvent& event)
{
  BackgroundManagerTask* pTask = lookupTask (event.GetId());
  if (! pTask) {
    sys_error (ERR_SEVERE, "Unable to lookup task for button");
    return;
  }

  pTask->supervisor()->onCancel();
}

BackgroundManagerTask*
BackgroundManager::lookupTask (BackgroundSupervisor* pSupervisor)
{
  BackgroundManagerTask* pTask = NULL;

  wxCriticalSectionLocker locker (m_criticalSection);
  for (TaskContainer::iterator iTask = m_vecpTasks.begin(); iTask != m_vecpTasks.end(); iTask++) {
    if ((*iTask)->supervisor() == pSupervisor) {
      pTask = *iTask;
      break;
    }
  }

  return pTask;
}

BackgroundManagerTask*
BackgroundManager::lookupTask (int iButtonID)
{
  BackgroundManagerTask* pTask = NULL;

  wxCriticalSectionLocker locker (m_criticalSection);
  for (TaskContainer::iterator iTask = m_vecpTasks.begin(); iTask != m_vecpTasks.end(); iTask++) {
    if ((*iTask)->buttonID() == iButtonID) {
      pTask = *iTask;
      break;
    }
  }

  return pTask;
}

void
BackgroundManager::resizeWindow()
{
  int iHighestPosition = -1;

  wxCriticalSectionLocker lock (m_criticalSection);
  for (TaskContainer::iterator i = m_vecpTasks.begin(); i != m_vecpTasks.end(); i++)
    if (iHighestPosition < (*i)->position())
      iHighestPosition = (*i)->position();

  wxSize sizeWindow (m_sizeCell.x, m_sizeCell.y * (iHighestPosition + 1));
  SetClientSize (sizeWindow);
  m_pCanvas->Refresh();
}



IMPLEMENT_DYNAMIC_CLASS(BackgroundManagerCanvas, wxPanel)
BEGIN_EVENT_TABLE(BackgroundManagerCanvas, wxPanel)
END_EVENT_TABLE()

BackgroundManagerCanvas::BackgroundManagerCanvas (BackgroundManager* pMgr)
: wxPanel (pMgr), m_pBackgroundManager(pMgr)
{
}


#endif // HAVE_WXTHREADS
