/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          graph3dview.cpp
**   Purpose:       3d graph view classes
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxUSE_GLCANVAS

#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif

#include "wx/timer.h"
#include "wx/glcanvas.h"

#include <GL/gl.h>
#include <GL/glu.h>

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

#if defined(MSVC) || HAVE_SSTREAM
#include <sstream>
#else
#include <sstream_subst>
#endif

inline void
Graph3dFileView::intensityToColor (double dIntensity, GLfloat* vecColor)
{
  if (dIntensity < 0 || dIntensity > 1) {
    vecColor[0] = vecColor[1] = vecColor[2] = 1;
    return;
  }

  float fRange = dIntensity * 5;
  int iRange = static_cast<int>(floor (fRange));
  float fFrac = fRange - iRange;

  // Rainbow: Purple->Blue->Cyan->Green->Yellow->Red = (1,0,1)-(0,0,1)-(0,1,1)-(0,1,0)-(1,1,0)-(1,0,0)
  switch (iRange) {
  case 0:
    vecColor[0] = 1.0f - fFrac; vecColor[1] = 0.0f; vecColor[2] = 1.0f;
    break;
  case 1:
    vecColor[0] = 0.0f; vecColor[1] = fFrac; vecColor[2] = 1.0f;
    break;
  case 2:
    vecColor[0] = 0.0f; vecColor[1] = 1.0f; vecColor[2] = 1.0f - fFrac;
    break;
  case 3:
    vecColor[0] = fFrac; vecColor[1] = 1.0f; vecColor[2] = 0.0f;
    break;
  case 4:
    vecColor[0] = 1.0f; vecColor[1] = 1.0f - fFrac; vecColor[2] = 0.0f;
    break;
  case 5:
    vecColor[0] = 1.0f; vecColor[1] = 0.0f; vecColor[2] = 0.0f;
    break;
  }
}

//***********************************************************************
// Function: CalculateVectorNormal
//
// Purpose: Given three points of a 3D plane, this function calculates
//          the normal vector of that plane.
//
// Parameters:
//     fVert1[]   == array for 1st point (3 elements are x, y, and z).
//     fVert2[]   == array for 2nd point (3 elements are x, y, and z).
//     fVert3[]   == array for 3rd point (3 elements are x, y, and z).
//
// Returns:
//     fNormalX   == X vector for the normal vector
//     fNormalY   == Y vector for the normal vector
//     fNormalZ   == Z vector for the normal vector
//*************************************************************************

template<class T>
static void
CalculateVectorNormal (T fVert1[], T fVert2[], T fVert3[], T *fNormalX, T *fNormalY, T *fNormalZ)
{
  T Qx = fVert2[0] - fVert1[0];
  T Qy = fVert2[1] - fVert1[1];
  T Qz = fVert2[2] - fVert1[2];
  T Px = fVert3[0] - fVert1[0];
  T Py = fVert3[1] - fVert1[1];
  T Pz = fVert3[2] - fVert1[2];

  *fNormalX = Py*Qz - Pz*Qy;
  *fNormalY = Pz*Qx - Px*Qz;
  *fNormalZ = Px*Qy - Py*Qx;
}

IMPLEMENT_DYNAMIC_CLASS(Graph3dFileView, wxView)

BEGIN_EVENT_TABLE(Graph3dFileView, wxView)
EVT_MENU(IFMENU_FILE_PROPERTIES, Graph3dFileView::OnProperties)
EVT_MENU(GRAPH3D_VIEW_LIGHTING, Graph3dFileView::OnLighting)
EVT_MENU(GRAPH3D_VIEW_COLOR, Graph3dFileView::OnColor)
EVT_MENU(GRAPH3D_VIEW_SMOOTH, Graph3dFileView::OnSmooth)
EVT_MENU(GRAPH3D_VIEW_WIREFRAME, Graph3dFileView::OnWireframe)
EVT_MENU(GRAPH3D_VIEW_SCALE_MINMAX, Graph3dFileView::OnScaleSet)
EVT_MENU(GRAPH3D_VIEW_SCALE_AUTO, Graph3dFileView::OnScaleAuto)
EVT_MENU(GRAPH3D_VIEW_SCALE_FULL, Graph3dFileView::OnScaleFull)
END_EVENT_TABLE()

