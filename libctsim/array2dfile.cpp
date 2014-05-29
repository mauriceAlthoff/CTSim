/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         array2dfile.cpp
**      Purpose:      2-dimension array file class
**      Programmer:   Kevin Rosenberg
**      Date Started: June 2000
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

#include "array2dfile.h"
#include <ctime>
#ifdef MSVC
typedef long off_t;
#endif

#include <sstream>

const kuint16 Array2dFile::m_signature = ('I'*256+'F');

///////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//
//     Name: Array2dFileLabel
//     Purpose: Labels for Array2dFiles
///////////////////////////////////////////////////////////////////////////

void
Array2dFileLabel::init (void)
{
  m_calcTime = 0;
  m_labelType = L_EMPTY;
  time_t t = time(0);
  tm* lt = localtime(&t);
  m_year = lt->tm_year;
  m_month = lt->tm_mon;
  m_day = lt->tm_mday;
  m_hour = lt->tm_hour;
  m_minute = lt->tm_min;
  m_second = lt->tm_sec;
}

Array2dFileLabel::Array2dFileLabel()
{
  init();
}

Array2dFileLabel::Array2dFileLabel(const char* const str, double ctime)
: m_strLabel (str)
{
  init();

  m_labelType = L_USER;
  m_calcTime = ctime;
}

Array2dFileLabel::Array2dFileLabel(const int type, const char* const str, double ctime)
:  m_strLabel (str)
{
  init();

  m_labelType = type;
  m_calcTime = ctime;
}

Array2dFileLabel::~Array2dFileLabel()
{
}

void
Array2dFileLabel::setDateTime (int year, int month, int day, int hour, int minute, int second)
{
  m_year = year;
  m_month = month;
  m_day = day;
  m_hour = hour;
  m_minute = minute;
  m_second = second;
}

void
Array2dFileLabel::getDateTime (int& year, int& month, int& day, int& hour, int& minute, int& second) const
{
  year = m_year;
  month = m_month;
  day = m_day;
  hour = m_hour;
  minute = m_minute;
  second = m_second;
}

const std::string&
Array2dFileLabel::getDateString (void) const
{
  char szDate [128];
  snprintf (szDate, sizeof(szDate), "%2d/%02d/%4d %02d:%02d:%02d",
    m_month + 1, m_day, m_year + 1900, m_hour, m_minute, m_second);
  m_strDate = szDate;
  return m_strDate;
}


Array2dFileLabel::Array2dFileLabel (const Array2dFileLabel& rhs)
{
  m_calcTime = rhs.m_calcTime;
  m_labelType = rhs.m_labelType;
  m_strLabel = rhs.m_strLabel;
  m_year = rhs.m_year; m_month = rhs.m_month; m_day = rhs.m_day;
  m_hour = rhs.m_hour; m_minute = rhs.m_minute; m_second = rhs.m_second;
}

Array2dFileLabel&
Array2dFileLabel::operator= (const Array2dFileLabel& rhs)
{
  m_calcTime = rhs.m_calcTime;
  m_labelType = rhs.m_labelType;
  m_strLabel = rhs.m_strLabel;
  m_year = rhs.m_year; m_month = rhs.m_month; m_day = rhs.m_day;
  m_hour = rhs.m_hour; m_minute = rhs.m_minute; m_second = rhs.m_second;

  return (*this);
}

void
Array2dFileLabel::print (std::ostream& os) const
{
  if (m_labelType == L_HISTORY) {
    os << "History: " << std::endl;
    os << "  " << m_strLabel << std::endl;
    os << "  calc time = " << m_calcTime << " secs" << std::endl;
    os << "  Timestamp = " << getDateString() << std::endl;
  } else if (m_labelType == L_USER) {
    os << "Note: " <<  m_strLabel << std::endl;
    os << "  Timestamp = %s" << getDateString() << std::endl;
  } else {
    os << "Unknown (" << m_labelType << "): " <<  m_strLabel << std::endl;
    os << "  Timestamp = %s" << getDateString() << std::endl;
  }
}

