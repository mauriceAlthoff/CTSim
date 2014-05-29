/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          ctsim.h
**   Purpose:       Header file for CTSim
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

#ifndef __CTSIMH__
#define __CTSIMH__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

#ifdef MSVC
#define HAVE_WXTHREADS 1
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/config.h"
#ifdef __WXMSW__
#include "wx/msw/helpchm.h"
#endif

#ifdef MSVC
#define CTSIM_MDI 1
#endif

#if defined(CTSIM_MDI) && !wxUSE_MDI_ARCHITECTURE
#error You must set wxUSE_MDI_ARCHITECTURE to 1 in setup.h!
#endif
#ifdef CTSIM_MDI
#include "wx/docmdi.h"
#endif

class wxMenu;
class wxDocument;
class ImageFileDocument;
class ProjectionFileDocument;
class PhantomFileDocument;
class PlotFileDocument;
class TextFileDocument;
class BackgroundManager;

#if wxUSE_GLCANVAS
class Graph3dFileDocument;
#endif

#include <vector>
#include "wx/docview.h"
#include "wx/textctrl.h"
#include "wx/menu.h"
#include "wx/help.h"
#include "wx/html/helpctrl.h"
#include "dlgezplot.h"
#include "ctsim-map.h"


#if defined(__WXMSW__) || defined (MSVC)
// #define CTSIM_WINHELP   1
#endif

// Define a new frame for main window
#if CTSIM_MDI
class MainFrame: public wxDocMDIParentFrame
#else
class MainFrame: public wxDocParentFrame
#endif
{
private:
  DECLARE_CLASS(MainFrame)
  DECLARE_EVENT_TABLE()

#ifndef CTSIM_MDI
  wxMenu* m_pWindowMenu;
#endif

  enum { MAX_WINDOW_MENUITEMS = 20 };
  wxMenuItem* m_apWindowMenuItems[MAX_WINDOW_MENUITEMS];
  wxDocument* m_apWindowMenuData[MAX_WINDOW_MENUITEMS];

  int m_iDefaultImportFormat;
  int m_iDefaultPhantomID;
  int m_iDefaultFilterID;
  int m_iDefaultFilterDomainID;
  unsigned int m_iDefaultFilterXSize;
  unsigned int m_iDefaultFilterYSize;
  double m_dDefaultFilterParam;
  double m_dDefaultFilterBandwidth;
  double m_dDefaultFilterInputScale;
  double m_dDefaultFilterOutputScale;

  bool m_bShuttingDown;

#if CTSIM_WINHELP
  wxCHMHelpController      m_winHelp;
#endif
  wxHtmlHelpController     m_htmlHelp;

public:
  MainFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type);
  virtual ~MainFrame();

  void OnSize (wxSizeEvent& event);

#if CTSIM_WINHELP
  wxCHMHelpController&   getWinHelpController()
  {return m_winHelp; }
#endif
  wxHtmlHelpController&  getHtmlHelpController()
  { return m_htmlHelp; }

  void showHelp (int commandID);

  void OnAbout (wxCommandEvent& event);
  void OnHelpContents (wxCommandEvent& event);
  void OnHelpTips (wxCommandEvent& event);
  void OnCreatePhantom (wxCommandEvent& event);
  void OnPreferences (wxCommandEvent& event);
  void OnLogEvent (wxCommandEvent& event);  // used by thread children
  void OnNewImageFile (wxCommandEvent& event);
  void OnNewProjectionFile (wxCommandEvent& event);

  void OnHelpButton (wxCommandEvent& event);
  void OnImport (wxCommandEvent& event);

#if defined(CTSIM_WINHELP) && (defined(DEBUG) || defined(_DEBUG))
  void OnHelpSecondary (wxCommandEvent& event);
#endif

  void OnCreateFilter (wxCommandEvent& event);
  void OnExit (wxCommandEvent& event);

  void OnUpdateUI (wxUpdateUIEvent& event);

  void OnWindowMenu0 (wxCommandEvent& event);
  void OnWindowMenu1 (wxCommandEvent& event);
  void OnWindowMenu2 (wxCommandEvent& event);
  void OnWindowMenu3 (wxCommandEvent& event);
  void OnWindowMenu4 (wxCommandEvent& event);
  void OnWindowMenu5 (wxCommandEvent& event);
  void OnWindowMenu6 (wxCommandEvent& event);
  void OnWindowMenu7 (wxCommandEvent& event);
  void OnWindowMenu8 (wxCommandEvent& event);
  void OnWindowMenu9 (wxCommandEvent& event);
  void OnWindowMenu10 (wxCommandEvent& event);
  void OnWindowMenu11 (wxCommandEvent& event);
  void OnWindowMenu12 (wxCommandEvent& event);
  void OnWindowMenu13 (wxCommandEvent& event);
  void OnWindowMenu14 (wxCommandEvent& event);
  void OnWindowMenu15 (wxCommandEvent& event);
  void OnWindowMenu16 (wxCommandEvent& event);
  void OnWindowMenu17 (wxCommandEvent& event);
  void OnWindowMenu18 (wxCommandEvent& event);
  void OnWindowMenu19 (wxCommandEvent& event);

  void DoWindowMenu (int iMenuPosition, wxCommandEvent& event);

  bool getShuttingDown() const { return m_bShuttingDown; }
};


