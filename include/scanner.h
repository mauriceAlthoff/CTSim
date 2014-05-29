/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          scanner.h
**   Purpose:       Scanner header file
**   Programmer:    Kevin Rosenberg
**   Date Started:  July 1, 1984
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

#ifndef SCANNER_H
#define SCANNER_H

#include "trace.h"

class Projections;
class Phantom;
class PhantomElement;
class SGP;

// Projections are collected along an array of ndet detectors.  The data
// for these detectors is stored in the class DetectorArray

typedef float DetectorValue;

class DetectorArray
{
 public:
  DetectorArray (const int ndet);
  ~DetectorArray ();

  const int nDet() const {return m_nDet;}
  const double viewAngle() const {return m_viewAngle;}
  DetectorValue* detValues() {return m_detValues;}
  const DetectorValue* detValues() const {return m_detValues;}

  void setViewAngle (double viewAngle)
      { m_viewAngle = viewAngle; }

 private:
  DetectorValue* m_detValues;   // Pointer to array of values recorded by detector
  int m_nDet;                   // Number of detectors in array */
  double m_viewAngle;           // View angle in radians

  DetectorArray& operator=(const DetectorArray& rhs);   // assignment
  DetectorArray (const DetectorArray& rhs);             // copy constructor
};


class Scanner
{
 public:
  static const int GEOMETRY_INVALID;
  static const int GEOMETRY_PARALLEL;
  static const int GEOMETRY_EQUILINEAR;
  static const int GEOMETRY_EQUIANGULAR;
  static const int GEOMETRY_LINOGRAM;


  Scanner (const Phantom& phm, const char* const geometryName, int nDet,
    int nView, int iOffsetView, int nSample, const double rot_anglen,
    double dFocalLengthRatio, double dCenterDetectorRatio, double dViewRatio, double dScanRatio);
  ~Scanner();

  void collectProjections (Projections& proj, const Phantom& phm, const int trace = Trace::TRACE_NONE,
    SGP* pSGP = NULL);

  void collectProjections (Projections& proj, const Phantom& phm, const int iStartView, const int iNumViews, const int iOffsetView, bool bStoreAtViewPosition, const int trace = Trace::TRACE_NONE, SGP* pSGP = NULL);

  void collectProjections (Projections& proj, const Phantom& phm, const int iStartView, const int iNumViews, const int iOffsetView, int iStorageOffset, const int trace = Trace::TRACE_NONE, SGP* pSGP = NULL);

  void setNView (int nView);
  void setOffsetView (int iOffsetView);

  bool fail() const {return m_fail;}
  const std::string& failMessage() const {return m_failMessage;}
  unsigned int nDet() const {return m_nDet;}
  unsigned int nView() const {return m_nView;}
  unsigned int offsetView() const {return m_iOffsetView;}
  unsigned int startView() const {return m_startView;}
  double rotInc() const {return m_rotInc;}
  double detInc() const {return m_detInc;}
  double detLen() const {return m_detLen;}
  double detStart() const {return m_detStart;}
  double focalLength() const {return m_dFocalLength;}
  double sourceDetectorLength() const {return m_dSourceDetectorLength;}
  double centerDetectorLength() const {return m_dCenterDetectorLength;}

  double viewDiameter() const {return m_dViewDiameter;}
  double scanDiameter() const {return m_dScanDiameter;}
  double fanBeamAngle() const {return m_dFanBeamAngle;}

  int geometry() const {return m_idGeometry;}

  static int getGeometryCount() {return s_iGeometryCount;}
  static const char* const* getGeometryNameArray() {return s_aszGeometryName;}
  static const char* const* getGeometryTitleArray() {return s_aszGeometryTitle;}
  static int convertGeometryNameToID (const char* const geometryName);
  static const char* convertGeometryIDToName (const int idGeometry);
  static const char* convertGeometryIDToTitle (const int idGeometry);

 private:
  bool m_fail;
  std::string m_failMessage;
  int m_idGeometry;
  unsigned int m_nDet;          /* Number of detectors in array */
  unsigned int m_nView;         /* Number of rotated views */
  unsigned int m_iOffsetView;
  unsigned int m_startView;
  unsigned int m_nSample;       /* Number of rays per detector */
  double m_dFocalLength;        // Focal Length, distance from source to center
  double m_dSourceDetectorLength; // Distance from source to detectors
  double m_dCenterDetectorLength; // Distance from center to detectors
  double m_dViewDiameter; // Diameter of area being processed
  double m_dScanDiameter; // Diamer of area being scanned
  double m_dViewRatio;   // View Ratio to diameter phantom
  double m_dFocalLengthRatio;   // Source to Center Length as ratio to viewDiameter radius
  double m_dCenterDetectorRatio; // Center to Detector Length as ratio of viewDiameter radius
  double m_dScanRatio;       // Scan length to view length ratio
  double m_dFanBeamAngle;
  double m_detLen;              // Total length of detector array
  double m_rotLen;              // Rotation angle length in radians (norm 2PI)
  double m_detInc;              // Increment between centers of detectors
  double m_rotInc;              // Increment in rotation angle between views
  double m_detStart;
  double m_dXCenter;            // Center of Phantom
  double m_dYCenter;
  double m_dAngularDetIncrement;
  double m_dAngularDetLen;

  int m_trace;
  struct {
    double xd1,yd1,xd2,yd2;     /* Coordinates of detector endpoints */
    double xs1,ys1,xs2,ys2;     /* Coordinates of source endpoints */
    double angle;               /* Starting angle */
    double dAngularDet;
  } m_initPos;

  GRFMTX_2D m_rotmtxIncrement;

#ifdef HAVE_SGP
  SGP* m_pSGP;                  // Pointer to graphics device
  double m_dXMinWin;            // Extent of graphics window
  double m_dXMaxWin;
  double m_dYMinWin;
  double m_dYMaxWin;
  double m_dTextHeight;
#endif

  static const char* const s_aszGeometryName[];
  static const char* const s_aszGeometryTitle[];
  static const int s_iGeometryCount;

  void projectSingleView (const Phantom& phm, DetectorArray& darray, const double xd1, const double yd1, const double xd2, const double yd2, const double xs1, const double ys1, const double xs2, const double ys2, const double dDetAngle);

  double projectSingleLine (const Phantom& phm, const double x1, const double y1, const double x2, const double y2);

  double projectLineAgainstPElem (const PhantomElement& pelem, const double x1, const double y1, const double x2, const double y2);

  void traceShowParam (const char* szLabel, const char *fmt, int row, int color, ...);
  void traceShowParamXOR (const char* szLabel, const char *fmt, int row, int color, ...);
  void traceShowParamRasterOp (int iRasterOp, const char* szLabel, const char* fmt, int row, int color, va_list va);


};

const static int PROJECTION_TRACE_ROW_PHANT_ID=0;
const static int PROJECTION_TRACE_ROW_GEOMETRY=1;
const static int PROJECTION_TRACE_ROW_FOCAL_LENGTH=2;
const static int PROJECTION_TRACE_ROW_FIELD_OF_VIEW=3;
const static int PROJECTION_TRACE_ROW_NDET=4;
const static int PROJECTION_TRACE_ROW_NVIEW=5;
const static int PROJECTION_TRACE_ROW_SAMPLES=6;
const static int PROJECTION_TRACE_ROW_CURR_VIEW=7;
const static int PROJECTION_TRACE_ROW_ATTEN=8;



#endif
