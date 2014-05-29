/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:         backprojectors.cpp         Classes for backprojection
**   Programmer:   Kevin Rosenberg
**   Date Started: June 2000
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
#include "interpolator.h"

const int Backprojector::BPROJ_INVALID = -1;
const int Backprojector::BPROJ_TRIG = 0;
const int Backprojector::BPROJ_TABLE = 1;
const int Backprojector::BPROJ_DIFF = 2;
const int Backprojector::BPROJ_IDIFF = 3;

const char* const Backprojector::s_aszBackprojectName[] =
{
  "trig",
  "table",
  "diff",
  "idiff",
};

const char* const Backprojector::s_aszBackprojectTitle[] =
{
  "Direct Trigometric",
  "Trigometric Table",
  "Difference Iteration",
  "Integer Difference Iteration",
};

const int Backprojector::s_iBackprojectCount = sizeof(s_aszBackprojectName) / sizeof(const char*);

const int Backprojector::INTERP_INVALID = -1;
const int Backprojector::INTERP_NEAREST = 0;
const int Backprojector::INTERP_LINEAR = 1;
const int Backprojector::INTERP_CUBIC = 2;
const int Backprojector::INTERP_FREQ_PREINTERPOLATION = 3;
#if HAVE_BSPLINE_INTERP
const int Backprojector::INTERP_BSPLINE = 4;
const int Backprojector::INTERP_1BSPLINE = 5;
const int Backprojector::INTERP_2BSPLINE = 6;
const int Backprojector::INTERP_3BSPLINE = 7;
#endif

const char* const Backprojector::s_aszInterpName[] =
{
  "nearest",
  "linear",
  "cubic",
#if HAVE_FREQ_PREINTERP
  "freq_preinterpolationj",
#endif
#if HAVE_BSPLINE_INTERP
  "bspline",
  "1bspline",
  "2bspline",
  "3bspline",
#endif
};

const char* const Backprojector::s_aszInterpTitle[] =
{
  "Nearest",
  "Linear",
  "Cubic",
#if HAVE_FREQ_PREINTERP
  "Frequency Preinterpolation",
#endif
#if HAVE_BSPLINE_INTERP
  "B-Spline",
  "B-Spline 1st Order",
  "B-Spline 2nd Order",
  "B-Spline 3rd Order",
#endif
};

const int Backprojector::s_iInterpCount = sizeof(s_aszInterpName) / sizeof(const char*);



Backprojector::Backprojector (const Projections& proj, ImageFile& im, const char* const backprojName,
                              const char* const interpName, const int interpFactor, const ReconstructionROI* pROI)
{
  m_fail = false;
  m_pBackprojectImplem = NULL;

  initBackprojector (proj, im, backprojName, interpName, interpFactor, pROI);
}

void
Backprojector::BackprojectView (const double* const viewData, const double viewAngle)
{
  if (m_pBackprojectImplem != NULL)
    m_pBackprojectImplem->BackprojectView (viewData, viewAngle);
}

void
Backprojector::PostProcessing()
{
  if (m_pBackprojectImplem != NULL)
    m_pBackprojectImplem->PostProcessing();
}

Backprojector::~Backprojector ()
{
  delete m_pBackprojectImplem;
}

// FUNCTION IDENTIFICATION
//     Backproject* projector = selectBackprojector (...)
//
// PURPOSE
//     Selects a backprojector based on BackprojType
//     and initializes the backprojector

