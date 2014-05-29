/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          nographics.cpp
**   Purpose:       Empty functions for duplicates of graphcal functions
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nographics.h"


#ifdef HAVE_WXWINDOWS
EZPlotDialog::EZPlotDialog (wxWindow *parent, bool bCancelButton)
{}

EZPlot*
EZPlotDialog::getEZPlot()
{ return NULL; }

int
EZPlotDialog::ShowModal()
{ return 0; }

#endif
