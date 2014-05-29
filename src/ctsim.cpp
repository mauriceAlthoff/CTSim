/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          ctsim.cpp
**   Purpose:       Top-level routines of CTSim program
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

#ifdef MSVC
#define strdup _strdup
#endif

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/filesys.h"
#include "wx/fs_zip.h"
#include "wx/cmdline.h"
#ifdef __WXMSW__
#include "wx/msw/helpchm.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "ct.h"
#include "ctndicom.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "dialogs.h"
#include "tips.h"
#include "backgroundmgr.h"

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_GETOPT_H) || defined(HAVE_GETOPT_LONG)
#ifdef MSVC
#define __STDC__ 1
#endif
#include "getopt.h"
#ifdef MSVC
#undef __STDC__
#endif
#endif


IMPLEMENT_APP(CTSimApp)

CTSimApp::CTSimApp()
: m_bAdvancedOptions(false), m_bSetModifyNewDocs(true), 
  m_bVerboseLogging(false), m_bShowStartupTips(true),
  m_iCurrentTip(0), m_bUseBackgroundTasks(false),
  m_docManager(NULL), m_pFrame(NULL), m_pConfig(0), m_pLog(0), m_pLogDoc(0),
  m_bPrintCmdLineImages(false), m_bCmdLineVerboseFlag(false)
{
  theApp = this;
}

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif


void CTSimApp::OnInitCmdLine(wxCmdLineParser& parser)
{
  static const wxCmdLineEntryDesc cmdLineDesc[] = {
    { wxCMD_LINE_SWITCH, _T("l"), _T("verbose"), _T("verbose logging") },
    { wxCMD_LINE_SWITCH, _T("v"), _T("version"), _T("print version") },
    { wxCMD_LINE_SWITCH, _T("p"), _T("print"), _T("print images from command line"),
      wxCMD_LINE_VAL_NONE,
      wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("print this help message"),
      wxCMD_LINE_VAL_NONE,
      wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_PARAM, NULL, NULL, _T("input file"),
      wxCMD_LINE_VAL_STRING,
      wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
  };

  parser.SetDesc(cmdLineDesc);
}

bool CTSimApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
  if (wxApp::OnCmdLineParsed(parser) == false)
    return false;

  if (parser.Found(_T("version"))) {
#ifdef VERSION
      std::cout << "Version: " << VERSION << std::endl;
#elif defined(CTSIMVERSION)
      std::cout << "Version: " << CTSIMVERSION << std::endl;
#else
      std::cout << "Version: " << "Unknown" << std::endl;
#endif
      return false;
  }
  if (parser.Found(_T("print"))) {
    m_bPrintCmdLineImages = true;
  }
  if (parser.Found(_T("verbose"))) {
    m_bCmdLineVerboseFlag = true;
  }

  return true;
}