bool
Backprojector::initBackprojector (const Projections& proj, ImageFile& im, const char* const backprojName,
                                  const char* const interpName, const int interpFactor, const ReconstructionROI* pROI)
{
  m_nameBackproject = backprojName;
  m_nameInterpolation = interpName;
  m_pBackprojectImplem = NULL;
  m_idBackproject = convertBackprojectNameToID (backprojName);
  if (m_idBackproject == BPROJ_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid backprojection name ";
    m_failMessage += backprojName;
  }
  m_idInterpolation = convertInterpNameToID (interpName);
  if (m_idInterpolation == INTERP_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid interpolation name ";
    m_failMessage += interpName;
  }

  if (m_fail || m_idBackproject == BPROJ_INVALID || m_idInterpolation == INTERP_INVALID) {
    m_fail = true;
    return false;
  }

  if (proj.geometry() == Scanner::GEOMETRY_EQUILINEAR)
    m_pBackprojectImplem = static_cast<Backproject*>(new BackprojectEquilinear(proj, im, m_idInterpolation, interpFactor, pROI));
  else if (proj.geometry() == Scanner::GEOMETRY_EQUIANGULAR)
    m_pBackprojectImplem = static_cast<Backproject*>(new BackprojectEquiangular(proj, im, m_idInterpolation, interpFactor, pROI));
  else if (proj.geometry() == Scanner::GEOMETRY_PARALLEL) {
    if (m_idBackproject == BPROJ_TRIG)
      m_pBackprojectImplem = static_cast<Backproject*>(new BackprojectTrig (proj, im, m_idInterpolation, interpFactor, pROI));
    else if (m_idBackproject == BPROJ_TABLE)
      m_pBackprojectImplem = static_cast<Backproject*>(new BackprojectTable (proj, im, m_idInterpolation, interpFactor, pROI));
    else if (m_idBackproject == BPROJ_DIFF)
      m_pBackprojectImplem = static_cast<Backproject*>(new BackprojectDiff (proj, im, m_idInterpolation, interpFactor, pROI));
    else if (m_idBackproject == BPROJ_IDIFF)
      m_pBackprojectImplem = static_cast<Backproject*>(new BackprojectIntDiff (proj, im, m_idInterpolation, interpFactor, pROI));
  } else {
    m_fail = true;
    m_failMessage = "Unable to select a backprojection method [Backprojector::initBackprojector]";
    return false;
  }

  return true;
}


int
Backprojector::convertBackprojectNameToID (const char* const backprojName)
{
  int backprojID = BPROJ_INVALID;

  for (int i = 0; i < s_iBackprojectCount; i++)
    if (strcasecmp (backprojName, s_aszBackprojectName[i]) == 0) {
      backprojID = i;
      break;
    }

    return (backprojID);
}

const char*
Backprojector::convertBackprojectIDToName (int bprojID)
{
  static const char *bprojName = "";

  if (bprojID >= 0 && bprojID < s_iBackprojectCount)
    return (s_aszBackprojectName[bprojID]);

  return (bprojName);
}

const char*
Backprojector::convertBackprojectIDToTitle (const int bprojID)
{
  static const char *bprojTitle = "";

  if (bprojID >= 0 && bprojID < s_iBackprojectCount)
    return (s_aszBackprojectTitle[bprojID]);

  return (bprojTitle);
}


int
Backprojector::convertInterpNameToID (const char* const interpName)
{
  int interpID = INTERP_INVALID;

  for (int i = 0; i < s_iInterpCount; i++)
    if (strcasecmp (interpName, s_aszInterpName[i]) == 0) {
      interpID = i;
      break;
    }

    return (interpID);
}

const char*
Backprojector::convertInterpIDToName (const int interpID)
{
  static const char *interpName = "";

  if (interpID >= 0 && interpID < s_iInterpCount)
    return (s_aszInterpName[interpID]);

  return (interpName);
}

const char*
Backprojector::convertInterpIDToTitle (const int interpID)
{
  static const char *interpTitle = "";

  if (interpID >= 0 && interpID < s_iInterpCount)
    return (s_aszInterpTitle[interpID]);

  return (interpTitle);
}



// CLASS IDENTICATION
//   Backproject
//
// PURPOSE
//   Pure virtual base class for all backprojectors.

Backproject::Backproject (const Projections& proj, ImageFile& im, int interpType, const int interpFactor,
                          const ReconstructionROI* pROI)
