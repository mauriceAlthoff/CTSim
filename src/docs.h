/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          docs.h
**   Purpose:       Header file for Document routines of CTSim program
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

#ifndef __DOCSH__
#define __DOCSH__

#include "wx/docview.h"

// #include "views.h"
#include "imagefile.h"
#include "phantom.h"
#include "projections.h"
#include "plotfile.h"
#include "threadrecon.h"

class ProjectionFileView;
class PhantomFileView;
class ImageFileView;
class PlotFileView;
class TextFileView;
class Graph3dFileView;


class ImageFileDocument: public wxDocument
{
private:
    DECLARE_DYNAMIC_CLASS(ImageFileDocument)
    ImageFile* m_pImageFile;
    bool m_bBadFileOpen;

public:
    virtual bool OnSaveDocument (const wxString& filename);
    virtual bool OnOpenDocument (const wxString& filename);
    virtual bool IsModified () const;
    virtual bool Revert ();
    virtual void Modify (bool mod);

    ImageFileDocument ()
      : m_bBadFileOpen(false)
    {
      m_pImageFile = new ImageFile;
    }

    virtual ~ImageFileDocument ()
    {
      delete m_pImageFile;
    }

    const ImageFile& getImageFile() const { return *m_pImageFile; }
    ImageFile& getImageFile() { return *m_pImageFile; }
    void setImageFile (ImageFile* pImageFile)
    {
      delete m_pImageFile;
      m_pImageFile = pImageFile;
    }

    ImageFileView* getView() const;
    bool getBadFileOpen() const { return m_bBadFileOpen; }
    void setBadFileOpen() { m_bBadFileOpen = true; }
    void Activate();
};

class BackgroundProcessingDocument : public wxDocument
{
private:
    DECLARE_DYNAMIC_CLASS(BackgroundProcessingDocument)
#ifdef HAVE_WXTHREADS
    typedef BackgroundSupervisor BackgroundObject;
    typedef std::vector<BackgroundObject*> BackgroundContainer;
    BackgroundContainer m_vecpBackgroundSupervisors;
    wxCriticalSection m_criticalSection;
#endif

public:
  BackgroundProcessingDocument()
    : wxDocument()
      {}

  void cancelRunningTasks();
#ifdef HAVE_WXTHREADS
  void addBackgroundSupervisor (BackgroundSupervisor* pSupervisor);
  void removeBackgroundSupervisor (BackgroundSupervisor* pSupervisor);
#endif

  DECLARE_EVENT_TABLE()
};

class ProjectionFileDocument: public BackgroundProcessingDocument
{
private:
    DECLARE_DYNAMIC_CLASS(ProjectionFileDocument)
    Projections* m_pProjectionFile;
    bool m_bBadFileOpen;

public:
    virtual bool OnSaveDocument (const wxString& filename);
    virtual bool OnOpenDocument (const wxString& filename);
    virtual bool IsModified () const;
    virtual void Modify (bool mod);

    ProjectionFileDocument ()
          : m_bBadFileOpen(false)
    {
      m_pProjectionFile = new Projections;
    }

    virtual ~ProjectionFileDocument ();

    const Projections& getProjections () const  { return *m_pProjectionFile; }
    Projections& getProjections ()      { return *m_pProjectionFile; }

    void setProjections (Projections* pProjections)
    { delete m_pProjectionFile;
      m_pProjectionFile = pProjections;
    }

    ProjectionFileView* getView() const;
    bool getBadFileOpen() const { return m_bBadFileOpen; }
    void setBadFileOpen() { m_bBadFileOpen = true; }
    void Activate();
};


class PhantomFileDocument: public BackgroundProcessingDocument
{
private:
    DECLARE_DYNAMIC_CLASS(PhantomFileDocument)
    Phantom m_phantom;
    int m_idPhantom;
    wxString m_namePhantom;
    bool m_bBadFileOpen;

public:
    PhantomFileDocument ()
        : m_idPhantom(Phantom::PHM_INVALID), m_bBadFileOpen(false)
    {}

    virtual ~PhantomFileDocument ();

    const int getPhantomID () const { return m_idPhantom; }

    const wxString& getPhantomName () const { return m_namePhantom; }

    const Phantom& getPhantom () const  { return m_phantom; }

    Phantom& getPhantom ()      { return m_phantom; }

    virtual bool OnOpenDocument (const wxString& filename);
    virtual bool OnSaveDocument (const wxString& filename);
    virtual bool IsModified () const;
    virtual void Modify (bool mod);
    PhantomFileView* getView() const;
    bool getBadFileOpen() const { return m_bBadFileOpen; }
    void setBadFileOpen() { m_bBadFileOpen = true; }
    void Activate();
};


class PlotFileDocument: public wxDocument
{
private:
    DECLARE_DYNAMIC_CLASS(PlotFileDocument)
    PlotFile m_plot;
    wxString m_namePlot;
    bool m_bBadFileOpen;

public:
    PlotFileDocument ()
      : m_bBadFileOpen(false)
    {}

    virtual ~PlotFileDocument ()
        {}

    const wxString& getPlotName () const
        { return m_namePlot; }

    const PlotFile& getPlotFile () const
        { return m_plot; }

    PlotFile& getPlotFile ()
        { return m_plot; }

    virtual bool OnOpenDocument (const wxString& filename);
    virtual bool OnSaveDocument (const wxString& filename);
    virtual bool IsModified () const;
    virtual void Modify (bool mod);
    PlotFileView* getView() const;
    bool getBadFileOpen() const { return m_bBadFileOpen; }
    void setBadFileOpen() { m_bBadFileOpen = true; }
    void Activate();
};


class TextFileDocument: public wxDocument
{
 private:
  DECLARE_DYNAMIC_CLASS(TextFileDocument)
  bool m_bBadFileOpen;

 public:
  TextFileDocument(void)
        : m_bBadFileOpen(false)
  {}

  virtual ~TextFileDocument(void) {}

  virtual bool OnSaveDocument(const wxString& filename);
  virtual bool OnOpenDocument(const wxString& filename);
  virtual bool IsModified(void) const;

  wxTextCtrl* getTextCtrl();

  TextFileView* getView() const;
  bool getBadFileOpen() const { return m_bBadFileOpen; }
  void setBadFileOpen() { m_bBadFileOpen = true; }
};


#if wxUSE_GLCANVAS
#include <GL/gl.h>
#include <GL/glu.h>

typedef GLfloat glTripleFloat[3];

class Graph3dFileDocument: public wxDocument
{
  friend class Graph3dFileView;

 private:
  DECLARE_DYNAMIC_CLASS(Graph3dFileDocument)
  bool m_bBadFileOpen;
  GLint m_nVertices;
  glTripleFloat* m_pVertices;
  glTripleFloat* m_pNormals;
  unsigned int m_nx;
  unsigned int m_ny;
  ImageFileArray m_array;

 public:
  Graph3dFileDocument(void);
  virtual ~Graph3dFileDocument(void);

  virtual bool OnSaveDocument (const wxString& filename);
  virtual bool OnOpenDocument (const wxString& filename);
  virtual bool IsModified () const;

  Graph3dFileView* getView() const;
  bool getBadFileOpen() const { return m_bBadFileOpen; }
  void setBadFileOpen()       { m_bBadFileOpen = true; }
  bool createFromImageFile (const ImageFile& rImageFile);

  int nx() const  { return m_nx; }
  int ny() const { return m_ny; }
  ImageFileArray getArray() { return m_array; }
  ImageFileArrayConst getArray() const { return m_array; }
  void Activate();
};
#endif // wxUSE_GLCANVAS


#endif