bool
CTSimApp::OnInit()
{
  if (! wxApp::OnInit())
    return false;

#ifdef HAVE_SETPRIORITY
  setpriority (PRIO_PROCESS, 0, 15);  // set to low scheduling priority
#endif

  openConfig();

  g_bRunningWXWindows = true;
  m_docManager = new wxDocManager (wxDEFAULT_DOCMAN_FLAGS, true);

  m_pDocTemplImage = new wxDocTemplate (m_docManager, _T("ImageFile"), _T("*.if"), _T(""), _T("if"), _T("ImageFile"), _T("ImageView"), CLASSINFO(ImageFileDocument), CLASSINFO(ImageFileView));
  m_pDocTemplProjection = new wxDocTemplate (m_docManager, _T("ProjectionFile"), _T("*.pj"), _T(""), _T("pj"), _T("ProjectionFile"), _T("ProjectionView"), CLASSINFO(ProjectionFileDocument), CLASSINFO(ProjectionFileView));
  m_pDocTemplPhantom = new wxDocTemplate (m_docManager, _T("PhantomFile"), _T("*.phm"), _T(""), _T("phm"), _T("PhantomFile"), _T("PhantomView"), CLASSINFO(PhantomFileDocument), CLASSINFO(PhantomFileView));
  m_pDocTemplPlot = new wxDocTemplate (m_docManager, _T("PlotFile"), _T("*.plt"), _T(""), _T("plt"), _T("PlotFile"), _T("PlotView"), CLASSINFO(PlotFileDocument), CLASSINFO(PlotFileView));
  m_pDocTemplText = new wxDocTemplate (m_docManager, _T("TextFile"), _T("*.txt"), _T(""), _T("txt"), _T("TextFile"), _T("TextView"), CLASSINFO(TextFileDocument), CLASSINFO(TextFileView), wxTEMPLATE_INVISIBLE);
#if wxUSE_GLCANVAS
  m_pDocTemplGraph3d = new wxDocTemplate (m_docManager, _T("Graph3dFile"), _T("*.g3d"), _T(""), _T("g3d"),
                                          _T("Graph3dFile"), _T("Graph3dView"), CLASSINFO(Graph3dFileDocument), 
                                          CLASSINFO(Graph3dFileView), wxTEMPLATE_INVISIBLE);
#endif

#if wxUSE_GIF
  wxImage::AddHandler(new wxGIFHandler);     // Required for images in the online documentation
#endif

#if wxUSE_STREAMS && wxUSE_ZIPSTREAM && wxUSE_ZLIB
  wxFileSystem::AddHandler(new wxZipFSHandler);     // Required for advanced HTML help
#endif

  // Create the main frame window
  int xDisplay, yDisplay;
  ::wxDisplaySize (&xDisplay, &yDisplay);

#ifdef CTSIM_MDI
  wxSize frameSize(nearest<int>(xDisplay * .75), nearest<int>(yDisplay * .75));
#else
  wxSize frameSize(nearest<int>(xDisplay * .6), nearest<int>(yDisplay * .4));
#endif

  m_pFrame = new MainFrame(m_docManager, (wxFrame *) NULL, -1, _T("CTSim"), wxPoint(0, 0), frameSize, wxDEFAULT_FRAME_STYLE);

  setIconForFrame (m_pFrame);
  m_pFrame->Centre(wxBOTH);
  m_pFrame->Show(true);
  SetTopWindow (m_pFrame);

  if (m_pConfig)
    m_docManager->FileHistoryLoad(*m_pConfig);

#ifdef CTSIM_MDI
  m_pLogDoc = newTextDoc();
  if (m_pLogDoc) {
    m_pLog = m_pLogDoc->getTextCtrl();
    m_pLogDoc->SetDocumentName(_T("Log.txt"));
    m_pLogDoc->SetFilename(_T("Log.txt"));
    m_pLogDoc->getView()->getFrame()->SetTitle(_T("Log"));
    int xSize, ySize;
    m_pFrame->GetClientSize(&xSize, &ySize);
    int yLogSize = ySize / 4;
    m_pLogDoc->getView()->getFrame()->SetSize (0, ySize - yLogSize, xSize, yLogSize);
    m_pLogDoc->getView()->getFrame()->Show (true);
  } else
#else
    m_pLog = new wxTextCtrl (m_pFrame, -1, _T("Log Window\n"), wxPoint(0, 0), frameSize, wxTE_MULTILINE | wxTE_READONLY);
#endif
  wxLog::SetActiveTarget (new wxLogTextCtrl(m_pLog));

  wxString helpDir;
  if (! m_pConfig->Read(_T("HelpDir"), &helpDir))
    helpDir = ::wxGetCwd();
#ifdef CTSIM_WINHELP
  if (! m_pFrame->getWinHelpController().Initialize(helpDir + _T("/ctsim")))
    *m_pLog << _T("Cannot initialize the Windows Help system") << _T("\n");
#else
#ifdef DATADIR
  wxString docDir (DATADIR, *wxConvCurrent);
#else
  wxString docDir (::wxGetCwd());
#endif
  wxString docFile = docDir + _T("ctsim.htb");
  if (! m_pFrame->getHtmlHelpController().AddBook(docFile) &&
      ! m_pFrame->getHtmlHelpController().AddBook(_T("/usr/share/ctsim/ctsim.htb")) &&
      ! m_pFrame->getHtmlHelpController().AddBook(_T("/tmp/ctsim.htb")))
    *m_pLog << _T("Cannot initialize the HTML Help system") << _T("\n");
  else {
    if (::wxDirExists (_T("/tmp")))
      m_pFrame->getHtmlHelpController().SetTempDir(_T("/tmp"));
    m_pFrame->getHtmlHelpController().UseConfig (m_pConfig);
  }
#endif

  for (int i = optind + 1; i <= argc; i++) {
    wxString filename = argv [i - 1];
    wxDocument* pNewDoc = m_docManager->CreateDocument (filename, wxDOC_SILENT);
    if (m_bPrintCmdLineImages) {
      wxView* pNewView = pNewDoc->GetFirstView();
      wxPrintout *printout = pNewView->OnCreatePrintout();
      if (printout) {
        wxPrinter printer;
        printer.Print(pNewView->GetFrame(), printout, TRUE);
        delete printout;
      }
      wxCommandEvent nullEvent;
      nullEvent.SetId (wxID_CLOSE);
      m_docManager->OnFileClose (nullEvent);
    }
  }
  if (m_bPrintCmdLineImages) {
    wxCommandEvent closeEvent;
    closeEvent.SetInt (MAINMENU_FILE_EXIT);
    m_pFrame->AddPendingEvent(closeEvent);
  }

  if (getStartupTips())
    ShowTips();

#ifdef HAVE_WXTHREADS
  m_pBackgroundMgr = new BackgroundManager;
#endif

  return true;
}

void
CTSimApp::ShowTips()
{
  CTSimTipProvider tipProvider (m_iCurrentTip);
  setStartupTips (::wxShowTip (m_pFrame, &tipProvider, getStartupTips()));
  m_iCurrentTip = tipProvider.GetCurrentTip();
}


#include "./ctsim.xpm"
void
CTSimApp::setIconForFrame(wxFrame* pFrame)
{
  wxIcon iconApp (ctsim16_xpm);

  if (iconApp.Ok())
    pFrame->SetIcon (iconApp);
}

void
CTSimApp::usage(const char* program)
{
  std::cout << "usage: " << fileBasename(program) << " [files-to-open...] [OPTIONS]\n";
  std::cout << "Computed Tomography Simulator (Graphical Shell)\n";
  std::cout << "\n";
  std::cout << "  --version Display version\n";
  std::cout << "  --help    Display this help message\n";
}

int
CTSimApp::OnExit()
{
  closeConfig();

#ifdef HAVE_DMALLOC
  dmalloc_shutdown();
#endif
  return 0;
}

void
CTSimApp::openConfig()
{
#ifdef MSVC
  m_pConfig = new wxConfig(_T("ctsim"), _T("Kevin Rosenberg"), _T(""), _T(""), wxCONFIG_USE_LOCAL_FILE);
#else
  m_pConfig = new wxConfig(_T("ctsim"), _T("Kevin Rosenberg"), _T(".ctsim"), _T(""), wxCONFIG_USE_LOCAL_FILE);
#endif

  wxConfigBase::Set(m_pConfig);
  m_pConfig->Read (_T("AdvancedOptions"), &m_bAdvancedOptions);
  m_pConfig->Read (_T("SetModifyNewDocs"), &m_bSetModifyNewDocs);
  m_pConfig->Read (_T("VerboseLogging"), &m_bVerboseLogging);
  m_pConfig->Read (_T("StartupTips"), &m_bShowStartupTips);
  m_pConfig->Read (_T("CurrentTip"), &m_iCurrentTip);
  m_pConfig->Read (_T("UseBackgroundTasks"), &m_bUseBackgroundTasks);
#ifdef HAVE_FFTW
  wxString strFftwWisdom;
  m_pConfig->Read (_T("FftwWisdom"), strFftwWisdom);
  if (strFftwWisdom.size() > 0)
    fftw_import_wisdom_from_string (strFftwWisdom.mb_str(wxConvUTF8));
#endif
}