: proj(proj), im(im), interpType(interpType), m_interpFactor(interpFactor), m_bPostProcessingDone(false)
{
  detInc = proj.detInc();
  nDet = proj.nDet();
  iDetCenter = (nDet - 1) / 2;  // index refering to L=0 projection

  if (proj.geometry() == Scanner::GEOMETRY_PARALLEL)
    rotScale = PI / proj.nView(); // scale by number of PI rotations
  else if (proj.geometry() == Scanner::GEOMETRY_EQUIANGULAR || proj.geometry() == Scanner::GEOMETRY_EQUILINEAR)
    rotScale =  (2 * PI) / proj.nView(); // scale by number of 2PI rotations
  else
    sys_error (ERR_SEVERE, "Invalid geometry type %d [Backproject::Backproject]", proj.geometry());

  v = im.getArray();
  nx = im.nx();
  ny = im.ny();
  im.arrayDataClear();

  xMin = -proj.phmLen() / 2;      // Retangular coords of phantom
  xMax = xMin + proj.phmLen();
  yMin = -proj.phmLen() / 2;
  yMax = yMin + proj.phmLen();

  if (pROI) {
    if (pROI->m_dXMin > xMin)
      xMin = pROI->m_dXMin;
    if (pROI->m_dXMax < xMax)
      xMax = pROI->m_dXMax;
    if (pROI->m_dYMin > yMin)
      yMin = pROI->m_dYMin;
    if (pROI->m_dYMax < yMax)
      yMax = pROI->m_dYMax;

    if (xMin > xMax) {
      double temp = xMin;
      xMin = xMax;
      xMax = temp;
    }
    if (yMin > yMax) {
      double temp = yMin;
      yMin = yMax;
      yMax = temp;
    }
  }

  xInc = (xMax - xMin) / nx;    // size of cells
  yInc = (yMax - yMin) / ny;

  im.setAxisIncrement (xInc, yInc);
  im.setAxisExtent (xMin, xMax, yMin, yMax);

  m_dFocalLength = proj.focalLength();
  m_dSourceDetectorLength = proj.sourceDetectorLength();
}

Backproject::~Backproject ()
{}

void
Backproject::PostProcessing()
{
  m_bPostProcessingDone = true;
}

void
Backproject::ScaleImageByRotIncrement ()
{
  for (int ix = 0; ix < nx; ix++)
    for (int iy = 0; iy < ny; iy++)
      v[ix][iy] *= rotScale;
}

void Backproject::errorIndexOutsideDetector (int ix, int iy, double theta, double r, double phi, double L, int iDetPos)
{
  sys_error (ERR_WARNING, "r=%f, phi=%f", r, phi);
  errorIndexOutsideDetector (ix, iy, theta, L, iDetPos);
}

void Backproject::errorIndexOutsideDetector (int ix, int iy, double theta, double L, int iDetPos)
{
#if 1
  std::ostringstream os;
  os << "ix=" << ix << ", iy=" << iy << ", theta=" << theta << ", L=" << L << ", detinc=" << detInc << "\n";
  os << "ndet=" << nDet << ", detInc=" << detInc << ", iDetCenter=" << iDetCenter << "\n";
  os << "xMin=" << xMin << ", xMax=" << xMax << ", xInc=" << xInc << "\n";
  os << "yMin=" << yMin << ", yMax=" << yMax << ", yInc=" << yInc << "\n";
  os << "iDetPos index outside bounds: " << iDetPos << " [backprojector]";;

  sys_error (ERR_WARNING, os.str().c_str());
#endif
}


// CLASS IDENTICATION
//   BackprojectTrig
//
// PURPOSE
//   Uses trigometric functions at each point in image for backprojection.

