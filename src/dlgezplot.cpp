/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dlgezplot.cpp
**   Purpose:       EZPlot Dialog
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_WXWINDOWS

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/utils.h"
#include "wx/frame.h"
#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/layout.h"
#include "wx/event.h"
#include "wx/intl.h"
#include "wx/settings.h"
#include "wx/dcclient.h"
#include "wx/timer.h"
#endif

#include "ct.h"
#include "../src/ctsim.h"
#include "dlgezplot.h"


static const int LAYOUT_X_MARGIN = 4;
static const int LAYOUT_Y_MARGIN = 4;

BEGIN_EVENT_TABLE(EZPlotControl, wxPanel)
EVT_PAINT(EZPlotControl::OnPaint)
END_EVENT_TABLE()

IMPLEMENT_CLASS(EZPlotControl, wxPanel)


EZPlotControl::EZPlotControl (wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                              long style, const wxValidator& validator, const wxString& name)
     : m_pEZPlot(0), m_pSGPDriver(0), m_pSGP(0), m_pDC(0)
{
  Create(parent, id, pos, size, style, name);

  SetSize (GetBestSize());

  m_pEZPlot = new EZPlot;
}

wxSize
EZPlotControl::GetBestSize () const
{
  return wxSize (500,500);
}

EZPlotControl::~EZPlotControl()
{
  delete m_pEZPlot;
  delete m_pSGP;
  delete m_pSGPDriver;
  delete m_pDC;
}

void
EZPlotControl::OnPaint (wxPaintEvent& event)
{
  wxPaintDC dc(this);
  GetClientSize (&m_iClientX, &m_iClientY);
  m_pSGPDriver = new SGPDriver (&dc, m_iClientX, m_iClientY);
  m_pSGP = new SGP (*m_pSGPDriver);
  m_pSGP->setTextPointSize(10);
//  m_pSGP->setViewport (0, 0, 1., 0.5);  // for debugging testing only
  if (m_pEZPlot && m_pSGP) {
    m_pSGP->eraseWindow();
    m_pEZPlot->plot (m_pSGP);
  }
}


wxEZPlotDialog::wxEZPlotDialog (wxWindow *parent, bool bCancelButton)
: wxDialog((parent ? parent : theApp->getMainFrame()), -1, _T("EZPlot"), wxDefaultPosition, wxDefaultSize, wxDIALOG_MODAL),
  m_parentTop(0)
{
  if (! parent)
    parent = theApp->getMainFrame();

  m_parentTop = parent;
  while ( m_parentTop && m_parentTop->GetParent() )
    m_parentTop = m_parentTop->GetParent();

  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (m_pEZPlotCtrl = new EZPlotControl (this), 0, wxALIGN_CENTER | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Ok"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  if (bCancelButton) {
    wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
    pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  }
  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}



EZPlotDialog::EZPlotDialog(wxWindow* parent, bool bCancelButton)
    : m_pDialog(new wxEZPlotDialog(parent, bCancelButton))
{
}

EZPlot*
EZPlotDialog::getEZPlot()
{ return m_pDialog->getEZPlot(); }

int
EZPlotDialog::ShowModal()
{ return m_pDialog->ShowModal(); }

wxEZPlotDialog::~wxEZPlotDialog()
{
  if ( m_parentTop )
    m_parentTop->Enable(TRUE);
}



#endif // HAVE_WXWINDOWS
