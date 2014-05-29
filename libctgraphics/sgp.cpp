/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:       sgp.cpp             Simple Graphics Package
**      Programmer: Kevin Rosenberg
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

#include <stdio.h>
#include <math.h>
#include "ctsupport.h"
#include "sgp.h"


SGP_RGBColor SGP::s_aRGBColor[] =
{
  SGP_RGBColor (0, 0, 0),
  SGP_RGBColor (0, 0, 128),
  SGP_RGBColor (0, 128, 0),
  SGP_RGBColor (0, 128, 128),
  SGP_RGBColor (128, 0, 0),
  SGP_RGBColor (128, 0, 128),
  SGP_RGBColor (128, 128, 0),
  SGP_RGBColor (80, 80, 80),
  SGP_RGBColor (160, 160, 160),
  SGP_RGBColor (0, 0, 255),
  SGP_RGBColor (0, 255, 0),
  SGP_RGBColor (0, 255, 255),
  SGP_RGBColor (255, 0, 0),
  SGP_RGBColor (255, 0, 255),
  SGP_RGBColor (255, 255, 0),
  SGP_RGBColor (255, 255, 255),
};

int SGP::s_iRGBColorCount = sizeof(s_aRGBColor) / sizeof(class SGP_RGBColor);

#ifdef HAVE_WXWINDOWS
SGPDriver::SGPDriver (wxDC* pDC, int xsize, int ysize)
  : m_iPhysicalXSize(xsize), m_iPhysicalYSize(ysize), m_idDriver(0), m_pDC(pDC)
{
  m_idDriver |= SGPDRIVER_WXWINDOWS;
}
#endif

SGPDriver::SGPDriver (const char* szWinTitle, int xsize, int ysize)
  : m_iPhysicalXSize(xsize), m_iPhysicalYSize(ysize), m_sWindowTitle(szWinTitle), m_idDriver(0)
{
#ifdef HAVE_G2_H
  m_idG2 = g2_open_X11X (m_iPhysicalXSize, m_iPhysicalYSize, 10, 10, const_cast<char*>(szWinTitle), const_cast<char*>(szWinTitle), NULL, -1, -1);
  m_idDriver |= SGPDRIVER_G2;
#endif
}

SGPDriver::~SGPDriver ()
{
#if HAVE_G2_H
  if (isG2())
    g2_close (m_idG2);
#endif
}


// NAME
//   SGP::SGP        Constructor for Simple Graphics Package

SGP::SGP (const SGPDriver& driver)
  : m_driver (driver)
{
  m_iPhysicalXSize = m_driver.getPhysicalXSize();
  m_iPhysicalYSize = m_driver.getPhysicalYSize();

  wc_to_ndc.setIdentity ();
  mc_to_ndc.setIdentity();
  ndc_to_mc.setIdentity();
  m_ctm.setIdentity();

#ifdef HAVE_WXWINDOWS
  initFromDC (driver.idWX());
#endif

  setWindow (0., 0., 1., 1.);
  setViewport (0., 0., 1., 1.);
  moveAbs (0., 0.);
  stylusNDC (0., 0., false);

  setTextAngle (0.);
  setTextPointSize (8);
  setColor (C_BLACK);
  setLineStyle (LS_SOLID);
  setMarker (MARKER_POINT);
}


#ifdef HAVE_WXWINDOWS
void
SGP::initFromDC (wxDC* pDC)
{
  m_pen.SetWidth (1);

  if (m_driver.isWX()) {
    static const double dScreenDPI = 82;
    static const double dPointsPerInch = 72.;
    m_dPointsPerPixel = dPointsPerInch / dScreenDPI;
    const int iTestPointSize = 12;
    m_pFont = new wxFont (iTestPointSize, wxFONTFAMILY_ROMAN, 
                          wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#ifdef MSVC
    m_pFont->SetFaceName(wxString("times new roman"));
#endif
    m_driver.idWX()->SetFont (*m_pFont);
    double dTestCharHeight = m_driver.idWX()->GetCharHeight();
    m_dPointsPerPixel = iTestPointSize / dTestCharHeight;
        m_driver.idWX()->SetBackground (*wxWHITE_BRUSH);
  }
}
#endif


SGP::~SGP()
{
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
 //   m_driver.idWX()->SetFont (wxNullFont);
    delete m_pFont;
  }
#endif
}

