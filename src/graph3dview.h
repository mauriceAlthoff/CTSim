/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          graph3dview.h
**   Purpose:       Header file for 3d graph view
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

#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif

#include "wx/timer.h"
#include "wx/glcanvas.h"

#include <GL/gl.h>
#include <GL/glu.h>


class Graph3dFileCanvas;
class Graph3dFileView : public wxView
{
  friend class Graph3dFileCanvas;

private:
  DECLARE_DYNAMIC_CLASS(Graph3dFileView)
  DECLARE_EVENT_TABLE()

  wxMenu* m_pFileMenu;
  wxMenu *m_pViewMenu;
  wxStatusBar* m_pStatusBar;

  Graph3dFileCanvas* m_pCanvas;
  GLfloat m_dXRotate;
  GLfloat m_dYRotate;
  GLfloat m_dZRotate;
  bool m_bDoubleBuffer;
  bool m_bSmooth;
  bool m_bWireframe;
  bool m_bLighting;
  bool m_bColor;
  bool m_bColorScaleMinSet;
  bool m_bColorScaleMaxSet;
  enum {
    DISPLAYLIST_COLOR = 1,
    DISPLAYLIST_NO_COLOR = 2,
  };

  double m_dGraphMin;
  double m_dGraphMax;
  double m_dColorScaleMin;
  double m_dColorScaleMax;
  wxGLContext *m_pGLContext;

#if CTSIM_MDI
  wxDocMDIChildFrame* m_pFrame;
  wxDocMDIChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#else
  wxDocChildFrame* m_pFrame;
  wxDocChildFrame* CreateChildFrame(wxDocument *doc, wxView *view);
#endif

  void Draw();
  void DrawSurface();
  void CreateDisplayList();
  void InitMaterials();
  void InitGL();

  Graph3dFileCanvas *CreateCanvas (wxFrame* parent);

  wxWindow* getFrameForChild()
#if CTSIM_MDI
  { return theApp->getMainFrame(); }
#else
  { return m_pFrame; }
#endif

  void intensityToColor (double dIntensity, GLfloat* vecColor);

public:
  Graph3dFileView();
  virtual ~Graph3dFileView();
  void canvasClosed()
  { m_pCanvas = NULL; m_pFrame = NULL; }

  bool OnCreate(wxDocument *doc, long flags);
  void OnDraw(wxDC* dc);
  void OnUpdate(wxView *sender, wxObject *hint = NULL);
  bool OnClose (bool deleteWindow = true);
  void OnProperties (wxCommandEvent& event);
  void OnLighting (wxCommandEvent& event);
  void OnWireframe (wxCommandEvent& event);
  void OnColor (wxCommandEvent& event);
  void OnSmooth (wxCommandEvent& event);
  void OnScaleSet (wxCommandEvent& event);
  void OnScaleAuto (wxCommandEvent& event);
  void OnScaleFull (wxCommandEvent& event);

  void setInitialClientSize();

#if CTSIM_MDI
  wxDocMDIChildFrame* getFrame() { return m_pFrame; }
#else
  wxDocChildFrame* getFrame() { return m_pFrame; }
#endif
  Graph3dFileCanvas* getCanvas() { return m_pCanvas; }
  Graph3dFileDocument* GetDocument()
  { return dynamic_cast<Graph3dFileDocument*>(wxView::GetDocument()); }
};


class Graph3dFileCanvas: public wxGLCanvas
{
private:
  DECLARE_EVENT_TABLE()

  void Reshape (int width, int height);
  Graph3dFileView* m_pView;

public:
   Graph3dFileCanvas (Graph3dFileView* view, wxWindow *parent,
                      int* attribList,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize, long style = 0);
   virtual ~Graph3dFileCanvas();

   void OnPaint(wxPaintEvent& event);
   virtual wxSize GetBestSize() const;
   void OnSize(wxSizeEvent& event);
   void OnEraseBackground(wxEraseEvent& event);
   void OnChar(wxKeyEvent& event);
   void OnMouseEvent(wxMouseEvent& event);
   void setView (Graph3dFileView* pView)  { m_pView = pView; }
};
