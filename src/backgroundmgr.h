/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          backgroundmgr.h
**   Purpose:       Header file for background manager
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

#ifndef _BACKGROUND_MGR_H


#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/minifram.h"
#include "wx/gauge.h"

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "threadrecon.h"

#include <list>

#ifdef HAVE_WXTHREADS

class BackgroundManagerCanvas;
class BackgroundManagerTask;


class BackgroundManager : public wxMiniFrame
{
private:

  DECLARE_DYNAMIC_CLASS(BackgroundManager)

  wxCriticalSection m_criticalSection;
  BackgroundManagerCanvas* m_pCanvas;
  int m_iNumTasks;

  typedef std::list<BackgroundManagerTask*> TaskContainer;
  TaskContainer m_vecpTasks;

  void resizeWindow();
  BackgroundManagerTask* lookupTask (BackgroundSupervisor* pSupervisor);
  BackgroundManagerTask* lookupTask (int iButtonID);

  static int s_iNextButtonID;

  wxSize m_sizeGauge;
  wxSize m_sizeLabel;
  wxSize m_sizeCell;
  wxSize m_sizeBorder;
  wxSize m_sizeCellSpacing;
  wxSize m_sizeButton;

public:
  BackgroundManager ();
  ~BackgroundManager();

  void OnAddTask (wxCommandEvent& event);
  void OnRemoveTask (wxCommandEvent& event);
  void OnUnitTick (wxCommandEvent& event);
  void OnCloseWindow(wxCloseEvent& event);
  void OnCancelButton(wxCommandEvent& event);

  DECLARE_EVENT_TABLE()
};


class BackgroundSupervisor;
class BackgroundManagerTask {
private:
  BackgroundSupervisor* m_pSupervisor;
  const std::string m_strName;
  const int m_iPosition;
  wxGauge* m_pGauge;
  wxStaticText* m_pLabel;
  wxButton* m_pButton;
  const int m_iButtonID;

public:
  BackgroundManagerTask (BackgroundSupervisor* pSupervisor, const char* const pszName, int iPos,
          wxGauge* pGauge, wxStaticText* pLabel, wxButton* pButton, int iButtonID)
          : m_pSupervisor(pSupervisor), m_strName(pszName), m_iPosition(iPos), m_pGauge(pGauge),
    m_pLabel(pLabel), m_pButton(pButton), m_iButtonID(iButtonID)
  {}

  int position() const {return m_iPosition;}
  const std::string& name() const {return m_strName;}
  BackgroundSupervisor* supervisor() {return m_pSupervisor;}

  wxGauge* gauge() {return m_pGauge;}
  wxStaticText* label() {return m_pLabel;}
  wxButton* button() {return m_pButton;}
  int buttonID() const {return m_iButtonID;}
};


class BackgroundManagerCanvas : public wxPanel {
private:
  DECLARE_DYNAMIC_CLASS(BackgroundManagerCanvas)
  BackgroundManager* m_pBackgroundManager;

public:
  BackgroundManagerCanvas (BackgroundManager* pBkgdMgr = NULL);

  DECLARE_EVENT_TABLE()
};

#endif // HAVE_WXTHREADS

#endif // _BACKGROUNDMGR_H