void
SGP::stylusNDC (double x, double y, bool beam)
{
  int xp = static_cast<int>(x * (m_iPhysicalXSize - 1) + 0.5);
  int yp = static_cast<int>(y * (m_iPhysicalYSize - 1) + 0.5);
  if (m_driver.isWX())
    yp = m_iPhysicalYSize - yp;

  if (beam) {
#if HAVE_WXWINDOWS
    if (m_driver.isWX())
      m_driver.idWX()->DrawLine (m_iCurrentPhysicalX, m_iCurrentPhysicalY, xp, yp);
#endif
#if HAVE_G2_H
    if (m_driver.isG2())
      g2_line (m_driver.idG2(), m_iCurrentPhysicalX, m_iCurrentPhysicalY, xp, yp);
#endif
  }
  m_iCurrentPhysicalX = xp;
  m_iCurrentPhysicalY = yp;
}

void
SGP::markerNDC (double x, double y)
{
  int xp = static_cast<int>(x * (m_iPhysicalXSize - 1) + 0.5);
  int yp = static_cast<int>(y * (m_iPhysicalYSize - 1) + 0.5);
  if (m_driver.isWX())
    yp = m_iPhysicalYSize - yp;

#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
      m_driver.idWX()->DrawPoint (xp, yp);
      m_driver.idWX()->DrawPoint (xp-1, yp-1);
      m_driver.idWX()->DrawPoint (xp+1, yp+1);
      m_driver.idWX()->DrawPoint (xp+1, yp-1);
      m_driver.idWX()->DrawPoint (xp-1, yp+1);
  }
#endif
  m_iCurrentPhysicalX = xp;
  m_iCurrentPhysicalY = yp;
}

void
SGP::pointNDC (double x, double y)
{
  int xp = static_cast<int>(x * (m_iPhysicalXSize - 1) + 0.5);
  int yp = static_cast<int>(y * (m_iPhysicalYSize - 1) + 0.5);
  if (m_driver.isWX())
    yp = m_iPhysicalYSize - yp;

#if HAVE_WXWINDOWS
    if (m_driver.isWX())
      m_driver.idWX()->DrawPoint (xp, yp);
#endif
  m_iCurrentPhysicalX = xp;
  m_iCurrentPhysicalY = yp;
}


// NAME
//    clear     Clear Window

void
SGP::eraseWindow ()
{
#if HAVE_G2_H
  if (m_driver.isG2())
    g2_clear (m_driver.idG2());
#endif
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
        wxBrush brushWhite;
        brushWhite.SetColour(255,255,255);
        m_driver.idWX()->SetBackground(brushWhite);
        m_driver.idWX()->Clear();
        m_driver.idWX()->SetBackground(wxNullBrush);
#if 1
        wxPen pen;
        pen.SetColour(255,255,255);
        m_driver.idWX()->SetBrush (brushWhite);
        m_driver.idWX()->DrawRectangle (0, 0, m_iPhysicalXSize, m_iPhysicalYSize);
        m_driver.idWX()->SetBrush (wxNullBrush);
#endif
  }
#endif
}

// NAME
//      sgp2_window             Set window in world coordinates


void
SGP::setWindow (double xmin, double ymin, double xmax, double ymax)
{
  if (xmin >= xmax || ymin >= ymax) {
    sys_error (ERR_WARNING, "Minimum > Maximum [sgp2_window]");
    return;
  }

  xw_min = xmin;
  yw_min = ymin;
  xw_max = xmax;
  yw_max = ymax;
  m_bRecalcTransform = true;
}


// NAME
//      sgp2_viewport                   Set viewport in NDC

