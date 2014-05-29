/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          view.h
**   Purpose:       Header file for View & Canvas routines of CTSim program
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

#ifndef __VIEWSH__
#define __VIEWSH__

#include "wx/wx.h"
#include "docs.h"
#include "imagefile.h"
#include "threadrecon.h"

#if wxUSE_GLCANVAS
#include "graph3dview.h"
#endif

class ImageFileCanvas;
class ImageFileView : public wxView
{
private:
  DECLARE_DYNAMIC_CLASS(ImageFileView)

  wxMemoryDC m_memoryDC;
  wxBitmap* m_pBitmap;
  wxMenu* m_pMenuAnalyze;

  ImageFileCanvas *CreateCanvas (wxFrame* parent);
#if CTSIM_MDI
  wxDocMDIChildFrame* m_pFrame;
  wxDocMDIChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#else
  wxDocChildFrame* m_pFrame;
  wxDocChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#endif

  ImageFileCanvas *m_pCanvas;
  wxMenu* m_pFileMenu;
  wxMenu* m_pFilterMenu;
  bool m_bMinSpecified;
  bool m_bMaxSpecified;
  double m_dMinPixel;
  double m_dMaxPixel;
  double m_dAutoScaleFactor;

  int m_iDefaultExportFormatID;

  wxFrame* getFrameForChild()
#if CTSIM_MDI
  { return theApp->getMainFrame(); }
#else
  { return m_pFrame; }
#endif

public:
  ImageFileView();
  virtual ~ImageFileView();
  void canvasClosed()
  { m_pCanvas = NULL; m_pFrame = NULL; }

  wxMenu* getFileMenu()
  { return m_pFileMenu; }

  bool OnCreate(wxDocument *doc, long flags);
  void OnDraw(wxDC* dc);
  void OnUpdate(wxView *sender, wxObject *hint = NULL);
  bool OnClose (bool deleteWindow = true);

  void OnEditCopy (wxCommandEvent& event);
  void OnEditCut (wxCommandEvent& event);
  void OnEditPaste (wxCommandEvent& event);

  void OnRevert (wxCommandEvent& event);
  void OnExport (wxCommandEvent& event);
  void OnProperties (wxCommandEvent& event);

  void OnCompare (wxCommandEvent& event);
  void OnScaleSize (wxCommandEvent& event);
  void OnInvertValues (wxCommandEvent& event);
  void OnSquare (wxCommandEvent& event);
  void OnSquareRoot (wxCommandEvent& event);
  void OnLog (wxCommandEvent& event);
  void OnExp (wxCommandEvent& event);
  void OnAdd (wxCommandEvent& event);
  void OnSubtract (wxCommandEvent& event);
  void OnMultiply (wxCommandEvent& event);
  void OnDivide (wxCommandEvent& event);
  void OnFourier (wxCommandEvent& event);
  void OnInverseFourier (wxCommandEvent& event);
  void OnShuffleNaturalToFourierOrder (wxCommandEvent& event);
  void OnShuffleFourierToNaturalOrder (wxCommandEvent& event);
#if wxUSE_GLCANVAS
  void OnConvert3d (wxCommandEvent& event);
#endif

#ifdef HAVE_FFT
  void OnFFT (wxCommandEvent& event);
  void OnIFFT (wxCommandEvent& event);
  void OnFFTRows (wxCommandEvent& event);
  void OnIFFTRows (wxCommandEvent& event);
  void OnFFTCols (wxCommandEvent& event);
  void OnIFFTCols (wxCommandEvent& event);
#endif

  void OnMagnitude (wxCommandEvent& event);
  void OnPhase (wxCommandEvent& event);
  void OnReal (wxCommandEvent& event);
  void OnImaginary (wxCommandEvent& event);