class wxDocManager;
class CTSimApp: public wxApp
{
private:
  enum { O_HELP, O_PRINT, O_VERSION };
  static struct option ctsimOptions[];

  bool m_bAdvancedOptions;
  bool m_bSetModifyNewDocs;
  bool m_bVerboseLogging;
  bool m_bShowStartupTips;
  long m_iCurrentTip;
  bool m_bUseBackgroundTasks;

  wxDocManager* m_docManager;
  MainFrame* m_pFrame;
  wxConfig* m_pConfig;
  wxTextCtrl* m_pLog;
  TextFileDocument* m_pLogDoc;
  wxDocTemplate* m_pDocTemplImage;
  wxDocTemplate* m_pDocTemplProjection;
  wxDocTemplate* m_pDocTemplPhantom;
  wxDocTemplate* m_pDocTemplPlot;
  wxDocTemplate* m_pDocTemplText;
#if wxUSE_GLCANVAS
  wxDocTemplate* m_pDocTemplGraph3d;
#endif

  void usage (const char* program);
  void openConfig();
  void closeConfig();
  BackgroundManager*  m_pBackgroundMgr;
  bool m_bPrintCmdLineImages;
  bool m_bCmdLineVerboseFlag;

  wxDocument* newDocumentHelper (wxDocTemplate* tmpl);

public:
  CTSimApp();
  void OnInitCmdLine(wxCmdLineParser& parser);
  bool OnCmdLineParsed(wxCmdLineParser& parser);
  bool OnInit();
  int OnExit();
  MainFrame* getMainFrame() const
  { return m_pFrame; }

  wxTextCtrl* getLog()
  { return m_pLog; }

  wxDocManager* getDocManager()
  { return m_docManager; }

  int getNumberCPU() const { return wxThread::GetCPUCount(); }

  EZPlotDialog* makeEZPlotDialog()
  { return new EZPlotDialog (m_pFrame); }

  void getCompatibleImages (const ImageFileDocument* pIFDoc, std::vector<ImageFileDocument*>& vecIF);
  bool getAdvancedOptions() const { return m_bAdvancedOptions; }
  void setAdvancedOptions (bool bAdv) { m_bAdvancedOptions = bAdv; }
  bool getVerboseLogging() const { return m_bVerboseLogging || m_bCmdLineVerboseFlag; }
  void setVerboseLogging (bool bVerbose) { m_bVerboseLogging = bVerbose; }
  bool getStartupTips() const { return m_bShowStartupTips; }
  void setStartupTips(bool bTips) { m_bShowStartupTips = bTips; }
  bool getUseBackgroundTasks() const { return m_bUseBackgroundTasks; }
  void setUseBackgroundTasks(bool bBkgd) { m_bUseBackgroundTasks = bBkgd; }

  BackgroundManager* getBackgroundManager() {return m_pBackgroundMgr;}

  void ShowTips();

  void setIconForFrame (wxFrame* pFrame);
  wxConfig* getConfig()
  { return m_pConfig; }
  bool getAskDeleteNewDocs() const { return m_bSetModifyNewDocs; }
  void setAskDeleteNewDocs(bool bAsk) { m_bSetModifyNewDocs = bAsk; }

  wxDocTemplate* getDocTemplImage() { return m_pDocTemplImage; }
  wxDocTemplate* getDocTemplProjection() { return m_pDocTemplProjection; }
  wxDocTemplate* getDocTemplPhantom() { return m_pDocTemplPhantom; }
  wxDocTemplate* getDocTemplPlot() { return m_pDocTemplPlot; }
  wxDocTemplate* getDocTemplText() { return m_pDocTemplText; }
#if wxUSE_GLCANVAS
  wxDocTemplate* getDocTemplGraph3d() { return m_pDocTemplGraph3d; }
#endif
  TextFileDocument* getLogDoc() { return m_pLogDoc; }

  ProjectionFileDocument* newProjectionDoc();
  ImageFileDocument* newImageDoc();
  PhantomFileDocument* newPhantomDoc();
  PlotFileDocument* newPlotDoc();
  TextFileDocument* newTextDoc();
#if wxUSE_GLCANVAS
  Graph3dFileDocument* newGraph3dDoc();
#endif
};

