/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         plotfile.cpp
**      Purpose:      plotfile class
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

#include "ct.h"
#include <ctime>


///////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//
//     Name: PlotFile
//     Purpose: Plot File storage
///////////////////////////////////////////////////////////////////////////


PlotFile::PlotFile (int nCurves, int nRecords)
{
  initHeaders ();
  setCurveSize (nCurves, nRecords);
}

PlotFile::PlotFile ()
{
  initHeaders ();
}

PlotFile::~PlotFile ()
{
}

void
PlotFile::initHeaders ()
{
  m_iNumColumns = 0;
  m_iNumRecords = 0;
  time_t currentTime = time (NULL);
  m_strDate = ctime (&currentTime);
  m_vecStrDescriptions.clear();
  m_vecStrEzsetCommands.clear();
}

void
PlotFile::setCurveSize (int nCols, int nRecords, bool bScatterPlot)
{
  m_iNumColumns = nCols;
  m_iNumRecords = nRecords;
  m_bScatterPlot = bScatterPlot;
  m_vecCurves.clear();
  m_vecCurves.reserve (m_iNumColumns * m_iNumRecords);
}

// Storage format
//   a Column's records are stored sequentially. It begins at iCol * m_iNumRecords
bool
PlotFile::addColumn (int iCol, const double* const pdColData)
{
  if (iCol < 0 || iCol >= m_iNumColumns) {
    sys_error (ERR_SEVERE, "Illegal column number %d [PlotFile::addColumn]", iCol);
    return (false);
  }

  for (int iRec = 0; iRec < m_iNumRecords; iRec++) {
    m_vecCurves[ iRec + (iCol * m_iNumRecords) ] = pdColData [iRec];
#if 0
    sys_error (ERR_TRACE, "Storing m_vecCurves[%d] = %f",
               iRec + (iCol * m_iNumRecords),
               pdColData [iRec]);
#endif
  }
  return true;
}

bool
PlotFile::addColumn (int iCol, const float* const pdColData)
{
  if (iCol < 0 || iCol >= m_iNumColumns) {
    sys_error (ERR_SEVERE, "Illegal column number %d [PlotFile::addColumn]", iCol);
    return (false);
  }

  for (int iRec = 0; iRec < m_iNumRecords; iRec++)
    m_vecCurves[ iRec + (iCol * m_iNumRecords) ] = pdColData [iRec];

  return true;
}

void
PlotFile::getColumn (int iCol, double* pdColData) const
{
  if (iCol < 0 || iCol >= m_iNumColumns) {
    sys_error (ERR_SEVERE, "Illegal column number %d [PlotFile::addColumn]", iCol);
    return;
  }

  for (int iRec = 0; iRec < m_iNumRecords; iRec++)
    pdColData[iRec] = m_vecCurves[ iRec + (iCol * m_iNumRecords) ];

}

bool
PlotFile::getMinMax (int iStartingCol, double& dMin, double& dMax) const
{
  if (iStartingCol >= m_iNumColumns) {
    sys_error (ERR_WARNING, "iStartingCol >= iNumColumns");
    return false;
  }

  int iOffset = iStartingCol * m_iNumRecords;
  dMin = m_vecCurves[ 0 + iOffset ];
  dMax = dMin;

  for (int iCol = iStartingCol; iCol < m_iNumColumns; iCol++) {
    int iOffset = iCol * m_iNumRecords;
    for (int iRec = 0; iRec < m_iNumRecords; iRec++) {
      double dVal = m_vecCurves[ iRec + iOffset ];
      if (dVal < dMin)
        dMin = dVal;
      else if (dVal > dMax)
        dMax = dVal;
    }
  }

  //  sys_error (ERR_TRACE, "dMin=%f, dMax=%f", dMin, dMax);
  return true;
}

bool
PlotFile::statistics (int iStartingCol, double& min, double& max, double& mean, double& mode, double& median, double &stddev) const
{
  if (iStartingCol >= m_iNumColumns) {
    sys_error (ERR_WARNING, "iStartingCol >= iNumColumns");
    return false;
  }

  int iNPoints = (m_iNumColumns - iStartingCol) * m_iNumRecords;
  std::vector<double> vec;
  vec.resize (iNPoints);

  int iVec = 0;
  for (int iCol = iStartingCol; iCol < m_iNumColumns; iCol++) {
    int iOffset = iCol * m_iNumRecords;
    for (int iRec = 0; iRec < m_iNumRecords; iRec++)
      vec[iVec++] = m_vecCurves[ iRec + iOffset ];
  }

  vectorNumericStatistics (vec, iNPoints, min, max, mean, mode, median, stddev);

  return true;
}

bool
PlotFile::fileWrite (const char* const filename)
{
  m_strFilename = filename;

  fstream fs (m_strFilename.c_str(), std::ios::out | std::ios::trunc);
  if (fs.fail()) {
    sys_error (ERR_WARNING, "Error opening file %s for writing [fileCreate]", m_strFilename.c_str());
    return false;
  }

  if (! headerWrite(fs) || ! columnsWrite (fs))
    return false;

  return true;
}

bool
PlotFile::fileRead (const char* const filename)
{
  m_strFilename = filename;

#ifdef MSVC
  fstream fs (m_strFilename.c_str(), std::ios::in);
#else
  fstream fs (m_strFilename.c_str(), std::ios::in); // | std::ios::nocreate);
#endif

  if (fs.fail()) {
    sys_error (ERR_WARNING, "Unable to open file %s [fileRead]", m_strFilename.c_str());
    return false;
  }

  if (! headerRead(fs))
    return false;

  setCurveSize (m_iNumColumns, m_iNumRecords);

  if (! columnsRead(fs))
    return false;;

  return true;
}

