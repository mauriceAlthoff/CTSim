/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          projections.h
**   Purpose:       Header file for Projections class
**   Programmer:    Kevin Rosenberg
**   Date Started:  July 1, 1984
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

#ifndef PROJECTIONS_H
#define PROJECTIONS_H

class Scanner;
class DetectorArray;
class Array2dFileLabel;
class fnetorderstream;

#include "array2dfile.h"
#include "imagefile.h"
#include <complex>
#include <vector>


//used for rebinning divergent beam projections to parallel
class ParallelRaysumCoordinate {
public:
  double m_dT;      // Distance from center of origin
  double m_dTheta;  // perpendicular angle to origin
  double m_dRaysum;

  static bool compareByTheta (ParallelRaysumCoordinate* a, ParallelRaysumCoordinate* b)
  { return a->m_dTheta == b->m_dTheta ? b->m_dT > a->m_dT : b->m_dTheta > a->m_dTheta; }

  // sort first by T, then Theta
  static bool compareByT (ParallelRaysumCoordinate* a, ParallelRaysumCoordinate* b)
  { return a->m_dT == b->m_dT ? b->m_dTheta > a->m_dTheta : b->m_dT > a->m_dT; }

};

class ParallelRaysums {
public:

  enum {
    THETA_RANGE_UNCONSTRAINED = 0,
    THETA_RANGE_NORMALIZE_TO_TWOPI,
    THETA_RANGE_FOLD_TO_PI,
  };

  ParallelRaysums (const Projections* pProjections, int iThetaRange);
  ~ParallelRaysums ();

  typedef std::vector<ParallelRaysumCoordinate*> CoordinateContainer;
  typedef CoordinateContainer::iterator CoordinateIterator;

  CoordinateContainer& getCoordinates() {return m_vecpCoordinates;}
  int getNumCoordinates() const {return m_iNumCoordinates;}
  void getLimits (double* dMinT, double* dMaxT, double* dMinTheta, double* dMaxTheta) const;
  CoordinateContainer& getSortedByT();
  CoordinateContainer& getSortedByTheta();
  double interpolate (double* pdX, double* pdY, int n, double dXValue, int* iLastFloor = NULL);
  void getThetaAndRaysumsForT (int iT, double* pdTheta, double* pdRaysum);
  void getDetPositions (double* pdDetPos);

private:
  CoordinateContainer m_vecpCoordinates;
  CoordinateContainer m_vecpSortedByT;
  CoordinateContainer m_vecpSortedByTheta;
  ParallelRaysumCoordinate* m_pCoordinates;
  int m_iNumCoordinates;
  int m_iNumView;
  int m_iNumDet;
  int m_iThetaRange;
};



// Projections
class Projections
{
 public:

  static const int POLAR_INTERP_INVALID;
  static const int POLAR_INTERP_NEAREST;
  static const int POLAR_INTERP_BILINEAR;
  static const int POLAR_INTERP_BICUBIC;

  Projections (const int nView, const int nDet);
  Projections (const Scanner& scanner);
  Projections ();
  ~Projections ();

  static const int getInterpCount() {return s_iInterpCount;}
  static const char* const* getInterpNameArray() {return s_aszInterpName;}
  static const char* const* getInterpTitleArray() {return s_aszInterpTitle;}
  static int convertInterpNameToID (const char* const interpName);
  static const char* convertInterpIDToName (const int interpID);
  static const char* convertInterpIDToTitle (const int interpID);

  void initFromScanner (const Scanner& scanner);
  bool initFromSomatomAR_STAR (int iNViews, int iNDets, unsigned char* pData, unsigned long lDataLength);

  void printProjectionData (int startView, int endView);
  void printProjectionData ();
  void printScanInfo (std::ostringstream& os) const;

  int Helical180LI(int interpView);
  int Helical180LI_Equiangular(int interpView);
  int HalfScanFeather(void);

  bool read (const std::string& fname);
  bool read (const char* fname);
  bool write (const char* fname);
  bool write (const std::string& fname);
  bool detarrayRead (fnetorderstream& fs, DetectorArray& darray, const int view_num);
  bool detarrayWrite (fnetorderstream& fs, const DetectorArray& darray, const int view_num);

  Projections* interpolateToParallel() const;