void
BackprojectTrig::BackprojectView (const double* const filteredProj, const double view_angle)
{
  double theta = view_angle;

  CubicPolyInterpolator* pCubicInterp = NULL;
  if (interpType == Backprojector::INTERP_CUBIC)
    pCubicInterp = new CubicPolyInterpolator (filteredProj, nDet);

  double x = xMin + xInc / 2;   // Rectang coords of center of pixel
  for (int ix = 0; ix < nx; x += xInc, ix++) {
    double y = yMin + yInc / 2;
    for (int iy = 0; iy < ny; y += yInc, iy++) {
      double r = sqrt (x * x + y * y);   // distance of cell from center
      double phi = atan2 (y, x);         // angle of cell from center
      double L = r * cos (theta - phi);  // position on detector

      if (interpType == Backprojector::INTERP_NEAREST) {
        int iDetPos = iDetCenter + nearest<int> (L / detInc); // calc'd index in the filter raysum array

        if (iDetPos >= 0 && iDetPos < nDet)
          v[ix][iy] += rotScale * filteredProj[iDetPos];
      } else if (interpType == Backprojector::INTERP_LINEAR) {
        double p = L / detInc;  // position along detector
        double pFloor = floor (p);
        int iDetPos = iDetCenter + static_cast<int>(pFloor);
        double frac = p - pFloor;       // fraction distance from det
        if (iDetPos >= 0 && iDetPos < nDet - 1)
          v[ix][iy] += rotScale * ((1-frac) * filteredProj[iDetPos] + frac * filteredProj[iDetPos+1]);
      } else if (interpType == Backprojector::INTERP_CUBIC) {
        double p = iDetCenter + (L / detInc);   // position along detector
        if (p >= 0 && p < nDet)
          v[ix][iy] += rotScale * pCubicInterp->interpolate (p);
      }
    }
  }

  if (interpType == Backprojector::INTERP_CUBIC)
    delete pCubicInterp;
}


// CLASS IDENTICATION
//   BackprojectTable
//
// PURPOSE
//   Precalculates trigometric function value for each point in image for backprojection.

BackprojectTable::BackprojectTable (const Projections& proj, ImageFile& im, int interpType,
                                    const int interpFactor, const ReconstructionROI* pROI)
: Backproject (proj, im, interpType, interpFactor, pROI)
{
  arrayR.initSetSize (im.nx(), im.ny());
  arrayPhi.initSetSize (im.nx(), im.ny());
  r = arrayR.getArray();
  phi = arrayPhi.getArray();

  double x, y;                  // Rectang coords of center of pixel
  int ix, iy;
  for (x = xMin + xInc / 2, ix = 0; ix < nx; x += xInc, ix++)
    for (y = yMin + yInc / 2, iy = 0; iy < ny; y += yInc, iy++) {
      r[ix][iy] = sqrt (x * x + y * y);
      phi[ix][iy] = atan2 (y, x);
    }
}

BackprojectTable::~BackprojectTable ()
{
}

void
BackprojectTable::PostProcessing()
{
  if (! m_bPostProcessingDone) {
    ScaleImageByRotIncrement();
    m_bPostProcessingDone = true;
  }
}

void
BackprojectTable::BackprojectView (const double* const filteredProj, const double view_angle)
{
  double theta = view_angle;

  CubicPolyInterpolator* pCubicInterp = NULL;
  if (interpType == Backprojector::INTERP_CUBIC)
    pCubicInterp = new CubicPolyInterpolator (filteredProj, nDet);

  for (int ix = 0; ix < nx; ix++) {
    ImageFileColumn pImCol = v[ix];

    for (int iy = 0; iy < ny; iy++) {
      double L = r[ix][iy] * cos (theta - phi[ix][iy]);

      if (interpType == Backprojector::INTERP_NEAREST) {
        int iDetPos = iDetCenter + nearest<int>(L / detInc);    // calc index in the filtered raysum vector

        if (iDetPos >= 0 && iDetPos < nDet)
          pImCol[iy] += filteredProj[iDetPos];
      } else if (interpType == Backprojector::INTERP_LINEAR) {
        double dPos = L / detInc;               // position along detector
        double dPosFloor = floor (dPos);
        int iDetPos = iDetCenter + static_cast<int>(dPosFloor);
        double frac = dPos - dPosFloor; // fraction distance from det
        if (iDetPos >= 0 && iDetPos < nDet - 1)
          pImCol[iy] += ((1-frac) * filteredProj[iDetPos] + frac * filteredProj[iDetPos+1]);
      } else if (interpType == Backprojector::INTERP_CUBIC) {
        double p = iDetCenter + (L / detInc);   // position along detector
        if (p >= 0 && p < nDet)
          pImCol[iy] += pCubicInterp->interpolate (p);
      }
    }   // end for y
  }     // end for x

  if (interpType == Backprojector::INTERP_CUBIC)
    delete pCubicInterp;
}


