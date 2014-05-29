/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         sgp.h
**      Purpose:      Header file for Simple Graphics Package
**      Author:       Kevin Rosenberg
**      Date Started: 1984
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

#ifndef __H_SGP
#define __H_SGP

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "transformmatrix.h"

#ifdef HAVE_WXWINDOWS
#include <wx/wx.h>
#include <wx/font.h>
#endif

#if HAVE_G2_H
extern "C" {
#include "g2.h"
#include "g2_X11.h"
}
#endif

#include <string>

class SGPDriver {
private:
  int m_iPhysicalXSize;
  int m_iPhysicalYSize;
  std::string m_sWindowTitle;
  int m_idDriver;

#ifdef HAVE_WXWINDOWS
  wxDC* m_pDC;
#endif
  int m_idG2;

public:
  enum {
    SGPDRIVER_WXWINDOWS = 1,
    SGPDRIVER_G2 = 2,
    SGPDRIVER_OPENGL = 4,
  };

#ifdef HAVE_WXWINDOWS
  SGPDriver (wxDC* pDC, int xsize = 640, int ysize = 480);
#endif

  SGPDriver (const char* szWinTitle = "", int xsize = 640, int ysize = 480);

  ~SGPDriver ();

  int getPhysicalXSize () const
    { return m_iPhysicalXSize; }

  int getPhysicalYSize () const
    { return m_iPhysicalYSize; }

  const std::string& getWindowTitle () const
    { return m_sWindowTitle; }

  bool isWX () const
  { return (m_idDriver & SGPDRIVER_WXWINDOWS ? true : false); }

  bool isG2 () const
  { return (m_idDriver & SGPDRIVER_G2 ? true : false); }

  int idG2 () const
    { return m_idG2; }

#ifdef HAVE_WXWINDOWS
  wxDC* idWX () const
    { return m_pDC; }

  void setDC (wxDC* dc)
      { m_pDC = dc; }
#endif
};


class SGP_RGBColor;
class SGP {
private:
  int m_iPhysicalXSize;   // Physical Window size
  int m_iPhysicalYSize;
  SGPDriver m_driver;

  double xw_min;    // Window extents
  double yw_min;
  double xw_max;
  double yw_max;
  double xv_min;    // Viewport extents
  double yv_min;
  double xv_max;
  double yv_max;
  double viewNDC[4];   // Viewport array for clip_rect()

  int m_iCurrentPhysicalX;
  int m_iCurrentPhysicalY;
  double m_dCurrentWorldX;
  double m_dCurrentWorldY;
  double m_dTextAngle;
  int m_iTextPointSize;
  bool m_bRecalcTransform;
  double m_dPointsPerPixel;  // points (72pt/in) per screen pixel;
  int m_iLinestyle;
  int m_iMarker;

  // Master coordinates are coordinates before CTM transformation
  // World coordinates are coordinates defined by setWindow()
  // Normalized device coordinates range from 0. to 1. in both axes
  TransformationMatrix2D wc_to_ndc;     // World coord to NDC matrix
  TransformationMatrix2D mc_to_ndc;     // Master to NDC
  TransformationMatrix2D ndc_to_mc;     // NDC to Master
  TransformationMatrix2D m_ctm;         // Current transfromation matrix

  void calc_transform ();

  static SGP_RGBColor s_aRGBColor[];
  static int s_iRGBColorCount;

#if HAVE_WXWINDOWS
  wxPen m_pen;
  wxFont* m_pFont;

  void initFromDC (wxDC* pDC);
#endif

public:
  enum {                  // linestyles
      LS_NOLINE = 0,
      LS_SOLID = 0xffff,
      LS_DASH1 = 0xff00,
      LS_DASH2 = 0xf0f0,
      LS_DASH3 = 0xcccc,
      LS_DASH4 = 0xff3e,
      LS_DOTTED = 0xaaaa,
  };

