/*****************************************************************************
** FILE IDENTIFICATION
**
**  ezplot.cpp
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

#include "ezplot.h"
#include <algorithm>

#ifdef __GNUWIN32__
int snprintf (char *, size_t, const char*, ...);
#endif

// Defaults
static const double TICKRATIO = 0.4;    // ratio of minor to major tick lengths
static const int MAXNUMFMT = 15;        // maximum length of a numeric format
static const int DEF_CURVE_CLR = C_RED;


EZPlotCurve::EZPlotCurve (const double* xData, const double* yData, int n)
: x(new double[n]), y(new double[n])
{
  for (int i = 0; i < n; i++) {
    x[i] = xData[i];
    y[i] = yData[i];
  }

  m_iPointCount = n;
}

EZPlotCurve::~EZPlotCurve ()
{
  delete x;
  delete y;
}


void
EZPlot::addCurve (const double *y, int n)
{
  double* x = new double [n];

  for (int i = 0; i < n; i++)
    x[i] = i;

  addCurve (x, y, n);
  delete x;
}


void
EZPlot::addCurve (const float *y, int n)
{
  double* yDouble = new double [n];

  for (int i = 0; i < n; i++)
    yDouble[i] = y[i];

  addCurve (yDouble, n);
  delete yDouble;
}


void
EZPlot::addCurve (const float x[], const double y[], int num)
{
  double* dx = new double [num];

  for (int i = 0; i < num; i++)
    dx[i] = x[i];

  addCurve (dx, y, num);
  delete dx;
}

void
EZPlot::addCurve (const double* const x, const float* const y, int num)
{
  double* dy = new double [num];

  for (int i = 0; i < num; i++)
    dy[i] = y[i];

  addCurve (x, dy, num);

  delete dy;
}


void
EZPlot::addCurve (const double* const x, const double* const y, int num)
{
  if (num < 1)
    return;

  EZPlotCurve* pCurve = new EZPlotCurve (x, y, num);
  m_vecCurves.push_back (pCurve);
}


EZPlot::~EZPlot ()
{
  for (EZPlotCurveIterator i = m_vecCurves.begin(); i != m_vecCurves.end(); i++)
    delete *i;
}

void
EZPlot::clearCurves ()
{
  for (EZPlotCurveIterator i = m_vecCurves.begin(); i != m_vecCurves.end(); i++)
    delete *i;
  m_vecCurves.erase (m_vecCurves.begin(), m_vecCurves.end());
  initPlotSettings();
}


EZPlot::EZPlot ()
{
    initKeywords();

    m_pol.addSkipWord ("please");

    m_pol.addSkipWord ("use");

    m_pol.addSkipWord ("are");

    m_pol.addSkipWord ("and");

    m_pol.addSkipWord ("is");

    m_pol.addSkipWord ("the");

    m_pol.addSkipWord ("equals");

    m_pol.addSkipChar ('=');



    m_pol.usefile (POL::P_USE_STR,"");

    m_pol.set_inputline ("!eoc ,");

    m_pol.reader ();

    m_pol.closefile ();



    initPlotSettings();
}

void
EZPlot::initPlotSettings ()
{
  m_iCurrentCurve = -1;

  m_pSGP = NULL;


  c_xlabel = "";
  c_ylabel =  "";
  c_title = "";

  o_xporigin = 0.0;
  o_yporigin = 0.0;
  o_xlength  = 1.0;
  o_ylength  = 1.0;

  o_xaxis = LINEAR;
  o_yaxis = LINEAR;

  o_grid = FALSE;
  o_box = FALSE;

  o_xmajortick = 10;
  o_ymajortick =  8;
  o_xminortick =  4;
  o_yminortick =  4;

  o_color = DEF_CURVE_CLR;
  o_symfreq = 1;
  o_symbol = -1;
  o_linestyle = SGP::LS_SOLID;

  o_xtlabel = TRUE;
  o_ytlabel = TRUE;
  o_xticks = BELOW;
  o_yticks = LEFT;

  o_legendbox = INSIDE;
  o_tag = FALSE;

  s_xtitle   = FALSE;
  s_ytitle   = FALSE;
  s_xcross   = FALSE;
  s_ycross   = FALSE;
  s_lxfrac   = FALSE;
  s_lyfrac   = FALSE;
  s_xlegend  = FALSE;
  s_ylegend  = FALSE;
  s_textsize = FALSE;
  s_xmin     = FALSE;
  s_xmax     = FALSE;
  s_ymin     = FALSE;
  s_ymax     = FALSE;

  clr_axis   = C_LTGRAY;                // set fixed colors
  clr_title  = C_RED;
  clr_label  = C_BLUE;
  clr_legend = C_CYAN;
  clr_number = C_GREEN;
  clr_grid   = C_LTGRAY;
}

void

EZPlot::setColor (unsigned int iCurve, int iColor)

{

  if (m_veciColor.size() <= iCurve) {

    m_veciColor.resize ((m_iCurrentCurve + 1) * 2);

    m_vecbColorSet.resize ((m_iCurrentCurve + 1) * 2);

  }

  m_veciColor [iCurve] = iColor;

  m_vecbColorSet [iCurve] = true;

}



void

EZPlot::setSymbol (unsigned int iCurve, int iSymbol)

{

  if (m_veciSymbol.size() <= iCurve) {

    m_veciSymbol.resize ((m_iCurrentCurve + 1) * 2);

    m_vecbSymbolSet.resize ((m_iCurrentCurve + 1) * 2);

  }

  m_veciSymbol [iCurve] = iSymbol;

  m_vecbSymbolSet [iCurve] = true;

}



void

EZPlot::setSymbolFreq (unsigned int iCurve, int iSymbolFreq)

{

  if (m_veciSymbolFreq.size() <= iCurve) {

    m_veciSymbolFreq.resize ((m_iCurrentCurve + 1) * 2);

    m_vecbSymbolFreqSet.resize ((m_iCurrentCurve + 1) * 2);

  }

  m_veciSymbolFreq [iCurve] = iSymbolFreq;

  m_vecbSymbolFreqSet [iCurve] = true;

}



void

EZPlot::setLinestyle (unsigned int iCurve, int iLinestyle)

{

  if (m_veciLinestyle.size() <= iCurve) {

    m_veciLinestyle.resize ((m_iCurrentCurve + 1) * 2);

    m_vecbLinestyleSet.resize ((m_iCurrentCurve + 1) * 2);

  }

  m_veciLinestyle [iCurve] = iLinestyle;

  m_vecbLinestyleSet [iCurve] = true;

}



void

EZPlot::setLegend (unsigned int iCurve, const std::string& strLegend)

{

  if (m_vecsLegend.size() <= iCurve) {

    m_vecsLegend.resize ((m_iCurrentCurve + 1) * 2);

    m_vecbLegendSet.resize ((m_iCurrentCurve + 1) * 2);

  }

  m_vecsLegend [iCurve] = strLegend;

  m_vecbLegendSet [iCurve] = true;

}



void

EZPlot::setLegend (unsigned int iCurve, const char* const pszLegend)

{

  if (m_vecsLegend.size() <= iCurve) {

    m_vecsLegend.resize ((m_iCurrentCurve + 1) * 2);

    m_vecbLegendSet.resize ((m_iCurrentCurve + 1) * 2);

  }

  m_vecsLegend [iCurve] = pszLegend;

  m_vecbLegendSet [iCurve] = true;

}



int

EZPlot::getColor (unsigned int iCurve) const

{

  if (m_veciColor.size() > iCurve && m_vecbColorSet[iCurve])

    return m_veciColor[iCurve];

  else

    return o_color;

}



int

EZPlot::getSymbol (unsigned int iCurve) const

{

  if (m_veciSymbol.size() > iCurve && m_vecbSymbolSet[iCurve])

    return m_veciSymbol[iCurve];

  else

    return o_symbol;

}



int

EZPlot::getSymbolFreq (unsigned int iCurve) const

{

  if (m_veciSymbolFreq.size() > iCurve && m_vecbSymbolFreqSet[iCurve])

    return m_veciSymbolFreq[iCurve];

  else

    return o_symfreq;

}



int

EZPlot::getLinestyle (unsigned int iCurve) const

{

  if (m_veciLinestyle.size() > iCurve && m_vecbLinestyleSet[iCurve])

    return m_veciLinestyle[iCurve];

  else

    return o_linestyle;

}



const std::string*

EZPlot::getLegend (unsigned int iCurve) const

{

  if (m_vecsLegend.size() > iCurve && m_vecbLegendSet[iCurve])

    return &m_vecsLegend[iCurve];

  else

    return NULL;

}





/* NAME
*   plot                Plots all curves collected by addCurves ()
*
* SYNOPSIS
*   plot()
*
* DESCRIPTION
*   This routine plots the curves that have stored by addCurves().
*
* CALLS
*   drawAxes() & make_numfmt()
*/