// CLASS IDENTICATION
//   BackprojectDiff
//
// PURPOSE
//   Backprojects by precalculating the change in L position for each x & y step in the image.
//   Iterates in x & y direction by adding difference in L position

BackprojectDiff::BackprojectDiff (const Projections& proj, ImageFile& im, int interpType,
                                  const int interpFactor, const ReconstructionROI* pROI)
:  Backproject (proj, im, interpType, interpFactor, pROI)
{
  // calculate center of first pixel v[0][0]
  double x = xMin + xInc / 2;
  double y = yMin + yInc / 2;
  start_r = sqrt (x * x + y * y);
  start_phi = atan2 (y, x);

  im.arrayDataClear();
}

BackprojectDiff::~BackprojectDiff ()
{
}

void
BackprojectDiff::PostProcessing()
{
  if (! m_bPostProcessingDone) {
    ScaleImageByRotIncrement();
    m_bPostProcessingDone = true;
  }
}

void
BackprojectDiff::BackprojectView (const double* const filteredProj, const double view_angle)
{
  double theta = view_angle;

  // Distance between detectors for an angle given in units of detectors
  double det_dx = xInc * cos (theta) / detInc;
  double det_dy = yInc * sin (theta) / detInc;

  // calculate detPosition for first point in image (ix=0, iy=0)
  double detPosColStart = iDetCenter + start_r * cos (theta - start_phi) / detInc;

  CubicPolyInterpolator* pCubicInterp = NULL;
  double* deltaFilteredProj = NULL;
  if (interpType == Backprojector::INTERP_LINEAR) {
    // precalculate scaled difference for linear interpolation
    deltaFilteredProj = new double [nDet];
    for (int i = 0; i < nDet - 1; i++)
      deltaFilteredProj[i] = filteredProj[i+1] - filteredProj[i];
    deltaFilteredProj[nDet - 1] = 0;  // last detector
  } else if (interpType == Backprojector::INTERP_CUBIC) {
    pCubicInterp = new CubicPolyInterpolator (filteredProj, nDet);
  }

  int iLastDet = nDet - 1;
  for (int ix = 0; ix < nx; ix++, detPosColStart += det_dx) {
    double curDetPos = detPosColStart;
    ImageFileColumn pImCol = v[ix];

    for (int iy = 0; iy < ny; iy++, curDetPos += det_dy) {
      if (interpType == Backprojector::INTERP_NEAREST) {
        int iDetPos = nearest<int> (curDetPos); // calc index in the filtered raysum vector

        if (iDetPos >= 0 && iDetPos < nDet)
          *pImCol++ += filteredProj[iDetPos];
      } else if (interpType == Backprojector::INTERP_LINEAR) {
        double detPosFloor = floor (curDetPos);
        int iDetPos = static_cast<int>(detPosFloor);
        double frac = curDetPos - detPosFloor;  // fraction distance from det
        if (iDetPos >= 0 && iDetPos <= iLastDet)
            *pImCol++ += filteredProj[iDetPos] + (frac * deltaFilteredProj[iDetPos]);
      } else if (interpType == Backprojector::INTERP_CUBIC) {
        double p = curDetPos;   // position along detector
        if (p >= 0 && p < nDet)
          *pImCol++  += pCubicInterp->interpolate (p);
      }
    }   // end for y
  }     // end for x

  if (interpType == Backprojector::INTERP_LINEAR)
    delete deltaFilteredProj;
  else if (interpType == Backprojector::INTERP_CUBIC)
    delete pCubicInterp;
}


// CLASS IDENTICATION
//   BackprojectIntDiff
//
// PURPOSE
//   Highly optimized and integer version of BackprojectDiff

