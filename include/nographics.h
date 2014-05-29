/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          nographics.h
**   Purpose:       Headers for empty duplicate of graphics functions
**   Programmer:    Kevin Rosenberg
**   Date Started:  Mar 2001
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

#ifndef __NOGRAPHICS_H_
#define __NOGRAPHICS_H_

#include <cstdio>

#ifdef HAVE_WXWINDOWS
class EZPlot;
class wxWindow;
class wxEZPlotDialog;

class EZPlotDialog
{
private:
  wxEZPlotDialog* m_pDialog;

public:
  EZPlotDialog (wxWindow *parent = NULL, bool bCancelButton = false);

  EZPlot* getEZPlot ();
  int ShowModal();
};
#endif

#endif

