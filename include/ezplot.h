/*****************************************************************************
** FILE IDENTIFICATION
**
**  Name:    ezplot.h
**  Purpose: Header file for EZplot library
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


#ifndef __H_EZPLOT
#define __H_EZPLOT

#include <cstdio>
#include <cctype>
#include <cmath>
#include <stddef.h>
#include "ctsupport.h"
#include "sgp.h"
#include "pol.h"


class EZPlotCurve {
public:
  double *x;
  double *y;
  int m_iPointCount;

  EZPlotCurve (const double* x, const double* y, int n);

  ~EZPlotCurve();
};

//----------------------------------------------------------------------
//                             GLOBAL VARIABLES
//----------------------------------------------------------------------

// axis definitions
enum {
  LINEAR =      1,              // linear axis
  LOG,          // logrithmic axis
  NOAXIS,       // don't plot axis
};

// tick definitions
enum {
  ABOVE = 1,
 BELOW,
 RIGHT,
 LEFT,
};

// line types
enum {
 NOLINE =       0,
 SOLID,
 DASH,
 DASH1,
 DASH2,
 DASH3,
 DASH4,
};

// symbol definitions
enum {
 SB_CROSS = 1,
 SB_PLUS,
 SB_BOX,
 SB_CIRCLE,
 SB_ERRORBAR,
 SB_POINT,
 MAXSYMBOL,
};

enum {
 INSIDE = 1,            // values of o_legendbox
 OUTSIDE,
 NOLEGEND,
};


struct KeywordCodeTable {
  const char* keyword;
  int code;
};

/*-----------------------------------------------------------------------------
*                               GLOBAL VARIABLES
*
* Naming Convention:
*       i_   Internal variable
*               Not user changable
*       o_   Option variable
*               Normal variable that is user modifiable
*               These variables must always have a valid value
*       d_   Device variable
*               Variables controlling devices
*      clr_ Color variable
*               Holds a color value
*       c_   Character string variable
*               Contains a character string
*       v_   Value variable
*               User modifiable variable associated with the set variable (s_)
*               These variables do not always have a valid value
*               These variables change assumption EZPLOT makes about the plot
*       s_   Set variable.
*               TRUE if associated value variable (v_) has been set by the user
*---------------------------------------------------------------------------*/

#include <vector>

typedef std::vector<EZPlotCurve*>::iterator EZPlotCurveIterator;
typedef std::vector<EZPlotCurve*>::const_iterator EZPlotCurveConstIterator;

class SGP;
class EZPlot {
private:
  //----------------------------------------------------------------------
  //                    POL Codes
  //----------------------------------------------------------------------

  enum {
    S_DATA = 2,
      S_HELP,
      S_EXIT,
      S_CURVE,
      S_SOLID,
      S_DASH,
      S_NOLINE,
      S_BLACK,
      S_RED,
      S_GREEN,
      S_BLUE,
      S_SYMBOL,
      S_PEN,
      S_EVERY,
      S_NONE,
      S_LEGEND,
      S_XLEGEND,
      S_YLEGEND,
      S_XLIN,
      S_YLIN,
      S_XLOG,
      S_YLOG,
      S_XLABEL,
      S_YLABEL,
      S_XLENGTH,
      S_YLENGTH,
      S_XTICKS,
      S_YTICKS,
      S_ABOVE,
      S_LABEL,
      S_BELOW,
      S_NOLABEL,
      S_RIGHT,
      S_LEFT,
      S_XAUTOSCALE,
      S_YAUTOSCALE,
      S_XMIN,
      S_YMIN,
      S_XMAX,
      S_YMAX,
      S_LXFRAC,
      S_LYFRAC,
      S_XCROSS,
      S_YCROSS,
      S_NOXAXIS,
      S_NOYAXIS,
      S_XPORIGIN,
      S_YPORIGIN,
      S_TITLE,
      S_XTITLE,
      S_YTITLE,
      S_REPLOT,
      S_CLEAR,
      S_STORE,
      S_RESTORE,
      S_AMARK,
      S_NO,
      S_INTERACTIVE,
      S_UNITS,
      S_INCHES,
      S_USER,
      S_BOX,
      S_NOBOX,
      S_GRID,
      S_NOGRID,
      S_MAJOR,
      S_MINOR,
      S_COLOR,
      S_LEGENDBOX,
      S_TAG,
      S_TEXTSIZE,
  };

  static const struct KeywordCodeTable m_sKeywords[];
  static const int NKEYS;

  std::vector<class EZPlotCurve*> m_vecCurves;
  std::vector<int> m_veciColor;
  std::vector<bool> m_vecbColorSet;
  std::vector<int> m_veciSymbol;
  std::vector<bool> m_vecbSymbolSet;
  std::vector<int> m_veciSymbolFreq;
  std::vector<bool> m_vecbSymbolFreqSet;
  std::vector<int> m_veciLinestyle;
  std::vector<bool> m_vecbLinestyleSet;
  std::vector<std::string> m_vecsLegend;
  std::vector<bool> m_vecbLegendSet;

  int getColor (unsigned int iCurve) const;
  int getSymbol (unsigned int iCurve) const;
  const std::string* getLegend (unsigned int iCurve) const;
  int getSymbolFreq (unsigned int iCurve) const;
  int getLinestyle (unsigned int iCurve) const;

  void setColor (unsigned int iCurve, int iColor);
  void setSymbol (unsigned int iCurve, int iSymbol);
  void setSymbolFreq (unsigned int iCurve, int iSymbolFreq);
  void setLinestyle (unsigned int iCurve, int iLinestyle);
  void setLegend (unsigned int iCurve, const std::string& strLegend);
  void setLegend (unsigned int iCurve, const char* const pszLegend);