void
CTSimApp::closeConfig()
{
  m_pConfig->Write (_T("AdvancedOptions"), m_bAdvancedOptions);
  m_pConfig->Write (_T("SetModifyNewDocs"), m_bSetModifyNewDocs);
  m_pConfig->Write (_T("VerboseLogging"), m_bVerboseLogging);
  m_pConfig->Write (_T("StartupTips"), m_bShowStartupTips);
  m_pConfig->Write (_T("CurrentTip"), m_iCurrentTip);
  m_pConfig->Write (_T("UseBackgroundTasks"), m_bUseBackgroundTasks);
#ifdef HAVE_FFTW
  const char* const pszWisdom = fftw_export_wisdom_to_string();
  wxString strFftwWisdom (pszWisdom, *wxConvCurrent);
  fftw_free ((void*) pszWisdom);
  m_pConfig->Write (_T("FftwWisdom"), strFftwWisdom);
#endif

  delete m_pConfig;
}


// Top-level window for CTSim

#if CTSIM_MDI
IMPLEMENT_CLASS(MainFrame, wxMDIParentFrame)

BEGIN_EVENT_TABLE(MainFrame, wxMDIParentFrame)
#else
IMPLEMENT_CLASS(MainFrame, wxDocParentFrame)

BEGIN_EVENT_TABLE(MainFrame, wxDocParentFrame)
#endif

EVT_MENU(MAINMENU_FILE_PREFERENCES, MainFrame::OnPreferences)
EVT_MENU(MAINMENU_HELP_ABOUT, MainFrame::OnAbout)
EVT_MENU(MAINMENU_HELP_CONTENTS, MainFrame::OnHelpContents)
EVT_MENU(MAINMENU_HELP_TIPS, MainFrame::OnHelpTips)
EVT_MENU(MAINMENU_IMPORT, MainFrame::OnImport)
EVT_MENU(IDH_QUICKSTART, MainFrame::OnHelpButton)
EVT_MENU(MAINMENU_LOG_EVENT, MainFrame::OnLogEvent)
EVT_MENU(NEW_IMAGEFILE_EVENT, MainFrame::OnNewImageFile)
EVT_MENU(NEW_PROJECTIONFILE_EVENT, MainFrame::OnNewProjectionFile)
EVT_BUTTON(IDH_DLG_RASTERIZE, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_PROJECTIONS, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_RECONSTRUCTION, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_FILTER, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_MINMAX, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_EXPORT, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_PHANTOM, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_COMPARISON, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_PREFERENCES, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_POLAR, MainFrame::OnHelpButton)
EVT_BUTTON(IDH_DLG_AUTOSCALE, MainFrame::OnHelpButton)

EVT_SIZE(MainFrame::OnSize)

#if defined(CTSIM_WINHELP) && (defined(DEBUG) || defined(_DEBUG))
EVT_MENU(MAINMENU_HELP_SECONDARY, MainFrame::OnHelpSecondary)
#endif
EVT_MENU(MAINMENU_FILE_CREATE_PHANTOM, MainFrame::OnCreatePhantom)
EVT_MENU(MAINMENU_FILE_CREATE_FILTER, MainFrame::OnCreateFilter)
EVT_MENU(MAINMENU_FILE_EXIT, MainFrame::OnExit)
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MainFrame::OnMRUFile)
EVT_MENU(MAINMENU_WINDOW_BASE, MainFrame::OnWindowMenu0)
EVT_MENU(MAINMENU_WINDOW_BASE+1, MainFrame::OnWindowMenu1)
EVT_MENU(MAINMENU_WINDOW_BASE+2, MainFrame::OnWindowMenu2)
EVT_MENU(MAINMENU_WINDOW_BASE+3, MainFrame::OnWindowMenu3)
EVT_MENU(MAINMENU_WINDOW_BASE+4, MainFrame::OnWindowMenu4)
EVT_MENU(MAINMENU_WINDOW_BASE+5, MainFrame::OnWindowMenu5)
EVT_MENU(MAINMENU_WINDOW_BASE+6, MainFrame::OnWindowMenu6)
EVT_MENU(MAINMENU_WINDOW_BASE+7, MainFrame::OnWindowMenu7)
EVT_MENU(MAINMENU_WINDOW_BASE+8, MainFrame::OnWindowMenu8)
EVT_MENU(MAINMENU_WINDOW_BASE+9, MainFrame::OnWindowMenu9)
EVT_MENU(MAINMENU_WINDOW_BASE+10, MainFrame::OnWindowMenu10)
EVT_MENU(MAINMENU_WINDOW_BASE+11, MainFrame::OnWindowMenu11)
EVT_MENU(MAINMENU_WINDOW_BASE+12, MainFrame::OnWindowMenu12)
EVT_MENU(MAINMENU_WINDOW_BASE+13, MainFrame::OnWindowMenu13)
EVT_MENU(MAINMENU_WINDOW_BASE+14, MainFrame::OnWindowMenu14)
EVT_MENU(MAINMENU_WINDOW_BASE+15, MainFrame::OnWindowMenu15)
EVT_MENU(MAINMENU_WINDOW_BASE+16, MainFrame::OnWindowMenu16)
EVT_MENU(MAINMENU_WINDOW_BASE+17, MainFrame::OnWindowMenu17)
EVT_MENU(MAINMENU_WINDOW_BASE+18, MainFrame::OnWindowMenu18)
EVT_MENU(MAINMENU_WINDOW_BASE+19, MainFrame::OnWindowMenu19)
EVT_UPDATE_UI_RANGE(MAINMENU_WINDOW_BASE, MAINMENU_WINDOW_BASE+20, MainFrame::OnUpdateUI)
END_EVENT_TABLE()