void
EZPlot::plot (SGP* pSGP)
{
  if (m_vecCurves.size() <= 0)
    return;

  m_pSGP = pSGP;

  m_pSGP->setWindow (0., 0., 1., 1.);

  if (s_textsize == TRUE)
    m_pSGP->setTextPointSize (v_textsize);

  charheight = m_pSGP->getCharHeight();
  charwidth = m_pSGP->getCharWidth();
  double symheight = charheight * 0.3;       // size of symbol in NDC
  double symwidth = symheight;

  if (m_vecCurves.size() < 1)
    return; // can't plot if there are no curves to plot

  // find a curve with at least one point to get base point
  double xmin, xmax, ymin, ymax;   // extent of curves in world coord
  bool found = false;

  for (unsigned int iCurve = 0; iCurve < m_vecCurves.size(); iCurve++) {
    const EZPlotCurve* const pCurve = m_vecCurves [iCurve];
    if (pCurve->m_iPointCount > 0) {
      xmin = pCurve->x[0];
      xmax = xmin;
      ymin = pCurve->y[0];
      ymax = ymin;
      found = true;
      break;
    }
  }
  if (! found)
    return; // curve(s) are empty, so no plotting

  for (unsigned int iCurve = 0; iCurve < m_vecCurves.size(); iCurve++) {
    const EZPlotCurve* const pCurve = m_vecCurves [iCurve];

    for (int ip = 0; ip < pCurve->m_iPointCount; ip++) {
      if (pCurve->x[ip] > xmax)
        xmax = pCurve->x[ip];
      else if (pCurve->x[ip] < xmin)
        xmin = pCurve->x[ip];
      if (pCurve->y[ip] > ymax)
        ymax = pCurve->y[ip];
      else if (pCurve->y[ip] < ymin)
        ymin = pCurve->y[ip];
    }
  }

  // extend graph limits for user defined axis cross positions
  if (s_xcross == TRUE) {
    if (v_xcross < xmin)
      xmin = v_xcross;
    else if (v_xcross > xmax)
      xmax = v_xcross;
  }

  if (s_ycross == TRUE) {
    if (v_ycross < ymin)
      ymin = v_ycross;
    else if (v_ycross > ymax)
      ymax = v_ycross;
  }

  // find nice endpoints for axes
  if (! axis_scale (xmin, xmax, o_xmajortick - 1, &xgw_min, &xgw_max, &x_nint) || ! axis_scale (ymin, ymax, o_ymajortick - 1, &ygw_min, &ygw_max, &y_nint))
    return;


  // check if user set x-axis extents
  if (s_xmin == TRUE) {
    xgw_min = v_xmin;
    x_nint = o_xmajortick - 1;
  }
  if (s_xmax == TRUE) {
    xgw_max = v_xmax;
    x_nint = o_xmajortick - 1;
  }

  // check if user set y-axis extents
  if (s_ymin == TRUE) {
    ygw_min = v_ymin;
    y_nint = o_ymajortick - 1;
  }
  if (s_ymax == TRUE) {
    ygw_max = v_ymax;
    y_nint = o_ymajortick - 1;
  }

  // calculate increment between major axis in world coordinates
  xw_tickinc = (xgw_max - xgw_min) / x_nint;
  yw_tickinc = (ygw_max - ygw_min) / y_nint;

  // we have now calcuated xgw_min, xyw_max, ygw_min, & ygw_max

  // set the number of decimal point to users' setting or default
  // Two formats for numbers: Fixed:    -nnn.f and  Exponent: -n.fffE+eee
  if (s_lxfrac == TRUE)
    x_frac = v_lxfrac;
  else
    x_frac = -1;

  if (s_lyfrac == TRUE)
    y_frac = v_lyfrac;
  else
    y_frac = -1;

  make_numfmt (x_numfmt, &x_fldwid, &x_frac, xgw_min, xgw_max, x_nint);
  make_numfmt (y_numfmt, &y_fldwid, &y_frac, ygw_min, ygw_max, y_nint);

  xtl_wid = x_fldwid * charwidth;               // calc size of tick labels
  ytl_wid = y_fldwid * charwidth;
  tl_height = charheight;

  // calculate the extent of the plot frame
  xp_min = o_xporigin;
  yp_min = o_yporigin;
  xp_max = xp_min + o_xlength;
  yp_max = yp_min + o_ylength;

  xp_min = clamp (xp_min, 0., 1.);
  xp_max = clamp (xp_max, 0., 1.);
  yp_min = clamp (yp_min, 0., 1.);
  yp_max = clamp (yp_max, 0., 1.);

  xa_min = xp_min;              // extent of axes
  xa_max = xp_max;
  ya_min = yp_min;
  ya_max = yp_max;

  // adjust frame for title
  title_row = ya_max;;

  if (c_title.length() > 0)
    ya_max -= 2 * charheight;

  else

    ya_max -= 0.7 * charheight;  // allow room for yaxis ticklabel


  // calculate legend box boundaries
  int max_leg = 0;                      // longest legend in characters
  int num_leg = 0;                      // number of legend titles

  for (unsigned int iCurve = 0; iCurve < m_vecCurves.size(); iCurve++) {
    const std::string* pstrLegend = getLegend (iCurve);

    if (pstrLegend && pstrLegend->length() > 0) {

      int nLegend = pstrLegend->length();
      if (nLegend > 0) {
        ++num_leg;
        if (nLegend > max_leg)

          nLegend = max_leg;

      }
    }
  }

  if (num_leg > 0 && o_legendbox != NOLEGEND) {
    double leg_width  = (max_leg + 2) * charwidth;      // size of legend box
    double leg_height = num_leg * 3 * charheight;

    if (s_xlegend == TRUE)
      xl_max = v_xlegend;
    else {
      xl_max = xa_max;
      if (o_legendbox == OUTSIDE)
        xa_max -= (leg_width + 0.5 * charwidth);
    }
    xl_min = xl_max - leg_width;

    if (s_ylegend == TRUE)
      yl_max = v_ylegend;
    else
      yl_max = ya_max;

    yl_min = yl_max - leg_height;

    m_pSGP->setColor (clr_legend);
    m_pSGP->drawRect (xl_min, yl_min, xl_max, yl_max);

    int iLegend = 0;                    // current legend position

    for (unsigned int iCurve = 0; iCurve < m_vecCurves.size(); iCurve++) {
      const std::string* pstrLegend = getLegend (iCurve);
      if (! pstrLegend || pstrLegend->length() == 0)
        continue;

      double xmin = xl_min + 1.0 * charwidth;
      double xmax = xl_max - 1.0 * charwidth;
      double y = yl_max - (2.0 + iLegend * 3) * charheight;

      m_pSGP->moveAbs (xmin, y + 0.5 * charheight);

      m_pSGP->drawText (pstrLegend->c_str());
      m_pSGP->setColor (getColor (iCurve));

      int iLS = getLinestyle (iCurve);
      if (iLS != SGP::LS_NOLINE) {
        m_pSGP->setLineStyle (iLS);
        m_pSGP->moveAbs (xmin, y);
        m_pSGP->lineAbs (xmax, y);
      }

      int iSymbol = getSymbol (iCurve);
      if (iSymbol > 0) {
        double xinc = (xmax - xmin) / (5 - 1);
        m_pSGP->setLineStyle (SGP::LS_SOLID);
        for (int j = 0; j < 5; j++) {
          m_pSGP->moveAbs (xmin + j * xinc, y);
          symbol (iSymbol, symwidth, symheight);
        }
      }
      ++iLegend;        // move to next legend position
    }
  }   // end legend printing

  // calculate the extent of the axes

  /*-------------------------*/
  /* adjust frame for labels */
  /*-------------------------*/

  // X-Label
  if (c_xlabel.length() > 0)
    ya_min += 1.5 * charheight;
  xlbl_row = xp_min;            // put x-label on bottom of plot frame

  // Y-Label
  if (c_ylabel.length() > 0) {

    m_pSGP->setTextAngle (HALFPI);

    m_pSGP->setTextSize (1.5 * charheight);

    double xExtent, yExtent;

    m_pSGP->getTextExtent (c_ylabel.c_str(), &xExtent, &yExtent);

    m_pSGP->setTextSize (charheight);

    xa_min += xExtent;

    m_pSGP->setTextAngle (0.0);

  }
  ylbl_col = xp_min;

  /*------------------------------*/
  /* adjust frame for tick labels */
  /*------------------------------*/

  // Calc offset of tick labels from axes
  if (o_xaxis == NOAXIS || o_xtlabel == FALSE)
    xtl_ofs = 0.0;
  else if (o_xticks == BELOW)
    xtl_ofs = -0.5 * charheight;
  else if (o_xticks == ABOVE)
    xtl_ofs = 0.5 * charheight;

  if (o_yaxis == NOAXIS || o_ytlabel == FALSE)
    ytl_ofs = 0.0;
  else if (o_yticks == LEFT) {
    double xExtentMin, xExtentMax, yExtent;
    char s[1024];
    snprintf (s, sizeof(s), y_numfmt, ymin);
    m_pSGP->getTextExtent (s, &xExtentMin, &yExtent);
    snprintf (s, sizeof(s), y_numfmt, ymax);
    m_pSGP->getTextExtent (s, &xExtentMax, &yExtent);
    if (xExtentMin > xExtentMax)
      xExtentMax = xExtentMin;
    ytl_ofs = -xExtentMax;
  } else if (o_yticks == RIGHT)
    ytl_ofs = 1.5 * charwidth;

  xa_max -= 0.7 * x_fldwid * charwidth; // make room for last x tick label

  xt_min = xa_min;
  yt_min = ya_min;
  xt_max = xa_max;
  yt_max = ya_max;

  // see if need to shrink axis extents and/or tick extents
  if (xtl_ofs != 0.0 && s_ycross == FALSE) {
    if (o_xticks == BELOW) {
      ya_min += 1.5 * charheight;
      yt_min = ya_min;
    } else if (o_xticks == ABOVE) {
      ya_min += 0.0;
      yt_min = ya_min + 1.5 * charheight;
    }
  } else   // noaxis, no t-labels, or user set cross
    yt_min = ya_min;

  if (ytl_ofs != 0.0 && s_xcross == FALSE) {
    if (o_yticks == LEFT) {
      xa_min += 2*charwidth - ytl_ofs; // (2 + y_fldwid) * charwidth;
      xt_min = xa_min;
    } else if (o_yticks == RIGHT) {
      xa_min += 0.0;
      xt_min = xa_min + ytl_ofs; // + y_fldwid * charwidth;
    }
  } else
    xt_min = xa_min;

  // decrease size of graph, if necessary, to accommadate space
  // between axis boundary and boundary of ticks
  double x_added_ticks = 0; // number of tick spaces added to axis
  double y_added_ticks = 0;
  if (o_xaxis == NOAXIS || o_xtlabel == FALSE)
    x_added_ticks = 0;
  if (o_yaxis == NOAXIS || o_ytlabel == FALSE)
    y_added_ticks = 0;

  if (o_grid == TRUE) {
    if (x_added_ticks < 0)
      x_added_ticks = 2;
    if (y_added_ticks < 0)
      y_added_ticks = 2;
  }

  if (x_added_ticks < 0) {
    if (o_yticks == LEFT || s_ycross)
      x_added_ticks = 1;
    else
      x_added_ticks = 2;
  }

  if (y_added_ticks < 0) {
    if (o_xticks == BELOW || s_xcross)
      y_added_ticks = 1;
    else
      y_added_ticks = 2;
  }

  xn_tickinc = (xt_max - xt_min) / (x_nint + x_added_ticks);
  yn_tickinc = (yt_max - yt_min) / (y_nint + y_added_ticks);

  xt_min += 0.5 * x_added_ticks * xn_tickinc;
  xt_max -= 0.5 * x_added_ticks * xn_tickinc;
  yt_min += 0.5 * y_added_ticks * yn_tickinc;
  yt_max -= 0.5 * y_added_ticks * yn_tickinc;

  xgn_min = xt_min;
  xgn_max = xt_max;
  ygn_min = yt_min;
  ygn_max = yt_max;

  //------------------------------------------------------------------------

  m_xWorldScale = (xgn_max - xgn_min) / (xgw_max - xgw_min);
  m_yWorldScale = (ygn_max - ygn_min) / (ygw_max - ygw_min);

  // PLOT CURVES

  m_pSGP->setLineStyle (SGP::LS_SOLID);
  drawAxes();


  double clipRect[4];

  clipRect[0] = xgn_min; clipRect[1] = ygn_min; clipRect[2] = xgn_max; clipRect[3] = ygn_max;



  for (unsigned int iCurve = 0; iCurve < m_vecCurves.size(); iCurve++) {
    const EZPlotCurve* const pCurve = m_vecCurves [iCurve];

    m_pSGP->setColor (getColor (iCurve));
    int iSym = getSymbol (iCurve);
    int iLS = getLinestyle (iCurve);

    if (iLS != SGP::LS_NOLINE) {
      m_pSGP->setLineStyle (iLS);
      double x1 = convertWorldToNDC_X (pCurve->x[0]);
      double y1 = convertWorldToNDC_Y (pCurve->y[0]);

      for (int i = 1; i < pCurve->m_iPointCount; i++) {
        double x2 = convertWorldToNDC_X (pCurve->x[i]);
        double y2 = convertWorldToNDC_Y (pCurve->y[i]);
        double x2Clip = x2;
        double y2Clip = y2;

        if (clip_rect (x1, y1, x2Clip, y2Clip, clipRect)) {
          m_pSGP->moveAbs (x1, y1);
          m_pSGP->lineAbs (x2Clip, y2Clip);
        }
        x1 = x2;
        y1 = y2;
      }
    }
    if (iSym > 0) {
      int iSymFreq = getSymbolFreq (iCurve);
      m_pSGP->setLineStyle (SGP::LS_SOLID);
      double x = convertWorldToNDC_X (pCurve->x[0]);
      double y = convertWorldToNDC_Y (pCurve->y[0]);
      m_pSGP->moveAbs (x, y);
      symbol (iSym, symwidth, symheight);
      for (int i = 1; i < pCurve->m_iPointCount; i++)
        if (i % iSymFreq == 0 || i == pCurve->m_iPointCount - 1) {
          x = convertWorldToNDC_X (pCurve->x[i]);
          y = convertWorldToNDC_Y (pCurve->y[i]);

          if (y >= ygn_min && y <= ygn_max) {
            m_pSGP->moveAbs (x, y);
            symbol (iSym, symwidth, symheight);
          }
        }
    }
  }
}


