/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          views.cpp
**   Purpose:       View & Canvas routines for CTSim program
**   Programmer:    Kevin Rosenberg
**   Date Started:  July 2000
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
#include "wx/wx.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "wx/image.h"
#include "wx/progdlg.h"
#include "wx/clipbrd.h"

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "dialogs.h"
#include "dlgprojections.h"
#include "dlgreconstruct.h"
#include "backprojectors.h"
#include "reconstruct.h"
#include "timer.h"
#include "threadproj.h"
#include "threadrecon.h"
#include "threadraster.h"

#if defined(MSVC) || HAVE_SSTREAM
#include <sstream>
#else
#include <sstream_subst>
#endif

// Used to reduce calls to progress bar update function
const short int ITER_PER_UPDATE = 10;

// ImageFileCanvas

BEGIN_EVENT_TABLE(ImageFileCanvas, wxScrolledWindow)
EVT_MOUSE_EVENTS(ImageFileCanvas::OnMouseEvent)
EVT_CHAR(ImageFileCanvas::OnChar)
END_EVENT_TABLE()


ImageFileCanvas::ImageFileCanvas (ImageFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style)
: wxScrolledWindow(frame, -1, pos, size, style), m_pView(v), m_xCursor(-1), m_yCursor(-1)
{
}

ImageFileCanvas::~ImageFileCanvas()
{}

void
ImageFileCanvas::OnDraw(wxDC& dc)
{
  if (m_pView)
    m_pView->OnDraw(& dc);
}

void
ImageFileCanvas::DrawRubberBandCursor (wxDC& dc, int x, int y)
{
  const ImageFile& rIF = m_pView->GetDocument()->getImageFile();
  int nx = rIF.nx();
  int ny = rIF.ny();

  int yPt = ny - y - 1;
  dc.SetLogicalFunction (wxINVERT);
  dc.SetPen (*wxGREEN_PEN);
  dc.DrawLine (0, yPt, nx, yPt);
  dc.DrawLine (x, 0, x, ny);
  dc.SetLogicalFunction (wxCOPY);
}

bool
ImageFileCanvas::GetCurrentCursor (int& x, int& y)
{
  x = m_xCursor;
  y = m_yCursor;

  if (m_xCursor >= 0 && m_yCursor >= 0)
    return true;
  else
    return false;
}

void
ImageFileCanvas::OnMouseEvent(wxMouseEvent& event)
{
  if (! m_pView)
    return;


  wxClientDC dc(this);
  PrepareDC(dc);

  wxPoint pt(event.GetLogicalPosition(dc));

  const ImageFileDocument* pIFDoc = m_pView->GetDocument();
  if (! pIFDoc)
    return;
  const ImageFile& rIF = pIFDoc->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  int nx = rIF.nx();
  int ny = rIF.ny();
  const int yPt = ny - 1 - pt.y;
  if (event.RightIsDown()) {
    if (pt.x >= 0 && pt.x < nx && pt.y >= 0 && pt.y < ny) {
      wxString os;
      os << _T("Image value (") << pt.x << _T(",") << yPt << _T(") = ")
         << v[pt.x][yPt];
      if (rIF.isComplex()) {
        double dImag = rIF.getImaginaryArray()[pt.x][yPt];
        if (dImag < 0)
          os << _T(" - ") << -dImag;
        else
          os << _T(" + ") << dImag;
        os << _T("i\n");
      } else
        os << _T("\n");
      *theApp->getLog() << os;
    } else
      *theApp->getLog() << _T("Mouse out of image range (") << pt.x << _T(",") << yPt << _T(")\n");
  }
  else if (event.LeftIsDown() || event.LeftUp() || event.RightUp()) {
    if (pt.x >= 0 && pt.x < nx && pt.y >= 0 && pt.y < ny) {
      if (m_xCursor >= 0 && m_yCursor >= 0) {
        DrawRubberBandCursor (dc, m_xCursor, m_yCursor);
      }
      DrawRubberBandCursor (dc, pt.x, yPt);
      m_xCursor = pt.x;
      m_yCursor = yPt;
      wxMenu* pMenu = m_pView->getMenuAnalyze();
      if (pMenu && ! pMenu->IsEnabled(IFMENU_PLOT_ROW)) {
        pMenu->Enable (IFMENU_PLOT_ROW, true);
        pMenu->Enable (IFMENU_PLOT_COL, true);
        pMenu->Enable (IFMENU_COMPARE_ROW, true);
        pMenu->Enable (IFMENU_COMPARE_COL, true);
        pMenu->Enable (IFMENU_PLOT_FFT_ROW, true);
        pMenu->Enable (IFMENU_PLOT_FFT_COL, true);
      }
    } else
      *theApp->getLog() << _T("Mouse out of image range (") << pt.x << _T(",") << yPt << _T(")\n");
  }
  if (event.LeftUp()) {
    wxString os;
    os << _T("Selected column ") << pt.x << _T(" , row ") << yPt << _T("\n");
    *theApp->getLog() << os;
  }
}

void
ImageFileCanvas::OnChar (wxKeyEvent& event)
{
  if (event.GetKeyCode() == WXK_ESCAPE) {
    m_xCursor = -1;
    m_yCursor = -1;
    if (m_pView)
      m_pView->OnUpdate (NULL);
  } else
    event.Skip();
}

wxSize
ImageFileCanvas::GetBestSize() const
{
  const int iMinX = 50;
  const int iMinY = 20;
  wxSize bestSize (iMinX,iMinY);

  if (m_pView) {
    const ImageFile& rIF = m_pView->GetDocument()->getImageFile();
    bestSize.Set  (rIF.nx(), rIF.ny());
  }

  if (bestSize.x > 800)
    bestSize.x = 800;
  if (bestSize.y > 800)
    bestSize.y = 800;

  if (bestSize.y < iMinY)
    bestSize.y = iMinY;
  if (bestSize.x < iMinX)
    bestSize.x = iMinX;

  return bestSize;
}


// ImageFileView

IMPLEMENT_DYNAMIC_CLASS(ImageFileView, wxView)

BEGIN_EVENT_TABLE(ImageFileView, wxView)
EVT_MENU(IFMENU_FILE_EXPORT, ImageFileView::OnExport)
EVT_MENU(IFMENU_FILE_PROPERTIES, ImageFileView::OnProperties)
EVT_MENU(IFMENU_EDIT_COPY, ImageFileView::OnEditCopy)
EVT_MENU(IFMENU_EDIT_CUT, ImageFileView::OnEditCut)
EVT_MENU(IFMENU_EDIT_PASTE, ImageFileView::OnEditPaste)
EVT_MENU(IFMENU_VIEW_SCALE_MINMAX, ImageFileView::OnScaleMinMax)
EVT_MENU(IFMENU_VIEW_SCALE_AUTO, ImageFileView::OnScaleAuto)
EVT_MENU(IFMENU_VIEW_SCALE_FULL, ImageFileView::OnScaleFull)
EVT_MENU(IFMENU_COMPARE_IMAGES, ImageFileView::OnCompare)
EVT_MENU(IFMENU_COMPARE_ROW, ImageFileView::OnCompareRow)
EVT_MENU(IFMENU_COMPARE_COL, ImageFileView::OnCompareCol)
EVT_MENU(IFMENU_FILTER_INVERTVALUES, ImageFileView::OnInvertValues)
EVT_MENU(IFMENU_FILTER_SQUARE, ImageFileView::OnSquare)
EVT_MENU(IFMENU_FILTER_SQRT, ImageFileView::OnSquareRoot)
EVT_MENU(IFMENU_FILTER_LOG, ImageFileView::OnLog)
EVT_MENU(IFMENU_FILTER_EXP, ImageFileView::OnExp)
EVT_MENU(IFMENU_FILTER_FOURIER, ImageFileView::OnFourier)
EVT_MENU(IFMENU_FILTER_INVERSE_FOURIER, ImageFileView::OnInverseFourier)
EVT_MENU(IFMENU_FILTER_SHUFFLEFOURIERTONATURALORDER, ImageFileView::OnShuffleFourierToNaturalOrder)
EVT_MENU(IFMENU_FILTER_SHUFFLENATURALTOFOURIERORDER, ImageFileView::OnShuffleNaturalToFourierOrder)
EVT_MENU(IFMENU_IMAGE_ADD, ImageFileView::OnAdd)
EVT_MENU(IFMENU_IMAGE_SUBTRACT, ImageFileView::OnSubtract)
EVT_MENU(IFMENU_IMAGE_MULTIPLY, ImageFileView::OnMultiply)
EVT_MENU(IFMENU_IMAGE_DIVIDE, ImageFileView::OnDivide)
EVT_MENU(IFMENU_IMAGE_SCALESIZE, ImageFileView::OnScaleSize)
#if wxUSE_GLCANVAS
EVT_MENU(IFMENU_IMAGE_CONVERT3D, ImageFileView::OnConvert3d)
#endif
#ifdef HAVE_FFT
EVT_MENU(IFMENU_FILTER_FFT, ImageFileView::OnFFT)
EVT_MENU(IFMENU_FILTER_IFFT, ImageFileView::OnIFFT)
EVT_MENU(IFMENU_FILTER_FFT_ROWS, ImageFileView::OnFFTRows)
EVT_MENU(IFMENU_FILTER_IFFT_ROWS, ImageFileView::OnIFFTRows)
EVT_MENU(IFMENU_FILTER_FFT_COLS, ImageFileView::OnFFTCols)
EVT_MENU(IFMENU_FILTER_IFFT_COLS, ImageFileView::OnIFFTCols)
#endif
EVT_MENU(IFMENU_FILTER_MAGNITUDE, ImageFileView::OnMagnitude)
EVT_MENU(IFMENU_FILTER_PHASE, ImageFileView::OnPhase)
EVT_MENU(IFMENU_FILTER_REAL, ImageFileView::OnReal)
EVT_MENU(IFMENU_FILTER_IMAGINARY, ImageFileView::OnImaginary)
EVT_MENU(IFMENU_PLOT_ROW, ImageFileView::OnPlotRow)
EVT_MENU(IFMENU_PLOT_COL, ImageFileView::OnPlotCol)
#ifdef HAVE_FFT
EVT_MENU(IFMENU_PLOT_FFT_ROW, ImageFileView::OnPlotFFTRow)
EVT_MENU(IFMENU_PLOT_FFT_COL, ImageFileView::OnPlotFFTCol)
#endif
EVT_MENU(IFMENU_PLOT_HISTOGRAM, ImageFileView::OnPlotHistogram)
END_EVENT_TABLE()

ImageFileView::ImageFileView()
  :  wxView(), m_pBitmap(0), m_pFrame(0), m_pCanvas(0), m_pFileMenu(0),
     m_pFilterMenu(0), m_bMinSpecified(false), m_bMaxSpecified(false),
     m_iDefaultExportFormatID(ImageFile::EXPORT_FORMAT_PNG)
{}

ImageFileView::~ImageFileView()
{
  GetDocumentManager()->FileHistoryRemoveMenu (m_pFileMenu);
  GetDocumentManager()->ActivateView(this, FALSE);
}


void
ImageFileView::OnProperties (wxCommandEvent& event)
{
  const ImageFile& rIF = GetDocument()->getImageFile();
  if (rIF.nx() == 0 || rIF.ny() == 0)
    *theApp->getLog() << _T("Properties: empty imagefile\n");
  else {
    const std::string rFilename (m_pFrame->GetTitle().mb_str(wxConvUTF8));
    std::ostringstream os;
    double min, max, mean, mode, median, stddev;
    rIF.statistics (rIF.getArray(), min, max, mean, mode, median, stddev);
    os << "Filename: " << rFilename << "\n";
    os << "Size: (" << rIF.nx() << "," << rIF.ny() << ")\n";
    os << "Data type: ";
    if (rIF.isComplex())
      os << "Complex\n";
    else
      os << "Real\n";
    os << "Minimum: "<<min<<"\nMaximum: "<<max<<"\nMean: "<<mean<<"\nMedian: "<<median<<"\nMode: "<<mode<<"\nStandard Deviation: "<<stddev << "\n";
    if (rIF.isComplex()) {
      rIF.statistics (rIF.getImaginaryArray(), min, max, mean, mode, median, stddev);
      os << "Imaginary: min: "<<min<<"\nmax: "<<max<<"\nmean: "<<mean<<"\nmedian: "<<median<<"\nmode: "<<mode<<"\nstddev: "<<stddev << "\n";
    }
    if (rIF.nLabels() > 0) {
      rIF.printLabelsBrief (os);
    }
    *theApp->getLog() << _T(">>>>\n") << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("<<<<\n");
    wxMessageDialog dialogMsg (getFrameForChild(), wxConvUTF8.cMB2WX(os.str().c_str()), _T("Imagefile Properties"), wxOK | wxICON_INFORMATION);
    dialogMsg.ShowModal();
  }
  GetDocument()->Activate();
}

void
ImageFileView::OnScaleAuto (wxCommandEvent& event)
{
  const ImageFile& rIF = GetDocument()->getImageFile();
  double min, max, mean, mode, median, stddev;
  rIF.statistics(min, max, mean, mode, median, stddev);
  DialogAutoScaleParameters dialogAutoScale (getFrameForChild(), mean, mode, median, stddev, m_dAutoScaleFactor);
  int iRetVal = dialogAutoScale.ShowModal();
  if (iRetVal == wxID_OK) {
    m_bMinSpecified = true;
    m_bMaxSpecified = true;
    double dMin, dMax;
    if (dialogAutoScale.getMinMax (&dMin, &dMax)) {
      m_dMinPixel = dMin;
      m_dMaxPixel = dMax;
      m_dAutoScaleFactor = dialogAutoScale.getAutoScaleFactor();
      OnUpdate(this, NULL);
      GetDocument()->UpdateAllViews (this);
    }
  }
  GetDocument()->Activate();
}

void
ImageFileView::OnScaleMinMax (wxCommandEvent& event)
{
  const ImageFile& rIF = GetDocument()->getImageFile();
  double min, max;
  if (! m_bMinSpecified && ! m_bMaxSpecified)
    rIF.getMinMax (min, max);

  if (m_bMinSpecified)
    min = m_dMinPixel;
  if (m_bMaxSpecified)
    max = m_dMaxPixel;

  DialogGetMinMax dialogMinMax (getFrameForChild(), _T("Set Image Minimum & Maximum"), min, max);
  int retVal = dialogMinMax.ShowModal();
  if (retVal == wxID_OK) {
    m_bMinSpecified = true;
    m_bMaxSpecified = true;
    m_dMinPixel = dialogMinMax.getMinimum();
    m_dMaxPixel = dialogMinMax.getMaximum();
    OnUpdate(this, NULL);
    GetDocument()->UpdateAllViews (this);
  }
  GetDocument()->Activate();
}

void
ImageFileView::OnScaleFull (wxCommandEvent& event)
{
  if (m_bMinSpecified || m_bMaxSpecified) {
    m_bMinSpecified = false;
    m_bMaxSpecified = false;
    OnUpdate(this, NULL);
    GetDocument()->UpdateAllViews (this);
  }
  GetDocument()->Activate();
}

void
ImageFileView::OnCompare (wxCommandEvent& event)
{
  std::vector<ImageFileDocument*> vecIF;
  theApp->getCompatibleImages (GetDocument(), vecIF);

  if (vecIF.size() == 0) {
    wxMessageBox(_T("There are no compatible image files open for comparision"), _T("No comparison images"));
  } else {
    DialogGetComparisonImage dialogGetCompare(getFrameForChild(), _T("Get Comparison Image"), vecIF, true);

    if (dialogGetCompare.ShowModal() == wxID_OK) {
      const ImageFile& rIF = GetDocument()->getImageFile();
      ImageFileDocument* pCompareDoc = dialogGetCompare.getImageFileDocument();
      const ImageFile& rCompareIF = pCompareDoc->getImageFile();
      std::ostringstream os;
      double min, max, mean, mode, median, stddev;
      rIF.statistics (min, max, mean, mode, median, stddev);
      os << m_pFrame->GetTitle().mb_str(wxConvUTF8) << ": minimum=" << min << ", maximum=" << max << ", mean=" << mean << ", mode=" << mode << ", median=" << median << ", stddev=" << stddev << "\n";
      rCompareIF.statistics (min, max, mean, mode, median, stddev);
      os << dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8) << ": minimum=" << min << ", maximum=" << max << ", mean=" << mean << ", mode=" << mode << ", median=" << median << ", stddev=" << stddev << "\n";
      double d, r, e;
      rIF.comparativeStatistics (rCompareIF, d, r, e);
      os << "Comparative Statistics: d=" << d << ", r=" << r << ", e=" << e << "\n";
      *theApp->getLog() << _T(">>>>\n") << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("<<<<\n");
      if (dialogGetCompare.getMakeDifferenceImage()) {
        ImageFile* pDifferenceImage = new ImageFile;

        pDifferenceImage->setArraySize (rIF.nx(), rIF.ny());
        if (! rIF.subtractImages (rCompareIF, *pDifferenceImage)) {
          *theApp->getLog() << _T("Unable to subtract images\n");
          delete pDifferenceImage;
          return;
        }
        ImageFileDocument* pDifferenceDoc = theApp->newImageDoc();
        if (! pDifferenceDoc) {
          sys_error (ERR_SEVERE, "Unable to create image file");
          return;
        }
        pDifferenceDoc->setImageFile (pDifferenceImage);

        wxString s = m_pFrame->GetTitle() + _T(": ");
        pDifferenceImage->labelsCopy (rIF, s.mb_str(wxConvUTF8));
        s = dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
        pDifferenceImage->labelsCopy (rCompareIF, s.mb_str(wxConvUTF8));
        std::ostringstream osLabel;
        osLabel << "Compare image " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str()
                << " and " << dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle().c_str() << ": "
          << os.str().c_str();
        pDifferenceImage->labelAdd (os.str().c_str());
        if (theApp->getAskDeleteNewDocs())
          pDifferenceDoc->Modify (true);
        OnUpdate(this, NULL);
        pDifferenceDoc->UpdateAllViews(this);
        pDifferenceDoc->getView()->setInitialClientSize();
        pDifferenceDoc->Activate();
      }
      wxMessageBox(wxConvUTF8.cMB2WX(os.str().c_str()), _T("Image Comparison"));
    }
  }
}