void
Array2dFileLabel::printBrief (std::ostream& os) const
{
  if (m_labelType == L_HISTORY) {
    os << "History (";
    if (m_calcTime > 0)
      os << m_calcTime << " secs, ";
    os << getDateString() << "): " << m_strLabel << std::endl;
  } else if (m_labelType == L_USER) {
    os << "Note (" <<  getDateString() << "): " << m_strLabel << std::endl;
  } else {
    os << "Unknown (" << getDateString() << "): " << m_strLabel << std::endl;
  }
}

void
Array2dFileLabel::printBrief (std::ostringstream& os) const
{
  if (m_labelType == L_HISTORY) {
    os << "History (";
    if (m_calcTime > 0)
      os << m_calcTime << " secs, ";
    os << getDateString().c_str() << "): " << m_strLabel.c_str() << "\n";
  } else if (m_labelType == L_USER) {
    os << "Note (" <<  getDateString() << "): " << m_strLabel << "\n";
  } else {
    os << "Unknown (" << getDateString() << "): " << m_strLabel << "\n";
  }
}


///////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//
//     Name: Array2dFile
//     Purpose: Array2dFiles
///////////////////////////////////////////////////////////////////////////


Array2dFile::Array2dFile (int x, int y, int pixelSize, int pixelFormat, int dataType)
{
  init();
  setArraySize (x, y, pixelSize, pixelFormat, dataType);
}

Array2dFile::~Array2dFile (void)
{
  freeArrays ();
  for (labelIterator l = m_labels.begin(); l != m_labels.end(); l++)
    delete *l;
}

Array2dFile::Array2dFile (void)
{
  init();
}

void
Array2dFile::init (void)
{
  m_pixelSize = 0;
  m_pixelFormat = PIXEL_INVALID;
  m_arrayData = NULL;
  m_imaginaryArrayData = NULL;
  m_dataType = DATA_TYPE_INVALID;
  m_nx = 0;
  m_ny = 0;
  m_headersize = 0;
  m_axisIncrementKnown = false;
  m_axisIncrementX = m_axisIncrementY = 0;
  m_axisExtentKnown = false;
  m_minX = m_maxX = m_minY = m_maxY = 0;
  m_offsetPV = 0;
  m_scalePV = 1;
}


void
Array2dFile::setArraySize (int x, int y, int pixelSize, int pixelFormat, int dataType)
{
  m_pixelSize = pixelSize;
  m_pixelFormat = pixelFormat;
  m_dataType = dataType;
  setArraySize (x, y);
}

void
Array2dFile::setArraySize (int x, int y)
{
  m_nx = x;
  m_ny = y;
  allocArrays ();
}

bool
Array2dFile::reallocComplexToReal ()
{
  if (m_dataType != DATA_TYPE_COMPLEX)
    return false;

  freeArray (m_imaginaryArrayData);
  m_dataType = DATA_TYPE_REAL;

  return true;
}


bool
Array2dFile::reallocRealToComplex ()
{
  if (m_dataType != DATA_TYPE_REAL)
    return false;

  allocArray (m_imaginaryArrayData);
  m_dataType = DATA_TYPE_COMPLEX;

  return true;
}



void
Array2dFile::allocArrays ()
{
  if (m_arrayData)
    freeArray (m_arrayData);
  if (m_imaginaryArrayData)
    freeArray (m_imaginaryArrayData);

  allocArray (m_arrayData);
  if (m_dataType == DATA_TYPE_COMPLEX)
    allocArray (m_imaginaryArrayData);
}

void
Array2dFile::allocArray (unsigned char**& rppData)
{
  m_arraySize = m_nx * m_ny * m_pixelSize;
  rppData = new unsigned char* [m_nx];
  int columnBytes = m_ny * m_pixelSize;
  for (unsigned int i = 0; i < m_nx; i++)
    rppData[i] = new unsigned char [columnBytes];
}

void
Array2dFile::freeArrays ()
{
  if (m_arrayData)
    freeArray (m_arrayData);

  if (m_imaginaryArrayData)
    freeArray (m_imaginaryArrayData);

}

void
Array2dFile::freeArray (unsigned char**& rppData)
{
   for (unsigned int i = 0; i < m_nx; i++)
      delete rppData[i];
    delete rppData;
    rppData = NULL;
}