/* NAME
*   drawAxes                    INTERNAL routine to draw axis & label them
*
* SYNOPSIS
*   drawAxes()
*/


void
EZPlot::drawAxes()
{
  double xticklen = 0, yticklen = 0; // length of ticks in NDC
  double minorinc;              // increment between minor axes
  double xaxispos, yaxispos;    // crossing of axes
  double x, y, x2, y2;
  bool axis_near;       // TRUE if axis too close to print t-label
  int i, j;
  char str[256];
  char *numstr;

  m_pSGP->setTextSize (charheight);
  m_pSGP->setTextColor (1, -1);

  if (o_xticks == ABOVE)
    xticklen = 0.5 * charheight;
  else if (o_xticks == BELOW)
    xticklen = -0.5 * charheight;

  if (o_yticks == RIGHT)
    yticklen = charwidth;
  else if (o_yticks == LEFT)
    yticklen = -charwidth;

  if (c_title.length() > 0) {
    double wText, hText;
    m_pSGP->setTextSize (charheight * 2.0);
    m_pSGP->getTextExtent (c_title.c_str(), &wText, &hText);
    m_pSGP->moveAbs (xa_min + (xa_max-xa_min)/2 - wText/2, title_row);
    m_pSGP->setTextColor (clr_title, -1);
    m_pSGP->drawText (c_title);
    m_pSGP->setTextSize (charheight);
  }

  if (o_grid == TRUE || o_box == TRUE) {
    m_pSGP->setColor (clr_axis);
    m_pSGP->moveAbs (xa_min, ya_min);
    m_pSGP->lineAbs (xa_max, ya_min);
    m_pSGP->lineAbs (xa_max, ya_max);
    m_pSGP->lineAbs (xa_min, ya_max);
    m_pSGP->lineAbs (xa_min, ya_min);
  }

  // calculate position of axes

  // x-axis
  if (s_ycross == TRUE) {       // convert users' world-coord
    xaxispos = convertWorldToNDC_Y (v_ycross);// axis to its position in NDC
    x = convertWorldToNDC_X (xgw_min);
  } else
    xaxispos = ya_min;

  // y-axis
  if (s_xcross == TRUE) {       // convert users' world-coord
    yaxispos = convertWorldToNDC_X (v_xcross);// axis to its NDC position
    y = convertWorldToNDC_Y (ygw_min);
  } else
    yaxispos = xa_min;

  /*-------------*/
  /* draw x-axis */
  /*-------------*/

  if (o_xaxis == LINEAR) {
    // draw axis line

    m_pSGP->setColor (clr_axis);
    if (o_tag && !o_grid && !o_box && s_xcross) {
      m_pSGP->moveAbs (xa_min, xaxispos - charheight);
      m_pSGP->lineAbs (xa_min, xaxispos + charheight);
    }
    m_pSGP->moveAbs (xa_min, xaxispos);
    m_pSGP->lineAbs (xa_max, xaxispos);
    if (o_tag && !o_grid && !o_box) {
      m_pSGP->moveAbs (xa_max, xaxispos - charheight);
      m_pSGP->lineAbs (xa_max, xaxispos + charheight);
    }

    if (o_grid == TRUE) {
      m_pSGP->setColor (clr_grid);
      for (i = 0; i <= x_nint; i++) {
        m_pSGP->moveAbs (xt_min + xn_tickinc * i, ya_max);
        m_pSGP->lineAbs (xt_min + xn_tickinc * i, ya_min);
      }
    }
    m_pSGP->setTextSize (charheight * 1.5);
    m_pSGP->setTextColor (clr_label, -1);

    double wText, hText;

    m_pSGP->getTextExtent (c_xlabel.c_str(), &wText, &hText);
    m_pSGP->moveAbs (xa_min + (xa_max-xa_min)/2 - wText / 2, xlbl_row +  charheight * 1.5);

    m_pSGP->drawText (c_xlabel);
    m_pSGP->setTextSize (charheight);
    minorinc = xn_tickinc / (o_xminortick + 1);

    for (i = 0; i <= x_nint; i++) {
      x = xt_min + xn_tickinc * i;
      m_pSGP->setColor (clr_axis);
      m_pSGP->moveAbs (x, xaxispos);
      m_pSGP->lineAbs (x, xaxispos + xticklen);
      if (i != x_nint)
        for (j = 1; j <= o_xminortick; j++) {
          x2 = x + minorinc * j;
          m_pSGP->moveAbs (x2, xaxispos);
          m_pSGP->lineAbs (x2, xaxispos + TICKRATIO * xticklen);
        }
        axis_near = FALSE;
        if (xaxispos + xtl_ofs > ya_min && o_yaxis != NOAXIS) {
          double xw = xgw_min + i * xw_tickinc;
          double x = convertWorldToNDC_X (xw);
          double d = x - yaxispos;
          if (o_yticks == RIGHT && d >= 0  && d < 0.9 * xn_tickinc)
            axis_near = TRUE;
          if (o_yticks == LEFT && d <= 0 && d > -0.9 * xn_tickinc)
            axis_near = TRUE;
        }

        if (o_xtlabel == TRUE && axis_near == FALSE) {
          snprintf (str, sizeof(str), x_numfmt, xgw_min + xw_tickinc * i);
          numstr = str_skip_head (str, " ");
          double xExtent, yExtent;
          m_pSGP->getTextExtent (numstr, &xExtent, &yExtent);
          m_pSGP->moveAbs (x - xExtent/2, xaxispos + xtl_ofs);
          m_pSGP->setTextColor (clr_number, -1);
          m_pSGP->drawText (numstr);
        }
    }
  }             // X - Axis


  /*--------*/
  /* y-axis */
  /*--------*/

  if (o_yaxis == LINEAR) {

    m_pSGP->setColor (clr_axis);
    if (o_tag && !o_grid && !o_box && s_ycross) {
      m_pSGP->moveAbs (yaxispos - charwidth, ya_min);
      m_pSGP->lineAbs (yaxispos + charwidth, ya_min);
    }
    m_pSGP->moveAbs (yaxispos, ya_min);
    m_pSGP->lineAbs (yaxispos, ya_max);
    if (o_tag && !o_grid && !o_box) {
      m_pSGP->moveAbs (yaxispos - charwidth, ya_max);
      m_pSGP->lineAbs (yaxispos + charwidth, ya_max);
    }

    if (o_grid == TRUE) {
      m_pSGP->setColor (clr_grid);
      for (i = 0; i <= y_nint; i++) {
        y = yt_min + yn_tickinc * i;
        m_pSGP->moveAbs (xa_max, y);
        m_pSGP->lineAbs (xa_min, y);
      }
    }

    m_pSGP->setTextAngle (HALFPI);
    m_pSGP->setTextSize (1.5 * charheight);
    m_pSGP->setTextColor (clr_label, -1);

    double xExtent, yExtent;

    m_pSGP->getTextExtent (c_ylabel.c_str(), &xExtent, &yExtent);
    m_pSGP->moveAbs (ylbl_col, ya_min + (ya_max-ya_min)/2 - yExtent / 2);
    m_pSGP->drawText (c_ylabel);

    m_pSGP->setTextAngle (0.0);
    m_pSGP->setTextSize (charheight);
    minorinc = yn_tickinc / (o_yminortick + 1);

    for (i = 0; i <= y_nint; i++) {
      y = yt_min + yn_tickinc * i;
      m_pSGP->setColor (clr_axis);
      m_pSGP->moveAbs (yaxispos, y);
      m_pSGP->lineAbs (yaxispos + yticklen, y);
      if (i != y_nint)
        for (j = 1; j <= o_yminortick; j++) {
          y2 = y + minorinc * j;
          m_pSGP->moveAbs (yaxispos, y2);
          m_pSGP->lineAbs (yaxispos + TICKRATIO * yticklen, y2);
        }
        axis_near = FALSE;
        if (yaxispos + ytl_ofs > xa_min && o_xaxis != NOAXIS) {
          double yw = ygw_min + i * yw_tickinc;
          double y = convertWorldToNDC_Y (yw);
          double d = y - xaxispos;
          if (o_xticks == ABOVE && d >= 0 && d < 0.9 * yn_tickinc)
            axis_near = TRUE;
          if (o_xticks == BELOW && d <= 0 && d > -0.9 * yn_tickinc)
            axis_near = TRUE;
        }
        if (o_ytlabel == TRUE && axis_near == FALSE) {
          snprintf (str, sizeof(str), y_numfmt, ygw_min + yw_tickinc * i);
          double xExtent, yExtent;
          m_pSGP->getTextExtent (str, &xExtent, &yExtent);
          if (o_yticks == LEFT)
            m_pSGP->moveAbs (yaxispos - 1.5 * charwidth - xExtent, y + 0.5 * yExtent);
          else
            m_pSGP->moveAbs (yaxispos + 1.5 * charwidth, y + 0.5 * yExtent);
          m_pSGP->setTextColor (clr_number, -1);
          m_pSGP->drawText (str);
        }
    }
  }             // Y - Axis
}


