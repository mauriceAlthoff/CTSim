/*****************************************************************************
 ** FILE IDENTIFICATION
 **
 **   Name:          dlgprojections.cpp
 **   Purpose:       Projection Collection Animation Dialog
 **   Programmer:    Kevin Rosenberg
 **   Date Started:  August 2000
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

#include "dlgprojections.h"
#include "ct.h"


static const int LAYOUT_X_MARGIN = 4;
static const int LAYOUT_Y_MARGIN = 4;

BEGIN_EVENT_TABLE(ProjectionsDialog, wxDialog)
  EVT_BUTTON(wxID_CANCEL, ProjectionsDialog::OnCancel)
  EVT_BUTTON(ProjectionsDialog::ID_BTN_PAUSE, ProjectionsDialog::OnPause)
  EVT_BUTTON(ProjectionsDialog::ID_BTN_STEP, ProjectionsDialog::OnStep)
  EVT_CLOSE(ProjectionsDialog::OnClose)
  EVT_PAINT(ProjectionsDialog::OnPaint)
  END_EVENT_TABLE()

  IMPLEMENT_CLASS(ProjectionsDialog, wxDialog)


ProjectionsDialog::ProjectionsDialog (Scanner& rScanner, Projections& rProj, const Phantom& rPhantom, const int iTrace, wxWindow *parent)
: wxDialog(parent, -1, _T("Collect Projections"), wxDefaultPosition), m_rScanner(rScanner), m_rProjections(rProj), m_rPhantom(rPhantom),
      m_pSGPDriver(NULL), m_pSGP(NULL), m_iTrace(iTrace), m_pDC(NULL), m_btnAbort(0), m_btnPause(0), m_btnStep(0)
{
  m_state = Continue;
  m_iLastView = -1;
  m_parentTop = parent;
  while ( m_parentTop && m_parentTop->GetParent() )
    m_parentTop = m_parentTop->GetParent();

  m_btnAbort = new wxButton(this, wxID_CANCEL, _("Cancel"));
  wxLayoutConstraints* c = new wxLayoutConstraints;
  c->right.SameAs(this, wxRight, 2*LAYOUT_X_MARGIN);
  c->bottom.SameAs(this, wxBottom, 2*LAYOUT_Y_MARGIN);

  wxSize sizeBtn = wxButton::GetDefaultSize();
  c->width.Absolute(sizeBtn.x);
  c->height.Absolute(sizeBtn.y);

  m_btnAbort->SetConstraints(c);

  m_btnPause = new wxButton (this, ID_BTN_PAUSE, _T("Pause"));
  wxLayoutConstraints* cPause = new wxLayoutConstraints;
  cPause->right.SameAs(this, wxRight, 3*LAYOUT_X_MARGIN + sizeBtn.x);
  cPause->bottom.SameAs(this, wxBottom, 2*LAYOUT_Y_MARGIN);
  cPause->width.Absolute(sizeBtn.x);
  cPause->height.Absolute(sizeBtn.y);
  m_btnPause->SetConstraints(cPause);

  m_btnStep = new wxButton (this, ID_BTN_STEP, _T("Step"));
  wxLayoutConstraints* cStep = new wxLayoutConstraints;
  cStep->right.SameAs(this, wxRight, 5*LAYOUT_X_MARGIN + sizeBtn.x * 2);
  cStep->bottom.SameAs(this, wxBottom, 2*LAYOUT_Y_MARGIN);
  cStep->width.Absolute(sizeBtn.x);
  cStep->height.Absolute(sizeBtn.y);
  m_btnStep->SetConstraints(cStep);

  SetAutoLayout(TRUE);
  Layout();

  wxSize sizeDlg (500,500);
  if (sizeDlg.x != sizeDlg.y) {
    sizeDlg.x = imax(sizeDlg.x,sizeDlg.y);
    sizeDlg.y = imax(sizeDlg.x,sizeDlg.y);
  }
  if (m_iTrace >= Trace::TRACE_PLOT)
    sizeDlg.x += 250;

  m_iClientX = sizeDlg.x;
  m_iClientY = sizeDlg.y;
  SetClientSize(sizeDlg);

  Centre(wxCENTER_FRAME | wxBOTH);

  if ( m_parentTop )
    m_parentTop->Enable(FALSE);

  Show(TRUE);
  Enable(TRUE); // enable this window

  m_bitmap.Create (m_iClientX, m_iClientY); // save a copy of screen
  m_pDC = dynamic_cast<wxDC*> (new wxClientDC (this));
  int x, y;
  this->GetClientSize(&x, &y);
  m_pSGPDriver = new SGPDriver (m_pDC, x, y);
  m_pSGP = new SGP (*m_pSGPDriver);

  wxYield();     // Update the display

  m_pSGP->setTextPointSize(7);
#ifdef __WXMAC__
  //  MacUpdateImmediately();
#endif
}

void
ProjectionsDialog::showView (int iViewNumber)
{
  if ( iViewNumber < m_rProjections.nView() ) {
    m_iLastView = iViewNumber;
    ::wxYield();        // update the display
    m_pSGP->eraseWindow();
    m_btnPause->Refresh();
    m_btnStep->Refresh();
    m_btnAbort->Refresh();

    if (m_iTrace >= Trace::TRACE_PLOT)
      m_pSGP->setViewport (0, 0, 0.66, 1);
    ::wxYield();        // update the display
    m_rScanner.collectProjections (m_rProjections, m_rPhantom, iViewNumber, 1, m_rScanner.offsetView(), true, m_iTrace, m_pSGP);
    ::wxYield();        // update the display
    if (m_iTrace >= Trace::TRACE_PLOT) {
      const DetectorArray& detArray = m_rProjections.getDetectorArray (iViewNumber);
      const DetectorValue* detValues = detArray.detValues();
      double* detPos = new double [detArray.nDet()];
      for (int i = 0; i < detArray.nDet(); i++)
        detPos[i] = i;
      EZPlot ezplot;
      ezplot.ezset ("grid");
      ezplot.ezset ("box");
      ezplot.ezset ("yticks left");
      ezplot.ezset ("xticks major 5");
      ezplot.ezset ("yticks major 10");
      ezplot.addCurve (detValues, detPos, detArray.nDet());
#if 1
      ezplot.ezset ("xporigin 0.67");
      ezplot.ezset ("yporigin 0.10");
      ezplot.ezset ("xlength  0.33");
      ezplot.ezset ("ylength  0.90");
      m_pSGP->setViewport (0., 0., 1., 1.);
#else
      m_pSGP->setViewport (0.67, 0.1, 1., 1.);
#endif
      ezplot.plot (m_pSGP);
      delete detPos;
    }
  }
}

bool
ProjectionsDialog::projectView (int iViewNumber)
{
  if (iViewNumber <= m_iLastView)  // already done this view
    return true;

  if (iViewNumber < m_rProjections.nView()) {
    showView (iViewNumber);
    wxYield();        // update the display
    if (m_iTrace >= Trace::TRACE_PLOT) {
      ::wxMilliSleep(500);
    }
  } else {
    m_state = Finished;    // so that we return TRUE below and
    // that [Cancel] handler knew what to do
  }

#ifdef __WXMAC__
  //  MacUpdateImmediately();
#endif

  return m_state != Cancelled;
}


// EVENT HANDLERS

void ProjectionsDialog::OnCancel (wxCommandEvent& event)
{
  if ( m_state == Finished ) {
    // this means that the count down is already finished and we're being
    // shown as a modal dialog - so just let the default handler do the job
    event.Skip();
  } else {
    // request to cancel was received, the next time Update() is called we
    // will handle it
    m_state = Cancelled;

    // update the button state immediately so that the user knows that the
    // request has been noticed
    m_btnAbort->Disable();
  }
}


void
ProjectionsDialog::OnPause (wxCommandEvent& event)
{
  if ( m_state == Finished ) {
    event.Skip();
  } else if (m_state == Continue) {
    m_memoryDC.SelectObject (m_bitmap);       // in memoryDC
    m_pSGP->setDC (&m_memoryDC);
    showView (m_iLastView);
    m_state = Paused;
    m_btnPause->SetLabel (_T("Resume"));
    m_pSGP->setDC (m_pDC);
    m_memoryDC.SelectObject(wxNullBitmap);
  } else if (m_state == Paused) {
    m_state = Continue;
    m_btnPause->SetLabel (_T("Pause"));
  }
}

void
ProjectionsDialog::OnStep (wxCommandEvent& event)
{
  if ( m_state == Finished ) {
    event.Skip();
  } else if (m_state == Continue) {
    m_memoryDC.SelectObject (m_bitmap);       // in memoryDC
    m_pSGP->setDC (&m_memoryDC);
    showView (m_iLastView);
    // m_rScanner.collectProjections (m_rProjections, m_rPhantom, m_iLastView, 1, true, m_iTrace, m_pSGP);
    m_state = Paused;
    m_btnPause->SetLabel (_T("Resume"));
    m_pSGP->setDC (m_pDC);
    m_memoryDC.SelectObject(wxNullBitmap);
    Refresh();
  } else if (m_state == Paused) {
    m_memoryDC.SelectObject (m_bitmap);       // in memoryDC
    m_pSGP->setDC (&m_memoryDC);
    projectView (m_iLastView + 1);
    m_pSGP->setDC (m_pDC);
    m_memoryDC.SelectObject(wxNullBitmap);
    Refresh();
  }
}

void ProjectionsDialog::OnClose(wxCloseEvent& event)
{
  if ( m_state == Uncancellable )
    event.Veto(TRUE);    // can't close this dialog
  else if ( m_state == Finished )
    event.Skip(); // let the default handler close the window as we already terminated
  else
    m_state = Cancelled;          // next Update() will notice it
}

void
ProjectionsDialog::OnPaint (wxPaintEvent& event)
{
  wxPaintDC paintDC (this);
  if (m_state == Paused) {
    paintDC.DrawBitmap (m_bitmap, 0, 0, false);
  }
}


/////////////////////////////////////////////////////
// destruction

ProjectionsDialog::~ProjectionsDialog()
{
  if ( m_parentTop )
    m_parentTop->Enable(TRUE);

  delete m_pSGP;
  delete m_pSGPDriver;
  delete m_pDC;
}
