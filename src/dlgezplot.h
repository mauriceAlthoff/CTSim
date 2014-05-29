/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dlgezplot.h
**   Purpose:       Headers for EZPlot Dialog
**   Programmer:    Kevin Rosenberg
**   Date Started:  Jan 2001
**
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

#ifndef __DLGEZPLOT_H_
#define __DLGEZPLOT_H_

#include "wx/setup.h"
#include "wx/dialog.h"
#include "wx/dcmemory.h"

class wxButton;
class SGP;
class SGPDriver;
class EZPlot;


class EZPlotControl : public wxPanel
{
private:
  DECLARE_DYNAMIC_CLASS (EZPlotControl)
  DECLARE_EVENT_TABLE ()
  EZPlot* m_pEZPlot;
   int m_iClientX;   // size of client window
   int m_iClientY;

   SGPDriver* m_pSGPDriver;
   SGP* m_pSGP;
   wxDC* m_pDC;

public:
    EZPlotControl (wxWindow *parent, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxSTATIC_BORDER,
               const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = _T("EZPlotCtrl"));

   virtual ~EZPlotControl();

  virtual wxSize GetBestSize() const;

  EZPlot* getEZPlot()
  { return m_pEZPlot; }

  void OnPaint(wxPaintEvent& event);
};


class wxEZPlotDialog : public wxDialog
{
private:
   wxWindow *m_parentTop;     // parent top level window (may be NULL)
   EZPlotControl* m_pEZPlotCtrl;

public:
  wxEZPlotDialog (wxWindow *parent = NULL, bool bCancelButton = false);

   ~wxEZPlotDialog();

   EZPlot* getEZPlot ()
   { if (m_pEZPlotCtrl) return m_pEZPlotCtrl->getEZPlot(); else return NULL; }
};

class EZPlotDialog
{
private:
  wxEZPlotDialog* m_pDialog;

public:
  EZPlotDialog (wxWindow *parent = NULL, bool bCancelButton = false);

  EZPlot* getEZPlot ();
  int ShowModal();
};
#endif