  bool convertPolar (ImageFile& rIF, int iInterpolation);
  bool convertFFTPolar (ImageFile& rIF, int iInterpolation, int iZeropad);
  void calcArrayPolarCoordinates (unsigned int nx, unsigned int ny, double** ppdView, double** ppdDet,
    int iNumDetWithZeros, double dZeropadRatio, double dDetInc);
  void interpolatePolar (ImageFileArray& v, ImageFileArray& vImag, unsigned int nx, unsigned int ny, std::complex<double>** ppcDetValue,
    double** ppdDet, double** ppdView, unsigned int nView, unsigned int nDet, unsigned int nDetWithZeros,
    int iInterpolate);

  bool reconstruct (ImageFile& im, const char* const filterName, double filt_param, const char* const filterMethodName, const int zeropad, const char* frequencyFilterName, const char* const interpName, int interp_param, const char* const backprojName, const int trace) const;

  void setNView (int nView);  // used in MPI to restrict # of views
  void setRotInc (double rotInc) { m_rotInc = rotInc;}
  void setDetInc (double detInc) { m_detInc = detInc;}
  void setCalcTime (double calcTime) {m_calcTime = calcTime;}
  void setRemark (const char* remark) {m_remark = remark; m_label.setLabelString(remark);}
  void setRemark (const std::string& remark) {setRemark(remark.c_str());}

  double detStart() const {return m_detStart;}
  double rotStart() const {return m_rotStart;}
  double calcTime() const {return m_calcTime;}
  double viewLen() const {return m_dViewDiameter / SQRT2;}
  const char*  remark() const {return m_remark.c_str();}
  double detInc() const {return m_detInc;}
  double rotInc() const {return m_rotInc;}
  int nDet() const {return m_nDet;}
  int nView() const {return m_nView;}
  int geometry() const {return m_geometry;}
  double focalLength() const {return m_dFocalLength;}
  double sourceDetectorLength() const { return m_dSourceDetectorLength;}
  double viewDiameter() const {return m_dViewDiameter; }
  double phmLen() const { return m_dViewDiameter / SQRT2; }
  void setPhmLen(double phmLen) { m_dViewDiameter = phmLen * SQRT2; }

  const std::string& getFilename() const {return m_filename;}
  Array2dFileLabel& getLabel() {return m_label;}
  const Array2dFileLabel& getLabel() const {return m_label;}

  DetectorArray& getDetectorArray (const int iview)
      { return (*m_projData[iview]); }

  const DetectorArray& getDetectorArray (const int iview) const
      { return (*m_projData[iview]); }

  static bool copyHeader (const char* const filename, std::ostream& os);
  static bool copyHeader (const std::string& filename, std::ostream& os);

  static bool copyViewData (const char* const filename, std::ostream& os, int startView, int endView);
  static bool copyViewData (const std::string& filename, std::ostream& os, int startView, int endView);

 private:
  int m_headerSize;             // Size of disk file header
  int m_geometry;               // Geometry of scanner
  class DetectorArray **m_projData;     // Pointer to array of detarray_st pointers
  std::string m_remark;         // description of raysum data
  int m_nDet;                   // number of detectors in array
  int m_nView;                  // number of rotated views
  double m_calcTime;            // time required to calculate raysums
  double m_rotStart;            // starting view rotation
  double m_rotInc;              // angle between rotations
  double m_detStart;            // distance of beginning detector to center phantom
  double m_detInc;              // increment between detectors
  double m_dFocalLength;
  double m_dSourceDetectorLength;
  double m_dViewDiameter;
  double m_dFanBeamAngle;
  kuint32 m_year;                   // Creation date & time
  kuint32 m_month;
  kuint32 m_day;
  kuint32 m_hour;
  kuint32 m_minute;
  kuint32 m_second;
  std::string m_filename;
  Array2dFileLabel m_label;

  const static kuint16 m_signature;

  static const char* const s_aszInterpName[];
  static const char* const s_aszInterpTitle[];
  static const int s_iInterpCount;

  bool headerWrite (fnetorderstream& fs);
  bool headerRead (fnetorderstream& fs);
  void newProjData ();
  void deleteProjData ();

  void init (const int nView, const int nDet);

  // prevent default methods
  Projections& operator= (const Projections& rhs);   // assignment
  Projections(const Projections& rhs);               // copy
};


#endif