  void OnScaleAuto (wxCommandEvent& event);
  void OnScaleMinMax (wxCommandEvent& event);
  void OnScaleFull (wxCommandEvent& event);
  void OnPlotRow (wxCommandEvent& event);
  void OnPlotCol (wxCommandEvent& event);
#if HAVE_FFT
  void OnPlotFFTRow (wxCommandEvent& event);
  void OnPlotFFTCol (wxCommandEvent& event);
#endif
  void OnPlotHistogram (wxCommandEvent& event);
  void OnCompareRow (wxCommandEvent& event);
  void OnCompareCol (wxCommandEvent& event);

#if CTSIM_MDI
  wxDocMDIChildFrame* getFrame() { return m_pFrame; }
#else
  wxDocChildFrame* getFrame() { return m_pFrame; }
#endif
  void setInitialClientSize();

  wxMenu* getMenuAnalyze() { return m_pMenuAnalyze; }

  ImageFileDocument* GetDocument()
  { return dynamic_cast<ImageFileDocument*>(wxView::GetDocument()); }

  DECLARE_EVENT_TABLE()
};

class ImageFileCanvas: public wxScrolledWindow
{
private:
  ImageFileView* m_pView;

  int m_xCursor;
  int m_yCursor;

public:
  ImageFileCanvas (ImageFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style);
  virtual ~ImageFileCanvas();

  virtual void OnDraw(wxDC& dc);
  void OnChar(wxKeyEvent& event);
  void OnMouseEvent(wxMouseEvent& event);
  void DrawRubberBandCursor (wxDC& dc, int x, int y);
  bool GetCurrentCursor (int& x, int& y);

  virtual wxSize GetBestSize() const;
  void setView(ImageFileView* pView)
  { m_pView = pView; }


  DECLARE_EVENT_TABLE()
};


class ProjectionFileCanvas;
class ProjectionFileView : public wxView
{
private:
  DECLARE_DYNAMIC_CLASS(ProjectionFileView)

  wxMemoryDC m_memoryDC;
  wxBitmap* m_pBitmap;

  ProjectionFileCanvas *CreateCanvas (wxFrame* parent);
#if CTSIM_MDI
  wxDocMDIChildFrame* m_pFrame;
  wxDocMDIChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#else
  wxDocChildFrame* m_pFrame;
  wxDocChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#endif

  ProjectionFileCanvas *m_pCanvas;
  wxMenu* m_pFileMenu;
  wxMenu* m_pReconstructMenu;
  wxMenu* m_pConvertMenu;

  int m_iDefaultNX;
  int m_iDefaultNY;
  int m_iDefaultFilter;
  int m_iDefaultFilterMethod;
  double m_dDefaultFilterParam;
  int m_iDefaultFilterGeneration;
  int m_iDefaultZeropad;
  int m_iDefaultInterpolation;
  int m_iDefaultInterpParam;
  int m_iDefaultBackprojector;
  int m_iDefaultTrace;

  int m_iDefaultPolarNX;
  int m_iDefaultPolarNY;
  int m_iDefaultPolarInterpolation;
  int m_iDefaultPolarZeropad;

  wxWindow* getFrameForChild()
#if CTSIM_MDI
  { return theApp->getMainFrame(); }
#else
  { return m_pFrame; }
#endif

public:
  ProjectionFileView();
  virtual ~ProjectionFileView();
  void canvasClosed()
  { m_pCanvas = NULL; m_pFrame = NULL; }

  bool OnCreate(wxDocument *doc, long flags);
  void OnDraw(wxDC* dc);
  void OnUpdate(wxView *sender, wxObject *hint = NULL);
  bool OnClose (bool deleteWindow = true);
  void OnProperties (wxCommandEvent& event);
  void OnReconstructFBP (wxCommandEvent& event);
  void OnReconstructFBPRebin (wxCommandEvent& event);
  void OnReconstructFourier (wxCommandEvent& event);
  void OnConvertRectangular (wxCommandEvent& event);
  void OnConvertPolar (wxCommandEvent& event);
  void OnConvertFFTPolar (wxCommandEvent& event);
  void OnPlotTThetaSampling (wxCommandEvent& event);
  void OnPlotHistogram (wxCommandEvent& event);
  void OnConvertParallel (wxCommandEvent& event);
  void OnArtifactReduction (wxCommandEvent& event);

