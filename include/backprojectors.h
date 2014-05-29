/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         backproject.h
**      Purpose:      Backprojection classes
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


#ifndef __BACKPROJECTORS_H
#define __BACKPROJECTORS_H

#undef HAVE_BSPLINE_INTERP

#include "imagefile.h"


class Backproject;
class ImageFile;
class Projections;
struct ReconstructionROI;

class Backprojector
{
 public:
  static const int BPROJ_INVALID;
  static const int BPROJ_TRIG;
  static const int BPROJ_TABLE;
  static const int BPROJ_DIFF;
  static const int BPROJ_IDIFF;

  static const int INTERP_INVALID;
  static const int INTERP_NEAREST;
  static const int INTERP_LINEAR;
  static const int INTERP_CUBIC;
  static const int INTERP_FREQ_PREINTERPOLATION;
#if HAVE_BSPLINE_INTERP
  static const int INTERP_BSPLINE;
  static const int INTERP_1BSPLINE;
  static const int INTERP_2BSPLINE;
  static const int INTERP_3BSPLINE;
#endif

  Backprojector (const Projections& proj, ImageFile& im, const char* const backprojName,
    const char* const interpName, const int interpFactor, const ReconstructionROI* pROI);

  ~Backprojector ();

  void BackprojectView (const double* const viewData, const double viewAngle);
  void PostProcessing();

  bool fail() const {return m_fail;}
  const std::string& failMessage() const {return m_failMessage;}

  static const int getBackprojectCount() {return s_iBackprojectCount;}
  static const char* const* getBackprojectNameArray() {return s_aszBackprojectName;}
  static const char* const* getBackprojectTitleArray() {return s_aszBackprojectTitle;}
  static int convertBackprojectNameToID (const char* const bprojName);
  static const char* convertBackprojectIDToName (const int bprojID);
  static const char* convertBackprojectIDToTitle (const int bprojID);

  static const int getInterpCount() {return s_iInterpCount;}
  static const char* const * getInterpNameArray() {return s_aszInterpName;}
  static const char* const * getInterpTitleArray() {return s_aszInterpTitle;}
  static int convertInterpNameToID (const char* const interpName);
  static const char* convertInterpIDToName (const int interpID);
  static const char* convertInterpIDToTitle (const int interpID);


 private:
  std::string m_nameBackproject;
  std::string m_nameInterpolation;
  int m_idBackproject;
  int m_idInterpolation;
  Backproject* m_pBackprojectImplem;
  bool m_fail;
  std::string m_failMessage;

  static const char* const s_aszBackprojectName[];
  static const char* const s_aszBackprojectTitle[];
  static const int s_iBackprojectCount;

  static const char* const s_aszInterpName[];
  static const char* const s_aszInterpTitle[];
  static const int s_iInterpCount;

  bool initBackprojector (const Projections& proj, ImageFile& im, const char* const backprojName,
    const char* const interpName, const int interpFactor, const ReconstructionROI* pROI);
};


class Backproject
{
 public:
    Backproject (const Projections& proj, ImageFile& im, int interpID, const int interpFactor,
      const ReconstructionROI* pROI);

    virtual ~Backproject ();

    virtual void BackprojectView (const double* const viewData, const double viewAngle) = 0;
    virtual void PostProcessing (); // call after backprojecting all views

 protected:
    void ScaleImageByRotIncrement ();
    void errorIndexOutsideDetector (int ix, int iy, double theta, double r, double phi, double L, int ni);
    void errorIndexOutsideDetector (int ix, int iy, double theta, double L, int ni);
    const Projections& proj;
    ImageFile& im;
    int interpType;
    ImageFileArray v;
    kint32 nx;
    kint32 ny;
    double detInc;
    double rotScale;
    int iDetCenter;             // index refering to L=0 projection
    int nDet;
    double xMin, xMax, yMin, yMax;     // Retangular coords of phantom
    double xInc, yInc;  // size of cells
    int m_interpFactor;
    double m_dFocalLength;
    double m_dSourceDetectorLength;
    bool m_bPostProcessingDone;

 private:
    Backproject (const Backproject& rhs);
    Backproject& operator= (const Backproject& rhs);
};


class BackprojectTrig : public Backproject
{
 public:
  BackprojectTrig (const Projections& proj, ImageFile& im, int interpID, const int interpFactor, const ReconstructionROI* pROI)
      : Backproject (proj, im, interpID, interpFactor, pROI)
      {}

  void BackprojectView (const double* const t, const double view_angle);
};


class BackprojectTable : public Backproject
{
 public:
  BackprojectTable (const Projections& proj, ImageFile& im, int interpID, const int interpFactor, const ReconstructionROI* pROI);
  virtual ~BackprojectTable ();

  virtual void BackprojectView (const double* const t, const double view_angle);
  virtual void PostProcessing (); // call after backprojecting all views

 protected:
  Array2d<kfloat64> arrayR;
  Array2d<kfloat64> arrayPhi;
  kfloat64** r;
  kfloat64** phi;
};


class BackprojectDiff : public Backproject
{
 public:
  BackprojectDiff (const Projections& proj, ImageFile& im, int interpID, const int interpFactor, const ReconstructionROI* pROI);
  ~BackprojectDiff ();

  virtual void BackprojectView (const double* const t, const double view_angle);
  virtual void PostProcessing (); // call after backprojecting all views

 protected:
  double start_r;
  double start_phi;
  double im_xinc, im_yinc;
};


class BackprojectIntDiff : public BackprojectDiff
{
 public:
  BackprojectIntDiff (const Projections& proj, ImageFile& im, int interpID, const int interpFactor, const ReconstructionROI* pROI)
    :  BackprojectDiff (proj, im, interpID, interpFactor, pROI)
    {}

  void BackprojectView (const double* const t, const double view_angle);
};

class BackprojectEquilinear : public BackprojectTable
{
 public:
  BackprojectEquilinear (const Projections& proj, ImageFile& im, int interpID, const int interpFactor,  const ReconstructionROI* pROI)
      : BackprojectTable (proj, im, interpID, interpFactor, pROI)
      {}

  void BackprojectView (const double* const t, const double view_angle);

  virtual ~BackprojectEquilinear()
      {}
};

class BackprojectEquiangular : public BackprojectTable
{
 public:
  BackprojectEquiangular (const Projections& proj, ImageFile& im, int interpID, const int interpFactor,  const ReconstructionROI* pROI)
      : BackprojectTable (proj, im, interpID, interpFactor, pROI)
      {}

  void BackprojectView (const double* const t, const double view_angle);

  virtual ~BackprojectEquiangular()
      {}
};


#endif