void
BackprojectIntDiff::BackprojectView (const double* const filteredProj, const double view_angle)
{
  double theta = view_angle;  // add half PI to view angle to get perpendicular theta angle
#if SIZEOF_LONG == 4
  static const int scaleShift = 16;
#elif SIZEOF_LONG == 8
  static const int scaleShift = 32;
#endif
  static const long scale = (1L << scaleShift);
  static const long scaleBitmask = scale - 1;
  static const long halfScale = scale / 2;
  static const double dInvScale = 1. / scale;

  const long det_dx = nearest<long> (xInc * cos (theta) / detInc * scale);
  const long det_dy = nearest<long> (yInc * sin (theta) / detInc * scale);

  // calculate L for first point in image (0, 0)
  long detPosColStart = nearest<long> ((start_r * cos (theta - start_phi) / detInc + iDetCenter) * scale);

  double* deltaFilteredProj = NULL;
  CubicPolyInterpolator* pCubicInterp = NULL;
  if (interpType == Backprojector::INTERP_LINEAR) {
    // precalculate scaled difference for linear interpolation
    deltaFilteredProj = new double [nDet];
    for (int i = 0; i < nDet - 1; i++)
      deltaFilteredProj[i] = (filteredProj[i+1] - filteredProj[i]) * dInvScale;
    deltaFilteredProj[nDet - 1] = 0;  // last detector
  } else if (interpType == Backprojector::INTERP_CUBIC) {
    pCubicInterp = new CubicPolyInterpolator (filteredProj, nDet);
  }

  int iLastDet = nDet - 1;
  for (int ix = 0; ix < nx; ix++, detPosColStart += det_dx) {
    long curDetPos = detPosColStart;
    ImageFileColumn pImCol = v[ix];

    if (interpType == Backprojector::INTERP_NEAREST) {
      for (int iy = 0; iy < ny; iy++, curDetPos += det_dy) {
        const int iDetPos = (curDetPos + halfScale) >> scaleShift;
        if (iDetPos >= 0 && iDetPos <= iLastDet)
          *pImCol++ += filteredProj[iDetPos];
        else
          pImCol++;

      } // end for iy
    } else if (interpType == Backprojector::INTERP_FREQ_PREINTERPOLATION) {
      for (int iy = 0; iy < ny; iy++, curDetPos += det_dy) {
        const int iDetPos = ((curDetPos + halfScale) >> scaleShift) * m_interpFactor;
        if (iDetPos >= 0 && iDetPos <= iLastDet)
          *pImCol++ += filteredProj[iDetPos];
        else
          pImCol++;
      } // end for iy
    } else if (interpType == Backprojector::INTERP_LINEAR) {
      for (int iy = 0; iy < ny; iy++, curDetPos += det_dy) {
        const long iDetPos = curDetPos >> scaleShift;
        if (iDetPos >= 0 && iDetPos <= iLastDet) {
          const long detRemainder = curDetPos & scaleBitmask;
          *pImCol++ += filteredProj[iDetPos] + (detRemainder * deltaFilteredProj[iDetPos]);
        } else
          pImCol++;
      } // end for iy
    } else if (interpType == Backprojector::INTERP_CUBIC) {
      for (int iy = 0; iy < ny; iy++, curDetPos += det_dy) {
        *pImCol++ += pCubicInterp->interpolate (static_cast<double>(curDetPos) / scale);
      }
    } // end Cubic
  } // end for ix

  if (interpType == Backprojector::INTERP_LINEAR)
    delete deltaFilteredProj;
  else if (interpType == Backprojector::INTERP_CUBIC)
    delete pCubicInterp;
}