void
SGP::setViewport (double xmin, double ymin, double xmax, double ymax)
{
  if (xmin >= xmax || ymin >= ymax) {
    sys_error (ERR_WARNING, "Minimum > Maximum [sgp2_viewport]");
    return;
  }

  xv_min = xmin;
  yv_min = ymin;
  xv_max = xmax;
  yv_max = ymax;
  m_bRecalcTransform = true;

  viewNDC[0] = xmin;                    // Array for clip_rect()
  viewNDC[1] = ymin;
  viewNDC[2] = xmax;
  viewNDC[3] = ymax;
}

void
SGP::getViewport (double& xmin, double& ymin, double& xmax, double& ymax)
{
    xmin = xv_min;
    ymin = yv_min;
    xmax = xv_max;
    ymax = yv_max;
}

void
SGP::getWindow (double& xmin, double& ymin, double& xmax, double& ymax)
{
    xmin = xw_min;
    ymin = yw_min;
    xmax = xw_max;
    ymax = yw_max;
}


// NAME
//      frameViewport           draw box around viewport

void
SGP::frameViewport (void)
{
  stylusNDC (xv_min, yv_min, false);
  stylusNDC (xv_max, yv_min, true);
  stylusNDC (xv_max, yv_max, true);
  stylusNDC (xv_min, yv_max, true);
  stylusNDC (xv_min, yv_min, true);
}

void
SGP::setTextColor (int iFGcolor, int iBGcolor)
{
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    if (iFGcolor >= 0) {
      wxColor colour (s_aRGBColor[iFGcolor].getRed(), s_aRGBColor[iFGcolor].getGreen(), s_aRGBColor[iFGcolor].getBlue());
      m_driver.idWX()->SetTextForeground (colour);
    }
    if (iBGcolor >= 0) {
      wxColor colour (s_aRGBColor[iBGcolor].getRed(), s_aRGBColor[iBGcolor].getGreen(), s_aRGBColor[iBGcolor].getBlue());
      m_driver.idWX()->SetTextBackground (colour);
    }
  }
#endif
}

void
SGP::setColor (int icol)
{
  if (icol >= 0 && icol < s_iRGBColorCount) {
#if HAVE_G2_H
    if (m_driver.isG2()) {
      int iInk = g2_ink (m_driver.idG2(), s_aRGBColor[icol].getRed() / 255., s_aRGBColor[icol].getGreen() / 255., s_aRGBColor[icol].getBlue() / 255.);
      g2_pen (m_driver.idG2(), iInk);
    }
#endif
#if HAVE_WXWINDOWS
    if (m_driver.isWX()) {
      wxColor colour (s_aRGBColor[icol].getRed(), s_aRGBColor[icol].getGreen(), s_aRGBColor[icol].getBlue());
      m_pen.SetColour (colour);
      m_driver.idWX()->SetPen (m_pen);
    }
#endif
  }
}

void
SGP::setPenWidth (int iWidth)
{
  if (iWidth >= 0) {
#if HAVE_WXWINDOWS
    if (m_driver.isWX()) {
      m_pen.SetWidth (iWidth);
      m_driver.idWX()->SetPen (m_pen);
    }
#endif
  }
}

void
SGP::setRasterOp (int ro)
{
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    int wxFxn = -1;
    switch (ro) {
    case RO_AND:
      wxFxn = wxAND;
      break;
    case RO_AND_INVERT:
      wxFxn = wxAND_INVERT;
      break;
    case RO_AND_REVERSE:
      wxFxn = wxAND_REVERSE;
      break;
    case RO_CLEAR:
      wxFxn = wxCLEAR;
      break;
    case RO_COPY:
      wxFxn = wxCOPY;
      break;
    case RO_EQUIV:
      wxFxn = wxEQUIV;
      break;
    case RO_INVERT:
      wxFxn = wxINVERT;
      break;
    case RO_NAND:
      wxFxn = wxNAND;
      break;
    case RO_NOR:
      wxFxn = wxNOR;
      break;
    case RO_NO_OP:
      wxFxn = wxNO_OP;
      break;
    case RO_OR:
      wxFxn = wxOR;
      break;
    case RO_OR_INVERT:
      wxFxn = wxOR_INVERT;
      break;
    case RO_OR_REVERSE:
      wxFxn = wxOR_REVERSE;
      break;
    case RO_SET:
      wxFxn = wxSET;
      break;
    case RO_SRC_INVERT:
      wxFxn = wxSRC_INVERT;
      break;
    case RO_XOR:
      wxFxn = wxXOR;
      break;
    }
    if (wxFxn >= 0)
      m_driver.idWX()->SetLogicalFunction (wxFxn);
  }
