/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:         trace.cpp        Class for trace
**   Programmer:   Kevin Rosenberg
**   Date Started: June 2000
**
**  This is part of the CTSim program
**  Copyright (C) 1983-2009 Kevin Rosenberg
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

const int Trace::TRACE_INVALID = -1;
const int Trace::TRACE_NONE = 0;
const int Trace::TRACE_CONSOLE = 1;
const int Trace::TRACE_PHANTOM = 2;
const int Trace::TRACE_PROJECTIONS = 3;
const int Trace::TRACE_PLOT = 4;
const int Trace::TRACE_CLIPPING = 5;

const int Trace::BIT_CONSOLE = 0x0001;
const int Trace::BIT_PHANTOM = 0x0002;
const int Trace::BIT_PROJECTIONS = 0x0004;
const int Trace::BIT_PLOT = 0x0008;
const int Trace::BIT_CLIPPING = 0x0010;

const char* Trace::s_aszTraceName[] =
{
  "none",
  "console",
  "phantom",
  "proj",
  "plot",
  "clipping",
};

const char* Trace::s_aszTraceTitle[] =
{
  "None",
  "Console",
  "Phantom",
  "Projections",
  "Plot",
  "Clipping",
};

const int Trace::s_iTraceCount = sizeof(s_aszTraceName) / sizeof(const char*);


const char*
Trace::convertTraceIDToName (const int idTrace)
{
  const char *name = "";

  if (idTrace >= 0 && idTrace < s_iTraceCount)
      return (s_aszTraceName[idTrace]);

  return (name);
}

const char*
Trace::convertTraceIDToTitle (const int idTrace)
{
  const char *title = "";

  if (idTrace >= 0 && idTrace < s_iTraceCount)
      return (s_aszTraceName[idTrace]);

  return (title);
}

int
Trace::convertTraceNameToID (const char* const traceName)
{
  int id = Trace::TRACE_INVALID;

  for (int i = 0; i < s_iTraceCount; i++)
      if (strcasecmp (traceName, s_aszTraceName[i]) == 0) {
          id = i;
          break;
      }

  return (id);
}

