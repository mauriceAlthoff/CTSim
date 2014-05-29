/*****************************************************************************
**  FILE IDENTIFICATION
**
**      EZSET - Parameter control for EZPLOT
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

#include "ctsupport.h"
#include "ezplot.h"
#include "pol.h"


bool
EZPlot::ezset (const std::string& command)
{
  return ezset (command.c_str());
}

bool
EZPlot::ezset (const char* const command)
{

    return ezcmd (command);
}

bool
EZPlot::ezcmd (const char* const comm)
{
  m_pol.usefile (POL::P_USE_STR, "");
  m_pol.set_inputline (comm);

  char str [POL::MAXTOK+1];
  int code;
  bool retval = true;
  if (! m_pol.readUserToken (str, &code)) {
    sys_error (ERR_WARNING, "Illegal EZSET command: %s", str);
    m_pol.reader();
    retval = false;
  }
  else
    retval = do_cmd (code);

  m_pol.closefile();                    /* close input string file */
  return (retval);
}


bool
EZPlot::do_cmd (int lx)
{
  char str [1024];
  char strIn [1024];
  int n;
  double f;

  switch (lx) {
  case S_TEXTSIZE:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      if (f >= 0.0 && f <= 1.0) {
        v_textsize = f;
        s_textsize = TRUE;
      } else
        s_textsize = FALSE;
    }
    break;
  case S_REPLOT:
    plot (m_pSGP);
    break;
  case S_CLEAR:
    clearCurves ();
    break;
  case S_TITLE:
    m_pol.readText (strIn, sizeof(strIn));
    c_title = strIn;
    break;
  case S_LEGEND:
    m_pol.readText (strIn, sizeof(strIn));
    if (m_iCurrentCurve >= 0)
      setLegend (m_iCurrentCurve, strIn);
    break;
  case S_XLABEL:
    m_pol.readText (strIn, sizeof(strIn));
    c_xlabel = strIn;
    break;
  case S_YLABEL:
    m_pol.readText (strIn, sizeof(strIn));
    c_ylabel = strIn;
    break;
  case S_XCROSS:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      v_xcross = f;
      s_xcross = TRUE;
    } else
      s_xcross = FALSE;
    break;
  case S_YCROSS:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      v_ycross = f;
      s_ycross = TRUE;
    } else
      s_ycross = FALSE;
    break;
  case S_NOXAXIS:
    o_xaxis = NOAXIS;
    break;
  case S_NOYAXIS:
    o_yaxis = NOAXIS;
    break;
  case S_XLIN:
    o_xaxis = LINEAR;
    break;
  case S_YLIN:
    o_yaxis = LINEAR;
    break;
  case S_XLOG:
    o_xaxis = LOG;
    break;
  case S_YLOG:
    o_yaxis = LOG;
    break;
  case S_XAUTOSCALE:
    s_xmin = FALSE;
    s_xmax = FALSE;
    break;
  case S_YAUTOSCALE:
    s_ymin = FALSE;
    s_ymax = FALSE;
    break;
  case S_XMIN:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      v_xmin = f;
      s_xmin = TRUE;
    }
    break;
  case S_XMAX:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      v_xmax = f;
      s_xmax = TRUE;
    }
    break;
  case S_YMIN:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      v_ymin = f;
      s_ymin = TRUE;
    }
    break;
  case S_YMAX:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE) {
      v_ymax = f;
      s_ymax = TRUE;
    }
    break;
  case S_SOLID:
    o_linestyle = SGP::LS_SOLID;
    break;
  case S_DASH:
    int ls;
    ls = SGP::LS_DASH1;
    if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE) {
      if (n == 1)
        ls = SGP::LS_DASH1;
      else if (n == 2)
        ls = SGP::LS_DASH2;
      else if (n == 3)
        ls = SGP::LS_DASH3;
      else if (n == 4)
        ls = SGP::LS_DASH4;
      else if (n == 5)
        ls = SGP::LS_DOTTED;
    }
    if (m_iCurrentCurve < 0)
      o_linestyle = ls;
    else
      setLinestyle (m_iCurrentCurve, ls);
    break;
  case S_NOLINE:
    o_linestyle = SGP::LS_NOLINE;
    break;
  case S_PEN:
  case S_COLOR:
    if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE)
    {
      if (n >= 0) {
        if (m_iCurrentCurve < 0)
          o_color = n;
        else
          setColor (m_iCurrentCurve, n);
      } else
        bad_option("The color you picked");
    }
    break;
  case S_BOX:
    o_box = TRUE;
    break;
  case S_NOBOX:
    o_box = FALSE;
    break;
  case S_GRID:
    o_grid = TRUE;
    break;
  case S_NOGRID:
    o_grid = FALSE;
    break;
  case S_XLENGTH:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE)
      if (f > 0.0 && f <= 1.0)
        o_xlength = f;
      break;
  case S_YLENGTH:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE)
      if (f > 0.0 && f <= 1.0)
        o_ylength = f;
      break;
  case S_XPORIGIN:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE)
      if (f >= 0.0 && f < 1.0)
        o_xporigin = f;
      break;
  case S_YPORIGIN:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE)
      if (f >= 0.0 && f < 1.0)
        o_yporigin = f;
      break;
  case S_TAG:
    if (m_pol.readWord("no", 2) == TRUE)
      o_tag = FALSE;
    else if (m_pol.readWord("off", 2) == TRUE)
      o_tag = FALSE;
    else
      o_tag = TRUE;
    break;
  case S_LEGENDBOX:
    if (m_pol.readWord("inside", 2) == TRUE)
      o_legendbox = INSIDE;
    else if (m_pol.readWord("outside", 3) == TRUE)
      o_legendbox = OUTSIDE;
    else if (m_pol.readWord("none",2) == TRUE)
      o_legendbox = NOLEGEND;
    else {
      m_pol.readText (str, POL::MAXTOK);
      bad_option(str);
    }
    break;
  case S_XLEGEND:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE)
    {
      if (f >= 0.0 && f < 1.0) {
        v_xlegend = f;
        s_xlegend = TRUE;
      }
      else
        s_xlegend = FALSE;
    }
    break;
  case S_YLEGEND:
    if (m_pol.readFloat (&f, POL::TT_REAL, FALSE, 0.0, 0.0) == TRUE)
    {
      if (f >= 0.0 && f < 1.0) {
        v_ylegend = f;
        s_ylegend = TRUE;
      }
      else
        s_ylegend = FALSE;
    }
    break;
  case S_SYMBOL:
    if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE) {
      if (n > 0 && n <= MAXSYMBOL) {
        if (m_iCurrentCurve < 0)
          o_symbol = n;
        else
          setSymbol (m_iCurrentCurve, n);
      }
    } else {
      if (m_pol.readWord("every",5) == TRUE) {
        if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE) {
          int sym = 1;
          if (n > 0)
            sym = n;
          if (m_iCurrentCurve < 0)
            o_symfreq = sym;
          else
            setSymbolFreq (m_iCurrentCurve, sym);
        }
      } else if (m_pol.readWord ("none",4) == TRUE) {
        o_symbol = -1;
      }
    }
    break;
  case S_CURVE:
    if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE) {
      if (n > 0)
        m_iCurrentCurve = n - 1;
    } else {
      if (m_pol.readWord ("all",3) == TRUE)
        m_iCurrentCurve = -1;
    }
    break;
  case S_XTICKS:
    if (m_pol.readUserToken(str,&lx) == FALSE)
      break;
    if (lx == S_ABOVE)
      o_xticks = ABOVE;
    else if (lx == S_BELOW)
      o_xticks = BELOW;
    else if (lx == S_NOLABEL)
      o_xtlabel = FALSE;
    else if (lx == S_LABEL)
      o_xtlabel = TRUE;
    else if (lx == S_MAJOR) {
      if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE)
        if (n > 1 && n < 100)
          o_xmajortick = n;
    } else if (lx == S_MINOR)
      if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE)
        if (n >= 0 && n < 100)
          o_xminortick = n;
        break;
  case S_YTICKS:
    if (m_pol.readUserToken(str,&lx) == FALSE)
      break;
    if (lx == S_RIGHT)
      o_yticks = RIGHT;
    else if (lx == S_LEFT)
      o_yticks = LEFT;
    else if (lx == S_NOLABEL)
      o_ytlabel = FALSE;
    else if (lx == S_LABEL)
      o_ytlabel = TRUE;
    else if (lx == S_MAJOR) {
      if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE)
        if (n > 1 && n < 100)
          o_ymajortick = n;
    } else if (lx == S_MINOR)
      if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE)
        if (n >= 0 && n < 100)
          o_yminortick = n;
        break;
  case S_LXFRAC:
    if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE) {
      if (n >= 0) {
        v_lxfrac = n;
        s_lxfrac = TRUE;
      }
    } else
      s_lxfrac = FALSE;
    break;
  case S_LYFRAC:
    if (m_pol.readInteger (&n, POL::TT_REAL, FALSE, 0, 0) == TRUE) {
      if (n >= 0) {
        v_lyfrac = n;
        s_lyfrac = TRUE;
      }
    } else
      s_lyfrac = FALSE;
    break;
    break;
  default:
    fprintf (stderr, "Unimplemented EZPLOT command\n");
    break;
  }

  m_pol.reader ();
  return (true);
}