Graph3dFileView::Graph3dFileView ()
  : m_pFileMenu(NULL), m_pViewMenu(NULL), m_pStatusBar(NULL), m_pCanvas(NULL),
    m_dXRotate(-180), m_dYRotate(-210), m_dZRotate(195),
    m_bDoubleBuffer(true), m_bSmooth(true), m_bWireframe(false),
    m_bLighting(false), m_bColor(true),
    m_bColorScaleMinSet(false), m_bColorScaleMaxSet(false),
    m_pGLContext(NULL), m_pFrame(NULL)
{}


Graph3dFileView::~Graph3dFileView()
{
  GetDocumentManager()->FileHistoryRemoveMenu (m_pFileMenu);
  GetDocumentManager()->ActivateView(this, false);
  if (m_pGLContext)
    delete m_pGLContext;
}

bool
Graph3dFileView::OnCreate (wxDocument *doc, long WXUNUSED(flags) )
{
  m_pFrame = CreateChildFrame(doc, this);
  SetFrame (m_pFrame);

  m_pViewMenu->Check (GRAPH3D_VIEW_COLOR, m_bColor);
  m_pViewMenu->Check (GRAPH3D_VIEW_LIGHTING, m_bLighting);
  m_pViewMenu->Check (GRAPH3D_VIEW_SMOOTH, m_bSmooth);
  m_pViewMenu->Check (GRAPH3D_VIEW_WIREFRAME, m_bWireframe);

  m_pFrame->SetFocus();
  m_pFrame->Show(true);
  Activate(true);

#if 1
  m_pCanvas = CreateCanvas (m_pFrame);
  m_pGLContext = new wxGLContext (m_pCanvas);
  m_pFrame->SetClientSize (m_pCanvas->GetBestSize());
  m_pCanvas->SetClientSize (m_pCanvas->GetBestSize());
  m_pFrame->SetTitle(doc->GetFilename());

  m_pCanvas->SetCurrent(*m_pGLContext);
  InitGL(); // Crash
  m_pCanvas->SwapBuffers();
#endif

  return true;
}

Graph3dFileCanvas*
Graph3dFileView::CreateCanvas (wxFrame* parent)
{
  Graph3dFileCanvas* pCanvas =
    new Graph3dFileCanvas (this, parent, NULL,
                           wxDefaultPosition, wxDefaultSize,
                           wxFULL_REPAINT_ON_RESIZE);

  pCanvas->SetBackgroundColour(*wxWHITE);
  pCanvas->ClearBackground();

  return pCanvas;
}


void
Graph3dFileView::DrawSurface()
{
  if (! GetDocument())
    return;

  if (m_bSmooth) {
    glShadeModel (GL_SMOOTH);
  } else {
    glShadeModel (GL_FLAT);
  }

  if (m_bLighting) {
    glEnable (GL_LIGHTING);
  } else {
    glDisable (GL_LIGHTING);
  }

  unsigned int nx = GetDocument()->nx();
  unsigned int ny = GetDocument()->ny();

  glRotated (m_dZRotate, 0.0, 1.0, 0.0);
  glRotated (m_dXRotate, 0.0, 0.0, 1.0);
  glRotated (m_dYRotate, 1.0, 0.0, 0.0);
  glTranslated (-static_cast<double>(nx - 1) / 2, 0.0, -static_cast<double>(ny - 1) / 2);

  InitMaterials();
  if (m_bWireframe) {
    if (! m_bColor)
      glColor3f (1.0f, 1.0f, 1.0f);
    glPolygonOffset (1.0f, 1.0f);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glCallList (DISPLAYLIST_COLOR);

    glEnable (GL_DEPTH_TEST);
    glColor3f (1.0f, 1.0f, 1.0f);
    glPolygonOffset (0.0f, 0.0f);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glCallList (DISPLAYLIST_NO_COLOR);

  } else {
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    if (! m_bColor) {
      glColor3f (1.0f, 1.0f, 1.0f);
      glCallList (DISPLAYLIST_NO_COLOR);
    } else {
      glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
      glEnable (GL_COLOR_MATERIAL);
      glShadeModel (GL_FLAT);
      glCallList (DISPLAYLIST_COLOR);
    }
  }

}