void
BackprojectEquiangular::BackprojectView (const double* const filteredProj, const double view_angle)
{
  double beta = view_angle;

  CubicPolyInterpolator* pCubicInterp = NULL;
  if (interpType == Backprojector::INTERP_CUBIC)
    pCubicInterp = new CubicPolyInterpolator (filteredProj, nDet);

  for (int ix = 0; ix < nx; ix++) {
    ImageFileColumn pImCol = v[ix];

    for (int iy = 0; iy < ny; iy++) {
      double dAngleDiff = beta - phi[ix][iy];
      double rcos_t = r[ix][iy] * cos (dAngleDiff);
      double rsin_t = r[ix][iy] * sin (dAngleDiff);
      double dFLPlusSin = m_dFocalLength + rsin_t;
      double gamma =  atan (rcos_t / dFLPlusSin);
      double dPos = gamma / detInc;  // position along detector
      double dL2 = dFLPlusSin * dFLPlusSin + (rcos_t * rcos_t);

      if (interpType == Backprojector::INTERP_NEAREST) {
        int iDetPos = iDetCenter + nearest<int>(dPos);  // calc index in the filtered raysum vector
        if (iDetPos >= 0 && iDetPos < nDet)
          pImCol[iy] += filteredProj[iDetPos] / dL2;
      } else if (interpType == Backprojector::INTERP_LINEAR) {
        double dPosFloor = floor (dPos);
        int iDetPos = iDetCenter + static_cast<int>(dPosFloor);
        double frac = dPos - dPosFloor; // fraction distance from det
        if (iDetPos >= 0 && iDetPos < nDet - 1)
          pImCol[iy] += (filteredProj[iDetPos] + frac * (filteredProj[iDetPos+1] - filteredProj[iDetPos])) / dL2;
      } else if (interpType == Backprojector::INTERP_CUBIC) {
        double d = iDetCenter + dPos;           // position along detector
        if (d >= 0 && d < nDet)
          pImCol[iy] += pCubicInterp->interpolate (d) / dL2;
      }
    }   // end for y
  }     // end for x

  if (interpType == Backprojector::INTERP_CUBIC)
    delete pCubicInterp;
}

void
BackprojectEquilinear::BackprojectView (const double* const filteredProj, const double view_angle)
{
  double beta = view_angle;

  CubicPolyInterpolator* pCubicInterp = NULL;
  if (interpType == Backprojector::INTERP_CUBIC)
    pCubicInterp = new CubicPolyInterpolator (filteredProj, nDet);

  for (int ix = 0; ix < nx; ix++) {
    ImageFileColumn pImCol = v[ix];

    for (int iy = 0; iy < ny; iy++) {
      double dAngleDiff = beta - phi[ix][iy];
      double rcos_t = r[ix][iy] * cos (dAngleDiff);
      double rsin_t = r[ix][iy] * sin (dAngleDiff);

      double dU = (m_dFocalLength + rsin_t) / m_dFocalLength;
      double dDetPos =  rcos_t / dU;
      // Scale for imaginary detector that passes through origin of phantom, see Kak-Slaney Figure 3.22.
      dDetPos *= m_dSourceDetectorLength / m_dFocalLength;
      double dPos = dDetPos / detInc;  // position along detector array

      if (interpType == Backprojector::INTERP_NEAREST) {
        int iDetPos = iDetCenter + nearest<int>(dPos);  // calc index in the filtered raysum vector
        if (iDetPos >= 0 && iDetPos < nDet)
          pImCol[iy] += (filteredProj[iDetPos] / (dU * dU));
      } else if (interpType == Backprojector::INTERP_LINEAR) {
        double dPosFloor = floor (dPos);
        int iDetPos = iDetCenter + static_cast<int>(dPosFloor);
        double frac = dPos - dPosFloor; // fraction distance from det
        if (iDetPos >= 0 && iDetPos < nDet - 1)
          pImCol[iy] += (filteredProj[iDetPos] + frac * (filteredProj[iDetPos+1] - filteredProj[iDetPos]))
                           / (dU * dU);
      } else if (interpType == Backprojector::INTERP_CUBIC) {
        double d = iDetCenter + dPos;           // position along detector
        if (d >= 0 && d < nDet)
          pImCol[iy] += pCubicInterp->interpolate (d) / (dU * dU);
      }
    }   // end for y
  }     // end for x

  if (interpType == Backprojector::INTERP_CUBIC)
    delete pCubicInterp;
}