bool
Array2dFile::fileWrite (const std::string& filename)
{
  return fileWrite (filename.c_str());
}

bool
Array2dFile::fileWrite (const char* const filename)
{
  m_filename = filename;

  frnetorderstream fs (m_filename.c_str(), std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
  if (fs.fail()) {
    sys_error (ERR_WARNING, "Error opening file %s for writing [fileCreate]", m_filename.c_str());
    return false;
  }
  if (! headerWrite(fs))
    return false;

  if (! arrayDataWrite (fs))
    return false;

  if (! labelsWrite (fs))
    return false;

  return true;
}

bool
Array2dFile::fileRead (const std::string& filename)
{
  return fileRead (filename.c_str());
}

bool
Array2dFile::fileRead (const char* const filename)
{
  m_filename = filename;

  frnetorderstream fs (m_filename.c_str(), std::ios::out | std::ios::in | std::ios::binary);

  if (fs.fail()) {
    sys_error (ERR_WARNING, "Unable to open file %s [fileRead]", m_filename.c_str());
    return false;
  }

  if (! headerRead(fs))
    return false;

  allocArrays ();

  if (! arrayDataRead(fs))
    return false;;

  if (! labelsRead (fs))
    return false;

  return true;
}

void
Array2dFile::setAxisIncrement (double incX, double incY)
{
  m_axisIncrementKnown = true;
  m_axisIncrementX = incX;
  m_axisIncrementY = incY;
}

void
Array2dFile::setAxisExtent (double minX, double maxX, double minY, double maxY)
{
  m_axisExtentKnown = true;
  m_minX = minX;
  m_maxY = maxX;
  m_minX = minX;
  m_maxY = maxY;
}

bool
Array2dFile::headerRead (frnetorderstream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to read header with file closed [headerRead]");
    return false;
  }

  fs.seekg (0);
  kuint16 file_signature;

  fs.readInt16 (m_headersize);
  fs.readInt16 (file_signature);
  fs.readInt16 (m_pixelFormat);
  fs.readInt16 (m_pixelSize);
  fs.readInt16 (m_numFileLabels);
  fs.readInt32 (m_nx);
  fs.readInt32 (m_ny);
  fs.readInt16 (m_dataType);
  fs.readInt16 (m_axisIncrementKnown);
  fs.readFloat64 (m_axisIncrementX);
  fs.readFloat64 (m_axisIncrementY);
  fs.readInt16 (m_axisExtentKnown);
  fs.readFloat64 (m_minX);
  fs.readFloat64 (m_maxX);
  fs.readFloat64 (m_minY);
  fs.readFloat64 (m_maxY);
  fs.readFloat64 (m_offsetPV);
  fs.readFloat64 (m_scalePV);

  int read_m_headersize = fs.tellg();
  if (read_m_headersize != m_headersize) {
    sys_error (ERR_WARNING, "Read m_headersize %d != file m_headersize %d", read_m_headersize, m_headersize);
    return false;
  }
  if (file_signature != m_signature) {
    sys_error (ERR_WARNING, "File signature %d != true signature %d", file_signature, m_signature);
    return false;
  }

  return ! fs.fail();
}


bool
Array2dFile::headerWrite (frnetorderstream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to write header with ! fs");
    return false;
  }

  m_numFileLabels = m_labels.size();

  fs.seekp (0);
  fs.writeInt16 (m_headersize);
  fs.writeInt16 (m_signature);
  fs.writeInt16 (m_pixelFormat);
  fs.writeInt16 (m_pixelSize);
  fs.writeInt16 (m_numFileLabels);
  fs.writeInt32 (m_nx);
  fs.writeInt32 (m_ny);
  fs.writeInt16 (m_dataType);
  fs.writeInt16 (m_axisIncrementKnown);
  fs.writeFloat64 (m_axisIncrementX);
  fs.writeFloat64 (m_axisIncrementY);
  fs.writeInt16 (m_axisExtentKnown);
  fs.writeFloat64 (m_minX);
  fs.writeFloat64 (m_maxX);
  fs.writeFloat64 (m_minY);
  fs.writeFloat64 (m_maxY);
  fs.writeFloat64 (m_offsetPV);
  fs.writeFloat64 (m_scalePV);

  m_headersize = static_cast<kuint16>(fs.tellp());
  fs.seekp (0);
  fs.writeInt16 (m_headersize);

  return ! fs.fail();
}