void
ImageFileView::OnInvertValues (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.invertPixelValues (rIF);
  rIF.labelAdd ("Invert Pixel Values");
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnSquare (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.square (rIF);
  rIF.labelAdd ("Square Pixel Values");
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnSquareRoot (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.sqrt (rIF);
  rIF.labelAdd ("Square-root Pixel Values");
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnLog (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.log (rIF);
  rIF.labelAdd ("Logrithm base-e Pixel Values");
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnExp (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.exp (rIF);
  rIF.labelAdd ("Exponent base-e Pixel Values");
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnAdd (wxCommandEvent& event)
{
  std::vector<ImageFileDocument*> vecIF;
  theApp->getCompatibleImages (GetDocument(), vecIF);

  if (vecIF.size() == 0) {
    wxMessageBox (_T("There are no compatible image files open for comparision"), _T("No comparison images"));
  } else {
    DialogGetComparisonImage dialogGetCompare (getFrameForChild(), _T("Get Image to Add"), vecIF, false);

    if (dialogGetCompare.ShowModal() == wxID_OK) {
      ImageFile& rIF = GetDocument()->getImageFile();
      ImageFileDocument* pRHSDoc = dialogGetCompare.getImageFileDocument();
      const ImageFile& rRHSIF = pRHSDoc->getImageFile();
      ImageFileDocument* pNewDoc = theApp->newImageDoc();
      if (! pNewDoc) {
        sys_error (ERR_SEVERE, "Unable to create image file");
        return;
      }
      ImageFile& newImage = pNewDoc->getImageFile();
      newImage.setArraySize (rIF.nx(), rIF.ny());
      rIF.addImages (rRHSIF, newImage);
      std::ostringstream os;
      os << "Add image " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str() << " and "
         << dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle().c_str();
      wxString s = dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rIF, s.mb_str(wxConvUTF8));
      s = dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rRHSIF, s.mb_str(wxConvUTF8));
      newImage.labelAdd (os.str().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      if (theApp->getAskDeleteNewDocs())
        pNewDoc->Modify (true);
      OnUpdate(this, NULL);
      pNewDoc->UpdateAllViews (this);
      pNewDoc->getView()->setInitialClientSize();
      pNewDoc->Activate();
    }
  }
}

void
ImageFileView::OnSubtract (wxCommandEvent& event)
{
  std::vector<ImageFileDocument*> vecIF;
  theApp->getCompatibleImages (GetDocument(), vecIF);

  if (vecIF.size() == 0) {
    wxMessageBox (_T("There are no compatible image files open for comparision"), _T("No comparison images"));
  } else {
    DialogGetComparisonImage dialogGetCompare (getFrameForChild(), _T("Get Image to Subtract"), vecIF, false);

    if (dialogGetCompare.ShowModal() == wxID_OK) {
      ImageFile& rIF = GetDocument()->getImageFile();
      ImageFileDocument* pRHSDoc = dialogGetCompare.getImageFileDocument();
      const ImageFile& rRHSIF = pRHSDoc->getImageFile();
      ImageFileDocument* pNewDoc = theApp->newImageDoc();
      if (! pNewDoc) {
        sys_error (ERR_SEVERE, "Unable to create image file");
        return;
      }
      ImageFile& newImage = pNewDoc->getImageFile();
      newImage.setArraySize (rIF.nx(), rIF.ny());
      rIF.subtractImages (rRHSIF, newImage);
      std::ostringstream os;
      os << "Subtract image " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str() << " and "
         << dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle().c_str();
      wxString s = dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rIF, s.mb_str(wxConvUTF8));
      s = dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rRHSIF, s.mb_str(wxConvUTF8));
      newImage.labelAdd (os.str().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      if (theApp->getAskDeleteNewDocs())
        pNewDoc->Modify (true);
      OnUpdate(this, NULL);
      pNewDoc->UpdateAllViews (this);
      pNewDoc->getView()->setInitialClientSize();
      pNewDoc->Activate();
    }
  }
}

void
ImageFileView::OnMultiply (wxCommandEvent& event)
{
  std::vector<ImageFileDocument*> vecIF;
  theApp->getCompatibleImages (GetDocument(), vecIF);

  if (vecIF.size() == 0) {
    wxMessageBox (_T("There are no compatible image files open for comparision"), _T("No comparison images"));
  } else {
    DialogGetComparisonImage dialogGetCompare (getFrameForChild(), _T("Get Image to Multiply"), vecIF, false);

    if (dialogGetCompare.ShowModal() == wxID_OK) {
      ImageFile& rIF = GetDocument()->getImageFile();
      ImageFileDocument* pRHSDoc = dialogGetCompare.getImageFileDocument();
      const ImageFile& rRHSIF = pRHSDoc->getImageFile();
      ImageFileDocument* pNewDoc = theApp->newImageDoc();
      if (! pNewDoc) {
        sys_error (ERR_SEVERE, "Unable to create image file");
        return;
      }
      ImageFile& newImage = pNewDoc->getImageFile();
      newImage.setArraySize (rIF.nx(), rIF.ny());
      rIF.multiplyImages (rRHSIF, newImage);
      std::ostringstream os;
      os << "Multiply image " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str() << " and "
         << dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle().c_str();
      wxString s = dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rIF, s.mb_str(wxConvUTF8));
      s = dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rRHSIF, s.mb_str(wxConvUTF8));
      newImage.labelAdd (os.str().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      if (theApp->getAskDeleteNewDocs())
        pNewDoc->Modify (true);
      OnUpdate(this, NULL);
      pNewDoc->UpdateAllViews (this);
      pNewDoc->getView()->setInitialClientSize();
      pNewDoc->Activate();
    }
  }
}

void
ImageFileView::OnDivide (wxCommandEvent& event)
{
  std::vector<ImageFileDocument*> vecIF;
  theApp->getCompatibleImages (GetDocument(), vecIF);

  if (vecIF.size() == 0) {
    wxMessageBox (_T("There are no compatible image files open for comparision"), _T("No comparison images"));
  } else {
    DialogGetComparisonImage dialogGetCompare (getFrameForChild(), _T("Get Image to Divide"), vecIF, false);

    if (dialogGetCompare.ShowModal() == wxID_OK) {
      ImageFile& rIF = GetDocument()->getImageFile();
      ImageFileDocument* pRHSDoc = dialogGetCompare.getImageFileDocument();
      const ImageFile& rRHSIF = pRHSDoc->getImageFile();
      ImageFileDocument* pNewDoc = theApp->newImageDoc();
      if (! pNewDoc) {
        sys_error (ERR_SEVERE, "Unable to create image file");
        return;
      }
      ImageFile& newImage = pNewDoc->getImageFile();
      newImage.setArraySize (rIF.nx(), rIF.ny());
      rIF.divideImages (rRHSIF, newImage);
      std::ostringstream os;
      os << "Divide image " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str() << " by "
         << dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle().c_str();
      wxString s = dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rIF, s.mb_str(wxConvUTF8));
      s = dynamic_cast<wxFrame*>(pRHSDoc->GetFirstView()->GetFrame())->GetTitle() + _T(": ");
      newImage.labelsCopy (rRHSIF, s.mb_str(wxConvUTF8));
      newImage.labelAdd (os.str().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      if (theApp->getAskDeleteNewDocs())
        pNewDoc->Modify (true);
      OnUpdate(this, NULL);
      pNewDoc->UpdateAllViews (this);
      pNewDoc->getView()->setInitialClientSize();
      pNewDoc->Activate();
    }
  }
}


#ifdef HAVE_FFT
void
ImageFileView::OnFFT (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.fft (rIF);
  rIF.labelAdd ("FFT Image");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnIFFT (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.ifft (rIF);
  rIF.labelAdd ("IFFT Image");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnFFTRows (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.fftRows (rIF);
  rIF.labelAdd ("FFT Rows");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnIFFTRows (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.ifftRows (rIF);
  rIF.labelAdd ("IFFT Rows");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnFFTCols (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.fftCols (rIF);
  rIF.labelAdd ("FFT Columns");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnIFFTCols (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.ifftCols (rIF);
  rIF.labelAdd ("IFFT Columns");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}
#endif

void
ImageFileView::OnFourier (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  wxProgressDialog dlgProgress (_T("Fourier"), _T("Fourier Progress"), 1, getFrameForChild(), wxPD_APP_MODAL);
  rIF.fourier (rIF);
  rIF.labelAdd ("Fourier Image");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnInverseFourier (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  wxProgressDialog dlgProgress (_T("Inverse Fourier"), _T("Inverse Fourier Progress"), 1, getFrameForChild(), wxPD_APP_MODAL);
  rIF.inverseFourier (rIF);
  rIF.labelAdd ("Inverse Fourier Image");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnShuffleNaturalToFourierOrder (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  Fourier::shuffleNaturalToFourierOrder (rIF);
  rIF.labelAdd ("Shuffle Natural To Fourier Order");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnShuffleFourierToNaturalOrder (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  Fourier::shuffleFourierToNaturalOrder (rIF);
  rIF.labelAdd ("Shuffle Fourier To Natural Order");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnMagnitude (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  rIF.magnitude (rIF);
  rIF.labelAdd ("Magnitude");
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews (this);
  GetDocument()->Activate();
}

void
ImageFileView::OnPhase (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  if (rIF.isComplex()) {
    rIF.phase (rIF);
    rIF.labelAdd ("Phase of complex-image");
    m_bMinSpecified = false;
    m_bMaxSpecified = false;
    if (theApp->getAskDeleteNewDocs())
      GetDocument()->Modify (true);
    OnUpdate(this, NULL);
    GetDocument()->UpdateAllViews (this);
  }
  GetDocument()->Activate();
}

void
ImageFileView::OnReal (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  if (rIF.isComplex()) {
    rIF.real (rIF);
    rIF.labelAdd ("Real component of complex-image");
    m_bMinSpecified = false;
    m_bMaxSpecified = false;
    if (theApp->getAskDeleteNewDocs())
      GetDocument()->Modify (true);
    OnUpdate(this, NULL);
    GetDocument()->UpdateAllViews (this);
  }
  GetDocument()->Activate();
}

void
ImageFileView::OnImaginary (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  if (rIF.isComplex()) {
    rIF.imaginary (rIF);
    rIF.labelAdd ("Imaginary component of complex-image");
    m_bMinSpecified = false;
    m_bMaxSpecified = false;
    if (theApp->getAskDeleteNewDocs())
      GetDocument()->Modify (true);
    OnUpdate(this, NULL);
    GetDocument()->UpdateAllViews (this);
  }
  GetDocument()->Activate();
}


ImageFileCanvas*
ImageFileView::CreateCanvas (wxFrame* parent)
{
  ImageFileCanvas* pCanvas = new ImageFileCanvas (this, parent, wxPoint(-1,-1),
                                                  wxSize(-1,-1), 0);
  pCanvas->SetBackgroundColour(*wxWHITE);
  pCanvas->ClearBackground();

  return pCanvas;
}

#if CTSIM_MDI
wxDocMDIChildFrame*
#else
wxDocChildFrame*
#endif
ImageFileView::CreateChildFrame(wxDocument *doc, wxView *view)
{
#if CTSIM_MDI
  wxDocMDIChildFrame* subframe = new wxDocMDIChildFrame (doc, view, theApp->getMainFrame(), -1, _T("ImageFile Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#else
  wxDocChildFrame* subframe = new wxDocChildFrame (doc, view, theApp->getMainFrame(), -1, _T("ImageFile Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#endif
  theApp->setIconForFrame (subframe);

  m_pFileMenu = new wxMenu;
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  m_pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
  m_pFileMenu->Append(wxID_SAVE, _T("&Save\tCtrl-S"));
  m_pFileMenu->Append(wxID_SAVEAS, _T("Save &As..."));
  m_pFileMenu->Append(wxID_CLOSE, _T("&Close\tCtrl-W"));
  m_pFileMenu->Append(wxID_REVERT, _T("Re&vert"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(IFMENU_FILE_PROPERTIES, _T("P&roperties\tCtrl-I"));
  m_pFileMenu->Append(IFMENU_FILE_EXPORT, _T("Expor&t..."));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_PRINT, _T("&Print..."));
  m_pFileMenu->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
  m_pFileMenu->Append(wxID_PREVIEW, _T("Print Preview"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(MAINMENU_IMPORT, _T("&Import...\tCtrl-M"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  m_pFileMenu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));
  GetDocumentManager()->FileHistoryAddFilesToMenu(m_pFileMenu);
  GetDocumentManager()->FileHistoryUseMenu(m_pFileMenu);

  wxMenu* edit_menu = new wxMenu;
  edit_menu->Append(IFMENU_EDIT_COPY, _T("Copy\tCtrl-C"));
  edit_menu->Append(IFMENU_EDIT_CUT, _T("Cut\tCtrl-X"));
  edit_menu->Append(IFMENU_EDIT_PASTE, _T("Paste\tCtrl-V"));

  wxMenu *view_menu = new wxMenu;
  view_menu->Append(IFMENU_VIEW_SCALE_MINMAX, _T("Display Scale S&et...\tCtrl-E"));
  view_menu->Append(IFMENU_VIEW_SCALE_AUTO, _T("Display Scale &Auto...\tCtrl-A"));
  view_menu->Append(IFMENU_VIEW_SCALE_FULL, _T("Display F&ull Scale\tCtrl-U"));

  m_pFilterMenu = new wxMenu;
  m_pFilterMenu->Append (IFMENU_FILTER_INVERTVALUES, _T("In&vert Values"));
  m_pFilterMenu->Append (IFMENU_FILTER_SQUARE, _T("&Square"));
  m_pFilterMenu->Append (IFMENU_FILTER_SQRT, _T("Square &Root"));
  m_pFilterMenu->Append (IFMENU_FILTER_LOG, _T("&Log"));
  m_pFilterMenu->Append (IFMENU_FILTER_EXP, _T("E&xp"));
  m_pFilterMenu->AppendSeparator();
#ifdef HAVE_FFT
  m_pFilterMenu->Append (IFMENU_FILTER_FFT, _T("2-D &FFT\tCtrl-2"));
  m_pFilterMenu->Append (IFMENU_FILTER_IFFT, _T("2-D &IFFT\tAlt-2"));
  m_pFilterMenu->Append (IFMENU_FILTER_FFT_ROWS, _T("FFT Rows"));
  m_pFilterMenu->Append (IFMENU_FILTER_IFFT_ROWS, _T("IFFT Rows"));
  m_pFilterMenu->Append (IFMENU_FILTER_FFT_COLS, _T("FFT Columns"));
  m_pFilterMenu->Append (IFMENU_FILTER_IFFT_COLS, _T("IFFT Columns"));
  m_pFilterMenu->Append (IFMENU_FILTER_FOURIER, _T("2-D F&ourier"));
  m_pFilterMenu->Append (IFMENU_FILTER_INVERSE_FOURIER, _T("2-D Inverse Fo&urier"));
#else
  m_pFilterMenu->Append (IFMENU_FILTER_FOURIER, _T("&Fourier"));
  m_pFilterMenu->Append (IFMENU_FILTER_INVERSE_FOURIER, _T("&Inverse Fourier"));
#endif
  m_pFilterMenu->Append (IFMENU_FILTER_SHUFFLEFOURIERTONATURALORDER, _T("Shuffl&e Fourier to Natural Order"));
  m_pFilterMenu->Append (IFMENU_FILTER_SHUFFLENATURALTOFOURIERORDER, _T("Shuffle &Natural to Fourier Order"));
  m_pFilterMenu->AppendSeparator();
  m_pFilterMenu->Append (IFMENU_FILTER_MAGNITUDE, _T("&Magnitude"));
  m_pFilterMenu->Append (IFMENU_FILTER_PHASE, _T("&Phase"));
  m_pFilterMenu->Append (IFMENU_FILTER_REAL, _T("Re&al"));
  m_pFilterMenu->Append (IFMENU_FILTER_IMAGINARY, _T("Ima&ginary"));

  wxMenu* image_menu = new wxMenu;
  image_menu->Append (IFMENU_IMAGE_ADD, _T("&Add..."));
  image_menu->Append (IFMENU_IMAGE_SUBTRACT, _T("&Subtract..."));
  image_menu->Append (IFMENU_IMAGE_MULTIPLY, _T("&Multiply..."));
  image_menu->Append (IFMENU_IMAGE_DIVIDE, _T("&Divide..."));
  image_menu->AppendSeparator();
  image_menu->Append (IFMENU_IMAGE_SCALESIZE, _T("S&cale Size..."));
#if wxUSE_GLCANVAS
  image_menu->Append (IFMENU_IMAGE_CONVERT3D, _T("Convert &3-D\tCtrl-3"));
#endif

  m_pMenuAnalyze = new wxMenu;
  m_pMenuAnalyze->Append (IFMENU_PLOT_ROW, _T("Plot &Row"));
  m_pMenuAnalyze->Append (IFMENU_PLOT_COL, _T("Plot &Column"));
  m_pMenuAnalyze->Append (IFMENU_PLOT_HISTOGRAM, _T("Plot &Histogram"));
  m_pMenuAnalyze->AppendSeparator();
  m_pMenuAnalyze->Append (IFMENU_PLOT_FFT_ROW, _T("P&lot FFT Row"));
  m_pMenuAnalyze->Append (IFMENU_PLOT_FFT_COL, _T("Plo&t FFT Column"));
  m_pMenuAnalyze->AppendSeparator();
  m_pMenuAnalyze->Append (IFMENU_COMPARE_IMAGES, _T("Compare &Images..."));
  m_pMenuAnalyze->Append (IFMENU_COMPARE_ROW, _T("Compare Ro&w"));
  m_pMenuAnalyze->Append (IFMENU_COMPARE_COL, _T("Compare Colu&mn"));
  m_pMenuAnalyze->Enable (IFMENU_PLOT_ROW, false);
  m_pMenuAnalyze->Enable (IFMENU_PLOT_COL, false);
  m_pMenuAnalyze->Enable (IFMENU_COMPARE_ROW, false);
  m_pMenuAnalyze->Enable (IFMENU_COMPARE_COL, false);
  m_pMenuAnalyze->Enable (IFMENU_PLOT_FFT_ROW, false);
  m_pMenuAnalyze->Enable (IFMENU_PLOT_FFT_COL, false);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append (MAINMENU_HELP_TIPS, _T("&Tips"));
  help_menu->Append (IDH_QUICKSTART, _T("&Quick Start"));
  help_menu->Append(MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append(m_pFileMenu, _T("&File"));
  menu_bar->Append(edit_menu, _T("&Edit"));
  menu_bar->Append(view_menu, _T("&View"));
  menu_bar->Append(image_menu, _T("&Image"));
  menu_bar->Append(m_pFilterMenu, _T("Fi&lter"));
  menu_bar->Append(m_pMenuAnalyze, _T("&Analyze"));
  menu_bar->Append(help_menu, _T("&Help"));

  subframe->SetMenuBar(menu_bar);

  subframe->Centre(wxBOTH);

  wxAcceleratorEntry accelEntries[10];
  accelEntries[0].Set (wxACCEL_CTRL, static_cast<int>('A'), IFMENU_VIEW_SCALE_AUTO);
  accelEntries[1].Set (wxACCEL_CTRL, static_cast<int>('U'), IFMENU_VIEW_SCALE_FULL);
  accelEntries[2].Set (wxACCEL_CTRL, static_cast<int>('E'), IFMENU_VIEW_SCALE_MINMAX);
  accelEntries[3].Set (wxACCEL_CTRL, static_cast<int>('I'), IFMENU_FILE_PROPERTIES);
  accelEntries[4].Set (wxACCEL_CTRL, static_cast<int>('C'), IFMENU_EDIT_COPY);
  accelEntries[5].Set (wxACCEL_CTRL, static_cast<int>('X'), IFMENU_EDIT_CUT);
  accelEntries[6].Set (wxACCEL_CTRL, static_cast<int>('V'), IFMENU_EDIT_PASTE);
  int iEntry = 7;
#ifdef HAVE_FFT
  accelEntries[iEntry++].Set (wxACCEL_CTRL, static_cast<int>('2'), IFMENU_FILTER_FFT);
  accelEntries[iEntry++].Set (wxACCEL_ALT,  static_cast<int>('2'), IFMENU_FILTER_IFFT);
#endif
#if wxUSE_GLCANVAS
  accelEntries[iEntry++].Set (wxACCEL_CTRL, static_cast<int>('3'), IFMENU_IMAGE_CONVERT3D);
#endif

  wxAcceleratorTable accelTable (iEntry, accelEntries);
  subframe->SetAcceleratorTable (accelTable);

  return subframe;
}


bool
ImageFileView::OnCreate (wxDocument *doc, long WXUNUSED(flags) )
{
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  m_dAutoScaleFactor = 1.;

  m_pFrame = CreateChildFrame(doc, this);
  SetFrame (m_pFrame);
  m_pCanvas = CreateCanvas (m_pFrame);
  m_pFrame->SetClientSize (m_pCanvas->GetBestSize());
  m_pCanvas->SetClientSize (m_pCanvas->GetBestSize());
  m_pFrame->SetTitle(doc->GetFilename());

  m_pFrame->Show(true);
  Activate(true);

  return true;
}

void
ImageFileView::setInitialClientSize ()
{
  if (m_pFrame && m_pCanvas) {
    wxSize bestSize = m_pCanvas->GetBestSize();

    m_pFrame->SetClientSize (bestSize);
    m_pFrame->Show (true);
    m_pFrame->SetFocus();
  }
}

void
ImageFileView::OnDraw (wxDC* dc)
{
  if (m_pBitmap && m_pBitmap->Ok()) {
#ifdef DEBUG
    *theApp->getLog() << _T("Drawing bitmap\n");
#endif
    dc->DrawBitmap(*m_pBitmap, 0, 0, false);
  }

  int xCursor, yCursor;
  if (m_pCanvas->GetCurrentCursor (xCursor, yCursor))
    m_pCanvas->DrawRubberBandCursor (*dc, xCursor, yCursor);
}


void
ImageFileView::OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
  const ImageFile& rIF = GetDocument()->getImageFile();
  if (m_pFilterMenu && rIF.isComplex()) {
    m_pFilterMenu->Enable(IFMENU_FILTER_REAL, true);
    m_pFilterMenu->Enable(IFMENU_FILTER_IMAGINARY, true);
    m_pFilterMenu->Enable(IFMENU_FILTER_PHASE, true);
  } else {
    m_pFilterMenu->Enable(IFMENU_FILTER_REAL, false);
    m_pFilterMenu->Enable(IFMENU_FILTER_IMAGINARY, false);
    m_pFilterMenu->Enable(IFMENU_FILTER_PHASE, false);
  }
  ImageFileArrayConst v = rIF.getArray();
  int nx = rIF.nx();
  int ny = rIF.ny();
  if (v != NULL && nx != 0 && ny != 0) {
    if (! m_bMinSpecified || ! m_bMaxSpecified) {
      double min, max;
      rIF.getMinMax (min, max);
      if (! m_bMinSpecified)
        m_dMinPixel = min;
      if (! m_bMaxSpecified)
        m_dMaxPixel = max;
    }
    double scaleWidth = m_dMaxPixel - m_dMinPixel;

    unsigned char* imageData = new unsigned char [nx * ny * 3];
    if (! imageData) {
      sys_error (ERR_SEVERE, "Unable to allocate memory for Image display");
      return;
    }
    for (int ix = 0; ix < nx; ix++) {
      for (int iy = 0; iy < ny; iy++) {
        double scaleValue = ((v[ix][iy] - m_dMinPixel) / scaleWidth) * 255;
        int intensity = static_cast<int>(scaleValue + 0.5);
        intensity = clamp (intensity, 0, 255);
        int baseAddr = ((ny - 1 - iy) * nx + ix) * 3;
        imageData[baseAddr] = imageData[baseAddr+1] = imageData[baseAddr+2] = intensity;
      }
    }
    wxImage image (nx, ny, imageData, true);
    if (m_pBitmap) {
      delete m_pBitmap;
      m_pBitmap = NULL;
    }
#ifdef DEBUG
    *theApp->getLog() << _T("Making new bitmap\n");
#endif
    m_pBitmap = new wxBitmap (image);
    delete imageData;
    m_pCanvas->SetScrollbars(20, 20, nx/20, ny/20);
    m_pCanvas->SetBackgroundColour(*wxWHITE);
  }

  if (m_pCanvas)
    m_pCanvas->Refresh();
}

bool
ImageFileView::OnClose (bool deleteWindow)
{
  if (! GetDocument() || ! GetDocument()->Close())
    return false;

  Activate (false);
  if (m_pCanvas) {
    m_pCanvas->setView(NULL);
    m_pCanvas = NULL;
  }
  wxString s(theApp->GetAppName());
  if (m_pFrame)
    m_pFrame->SetTitle(s);

  SetFrame(NULL);

  if (deleteWindow) {
    delete m_pFrame;
    m_pFrame = NULL;
    if (GetDocument() && GetDocument()->getBadFileOpen())
      ::wxYield();  // wxWindows bug workaround
  }

  return true;
}

void
ImageFileView::OnEditCopy (wxCommandEvent& event)
{
  wxBitmapDataObject *pBitmapObject = new wxBitmapDataObject;

  if (m_pBitmap)
    pBitmapObject->SetBitmap (*m_pBitmap);

  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData (pBitmapObject);
    wxTheClipboard->Close();
  }
}

void
ImageFileView::OnEditCut (wxCommandEvent& event)
{
  OnEditCopy (event);
  ImageFile& rIF = GetDocument()->getImageFile();
  int nx = rIF.nx();
  int ny = rIF.ny();
  ImageFile* pIF = new ImageFile (nx, ny);
  pIF->arrayDataClear();
  GetDocument()->setImageFile (pIF); // deletes old IF
  OnUpdate(this, NULL);
  GetDocument()->UpdateAllViews();
  if (theApp->getAskDeleteNewDocs())
    GetDocument()->Modify (true);
}

void
ImageFileView::OnEditPaste (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();

  if (wxTheClipboard->Open()) {
    wxBitmap bitmap;
    if (wxTheClipboard->IsSupported (wxDF_BITMAP)) {
      wxBitmapDataObject bitmapObject;
      wxTheClipboard->GetData (bitmapObject);
      bitmap = bitmapObject.GetBitmap ();
    }
    wxTheClipboard->Close();

    int nx = rIF.nx();
    int ny = rIF.ny();
    bool bMonochrome = false;

    if (bitmap.Ok() == true && bitmap.GetWidth() == nx && bitmap.GetHeight() == ny) {
      wxImage image (bitmap.ConvertToImage());
      double dScale3 = 3 * 255;
      unsigned char* pixels = image.GetData();
      ImageFileArray v = rIF.getArray();
      for (unsigned int ix = 0; ix < rIF.nx(); ix++) {
        for (unsigned int iy = 0; iy < rIF.ny(); iy++) {
          unsigned int iBase = 3 * (iy * nx + ix);
          if (ix == 0 && iy == 0 && (pixels[iBase] == pixels[iBase+1] && pixels[iBase+1] == pixels[iBase+2]))
            bMonochrome = true;
          if (bMonochrome) {
            v[ix][ny - 1 - iy] = (pixels[iBase]+pixels[iBase+1]+pixels[iBase+2]) / dScale3;
          } else {
            double dR = pixels[iBase] / 255.;
            double dG = pixels[iBase+1] / 255.;
            double dB = pixels[iBase+2] / 255.;
            v[ix][ny - 1 - iy] = ImageFile::colorToGrayscale (dR, dG, dB);
          }
        }
      }
      OnUpdate(this, NULL);
      GetDocument()->UpdateAllViews();
      if (theApp->getAskDeleteNewDocs())
        GetDocument()->Modify(true);
    }
  }
}

void
ImageFileView::OnExport (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  int nx = rIF.nx();
  int ny = rIF.ny();
  if (v != NULL && nx != 0 && ny != 0) {
    if (! m_bMinSpecified || ! m_bMaxSpecified) {
      double min, max;
      rIF.getMinMax (min, max);
      if (! m_bMinSpecified)
        m_dMinPixel = min;
      if (! m_bMaxSpecified)
        m_dMaxPixel = max;
    }

    DialogExportParameters dialogExport (getFrameForChild(), m_iDefaultExportFormatID);
    if (dialogExport.ShowModal() == wxID_OK) {
      wxString strFormatName (dialogExport.getFormatName (), wxConvUTF8);
      m_iDefaultExportFormatID = ImageFile::convertExportFormatNameToID (strFormatName.mb_str(wxConvUTF8));

      wxString strExt;
      wxString strWildcard;
      if (m_iDefaultExportFormatID == ImageFile::EXPORT_FORMAT_PGM || m_iDefaultExportFormatID == ImageFile::EXPORT_FORMAT_PGMASCII) {
        strExt = _T(".pgm");
        strWildcard = _T("PGM Files (*.pgm)|*.pgm");
      }
#ifdef HAVE_PNG
      else if (m_iDefaultExportFormatID == ImageFile::EXPORT_FORMAT_PNG || m_iDefaultExportFormatID == ImageFile::EXPORT_FORMAT_PNG16) {
        strExt = _T(".png");
        strWildcard = _T("PNG Files (*.png)|*.png");
      }
#endif
#ifdef HAVE_CTN_DICOM
      else if (m_iDefaultExportFormatID == ImageFile::EXPORT_FORMAT_DICOM) {
        strExt = _T("");
        strWildcard = _T("DICOM Files (*.*)|*.*");
      }
#endif
      else if (m_iDefaultExportFormatID == ImageFile::EXPORT_FORMAT_TEXT) {
        strExt = _T(".txt");
        strWildcard = _T("Text (*.txt)|*.txt");
      }
      else {
        strExt = _T("");
        strWildcard = _T("Miscellaneous (*.*)|*.*");
      }

#if WXWIN_COMPATIBILITY_2_4
      const wxString& strFilename = wxFileSelector (_T("Export Filename"), _T(""),
        _T(""), strExt, strWildcard, wxOVERWRITE_PROMPT | wxHIDE_READONLY | wxSAVE);
#else
      const wxString& strFilename = wxFileSelector (_T("Export Filename"), _T(""),
        _T(""), strExt, strWildcard, wxOVERWRITE_PROMPT | wxSAVE);
#endif
      if (strFilename) {
        rIF.exportImage (strFormatName.mb_str(wxConvUTF8), strFilename.mb_str(wxConvUTF8), 1, 1, m_dMinPixel, m_dMaxPixel);
        *theApp->getLog() << _T("Exported file ") << strFilename << _T("\n");
      }
    }
  }
}

void
ImageFileView::OnScaleSize (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  unsigned int iOldNX = rIF.nx();
  unsigned int iOldNY = rIF.ny();

  DialogGetXYSize dialogGetXYSize (getFrameForChild(), _T("Set New X & Y Dimensions"), iOldNX, iOldNY);
  if (dialogGetXYSize.ShowModal() == wxID_OK) {
    unsigned int iNewNX = dialogGetXYSize.getXSize();
    unsigned int iNewNY = dialogGetXYSize.getYSize();
    std::ostringstream os;
    os << "Scale Size from (" << iOldNX << "," << iOldNY << ") to (" << iNewNX << "," << iNewNY << ")";
    ImageFileDocument* pScaledDoc = theApp->newImageDoc();
    if (! pScaledDoc) {
      sys_error (ERR_SEVERE, "Unable to create image file");
      return;
    }
    ImageFile& rScaledIF = pScaledDoc->getImageFile();
    rScaledIF.setArraySize (iNewNX, iNewNY);
    rScaledIF.labelsCopy (rIF);
    rScaledIF.labelAdd (os.str().c_str());
    rIF.scaleImage (rScaledIF);
    *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
    if (theApp->getAskDeleteNewDocs())
      pScaledDoc->Modify (true);
    OnUpdate(this, NULL);
    pScaledDoc->UpdateAllViews (this);
    pScaledDoc->getView()->setInitialClientSize();
    pScaledDoc->Activate();
  }
}

#if wxUSE_GLCANVAS
void
ImageFileView::OnConvert3d (wxCommandEvent& event)
{
  ImageFile& rIF = GetDocument()->getImageFile();
  Graph3dFileDocument* pGraph3d = theApp->newGraph3dDoc();
  pGraph3d->getView()->getFrame()->Show (false);
  pGraph3d->setBadFileOpen();
  pGraph3d->createFromImageFile (rIF);
  pGraph3d->UpdateAllViews();
  pGraph3d->getView()->getFrame()->Show (true);
  pGraph3d->getView()->Activate(true);
  ::wxYield();
  pGraph3d->getView()->getCanvas()->SetFocus();
}
#endif

void
ImageFileView::OnPlotRow (wxCommandEvent& event)
{
  int xCursor, yCursor;
  if (! m_pCanvas->GetCurrentCursor (xCursor, yCursor)) {
    wxMessageBox (_T("No row selected. Please use left mouse button on image to select column"),_T("Error"));
    return;
  }

  const ImageFile& rIF = GetDocument()->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  ImageFileArrayConst vImag = rIF.getImaginaryArray();
  int nx = rIF.nx();
  int ny = rIF.ny();

  if (v != NULL && yCursor < ny) {
    double* pX = new double [nx];
    double* pYReal = new double [nx];
    double *pYImag = NULL;
    double *pYMag = NULL;
    if (rIF.isComplex()) {
      pYImag = new double [nx];
      pYMag = new double [nx];
    }
    for (int i = 0; i < nx; i++) {
      pX[i] = i;
      pYReal[i] = v[i][yCursor];
      if (rIF.isComplex()) {
        pYImag[i] = vImag[i][yCursor];
        pYMag[i] = ::sqrt (v[i][yCursor] * v[i][yCursor] + vImag[i][yCursor] * vImag[i][yCursor]);
      }
    }
    PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
    if (! pPlotDoc) {
      sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
    } else {
      PlotFile& rPlotFile = pPlotDoc->getPlotFile();
      std::ostringstream os;
      os << "Row " << yCursor;
      std::string title("title ");
      title += os.str();
      rPlotFile.addEzsetCommand (title.c_str());
      rPlotFile.addEzsetCommand ("xlabel Column");
      rPlotFile.addEzsetCommand ("ylabel Pixel Value");
      rPlotFile.addEzsetCommand ("lxfrac 0");
      rPlotFile.addEzsetCommand ("box");
      rPlotFile.addEzsetCommand ("grid");
      rPlotFile.addEzsetCommand ("curve 1");
      rPlotFile.addEzsetCommand ("color 1");
      if (rIF.isComplex()) {
        rPlotFile.addEzsetCommand ("dash 1");
        rPlotFile.addEzsetCommand ("curve 2");
        rPlotFile.addEzsetCommand ("color 4");
        rPlotFile.addEzsetCommand ("dash 3");
        rPlotFile.addEzsetCommand ("curve 3");
        rPlotFile.addEzsetCommand ("color 0");
        rPlotFile.addEzsetCommand ("solid");
        rPlotFile.setCurveSize (4, nx);
      } else
        rPlotFile.setCurveSize (2, nx);
      rPlotFile.addColumn (0, pX);
      rPlotFile.addColumn (1, pYReal);
      if (rIF.isComplex()) {
        rPlotFile.addColumn (2, pYImag);
        rPlotFile.addColumn (3, pYMag);
      }
      for (unsigned int iL = 0; iL < rIF.nLabels(); iL++)
        rPlotFile.addDescription (rIF.labelGet(iL).getLabelString().c_str());
      os << ": plot of " << wxConvUTF8.cWX2MB(dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      rPlotFile.addDescription (os.str().c_str());
    }
    delete pX;
    delete pYReal;
    if (rIF.isComplex()) {
      delete pYImag;
      delete pYMag;
    }
    if (theApp->getAskDeleteNewDocs())
      pPlotDoc->Modify (true);
    pPlotDoc->getView()->getFrame()->Show(true);
    pPlotDoc->UpdateAllViews ();
    pPlotDoc->Activate();
  }
}

void
ImageFileView::OnPlotCol (wxCommandEvent& event)
{
  int xCursor, yCursor;
  if (! m_pCanvas->GetCurrentCursor (xCursor, yCursor)) {
    wxMessageBox (_T("No column selected. Please use left mouse button on image to select column"),_T("Error"));
    return;
  }

  const ImageFile& rIF = GetDocument()->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  ImageFileArrayConst vImag = rIF.getImaginaryArray();
  int nx = rIF.nx();
  int ny = rIF.ny();

  if (v != NULL && xCursor < nx) {
    double* const pX = new double [ny];
    double* const pYReal = new double [ny];
    double* pYImag = NULL;
    double* pYMag = NULL;
    if (rIF.isComplex()) {
      pYImag = new double [ny];
      pYMag = new double [ny];
    }
    for (int i = 0; i < ny; i++) {
      pX[i] = i;
      pYReal[i] = v[xCursor][i];
      if (rIF.isComplex()) {
        pYImag[i] = vImag[xCursor][i];
        pYMag[i] = ::sqrt (v[xCursor][i] * v[xCursor][i] + vImag[xCursor][i] * vImag[xCursor][i]);
      }
    }
    PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
    if (! pPlotDoc) {
      sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
    } else {
      PlotFile& rPlotFile = pPlotDoc->getPlotFile();
      std::ostringstream os;
      os << "Column " << xCursor;
      std::string title("title ");
      title += os.str();
      rPlotFile.addEzsetCommand (title.c_str());
      rPlotFile.addEzsetCommand ("xlabel Row");
      rPlotFile.addEzsetCommand ("ylabel Pixel Value");
      rPlotFile.addEzsetCommand ("lxfrac 0");
      rPlotFile.addEzsetCommand ("box");
      rPlotFile.addEzsetCommand ("grid");
      rPlotFile.addEzsetCommand ("curve 1");
      rPlotFile.addEzsetCommand ("color 1");
      if (rIF.isComplex()) {
        rPlotFile.addEzsetCommand ("dash 1");
        rPlotFile.addEzsetCommand ("curve 2");
        rPlotFile.addEzsetCommand ("color 4");
        rPlotFile.addEzsetCommand ("dash 3");
        rPlotFile.addEzsetCommand ("curve 3");
        rPlotFile.addEzsetCommand ("color 0");
        rPlotFile.addEzsetCommand ("solid");
        rPlotFile.setCurveSize (4, ny);
      } else
        rPlotFile.setCurveSize (2, ny);
      rPlotFile.addColumn (0, pX);
      rPlotFile.addColumn (1, pYReal);
      if (rIF.isComplex()) {
        rPlotFile.addColumn (2, pYImag);
        rPlotFile.addColumn (3, pYMag);
      }
      for (unsigned int iL = 0; iL < rIF.nLabels(); iL++)
        rPlotFile.addDescription (rIF.labelGet(iL).getLabelString().c_str());
      os << " : plot of " << wxConvUTF8.cWX2MB(dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      rPlotFile.addDescription (os.str().c_str());
    }
    delete pX;
    delete pYReal;
    if (rIF.isComplex()) {
      delete pYImag;
      delete pYMag;
    }
    if (theApp->getAskDeleteNewDocs())
      pPlotDoc->Modify (true);
    pPlotDoc->getView()->getFrame()->Show(true);
    pPlotDoc->UpdateAllViews ();
    pPlotDoc->Activate();
  }
}

#ifdef HAVE_FFT
void
ImageFileView::OnPlotFFTRow (wxCommandEvent& event)
{
  int xCursor, yCursor;
  if (! m_pCanvas->GetCurrentCursor (xCursor, yCursor)) {
    wxMessageBox (_T("No row selected. Please use left mouse button on image to select column"),_T("Error"));
    return;
  }

  const ImageFile& rIF = GetDocument()->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  ImageFileArrayConst vImag = rIF.getImaginaryArray();
  int nx = rIF.nx();
  int ny = rIF.ny();

  if (v != NULL && yCursor < ny) {
    fftw_complex* pcIn = static_cast<fftw_complex*>(fftw_malloc (sizeof(fftw_complex) * nx));

    int i;
    for (i = 0; i < nx; i++) {
      pcIn[i][0] = v[i][yCursor];
      if (rIF.isComplex())
        pcIn[i][1] = vImag[i][yCursor];
      else
        pcIn[i][1] = 0;
    }

    fftw_plan plan = fftw_plan_dft_1d (nx, pcIn, pcIn, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);

    double* pX = new double [nx];
    double* pYReal = new double [nx];
    double* pYImag = new double [nx];
    double* pYMag = new double [nx];
    for (i = 0; i < nx; i++) {
      pX[i] = i;
      pYReal[i] = pcIn[i][0] / nx;
      pYImag[i] = pcIn[i][1] / nx;
      pYMag[i] = ::sqrt (pcIn[i][0] * pcIn[i][0] + pcIn[i][1] * pcIn[i][1]);
    }
    Fourier::shuffleFourierToNaturalOrder (pYReal, nx);
    Fourier::shuffleFourierToNaturalOrder (pYImag, nx);
    Fourier::shuffleFourierToNaturalOrder (pYMag, nx);

    PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
    if (! pPlotDoc) {
      sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
    } else {
      PlotFile& rPlotFile = pPlotDoc->getPlotFile();
      std::ostringstream os;
      os << "Row " << yCursor;
      std::string title("title ");
      title += os.str();
      rPlotFile.addEzsetCommand (title.c_str());
      rPlotFile.addEzsetCommand ("xlabel Column");
      rPlotFile.addEzsetCommand ("ylabel Pixel Value");
      rPlotFile.addEzsetCommand ("lxfrac 0");
      rPlotFile.addEzsetCommand ("curve 1");
      rPlotFile.addEzsetCommand ("color 1");
      rPlotFile.addEzsetCommand ("dash 1");
      rPlotFile.addEzsetCommand ("curve 2");
      rPlotFile.addEzsetCommand ("color 4");
      rPlotFile.addEzsetCommand ("dash 3");
      rPlotFile.addEzsetCommand ("curve 3");
      rPlotFile.addEzsetCommand ("color 0");
      rPlotFile.addEzsetCommand ("solid");
      rPlotFile.addEzsetCommand ("box");
      rPlotFile.addEzsetCommand ("grid");
      rPlotFile.setCurveSize (4, nx);
      rPlotFile.addColumn (0, pX);
      rPlotFile.addColumn (1, pYReal);
      rPlotFile.addColumn (2, pYImag);
      rPlotFile.addColumn (3, pYMag);
      for (unsigned int iL = 0; iL < rIF.nLabels(); iL++)
        rPlotFile.addDescription (rIF.labelGet(iL).getLabelString().c_str());
      os << ": FFT plot of " << wxConvUTF8.cWX2MB(dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      rPlotFile.addDescription (os.str().c_str());
    }
    delete pX;
    delete pYReal;
    delete pYImag;
    delete pYMag;
    fftw_free(pcIn);

    if (theApp->getAskDeleteNewDocs())
      pPlotDoc->Modify (true);
    pPlotDoc->getView()->getFrame()->Show(true);
    pPlotDoc->UpdateAllViews ();
    pPlotDoc->Activate();
  }
}

void
ImageFileView::OnPlotFFTCol (wxCommandEvent& event)
{
  int xCursor, yCursor;
  if (! m_pCanvas->GetCurrentCursor (xCursor, yCursor)) {
    wxMessageBox (_T("No column selected. Please use left mouse button on image to select column"),_T("Error"));
    return;
  }

  const ImageFile& rIF = GetDocument()->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  ImageFileArrayConst vImag = rIF.getImaginaryArray();
  int nx = rIF.nx();
  int ny = rIF.ny();

  if (v != NULL && xCursor < nx) {
    fftw_complex* pcIn = new fftw_complex [ny];
    double *pdTemp = new double [ny];

    int i;
    for (i = 0; i < ny; i++)
      pdTemp[i] = v[xCursor][i];
    Fourier::shuffleNaturalToFourierOrder (pdTemp, ny);
    for (i = 0; i < ny; i++)
      pcIn[i][0] = pdTemp[i];

    for (i = 0; i < ny; i++) {
      if (rIF.isComplex())
        pdTemp[i] = vImag[xCursor][i];
      else
        pdTemp[i] = 0;
    }
    Fourier::shuffleNaturalToFourierOrder (pdTemp, ny);
    for (i = 0; i < ny; i++)
      pcIn[i][1] = pdTemp[i];

    fftw_plan plan = fftw_plan_dft_1d (ny, pcIn, pcIn, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);

    double* pX = new double [ny];
    double* pYReal = new double [ny];
    double* pYImag = new double [ny];
    double* pYMag = new double [ny];
    for (i = 0; i < ny; i++) {
      pX[i] = i;
      pYReal[i] = pcIn[i][0] / ny;
      pYImag[i] = pcIn[i][1] / ny;
      pYMag[i] = ::sqrt (pcIn[i][0] * pcIn[i][0] + pcIn[i][1] * pcIn[i][1]);
    }

    PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
    if (! pPlotDoc) {
      sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
    } else {
      PlotFile& rPlotFile = pPlotDoc->getPlotFile();
      std::ostringstream os;
      os << "Column " << xCursor;
      std::string title("title ");
      title += os.str();
      rPlotFile.addEzsetCommand (title.c_str());
      rPlotFile.addEzsetCommand ("xlabel Column");
      rPlotFile.addEzsetCommand ("ylabel Pixel Value");
      rPlotFile.addEzsetCommand ("lxfrac 0");
      rPlotFile.addEzsetCommand ("curve 1");
      rPlotFile.addEzsetCommand ("color 1");
      rPlotFile.addEzsetCommand ("dash 1");
      rPlotFile.addEzsetCommand ("curve 2");
      rPlotFile.addEzsetCommand ("color 4");
      rPlotFile.addEzsetCommand ("dash 3");
      rPlotFile.addEzsetCommand ("curve 3");
      rPlotFile.addEzsetCommand ("color 0");
      rPlotFile.addEzsetCommand ("solid");
      rPlotFile.addEzsetCommand ("box");
      rPlotFile.addEzsetCommand ("grid");
      rPlotFile.setCurveSize (4, ny);
      rPlotFile.addColumn (0, pX);
      rPlotFile.addColumn (1, pYReal);
      rPlotFile.addColumn (2, pYImag);
      rPlotFile.addColumn (3, pYMag);
      for (unsigned int iL = 0; iL < rIF.nLabels(); iL++)
        rPlotFile.addDescription (rIF.labelGet(iL).getLabelString().c_str());
      os << ": FFT plot of " << wxConvUTF8.cWX2MB(dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      rPlotFile.addDescription (os.str().c_str());
    }
    delete pX;
    delete pYReal;
    delete pYImag;
    delete pYMag;
    delete pdTemp;
    delete [] pcIn;

    if (theApp->getAskDeleteNewDocs())
      pPlotDoc->Modify (true);
    pPlotDoc->getView()->getFrame()->Show(true);
    pPlotDoc->UpdateAllViews ();
    pPlotDoc->Activate();
  }
}
#endif

void
ImageFileView::OnCompareCol (wxCommandEvent& event)
{
  int xCursor, yCursor;
  if (! m_pCanvas->GetCurrentCursor (xCursor, yCursor)) {
    wxMessageBox (_T("No column selected. Please use left mouse button on image to select column"),_T("Error"));
    return;
  }

  std::vector<ImageFileDocument*> vecIFDoc;
  theApp->getCompatibleImages (GetDocument(), vecIFDoc);
  if (vecIFDoc.size() == 0) {
    wxMessageBox (_T("No compatible images for Column Comparison"), _T("Error"));
    return;
  }
  DialogGetComparisonImage dialogGetCompare (getFrameForChild(), _T("Get Comparison Image"), vecIFDoc, false);

  if (dialogGetCompare.ShowModal() == wxID_OK) {
    ImageFileDocument* pCompareDoc = dialogGetCompare.getImageFileDocument();
    const ImageFile& rIF = GetDocument()->getImageFile();
    const ImageFile& rCompareIF = pCompareDoc->getImageFile();

    ImageFileArrayConst v1 = rIF.getArray();
    ImageFileArrayConst v2 = rCompareIF.getArray();
    int nx = rIF.nx();
    int ny = rIF.ny();

    if (v1 != NULL && xCursor < nx) {
      double* pX = new double [ny];
      double* pY1 = new double [ny];
      double* pY2 = new double [ny];
      for (int i = 0; i < ny; i++) {
        pX[i] = i;
        pY1[i] = v1[xCursor][i];
        pY2[i] = v2[xCursor][i];
      }
      PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
      if (! pPlotDoc) {
        sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
      } else {
        PlotFile& rPlotFile = pPlotDoc->getPlotFile();
        std::ostringstream os;
        os << "Column " << xCursor << ": Comparison";
        std::string title("title ");
        title += os.str();
        rPlotFile.addEzsetCommand (title.c_str());
        rPlotFile.addEzsetCommand ("xlabel Row");
        rPlotFile.addEzsetCommand ("ylabel Pixel Value");
        rPlotFile.addEzsetCommand ("lxfrac 0");
        rPlotFile.addEzsetCommand ("curve 1");
        rPlotFile.addEzsetCommand ("color 2");
        rPlotFile.addEzsetCommand ("curve 2");
        rPlotFile.addEzsetCommand ("color 4");
        rPlotFile.addEzsetCommand ("dash 5");
        rPlotFile.addEzsetCommand ("box");
        rPlotFile.addEzsetCommand ("grid");
        rPlotFile.setCurveSize (3, ny);
        rPlotFile.addColumn (0, pX);
        rPlotFile.addColumn (1, pY1);
        rPlotFile.addColumn (2, pY2);

        unsigned int iL;
        for (iL = 0; iL < rIF.nLabels(); iL++) {
          std::ostringstream os;
          os << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str();
          os << ": " << rIF.labelGet(iL).getLabelString();
          rPlotFile.addDescription (os.str().c_str());
        }
        for (iL = 0; iL < rCompareIF.nLabels(); iL++) {
          std::ostringstream os;
          os << dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8);
          os << ": ";
          os << rCompareIF.labelGet(iL).getLabelString();
          rPlotFile.addDescription (os.str().c_str());
        }
        os << " between " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8) << " and "
           << dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8);
        *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
        rPlotFile.addDescription (os.str().c_str());
      }
      delete pX;
      delete pY1;
      delete pY2;
      if (theApp->getAskDeleteNewDocs())
        pPlotDoc->Modify (true);
      pPlotDoc->getView()->getFrame()->Show(true);
      pPlotDoc->UpdateAllViews ();
      pPlotDoc->Activate();
    }
  }
}

void
ImageFileView::OnCompareRow (wxCommandEvent& event)
{
  int xCursor, yCursor;
  if (! m_pCanvas->GetCurrentCursor (xCursor, yCursor)) {
    wxMessageBox (_T("No column selected. Please use left mouse button on image to select column"),_T("Error"));
    return;
  }

  std::vector<ImageFileDocument*> vecIFDoc;
  theApp->getCompatibleImages (GetDocument(), vecIFDoc);

  if (vecIFDoc.size() == 0) {
    wxMessageBox (_T("No compatible images for Row Comparison"), _T("Error"));
    return;
  }

  DialogGetComparisonImage dialogGetCompare (getFrameForChild(), _T("Get Comparison Image"), vecIFDoc, false);

  if (dialogGetCompare.ShowModal() == wxID_OK) {
    ImageFileDocument* pCompareDoc = dialogGetCompare.getImageFileDocument();
    const ImageFile& rIF = GetDocument()->getImageFile();
    const ImageFile& rCompareIF = pCompareDoc->getImageFile();

    ImageFileArrayConst v1 = rIF.getArray();
    ImageFileArrayConst v2 = rCompareIF.getArray();
    int nx = rIF.nx();
    int ny = rIF.ny();

    if (v1 != NULL && yCursor < ny) {
      double* pX = new double [nx];
      double* pY1 = new double [nx];
      double* pY2 = new double [nx];
      for (int i = 0; i < nx; i++) {
        pX[i] = i;
        pY1[i] = v1[i][yCursor];
        pY2[i] = v2[i][yCursor];
      }
      PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
      if (! pPlotDoc) {
        sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
      } else {
        PlotFile& rPlotFile = pPlotDoc->getPlotFile();
        std::ostringstream os;
        os << "Row " << yCursor << ": Comparison";
        std::string title("title ");
        title += os.str();
        rPlotFile.addEzsetCommand (title.c_str());
        rPlotFile.addEzsetCommand ("xlabel Column");
        rPlotFile.addEzsetCommand ("ylabel Pixel Value");
        rPlotFile.addEzsetCommand ("lxfrac 0");
        rPlotFile.addEzsetCommand ("curve 1");
        rPlotFile.addEzsetCommand ("color 2");
        rPlotFile.addEzsetCommand ("curve 2");
        rPlotFile.addEzsetCommand ("color 4");
        rPlotFile.addEzsetCommand ("dash 5");
        rPlotFile.addEzsetCommand ("box");
        rPlotFile.addEzsetCommand ("grid");
        rPlotFile.setCurveSize (3, nx);
        rPlotFile.addColumn (0, pX);
        rPlotFile.addColumn (1, pY1);
        rPlotFile.addColumn (2, pY2);
        unsigned int iL;
        for (iL = 0; iL < rIF.nLabels(); iL++) {
          std::ostringstream os;
          os << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8);
          os << ": ";
          os << rIF.labelGet(iL).getLabelString();
          rPlotFile.addDescription (os.str().c_str());
        }
        for (iL = 0; iL < rCompareIF.nLabels(); iL++) {
          std::ostringstream os;
          os << dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8) << ": "
             << rCompareIF.labelGet(iL).getLabelString();
          rPlotFile.addDescription (os.str().c_str());
        }
        os << " between " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8) << " and "
           << dynamic_cast<wxFrame*>(pCompareDoc->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8);
        *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
        rPlotFile.addDescription (os.str().c_str());
      }
      delete pX;
      delete pY1;
      delete pY2;
      if (theApp->getAskDeleteNewDocs())
        pPlotDoc->Modify (true);
      pPlotDoc->getView()->getFrame()->Show(true);
      pPlotDoc->UpdateAllViews ();
      pPlotDoc->Activate();
    }
  }
}

static int NUMBER_HISTOGRAM_BINS = 256;

void
ImageFileView::OnPlotHistogram (wxCommandEvent& event)
{
  const ImageFile& rIF = GetDocument()->getImageFile();
  ImageFileArrayConst v = rIF.getArray();
  int nx = rIF.nx();
  int ny = rIF.ny();

  if (v != NULL && nx > 0 && ny > 0) {
    PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
    if (! pPlotDoc) {
      sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
      return;
    }

    double* pX = new double [NUMBER_HISTOGRAM_BINS];
    double* pY = new double [NUMBER_HISTOGRAM_BINS];
    double dMin, dMax;
    rIF.getMinMax (dMin, dMax);
    double dBinWidth = (dMax - dMin) / NUMBER_HISTOGRAM_BINS;

    for (int i = 0; i < NUMBER_HISTOGRAM_BINS; i++) {
      pX[i] = dMin + (i + 0.5) * dBinWidth;
      pY[i] = 0;
    }
    for (int ix = 0; ix < nx; ix++)
      for (int iy = 0; iy < ny; iy++) {
        int iBin = nearest<int> ((v[ix][iy] - dMin) / dBinWidth);
        if (iBin >= 0 && iBin < NUMBER_HISTOGRAM_BINS)
          pY[iBin] += 1;
      }

      PlotFile& rPlotFile = pPlotDoc->getPlotFile();
      std::ostringstream os;
      os << "Histogram";
      std::string title("title ");
      title += os.str();
      rPlotFile.addEzsetCommand (title.c_str());
      rPlotFile.addEzsetCommand ("xlabel Pixel Value");
      rPlotFile.addEzsetCommand ("ylabel Count");
      rPlotFile.addEzsetCommand ("box");
      rPlotFile.addEzsetCommand ("grid");
      rPlotFile.setCurveSize (2, NUMBER_HISTOGRAM_BINS);
      rPlotFile.addColumn (0, pX);
      rPlotFile.addColumn (1, pY);
      for (unsigned int iL = 0; iL < rIF.nLabels(); iL++) {
        std::ostringstream os;
        os << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().mb_str(wxConvUTF8);
        os << ": " << rIF.labelGet(iL).getLabelString();
        rPlotFile.addDescription (os.str().c_str());
      }
      os << " plot of " << wxConvUTF8.cWX2MB(dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str());
      *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
      rPlotFile.addDescription (os.str().c_str());
      delete pX;
      delete pY;
      if (theApp->getAskDeleteNewDocs())
        pPlotDoc->Modify (true);
      pPlotDoc->getView()->getFrame()->Show(true);
      pPlotDoc->UpdateAllViews ();
      pPlotDoc->Activate();
  }
}


// PhantomCanvas

PhantomCanvas::PhantomCanvas (PhantomFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style)
  : wxScrolledWindow(frame, -1, pos, size, style), m_pView(v)
{
}

PhantomCanvas::~PhantomCanvas ()
{
  m_pView = NULL;
}

void
PhantomCanvas::OnDraw (wxDC& dc)
{
  if (m_pView)
    m_pView->OnDraw(& dc);
}

wxSize
PhantomCanvas::GetBestSize() const
{
  if (! m_pView)
    return wxSize(0,0);

  int xSize, ySize;
  theApp->getMainFrame()->GetClientSize (&xSize, &ySize);
  xSize = maxValue<int> (xSize, ySize);
#ifdef CTSIM_MDI
  ySize = xSize = (xSize / 4);
#else
  xSize = ySize = static_cast<int>(ySize * .7);
#endif

  return wxSize (xSize, ySize);
}



// PhantomFileView

IMPLEMENT_DYNAMIC_CLASS(PhantomFileView, wxView)

BEGIN_EVENT_TABLE(PhantomFileView, wxView)
EVT_MENU(PHMMENU_FILE_PROPERTIES, PhantomFileView::OnProperties)
EVT_MENU(PHMMENU_PROCESS_RASTERIZE, PhantomFileView::OnRasterize)
EVT_MENU(PHMMENU_PROCESS_PROJECTIONS, PhantomFileView::OnProjections)
END_EVENT_TABLE()

PhantomFileView::PhantomFileView()
: wxView(), m_pFrame(NULL), m_pCanvas(NULL), m_pFileMenu(0)
{
#if defined(DEBUG) || defined(_DEBUG)
  m_iDefaultNDet = 165;
  m_iDefaultNView = 180;
  m_iDefaultNSample = 1;
#else
  m_iDefaultNDet = 367;
  m_iDefaultNView = 320;
  m_iDefaultNSample = 2;
#endif
  m_iDefaultOffsetView = 0;
  m_dDefaultRotation = 1;
  m_dDefaultFocalLength = 2;
  m_dDefaultCenterDetectorLength = 2;
  m_dDefaultViewRatio = 1;
  m_dDefaultScanRatio = 1;
  m_iDefaultGeometry = Scanner::GEOMETRY_PARALLEL;
  m_iDefaultTrace = Trace::TRACE_NONE;

#ifdef DEBUG
  m_iDefaultRasterNX = 115;
  m_iDefaultRasterNY = 115;
  m_iDefaultRasterNSamples = 1;
#else
  m_iDefaultRasterNX = 256;
  m_iDefaultRasterNY = 256;
  m_iDefaultRasterNSamples = 2;
#endif
  m_dDefaultRasterViewRatio = 1;
}

PhantomFileView::~PhantomFileView()
{
  GetDocumentManager()->FileHistoryRemoveMenu (m_pFileMenu);
  GetDocumentManager()->ActivateView(this, FALSE);
}

void
PhantomFileView::OnProperties (wxCommandEvent& event)
{
  const int idPhantom = GetDocument()->getPhantomID();
  const wxString& namePhantom = GetDocument()->getPhantomName();
  std::ostringstream os;
  os << "Phantom " << namePhantom.c_str() << " (" << idPhantom << ")" << "\n";
  const Phantom& rPhantom = GetDocument()->getPhantom();
  rPhantom.printDefinitions (os);
#if DEBUG
  rPhantom.print (os);
#endif
  *theApp->getLog() << _T(">>>>\n") << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("<<<<\n");
  wxMessageBox (wxConvUTF8.cMB2WX(os.str().c_str()), _T("Phantom Properties"));
  GetDocument()->Activate();
}


void
PhantomFileView::OnProjections (wxCommandEvent& event)
{
  DialogGetProjectionParameters dialogProjection (getFrameForChild(),
    m_iDefaultNDet, m_iDefaultNView, m_iDefaultOffsetView, m_iDefaultNSample, m_dDefaultRotation,
    m_dDefaultFocalLength, m_dDefaultCenterDetectorLength, m_dDefaultViewRatio, m_dDefaultScanRatio,
    m_iDefaultGeometry, m_iDefaultTrace);
  int retVal = dialogProjection.ShowModal();
  if (retVal != wxID_OK)
    return;

  m_iDefaultNDet = dialogProjection.getNDet();
  m_iDefaultNView = dialogProjection.getNView();
  m_iDefaultOffsetView = dialogProjection.getOffsetView();
  m_iDefaultNSample = dialogProjection.getNSamples();
  m_iDefaultTrace = dialogProjection.getTrace();
  m_dDefaultRotation = dialogProjection.getRotAngle();
  m_dDefaultFocalLength = dialogProjection.getFocalLengthRatio();
  m_dDefaultCenterDetectorLength = dialogProjection.getCenterDetectorLengthRatio();
  m_dDefaultViewRatio = dialogProjection.getViewRatio();
  m_dDefaultScanRatio = dialogProjection.getScanRatio();
  wxString sGeometry (dialogProjection.getGeometry(), wxConvUTF8);
  m_iDefaultGeometry = Scanner::convertGeometryNameToID (sGeometry.mb_str(wxConvUTF8));
  double dRotationRadians = m_dDefaultRotation;
  m_dDefaultRotation /= TWOPI;  // convert back to fraction of a circle

  if (m_iDefaultNDet <= 0 || m_iDefaultNView <= 0 || sGeometry == _T(""))
    return;

  const Phantom& rPhantom = GetDocument()->getPhantom();
  Scanner theScanner (rPhantom, sGeometry.mb_str(wxConvUTF8), m_iDefaultNDet, m_iDefaultNView, m_iDefaultOffsetView, m_iDefaultNSample,
    dRotationRadians, m_dDefaultFocalLength, m_dDefaultCenterDetectorLength, m_dDefaultViewRatio, m_dDefaultScanRatio);
  if (theScanner.fail()) {
    wxString msg = _T("Failed making scanner\n");
    msg += wxConvUTF8.cMB2WX(theScanner.failMessage().c_str());
    *theApp->getLog() << msg << _T("\n");
    wxMessageBox (msg, _T("Error"));
    return;
  }

  std::ostringstream os;
  os << "Projections for " << rPhantom.name().c_str()
     << ": nDet=" << m_iDefaultNDet
     << ", nView=" << m_iDefaultNView
     << ", gantry offset=" << m_iDefaultOffsetView
     << ", nSamples=" << m_iDefaultNSample
     << ", RotAngle=" << m_dDefaultRotation
     << ", FocalLengthRatio=" << m_dDefaultFocalLength
     << ", CenterDetectorLengthRatio=" << m_dDefaultCenterDetectorLength
     << ", ViewRatio=" << m_dDefaultViewRatio
     << ", ScanRatio=" << m_dDefaultScanRatio
     << ", Geometry=" << sGeometry.mb_str(wxConvUTF8)
     << ", FanBeamAngle=" << convertRadiansToDegrees (theScanner.fanBeamAngle());

  Timer timer;
  Projections* pProj = NULL;
  if (m_iDefaultTrace > Trace::TRACE_CONSOLE) {
    pProj = new Projections;
    pProj->initFromScanner (theScanner);

    ProjectionsDialog dialogProjections (theScanner, *pProj, rPhantom, m_iDefaultTrace, dynamic_cast<wxWindow*>(getFrameForChild()));
    for (int iView = 0; iView < pProj->nView(); iView++) {
      ::wxYield();
      if (dialogProjections.isCancelled() || ! dialogProjections.projectView (iView)) {
        delete pProj;
        return;
      }
      ::wxYield();
      while (dialogProjections.isPaused()) {
        ::wxYield();
        ::wxMilliSleep(50);
      }
    }
  } else {
#if HAVE_WXTHREADS
    if (theApp->getUseBackgroundTasks()) {
      ProjectorSupervisorThread* pProjector = new ProjectorSupervisorThread
        (this, m_iDefaultNDet, m_iDefaultNView, m_iDefaultOffsetView, 
         sGeometry.mb_str(wxConvUTF8), m_iDefaultNSample, dRotationRadians,
         m_dDefaultFocalLength, m_dDefaultCenterDetectorLength, m_dDefaultViewRatio, 
         m_dDefaultScanRatio, wxConvUTF8.cMB2WX(os.str().c_str()));
      if (pProjector->Create() != wxTHREAD_NO_ERROR) {
        sys_error (ERR_SEVERE, "Error creating projector thread");
        delete pProjector;
        return;
      }
      pProjector->SetPriority(60);
      pProjector->Run();
      return;
    } else
#endif // HAVE_WXTHREADS
    {
      pProj = new Projections;
      pProj->initFromScanner (theScanner);
      wxProgressDialog dlgProgress (_T("Projection"), _T("Projection Progress"), pProj->nView() + 1, getFrameForChild(), wxPD_CAN_ABORT );
      for (int i = 0; i < pProj->nView(); i++) {
        //theScanner.collectProjections (*pProj, rPhantom, i, 1, true, m_iDefaultTrace);
        theScanner.collectProjections (*pProj, rPhantom, i, 1, theScanner.offsetView(), true, m_iDefaultTrace);
        if ((i + 1) % ITER_PER_UPDATE == 0)
          if (! dlgProgress.Update (i+1)) {
            delete pProj;
            return;
          }
      }
    }
  }

  *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
  pProj->setRemark (os.str());
  pProj->setCalcTime (timer.timerEnd());

  ProjectionFileDocument* pProjectionDoc = theApp->newProjectionDoc();
  if (! pProjectionDoc) {
    sys_error (ERR_SEVERE, "Unable to create projection document");
    return;
  }
  pProjectionDoc->setProjections (pProj);
  if (theApp->getAskDeleteNewDocs())
    pProjectionDoc-> Modify(true);
  OnUpdate(this, NULL);
  pProjectionDoc->UpdateAllViews (this);
  pProjectionDoc->getView()->setInitialClientSize();
  pProjectionDoc->Activate();
}
void
PhantomFileView::OnRasterize (wxCommandEvent& event)
{
  DialogGetRasterParameters dialogRaster (getFrameForChild(), m_iDefaultRasterNX, m_iDefaultRasterNY,
    m_iDefaultRasterNSamples, m_dDefaultRasterViewRatio);
  int retVal = dialogRaster.ShowModal();
  if (retVal != wxID_OK)
    return;

  m_iDefaultRasterNX = dialogRaster.getXSize();
  m_iDefaultRasterNY  = dialogRaster.getYSize();
  m_iDefaultRasterNSamples = dialogRaster.getNSamples();
  m_dDefaultRasterViewRatio = dialogRaster.getViewRatio();
  if (m_iDefaultRasterNSamples < 1)
    m_iDefaultRasterNSamples = 1;
  if (m_dDefaultRasterViewRatio < 0)
    m_dDefaultRasterViewRatio = 0;
  if (m_iDefaultRasterNX <= 0 || m_iDefaultRasterNY <= 0)
    return;

  const Phantom& rPhantom = GetDocument()->getPhantom();
  std::ostringstream os;
  os << "Rasterize Phantom " << rPhantom.name() << ": XSize=" << m_iDefaultRasterNX << ", YSize="
    << m_iDefaultRasterNY << ", ViewRatio=" << m_dDefaultRasterViewRatio << ", nSamples="
    << m_iDefaultRasterNSamples;;

#if HAVE_WXTHREADS
  if (theApp->getUseBackgroundTasks()) {
    RasterizerSupervisorThread* pThread = new RasterizerSupervisorThread
      (this, m_iDefaultRasterNX, m_iDefaultRasterNY,
       m_iDefaultRasterNSamples, m_dDefaultRasterViewRatio, 
       wxConvUTF8.cMB2WX(os.str().c_str()));
    if (pThread->Create() != wxTHREAD_NO_ERROR) {
      *theApp->getLog() << _T("Error creating rasterizer thread\n");
      return;
    }
    pThread->SetPriority (60);
    pThread->Run();
  } else
#endif
  {
    ImageFile* pImageFile = new ImageFile (m_iDefaultRasterNX, m_iDefaultRasterNY);

    wxProgressDialog dlgProgress (_T("Rasterize"),
                                  _T("Rasterization Progress"),
                                  pImageFile->nx() + 1,
                                  getFrameForChild(),
                                  wxPD_CAN_ABORT );
    Timer timer;
    for (unsigned int i = 0; i < pImageFile->nx(); i++) {
      rPhantom.convertToImagefile (*pImageFile, m_dDefaultRasterViewRatio,
                                   m_iDefaultRasterNSamples, Trace::TRACE_NONE,
                                   i, 1, true);
      if ((i + 1) % ITER_PER_UPDATE == 0)
        if (! dlgProgress.Update (i+1)) {
          delete pImageFile;
          return;
        }
    }

    ImageFileDocument* pRasterDoc = theApp->newImageDoc();
    if (! pRasterDoc) {
      sys_error (ERR_SEVERE, "Unable to create image file");
      return;
    }
    pRasterDoc->setImageFile (pImageFile);
    if (theApp->getAskDeleteNewDocs())
      pRasterDoc->Modify (true);
    *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
    pImageFile->labelAdd (os.str().c_str(), timer.timerEnd());

    pRasterDoc->UpdateAllViews(this);
    pRasterDoc->getView()->setInitialClientSize();
    pRasterDoc->Activate();
  }
}


PhantomCanvas*
PhantomFileView::CreateCanvas (wxFrame *parent)
{
  PhantomCanvas* pCanvas =
    new PhantomCanvas (this, parent, wxPoint(-1,-1),
                       wxSize(-1,-1), wxFULL_REPAINT_ON_RESIZE);
  pCanvas->SetBackgroundColour(*wxWHITE);
  pCanvas->ClearBackground();
  return pCanvas;
}

#if CTSIM_MDI
wxDocMDIChildFrame*
#else
wxDocChildFrame*
#endif
PhantomFileView::CreateChildFrame(wxDocument *doc, wxView *view)
{
#if CTSIM_MDI
  wxDocMDIChildFrame *subframe = new wxDocMDIChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Phantom Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#else
  wxDocChildFrame *subframe = new wxDocChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Phantom Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#endif
  theApp->setIconForFrame (subframe);

  m_pFileMenu = new wxMenu;

  m_pFileMenu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  m_pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
  m_pFileMenu->Append(wxID_SAVEAS, _T("Save &As..."));
  m_pFileMenu->Append(wxID_CLOSE, _T("&Close"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(PHMMENU_FILE_PROPERTIES, _T("P&roperties\tCtrl-I"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_PRINT, _T("&Print..."));
  m_pFileMenu->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
  m_pFileMenu->Append(wxID_PREVIEW, _T("Print Pre&view"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(MAINMENU_IMPORT, _T("&Import...\tCtrl-M"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  m_pFileMenu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));
  GetDocumentManager()->FileHistoryAddFilesToMenu(m_pFileMenu);
  GetDocumentManager()->FileHistoryUseMenu(m_pFileMenu);

  wxMenu *process_menu = new wxMenu;
  process_menu->Append(PHMMENU_PROCESS_RASTERIZE, _T("&Rasterize...\tCtrl-R"));
  process_menu->Append(PHMMENU_PROCESS_PROJECTIONS, _T("&Projections...\tCtrl-J"));

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append (MAINMENU_HELP_TIPS, _T("&Tips"));
  help_menu->Append (IDH_QUICKSTART, _T("&Quick Start"));
  help_menu->Append(MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append(m_pFileMenu, _T("&File"));
  menu_bar->Append(process_menu, _T("&Process"));
  menu_bar->Append(help_menu, _T("&Help"));

  subframe->SetMenuBar(menu_bar);
  subframe->Centre(wxBOTH);

  wxAcceleratorEntry accelEntries[3];
  accelEntries[0].Set (wxACCEL_CTRL, static_cast<int>('J'), PHMMENU_PROCESS_PROJECTIONS);
  accelEntries[1].Set (wxACCEL_CTRL, static_cast<int>('R'), PHMMENU_PROCESS_RASTERIZE);
  accelEntries[2].Set (wxACCEL_CTRL, static_cast<int>('I'), PHMMENU_FILE_PROPERTIES);
  wxAcceleratorTable accelTable (3, accelEntries);
  subframe->SetAcceleratorTable (accelTable);

  return subframe;
}


bool
PhantomFileView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
  m_pFrame = CreateChildFrame(doc, this);
  SetFrame(m_pFrame);
  m_pCanvas = CreateCanvas (m_pFrame);
  m_pFrame->SetClientSize (m_pCanvas->GetBestSize());
  m_pCanvas->SetClientSize (m_pCanvas->GetBestSize());
  m_pFrame->SetTitle (doc->GetFilename());

  m_pFrame->Show(true);
  Activate(true);

  return true;
}

void
PhantomFileView::OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
  if (m_pCanvas)
    m_pCanvas->Refresh();
}

bool
PhantomFileView::OnClose (bool deleteWindow)
{
  if (! GetDocument() || ! GetDocument()->Close())
    return false;

  Activate(false);
  if (m_pCanvas) {
    m_pCanvas->setView(NULL);
    m_pCanvas = NULL;
  }
  wxString s(wxTheApp->GetAppName());
  if (m_pFrame)
    m_pFrame->SetTitle(s);

  SetFrame(NULL);

  if (deleteWindow) {
    delete m_pFrame;
    m_pFrame = NULL;
    if (GetDocument() && GetDocument()->getBadFileOpen())
      ::wxYield();  // wxWindows bug workaround
  }

  return true;
}

void
PhantomFileView::OnDraw (wxDC* dc)
{
  int xsize, ysize;
  m_pCanvas->GetClientSize (&xsize, &ysize);
  SGPDriver driver (dc, xsize, ysize);
  SGP sgp (driver);
  const Phantom& rPhantom = GetDocument()->getPhantom();
  sgp.setColor (C_RED);
  rPhantom.show (sgp);
}

// ProjectionCanvas

ProjectionFileCanvas::ProjectionFileCanvas (ProjectionFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style)
: wxScrolledWindow(frame, -1, pos, size, style)
{
  m_pView = v;
}

ProjectionFileCanvas::~ProjectionFileCanvas ()
{
  m_pView = NULL;
}

void
ProjectionFileCanvas::OnDraw(wxDC& dc)
{
  if (m_pView)
    m_pView->OnDraw(& dc);
}

wxSize
ProjectionFileCanvas::GetBestSize () const
{
  const int iMinX = 50;
  const int iMinY = 20;
  wxSize bestSize (iMinX,iMinY);

  if (m_pView) {
    Projections& rProj = m_pView->GetDocument()->getProjections();
    bestSize.Set (rProj.nDet(), rProj.nView());
  }

  if (bestSize.x > 800)
    bestSize.x = 800;
  if (bestSize.y > 800)
    bestSize.y = 800;

  if (bestSize.x < iMinX)
    bestSize.x = iMinX;
  if (bestSize.y < iMinY)
    bestSize.y = iMinY;

  return bestSize;
}


// ProjectionFileView

IMPLEMENT_DYNAMIC_CLASS(ProjectionFileView, wxView)

BEGIN_EVENT_TABLE(ProjectionFileView, wxView)
EVT_MENU(PJMENU_FILE_PROPERTIES, ProjectionFileView::OnProperties)
EVT_MENU(PJMENU_RECONSTRUCT_FBP, ProjectionFileView::OnReconstructFBP)
EVT_MENU(PJMENU_RECONSTRUCT_FBP_REBIN, ProjectionFileView::OnReconstructFBPRebin)
EVT_MENU(PJMENU_RECONSTRUCT_FOURIER, ProjectionFileView::OnReconstructFourier)
EVT_MENU(PJMENU_CONVERT_RECTANGULAR, ProjectionFileView::OnConvertRectangular)
EVT_MENU(PJMENU_CONVERT_POLAR, ProjectionFileView::OnConvertPolar)
EVT_MENU(PJMENU_CONVERT_FFT_POLAR, ProjectionFileView::OnConvertFFTPolar)
EVT_MENU(PJMENU_CONVERT_PARALLEL, ProjectionFileView::OnConvertParallel)
EVT_MENU(PJMENU_PLOT_TTHETA_SAMPLING, ProjectionFileView::OnPlotTThetaSampling)
EVT_MENU(PJMENU_PLOT_HISTOGRAM, ProjectionFileView::OnPlotHistogram)
  // EVT_MENU(PJMENU_ARTIFACT_REDUCTION, ProjectionFileView::OnArtifactReduction)
END_EVENT_TABLE()


ProjectionFileView::ProjectionFileView()
  : wxView(), m_pBitmap(0), m_pFrame(0), m_pCanvas(0), m_pFileMenu(0)
{
#ifdef DEBUG
  m_iDefaultNX = 115;
  m_iDefaultNY = 115;
#else
  m_iDefaultNX = 256;
  m_iDefaultNY = 256;
#endif

  m_iDefaultFilter = SignalFilter::FILTER_ABS_BANDLIMIT;
  m_dDefaultFilterParam = 1.;
#if HAVE_FFTW
  m_iDefaultFilterMethod = ProcessSignal::FILTER_METHOD_RFFTW;
  m_iDefaultFilterGeneration = ProcessSignal::FILTER_GENERATION_INVERSE_FOURIER;
#else
  m_iDefaultFilterMethod = ProcessSignal::FILTER_METHOD_CONVOLUTION;
  m_iDefaultFilterGeneration = ProcessSignal::FILTER_GENERATION_DIRECT;
#endif
  m_iDefaultZeropad = 2;
  m_iDefaultBackprojector = Backprojector::BPROJ_IDIFF;
  m_iDefaultInterpolation = Backprojector::INTERP_LINEAR;
  m_iDefaultInterpParam = 1;
  m_iDefaultTrace = Trace::TRACE_NONE;

  m_iDefaultPolarNX = 256;
  m_iDefaultPolarNY = 256;
  m_iDefaultPolarInterpolation = Projections::POLAR_INTERP_BILINEAR;
  m_iDefaultPolarZeropad = 2;
}

ProjectionFileView::~ProjectionFileView()
{
  GetDocumentManager()->FileHistoryRemoveMenu (m_pFileMenu);
  GetDocumentManager()->ActivateView(this, FALSE);;
}

void
ProjectionFileView::OnProperties (wxCommandEvent& event)
{
  const Projections& rProj = GetDocument()->getProjections();
  std::ostringstream os;
  rProj.printScanInfo(os);
  *theApp->getLog() << _T(">>>>\n") << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("<<<<\n");
  wxMessageDialog dialogMsg (getFrameForChild(), wxConvUTF8.cMB2WX(os.str().c_str()), _T("Projection File Properties"), wxOK | wxICON_INFORMATION);
  dialogMsg.ShowModal();
  GetDocument()->Activate();
}


void
ProjectionFileView::OnConvertRectangular (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();

  int nDet = rProj.nDet();
  int nView = rProj.nView();
  ImageFile* pIF = new ImageFile (nDet, nView);
  ImageFileArray v = pIF->getArray();
  for (int iv = 0; iv < nView; iv++) {
    DetectorValue* detval = rProj.getDetectorArray(iv).detValues();

    for (int id = 0; id < nDet; id++)
      v[id][iv] = detval[id];
  }

  ImageFileDocument* pRectDoc = theApp->newImageDoc ();
  if (! pRectDoc) {
    sys_error (ERR_SEVERE, "Unable to create image file");
    return;
  }
  pRectDoc->setImageFile (pIF);
  pIF->labelAdd (rProj.getLabel().getLabelString().c_str(), rProj.calcTime());
  std::ostringstream os;
  os << "Convert projection file " << getFrame()->GetTitle().c_str() << " to rectangular image";
  *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
  pIF->labelAdd (os.str().c_str());
  if (theApp->getAskDeleteNewDocs())
    pRectDoc->Modify (true);
  pRectDoc->UpdateAllViews();
  pRectDoc->getView()->setInitialClientSize();
  pRectDoc->Activate();
}

void
ProjectionFileView::OnConvertPolar (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  DialogGetConvertPolarParameters dialogPolar (getFrameForChild(), _T("Convert Polar"), m_iDefaultPolarNX, m_iDefaultPolarNY,
    m_iDefaultPolarInterpolation, -1, IDH_DLG_POLAR);
  if (dialogPolar.ShowModal() == wxID_OK) {
    wxProgressDialog dlgProgress (_T("Convert Polar"), _T("Conversion Progress"), 1, getFrameForChild(), wxPD_APP_MODAL);
    wxString strInterpolation (dialogPolar.getInterpolationName(), wxConvUTF8);
    m_iDefaultPolarNX = dialogPolar.getXSize();
    m_iDefaultPolarNY = dialogPolar.getYSize();
    ImageFile* pIF = new ImageFile (m_iDefaultPolarNX, m_iDefaultPolarNY);
    m_iDefaultPolarInterpolation = Projections::convertInterpNameToID (strInterpolation.mb_str(wxConvUTF8));

    if (! rProj.convertPolar (*pIF, m_iDefaultPolarInterpolation)) {
      delete pIF;
      *theApp->getLog() << _T("Error converting to Polar\n");
      return;
    }

    ImageFileDocument* pPolarDoc = theApp->newImageDoc();
    if (! pPolarDoc) {
      sys_error (ERR_SEVERE, "Unable to create image file");
      return;
    }
    pPolarDoc->setImageFile (pIF);
    pIF->labelAdd (rProj.getLabel().getLabelString().c_str(), rProj.calcTime());
    std::ostringstream os;
    os << "Convert projection file " << getFrame()->GetTitle().c_str() << " to polar image: xSize="
      << m_iDefaultPolarNX << ", ySize=" << m_iDefaultPolarNY << ", interpolation="
      << strInterpolation.c_str();
    *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
    pIF->labelAdd (os.str().c_str());
    if (theApp->getAskDeleteNewDocs())
      pPolarDoc->Modify (true);
    pPolarDoc->UpdateAllViews ();
    pPolarDoc->getView()->setInitialClientSize();
    pPolarDoc->Activate();
  }
}

void
ProjectionFileView::OnConvertFFTPolar (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  DialogGetConvertPolarParameters dialogPolar (getFrameForChild(), _T("Convert to FFT Polar"), m_iDefaultPolarNX, m_iDefaultPolarNY,
    m_iDefaultPolarInterpolation, m_iDefaultPolarZeropad, IDH_DLG_FFT_POLAR);
  if (dialogPolar.ShowModal() == wxID_OK) {
    wxProgressDialog dlgProgress (_T("Convert FFT Polar"), _T("Conversion Progress"), 1, getFrameForChild(), wxPD_APP_MODAL);
    wxString strInterpolation (dialogPolar.getInterpolationName(), wxConvUTF8);
    m_iDefaultPolarNX = dialogPolar.getXSize();
    m_iDefaultPolarNY = dialogPolar.getYSize();
    m_iDefaultPolarZeropad = dialogPolar.getZeropad();
    ImageFile* pIF = new ImageFile (m_iDefaultPolarNX, m_iDefaultPolarNY);

    m_iDefaultPolarInterpolation = Projections::convertInterpNameToID (strInterpolation.mb_str(wxConvUTF8));
    if (! rProj.convertFFTPolar (*pIF, m_iDefaultPolarInterpolation, m_iDefaultPolarZeropad)) {
      delete pIF;
      *theApp->getLog() << _T("Error converting to polar\n");
      return;
    }
    ImageFileDocument* pPolarDoc = theApp->newImageDoc();
    if (! pPolarDoc) {
      sys_error (ERR_SEVERE, "Unable to create image file");
      return;
    }
    pPolarDoc->setImageFile (pIF);
    pIF->labelAdd (rProj.getLabel().getLabelString().c_str(), rProj.calcTime());
    std::ostringstream os;
    os << "Convert projection file " << getFrame()->GetTitle().c_str() << " to FFT polar image: xSize="
      << m_iDefaultPolarNX << ", ySize=" << m_iDefaultPolarNY << ", interpolation="
      << strInterpolation.c_str() << ", zeropad=" << m_iDefaultPolarZeropad;
    *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
    pIF->labelAdd (os.str().c_str());
    if (theApp->getAskDeleteNewDocs())
      pPolarDoc->Modify (true);
    pPolarDoc->UpdateAllViews (this);
    pPolarDoc->getView()->setInitialClientSize();
    pPolarDoc->Activate();
  }
}

void
ProjectionFileView::OnPlotTThetaSampling (wxCommandEvent& event)
{
  DialogGetThetaRange dlgTheta (this->getFrame(), ParallelRaysums::THETA_RANGE_UNCONSTRAINED);
  if (dlgTheta.ShowModal() != wxID_OK)
    return;

  int iThetaRange = dlgTheta.getThetaRange();

  Projections& rProj = GetDocument()->getProjections();
  ParallelRaysums parallel (&rProj, iThetaRange);
  PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
  PlotFile& rPlot = pPlotDoc->getPlotFile();
  ParallelRaysums::CoordinateContainer& coordContainer = parallel.getCoordinates();
  double* pdT = new double [parallel.getNumCoordinates()];
  double* pdTheta = new double [parallel.getNumCoordinates()];

  for (int i = 0; i < parallel.getNumCoordinates(); i++) {
    pdT[i] = coordContainer[i]->m_dT;
    pdTheta[i] = coordContainer[i]->m_dTheta;
  }
  rPlot.setCurveSize (2, parallel.getNumCoordinates(), true);
  rPlot.addEzsetCommand ("title T-Theta Sampling");
  rPlot.addEzsetCommand ("xlabel T");
  rPlot.addEzsetCommand ("ylabel Theta");
  rPlot.addEzsetCommand ("curve 1");
  if (rProj.nDet() < 50 && rProj.nView() < 50)
    rPlot.addEzsetCommand ("symbol 1"); // x symbol
  else
    rPlot.addEzsetCommand ("symbol 6"); // point symbol
  rPlot.addEzsetCommand ("noline");
  rPlot.addColumn (0, pdT);
  rPlot.addColumn (1, pdTheta);
  delete pdT;
  delete pdTheta;
  if (theApp->getAskDeleteNewDocs())
    pPlotDoc->Modify (true);
  pPlotDoc->getView()->getFrame()->Show(true);
  pPlotDoc->UpdateAllViews ();
  pPlotDoc->Activate();
}


void
ProjectionFileView::OnPlotHistogram (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  int nDet = rProj.nDet();
  int nView = rProj.nView();

  if (nDet < 1 || nView < 1)
    return;

  PlotFileDocument* pPlotDoc = theApp->newPlotDoc();
  if (! pPlotDoc) {
    sys_error (ERR_SEVERE, "Internal error: unable to create Plot file");
    return;
  }

  DetectorValue* pdDetval = rProj.getDetectorArray(0).detValues();
  double dMin = pdDetval[0], dMax = pdDetval[0];

  for (int iv = 0; iv < nView; iv++) {
    pdDetval = rProj.getDetectorArray(iv).detValues();
    for (int id = 0; id < nDet; id++) {
      double dV = pdDetval[id];
      if (dV < dMin)
        dMin = dV;
      else if (dV > dMax)
        dMax = dV;
    }
  }

  double* pX = new double [NUMBER_HISTOGRAM_BINS];
  double* pY = new double [NUMBER_HISTOGRAM_BINS];
  double dBinWidth = (dMax - dMin) / NUMBER_HISTOGRAM_BINS;

  for (int i = 0; i < NUMBER_HISTOGRAM_BINS; i++) {
    pX[i] = dMin + (i + 0.5) * dBinWidth;
    pY[i] = 0;
  }
  for (int j = 0; j < nView; j++) {
    pdDetval = rProj.getDetectorArray(j).detValues();
    for (int id = 0; id < nDet; id++) {
      int iBin = nearest<int> ((pdDetval[id] - dMin) / dBinWidth);
      if (iBin >= 0 && iBin < NUMBER_HISTOGRAM_BINS)
        pY[iBin] += 1;
    }
  }
  PlotFile& rPlotFile = pPlotDoc->getPlotFile();
  std::ostringstream os;
  os << "Histogram";
  std::string title("title ");
  title += os.str();
  rPlotFile.addEzsetCommand (title.c_str());
  rPlotFile.addEzsetCommand ("xlabel Detector Value");
  rPlotFile.addEzsetCommand ("ylabel Count");
  rPlotFile.addEzsetCommand ("box");
  rPlotFile.addEzsetCommand ("grid");
  rPlotFile.setCurveSize (2, NUMBER_HISTOGRAM_BINS);
  rPlotFile.addColumn (0, pX);
  rPlotFile.addColumn (1, pY);
  rPlotFile.addDescription (rProj.remark());
  os << " plot of " << dynamic_cast<wxFrame*>(GetDocument()->GetFirstView()->GetFrame())->GetTitle().c_str();
  *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
  rPlotFile.addDescription (os.str().c_str());
  delete pX;
  delete pY;
  if (theApp->getAskDeleteNewDocs())
    pPlotDoc->Modify (true);
  pPlotDoc->getView()->getFrame()->Show(true);
  pPlotDoc->UpdateAllViews ();
  pPlotDoc->Activate();
}


void
ProjectionFileView::OnConvertParallel (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  if (rProj.geometry() == Scanner::GEOMETRY_PARALLEL) {
    wxMessageBox (_T("Projections are already parallel"), _T("Error"));
    return;
  }
  wxProgressDialog dlgProgress (_T("Convert to Parallel"), _T("Conversion Progress"), 1, getFrameForChild(), wxPD_APP_MODAL);
  Projections* pProjNew = rProj.interpolateToParallel();
  ProjectionFileDocument* pProjDocNew = theApp->newProjectionDoc();
  pProjDocNew->setProjections (pProjNew);

  if (ProjectionFileView* projView = pProjDocNew->getView()) {
    projView->OnUpdate (projView, NULL);
    if (projView->getCanvas())
      projView->getCanvas()->SetClientSize (pProjNew->nDet(), pProjNew->nView());
    if (wxFrame* pFrame = projView->getFrame()) {
      pFrame->Show(true);
      pFrame->SetFocus();
      pFrame->Raise();
    }
    GetDocumentManager()->ActivateView (projView, true);
  }
  if (theApp->getAskDeleteNewDocs())
    pProjDocNew-> Modify(true);
  pProjDocNew->UpdateAllViews (this);
  pProjDocNew->getView()->setInitialClientSize();
  pProjDocNew->Activate();
}

void
ProjectionFileView::OnReconstructFourier (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  DialogGetConvertPolarParameters dialogPolar (getFrameForChild(), _T("Fourier Reconstruction"), m_iDefaultPolarNX, m_iDefaultPolarNY,
    m_iDefaultPolarInterpolation, m_iDefaultPolarZeropad, IDH_DLG_RECON_FOURIER);
  if (dialogPolar.ShowModal() == wxID_OK) {
    wxProgressDialog dlgProgress (_T("Reconstruction Fourier"), _T("Reconstruction Progress"), 1, getFrameForChild(), wxPD_APP_MODAL);
    wxString strInterpolation (dialogPolar.getInterpolationName(), wxConvUTF8);
    m_iDefaultPolarNX = dialogPolar.getXSize();
    m_iDefaultPolarNY = dialogPolar.getYSize();
    m_iDefaultPolarZeropad = dialogPolar.getZeropad();
    ImageFile* pIF = new ImageFile (m_iDefaultPolarNX, m_iDefaultPolarNY);

    m_iDefaultPolarInterpolation = Projections::convertInterpNameToID (strInterpolation.mb_str(wxConvUTF8));
    if (! rProj.convertFFTPolar (*pIF, m_iDefaultPolarInterpolation, m_iDefaultPolarZeropad)) {
      delete pIF;
      *theApp->getLog() << _T("Error converting to polar\n");
      return;
    }
#ifdef HAVE_FFT
    pIF->ifft(*pIF);
#endif
    pIF->magnitude(*pIF);
    Fourier::shuffleFourierToNaturalOrder (*pIF);

    ImageFileDocument* pPolarDoc = theApp->newImageDoc();
    if (! pPolarDoc) {
      sys_error (ERR_SEVERE, "Unable to create image file");
      return;
    }
    pPolarDoc->setImageFile (pIF);
    pIF->labelAdd (rProj.getLabel().getLabelString().c_str(), rProj.calcTime());
    std::ostringstream os;
    os << "Reconstruct Fourier " << getFrame()->GetTitle().mb_str(wxConvUTF8) << ": xSize="
      << m_iDefaultPolarNX << ", ySize=" << m_iDefaultPolarNY << ", interpolation="
      << strInterpolation.mb_str(wxConvUTF8) << ", zeropad=" << m_iDefaultPolarZeropad;
    *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
    pIF->labelAdd (os.str().c_str());
    if (theApp->getAskDeleteNewDocs())
      pPolarDoc->Modify (true);
    pPolarDoc->UpdateAllViews ();
    pPolarDoc->getView()->setInitialClientSize();
    pPolarDoc->Activate();
  }
}

void
ProjectionFileView::OnReconstructFBPRebin (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  doReconstructFBP (rProj, true);
}

void
ProjectionFileView::OnReconstructFBP (wxCommandEvent& event)
{
  Projections& rProj = GetDocument()->getProjections();
  doReconstructFBP (rProj, false);
}

void
ProjectionFileView::doReconstructFBP (const Projections& rProj, bool bRebinToParallel)
{
  ReconstructionROI defaultROI;
  defaultROI.m_dXMin = -rProj.phmLen() / 2;
  defaultROI.m_dXMax = defaultROI.m_dXMin + rProj.phmLen();
  defaultROI.m_dYMin = -rProj.phmLen() / 2;
  defaultROI.m_dYMax = defaultROI.m_dYMin + rProj.phmLen();

  DialogGetReconstructionParameters dialogReconstruction (getFrameForChild(), m_iDefaultNX, m_iDefaultNY,
    m_iDefaultFilter, m_dDefaultFilterParam, m_iDefaultFilterMethod, m_iDefaultFilterGeneration,
    m_iDefaultZeropad, m_iDefaultInterpolation, m_iDefaultInterpParam, m_iDefaultBackprojector,
    m_iDefaultTrace,  &defaultROI);

  int retVal = dialogReconstruction.ShowModal();
  if (retVal != wxID_OK)
    return;

  m_iDefaultNX = dialogReconstruction.getXSize();
  m_iDefaultNY = dialogReconstruction.getYSize();
  wxString optFilterName (dialogReconstruction.getFilterName(), wxConvUTF8);
  m_iDefaultFilter = SignalFilter::convertFilterNameToID (optFilterName.mb_str(wxConvUTF8));
  m_dDefaultFilterParam = dialogReconstruction.getFilterParam();
  wxString optFilterMethodName (dialogReconstruction.getFilterMethodName(), wxConvUTF8);
  m_iDefaultFilterMethod = ProcessSignal::convertFilterMethodNameToID(optFilterMethodName.mb_str(wxConvUTF8));
  m_iDefaultZeropad = dialogReconstruction.getZeropad();
  wxString optFilterGenerationName (dialogReconstruction.getFilterGenerationName(), wxConvUTF8);
  m_iDefaultFilterGeneration = ProcessSignal::convertFilterGenerationNameToID (optFilterGenerationName.mb_str(wxConvUTF8));
  wxString optInterpName (dialogReconstruction.getInterpName(), wxConvUTF8);
  m_iDefaultInterpolation = Backprojector::convertInterpNameToID (optInterpName.mb_str(wxConvUTF8));
  m_iDefaultInterpParam = dialogReconstruction.getInterpParam();
  wxString optBackprojectName (dialogReconstruction.getBackprojectName(), wxConvUTF8);
  m_iDefaultBackprojector = Backprojector::convertBackprojectNameToID (optBackprojectName.mb_str(wxConvUTF8));
  m_iDefaultTrace = dialogReconstruction.getTrace();
  dialogReconstruction.getROI (&defaultROI);

  if (m_iDefaultNX <= 0 && m_iDefaultNY <= 0)
    return;

  std::ostringstream os;
  os << "Reconstruct " << rProj.getFilename() << ": xSize=" << m_iDefaultNX << ", ySize=" << m_iDefaultNY << ", Filter=" << optFilterName.mb_str(wxConvUTF8) << ", FilterParam=" << m_dDefaultFilterParam << ", FilterMethod=" << optFilterMethodName.mb_str(wxConvUTF8) << ", FilterGeneration=" << optFilterGenerationName.mb_str(wxConvUTF8) << ", Zeropad=" << m_iDefaultZeropad << ", Interpolation=" << optInterpName.mb_str(wxConvUTF8) << ", InterpolationParam=" << m_iDefaultInterpParam << ", Backprojection=" << optBackprojectName.mb_str(wxConvUTF8);
  if (bRebinToParallel)
    os << "; Interpolate to Parallel";

  Timer timerRecon;
  ImageFile* pImageFile = NULL;
  if (m_iDefaultTrace > Trace::TRACE_CONSOLE) {
    pImageFile = new ImageFile (m_iDefaultNX, m_iDefaultNY);
    Reconstructor* pReconstructor = new Reconstructor (rProj, *pImageFile, optFilterName.mb_str(wxConvUTF8),
      m_dDefaultFilterParam, optFilterMethodName.mb_str(wxConvUTF8), m_iDefaultZeropad, optFilterGenerationName.mb_str(wxConvUTF8),
      optInterpName.mb_str(wxConvUTF8), m_iDefaultInterpParam, optBackprojectName.mb_str(wxConvUTF8), m_iDefaultTrace,
      &defaultROI, bRebinToParallel);

    ReconstructDialog* pDlgReconstruct = new ReconstructDialog (*pReconstructor, rProj, *pImageFile, m_iDefaultTrace, getFrameForChild());
    for (int iView = 0; iView < rProj.nView(); iView++) {
      ::wxYield();
      if (pDlgReconstruct->isCancelled() || ! pDlgReconstruct->reconstructView (iView, true)) {
        delete pDlgReconstruct;
        delete pReconstructor;
        return;
      }
      ::wxYield();
      ::wxYield();
      while (pDlgReconstruct->isPaused()) {
        ::wxYield();
        ::wxMilliSleep(50);
      }
    }
    pReconstructor->postProcessing();
    delete pDlgReconstruct;
    delete pReconstructor;
  } else {
#if HAVE_WXTHREADS
    if (theApp->getUseBackgroundTasks()) {
      ReconstructorSupervisorThread* pReconstructor = new ReconstructorSupervisorThread
        (this, m_iDefaultNX, m_iDefaultNY, optFilterName.mb_str(wxConvUTF8), 
         m_dDefaultFilterParam, optFilterMethodName.mb_str(wxConvUTF8),
         m_iDefaultZeropad, optFilterGenerationName.mb_str(wxConvUTF8), 
         optInterpName.mb_str(wxConvUTF8), m_iDefaultInterpParam,
         optBackprojectName.mb_str(wxConvUTF8), 
         wxConvUTF8.cMB2WX(os.str().c_str()), &defaultROI, bRebinToParallel);
      if (pReconstructor->Create() != wxTHREAD_NO_ERROR) {
        sys_error (ERR_SEVERE, "Error creating reconstructor thread");
        delete pReconstructor;
        return;
      }
      pReconstructor->SetPriority (60);
      pReconstructor->Run();
      return;
    } else
#endif
    {
      pImageFile = new ImageFile (m_iDefaultNX, m_iDefaultNY);
      wxProgressDialog dlgProgress (_T("Reconstruction"), _T("Reconstruction Progress"), rProj.nView() + 1, getFrameForChild(), wxPD_CAN_ABORT );
      Reconstructor* pReconstructor = new Reconstructor (rProj, *pImageFile, optFilterName.mb_str(wxConvUTF8),
        m_dDefaultFilterParam, optFilterMethodName.mb_str(wxConvUTF8), m_iDefaultZeropad, optFilterGenerationName.mb_str(wxConvUTF8),
        optInterpName.mb_str(wxConvUTF8), m_iDefaultInterpParam, optBackprojectName.mb_str(wxConvUTF8), m_iDefaultTrace,
        &defaultROI, bRebinToParallel);

      for (int iView = 0; iView < rProj.nView(); iView++) {
        pReconstructor->reconstructView (iView, 1);
        if ((iView + 1) % ITER_PER_UPDATE == 0)
          if (! dlgProgress.Update (iView + 1)) {
            delete pReconstructor;
            return; // don't make new window, thread will do this
          }
      }
      pReconstructor->postProcessing();
      delete pReconstructor;
    }
  }
  ImageFileDocument* pReconDoc = theApp->newImageDoc();
  if (! pReconDoc) {
    sys_error (ERR_SEVERE, "Unable to create image file");
    return;
  }
  *theApp->getLog() << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("\n");
  pImageFile->labelAdd (rProj.getLabel());
  pImageFile->labelAdd (os.str().c_str(), timerRecon.timerEnd());

  pReconDoc->setImageFile (pImageFile);
  if (theApp->getAskDeleteNewDocs())
    pReconDoc->Modify (true);
  pReconDoc->UpdateAllViews();
  pReconDoc->getView()->setInitialClientSize();
  pReconDoc->Activate();
}


void
ProjectionFileView::OnArtifactReduction (wxCommandEvent& event)
{
}


ProjectionFileCanvas*
ProjectionFileView::CreateCanvas (wxFrame *parent)
{
  ProjectionFileCanvas* pCanvas;
  int width, height;
  parent->GetClientSize(&width, &height);

  pCanvas = new ProjectionFileCanvas (this, parent, wxPoint(-1,-1), wxSize(width, height), 0);

  pCanvas->SetScrollbars(20, 20, 50, 50);
  pCanvas->SetBackgroundColour(*wxWHITE);
  pCanvas->ClearBackground();

  return pCanvas;
}

#if CTSIM_MDI
wxDocMDIChildFrame*
#else
wxDocChildFrame*
#endif
ProjectionFileView::CreateChildFrame(wxDocument *doc, wxView *view)
{
#ifdef CTSIM_MDI
  wxDocMDIChildFrame *subframe = new wxDocMDIChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Projection Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#else
  wxDocChildFrame *subframe = new wxDocChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Projection Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#endif
  theApp->setIconForFrame (subframe);

  m_pFileMenu = new wxMenu;

  m_pFileMenu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  m_pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
  m_pFileMenu->Append(wxID_SAVE, _T("&Save\tCtrl-S"));
  m_pFileMenu->Append(wxID_SAVEAS, _T("Save &As..."));
  m_pFileMenu->Append(wxID_CLOSE, _T("&Close\tCtrl-W"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(PJMENU_FILE_PROPERTIES, _T("P&roperties\tCtrl-I"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_PRINT, _T("&Print..."));
  m_pFileMenu->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
  m_pFileMenu->Append(wxID_PREVIEW, _T("Print Pre&view"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(MAINMENU_IMPORT, _T("&Import...\tCtrl-M"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  m_pFileMenu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));
  GetDocumentManager()->FileHistoryAddFilesToMenu(m_pFileMenu);
  GetDocumentManager()->FileHistoryUseMenu(m_pFileMenu);

  m_pConvertMenu = new wxMenu;
  m_pConvertMenu->Append (PJMENU_CONVERT_RECTANGULAR, _T("&Rectangular Image"));
  m_pConvertMenu->Append (PJMENU_CONVERT_POLAR, _T("&Polar Image...\tCtrl-L"));
  m_pConvertMenu->Append (PJMENU_CONVERT_FFT_POLAR, _T("FF&T->Polar Image...\tCtrl-T"));
  m_pConvertMenu->AppendSeparator();
  m_pConvertMenu->Append (PJMENU_CONVERT_PARALLEL, _T("&Interpolate to Parallel"));

  //  wxMenu* filter_menu = new wxMenu;
  //  filter_menu->Append (PJMENU_ARTIFACT_REDUCTION, _T("&Artifact Reduction"));

  wxMenu* analyze_menu = new wxMenu;
  analyze_menu->Append (PJMENU_PLOT_HISTOGRAM, _T("&Plot Histogram"));
  analyze_menu->Append (PJMENU_PLOT_TTHETA_SAMPLING, _T("Plot T-T&heta Sampling...\tCtrl-H"));

  m_pReconstructMenu = new wxMenu;
  m_pReconstructMenu->Append (PJMENU_RECONSTRUCT_FBP, _T("&Filtered Backprojection...\tCtrl-R"), _T("Reconstruct image using filtered backprojection"));
  m_pReconstructMenu->Append (PJMENU_RECONSTRUCT_FBP_REBIN, _T("Filtered &Backprojection (Rebin to Parallel)...\tCtrl-B"), _T("Reconstruct image using filtered backprojection"));
  m_pReconstructMenu->Append (PJMENU_RECONSTRUCT_FOURIER, _T("&Inverse Fourier...\tCtrl-E"), _T("Direct inverse Fourier"));

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append (MAINMENU_HELP_TIPS, _T("&Tips"));
  help_menu->Append (IDH_QUICKSTART, _T("&Quick Start"));
  help_menu->Append(MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append (m_pFileMenu, _T("&File"));
  menu_bar->Append (m_pConvertMenu, _T("&Convert"));
  //  menu_bar->Append (filter_menu, _T("Fi&lter"));
  menu_bar->Append (analyze_menu, _T("&Analyze"));
  menu_bar->Append (m_pReconstructMenu, _T("&Reconstruct"));
  menu_bar->Append (help_menu, _T("&Help"));

  subframe->SetMenuBar(menu_bar);
  subframe->Centre(wxBOTH);

  wxAcceleratorEntry accelEntries[7];
  accelEntries[0].Set (wxACCEL_CTRL, static_cast<int>('L'), PJMENU_CONVERT_POLAR);
  accelEntries[1].Set (wxACCEL_CTRL, static_cast<int>('T'), PJMENU_CONVERT_FFT_POLAR);
  accelEntries[2].Set (wxACCEL_CTRL, static_cast<int>('R'), PJMENU_RECONSTRUCT_FBP);
  accelEntries[3].Set (wxACCEL_CTRL, static_cast<int>('B'), PJMENU_RECONSTRUCT_FBP_REBIN);
  accelEntries[4].Set (wxACCEL_CTRL, static_cast<int>('E'), PJMENU_RECONSTRUCT_FOURIER);
  accelEntries[5].Set (wxACCEL_CTRL, static_cast<int>('I'), PJMENU_FILE_PROPERTIES);
  accelEntries[6].Set (wxACCEL_CTRL, static_cast<int>('H'), PJMENU_PLOT_TTHETA_SAMPLING);
  wxAcceleratorTable accelTable (7, accelEntries);
  subframe->SetAcceleratorTable (accelTable);

  return subframe;
}


bool
ProjectionFileView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
  m_pFrame = CreateChildFrame(doc, this);
  SetFrame(m_pFrame);
  m_pCanvas = CreateCanvas (m_pFrame);
  m_pFrame->SetClientSize (m_pCanvas->GetBestSize());
  m_pCanvas->SetClientSize (m_pCanvas->GetBestSize());
  m_pFrame->SetTitle (doc->GetFilename());

  m_pFrame->Show(true);
  Activate(true);

  return true;
}

void
ProjectionFileView::OnDraw (wxDC* dc)
{
  if (m_pBitmap && m_pBitmap->Ok())
    dc->DrawBitmap (*m_pBitmap, 0, 0, false);
}


void
ProjectionFileView::setInitialClientSize ()
{
  if (m_pFrame && m_pCanvas) {
    wxSize bestSize = m_pCanvas->GetBestSize();

    m_pFrame->SetClientSize (bestSize);
    m_pFrame->Show (true);
    m_pFrame->SetFocus();
  }
}

void
ProjectionFileView::OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
  const Projections& rProj = GetDocument()->getProjections();
  const int nDet = rProj.nDet();
  const int nView = rProj.nView();
  if (rProj.geometry() == Scanner::GEOMETRY_PARALLEL) {
    m_pReconstructMenu->Enable (PJMENU_RECONSTRUCT_FBP_REBIN, false);
    m_pConvertMenu->Enable (PJMENU_CONVERT_PARALLEL, false);
  } else {
    m_pReconstructMenu->Enable (PJMENU_RECONSTRUCT_FBP_REBIN, true);
    m_pConvertMenu->Enable (PJMENU_CONVERT_PARALLEL, true);
  }

  if (nDet != 0 && nView != 0) {
    const DetectorArray& detarray = rProj.getDetectorArray(0);
    const DetectorValue* detval = detarray.detValues();
    double min = detval[0];
    double max = detval[0];
    for (int iy = 0; iy < nView; iy++) {
      const DetectorArray& detarray = rProj.getDetectorArray(iy);
      const DetectorValue* detval = detarray.detValues();
      for (int ix = 0; ix < nDet; ix++) {
        if (min > detval[ix])
          min = detval[ix];
        else if (max < detval[ix])
          max = detval[ix];
      }
    }

    unsigned char* imageData = new unsigned char [nDet * nView * 3];
    if (! imageData) {
      sys_error (ERR_SEVERE, "Unable to allocate memory for image display");
      return;
    }
    double scale = (max - min) / 255;
    for (int iy2 = 0; iy2 < nView; iy2++) {
      const DetectorArray& detarray = rProj.getDetectorArray (iy2);
      const DetectorValue* detval = detarray.detValues();
      for (int ix = 0; ix < nDet; ix++) {
        int intensity = static_cast<int>(((detval[ix] - min) / scale) + 0.5);
        intensity = clamp(intensity, 0, 255);
        int baseAddr = (iy2 * nDet + ix) * 3;
        imageData[baseAddr] = imageData[baseAddr+1] = imageData[baseAddr+2] = intensity;
      }
    }
    wxImage image (nDet, nView, imageData, true);
    if (m_pBitmap) {
      delete m_pBitmap;
      m_pBitmap = NULL;
    }
    m_pBitmap = new wxBitmap (image);
    delete imageData;
  }

    m_pCanvas->SetScrollbars(20, 20, nDet/20, nView/20);
    m_pCanvas->SetBackgroundColour(*wxWHITE);

    if (m_pCanvas)
      m_pCanvas->Refresh();
}

bool
ProjectionFileView::OnClose (bool deleteWindow)
{
  //GetDocumentManager()->ActivateView (this, false);
  if (! GetDocument() || ! GetDocument()->Close())
    return false;

  Activate(false);
  if (m_pCanvas) {
        m_pCanvas->setView(NULL);
    m_pCanvas = NULL;
  }
  wxString s(wxTheApp->GetAppName());
  if (m_pFrame)
    m_pFrame->SetTitle(s);

  SetFrame(NULL);

  if (deleteWindow) {
    delete m_pFrame;
    m_pFrame = NULL;
    if (GetDocument() && GetDocument()->getBadFileOpen())
      ::wxYield();  // wxWindows bug workaround
  }

  return true;
}



// PlotFileCanvas
PlotFileCanvas::PlotFileCanvas (PlotFileView* v, wxFrame *frame,
                                const wxPoint& pos, const wxSize& size,
                                const long style)
  : wxScrolledWindow(frame, -1, pos, size, style), m_pView(v)
{
}

PlotFileCanvas::~PlotFileCanvas ()
{
}

wxSize
PlotFileCanvas::GetBestSize() const
{
  return wxSize (500, 300);
}


void
PlotFileCanvas::OnDraw(wxDC& dc)
{
  if (m_pView)
    m_pView->OnDraw(& dc);
}


// PlotFileView

IMPLEMENT_DYNAMIC_CLASS(PlotFileView, wxView)

BEGIN_EVENT_TABLE(PlotFileView, wxView)
EVT_MENU(PLOTMENU_FILE_PROPERTIES, PlotFileView::OnProperties)
EVT_MENU(PLOTMENU_VIEW_SCALE_MINMAX, PlotFileView::OnScaleMinMax)
EVT_MENU(PLOTMENU_VIEW_SCALE_AUTO, PlotFileView::OnScaleAuto)
EVT_MENU(PLOTMENU_VIEW_SCALE_FULL, PlotFileView::OnScaleFull)
END_EVENT_TABLE()

PlotFileView::PlotFileView()
: wxView(), m_pFrame(0), m_pCanvas(0), m_pEZPlot(0), m_pFileMenu(0),
  m_bMinSpecified(false), m_bMaxSpecified(false)
{
}

PlotFileView::~PlotFileView()
{
  if (m_pEZPlot)
    delete m_pEZPlot;

  GetDocumentManager()->FileHistoryRemoveMenu (m_pFileMenu);
  GetDocumentManager()->ActivateView(this, FALSE);
}

void
PlotFileView::OnProperties (wxCommandEvent& event)
{
  const PlotFile& rPlot = GetDocument()->getPlotFile();
  std::ostringstream os;
  os << "Columns: " << rPlot.getNumColumns() << ", Records: " << rPlot.getNumRecords() << "\n";
  rPlot.printHeadersBrief (os);
  *theApp->getLog() << _T(">>>>\n") << wxConvUTF8.cMB2WX(os.str().c_str()) << _T("<<<<<\n");
  wxMessageDialog dialogMsg (getFrameForChild(), wxConvUTF8.cMB2WX(os.str().c_str()), _T("Plot File Properties"), 
                             wxOK | wxICON_INFORMATION);
  dialogMsg.ShowModal();
  GetDocument()->Activate();
}


void
PlotFileView::OnScaleAuto (wxCommandEvent& event)
{
  const PlotFile& rPlotFile = GetDocument()->getPlotFile();
  double min, max, mean, mode, median, stddev;
  rPlotFile.statistics (1, min, max, mean, mode, median, stddev);
  DialogAutoScaleParameters dialogAutoScale (getFrameForChild(), mean, mode, median, stddev, m_dAutoScaleFactor);
  int iRetVal = dialogAutoScale.ShowModal();
  if (iRetVal == wxID_OK) {
    m_bMinSpecified = true;
    m_bMaxSpecified = true;
    double dMin, dMax;
    if (dialogAutoScale.getMinMax (&dMin, &dMax)) {
      m_dMinPixel = dMin;
      m_dMaxPixel = dMax;
      m_dAutoScaleFactor = dialogAutoScale.getAutoScaleFactor();
      OnUpdate (this, NULL);
    }
  }
  GetDocument()->Activate();
}

void
PlotFileView::OnScaleMinMax (wxCommandEvent& event)
{
  const PlotFile& rPlotFile = GetDocument()->getPlotFile();
  double min, max;

  if (! m_bMinSpecified || ! m_bMaxSpecified) {
    if (! rPlotFile.getMinMax (1, min, max)) {
      *theApp->getLog() << _T("Error: unable to find Min/Max\n");
      return;
    }
  }

  if (m_bMinSpecified)
    min = m_dMinPixel;
  if (m_bMaxSpecified)
    max = m_dMaxPixel;

  DialogGetMinMax dialogMinMax (getFrameForChild(), _T("Set Y-axis Minimum & Maximum"), min, max);
  int retVal = dialogMinMax.ShowModal();
  if (retVal == wxID_OK) {
    m_bMinSpecified = true;
    m_bMaxSpecified = true;
    m_dMinPixel = dialogMinMax.getMinimum();
    m_dMaxPixel = dialogMinMax.getMaximum();
    OnUpdate (this, NULL);
  }
  GetDocument()->Activate();
}

void
PlotFileView::OnScaleFull (wxCommandEvent& event)
{
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  OnUpdate (this, NULL);
  GetDocument()->Activate();
}


PlotFileCanvas*
PlotFileView::CreateCanvas (wxFrame* parent)
{
  PlotFileCanvas* pCanvas;

  pCanvas = new PlotFileCanvas (this, parent, wxPoint(-1,-1), wxSize(-1,-1),
                                wxFULL_REPAINT_ON_RESIZE);
  pCanvas->SetBackgroundColour(*wxWHITE);
  pCanvas->ClearBackground();

  return pCanvas;
}

#if CTSIM_MDI
wxDocMDIChildFrame*
#else
wxDocChildFrame*
#endif
PlotFileView::CreateChildFrame(wxDocument *doc, wxView *view)
{
#ifdef CTSIM_MDI
  wxDocMDIChildFrame *subframe = new wxDocMDIChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Plot Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#else
  wxDocChildFrame *subframe = new wxDocChildFrame(doc, view, theApp->getMainFrame(), -1, _T("Plot Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#endif
  theApp->setIconForFrame (subframe);

  m_pFileMenu = new wxMenu;

  m_pFileMenu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  m_pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
  m_pFileMenu->Append(wxID_SAVE, _T("&Save\tCtrl-S"));
  m_pFileMenu->Append(wxID_SAVEAS, _T("Save &As..."));
  m_pFileMenu->Append(wxID_CLOSE, _T("&Close\tCtrl-W"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(PLOTMENU_FILE_PROPERTIES, _T("P&roperties\tCtrl-I"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_PRINT, _T("&Print..."));
  m_pFileMenu->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
  m_pFileMenu->Append(wxID_PREVIEW, _T("Print Pre&view"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(MAINMENU_IMPORT, _T("&Import...\tCtrl-M"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  m_pFileMenu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));
  GetDocumentManager()->FileHistoryAddFilesToMenu(m_pFileMenu);
  GetDocumentManager()->FileHistoryUseMenu(m_pFileMenu);

  wxMenu *view_menu = new wxMenu;
  view_menu->Append(PLOTMENU_VIEW_SCALE_MINMAX, _T("Display Scale &Set...\tCtrl-E"));
  view_menu->Append(PLOTMENU_VIEW_SCALE_AUTO, _T("Display Scale &Auto...\tCtrl-A"));
  view_menu->Append(PLOTMENU_VIEW_SCALE_FULL, _T("Display &Full Scale\tCtrl-U"));

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append (MAINMENU_HELP_TIPS, _T("&Tips"));
  help_menu->Append (IDH_QUICKSTART, _T("&Quick Start"));
  help_menu->Append(MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append(m_pFileMenu, _T("&File"));
  menu_bar->Append(view_menu, _T("&View"));
  menu_bar->Append(help_menu, _T("&Help"));

  subframe->SetMenuBar(menu_bar);
  subframe->Centre(wxBOTH);

  wxAcceleratorEntry accelEntries[4];
  accelEntries[0].Set (wxACCEL_CTRL, static_cast<int>('E'), PLOTMENU_VIEW_SCALE_MINMAX);
  accelEntries[1].Set (wxACCEL_CTRL, static_cast<int>('A'), PLOTMENU_VIEW_SCALE_AUTO);
  accelEntries[2].Set (wxACCEL_CTRL, static_cast<int>('U'), PLOTMENU_VIEW_SCALE_FULL);
  accelEntries[3].Set (wxACCEL_CTRL, static_cast<int>('I'), PLOTMENU_FILE_PROPERTIES);
  wxAcceleratorTable accelTable (4, accelEntries);
  subframe->SetAcceleratorTable (accelTable);

  return subframe;
}


bool
PlotFileView::OnCreate (wxDocument *doc, long WXUNUSED(flags) )
{
  m_bMinSpecified = false;
  m_bMaxSpecified = false;
  m_dAutoScaleFactor = 1.;

  m_pFrame = CreateChildFrame(doc, this);
  SetFrame(m_pFrame);
  m_pCanvas = CreateCanvas (m_pFrame);
  m_pFrame->SetClientSize (m_pCanvas->GetBestSize());
  m_pCanvas->SetClientSize (m_pCanvas->GetBestSize());
  m_pFrame->SetTitle (_T("test"));
  *theApp->getLog() << _T("Plot doc name: ") << doc->GetDocumentName() << _T("\n");
  *theApp->getLog() << _T("Plot file name: ") << doc->GetFilename() << _T("\n");
  m_pFrame->SetTitle (doc->GetFilename());

  m_pFrame->Show(true);
  Activate(true);

  return true;
}

void
PlotFileView::setInitialClientSize ()
{
  if (m_pFrame && m_pCanvas) {
    wxSize bestSize = m_pCanvas->GetBestSize();

    m_pFrame->SetClientSize (bestSize);
    m_pFrame->Show (true);
    m_pFrame->SetFocus();
  }
}


void
PlotFileView::OnDraw (wxDC* dc)
{
  const PlotFile& rPlotFile = GetDocument()->getPlotFile();
  const int iNColumns = rPlotFile.getNumColumns();
  const int iNRecords = rPlotFile.getNumRecords();

  if (iNColumns > 0 && iNRecords > 0) {
    int xsize, ysize;
    m_pCanvas->GetClientSize (&xsize, &ysize);
    SGPDriver driver (dc, xsize, ysize);
    SGP sgp (driver);
    if (m_pEZPlot)
      m_pEZPlot->plot (&sgp);
  }
}


void
PlotFileView::OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
  const PlotFile& rPlotFile = GetDocument()->getPlotFile();
  const int iNColumns = rPlotFile.getNumColumns();
  const int iNRecords = rPlotFile.getNumRecords();
  const bool bScatterPlot = rPlotFile.getIsScatterPlot();

  if (iNColumns > 0 && iNRecords > 0) {
    if (m_pEZPlot)
      delete m_pEZPlot;
    m_pEZPlot = new EZPlot;

    for (unsigned int iEzset = 0; iEzset < rPlotFile.getNumEzsetCommands(); iEzset++)
      m_pEZPlot->ezset (rPlotFile.getEzsetCommand (iEzset));

    if (m_bMinSpecified) {
      std::ostringstream os;
      os << "ymin " << m_dMinPixel;
      m_pEZPlot->ezset (os.str());
    }

    if (m_bMaxSpecified) {
      std::ostringstream os;
      os << "ymax " << m_dMaxPixel;
      m_pEZPlot->ezset (os.str());
    }

    m_pEZPlot->ezset("box");
    m_pEZPlot->ezset("grid");

    double* pdX = new double [iNRecords];
    double* pdY = new double [iNRecords];
    if (! bScatterPlot) {
      rPlotFile.getColumn (0, pdX);

      for (int iCol = 1; iCol < iNColumns; iCol++) {
        rPlotFile.getColumn (iCol, pdY);
        m_pEZPlot->addCurve (pdX, pdY, iNRecords);
      }
    } else {
      rPlotFile.getColumn (0, pdX);
      rPlotFile.getColumn (1, pdY);
      m_pEZPlot->addCurve (pdX, pdY, iNRecords);
    }
    delete pdX;
    delete pdY;
  }

  if (m_pCanvas) {
    m_pCanvas->Refresh();
  }
}

bool
PlotFileView::OnClose (bool deleteWindow)
{
  if (! GetDocument() || ! GetDocument()->Close())
    return false;

  Activate(false);
  if (m_pCanvas) {
    m_pCanvas->setView (NULL);
    m_pCanvas = NULL;
  }
  wxString s(wxTheApp->GetAppName());
  if (m_pFrame)
    m_pFrame->SetTitle(s);

  SetFrame(NULL);
  if (deleteWindow) {
    delete m_pFrame;
    m_pFrame = NULL;
    if (GetDocument() && GetDocument()->getBadFileOpen())
      ::wxYield();  // wxWindows bug workaround
  }

  return true;
}


////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC_CLASS(TextFileView, wxView)

TextFileView::~TextFileView()
{
  GetDocumentManager()->FileHistoryRemoveMenu (m_pFileMenu);
  GetDocumentManager()->ActivateView(this, FALSE);;
}

bool TextFileView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
  m_pFrame = CreateChildFrame(doc, this);
  SetFrame (m_pFrame);

  int width, height;
  m_pFrame->GetClientSize(&width, &height);
  m_pFrame->SetTitle(doc->GetFilename());
  m_pCanvas = new TextFileCanvas (this, m_pFrame, wxPoint(-1,-1), wxSize(width, height), wxTE_MULTILINE | wxTE_READONLY);

  m_pFrame->Show (true);
  Activate (true);

  return true;
}

// Handled by wxTextWindow
void TextFileView::OnDraw(wxDC *WXUNUSED(dc) )
{
}

void TextFileView::OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
}

bool
TextFileView::OnClose (bool deleteWindow)
{
  if (! theApp->getMainFrame()->getShuttingDown())
    return false;

  Activate(false);
  //GetDocumentManager()->ActivateView (this, false);
  if (! GetDocument() || ! GetDocument()->Close())
    return false;

  SetFrame(NULL);
  if (deleteWindow) {
    delete m_pFrame;
    m_pFrame = NULL;
    if (GetDocument() && GetDocument()->getBadFileOpen())
      ::wxYield();  // wxWindows bug workaround
  }

  return TRUE;
}

#if CTSIM_MDI
wxDocMDIChildFrame*
#else
wxDocChildFrame*
#endif
TextFileView::CreateChildFrame (wxDocument *doc, wxView *view)
{
#if CTSIM_MDI
  wxDocMDIChildFrame* subframe = new wxDocMDIChildFrame (doc, view, theApp->getMainFrame(), -1, _T("TextFile Frame"), wxPoint(-1, -1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE, _T("Log"));
#else
  wxDocChildFrame* subframe = new wxDocChildFrame (doc, view, theApp->getMainFrame(), -1, _T("TextFile Frame"), wxPoint(-1, -1), wxSize(300, 150), wxDEFAULT_FRAME_STYLE, _T("Log"));
#endif
  theApp->setIconForFrame (subframe);

  m_pFileMenu = new wxMenu;

  m_pFileMenu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  m_pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
  m_pFileMenu->Append(wxID_SAVE, _T("&Save\tCtrl-S"));
  m_pFileMenu->Append(wxID_SAVEAS, _T("Save &As..."));
  //  m_pFileMenu->Append(wxID_CLOSE, _T("&Close\tCtrl-W"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_PRINT, _T("&Print..."));
  m_pFileMenu->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
  m_pFileMenu->Append(wxID_PREVIEW, _T("Print Pre&view"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(MAINMENU_IMPORT, _T("&Import...\tCtrl-M"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  m_pFileMenu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));
  GetDocumentManager()->FileHistoryAddFilesToMenu(m_pFileMenu);
  GetDocumentManager()->FileHistoryUseMenu(m_pFileMenu);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append (MAINMENU_HELP_TIPS, _T("&Tips"));
  help_menu->Append (IDH_QUICKSTART, _T("&Quick Start"));
  help_menu->Append(MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append(m_pFileMenu, _T("&File"));
  menu_bar->Append(help_menu, _T("&Help"));

  subframe->SetMenuBar(menu_bar);
  subframe->Centre(wxBOTH);

  return subframe;
}


// Define a constructor for my text subwindow
TextFileCanvas::TextFileCanvas (TextFileView* v, wxFrame* frame, const wxPoint& pos, const wxSize& size, long style)
  : wxTextCtrl (frame, -1, _T(""), pos, size, style), m_pView(v)
{
}

TextFileCanvas::~TextFileCanvas ()
{
  m_pView = NULL;
}

wxSize
TextFileCanvas::GetBestSize() const
{
  int xSize, ySize;
  theApp->getMainFrame()->GetClientSize (&xSize, &ySize);
  xSize = maxValue<int> (xSize, ySize);
#ifdef CTSIM_MDI
  ySize = xSize = (xSize / 4);
#else
  ySize = xSize;
#endif
  return wxSize (xSize, ySize);
}