  void doReconstructFBP (const Projections& rProj, bool bRebinToParallel);

#if CTSIM_MDI
  wxDocMDIChildFrame* getFrame() { return m_pFrame; }
#else
  wxDocChildFrame* getFrame() { return m_pFrame; }
#endif
  ProjectionFileCanvas* getCanvas() { return m_pCanvas; }
  void setInitialClientSize();

  wxMenu* getFileMenu()  { return m_pFileMenu; }
  wxMenu* getReconstructMenu()  { return m_pReconstructMenu; }

  ProjectionFileDocument* GetDocument()
  { return dynamic_cast<ProjectionFileDocument*>(wxView::GetDocument()); }
  DECLARE_EVENT_TABLE()
};

class ProjectionFileCanvas: public wxScrolledWindow
{
private:
  ProjectionFileView* m_pView;

public:
  ProjectionFileCanvas (ProjectionFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style);
  virtual ~ProjectionFileCanvas() ;

  virtual wxSize GetBestSize() const;
  virtual void OnDraw(wxDC& dc);
  void setView(ProjectionFileView* pView)
  { m_pView = pView; }
};


class PhantomCanvas;
class PhantomFileView : public wxView
{
private:
  DECLARE_DYNAMIC_CLASS(PhantomFileView)
  DECLARE_EVENT_TABLE()

  PhantomCanvas *CreateCanvas (wxFrame* parent);
#if CTSIM_MDI
  wxDocMDIChildFrame* m_pFrame;
  wxDocMDIChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#else
  wxDocChildFrame* m_pFrame;
  wxDocChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#endif

  PhantomCanvas *m_pCanvas;
  wxMenu* m_pFileMenu;

  int m_iDefaultNDet;
  int m_iDefaultNView;
  int m_iDefaultOffsetView;
  int m_iDefaultNSample;
  int m_iDefaultGeometry;
  int m_iDefaultTrace;
  double m_dDefaultRotation;
  double m_dDefaultFocalLength;
  double m_dDefaultCenterDetectorLength;
  double m_dDefaultViewRatio;
  double m_dDefaultScanRatio;

  int m_iDefaultRasterNX;
  int m_iDefaultRasterNY;
  int m_iDefaultRasterNSamples;
  double m_dDefaultRasterViewRatio;

  wxWindow* getFrameForChild()
#if CTSIM_MDI
  { return theApp->getMainFrame(); }
#else
  { return m_pFrame; }
#endif

public:
  PhantomFileView();
  virtual ~PhantomFileView();
  void canvasClosed()
  { m_pCanvas = NULL; m_pFrame = NULL; }

  bool OnCreate(wxDocument *doc, long flags);
  void OnUpdate(wxView *sender, wxObject *hint = NULL);
  bool OnClose (bool deleteWindow = true);
  void OnDraw(wxDC* dc);
  void OnProperties (wxCommandEvent& event);
  void OnRasterize (wxCommandEvent& event);
  void OnProjections (wxCommandEvent& event);

  PhantomFileDocument* GetDocument()
  { return dynamic_cast<PhantomFileDocument*>(wxView::GetDocument()); }

  wxMenu* getFileMenu() { return m_pFileMenu; }
#if CTSIM_MDI
  wxDocMDIChildFrame* getFrame() { return m_pFrame; }
#else
  wxDocChildFrame* getFrame() { return m_pFrame; }
#endif
};

class PhantomCanvas: public wxScrolledWindow
{
private:
  PhantomFileView* m_pView;

public:
  PhantomCanvas (PhantomFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style);
  virtual ~PhantomCanvas();

  virtual wxSize GetBestSize() const;
  void setView(PhantomFileView* pView)
  { m_pView = pView; }
  virtual void OnDraw(wxDC& dc);
};