bool
Array2dFile::arrayDataWrite (frnetorderstream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to arrayDataWrite with !fs");
    return false;
  }

  if (! m_arrayData)
    return false;

  fs.seekp (m_headersize);
  int columnSize = m_ny * m_pixelSize;
  for (unsigned int ix = 0; ix < m_nx; ix++) {
    unsigned char* ptrColumn = m_arrayData[ix];
    if (NativeBigEndian()) {
      for (unsigned int iy = 0; iy < m_ny; iy++) {
        ConvertReverseNetworkOrder (ptrColumn, m_pixelSize);
        fs.write (reinterpret_cast<const char*>(ptrColumn), m_pixelSize);
        ptrColumn += m_pixelSize;
      }
    } else
      fs.write (reinterpret_cast<const char*>(ptrColumn), columnSize);
  }
  if (m_dataType == DATA_TYPE_COMPLEX) {
    for (unsigned int ix = 0; ix < m_nx; ix++) {
      unsigned char* ptrColumn = m_imaginaryArrayData[ix];
      if (NativeBigEndian()) {
        for (unsigned int iy = 0; iy < m_ny; iy++) {
          ConvertReverseNetworkOrder (ptrColumn, m_pixelSize);
          fs.write (reinterpret_cast<const char*>(ptrColumn), m_pixelSize);
          ptrColumn += m_pixelSize;
        }
      } else
        fs.write (reinterpret_cast<const char*>(ptrColumn), columnSize);
    }
  }

  return true;
}


bool
Array2dFile::arrayDataRead (frnetorderstream& fs)
{
  if (! fs) {
    sys_error (ERR_WARNING, "Tried to arrayDataRead with ! fs");
    return false;
  }

  if (! m_arrayData)
    return false;

  fs.seekg (m_headersize);
  int columnSize = m_ny * m_pixelSize;
  for (unsigned int ix = 0; ix < m_nx; ix++) {
    unsigned char* ptrColumn = m_arrayData[ix];
    if (NativeBigEndian()) {
      for (unsigned int iy = 0; iy < m_ny; iy++) {
        fs.read (reinterpret_cast<char*>(ptrColumn), m_pixelSize);
        ConvertReverseNetworkOrder (ptrColumn, m_pixelSize);
        ptrColumn += m_pixelSize;
      }
    } else
      fs.read (reinterpret_cast<char*>(ptrColumn), columnSize);
  }
  if (m_dataType == DATA_TYPE_COMPLEX) {
    for (unsigned int ix = 0; ix < m_nx; ix++) {
      unsigned char* ptrColumn = m_imaginaryArrayData[ix];
      if (NativeBigEndian()) {
        for (unsigned int iy = 0; iy < m_ny; iy++) {
          fs.read (reinterpret_cast<char*>(ptrColumn), m_pixelSize);
          ConvertReverseNetworkOrder (ptrColumn, m_pixelSize);
          ptrColumn += m_pixelSize;
        }
      } else
        fs.read (reinterpret_cast<char*>(ptrColumn), columnSize);
    }
  }

  return true;
}

bool
Array2dFile::labelsRead (frnetorderstream& fs)
{
  off_t pos = m_headersize + m_arraySize;
  fs.seekg (pos);
  if (fs.fail())
    return false;

  for (int i = 0; i < m_numFileLabels; i++) {
    kuint16 labelType, year, month, day, hour, minute, second;
    kfloat64 calcTime;

    fs.readInt16 (labelType);
    fs.readInt16 (year);
    fs.readInt16 (month);
    fs.readInt16 (day);
    fs.readInt16 (hour);
    fs.readInt16 (minute);
    fs.readInt16 (second);
    fs.readFloat64 (calcTime);

    kuint16 strLength;
    fs.readInt16 (strLength);
    char* pszLabelStr = new char [strLength+1];
    fs.read (pszLabelStr, strLength);
    pszLabelStr[strLength] = 0;

    Array2dFileLabel* pLabel = new Array2dFileLabel (labelType, pszLabelStr, calcTime);
    delete pszLabelStr;

    pLabel->setDateTime (year, month, day, hour, minute, second);
    m_labels.push_back (pLabel);
  }

  return true;
}