#if CTSIM_MDI
MainFrame::MainFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type)
: wxDocMDIParentFrame(manager, NULL, id, title, pos, size, type, _T("MainFrame"))
#else
MainFrame::MainFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type)
: wxDocParentFrame(manager, frame, id, title, pos, size, type, _T("MainFrame"))
#endif
{
  m_bShuttingDown = false;

  //// Make a menubar
  wxMenu *file_menu = new wxMenu;

  file_menu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  file_menu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  file_menu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));

  file_menu->AppendSeparator();
  file_menu->Append (MAINMENU_IMPORT, _T("&Import...\tCtrl-M"));
  file_menu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  file_menu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));

  //  history of files visited
  theApp->getDocManager()->FileHistoryAddFilesToMenu(file_menu);
  theApp->getDocManager()->FileHistoryUseMenu(file_menu);

#ifndef CTSIM_MDI
  m_pWindowMenu = new wxMenu;
  m_pWindowMenu->UpdateUI (this);
#endif

  wxMenu* help_menu = new wxMenu;
  help_menu->Append (MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append (MAINMENU_HELP_TIPS, _T("&Tips"));
  help_menu->Append (IDH_QUICKSTART, _T("&Quick Start"));
#if defined(CTSIM_WINHELP) && (defined(DEBUG) || defined(_DEBUG))
  help_menu->Append (MAINMENU_HELP_SECONDARY, _T("&Secondary Help"));
#endif
  help_menu->Append (MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar* menu_bar = new wxMenuBar;

  menu_bar->Append(file_menu, _T("&File"));
#ifndef CTSIM_MDI
  menu_bar->Append(m_pWindowMenu, _T("&Window"));
#endif
  menu_bar->Append(help_menu, _T("&Help"));

  SetMenuBar(menu_bar);


#ifndef CTSIM_MDI
  int i;
  for (i = 0; i < MAX_WINDOW_MENUITEMS; i++) {
    m_apWindowMenuItems[i] = new wxMenuItem (m_pWindowMenu, MAINMENU_WINDOW_BASE+i, _T("[EMPTY]"));
    m_pWindowMenu->Append (m_apWindowMenuItems[i]);
    m_pWindowMenu->Enable (MAINMENU_WINDOW_BASE+i, false);
  }
#endif

  m_iDefaultPhantomID = Phantom::PHM_HERMAN;
  m_iDefaultFilterID = SignalFilter::FILTER_BANDLIMIT;
  m_iDefaultFilterDomainID = SignalFilter::DOMAIN_FREQUENCY;
  m_iDefaultFilterXSize = 256;
  m_iDefaultFilterYSize = 256;
  m_dDefaultFilterParam = 1.;
  m_dDefaultFilterBandwidth = 1.;
  m_dDefaultFilterInputScale = 1.;
  m_dDefaultFilterOutputScale = 1.;
  m_iDefaultImportFormat = ImageFile::IMPORT_FORMAT_PNG;

  wxAcceleratorEntry accelEntries[15];
  accelEntries[0].Set (wxACCEL_CTRL, static_cast<int>('O'), wxID_OPEN);
  accelEntries[1].Set (wxACCEL_CTRL, static_cast<int>('P'), MAINMENU_FILE_CREATE_PHANTOM);
  accelEntries[2].Set (wxACCEL_CTRL, static_cast<int>('F'), MAINMENU_FILE_CREATE_FILTER);
  accelEntries[3].Set (wxACCEL_CTRL, static_cast<int>('M'), MAINMENU_IMPORT);
  accelEntries[4].Set (wxACCEL_NORMAL, WXK_F1, MAINMENU_HELP_CONTENTS);
#ifndef CTSIM_MDI
  for (i = 0; i < 10; i++)
    accelEntries[i+5].Set (wxACCEL_CTRL, static_cast<int>('0'+i), MAINMENU_WINDOW_BASE+i);
  wxAcceleratorTable accelTable (15, accelEntries);
#else
  wxAcceleratorTable accelTable (5, accelEntries);
#endif

  SetAcceleratorTable (accelTable);
}

MainFrame::~MainFrame()
{
  m_bShuttingDown = true; // Currently used so that Log Window will close
#if 0
  // delete all non-modified documents
  wxList& rListDocs = theApp->getDocManager()->GetDocuments();
  for (wxNode* pNode = rListDocs.GetFirst(); pNode != NULL; pNode = pNode->GetNext()) {
    wxDocument* pDoc = dynamic_cast<wxDocument*>(pNode->GetData());
    if (pDoc && ! pDoc->IsModified()) {
      theApp->getDocManager()->RemoveDocument(pDoc);
      delete pDoc;
    }
  }
#endif
  ::wxYield();
  if (theApp->getConfig())
    theApp->getDocManager()->FileHistorySave (*theApp->getConfig());
  ::wxYield();
  delete theApp->getDocManager();

}

void
MainFrame::OnSize (wxSizeEvent& event)
{
#ifdef CTSIM_MDI
  if (theApp->getLogDoc()) {
    int xSize, ySize;
    GetClientSize(&xSize, &ySize);
    int yLogSize = ySize / 4;
    theApp->getLogDoc()->getView()->getFrame()->SetSize (0, ySize - yLogSize, xSize, yLogSize);
    theApp->getLogDoc()->getView()->getFrame()->Show (true);
  }
#endif

#if CTSIM_MDI
  wxDocMDIParentFrame::OnSize (event);
#else
  wxDocParentFrame::OnSize (event);
#endif
}

void
MainFrame::OnCreatePhantom(wxCommandEvent& event)
{
  DialogGetPhantom dialogPhantom (this, m_iDefaultPhantomID);
  int dialogReturn = dialogPhantom.ShowModal();
  if (dialogReturn == wxID_OK) {
    wxString selection (dialogPhantom.getPhantom(), *wxConvCurrent);
    if (theApp->getVerboseLogging())
      *theApp->getLog() << _T("Selected phantom ") << selection.c_str() << _T("\n");
    wxString filename = selection + _T(".phm");
    m_iDefaultPhantomID = Phantom::convertNameToPhantomID (selection.mb_str(wxConvUTF8));
    theApp->getDocManager()->CreateDocument (filename, wxDOC_SILENT);
  }

}

void
MainFrame::OnCreateFilter (wxCommandEvent& WXUNUSED(event))
{
  DialogGetFilterParameters dialogFilter (this, m_iDefaultFilterXSize, m_iDefaultFilterYSize, m_iDefaultFilterID, m_dDefaultFilterParam, m_dDefaultFilterBandwidth, m_iDefaultFilterDomainID, m_dDefaultFilterInputScale, m_dDefaultFilterOutputScale);
  int dialogReturn = dialogFilter.ShowModal();
  if (dialogReturn == wxID_OK) {
    wxString strFilter (dialogFilter.getFilterName(), *wxConvCurrent);
    wxString strDomain (dialogFilter.getDomainName(), *wxConvCurrent);
    m_iDefaultFilterID = SignalFilter::convertFilterNameToID (strFilter.mb_str(wxConvUTF8));
    m_iDefaultFilterDomainID = SignalFilter::convertDomainNameToID (strDomain.mb_str(wxConvUTF8));
    m_iDefaultFilterXSize = dialogFilter.getXSize();
    m_iDefaultFilterYSize = dialogFilter.getYSize();
    m_dDefaultFilterBandwidth = dialogFilter.getBandwidth();
    m_dDefaultFilterParam= dialogFilter.getFilterParam();
    m_dDefaultFilterInputScale = dialogFilter.getInputScale();
    m_dDefaultFilterOutputScale = dialogFilter.getOutputScale();
    wxString os;
    os << _T("Generate Filter=") << strFilter
       << _T(", size=(") << static_cast<int>(m_iDefaultFilterXSize) << _T(",")
       << static_cast<int>(m_iDefaultFilterYSize)
       << _T("), domain=") << strDomain.c_str() << _T(", filterParam=")
       << m_dDefaultFilterParam << _T(", bandwidth=") <<
      m_dDefaultFilterBandwidth
       << _T(", inputScale=") << m_dDefaultFilterInputScale << _T(", outputScale=") << m_dDefaultFilterOutputScale;
    *theApp->getLog() << os << _T("\n");
    ImageFileDocument* pFilterDoc = theApp->newImageDoc();
    pFilterDoc->setBadFileOpen();
    if (! pFilterDoc) {
      sys_error (ERR_SEVERE, "Unable to create filter image");
      return;
    }
    ImageFile& rIF = pFilterDoc->getImageFile();
    rIF.setArraySize (m_iDefaultFilterXSize, m_iDefaultFilterYSize);
    rIF.filterResponse (strDomain.mb_str(wxConvUTF8), m_dDefaultFilterBandwidth, strFilter.mb_str(wxConvUTF8), m_dDefaultFilterParam, m_dDefaultFilterInputScale, m_dDefaultFilterOutputScale);
    rIF.labelAdd (os.mb_str(wxConvUTF8));
    if (theApp->getAskDeleteNewDocs())
      pFilterDoc->Modify (true);
    pFilterDoc->UpdateAllViews();
    pFilterDoc->GetFirstView()->OnUpdate (NULL, NULL);
    pFilterDoc->getView()->getFrame()->SetClientSize(m_iDefaultFilterXSize, m_iDefaultFilterYSize);
    pFilterDoc->getView()->getFrame()->Show(true);
  }
}

void
CTSimApp::getCompatibleImages (const ImageFileDocument* pIFDoc, std::vector<ImageFileDocument*>& vecIF)
{
  const ImageFile& rIF = pIFDoc->getImageFile();
  unsigned int nx = rIF.nx();
  unsigned int ny = rIF.ny();
  wxList& rListDocs = m_docManager->GetDocuments();
  for (wxNode* pNode = rListDocs.GetFirst(); pNode != NULL; pNode = pNode->GetNext()) {
    wxDocument* pDoc = reinterpret_cast<wxDocument*>(pNode->GetData());
    ImageFileDocument* pIFCompareDoc = dynamic_cast<ImageFileDocument*>(pDoc);
    if (pIFCompareDoc && (pIFDoc != pIFCompareDoc)) {
      const ImageFile& rCompareIF = pIFCompareDoc->getImageFile();
      if (rCompareIF.nx() == nx && rCompareIF.ny() == ny)
        vecIF.push_back (pIFCompareDoc);
    }
  }
}


void
MainFrame::OnNewImageFile (wxCommandEvent& event)
{
  ImageFile* pImageFile = reinterpret_cast<ImageFile*>(event.GetClientData());

  ImageFileDocument* pImageDoc = theApp->newImageDoc();
  if (! pImageDoc) {
    sys_error (ERR_SEVERE, "Unable to create image file");
    return;
  }
  pImageDoc->setImageFile (pImageFile);
  if (theApp->getAskDeleteNewDocs())
    pImageDoc->Modify (true);
}

void
MainFrame::OnNewProjectionFile (wxCommandEvent& event)
{
  Projections* pProjections = reinterpret_cast<Projections*>(event.GetClientData());
  ProjectionFileDocument* pProjDoc = theApp->newProjectionDoc();
  if (! pProjDoc) {
    sys_error (ERR_SEVERE, "Unable to create projection file");
    return;
  }
  pProjDoc->setProjections (pProjections);
  if (theApp->getAskDeleteNewDocs())
    pProjDoc->Modify (true);
}

void
MainFrame::OnLogEvent (wxCommandEvent& event)
{
  *theApp->getLog() << event.GetString();
}

void
MainFrame::OnHelpTips (wxCommandEvent& event)
{
  theApp->ShowTips();
}

void
MainFrame::OnHelpContents (wxCommandEvent& event)
{
  showHelp (event.GetId());
}

void
MainFrame::OnHelpButton (wxCommandEvent& event)
{
  showHelp (event.GetId());
}

#if defined(CTSIM_WINHELP) && (defined(DEBUG) || defined(_DEBUG))
void
MainFrame::OnHelpSecondary (wxCommandEvent& event)
{
  m_htmlHelp.Display ("Contents");
}
#endif

void
MainFrame::showHelp (int commandID)
{
  switch (commandID) {

  case MAINMENU_HELP_CONTENTS:
#ifdef CTSIM_WINHELP
    m_winHelp.DisplayContents ();
#else
    m_htmlHelp.Display (_T("Contents"));
#endif
    break;

  default:
#ifdef CTSIM_WINHELP
    m_winHelp.DisplaySection (commandID);
#else
    m_htmlHelp.Display (commandID);
#endif
    break;
  }
}

void
MainFrame::OnExit (wxCommandEvent& WXUNUSED(event) )
{
  Close(true);
}

void
MainFrame::OnUpdateUI (wxUpdateUIEvent& rEvent)
{
#ifndef CTSIM_MDI
  int iPos = 0;
  wxList& rListDocs = theApp->getDocManager()->GetDocuments();
  wxNode* pNode = rListDocs.GetFirst();
  while (iPos < MAX_WINDOW_MENUITEMS && pNode != NULL) {
    wxDocument* pDoc = static_cast<wxDocument*>(pNode->GetData());
    wxString strFilename = pDoc->GetFilename();
    if (iPos < 10) {
      strFilename += _T("\tCtrl-");
      strFilename += static_cast<char>('0' + iPos);
    }
    static_cast<wxMenuItemBase*>(m_apWindowMenuItems[iPos])->SetName (strFilename);
    m_apWindowMenuData[iPos] = pDoc;
    m_pWindowMenu->Enable (MAINMENU_WINDOW_BASE+iPos, true);
    iPos++;
    pNode = pNode->GetNext();
  }
  for (int i = iPos; i < MAX_WINDOW_MENUITEMS; i++) {
    m_pWindowMenu->Enable (MAINMENU_WINDOW_BASE+i, false);
    static_cast<wxMenuItemBase*>(m_apWindowMenuItems[i])->SetName (_T("[EMPTY]"));
    m_apWindowMenuData[i] = NULL;
  }
#endif
}


void
MainFrame::DoWindowMenu (int iMenuPosition, wxCommandEvent& event)
{
  if (wxDocument* pDoc = m_apWindowMenuData [iMenuPosition]) {
    wxString strFilename = pDoc->GetFilename();
    const wxView* pView = pDoc->GetFirstView();
    if (pView) {
      wxWindow* pWindow = pView->GetFrame();
      pWindow->SetFocus();
      pWindow->Raise();
    }
  }
}

void MainFrame::OnWindowMenu0 (wxCommandEvent& event)
{ DoWindowMenu (0, event); }

void MainFrame::OnWindowMenu1 (wxCommandEvent& event)
{ DoWindowMenu (1, event); }

void MainFrame::OnWindowMenu2 (wxCommandEvent& event)
{ DoWindowMenu (2, event); }

void MainFrame::OnWindowMenu3 (wxCommandEvent& event)
{ DoWindowMenu (3, event); }

void MainFrame::OnWindowMenu4 (wxCommandEvent& event)
{ DoWindowMenu (4, event); }

void MainFrame::OnWindowMenu5 (wxCommandEvent& event)
{ DoWindowMenu (5, event); }

void MainFrame::OnWindowMenu6 (wxCommandEvent& event)
{ DoWindowMenu (6, event); }

void MainFrame::OnWindowMenu7 (wxCommandEvent& event)
{ DoWindowMenu (7, event); }

void MainFrame::OnWindowMenu8 (wxCommandEvent& event)
{ DoWindowMenu (8, event); }

void MainFrame::OnWindowMenu9 (wxCommandEvent& event)
{ DoWindowMenu (9, event); }

void MainFrame::OnWindowMenu10 (wxCommandEvent& event)
{ DoWindowMenu (10, event); }

void MainFrame::OnWindowMenu11 (wxCommandEvent& event)
{ DoWindowMenu (11, event); }

void MainFrame::OnWindowMenu12 (wxCommandEvent& event)
{ DoWindowMenu (12, event); }

void MainFrame::OnWindowMenu13 (wxCommandEvent& event)
{ DoWindowMenu (13, event); }

void MainFrame::OnWindowMenu14 (wxCommandEvent& event)
{ DoWindowMenu (14, event); }

void MainFrame::OnWindowMenu15 (wxCommandEvent& event)
{ DoWindowMenu (15, event); }

void MainFrame::OnWindowMenu16 (wxCommandEvent& event)
{ DoWindowMenu (16, event); }

void MainFrame::OnWindowMenu17 (wxCommandEvent& event)
{ DoWindowMenu (17, event); }

void MainFrame::OnWindowMenu18 (wxCommandEvent& event)
{ DoWindowMenu (18, event); }

void MainFrame::OnWindowMenu19 (wxCommandEvent& event)
{ DoWindowMenu (19, event); }


class BitmapControl : public wxPanel
{
private:
  DECLARE_DYNAMIC_CLASS (BitmapControl)
    DECLARE_EVENT_TABLE ()
    wxBitmap* m_pBitmap;

public:
  BitmapControl (wxBitmap* pBitmap, wxWindow *parent, wxWindowID id = -1,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxSTATIC_BORDER,
    const wxValidator& validator = wxDefaultValidator,
                 const wxString& name = _T("BitmapCtrl"));


  virtual ~BitmapControl();

  virtual wxSize GetBestSize() const;

  wxBitmap* getBitmap()
  { return m_pBitmap; }

  void OnPaint(wxPaintEvent& event);
};


BEGIN_EVENT_TABLE(BitmapControl, wxPanel)
EVT_PAINT(BitmapControl::OnPaint)
END_EVENT_TABLE()

IMPLEMENT_CLASS(BitmapControl, wxPanel)


BitmapControl::BitmapControl (wxBitmap* pBitmap, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                              long style, const wxValidator& validator, const wxString& name)
                              : m_pBitmap(pBitmap)
{
  Create(parent, id, pos, size, style, name);

  SetSize (GetBestSize());
}

wxSize
BitmapControl::GetBestSize () const
{
  if (m_pBitmap)
    return wxSize (m_pBitmap->GetWidth(), m_pBitmap->GetHeight());
  else
    return wxSize(0,0);
}

BitmapControl::~BitmapControl()
{}

void
BitmapControl::OnPaint (wxPaintEvent& event)
{
  wxPaintDC dc(this);
  if (m_pBitmap)
    dc.DrawBitmap (*m_pBitmap, 0, 0);
}


class BitmapDialog : public wxDialog {
private:
  BitmapControl* m_pBitmapCtrl;

public:
  BitmapDialog (wxBitmap* pBitmap, char const* pszTitle);
  virtual ~BitmapDialog();
};

BitmapDialog::BitmapDialog (wxBitmap* pBitmap, char const* pszTitle)
  : wxDialog(theApp->getMainFrame(), -1, wxString(pszTitle,*wxConvCurrent), wxDefaultPosition, wxDefaultSize, wxDIALOG_MODAL | wxDEFAULT_DIALOG_STYLE)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new BitmapControl (pBitmap, this), 0, wxALIGN_CENTER | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Ok"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

BitmapDialog::~BitmapDialog()
{}


void
MainFrame::OnPreferences (wxCommandEvent& WXUNUSED(event) )
{
  DialogPreferences dlg (this, _T("CTSim Preferences"), theApp->getAdvancedOptions(),
    theApp->getAskDeleteNewDocs(), theApp->getVerboseLogging(), theApp->getStartupTips(),
    theApp->getUseBackgroundTasks());
  if (dlg.ShowModal() == wxID_OK) {
    theApp->setAdvancedOptions (dlg.getAdvancedOptions());
    theApp->setAskDeleteNewDocs (dlg.getAskDeleteNewDocs());
    theApp->setVerboseLogging (dlg.getVerboseLogging());
    theApp->setStartupTips (dlg.getStartupTips());
    theApp->setUseBackgroundTasks (dlg.getUseBackgroundTasks());
  }
}

void
MainFrame::OnImport (wxCommandEvent& WXUNUSED(event) )
{
  DialogImportParameters dialogImport (this, m_iDefaultImportFormat);
  if (dialogImport.ShowModal() != wxID_OK)
    return;

  wxString strFormatName (dialogImport.getFormatName (), *wxConvCurrent);
  m_iDefaultImportFormat = ImageFile::convertImportFormatNameToID (strFormatName.mb_str(wxConvUTF8));

  wxString strExt;
  wxString strWildcard;
  if (m_iDefaultImportFormat == ImageFile::IMPORT_FORMAT_PPM) {
    strExt = _T(".ppm");
    strWildcard = _T("PPM Files (*.ppm)|*.ppm|PGM Files (*.pgm)|*.pgm");
  }
#ifdef HAVE_PNG
  else if (m_iDefaultImportFormat == ImageFile::IMPORT_FORMAT_PNG) {
    strExt = _T(".png");
    strWildcard = _T("PNG Files (*.png)|*.png");
  }
#endif
#ifdef HAVE_CTN_DICOM
  else if (m_iDefaultImportFormat == ImageFile::IMPORT_FORMAT_DICOM) {
    strExt = _T("*.*");
    strWildcard = _T("Dicom Files (*.*)|*.*");
  }
#endif
  else {
    return;
  }

  wxString strFilename = wxFileSelector (wxString(wxConvUTF8.cMB2WX("Import Filename")), wxString(wxConvUTF8.cMB2WX("")),
                                         wxString(wxConvUTF8.cMB2WX("")), strExt, strWildcard, wxOPEN);

  if (! strFilename.IsEmpty()) {
    if (m_iDefaultImportFormat == ImageFile::IMPORT_FORMAT_PPM || m_iDefaultImportFormat == ImageFile::IMPORT_FORMAT_PNG) {
      ImageFile* pIF = new ImageFile;
      if (pIF->importImage (strFormatName.mb_str(wxConvUTF8), strFilename.mb_str(wxConvUTF8))) {
        ImageFileDocument* pIFDoc = theApp->newImageDoc();
        pIFDoc->setImageFile(pIF);
        pIFDoc->getView()->getFrame()->Show(true);
        std::ostringstream os;
        os << "Import file " << strFilename.c_str() << " (type " << strFormatName.c_str() << ")";
        pIF->labelAdd (os.str().c_str());
        if (theApp->getAskDeleteNewDocs())
          pIFDoc->Modify (true);
        pIFDoc->UpdateAllViews();
        pIFDoc->GetFirstView()->OnUpdate (NULL, NULL);
        pIFDoc->getView()->getFrame()->Show(true);
      } else
        delete pIF;
    }
#ifdef HAVE_CTN_DICOM
    else if (m_iDefaultImportFormat == ImageFile::IMPORT_FORMAT_DICOM) {
      DicomImporter dicomImport (strFilename.mb_str(wxConvUTF8));
      if (dicomImport.fail()) {
        ::wxMessageBox (wxConvUTF8.cMB2WX(dicomImport.failMessage().c_str()), _T("Import Error"));
      } else if (dicomImport.testImage()) {
        ImageFileDocument* pIFDoc = theApp->newImageDoc();
        ImageFile* pIF = dicomImport.getImageFile();
        pIFDoc->setImageFile (pIF);
        std::ostringstream os;
        os << "Import file " << strFilename.c_str() << " (type " << strFormatName.c_str() << ")";
        pIF->labelAdd (os.str().c_str());
        if (theApp->getAskDeleteNewDocs())
          pIFDoc->Modify (true);
        pIFDoc->UpdateAllViews();
        pIFDoc->getView()->setInitialClientSize();
        pIFDoc->Activate();
      } else if (dicomImport.testProjections()) {
        ProjectionFileDocument* pProjDoc = theApp->newProjectionDoc();
        Projections* pProj = dicomImport.getProjections();
        pProjDoc->setProjections (pProj);
        pProjDoc->getView()->getFrame()->Show(true);
        std::ostringstream os;
        os << "Import projection file " << strFilename.c_str() << " (type " << strFormatName.c_str() << ")";
        pProj->setRemark (os.str().c_str());
        if (theApp->getAskDeleteNewDocs())
          pProjDoc->Modify (true);
        pProjDoc->UpdateAllViews();
        pProjDoc->getView()->setInitialClientSize();
        pProjDoc->Activate();
      } else
        ::wxMessageBox (_T("Unrecognized DICOM file contents"), _T("Import Error"));
    }
#endif
    else
      sys_error (ERR_WARNING, "Unknown import format type");
  }
}

#include "./splash.xpm"
void
MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
  wxString strOSDesc = ::wxGetOsDescription();
  *theApp->getLog() << _T("Operating System: ") << strOSDesc;
  *theApp->getLog() << _T(", wxWindows: ") << wxVERSION_STRING;
#ifdef _TIMESTAMP__
  *theApp->getLog() << _T(", Build Date: ") << wxConvUTF8.cMB2WX(_TIMESTAMP__);
#endif
#if defined(DEBUG)
  *theApp->getLog() << _T(", Debug version");
#else
  *theApp->getLog() << _T(", Release version");
#endif
#ifdef VERSION
  *theApp->getLog() << _T(" ") <<  wxConvUTF8.cMB2WX(VERSION);
#elif defined(CTSIMVERSION)
  *theApp->getLog() << _T(" ") <<  _T(CTSIMVERSION);
#endif
    *theApp->getLog() << _T("\n");

  wxBitmap bmp (splash);
  if (bmp.Ok()) {
    BitmapDialog dlg (&bmp, "About CTSim");
    dlg.ShowModal();
  } else {
    wxString msg = _T("CTSim\nThe Open Source Computed Tomography Simulator\n");
#ifdef VERSION
    msg << _T("Version: ") <<  wxConvUTF8.cMB2WX(VERSION) << _T("\n\n");
#elif defined(CTSIMVERSION)
    msg << _T("Version: ") <<  wxConvUTF8.cMB2WX(CTSIMVERSION) << _T("\n\n");
#endif
    msg += _T("Author: Kevin Rosenberg <kevin@rosenberg.net>\nUsage: ctsim [files-to-open..] [--help]");

    wxMessageBox(msg, _T("About CTSim"), wxOK | wxICON_INFORMATION, this);
    *theApp->getLog() << msg << wxConvUTF8.cMB2WX("\n");
  }
}