void
EZPlot::symbol (int sym, double symwidth, double symheight)
{
  if (sym <= 0)
    return;

  if (sym == SB_CROSS) {
    m_pSGP->markerRel (0, 0);
//    m_pSGP->moveRel (-0.5 * symwidth, -0.5 * symheight);
//    m_pSGP->lineRel (symwidth, symheight);
//    m_pSGP->moveRel (-symwidth, 0.0);
//    m_pSGP->lineRel (symwidth, -symheight);
//    m_pSGP->moveRel (-0.5 * symwidth, 0.5 * symheight);
  } else if (sym == SB_PLUS) {
    m_pSGP->moveRel (-0.5 * symwidth, 0.0);
    m_pSGP->lineRel (symwidth, 0.0);
    m_pSGP->moveRel (-0.5 * symwidth, -0.5 * symheight);
    m_pSGP->lineRel (0.0, symheight);
    m_pSGP->moveRel (0.0, -0.5 * symheight);
  } else if (sym == SB_BOX) {
    m_pSGP->moveRel (-0.5 * symwidth, -0.5 * symheight);
    m_pSGP->lineRel (symwidth, 0.0);
    m_pSGP->lineRel (0.0, symheight);
    m_pSGP->lineRel (-symwidth, 0.0);
    m_pSGP->lineRel (0.0, -symheight);
    m_pSGP->moveRel (0.5 * symwidth, 0.5 * symheight);
  } else if (sym == SB_CIRCLE) {
    m_pSGP->drawCircle (symwidth);
  } else if (sym == SB_ERRORBAR) {
    m_pSGP->moveRel (-0.5 * symwidth, 0.5 * symheight);
    m_pSGP->lineRel (symwidth, 0.0);
    m_pSGP->moveRel (-0.5 * symwidth, 0.0);
    m_pSGP->lineRel (0.0, -symheight);
    m_pSGP->moveRel (-0.5 * symwidth, 0.0);
    m_pSGP->lineRel (symwidth, 0.0);
    m_pSGP->moveRel (-0.5 * symwidth, 0.5 * symheight);
  } else if (sym == SB_POINT) {
    m_pSGP->pointRel (0, 0);
  }
}