#endif
}


void
SGP::setMarker (int idMarker)
{
  m_iMarker = idMarker;
}

//==============================================================
// set line style.  Pass 16 bit repeating pattern
//==============================================================
void
SGP::setLineStyle (int style)
{
  m_iLinestyle = style;

#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    switch (m_iLinestyle) {
    case LS_SOLID:
      m_pen.SetStyle (wxSOLID);
      break;
    case LS_DASH1:
      m_pen.SetStyle (wxLONG_DASH);
      break;
    case LS_DASH2:
      m_pen.SetStyle (wxSHORT_DASH);
      break;
    case LS_DASH3:
      m_pen.SetStyle (wxDOT_DASH);
      break;
    case LS_DASH4:
      m_pen.SetStyle (wxCROSS_HATCH);
      break;
    case LS_DOTTED:
      m_pen.SetStyle (wxDOT);
      break;
    default:
      m_pen.SetStyle (wxSOLID);
      break;
    }
    m_driver.idWX()->SetPen (m_pen);
  }
#endif
}

//==============================================================
// absolute draw to
//*==============================================================

void
SGP::lineAbs (double x, double y)
{
  if (m_bRecalcTransform)
    calc_transform();

  double x1 = m_dCurrentWorldX;
  double y1 = m_dCurrentWorldY;
  mc_to_ndc.transformPoint (&x1, &y1);

  double x2 = x;
  double y2 = y;
  mc_to_ndc.transformPoint (&x2, &y2);

  if (clip_rect (x1, y1, x2, y2, viewNDC) == true) { // clip to viewport
    stylusNDC (x1, y1, false);  // move to first point
    stylusNDC (x2, y2, true);  // draw to second point
  }

  m_dCurrentWorldX = x;
  m_dCurrentWorldY = y;
}

void
SGP::moveAbs (double x, double y)
{
    m_dCurrentWorldX = x;
    m_dCurrentWorldY = y;                       /* moves are not clipped */
}

void
SGP::lineRel (double x, double y)
{
  lineAbs (x + m_dCurrentWorldX, y + m_dCurrentWorldY);
}

void
SGP::moveRel (double x, double y)
{
  moveAbs (x + m_dCurrentWorldX, y + m_dCurrentWorldY);
}


// Height is in master coordinates
void
SGP::setTextSize (double height)
{
    height /= (yw_max - yw_min);  // convert to NDC
#if HAVE_G2_H
  if (m_driver.isG2())
    g2_set_font_size(m_driver.idG2(), (height * m_iPhysicalYSize));
#endif
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
      double dHeightPixels = height * m_iPhysicalYSize;
      double dHeightPoints = dHeightPixels * m_dPointsPerPixel;
      m_pFont->SetPointSize (nearest<int>(dHeightPoints));
#if DEBUG
      sys_error (ERR_TRACE, "Setting text size to %d points", 
                 nearest<int>(dHeightPoints));
#endif

      m_driver.idWX()->SetFont (*m_pFont);
  }
#endif
}

void
SGP::setTextNDCSize (double height)
{
    double dHeightPixels = height * m_iPhysicalYSize;
#if HAVE_G2_H
  if (m_driver.isG2())
    g2_set_font_size(m_driver.idG2(), nearest<int>(dHeightPixels));
#endif
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
      double dHeightPoints = dHeightPixels * m_dPointsPerPixel;
      m_pFont->SetPointSize (nearest<int>(dHeightPoints));
      m_driver.idWX()->SetFont (*m_pFont);
  }