// Create new documents

wxDocument* 
CTSimApp::newDocumentHelper (wxDocTemplate* tmpl) {
  wxDocument* newDoc = tmpl->CreateDocument (_T(""));
  if (newDoc) {
    newDoc->SetDocumentTemplate (tmpl);
    newDoc->OnNewDocument();
    wxString fname = newDoc->GetFilename();
    fname += _T(".");
    fname += tmpl->GetDefaultExtension();
    newDoc->SetDocumentName(fname);
    newDoc->SetTitle(fname);
    newDoc->SetFilename(fname, true);
  }

  return newDoc;
}

ProjectionFileDocument*
CTSimApp::newProjectionDoc()
{
  ProjectionFileDocument* newDoc = dynamic_cast<ProjectionFileDocument*>
    (newDocumentHelper(m_pDocTemplProjection));
  if (newDoc)
    newDoc->getView()->getFrame()->SetTitle(newDoc->GetDocumentName());
  return newDoc;
}

ImageFileDocument*
CTSimApp::newImageDoc()
{
  ImageFileDocument* newDoc = dynamic_cast<ImageFileDocument*>
    (newDocumentHelper(m_pDocTemplImage));
  if (newDoc)
    newDoc->getView()->getFrame()->SetTitle(newDoc->GetDocumentName());

  return newDoc;
}