/* NAME
*    axis_scale                 calculates graph axis scaling
*
*  SYNOPSIS:
*    retval = axis_scale (min, max, nint, minp, maxp, nintp,
*                          rec_total, rec_frac)
*
*    INPUT:
*       double min         Smallest value to plot
*       double max         Largest value to plot
*       int    nint        Number of intervals desired
*
*    OUTPUT:
*       int   retval       FALSE if illegal parameters, else TRUE
*       double *minp       Minimum graph value
*       double *maxp       Maximum graph value
*       int    *nintp      Number of intervals for graph
*      int    *rec_total  Recommended field width for printing out the number
*       int    *rec_frac   Recommended number of digits for print fraction
*/

int
EZPlot::axis_scale (double min, double max, int nint, double *minp, double *maxp, int *nintp)
{
  if (nint < 1) {
    sys_error (ERR_WARNING, "No intervals to plot: num intervals=%d [axis_scale]", nint);
    return (FALSE);
  }
  if (min >= max) {
    double scaled = fabs(max) / 10;
    if (scaled == 0)
      scaled = 0.1;
    *minp = min - scaled;
    *maxp = max + scaled;
    *nintp = 2;
    return (TRUE);
  }

  double eps = 0.025;
  double a = fabs(min);
  if (fabs(min) < fabs(max))
    a = fabs(max);
  double scale = pow (10.0, floor(log10(a)));
loop:
  double mina = min / scale;
  double maxa = max / scale;
  double d = (maxa - mina) / nint;
  double j = d * eps;
  double e = floor (log10(d));
  double f = d / pow (10.0, e);
  double v = 10.0;
  if (f < sqrt(2.0))
    v = 1.0;
  else if (f < sqrt (10.0))
    v = 2.0;
  else if (f < sqrt (50.0))
    v = 5.0;
  double wdt = v * pow (10.0, e);
  double g = floor (mina / wdt);
  if (fabs(g + 1 - mina / wdt) < j)
    g = g + 1;
#undef TEST1

#ifdef TEST1
  g++;
#endif
  *minp = wdt * g;
  double h = floor (maxa / wdt) + 1.0;
  if (fabs(maxa / wdt + 1 - h) < j)
    h = h - 1;
#ifdef TEST1
  h--;
#endif
  *maxp = wdt * h;
  *nintp = static_cast<int>(h - g);
  if (fabs(*maxp) >= 10.0 || fabs(*minp) >= 10.0) {
    scale = scale * 10.0;
    goto loop;
  }

  *minp *= scale;
  *maxp *= scale;

  return (TRUE);
}