void
Graph3dFileView::CreateDisplayList()
{
  if (! GetDocument())
    return;

  unsigned int nx = GetDocument()->nx();
  unsigned int ny = GetDocument()->ny();
  const ImageFileArrayConst v = GetDocument()->getArray();
  if (nx == 0 || ny == 0 || ! v)
    return;

  glNewList (DISPLAYLIST_COLOR, GL_COMPILE);

  double dMin = m_dColorScaleMin;
  double dIntensityScale = m_dColorScaleMax - m_dColorScaleMin;
  double actOffset = m_dGraphMin;
  double actScale = 0.4 * sqrt(nx*nx+ny*ny) / (m_dGraphMax - m_dGraphMin);
  double dXOffset = -(static_cast<double>(nx) - 1) / 2.;
  double dYOffset = -(static_cast<double>(ny) - 1) / 2.;
  dXOffset = 0;
  dYOffset = 0;

  double dXPos = -dXOffset;
  unsigned int ix;
  for (ix = 0; ix < nx - 1; ix++, dXPos++) {
    double dYPos = -dYOffset;
    glBegin(GL_QUAD_STRIP);
    double p1[3], p2[3], p3[3], n1[3];
    p1[0] = dXPos;  p1[1] = actScale * (v[ix][0] + actOffset); p1[2] = dYPos;
    p2[0] = dXPos+1; p2[1] = actScale * (v[ix+1][0] + actOffset); p2[2] = dYPos;
    p3[0] = dXPos; p3[1] = actScale * (v[ix][1] + actOffset); p3[2] = dYPos + 1;
    CalculateVectorNormal<double> (p1, p2, p3, &n1[0], &n1[1], &n1[2]);

    double dIntensity1 = 0., dIntensity2 = 0.;
    if (m_bColor) {
      dIntensity1 = (v[ix][0] - dMin) / dIntensityScale;
      dIntensity2 = (v[ix+1][0] - dMin) / dIntensityScale;
    }
    float vecColor[3];
    if (m_bColor) {
      intensityToColor (dIntensity1, vecColor);
      glColor3fv (vecColor);
    }
    glVertex3dv (p1); glNormal3dv (n1);
    if (m_bColor) {
      intensityToColor (dIntensity2, vecColor);
      glColor3fv (vecColor);
    }
    glVertex3dv (p2); glNormal3dv (n1);

    double lastP[3];
    lastP[0] = ix; lastP[1] = actScale * (v[ix][0] + actOffset); lastP[2] = 0;
    for (unsigned int iy = 1; iy < ny - 1; iy++, dYPos++) {
      p1[0] = dXPos; p1[1] = actScale * (v[ix][iy] + actOffset); p1[2] = dYPos;
      p2[0] = dXPos+1;  p2[1] = actScale * (v[ix+1][iy] + actOffset); p2[2] = dYPos;
      CalculateVectorNormal (p1, p2, lastP, &n1[0], &n1[1], &n1[2]);
      lastP[0] = p1[0]; lastP[1] = p1[1]; lastP[2] = p1[2];
      if (m_bColor) {
        dIntensity1 = (v[ix][iy] - dMin) / dIntensityScale;
        dIntensity2 = (v[ix+1][iy] - dMin) / dIntensityScale;
        intensityToColor (dIntensity1, vecColor);
        glColor3fv (vecColor);
      }
      glVertex3dv (p1); glNormal3dv (n1);
      if (m_bColor) {
        intensityToColor (dIntensity2, vecColor);
        glColor3fv (vecColor);
      }
      glVertex3dv (p2); glNormal3dv (n1);
    }
    glEnd(); // QUAD_STRIP
  }
  glEndList();


  glNewList (DISPLAYLIST_NO_COLOR, GL_COMPILE);
  dXPos = -dXOffset;
  for (ix = 0; ix < nx - 1; ix++, dXPos++) {
    double dYPos = -dYOffset;
    glBegin(GL_QUAD_STRIP);
    double p1[3], p2[3], p3[3], n1[3];
    p1[0] = dXPos;  p1[1] = actScale * (v[ix][0] + actOffset); p1[2] = dYPos;
    p2[0] = dXPos+1; p2[1] = actScale * (v[ix+1][0] + actOffset); p2[2] = dYPos;
    p3[0] = dXPos; p3[1] = actScale * (v[ix][1] + actOffset); p3[2] = dYPos + 1;
    CalculateVectorNormal<double> (p1, p2, p3, &n1[0], &n1[1], &n1[2]);

    glVertex3dv (p1);
    glNormal3dv (n1);
    glVertex3dv (p2);
    glNormal3dv (n1);
    double lastP[3];
    lastP[0] = ix; lastP[1] = actScale * (v[ix][0] + actOffset); lastP[2] = 0;
    for (unsigned int iy = 1; iy < ny - 1; iy++, dYPos++) {
      p1[0] = dXPos; p1[1] = actScale * (v[ix][iy] + actOffset); p1[2] = dYPos;
      p2[0] = dXPos+1;  p2[1] = actScale * (v[ix+1][iy] + actOffset); p2[2] = dYPos;
       CalculateVectorNormal (p1, p2, lastP, &n1[0], &n1[1], &n1[2]);
      lastP[0] = p1[0]; lastP[1] = p1[1]; lastP[2] = p1[2];
      glVertex3dv (p1); glNormal3dv (n1);
      glVertex3dv (p2); glNormal3dv (n1);
    }
    glEnd(); // QUAD_STRIP
  }
  glEndList();
}


