/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         trace.h
**      Purpose:      Header file for Trace class
**      Author:       Kevin Rosenberg
**      Date Started: Oct 2000
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

#ifndef TRACE_H
#define TRACE_H


class Trace
{
 public:
    static const int TRACE_INVALID;
    static const int TRACE_NONE;
    static const int TRACE_CONSOLE;
    static const int TRACE_PHANTOM;
    static const int TRACE_PROJECTIONS;
    static const int TRACE_PLOT;
    static const int TRACE_CLIPPING;

    static const int BIT_CONSOLE;
    static const int BIT_PHANTOM;
    static const int BIT_PROJECTIONS;
    static const int BIT_PLOT;
    static const int BIT_CLIPPING;

  Trace (const char* const traceString);

  void addTrace (const char* const traceString);

  bool isTrace (const char* const traceQuery) const;

  int getTraceLevel(void) const { return m_traceLevel; }

  static int convertTraceNameToID (const char* traceName);
  static const char* convertTraceIDToTitle (int idTrace);
  static const char* convertTraceIDToName (int idTrace);

  static const int getTraceCount() {return s_iTraceCount;}
  static const char** getTraceNameArray() {return s_aszTraceName;}
  static const char** getTraceTitleArray() {return s_aszTraceTitle;}

 private:

  int m_traceLevel;

  bool addTraceElements (const char* const traceString);

  static const char* s_aszTraceName[];
  static const char* s_aszTraceTitle[];
  static const int s_iTraceCount;

};


#endif

