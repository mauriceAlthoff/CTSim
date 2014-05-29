/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          docs.cpp
**   Purpose:       Document routines for CTSim program
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
#include "wx/txtstrm.h"
#include "wx/file.h"
#include "wx/thread.h"

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "ct.h"
#include "ctsim.h"
#include "docs.h"
#include "views.h"
#include "threadrecon.h"


// ImageFileDocument

IMPLEMENT_DYNAMIC_CLASS(ImageFileDocument, wxDocument)

bool ImageFileDocument::OnSaveDocument(const wxString& filename)
{
  if (! m_pImageFile->fileWrite (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to write image file ") << filename << _T("\n");
    return false;
  }
  if (theApp->getVerboseLogging())
    *theApp->getLog() << _T("Wrote image file ") << filename << _T("\n");
  Modify(false);
  return true;
}

bool ImageFileDocument::OnOpenDocument(const wxString& filename)
{
  if (! OnSaveModified())
    return false;

  if (! m_pImageFile->fileRead (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to read image file ") << filename << _T("\n");
    m_bBadFileOpen = true;
    return false;
  }

  if (theApp->getVerboseLogging() && filename != _T(""))
    *theApp->getLog() << _T("Read image file ") << filename << _T("\n");

  SetFilename(filename, true);
  Modify(false);
  getView()->setInitialClientSize();
  this->SetFilename(filename, true);
  UpdateAllViews();
  m_bBadFileOpen = false;

  return true;
}

bool
ImageFileDocument::IsModified(void) const
{
  return wxDocument::IsModified();
}

void
ImageFileDocument::Modify(bool mod)
{
  wxDocument::Modify(mod);
}

ImageFileView*
ImageFileDocument::getView() const
{
  return dynamic_cast<ImageFileView*>(GetFirstView());
}

bool
ImageFileDocument::Revert ()
{
  if (IsModified()) {
    wxString msg (_T("Revert to saved "));
    msg += GetFilename();
    msg += _T("?");
    wxMessageDialog dialog (getView()->getFrame(), msg, _T("Are you sure?"), wxYES_NO | wxNO_DEFAULT);
    if (dialog.ShowModal() == wxID_YES) {
      if (theApp->getVerboseLogging())
        *theApp->getLog() << _T("Reverting to saved ") << GetFilename() << _T("\n");
      Modify (false);
      OnOpenDocument (GetFilename());
    }
  }
  UpdateAllViews();

  return true;
}

void
ImageFileDocument::Activate()
{
#if CTSIM_MDI
  getView()->getFrame()->Activate();
#endif
};

// BackgroundProcessingDocument - Base Class

IMPLEMENT_DYNAMIC_CLASS(BackgroundProcessingDocument, wxDocument)
BEGIN_EVENT_TABLE(BackgroundProcessingDocument, wxDocument)
END_EVENT_TABLE()

#ifdef HAVE_WXTHREADS
void
BackgroundProcessingDocument::addBackgroundSupervisor (BackgroundSupervisor* pSupervisor)
{
  wxCriticalSectionLocker locker (m_criticalSection);
  if (pSupervisor)
    m_vecpBackgroundSupervisors.push_back (pSupervisor);
}

void
BackgroundProcessingDocument::removeBackgroundSupervisor (BackgroundSupervisor* pSupervisor)
{
  m_criticalSection.Enter();
  bool bFound = false;
  for (BackgroundContainer::iterator i = m_vecpBackgroundSupervisors.begin();
        i != m_vecpBackgroundSupervisors.end();
        i++)
          if (*i == pSupervisor) {
            m_vecpBackgroundSupervisors.erase(i);
            bFound = true;
            break;
        }
  m_criticalSection.Leave();

  if (! bFound)
     sys_error (ERR_SEVERE, "Could not find background task [OnRemoveBackground]");
}
#endif

void
BackgroundProcessingDocument::cancelRunningTasks()
{
#ifdef HAVE_WXTHREADS
  m_criticalSection.Enter();
  for (BackgroundContainer::iterator i = m_vecpBackgroundSupervisors.begin();
        i != m_vecpBackgroundSupervisors.end(); i++)
          (*i)->onCancel();
  m_criticalSection.Leave();

  while (m_vecpBackgroundSupervisors.size() > 0) {
     ::wxYield();
     ::wxMilliSleep(50);
  }
#endif
}


// ProjectionFileDocument

IMPLEMENT_DYNAMIC_CLASS(ProjectionFileDocument, BackgroundProcessingDocument)

bool
ProjectionFileDocument::OnSaveDocument(const wxString& filename)
{
  if (! m_pProjectionFile->write (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to write projection file ") << filename << _T("\n");
    return false;
  }
  if (theApp->getVerboseLogging())
    *theApp->getLog() << _T("Wrote projection file ") << filename << _T("\n");
  Modify(false);
  return true;
}

ProjectionFileDocument::~ProjectionFileDocument()
{
  cancelRunningTasks();

  delete m_pProjectionFile;
}

bool
ProjectionFileDocument::OnOpenDocument(const wxString& filename)
{
  if (! OnSaveModified())
    return false;

  if (! m_pProjectionFile->read (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to read projection file ") << filename << _T("\n");
    m_bBadFileOpen = true;
    return false;
  }
  m_bBadFileOpen = false;

  if (theApp->getVerboseLogging() && filename != _T(""))
    *theApp->getLog() << _T("Read projection file ") << filename << _T("\n");

  SetFilename(filename, true);
  Modify(false);
  getView()->setInitialClientSize();
  UpdateAllViews();

  return true;
}

bool
ProjectionFileDocument::IsModified(void) const
{
  return wxDocument::IsModified();
}

void
ProjectionFileDocument::Modify(bool mod)
{
  wxDocument::Modify(mod);
}


ProjectionFileView*
ProjectionFileDocument::getView() const
{
  return dynamic_cast<ProjectionFileView*>(GetFirstView());
}

void
ProjectionFileDocument::Activate()
{
#if CTSIM_MDI
  getView()->getFrame()->Activate();
#endif
};

// PhantomFileDocument

IMPLEMENT_DYNAMIC_CLASS(PhantomFileDocument, BackgroundProcessingDocument)

PhantomFileDocument::~PhantomFileDocument()
{
  cancelRunningTasks();
}

bool
PhantomFileDocument::OnOpenDocument(const wxString& constFilename)
{
  if (! OnSaveModified())
    return false;

  wxString filename (constFilename);

  if (wxFile::Exists (filename)) {
    m_phantom.createFromFile (filename.mb_str(wxConvUTF8));
    if (theApp->getVerboseLogging())
      *theApp->getLog() << _T("Read phantom file ") << filename << _T("\n");
  } else {
    filename.Replace (_T(".phm"), _T(""));
    m_phantom.createFromPhantom (filename.mb_str(wxConvUTF8));
  }
  m_namePhantom = filename;
  SetFilename (filename, true);
  if (m_phantom.fail()) {
    *theApp->getLog() << _T("Failure creating phantom ") << filename << _T("\n");
    m_bBadFileOpen = true;
    return false;
  }
  m_idPhantom = m_phantom.id();
  Modify(false);
  UpdateAllViews();
  m_bBadFileOpen = false;

  return true;
}

bool
PhantomFileDocument::OnSaveDocument(const wxString& filename)
{
  if (! m_phantom.fileWrite (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to write phantom file ") << filename << _T("\n");
    return false;
  }
  if (theApp->getVerboseLogging())
    *theApp->getLog() << _T("Wrote phantom file ") << filename << _T("\n");
  Modify(false);
  return true;
}

bool
PhantomFileDocument::IsModified(void) const
{
  return false;
}

void
PhantomFileDocument::Modify(bool mod)
{
  wxDocument::Modify(mod);
}


PhantomFileView*
PhantomFileDocument::getView() const
{
  return dynamic_cast<PhantomFileView*>(GetFirstView());
}

void
PhantomFileDocument::Activate()
{
#if CTSIM_MDI
  getView()->getFrame()->Activate();
#endif
};

// PlotFileDocument

IMPLEMENT_DYNAMIC_CLASS(PlotFileDocument, wxDocument)

bool
PlotFileDocument::OnSaveDocument(const wxString& filename)
{
  m_namePlot = filename.c_str();
  if (! m_plot.fileWrite (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to write plot file ") << filename << _T("\n");
    return false;
  }
  if (theApp->getVerboseLogging())
    *theApp->getLog() << _T("Wrote plot file ") << filename << _T("\n");
  Modify(false);
  return true;
}

bool
PlotFileDocument::OnOpenDocument(const wxString& filename)
{
  if (! OnSaveModified())
    return false;

  if (! m_plot.fileRead (filename.mb_str(wxConvUTF8))) {
    *theApp->getLog() << _T("Unable to read plot file ") << filename << _T("\n");
    m_bBadFileOpen = true;
    return false;
  }
  m_bBadFileOpen = false;

  if (theApp->getVerboseLogging() && filename != _T(""))
    *theApp->getLog() << _T("Read plot file ") << filename << _T("\n");

  SetFilename (filename, true);
  m_namePlot = filename.c_str();
  Modify (false);
  getView()->setInitialClientSize();
  UpdateAllViews();

  return true;
}


bool
PlotFileDocument::IsModified(void) const
{
  return wxDocument::IsModified();
}

void
PlotFileDocument::Modify (bool mod)
{
  wxDocument::Modify(mod);
}

PlotFileView*
PlotFileDocument::getView() const
{
  return dynamic_cast<PlotFileView*>(GetFirstView());
}

void
PlotFileDocument::Activate()
{
#if CTSIM_MDI
  getView()->getFrame()->Activate();
#endif
};

//////////////////////////////////////////////////////////////////////////
//
// TextFileDocument
//
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(TextFileDocument, wxDocument)

bool
TextFileDocument::OnSaveDocument(const wxString& filename)
{
  TextFileView *view = getView();
  if (! view->getTextCtrl()->SaveFile(filename))
    return false;
  Modify(false);
  return true;
}

bool
TextFileDocument::OnOpenDocument(const wxString& filename)
{
  TextFileView *view = getView();

  if (! view->getTextCtrl()->LoadFile(filename)) {
    m_bBadFileOpen = true;
    return false;
  }

  SetFilename (filename, true);
  Modify (false);
  UpdateAllViews();
  m_bBadFileOpen = false;
  return true;
}

bool
TextFileDocument::IsModified(void) const
{
  return false;

  TextFileView *view = getView();

  if (view)
    return (wxDocument::IsModified() || view->getTextCtrl()->IsModified());
  else
    return wxDocument::IsModified();
}


TextFileView*
TextFileDocument::getView() const
{
  return dynamic_cast<TextFileView*>(GetFirstView());
}

wxTextCtrl*
TextFileDocument::getTextCtrl()
{
  return dynamic_cast<TextFileView*>(GetFirstView())->getTextCtrl();
}

//////////////////////////////////////////////////////////////////////////
//
// Graph3dFileDocument
//
//////////////////////////////////////////////////////////////////////////

#if wxUSE_GLCANVAS

IMPLEMENT_DYNAMIC_CLASS(Graph3dFileDocument, wxDocument)

Graph3dFileDocument::Graph3dFileDocument(void)
: m_bBadFileOpen(false), m_nVertices(0), m_pVertices(0), m_pNormals(0),m_nx(0),m_ny(0),m_array(0)
{
}

Graph3dFileDocument::~Graph3dFileDocument()
{
}

bool
Graph3dFileDocument::OnSaveDocument(const wxString& filename)
{
  Modify(false);
  return true;
}

bool
Graph3dFileDocument::OnOpenDocument(const wxString& filename)
{
  SetFilename (filename, true);
  Modify (false);
  getView()->setInitialClientSize();
  UpdateAllViews();
  m_bBadFileOpen = false;
  return true;
}

bool
Graph3dFileDocument::IsModified(void) const
{
    return wxDocument::IsModified();
}


Graph3dFileView*
Graph3dFileDocument::getView() const
{
  return dynamic_cast<Graph3dFileView*>(GetFirstView());
}

bool
Graph3dFileDocument::createFromImageFile (const ImageFile& rImageFile)
{
  m_nx = rImageFile.nx();
  m_ny = rImageFile.ny();
  m_array = rImageFile.getArray();

  return true;
}

void
Graph3dFileDocument::Activate()
{
#if CTSIM_MDI
  getView()->getFrame()->Activate();
#endif
};


#endif // wxUSE_GLCANVAS