void
Graph3dFileView::OnProperties (wxCommandEvent& event)
{
  wxString os;
  *theApp->getLog() << _T(">>>>\n") << os << _T("<<<<\n");
  wxMessageDialog dialogMsg (getFrameForChild(), os,
                             _T("Imagefile Properties"), 
                             wxOK | wxICON_INFORMATION);
  dialogMsg.ShowModal();
}

void
Graph3dFileView::OnLighting (wxCommandEvent& event)
{
  m_bLighting = ! m_bLighting;
  m_pViewMenu->Check (GRAPH3D_VIEW_LIGHTING, m_bLighting);
  m_pCanvas->Refresh();
}

void
Graph3dFileView::OnWireframe (wxCommandEvent& event)
{
  m_bWireframe = ! m_bWireframe;
  m_pViewMenu->Check (GRAPH3D_VIEW_WIREFRAME, m_bWireframe);
  m_pCanvas->Refresh();
}

void
Graph3dFileView::OnColor (wxCommandEvent& event)
{
  m_bColor = ! m_bColor;
  m_pViewMenu->Check (GRAPH3D_VIEW_COLOR, m_bColor);
  m_pCanvas->Refresh();
}

void
Graph3dFileView::OnSmooth (wxCommandEvent& event)
{
  m_bSmooth = ! m_bSmooth;
  m_pViewMenu->Check (GRAPH3D_VIEW_SMOOTH, m_bSmooth);
  m_pCanvas->Refresh();
}



void
Graph3dFileView::OnDraw (wxDC* dc)
{
  if (m_pCanvas) {
    m_pCanvas->SetCurrent(*m_pGLContext);
#ifdef DEBUG
    *theApp->getLog() << _T("Drawing 3d surface\n");
#endif
    Draw();
    m_pCanvas->SwapBuffers();
  }

  if (m_pStatusBar) {
    wxString os;
    os << _T("Xangle=") << m_dXRotate << _T(", Yangle=") << m_dYRotate
       << _T(", Zangle=") << m_dZRotate;
    m_pStatusBar->SetStatusText (os);
  }
}


void
Graph3dFileView::Draw ()
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPushMatrix();
  DrawSurface();
  glPopMatrix();
  glFlush();
}


void
Graph3dFileView::InitMaterials()
{
  if (! GetDocument())
    return;
  int nx = GetDocument()->nx();
  int ny = GetDocument()->ny();

#if 1
  static float position0[] = {nx/2, ny*2, -ny*2, 0.0f,};
  static float ambient0[] = {.1f, .1f, .1f, 1.0f};
  static float diffuse0[] = {1.0f, 1.0f, 1.0f, 1.0f};
  static float position1[] = {-nx/2, -ny*2, -ny*2, 0.0f,};
  static float ambient1[] = {.1f, .1f, .1f, .1f};
  static float diffuse1[] = {1.0f, 1.0f, 1.0f, 1.0f};
  //  static float position0[] = {0.0f, 0.0f, 20.0f, 0.0f};
  //  static float position1[] = {0.0f, 0.0f, -20.0f, 0.0f};
  static float front_mat_shininess[] = {60.0f};
  static float front_mat_specular[] = {0.2f, 0.2f, 0.2f, 1.0f};
  static float front_mat_diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};
  static float back_mat_shininess[] = {10.0f};
  static float back_mat_specular[] = {0.1f, 0.1f, 0.1f, 1.0f};
  static float back_mat_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  static float lmodel_ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
  static float lmodel_twoside[] = {GL_TRUE};

  //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH, GL_DONT_CARE);
  glEnable(GL_NORMALIZE);

  glLightfv (GL_LIGHT0, GL_AMBIENT, ambient0);
  glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuse0);
  glLightfv (GL_LIGHT0, GL_POSITION, position0);
  glEnable (GL_LIGHT0);

  glLightfv (GL_LIGHT1, GL_AMBIENT, ambient1);
  glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuse1);
  glLightfv (GL_LIGHT1, GL_POSITION, position1);
  glEnable (GL_LIGHT1);

  glLightModelfv (GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glLightModelfv (GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

  glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, front_mat_shininess);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, front_mat_specular);
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, front_mat_diffuse);
  glMaterialfv (GL_BACK, GL_SHININESS, back_mat_shininess);
  glMaterialfv (GL_BACK, GL_SPECULAR, back_mat_specular);
  glMaterialfv (GL_BACK, GL_DIFFUSE, back_mat_diffuse);

  glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
  glColorMaterial (GL_FRONT_AND_BACK, GL_SPECULAR);
  glEnable(GL_COLOR_MATERIAL);