bool
Array2dFile::labelsWrite (frnetorderstream& fs)
{
  off_t pos = m_headersize + m_arraySize;
  fs.seekp (pos);

  for (constLabelIterator l = m_labels.begin(); l != m_labels.end(); l++) {
    const Array2dFileLabel& label = **l;
    kuint16 labelType = label.getLabelType();
    kfloat64 calcTime = label.getCalcTime();
    const char* const labelString = label.getLabelString().c_str();
    int year, month, day, hour, minute, second;
    kuint16 yearBuf, monthBuf, dayBuf, hourBuf, minuteBuf, secondBuf;

    label.getDateTime (year, month, day, hour, minute, second);
    yearBuf = year; monthBuf = month; dayBuf = day;
    hourBuf = hour; minuteBuf = minute; secondBuf = second;

    fs.writeInt16 (labelType);
    fs.writeInt16 (yearBuf);
    fs.writeInt16 (monthBuf);
    fs.writeInt16 (dayBuf);
    fs.writeInt16 (hourBuf);
    fs.writeInt16 (minuteBuf);
    fs.writeInt16 (secondBuf);
    fs.writeFloat64 (calcTime);
    kuint16 strlength = strlen (labelString);
    fs.writeInt16 (strlength);
    fs.write (labelString, strlength);
  }

  return true;
}

void
Array2dFile::labelAdd (const char* const lstr, double calc_time)
{
  labelAdd (Array2dFileLabel::L_HISTORY, lstr, calc_time);
}


void
Array2dFile::labelAdd (int type, const char* const lstr, double calc_time)
{
  Array2dFileLabel label (type, lstr, calc_time);

  labelAdd (label);
}


void
Array2dFile::labelAdd (const Array2dFileLabel& label)
{
  Array2dFileLabel* pLabel = new Array2dFileLabel(label);

  m_labels.push_back (pLabel);
}

void
Array2dFile::labelsCopy (const Array2dFile& copyFile, const char* const pszId)
{
  std::string id;
  if (pszId)
    id = pszId;
  for (unsigned int i = 0; i < copyFile.getNumLabels(); i++) {
    Array2dFileLabel l (copyFile.labelGet (i));
    std::string lstr = l.getLabelString();
    lstr = id + lstr;
    l.setLabelString (lstr);
    labelAdd (l);
  }
}

void
Array2dFile::arrayDataClear (void)
{
  if (m_arrayData) {
    int columnSize = m_ny * m_pixelSize;
    for (unsigned int ix = 0; ix < m_nx; ix++)
      memset (m_arrayData[ix], 0, columnSize);
  }
  if (m_imaginaryArrayData) {
    int columnSize = m_ny * m_pixelSize;
    for (unsigned int ix = 0; ix < m_nx; ix++)
      memset (m_arrayData[ix], 0, columnSize);
  }
}

void
Array2dFile::printLabels (std::ostream& os) const
{
  for (constLabelIterator l = m_labels.begin(); l != m_labels.end(); l++) {
    const Array2dFileLabel& label = **l;

    label.print (os);
    os << std::endl;
  }
}

void
Array2dFile::printLabelsBrief (std::ostream& os) const
{
  for (constLabelIterator l = m_labels.begin(); l != m_labels.end(); l++) {
    const Array2dFileLabel& label = **l;

    label.printBrief (os);
  }
}

void
Array2dFile::printLabelsBrief (std::ostringstream& os) const
{
  for (constLabelIterator l = m_labels.begin(); l != m_labels.end(); l++) {
    const Array2dFileLabel& label = **l;

    label.printBrief (os);
  }
}


const Array2dFileLabel&
Array2dFile::labelGet (int i) const
{
  return *m_labels[i];
}
