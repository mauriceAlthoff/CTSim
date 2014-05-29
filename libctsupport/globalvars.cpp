/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         globalvars.cpp
**      Purpose:      Global variables
**      Programmer:   Kevin Rosenberg
**      Date Started: Jan 2001
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

#include "ct.h"

#ifdef HAVE_WXWINDOWS

#include "../src/ctsim.h"

bool g_bRunningWXWindows = false;
CTSimApp* theApp = NULL;

#endif