#else
  GLfloat impLPos[]  = {1.0f, 1.0f, 1.0f, 0.0f};

  GLfloat defaultLightAmb   [] = {.2f, .2f, .2f, 1.0f};
  GLfloat defaultLightDiff  [] = {.2f, .2f, .2f, 1.0f};
  GLfloat defaultLightSpec  [] = { .3f, .3f, .3f, 1.0f};

  GLfloat defaultGlobalAmb [] = {.3f, .3f, .3f, 1.0f};
  GLfloat defaultGlobalDiff[] = {.3f, .3f, .3f, 1.0f};

  GLfloat defaultMatShine[] = {  30.0f };
  GLfloat defaultMatSpec[]  = { .4f, .4f, .4f, 1.0f};
  GLfloat defaultMatAmb[]   = { .3f, .3f, .3f, 1.0f};
  GLfloat defaultMatDiff[]  = { .5f, .5f, .5f, 1.0f};

  GLfloat brassMatAmb[]   = { .33f, .22f, .03f, 1.0f};
  GLfloat brassMatDiff[]  = { .78f, .57f, .11f, 1.0f};
  GLfloat brassMatSpec[]  = { .99f, .91f, .81f, 1.0f};
  GLfloat brassMatShine[] = {  27.8f };

  GLfloat emeraldMatAmb[]   = { .02f1, .1745f , .021f, 1.0f };
  GLfloat emeraldMatDiff[]  = { .075f, .6142f , .075f, 1.0f };
  GLfloat emeraldMatSpec[]  = { .633f, .7278f , .633f, 1.0f };
  GLfloat emeraldMatShine[] = {  76.8f };

  GLfloat slateMatAmb[]   = { .02f, .02f , .02f, 1.0f };
  GLfloat slateMatDiff[]  = { .02f, .01f , .01f, 1.0f };
  GLfloat slateMatSpec[]  = { .4f,  .4f ,  .4f , 1.0f };
  GLfloat slateMatShine[] = { .768f };

  //       double opnX = nx, opnY = ny, opnZ = z;
  //       eyeX = 1; eyeY = 1, eyeZ = 1;

  impLPos[0] = nx/2.; impLPos[1]= ny/2.; impLPos[2] = 0.;
  //opnListNum = 1;
  //impGraphicsFlag = IMP__3D;

  //       glutInitDisplayMode (GLUT_DOUBLE| GLUT_RGB | GLUT_DEPTH | GLUT_ACCUM);
  //       glutInitWindowSize (IMP_WIN_X, IMP_WIN_Y);
  //  glutInitWindowPosition (100, 100);
  //       glutCreateWindow ("- imp3D graphics -" );

  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

  glShadeModel (GL_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH, GL_DONT_CARE);
  glEnable(GL_NORMALIZE);


  glEnable(GL_DEPTH_TEST);

  glLightfv(GL_LIGHT0, GL_AMBIENT, defaultLightAmb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, defaultLightDiff);
  glLightfv(GL_LIGHT0, GL_SPECULAR,defaultLightSpec);

  glLightfv(GL_LIGHT1, GL_AMBIENT, defaultLightAmb);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, defaultLightDiff);
  glLightfv(GL_LIGHT1, GL_SPECULAR,defaultLightSpec);

  glLightfv(GL_LIGHT2, GL_AMBIENT , defaultLightAmb);
  glLightfv(GL_LIGHT2, GL_DIFFUSE , defaultLightDiff);
  glLightfv(GL_LIGHT2, GL_SPECULAR, defaultLightSpec);

  glLightfv(GL_LIGHT0, GL_POSITION,impLPos);
  glLightfv(GL_LIGHT1, GL_POSITION,impLPos);
  glLightfv(GL_LIGHT2, GL_POSITION,impLPos);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT  , defaultMatAmb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE  , defaultMatDiff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , defaultMatSpec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, defaultMatShine);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, defaultGlobalAmb);

  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

  glEnable(GL_COLOR_MATERIAL);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
  glEnable(GL_LIGHT0);
