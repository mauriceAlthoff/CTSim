/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dlgreconstruct.cpp
**   Purpose:       Reconstruction Animation Dialog
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
#include "wx/image.h"

#include "dlgreconstruct.h"
#include <algorithm>
#include "ct.h"


static const int LAYOUT_X_MARGIN = 4;
static const int LAYOUT_Y_MARGIN = 4;


const int ReconstructDialog::ID_BTN_PAUSE = 19998;
const int ReconstructDialog::ID_BTN_STEP = 19999;
const int ReconstructDialog::MAX_IMAGE_X = 400;
const int ReconstructDialog::MAX_IMAGE_Y = 400;


BEGIN_EVENT_TABLE(ReconstructDialog, wxDialog)
EVT_BUTTON(wxID_CANCEL, ReconstructDialog::OnCancel)
EVT_BUTTON(ID_BTN_PAUSE, ReconstructDialog::OnPause)
EVT_BUTTON(ID_BTN_STEP, ReconstructDialog::OnStep)
EVT_CLOSE(ReconstructDialog::OnClose)
EVT_PAINT(ReconstructDialog::OnPaint)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ReconstructDialog, wxDialog)


ReconstructDialog::ReconstructDialog (Reconstructor& rReconstruct, const Projections& rProj,
                                      ImageFile& rIF, const int iTrace, wxWindow *parent)
: wxDialog(parent, -1, _T("Reconstruction"), wxDefaultPosition), m_rReconstructor(rReconstruct),
           m_rProjections(rProj), m_rImageFile(rIF), m_pSGPDriver(NULL), m_pSGP(NULL),
  m_iTrace(iTrace), m_pDC(NULL), m_btnAbort(0), m_btnPause(0), m_btnStep(0)
{
    m_state = Continue;
    m_iLastView = -1;
    m_parentTop = parent;
    while ( m_parentTop && m_parentTop->GetParent() )
        m_parentTop = m_parentTop->GetParent();

    m_btnAbort = new wxButton(this, wxID_CANCEL, _T("Cancel"));
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

    m_nxGraph = 500;
    m_nyGraph = 500;
    wxSize sizeDlg (m_nxGraph, m_nyGraph);
    m_nxImage = m_rImageFile.nx();
    if (m_nxImage > MAX_IMAGE_X)
                   m_nxImage = MAX_IMAGE_X;
    m_nyImage = m_rImageFile.ny();
    if (m_nyImage > MAX_IMAGE_Y)
                   m_nyImage = MAX_IMAGE_Y;

    sizeDlg.x += m_nxImage;
    sizeDlg.y = imax (sizeDlg.y, m_nyImage);

    m_iClientX = sizeDlg.x;
    m_iClientY = sizeDlg.y;
    SetClientSize (sizeDlg);

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

    m_pSGPDriver->idWX()->SetFont(*wxSWISS_FONT);
}

void
ReconstructDialog::showView (int iViewNumber, bool bBackprojectView)
{
        if ( iViewNumber < m_rProjections.nView() ) {
                m_iLastView = iViewNumber;
                ::wxYield();        // update the display
                m_pSGP->eraseWindow();
                m_btnPause->Refresh();
                m_btnStep->Refresh();
                m_btnAbort->Refresh();

                char szProgress [256];
                snprintf (szProgress, sizeof(szProgress), "Reconstructing View %d (%.1f%%)", iViewNumber, 100 * iViewNumber / static_cast<double>(m_rProjections.nView()));
                m_pSGP->setViewport (0, 0, 1, 1);
                m_pSGP->setWindow (0, 0, 1, 1);
                m_pSGP->setTextColor (C_LTRED, -1);
                m_pSGP->setTextPointSize (20.);
                m_pSGP->moveAbs(0., m_pSGP->getCharHeight());
                m_pSGP->drawText (szProgress);
                m_pSGP->setTextPointSize (10.);

    int iXDialog, iYDialog;
    GetClientSize (&iXDialog, &iYDialog);
    double dGraphWidth = (iXDialog - m_nxImage) / static_cast<double>(iXDialog);

                m_rReconstructor.reconstructView (iViewNumber, 1, m_pSGP, bBackprojectView, dGraphWidth);
    ::wxYield();
::wxYield();
::wxYield();
::wxYield();

                ImageFileArrayConst v = m_rImageFile.getArray();
                int xBase = m_nxGraph;
                int yBase = 0;
                if (m_nyGraph > m_nyImage)
                        yBase = (m_nyGraph - m_nyImage) / 2;
                double minValue = v[0][0];
                double maxValue = v[0][0];
                for (int ix = 0; ix < m_nxImage; ix++) {
                        for (int iy = 0; iy < m_nyImage; iy++) {
                                double dPixel = v[ix][iy];
                                if (dPixel < minValue)
                                        minValue = dPixel;
                                else if (dPixel > maxValue)
                                        maxValue = dPixel;
                        }
                }
                unsigned char* imageData = new unsigned char [m_nxImage * m_nyImage * 3];
                double dScale = 255 / (maxValue - minValue);
                for (int ix2 = 0; ix2 < m_nxImage; ix2++) {
                        for (int iy = 0; iy < m_nyImage; iy++) {
                                double dPixel = v[ix2][iy];
                                dPixel = (dPixel - minValue) * dScale;
                                int intensity = nearest<int>(dPixel);
                                intensity = clamp (intensity, 0, 255);
                                int baseAddr = ((m_nyImage - 1 - iy) * m_nxImage + ix2) * 3;
                                imageData[baseAddr] = imageData[baseAddr+1] = imageData[baseAddr+2] = intensity;
                        }
                }
                wxImage image (m_nxImage, m_nyImage, imageData, true);
                wxBitmap bitmap (image);
                m_pSGP->getDriver().idWX()->DrawBitmap(bitmap, xBase, yBase, false);
                delete imageData;
        }

  ::wxYield();
}