#endif
}

void
SGP::setTextPointSize (double height)
{
#if HAVE_G2_H
    //  if (m_driver.isG2())
    //    g2_set_font_size(m_driver.idG2(), (height * m_iPhysicalYSize));
#endif
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    m_iTextPointSize = static_cast<int>(height+0.5);
    m_pFont->SetPointSize (m_iTextPointSize);
#if DEBUG
    sys_error (ERR_TRACE, "Setting point size to %d", m_iTextPointSize);
#endif
    m_driver.idWX()->SetFont (*m_pFont);
  }
#endif
}

void
SGP::getTextExtent (const char* szText, double* worldW, double* worldH)
{
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    wxCoord deviceW, deviceH;
    wxString str (wxConvCurrent->cMB2WC(szText));
    m_driver.idWX()->GetTextExtent (str, &deviceW, &deviceH);
    if (m_dTextAngle == 90 || m_dTextAngle == -90) {
      wxCoord temp = deviceW;
      deviceW = deviceH;
      deviceH = temp;
    }
    *worldW = (xw_max - xw_min) * deviceW / static_cast<double>(m_iPhysicalXSize);;
    *worldH = (yw_max - yw_min) * deviceH / static_cast<double>(m_iPhysicalYSize);
  }
#endif
}

double
SGP::getCharHeight ()
{
  double dHeight = (1. / 50.);

#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    dHeight = m_driver.idWX()->GetCharHeight();
    dHeight /= static_cast<double>(m_iPhysicalYSize);
    dHeight /= (yv_max - yv_min); // scale to viewport;
  }
#endif
  dHeight *= (yw_max - yw_min);  // scale to world coordinates
  return dHeight;
}

double
SGP::getCharWidth ()
{
  double dWidth = (1. / 80.);

#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    dWidth = m_driver.idWX()->GetCharWidth();
    dWidth /= static_cast<double>(m_iPhysicalXSize);
    dWidth /= (xv_max - xv_min); // scale to viewport
  }
#endif
  dWidth *= (xw_max - xw_min); //scale to world coordinates
  return dWidth;
}

void
SGP::setTextAngle (double angle)
{
  m_dTextAngle = convertRadiansToDegrees(angle);
}

void
SGP::polylineAbs (double x[], double y[], int n)
{
  if (m_bRecalcTransform)
    calc_transform();

  double x1 = x[0], y1 = y[0];
  mc_to_ndc.transformPoint (&x1, &y1);
  double x2 = x[1], y2 = y[1];
  mc_to_ndc.transformPoint (&x2, &y2);

  double xt = x2;       // don't pass (x2,y2) to clip, we need them
  double yt = y2;       // as the beginning point of the next line

  if (clip_rect (x1, y1, xt, yt, viewNDC)) {
    stylusNDC (x1, y1, false);
    stylusNDC (xt, yt, true);
  }

  for (int i = 2; i < n; i++) {
    x1 = x2; y1 = y2;                   // NDC endpoint of last line
    x2 = x[i];  y2 = y[i];
    mc_to_ndc.transformPoint (&x2, &y2);
    xt = x2;
    yt = y2;
    if (clip_rect (x1, y1, xt, yt, viewNDC)) {
      stylusNDC (x1, y1, false);
      stylusNDC (xt, yt, true);
    }
  }
}


void
SGP::markerAbs (double x, double y)
{
  if (m_bRecalcTransform)
    calc_transform();

  double xndc = x;
  double yndc = y;
  mc_to_ndc.transformPoint (&xndc, &yndc);
  markerNDC (xndc, yndc);
  m_dCurrentWorldX = x;
  m_dCurrentWorldY = y;
}


void
SGP::markerRel (double x, double y)
{
  markerAbs (x + m_dCurrentWorldX, y + m_dCurrentWorldY);
}