DECLARE_APP(CTSimApp)
extern class CTSimApp* theApp;

enum {
    MAINMENU_WINDOW_BASE = 500,
    MAINMENU_HELP_ABOUT = 600,
    MAINMENU_HELP_CONTENTS,
    MAINMENU_HELP_TIPS,
#if defined(CTSIM_WINHELP) && (defined(DEBUG) || defined(_DEBUG))
    MAINMENU_HELP_SECONDARY,
#endif
    MAINMENU_FILE_CREATE_PHANTOM,
    MAINMENU_FILE_CREATE_FILTER,
    MAINMENU_FILE_EXIT,
    MAINMENU_FILE_PREFERENCES,
    MAINMENU_LOG_EVENT,
    MAINMENU_IMPORT,

    PJMENU_FILE_PROPERTIES,
    PJMENU_RECONSTRUCT_FBP,
    PJMENU_RECONSTRUCT_FBP_REBIN,
    PJMENU_RECONSTRUCT_FOURIER,
    PJMENU_CONVERT_RECTANGULAR,
    PJMENU_CONVERT_POLAR,
    PJMENU_CONVERT_FFT_POLAR,
    PJMENU_CONVERT_PARALLEL,
    PJMENU_PLOT_TTHETA_SAMPLING,
    PJMENU_PLOT_HISTOGRAM,
    PJMENU_ARTIFACT_REDUCTION,

    IFMENU_FILE_EXPORT,
    IFMENU_FILE_PROPERTIES,

    IFMENU_EDIT_COPY,
    IFMENU_EDIT_CUT,
    IFMENU_EDIT_PASTE,

    IFMENU_PLOT_ROW,
    IFMENU_PLOT_COL,
    IFMENU_PLOT_FFT_ROW,
    IFMENU_PLOT_FFT_COL,
    IFMENU_PLOT_HISTOGRAM,

    IFMENU_VIEW_SCALE_AUTO,
    IFMENU_VIEW_SCALE_MINMAX,
    IFMENU_VIEW_SCALE_FULL,

    IFMENU_COMPARE_IMAGES,
    IFMENU_COMPARE_ROW,
    IFMENU_COMPARE_COL,
    IFMENU_IMAGE_SCALESIZE,
    IFMENU_IMAGE_ADD,
    IFMENU_IMAGE_SUBTRACT,
    IFMENU_IMAGE_MULTIPLY,
    IFMENU_IMAGE_DIVIDE,
#ifdef wxUSE_GLCANVAS
    IFMENU_IMAGE_CONVERT3D,
#endif

    IFMENU_FILTER_INVERTVALUES,
    IFMENU_FILTER_SQRT,
    IFMENU_FILTER_SQUARE,
    IFMENU_FILTER_LOG,
    IFMENU_FILTER_EXP,
    IFMENU_FILTER_FOURIER,
    IFMENU_FILTER_INVERSE_FOURIER,
    IFMENU_FILTER_FFT,
    IFMENU_FILTER_IFFT,
    IFMENU_FILTER_FFT_ROWS,
    IFMENU_FILTER_FFT_COLS,
    IFMENU_FILTER_IFFT_ROWS,
    IFMENU_FILTER_IFFT_COLS,
    IFMENU_FILTER_MAGNITUDE,
    IFMENU_FILTER_PHASE,
    IFMENU_FILTER_REAL,
    IFMENU_FILTER_IMAGINARY,
    IFMENU_FILTER_SHUFFLENATURALTOFOURIERORDER,
    IFMENU_FILTER_SHUFFLEFOURIERTONATURALORDER,

    PHMMENU_FILE_PROPERTIES,
    PHMMENU_PROCESS_RASTERIZE,
    PHMMENU_PROCESS_PROJECTIONS,

    PLOTMENU_FILE_PROPERTIES,
    PLOTMENU_VIEW_SCALE_MINMAX,
    PLOTMENU_VIEW_SCALE_AUTO,
    PLOTMENU_VIEW_SCALE_FULL,

    GRAPH3D_VIEW_WIREFRAME,
    GRAPH3D_VIEW_COLOR,
    GRAPH3D_VIEW_LIGHTING,
    GRAPH3D_VIEW_SMOOTH,
    GRAPH3D_VIEW_SCALE_AUTO,
    GRAPH3D_VIEW_SCALE_MINMAX,
    GRAPH3D_VIEW_SCALE_FULL,

    RECONSTRUCTION_THREAD_EVENT,
    NEW_IMAGEFILE_EVENT,
    NEW_PROJECTIONFILE_EVENT,
};

#endif