  enum {            // Codes for marker symbols
      MARKER_POINT = 0, // small dot
      MARKER_SQUARE = 1,        // empty square
      MARKER_FSQUARE = 2,       // filled square
      MARKER_DIAMOND = 3,       // empty diamond
      MARKER_FDIAMOND = 4,      // filled diamond
      MARKER_CROSS =  5,        // cross
      MARKER_XCROSS = 6,        // x
      MARKER_CIRCLE = 7,    // open circle
      MARKER_FCIRCLE = 8,       // filled circle
      MARKER_BSQUARE = 9,       // big open square
      MARKER_BDIAMOND = 10,     // big open diamond
  };
  enum  { MARK_COUNT = 11, };
  static const unsigned char MARKER_BITMAP[MARK_COUNT][5];

  SGP (const SGPDriver& driver);
  ~SGP();

  void drawCircle (const double r);
  void drawArc (const double r, double start, double stop);
  void drawRect (double xmin, double ymin, double xmax, double ymax);
  void lineAbs(double x, double y);
  void moveAbs(double x, double y);
  void lineRel(double x, double y);
  void moveRel(double x, double y);
  void drawText(const char *szMessage);
  void drawText(const std::string& rsMessage);
  void polylineAbs(double x[], double y[], int n);
  void markerAbs (double x, double y);
  void markerRel(double x, double y);
  void pointAbs(double x, double y);
  void pointRel(double x, double y);

  void eraseWindow ();
  void setWindow (double xmin, double ymin, double xmax, double ymax);
  void setViewport (double xmin, double ymin, double xmax, double ymax);
  void frameViewport();

  void setColor (int icol);
  void setLineStyle (int style);
  void setTextSize (double height);
  void setTextNDCSize (double height);
  void setTextPointSize (double height);
  void setTextAngle (double angle);
  void setTextColor (int iFGcolor, int iBGcolor);
  void setPenWidth (int width);
  void setMarker (int idMarker);
  void setRasterOp (int ro);

  void getWindow (double& xmin, double& ymin, double& xmax, double& ymax);
  void getViewport (double& xmin, double& ymin, double& xmax, double& ymax);
  void getTextExtent (const char *szText, double* x, double* y);
  double getCharHeight ();
  double getCharWidth ();
  SGPDriver& getDriver() {return m_driver;}
  const SGPDriver& getDriver() const {return m_driver;}

  void ctmClear ();
  void ctmSet (const TransformationMatrix2D& m);
  void preTranslate (double x, double y);
  void postTranslate (double x, double y);
  void preScale (double sx, double sy);
  void postScale (double sx, double sy);
  void preRotate (double theta);
  void postRotate (double theta);
  void preShear (double shrx, double shry);
  void postShear (double shrx, double shry);
  void transformNDCtoMC (double* x, double* y);
  void transformMCtoNDC (double* x, double* y);
  void transformMCtoNDC (double xIn, double yIn, double* xOut, double* yOut);

  void stylusNDC (double x, double y, bool beam);
  void pointNDC (double x, double y);
  void markerNDC (double x, double y);

#if HAVE_WXWINDOWS
  void setDC (wxDC* pDC);
#endif
};


enum {
    C_BLACK     = 0,     // color codes
    C_BLUE      = 1,
    C_GREEN     = 2,
    C_CYAN      = 3,
    C_RED       = 4,
    C_MAGENTA   = 5,
    C_BROWN     = 6,
    C_GRAY      = 7,
    C_LTGRAY    = 8,
    C_LTBLUE    = 9,
    C_LTGREEN   = 10,
    C_LTCYAN    = 11,
    C_LTRED     = 12,
    C_LTMAGENTA = 13,
    C_YELLOW    = 14,
    C_WHITE     = 15,
};

enum RasterOp {
    RO_AND = 1,
    RO_AND_INVERT,
    RO_AND_REVERSE,
    RO_CLEAR,
    RO_COPY,
    RO_EQUIV,
    RO_INVERT,
    RO_NAND,
    RO_NOR,
    RO_NO_OP,
    RO_OR,
    RO_OR_INVERT,
    RO_OR_REVERSE,
    RO_SET,
    RO_SRC_INVERT,
    RO_XOR,
};


class SGP_RGBColor {
 private:
  short int r;
  short int g;
  short int b;

 public:
  SGP_RGBColor (int r, int g, int b)
    : r(r), g(g), b(b)
    {}

  int getRed () const
    { return r; }

  int getGreen () const
    { return g; }

  int getBlue () const
    { return b; }

};

#endif