  int m_iCurrentCurve;

  // Colors
  int clr_axis;                 // color of all axis lines
  int clr_title;                        // color of main title
  int clr_label;                        // color of axis labels
  int clr_legend;                       // color of legend box
  int clr_grid;                 // color of grid lines
  int clr_number;                       // color of axis number labels

  // Options
  double o_xporigin, o_yporigin;        // origin of plot frame in NDC
  double o_xlength, o_ylength;  // length of plot frame in NDC

  std::string c_xlabel; // label for x axis
  std::string c_ylabel; // label for y axis
  std::string c_title;          // title to print above graph

  int o_linestyle, o_color;     // style to use for curves all subsequent curves to EZPLOT
  int o_xaxis, o_yaxis;         // Specifies where axis & labels are drawn
  bool o_grid;                  // Flag to draw a grid at major ticks
  bool o_box;                   // Flag to draw a box around the graph

  int o_xticks, o_yticks;               // direction to draw tick marks
  bool o_xtlabel, o_ytlabel;    // TRUE if label tick marks

  int o_xmajortick, o_ymajortick;       // number of major ticks to draw
  int o_xminortick, o_yminortick;       // number of minor ticks between major ticks

  int o_symbol;                 // Symbol type, (0 = no symbol)
  int o_symfreq;                        // frequency to draw symbols at curve points

  int o_legendbox;              // controls whether legend is inside or outside of the axis extents
  int o_tag;                    // controls whether to draw tag at end of axes

  // VALUE & SET variables
  double v_xmin, v_xmax, v_ymin, v_ymax;        // user supplied axis endpoints
  bool   s_xmin, s_xmax, s_ymin, s_ymax;        // TRUE is endpoint has been set
  double v_xtitle, v_ytitle;    // NDC position to plot title
  bool   s_xtitle, s_ytitle;    // TRUE if set position for title
  double v_xcross, v_ycross;    // position that axes cross
  bool   s_xcross, s_ycross;    // TRUE if set axes cross position
  double v_xlegend, v_ylegend;  // upper-left position of legend box in NDC
  bool   s_xlegend, s_ylegend;  // TRUE if set position of legend box
  int  v_lxfrac, v_lyfrac;      // number of digits to right of decimal place
  bool s_lxfrac, s_lyfrac;      // TRUE if set number of fractional digits
  double v_textsize;            // size of text in NDC
  bool   s_textsize;            // TRUE if user set size of text

  // Global variables
  double charheight;    // Height of characters in NDC
  double charwidth;     // Height of characters in NDC
  double  xp_min, xp_max, yp_min, yp_max;       // boundry of plot frame in NDC
  double  xa_min, xa_max, ya_min, ya_max;       // extent of axes in NDC
  double  xgw_min, xgw_max, ygw_min, ygw_max;   // boundary of graph in input coords
  double  xgn_min, xgn_max, ygn_min, ygn_max;   // boundy of graph in NDC
  double xt_min, xt_max, yt_min, yt_max;        // boundary of axis ticks
  double  xl_min, xl_max, yl_min, yl_max;       // boundary of legend box
  double title_row;     // y-coord of title row
  double xtl_ofs;               // Offset y-coord of x tick labels from axis
  double ytl_ofs;               // Offset x-coord of y tick labels from axis
  double xlbl_row;      // row of x label in world coord
  double ylbl_col;      // column of y label in world coord
  double xw_tickinc, yw_tickinc;        // increment between major ticks in WC
  double xn_tickinc, yn_tickinc;        // increment between major ticks in NDC
  int x_nint, y_nint;   // number of intervals along x & y axes
  int x_fldwid, x_frac; // numeric field sizes & number of digits
  int y_fldwid, y_frac; // in fraction of number, used for printf()
  double xtl_wid, ytl_wid;      // length of ticks labels in NDC
  double tl_height;     // height of tick labels in NDC
  char x_numfmt[20];    // format to print x tick labels
  char y_numfmt[20];    // format to print y tick labels

  double m_dVP_xmin, m_dVP_ymin;
  double m_dVP_xmax, m_dVP_ymax;
  double m_dVP_xscale, m_dVP_yscale;
  double m_xWorldScale, m_yWorldScale;

  void drawAxes();
  void symbol (int sym, double symwidth, double symheight);
  void make_numfmt(char *fmtstr, int *fldwid, int *nfrac, double min, double max, int nint);
  int axis_scale (double min, double max, int nint, double *minp, double *maxp, int *nintp);

  SGP* m_pSGP;
  POL m_pol;

  void clearCurves ();

  bool ezcmd (const char* const comm);
  bool do_cmd(int lx);
  void bad_option(const char *opt);
  void initPlotSettings();

  void initKeywords ();

  double convertWorldToNDC_X (double x)
  { return xgn_min + (x - xgw_min) * m_xWorldScale; }

  double convertWorldToNDC_Y (double y)
  { return ygn_min + (y - ygw_min) * m_yWorldScale; }

 public:
   EZPlot ();
   ~EZPlot ();

   bool ezset (const std::string& command);
   bool ezset (const char* const command);

   void addCurve (const float* x, const double* y, int num);
   void addCurve (const double* x, const float* y, int num);
   void addCurve (const double* x, const double* y, int num);
   void addCurve (const double* y, int n);
   void addCurve (const float* y, int n);

   void plot (SGP* pSGP);
};

#endif