PlotFileDocument*
CTSimApp::newPlotDoc()
{
  PlotFileDocument* newDoc = dynamic_cast<PlotFileDocument*>
    (newDocumentHelper(m_pDocTemplPlot));
  if (newDoc)
    newDoc->getView()->getFrame()->SetTitle(newDoc->GetDocumentName());

  return newDoc;
}


TextFileDocument*
CTSimApp::newTextDoc()
{
  TextFileDocument* newDoc = dynamic_cast<TextFileDocument*>
    (newDocumentHelper(m_pDocTemplText));
  if (newDoc)
    newDoc->getView()->getFrame()->SetTitle(newDoc->GetDocumentName());

  return newDoc;
}


PhantomFileDocument*
CTSimApp::newPhantomDoc()
{
  PhantomFileDocument* newDoc = dynamic_cast<PhantomFileDocument*>
    (newDocumentHelper(m_pDocTemplPhantom));
  if (newDoc)
    newDoc->getView()->getFrame()->SetTitle(newDoc->GetDocumentName());

  return newDoc;
}


#if wxUSE_GLCANVAS
Graph3dFileDocument*
CTSimApp::newGraph3dDoc()
{
  Graph3dFileDocument* newDoc = dynamic_cast<Graph3dFileDocument*>
    (newDocumentHelper(m_pDocTemplGraph3d));
  if (newDoc)
    newDoc->getView()->getFrame()->SetTitle(newDoc->GetDocumentName());

  return newDoc;
}

#endif