#endif

}


void
Graph3dFileView::InitGL ()
{
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

  glDisable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);

}

void
Graph3dFileView::OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
  if (! GetDocument())
    return;

  unsigned int nx = GetDocument()->nx();
  unsigned int ny = GetDocument()->ny();
  const ImageFileArrayConst v = GetDocument()->getArray();
  if (v != NULL && nx != 0 && ny != 0) {
    double min = v[0][0];
    double max = min;
    for (unsigned int ix = 0; ix < nx; ix++)
      for (unsigned int iy = 0; iy < ny; iy++) {
        double dVal = v[ix][iy];
        if (min > dVal)
          min = dVal;
        else if (max < dVal)
          max = dVal;
      }
      m_dGraphMin = min;
      m_dGraphMax = max;
      if (! m_bColorScaleMinSet)
        m_dColorScaleMin = min;
      if (! m_bColorScaleMaxSet)
        m_dColorScaleMax = max;
  }
  double dRadius = maxValue<int> (nx, ny) * SQRT2 / 2;

  if (m_pCanvas)
    m_pCanvas->SetCurrent(*m_pGLContext);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho (-dRadius, dRadius, -dRadius, dRadius, dRadius*5, -dRadius*5);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
#if 0
  GLfloat eyep[3], lookp[3], up[3];
  eyep[0] = -nx/2; eyep[1] = 0; eyep[2] = -ny/2;
  lookp[0] = 0; lookp[1] = 0, lookp[2] = 0;
  up[0] = 0; up[1] = 1; up[2] = 0;
  gluLookAt (eyep[0], eyep[1], eyep[2], lookp[0], lookp[1], lookp[2], up[0], up[1], up[2]);
#endif

  CreateDisplayList();

  if (m_pCanvas) {
        m_pCanvas->SwapBuffers();
    m_pCanvas->Refresh();
  }
}

bool
Graph3dFileView::OnClose (bool deleteWindow)
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
Graph3dFileView::setInitialClientSize ()
{
  if (m_pFrame && m_pCanvas) {
    wxSize bestSize = m_pCanvas->GetBestSize();

    m_pFrame->SetClientSize (bestSize);
    m_pFrame->Show (true);
    m_pFrame->SetFocus();
  }
}

void
Graph3dFileView::OnScaleAuto (wxCommandEvent& event)
{
#if 0
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
      OnUpdate (this, NULL);
    }
  }
#endif
}

void
Graph3dFileView::OnScaleSet (wxCommandEvent& event)
{
  if (! GetDocument())
    return;

  unsigned int nx = GetDocument()->nx();
  unsigned int ny = GetDocument()->ny();
  const ImageFileArrayConst v = GetDocument()->getArray();
  double dMin = 0., dMax = 0.;
  if (! m_bColorScaleMinSet && ! m_bColorScaleMaxSet) {
    dMax = dMin = v[0][0];
    for (unsigned ix = 0; ix < nx; ix++)
      for (unsigned int iy = 0; iy < ny; iy++)
        if (v[ix][iy] < dMin)
          dMin = v[ix][iy];
        else if (v[ix][iy] > dMax)
          dMax = v[ix][iy];
  }
  if (m_bColorScaleMinSet)
    dMin = m_dColorScaleMin;
  if (m_bColorScaleMaxSet)
    dMax = m_dColorScaleMax;

  DialogGetMinMax dialogMinMax (getFrameForChild(), _T("Set Color Scale Minimum & Maximum"), dMin, dMax);
  int retVal = dialogMinMax.ShowModal();
  if (retVal == wxID_OK) {
    m_bColorScaleMinSet = true;
    m_bColorScaleMaxSet = true;
    m_dColorScaleMin = dialogMinMax.getMinimum();
    m_dColorScaleMax = dialogMinMax.getMaximum();
    OnUpdate (this, NULL);
  }
}

