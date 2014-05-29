/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         array2dfile.h
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

#ifndef ARRAY2DFILE_H
#define ARRAY2DFILE_H

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
#include "fnetorderstream.h"
#include "array2d.h"

class Array2dFileLabel
{
public:
  enum {
    L_EMPTY = 0,
      L_HISTORY = 1,
      L_USER = 2,
  };

  Array2dFileLabel();

  Array2dFileLabel(const char* const str, double ctime = 0.);

  Array2dFileLabel(const int type, const char* const str, double ctime = 0.);

  ~Array2dFileLabel();

  const std::string& getLabelString (void) const
  { return m_strLabel; }

  kfloat64 getCalcTime (void) const
  { return m_calcTime; }

  void setCalcTime (kfloat64 calcTime)
  { m_calcTime = calcTime; }

  void setLabelType (int labelType)
  { m_labelType = labelType; }

  int getLabelType (void) const
  { return m_labelType; }

  std::string& setLabelString (const char* const str)
  { m_strLabel = str; return (m_strLabel); }

  std::string& setLabelString (const std::string& str)
  { m_strLabel = str; return (m_strLabel); }

  void setDateTime (int year, int month, int day, int hour, int minute, int second);

  void getDateTime (int& year, int& month, int& day, int& hour, int& minute, int& second) const;

  const std::string& getDateString () const;

  void print (std::ostream& os) const;
  void printBrief (std::ostream& os) const;
  void printBrief (std::ostringstream& os) const;

  Array2dFileLabel (const Array2dFileLabel& rhs);

  Array2dFileLabel& operator= (const Array2dFileLabel& rhs);

private:
  void init (void);

  kuint16 m_labelType;
  kuint16 m_year;
  kuint16 m_month;
  kuint16 m_day;
  kuint16 m_hour;
  kuint16 m_minute;
  kuint16 m_second;
  std::string m_strLabel;
  kfloat64 m_calcTime;

  mutable std::string m_strDate;
};


class Array2dFile
{
public:
  enum {
    PIXEL_INVALID = 0,
      PIXEL_INT8 = 1,
      PIXEL_UINT8 = 2,
      PIXEL_INT16 = 3,
      PIXEL_UINT16 = 4,
      PIXEL_INT32 = 5,
      PIXEL_UINT32 = 6,
      PIXEL_FLOAT32 = 7,
      PIXEL_FLOAT64 = 8,
  };

  enum {
    DATA_TYPE_INVALID = 0,
      DATA_TYPE_REAL,
      DATA_TYPE_COMPLEX,
  };

  Array2dFile (int nx, int ny, int pixelSize, int pixelFormat = PIXEL_INVALID, int dataType = DATA_TYPE_REAL);
  Array2dFile (void);
  ~Array2dFile ();

  void setArraySize (int nx, int ny, int pixelSize, int pixelFormat = PIXEL_INVALID, int dataType = DATA_TYPE_REAL);

  void setArraySize (int nx, int ny);

  unsigned int getNumLabels (void) const
  { return m_labels.size(); }

  const Array2dFileLabel& labelGet (int label_num) const;

  void labelAdd (const Array2dFileLabel& label);

  void labelAdd (const char* const m_strLabel, double calc_time=0.);

  void labelAdd (int type, const char* const m_strLabel, double calc_time=0.);

  void labelsCopy (const Array2dFile& file, const char* const idStr = NULL);

  void setPixelFormat (int type)
  { m_pixelFormat = type; }

  void setPixelSize (int size)
  { m_pixelSize = size; }

  kuint32 nx (void) const
  { return m_nx; }

  kuint32 ny (void) const
  { return m_ny; }

  bool isComplex() const
  { return m_dataType == DATA_TYPE_COMPLEX; }

  bool isReal() const
  { return m_dataType == DATA_TYPE_REAL; }

  int dataType () const
  { return static_cast<int>(m_dataType); }

  void setDataType (int dataType)
  { m_dataType = dataType; }

  void setAxisIncrement (double axisIncX, double axisIncY);

  bool reallocRealToComplex ();

  bool reallocComplexToReal ();

  void getPixelValueRange (double& pvmin, double& pvmax) const;
  void setAxisExtent (double minX, double maxX, double minY, double maxY);
  bool getAxisExtent (double& minX, double& maxX, double& minY, double& maxY) const
  { if (! m_axisExtentKnown) return false; minX = m_minX; maxX = m_maxX; minY = m_minY; maxY=m_maxY;
    return true; }

  void doPixelOffsetScale (double offset, double scale);

  kfloat64 axisIncrementX() const {return m_axisIncrementKnown ? m_axisIncrementX : 0.;}
  kfloat64 axisIncrementY() const {return m_axisIncrementKnown ? m_axisIncrementY : 0.;}

  void arrayDataClear (void);

  bool fileRead (const char* const filename);

  bool fileRead (const std::string& filename);

  bool fileWrite (const char* const filename);

  bool fileWrite (const std::string& filename);

  const std::string& getFilename (void) const
  {  return m_filename; }

  void printLabels (std::ostream& os) const;
  void printLabelsBrief (std::ostream& os) const;
  void printLabelsBrief (std::ostringstream& os) const;

  unsigned int nLabels() const
  { return m_labels.size(); }

  typedef std::vector<Array2dFileLabel*>::iterator labelIterator;
  typedef std::vector<Array2dFileLabel*>::const_iterator constLabelIterator;

protected:
         typedef std::vector<Array2dFileLabel*> labelContainer;

   static const kuint16 m_signature;
   kuint16 m_headersize;
   std::string  m_filename;

   kuint16 m_pixelSize;
   kuint16 m_pixelFormat;
   kuint16 m_axisIncrementKnown;
   kfloat64 m_axisIncrementX, m_axisIncrementY;
   kuint16 m_axisExtentKnown;
   kfloat64 m_minX, m_maxX, m_minY, m_maxY;
   kfloat64 m_offsetPV, m_scalePV;
   kuint32 m_nx;
   kuint32 m_ny;
   kuint32 m_arraySize;
   labelContainer m_labels;
   kuint16 m_numFileLabels;
   kuint16 m_dataType;
   unsigned char** m_arrayData;
   unsigned char** m_imaginaryArrayData;

private:
  void init (void);

  bool headerRead (frnetorderstream& fs);

  bool headerWrite (frnetorderstream& fs);

  bool arrayDataRead (frnetorderstream& fs);

  bool arrayDataWrite (frnetorderstream& fs);

  bool labelsRead (frnetorderstream& fs);

  bool labelsWrite (frnetorderstream& fs);

  bool labelSeek (int label_num);

  void allocArrays ();
  void freeArrays ();

  void allocArray (unsigned char**& rppData);
  void freeArray (unsigned char**& rppData);

  Array2dFile (const Array2dFile& rhs);        // copy constructor
  Array2dFile& operator= (const Array2dFile&); // assignment operator

};



#endif