class PlotFileCanvas;
class PlotFileView : public wxView
{
  DECLARE_DYNAMIC_CLASS(PlotFileView)

private:
#if CTSIM_MDI
  wxDocMDIChildFrame* m_pFrame;
  wxDocMDIChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#else
  wxDocChildFrame* m_pFrame;
  wxDocChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#endif

  PlotFileCanvas *m_pCanvas;
  EZPlot* m_pEZPlot;
  wxMenu* m_pFileMenu;

  bool m_bMinSpecified;
  bool m_bMaxSpecified;
  double m_dMinPixel;
  double m_dMaxPixel;
  double m_dAutoScaleFactor;

  PlotFileCanvas *CreateCanvas (wxFrame* parent);
  wxWindow* getFrameForChild()
#if CTSIM_MDI
  { return theApp->getMainFrame(); }
#else
  { return m_pFrame; }
#endif

public:
  PlotFileView();
  virtual ~PlotFileView();
  void canvasClosed()
  { m_pCanvas = NULL; m_pFrame = NULL; }

  bool OnCreate(wxDocument *doc, long flags);
  void OnDraw(wxDC* dc);
  void OnUpdate(wxView *sender, wxObject *hint = NULL);
  bool OnClose (bool deleteWindow = true);

  void OnProperties (wxCommandEvent& event);
  void OnScaleMinMax (wxCommandEvent& event);
  void OnScaleAuto (wxCommandEvent& event);
  void OnScaleFull (wxCommandEvent& event);

#if CTSIM_MDI
  wxDocMDIChildFrame* getFrame() { return m_pFrame; }
#else
  wxDocChildFrame* getFrame() { return m_pFrame; }
#endif

  void setInitialClientSize();
  wxMenu* getFileMenu() { return m_pFileMenu; }
  PlotFileDocument* GetDocument()
  { return dynamic_cast<PlotFileDocument*>(wxView::GetDocument()); }

  DECLARE_EVENT_TABLE()
};

class PlotFileCanvas: public wxScrolledWindow
{
private:
  PlotFileView* m_pView;

public:
  PlotFileCanvas (PlotFileView* v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style);
  virtual ~PlotFileCanvas();

  virtual void OnDraw(wxDC& dc);
  virtual wxSize GetBestSize() const;

  void setView (PlotFileView* pView)
  { m_pView = pView; }
};


class TextFileCanvas;
class TextFileView: public wxView
{
private:
  DECLARE_DYNAMIC_CLASS(TextFileView)

#if CTSIM_MDI
  wxDocMDIChildFrame* m_pFrame;
  wxDocMDIChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#else
  wxDocChildFrame* m_pFrame;
  wxDocChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#endif
    wxMenu* m_pFileMenu;
    TextFileCanvas *m_pCanvas;

public:
    TextFileView()
      : wxView() , m_pFrame(0), m_pCanvas(0)
    {}
    ~TextFileView();
    void canvasClosed()
    { m_pFrame = NULL; }

    bool OnCreate (wxDocument *doc, long flags);
    void OnDraw (wxDC *dc);
    void OnUpdate (wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose (bool deleteWindow = TRUE);

    TextFileDocument* GetDocument()
    { return dynamic_cast<TextFileDocument*>(wxView::GetDocument()); }

    TextFileCanvas* getTextCtrl() { return m_pCanvas; }
    wxMenu* getFileMenu() { return m_pFileMenu; }
#if CTSIM_MDI
  wxDocMDIChildFrame* getFrame() { return m_pFrame; }
#else
  wxDocChildFrame* getFrame() { return m_pFrame; }
#endif
};

class TextFileCanvas: public wxTextCtrl
{
    TextFileView *m_pView;

public:
    TextFileCanvas (TextFileView *v, wxFrame *frame, const wxPoint& pos, const wxSize& size, long style);
    ~TextFileCanvas ();
    virtual wxSize GetBestSize() const;
};


#endif