/* NAME
*   make_numfmt         Make a numeric format string
*
* SYNOPSIS
*   make_numfmt (fmtstr, fldwid, nfrac, min, max, nint)
*   char *fmtstr                Returned format string for printf()
*   int  *fldwid                Returned field width
*   int  *nfrac         If < 0, then calculate best number of
*                               fraction places & return that value
*                               If >= 0, then use that number of places
*   double min                  Minimum value
*   double max                  Maximum value
*   int nint                    Number of intervals between min & max
*
* DESCRIPTION
*   This  routine is written as an INTERNAL routine for EZPLOT
*/

static inline double
my_trunc (double x)
{
  double integer;

  modf (x, &integer);

  return (integer);
}

void
EZPlot::make_numfmt (char *fmtstr, int *fldwid, int *nfrac, double minval, double maxval, int nint)
{
  int wid, frac, expon;

  double delta = (maxval - minval) / nint;
  double absmin = fabs(minval);
  double absmax = fabs(maxval);

  if (absmin > absmax)

    absmax = absmin;
  double logt = log10( absmax );

  if (fabs(logt) >= 6) {                // use exponential format
    if (fabs(logt) > 99)
      expon = 5;                //  E+102
    else
      expon = 4;                //  E+00

    if (*nfrac < 0) {           // calculate frac
      delta /= pow (10., floor(logt));  // scale delta
      frac = static_cast<int>(fabs(my_trunc(log10(delta)))) + 1;
      if (frac < 1)
        frac = 1;               // to be safe, add decimal pt
    } else                      // use users' frac
      frac = *nfrac;

    wid = 2 + frac + expon;
    if (minval < 0. || maxval < 0.)
      ++wid;
    sprintf (fmtstr, "%s%d%s%d%s", "%", wid, ".", frac, "g");
  } else {                      // use fixed format
    wid = static_cast<int>(my_trunc(logt)) + 1;
    if (wid < 1)
      wid = 1;
    if (minval < 0. || maxval < 0.)
      ++wid;

    if (*nfrac < 0) {           // calculate frac
      if (delta >= 0.999999)
        frac = 1;               // add a decimal pt to be safe
      else
        frac = static_cast<int>(fabs(my_trunc(log10(delta)))) + 1;
    } else                      // use users' frac
      frac = *nfrac;

    wid += 1 + frac;
    sprintf (fmtstr, "%s%d%s%d%s", "%", wid, ".", frac, "f");
  }

  *fldwid = wid;
  *nfrac = frac;
}