void
SGP::pointAbs (double x, double y)
{
  if (m_bRecalcTransform)
    calc_transform();
  double xndc = x, yndc = y;
  mc_to_ndc.transformPoint (&xndc, &yndc);
  pointNDC (xndc, yndc);
  m_dCurrentWorldX = x;
  m_dCurrentWorldY = y;
}


void
SGP::pointRel (double x, double y)
{
  pointAbs (x + m_dCurrentWorldX, y + m_dCurrentWorldY);
}


void
SGP::drawText (const std::string& rsMessage)
{
  drawText (rsMessage.c_str());
}

void
SGP::drawText (const char *pszMessage)
{
  if (m_bRecalcTransform)
    calc_transform();

  double xndc = m_dCurrentWorldX;
  double yndc = m_dCurrentWorldY;
  mc_to_ndc.transformPoint (&xndc, &yndc);

  stylusNDC (xndc, yndc, false);            // move to location

#if HAVE_G2_H
  if (m_driver.isG2()) {
    g2_string (m_driver.idG2(), m_iCurrentPhysicalX, m_iCurrentPhysicalY, const_cast<char*>(pszMessage));
  }
#endif
#if HAVE_WXWINDOWS
  if (m_driver.isWX()) {
    wxString str(wxConvCurrent->cMB2WC(pszMessage));
    m_driver.idWX()->DrawRotatedText (str, m_iCurrentPhysicalX, m_iCurrentPhysicalY, m_dTextAngle);
  }
#endif
}


// NAME
//   drawRect                           Draw box in graphics mode
//
// SYNOPSIS
//   drawbox (xmin, ymin, xmax, ymax)
//   double xmin, ymin                  Lower left corner of box
//   double xmax, ymax                  Upper left corner of box
//
// NOTES
//   This routine leaves the current position of graphic cursor at lower
//   left corner of box.

void
SGP::drawRect (double xmin, double ymin, double xmax, double ymax)
{
        moveAbs (xmin, ymin);
        lineAbs (xmax, ymin);
        lineAbs (xmax, ymax);
        lineAbs (xmin, ymax);
        lineAbs (xmin, ymin);
}

// FUNCTION
// sgp2_circle - draw circle of radius r at current center

void
SGP::drawCircle (const double r)
{
        drawArc (r, 0.0, TWOPI);
}

//==============================================================
// draw arc around current center.  angles in radius
//==============================================================

void
SGP::drawArc (const double r, double start, double stop)
{
  if (start > stop) {
    double temp = start;
    start = stop;
    stop = temp;
  }

  double xCent = m_dCurrentWorldX;
  double yCent = m_dCurrentWorldY;

  double x = r * cos (start);
  double y = r * sin (start);
  moveAbs (xCent + x, yCent + y);          // move from center to start of arc

  const double thetaIncrement = (5 * (TWOPI / 360));  // 5 degree increments
  double cosTheta = cos (thetaIncrement);
  double sinTheta = sin (thetaIncrement);

  double angle;
  for (angle = start; angle < stop; angle += thetaIncrement) {
    double xp = cosTheta * x - sinTheta * y; // translate point by thetaIncrement
    double yp = sinTheta * x + cosTheta * y;
    lineAbs (xCent + xp, yCent + yp);
    x = xp; y = yp;
  }

  double c = cos (stop - angle);
  double s = sin (stop - angle);
  double xp = c * x - s * y;
  double yp = s * x + c * y;
  lineAbs (xCent + xp, yCent + yp);

  moveAbs (xCent, yCent);               // move back to center of circle
}



///////////////////////////////////////////////////////////////////////
// Coordinate Transformations
///////////////////////////////////////////////////////////////////////


void
SGP::transformNDCtoMC (double* x, double* y)
{
  if (m_bRecalcTransform)
    calc_transform();
  ndc_to_mc.transformPoint (x, y);
}


void
SGP::transformMCtoNDC (double* x, double* y)
{
  if (m_bRecalcTransform)
    calc_transform();
  mc_to_ndc.transformPoint (x, y);
}