void
Graph3dFileView::OnScaleFull (wxCommandEvent& event)
{
  if (m_bColorScaleMinSet || m_bColorScaleMaxSet) {
    m_bColorScaleMinSet = false;
    m_bColorScaleMaxSet = false;
    OnUpdate (this, NULL);
  }
}

#if CTSIM_MDI
wxDocMDIChildFrame*
#else
wxDocChildFrame*
#endif
Graph3dFileView::CreateChildFrame (wxDocument *doc, wxView *view)
{
#if CTSIM_MDI
  wxDocMDIChildFrame* subframe = new wxDocMDIChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Graph3dFile Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#else
  wxDocChildFrame* subframe = new wxDocChildFrame (doc, view, theApp->getMainFrame(), -1, _T("Graph3dFile Frame"), wxPoint(-1,-1), wxSize(-1,-1), wxDEFAULT_FRAME_STYLE);
#endif
  theApp->setIconForFrame (subframe);

// status bar text not showing tested with enlightenment. disabling for now...
#if 0
  m_pStatusBar = new wxStatusBar (subframe, -1);
  subframe->SetStatusBar (m_pStatusBar);
  m_pStatusBar->Show(true);
#endif

  m_pFileMenu = new wxMenu;

  m_pFileMenu->Append(MAINMENU_FILE_CREATE_PHANTOM, _T("Cr&eate Phantom...\tCtrl-P"));
  m_pFileMenu->Append(MAINMENU_FILE_CREATE_FILTER, _T("Create &Filter...\tCtrl-F"));
  m_pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
  m_pFileMenu->Append(wxID_CLOSE, _T("&Close\tCtrl-W"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(IFMENU_FILE_PROPERTIES, _T("P&roperties"));

  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_PRINT, _T("&Print..."));
  m_pFileMenu->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
  m_pFileMenu->Append(wxID_PREVIEW, _T("Print Preview"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append (MAINMENU_FILE_PREFERENCES, _T("Prefere&nces..."));
  m_pFileMenu->Append(MAINMENU_FILE_EXIT, _T("E&xit"));

  GetDocumentManager()->FileHistoryAddFilesToMenu(m_pFileMenu);
  GetDocumentManager()->FileHistoryUseMenu(m_pFileMenu);

  m_pViewMenu = new wxMenu;
  m_pViewMenu->Append(GRAPH3D_VIEW_WIREFRAME, _T("Wi&reframe\tCtrl-R"), _T(""), true);
  m_pViewMenu->Append(GRAPH3D_VIEW_SMOOTH, _T("S&mooth\tCtrl-M"), _T(""), true);
  m_pViewMenu->Append(GRAPH3D_VIEW_COLOR, _T("Co&lor\tCtrl-L"), _T(""), true);
  m_pViewMenu->Append(GRAPH3D_VIEW_LIGHTING, _T("Li&ghting\tCtrl-G"), _T(""), true);
  m_pViewMenu->AppendSeparator();
  m_pViewMenu->Append(GRAPH3D_VIEW_SCALE_MINMAX, _T("Color Scale S&et Min/Max...\tCtrl-E"));
  m_pViewMenu->Append(GRAPH3D_VIEW_SCALE_AUTO, _T("Color Scale &Auto...\tCtrl-A"));
  m_pViewMenu->Append(GRAPH3D_VIEW_SCALE_FULL, _T("Color F&ull Scale\tCtrl-U"));


  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MAINMENU_HELP_CONTENTS, _T("&Contents\tF1"));
  help_menu->Append(MAINMENU_HELP_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append(m_pFileMenu, _T("&File"));
  menu_bar->Append(m_pViewMenu, _T("&View"));
  menu_bar->Append(help_menu, _T("&Help"));

  subframe->SetMenuBar(menu_bar);

  subframe->Centre(wxBOTH);

  wxAcceleratorEntry accelEntries[7];
  accelEntries[0].Set (wxACCEL_CTRL, static_cast<int>('R'), GRAPH3D_VIEW_WIREFRAME);
  accelEntries[1].Set (wxACCEL_CTRL, static_cast<int>('L'), GRAPH3D_VIEW_COLOR);
  accelEntries[2].Set (wxACCEL_CTRL, static_cast<int>('G'), GRAPH3D_VIEW_LIGHTING);
  accelEntries[3].Set (wxACCEL_CTRL, static_cast<int>('M'), GRAPH3D_VIEW_SMOOTH);
  accelEntries[4].Set (wxACCEL_CTRL, static_cast<int>('E'), GRAPH3D_VIEW_SCALE_MINMAX);
  accelEntries[5].Set (wxACCEL_CTRL, static_cast<int>('A'), GRAPH3D_VIEW_SCALE_AUTO);
  accelEntries[6].Set (wxACCEL_CTRL, static_cast<int>('U'), GRAPH3D_VIEW_SCALE_FULL);
  wxAcceleratorTable accelTable (7, accelEntries);
  subframe->SetAcceleratorTable (accelTable);

  return subframe;
}




BEGIN_EVENT_TABLE(Graph3dFileCanvas, wxGLCanvas)
EVT_PAINT(Graph3dFileCanvas::OnPaint)
EVT_SIZE(Graph3dFileCanvas::OnSize)
EVT_CHAR(Graph3dFileCanvas::OnChar)
EVT_MOUSE_EVENTS(Graph3dFileCanvas::OnMouseEvent)
EVT_ERASE_BACKGROUND(Graph3dFileCanvas::OnEraseBackground)
END_EVENT_TABLE()


Graph3dFileCanvas::Graph3dFileCanvas (Graph3dFileView* view, wxWindow *parent,
                                      int* attribList,
                                      const wxPoint& pos,
                                      const wxSize& size, long style)
: wxGLCanvas (parent, wxID_ANY, attribList, pos, size, style), m_pView(view)
{
}


Graph3dFileCanvas::~Graph3dFileCanvas()
{
}

void
Graph3dFileCanvas::OnPaint (wxPaintEvent& event)
{
  wxPaintDC dc(this);
  if (m_pView)
    m_pView->OnDraw(& dc);
}


wxSize
Graph3dFileCanvas::GetBestSize() const
{
  return wxSize (400,400);
}

void
Graph3dFileCanvas::OnSize (wxSizeEvent& event)
{
#ifndef __WXMOTIF__
  // if (!GetContext()) return;
#endif

  int width, height;
  GetClientSize (&width, &height);
  Reshape (width, height); // Crash
}

void
Graph3dFileCanvas::OnChar(wxKeyEvent& event)
{
  if (! m_pView)
    return;

  wxCommandEvent dummyEvent;
  switch (event.GetKeyCode()) {
  case WXK_LEFT:
        m_pView->m_dZRotate += 15.0;
    Refresh (false);
    break;
  case WXK_RIGHT:
    m_pView->m_dZRotate -= 15.0;
    Refresh (false);
    break;
  case WXK_UP:
    m_pView->m_dXRotate += 15.0;
    Refresh (false);
    break;
  case WXK_DOWN:
    m_pView->m_dXRotate -= 15.0;
    Refresh (false);
    break;
  case 'y': case 'Y':
    m_pView->m_dYRotate += 15.0;
    Refresh (false);
    break;
  case 't': case 'T':
    m_pView->m_dYRotate -= 15.0;
    Refresh (false);
    break;
  case 'w': case 'W':
    m_pView->OnWireframe (dummyEvent);
    break;
  case 's': case 'S':
    m_pView->OnSmooth (dummyEvent);
    break;
  case 'l': case 'L':
    m_pView->OnLighting (dummyEvent);
    break;
  case 'c': case 'C':
    m_pView->OnColor (dummyEvent);
    break;
  default:
    event.Skip();
    return;
  }
}

void
Graph3dFileCanvas::Reshape (int width, int height)
{
  SetCurrent(*m_pView->m_pGLContext);
  glViewport (0, 0, (GLint)width, (GLint)height);
  SwapBuffers();
}


void
Graph3dFileCanvas::OnMouseEvent(wxMouseEvent& event)
{
  static int dragging = 0;
  static float last_x, last_y;

  if (! m_pView)
    return;

  if(event.LeftIsDown()) {
    if(! dragging) {
      dragging = 1;
    } else {
      m_pView->m_dXRotate -= (event.GetY() - last_y)*1.0;
      m_pView->m_dZRotate += (event.GetX() - last_x)*1.0;
      Refresh (false);
    }
    last_x = event.GetX();
    last_y = event.GetY();
  } else
    dragging = 0;
}

void
Graph3dFileCanvas::OnEraseBackground(wxEraseEvent& event)
{
  // Do nothing: avoid flashing.
}


#endif // wxUSE_GLCANVAS