bool
ReconstructDialog::reconstructView (int iViewNumber, bool bBackproject)
{
        if (iViewNumber <= m_iLastView)  // have already done this view
                return true;

        if (iViewNumber < m_rProjections.nView()) {
                ::wxYield();        // update the display
                showView (iViewNumber, bBackproject);
                ::wxYield();        // update the display
                if (m_iTrace >= Trace::TRACE_PLOT) {
                        ::wxMilliSleep(250);
                }
        } else {
                m_state = Finished;    // so that we return TRUE below and
        }                        // that [Cancel] handler knew what to do

        ::wxYield();        // update the display
        return m_state != Cancelled;
}


// EVENT HANDLERS

void ReconstructDialog::OnCancel (wxCommandEvent& event)
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
ReconstructDialog::OnPause (wxCommandEvent& event)
{
        if ( m_state == Finished ) {
                // this means that the count down is already finished and we're being
                // shown as a modal dialog - so just let the default handler do the job
                event.Skip();
        } else if (m_state == Continue) {
                m_memoryDC.SelectObject (m_bitmap);       // in memoryDC
                m_pSGP->setDC (&m_memoryDC);
                m_memoryDC.SetFont (*wxSWISS_FONT);
                showView (m_iLastView, false);
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
ReconstructDialog::OnStep (wxCommandEvent& event)
{
        if ( m_state == Finished ) {
                event.Skip();
        } else if (m_state == Continue) {
                m_memoryDC.SelectObject (m_bitmap);       // in memoryDC
                m_pSGP->setDC (&m_memoryDC);
                m_memoryDC.SetFont (*wxSWISS_FONT);
                showView (m_iLastView, false);
                m_state = Paused;
                m_btnPause->SetLabel (_T("Resume"));
                m_pSGP->setDC (m_pDC);
                m_memoryDC.SelectObject(wxNullBitmap);
                Refresh();
        } else if (m_state == Paused) {
                m_memoryDC.SelectObject (m_bitmap);       // in memoryDC
                m_pSGP->setDC (&m_memoryDC);
                m_memoryDC.SetFont (*wxSWISS_FONT);
                reconstructView (m_iLastView + 1);
                m_pSGP->setDC (m_pDC);
                m_memoryDC.SelectObject(wxNullBitmap);
                Refresh();
        }
}

void ReconstructDialog::OnClose(wxCloseEvent& event)
{
    if ( m_state == Uncancellable )
                event.Veto(TRUE);    // can't close this dialog
    else if ( m_state == Finished )
                event.Skip(); // let the default handler close the window as we already terminated
    else
                m_state = Cancelled;          // next Update() will notice it
}

void
ReconstructDialog::OnPaint (wxPaintEvent& event)
{
        wxPaintDC paintDC (this);
        if (m_state == Paused) {
                paintDC.DrawBitmap (m_bitmap, 0, 0, false);
        }
}


/////////////////////////////////////////////////////
// destruction

ReconstructDialog::~ReconstructDialog()
{
        if ( m_parentTop )
                m_parentTop->Enable(TRUE);

        delete m_pSGP;
        delete m_pSGPDriver;
        delete m_pDC;
}