void
SGP::transformMCtoNDC (double xIn, double yIn, double* x, double* y)
{
  if (m_bRecalcTransform)
    calc_transform();
  *x = xIn;
  *y = yIn;
  mc_to_ndc.transformPoint (x, y);
}


// NAME
//      calc_transform                  Calculate transform matrices

void
SGP::calc_transform ()
{
  double scaleX = (xv_max - xv_min) / (xw_max - xw_min);
  double scaleY = (yv_max - yv_min) / (yw_max - yw_min);

  wc_to_ndc.setIdentity();
  wc_to_ndc.mtx[0][0] = scaleX;
  wc_to_ndc.mtx[2][0] = xv_min - scaleX * xw_min;
  wc_to_ndc.mtx[1][1] = scaleY;
  wc_to_ndc.mtx[2][1] = yv_min - scaleY * yw_min;

  mc_to_ndc = m_ctm * wc_to_ndc;
  ndc_to_mc = mc_to_ndc.invert();

  m_bRecalcTransform = false;
}

void
SGP::ctmClear ()
{
  m_ctm.setIdentity();
  calc_transform();
}

void
SGP::ctmSet (const TransformationMatrix2D& m)
{
  m_ctm = m;
  calc_transform();
}


void
SGP::preTranslate  (double x, double y)
{
    TransformationMatrix2D m;

    m.setTranslate (x, y);
    ctmSet (m * m_ctm);
}


void
SGP::postTranslate (double x, double y)
{
    TransformationMatrix2D m;

    m.setTranslate (x, y);
    ctmSet (m_ctm * m);
}


void
SGP::preScale (double sx, double sy)
{
    TransformationMatrix2D m;

    m.setScale (sx, sy);
    ctmSet (m * m_ctm);
}


void
SGP::postScale (double sx, double sy)
{
    TransformationMatrix2D m;

    m.setScale (sx, sy);
    m_ctm = m_ctm * m;
    ctmSet (m_ctm * m);
}


void
SGP::preRotate (double theta)
{
    TransformationMatrix2D m;

    m.setRotate (theta);
    m_ctm = m * m_ctm;
    ctmSet (m * m_ctm);
}


void
SGP::postRotate (double theta)
{
    TransformationMatrix2D m;

    m.setRotate (theta);
    ctmSet (m_ctm * m);
}


void
SGP::preShear (double shrx, double shry)
{
    TransformationMatrix2D m;

    m.setShear (shrx, shry);
    ctmSet (m * m_ctm);
}


void
SGP::postShear (double shrx, double shry)
{
    TransformationMatrix2D m;

    m.setShear (shrx, shry);
    ctmSet (m_ctm * m);
}


////////////////////////////////////////////////////////////////////////
//  Bitmap Markers
////////////////////////////////////////////////////////////////////////

// Pixel patterns of marker symbols (1x1 to 5x5 matrix)
const unsigned char SGP::MARKER_BITMAP[MARK_COUNT][5] =
{
    {'\000', '\000', '\010', '\000', '\000'},    // small dot
    {'\000', '\034', '\024', '\034', '\000'},    // empty square
    {'\000', '\034', '\034', '\034', '\000'},    // filled square
    {'\000', '\010', '\024', '\010', '\000'},    // empty diamond
    {'\000', '\010', '\034', '\010', '\000'},    // filled diamond
    {'\010', '\010', '\076', '\010', '\010'},    // cross
    {'\000', '\024', '\010', '\024', '\000'},    // X
    {'\034', '\042', '\042', '\042', '\034'},    // open circle
    {'\034', '\076', '\076', '\076', '\034'},    // filled circle
    {'\076', '\042', '\042', '\042', '\076'},    // big open square
    {'\010', '\024', '\042', '\024', '\010'},    // big open diamond
};


#if HAVE_WXWINDOWS
void
SGP::setDC (wxDC* pDC)
{
  if (m_driver.isWX()) {
    m_driver.setDC(pDC);
    initFromDC (pDC);
    setTextPointSize (m_iTextPointSize);
  }
}
#endif
