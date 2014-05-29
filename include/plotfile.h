/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         plotfile.h
**      Purpose:      PlotFile class header
**      Programmer:   Kevin Rosenberg
**      Date Started: Dec 2000
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

#ifndef PLOTFILE_H
#define PLOTFILE_H

#ifndef MSVC
#include <unistd.h>
#endif

#include <sys/types.h>
#include <cstring>
#include <string>
#include <iosfwd>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>
#include "ctsupport.h"
#include "plotfile.h"


// Plotfile structure:
// 1. Lines that begin with # are comments
// 2. ASCII file format
// 3. Header lines begin with <tags> and end with </tags>
// 4. Valid headers
//      <plotfile ncolumns nrecords>    (signifies beginning of plotfile)
//      <description> (beginning of description lines)
//      <ezset>       (signifies beginning of ezset commands)
//      <columns>     Beginning of data columns
// 5. Data is ASCII file format, one record per line
//    Number of columns is variable and is set by ncolumns header



class PlotFile
{
private:
  std::string m_strFilename;
  std::string m_strDate;
  std::vector<std::string> m_vecStrDescriptions;
  std::vector<std::string> m_vecStrEzsetCommands;
  std::vector<double> m_vecCurves;
  int m_iNumColumns;
  int m_iNumRecords;
  bool m_bScatterPlot;

  bool headerRead (std::iostream& os);
  bool headerWrite (std::iostream& os);
  bool columnsRead (std::iostream& os);
  bool columnsWrite (std::iostream& os);

  void initHeaders ();

  PlotFile (const PlotFile& rhs);        // copy constructor
  PlotFile& operator= (const PlotFile&); // assignment operator

public:
  PlotFile (int iNColumns, int iNRecords);
  PlotFile (void);
  ~PlotFile ();

  void setCurveSize (int iNCurves, int iNRecords, bool bScatterPlot = false);

  void addDescription (const char* const pszDesc)
  { m_vecStrDescriptions.push_back (pszDesc); }

  void addEzsetCommand (const char* const pszCmd)
  { m_vecStrEzsetCommands.push_back (pszCmd); }

  bool addColumn (int iCol, const double* const pdColumn);
  bool addColumn (int iCol, const float* const pdColumn);
  void getColumn (int iCol, double *pdColumnData) const;

  const std::string& getDate () const
  { return m_strDate; }

  int getNumColumns () const
  { return m_iNumColumns; }

  int getNumRecords () const
  { return m_iNumRecords; }

  bool getIsScatterPlot() const
  { return m_bScatterPlot; }

  bool getMinMax (int startingCol, double& min, double& max) const;

  bool statistics (int startingCol, double& min, double& max, double& mean, double& mode, double& median, double &stddev) const;

  unsigned int getNumDescriptions (void) const
  { return m_vecStrDescriptions.size(); }

  const std::string& getDescription (int iDescIndex) const
  { return m_vecStrDescriptions[iDescIndex]; }

  unsigned int getNumEzsetCommands (void) const
  { return m_vecStrEzsetCommands.size(); }

  const std::string& getEzsetCommand (int iIndex) const
  { return m_vecStrEzsetCommands[iIndex]; }

  bool fileRead (const char* const filename);

  bool fileWrite (const char* const filename);

  const std::string& getFilename (void) const
  {  return m_strFilename; }

  void printHeaders (std::ostream& os) const;
  void printHeaders (std::ostringstream& os) const;
  void printHeadersBrief (std::ostream& os) const;
  void printHeadersBrief (std::ostringstream& os) const;
};


#endif