bool
PlotFile::headerRead (std::iostream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to read header with file closed [headerRead]");
    return false;
  }

  initHeaders();
  fs.seekg (0);
  bool bFinishedHeaders = false;

  fs >> m_iNumColumns;
  fs >> m_iNumRecords;

  if (fs.fail() || m_iNumColumns == 0 || m_iNumRecords == 0)
    return false;

  while (! bFinishedHeaders && ! fs.eof() && ! fs.fail()) {
    char line[1024];
    fs.getline (line, sizeof(line));
    int iSP = 0;
    while (line[iSP] == ' ')
      iSP++;
    if (line[iSP] == '\n' || ! line[iSP])
      ;
    else if (line[iSP] == '#') {
      iSP++;
      while (line[iSP] == ' ')
        iSP++;
      if (line[iSP] == '\n' || ! line[iSP])
        ;
      else
        addDescription (&line[iSP]);
    } else if (strstr (&line[iSP], "<datapoints>") != NULL) {
         bFinishedHeaders = true;
    } else
      addEzsetCommand (&line[iSP]);
  }

  return ! fs.fail();
}


bool
PlotFile::headerWrite (std::iostream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to write header with ! fs");
    return false;
  }

  fs.seekp (0);
  fs << m_iNumColumns << " " << m_iNumRecords << "\n";

  unsigned int i;
  for (i = 0; i < m_vecStrEzsetCommands.size(); i++)
      fs << m_vecStrEzsetCommands[i] << "\n";

  for (i = 0; i < m_vecStrDescriptions.size(); i++)
      fs << "# " << m_vecStrDescriptions[i] << "\n";

  if (! m_strDate.empty())
    fs << "# Date: " << m_strDate << "\n";

  return ! fs.fail();
}


bool
PlotFile::columnsWrite (std::iostream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to columnWrite with !fs");
    return false;
  }

  fs << "<datapoints>\n";

  int iStride = m_iNumRecords;
  for (int iRec = 0; iRec < m_iNumRecords; iRec++) {
    for (int iCol = 0; iCol < m_iNumColumns; iCol++)
      fs << m_vecCurves [iRec + (iCol * iStride)] << " ";
    fs << "\n";
  }

  fs << "</datapoints>\n";

  fs << "</plotfile>\n";

  return ! fs.fail();
}


bool
PlotFile::columnsRead (std::iostream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to arrayDataRead with ! fs");
    return false;
  }

  if (m_iNumColumns == 0 || m_iNumRecords == 0) {
    sys_error (ERR_WARNING, "Called PlotFile::columnsRead with 0 columns or records");
    return false;
  }

  bool bTerminateEarly = false;
  for (int iRec = 0; iRec < m_iNumRecords; iRec++) {
    for (int iCol = 0; iCol < m_iNumColumns; iCol++) {
      if (fs.eof()) {
        bTerminateEarly = true;
        break;
      }
      if (fs.fail())
        break;
      double d;
      fs >> d;
      m_vecCurves[ iRec + (iCol * m_iNumRecords) ] = d;
    }
  }

  return ! (bTerminateEarly || fs.fail());
}


void
PlotFile::printHeaders (std::ostream& os) const
{
  os << "EZSet Commands\n";
  for (unsigned int iEZ = 0; iEZ < m_vecStrEzsetCommands.size(); iEZ++)
    os << m_vecStrEzsetCommands[iEZ] << "\n";

  os << "Descriptions\n";
  for (unsigned int iDesc = 0; iDesc < m_vecStrDescriptions.size(); iDesc++)
    os << m_vecStrDescriptions[iDesc] << "\n";
}

void
PlotFile::printHeaders (std::ostringstream& os) const
{
  os << "EZSet Commands\n";
  for (unsigned int iEZ = 0; iEZ < m_vecStrEzsetCommands.size(); iEZ++)
    os << m_vecStrEzsetCommands[iEZ] << "\n";

  os << "Descriptions\n";
  for (unsigned int iDesc = 0; iDesc < m_vecStrDescriptions.size(); iDesc++)
    os << m_vecStrDescriptions[iDesc] << "\n";
}

void
PlotFile::printHeadersBrief (std::ostream& os) const
{
  os << "EZSet Commands\n";
  for (unsigned int iEZ = 0; iEZ < m_vecStrEzsetCommands.size(); iEZ++)
    os << m_vecStrEzsetCommands[iEZ] << "; ";
  if (m_vecStrEzsetCommands.size() > 0)
    os << "\n";

  os << "Descriptions\n";
  for (unsigned int iDesc = 0; iDesc < m_vecStrDescriptions.size(); iDesc++)
    os << m_vecStrDescriptions[iDesc] << "\n";
}

void
PlotFile::printHeadersBrief (std::ostringstream& os) const
{
  os << "EZSet Commands\n";
  for (unsigned int iEZ = 0; iEZ < m_vecStrEzsetCommands.size(); iEZ++)
    os << m_vecStrEzsetCommands[iEZ] << "; ";
  if (m_vecStrEzsetCommands.size() > 0)
    os << "\n";

  os << "Descriptions\n";
  for (unsigned int iDesc = 0; iDesc < m_vecStrDescriptions.size(); iDesc++)
    os << m_vecStrDescriptions[iDesc] << "\n";
}