void
EZPlot::bad_option (const char *opt)
{
  sys_error (ERR_WARNING, "INVALID option: %s", opt);
}


//----------------------------------------------------------------------
//                      KEYWORDS / CODES TABLE
//----------------------------------------------------------------------
const struct KeywordCodeTable EZPlot::m_sKeywords[] =
{
  {"solid",     S_SOLID},
  {"dash", S_DASH},
  {"curve", S_CURVE},
  {"noline",    S_NOLINE},
  {"black",     S_BLACK},
  {"red",               S_RED},
  {"blue",              S_BLUE},
  {"green",     S_GREEN},
  {"pen",               S_PEN},
  {"symbol",    S_SYMBOL},
  {"every",     S_EVERY},
  {"none",              S_NONE},
  {"legend",    S_LEGEND},
  {"xlegend",   S_XLEGEND},
  {"ylegend",   S_YLEGEND},

  {"xlin",              S_XLIN},
  {"ylin",              S_YLIN},
  {"xlog",              S_XLOG},
  {"ylog",              S_YLOG},
  {"xlabel",    S_XLABEL},
  {"ylabel",    S_YLABEL},
  {"xlength",   S_XLENGTH},
  {"ylength",   S_YLENGTH},

  {"xticks",    S_XTICKS},
  {"yticks",    S_YTICKS},
  {"above",     S_ABOVE},
  {"label",     S_LABEL},
  {"below",     S_BELOW},
  {"nolabel",   S_NOLABEL},
  {"right",     S_RIGHT},
  {"left",              S_LEFT},

  {"xautoscale",        S_XAUTOSCALE},
  {"yautoscale",        S_YAUTOSCALE},
  {"xmin",              S_XMIN},
  {"ymin",              S_YMIN},
  {"xmax",              S_XMAX},
  {"ymax",              S_YMAX},
  {"lxfrac",    S_LXFRAC},
  {"lyfrac",    S_LYFRAC},
  {"xcross",    S_XCROSS},
  {"ycross",    S_YCROSS},
  {"noxaxis",   S_NOXAXIS},
  {"noyaxis",   S_NOYAXIS},
  {"xporigin",  S_XPORIGIN},
  {"yporigin",  S_YPORIGIN},
  {"title",     S_TITLE},
  {"xtitle",    S_XTITLE},
  {"ytitle",    S_YTITLE},

  {"replot",    S_REPLOT},
  {"clear",     S_CLEAR},
  {"store",     S_STORE},
  {"restore",   S_RESTORE},
  {"amark",     S_AMARK},
  {"units",     S_UNITS},
  {"inches",    S_INCHES},
  {"user",              S_USER},

  {"data",              S_DATA},
  {"help",              S_HELP},
  {"exit",              S_EXIT},

  {"box",               S_BOX},
  {"nobox",     S_NOBOX},
  {"grid",              S_GRID},
  {"nogrid",    S_NOGRID},
  {"major",     S_MAJOR},
  {"minor",     S_MINOR},
  {"color",     S_COLOR},
  {"legendbox", S_LEGENDBOX},

  {"no",                S_NO},

  {"textsize",  S_TEXTSIZE},
};

const int EZPlot::NKEYS = (sizeof(EZPlot::m_sKeywords) / sizeof (struct KeywordCodeTable));

void
EZPlot::initKeywords ()
{
  for (int i = 0; i < NKEYS; i++)
    m_pol.addKeyword (m_sKeywords[i].keyword, m_sKeywords [i].code);
}
