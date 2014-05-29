/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          scanner.cpp
**   Purpose:       Classes for CT scanner
**   Programmer:    Kevin Rosenberg
**   Date Started:  1984
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


const int Scanner::GEOMETRY_INVALID = -1;
const int Scanner::GEOMETRY_PARALLEL = 0;
const int Scanner::GEOMETRY_EQUIANGULAR = 1;
const int Scanner::GEOMETRY_EQUILINEAR = 2;
const int Scanner::GEOMETRY_LINOGRAM = 3;

const char* const Scanner::s_aszGeometryName[] =
{
  "parallel",
  "equiangular",
  "equilinear",
  "linogram",
};

const char* const Scanner::s_aszGeometryTitle[] =
{
  "Parallel",
  "Equiangular",
  "Equilinear",
  "Linogram",
};

const int Scanner::s_iGeometryCount = sizeof(s_aszGeometryName) / sizeof(const char*);


// NAME
//   DetectorArray       Construct a DetectorArray

DetectorArray::DetectorArray (const int nDet)
{
  m_nDet = nDet;
  m_detValues = new DetectorValue [m_nDet];
}


// NAME
//   ~DetectorArray             Free memory allocated to a detector array

DetectorArray::~DetectorArray (void)
{
  delete [] m_detValues;
}



/* NAME
*   Scanner::Scanner            Construct a user specified detector structure
*
* SYNOPSIS
*   Scanner (phm, nDet, nView, nSample)
*   Phantom& phm                PHANTOM that we are making detector for
*   int geomety                Geometry of detector
*   int nDet                    Number of detector along detector array
*   int nView                   Number of rotated views
*   int nSample         Number of rays per detector
*/

Scanner::Scanner (const Phantom& phm, const char* const geometryName,
                  int nDet, int nView, int offsetView,
                                  int nSample, const double rot_anglen,
                  const double dFocalLengthRatio,
                                  const double dCenterDetectorRatio,
                  const double dViewRatio, const double dScanRatio)
{
  m_fail = false;
  m_idGeometry = convertGeometryNameToID (geometryName);
  if (m_idGeometry == GEOMETRY_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid geometry name ";
    m_failMessage += geometryName;
    return;
  }

  if (nView < 1 || nDet < 1) {
    m_fail = true;
    m_failMessage = "nView & nDet must be greater than 0";
    return;
  }
  if (nSample < 1)
    m_nSample = 1;

  m_nDet     = nDet;
  m_nView    = nView;
  m_iOffsetView = offsetView;
  m_nSample  = nSample;
  m_dFocalLengthRatio = dFocalLengthRatio;
  m_dCenterDetectorRatio = dCenterDetectorRatio;
  m_dViewRatio = dViewRatio;
  m_dScanRatio = dScanRatio;

  m_dViewDiameter = phm.getDiameterBoundaryCircle() * m_dViewRatio;
  m_dFocalLength = (m_dViewDiameter / 2) * m_dFocalLengthRatio;
  m_dCenterDetectorLength = (m_dViewDiameter / 2) * m_dCenterDetectorRatio;
  m_dSourceDetectorLength = m_dFocalLength + m_dCenterDetectorLength;
  m_dScanDiameter = m_dViewDiameter * m_dScanRatio;

  m_dXCenter = phm.xmin() + (phm.xmax() - phm.xmin()) / 2;
  m_dYCenter = phm.ymin() + (phm.ymax() - phm.ymin()) / 2;
  m_rotLen  = rot_anglen;
  m_rotInc  = m_rotLen / m_nView;
  if (m_idGeometry == GEOMETRY_PARALLEL) {
    m_dFanBeamAngle = 0;
    m_detLen   = m_dScanDiameter;
    m_detStart = -m_detLen / 2;
    m_detInc  = m_detLen / m_nDet;
    double dDetectorArrayEndOffset = 0;
    // For even number of detectors, make detInc slightly larger so that center lies
    // at nDet/2. Also, extend detector array by one detInc so that all of the phantom is scanned
    if (isEven (m_nDet)) { // Adjust for Even number of detectors
      m_detInc = m_detLen / (m_nDet - 1); // center detector = (nDet/2)
      dDetectorArrayEndOffset = m_detInc;
    }

    double dHalfDetLen = m_detLen / 2;
    m_initPos.xs1 = m_dXCenter - dHalfDetLen;
    m_initPos.ys1 = m_dYCenter + m_dFocalLength;
    m_initPos.xs2 = m_dXCenter + dHalfDetLen + dDetectorArrayEndOffset;
    m_initPos.ys2 = m_dYCenter + m_dFocalLength;
    m_initPos.xd1 = m_dXCenter - dHalfDetLen;
    m_initPos.yd1 = m_dYCenter - m_dCenterDetectorLength;
    m_initPos.xd2 = m_dXCenter + dHalfDetLen + dDetectorArrayEndOffset;
    m_initPos.yd2 = m_dYCenter - m_dCenterDetectorLength;
    m_initPos.angle = m_iOffsetView * m_rotInc;
    m_detLen += dDetectorArrayEndOffset;
  } else if (m_idGeometry == GEOMETRY_EQUILINEAR) {
  if (m_dScanDiameter / 2 >= m_dFocalLength) {
      m_fail = true;
      m_failMessage = "Invalid geometry: Focal length must be larger than scan length";
      return;
    }

    const double dAngle = asin ((m_dScanDiameter / 2) / m_dFocalLength);
    const double dHalfDetLen = m_dSourceDetectorLength * tan (dAngle);

    m_detLen = dHalfDetLen * 2;
    m_detStart = -dHalfDetLen;
    m_detInc  = m_detLen / m_nDet;
    double dDetectorArrayEndOffset = 0;
    if (isEven (m_nDet)) { // Adjust for Even number of detectors
      m_detInc = m_detLen / (m_nDet - 1); // center detector = (nDet/2)
      dDetectorArrayEndOffset = m_detInc;
      m_detLen += dDetectorArrayEndOffset;
    }

    m_dFanBeamAngle = dAngle * 2;
    m_initPos.xs1 = m_dXCenter;
    m_initPos.ys1 = m_dYCenter + m_dFocalLength;
    m_initPos.xs2 = m_dXCenter;
    m_initPos.ys2 = m_dYCenter + m_dFocalLength;
    m_initPos.xd1 = m_dXCenter - dHalfDetLen;
    m_initPos.yd1 = m_dYCenter - m_dCenterDetectorLength;
    m_initPos.xd2 = m_dXCenter + dHalfDetLen + dDetectorArrayEndOffset;
    m_initPos.yd2 = m_dYCenter - m_dCenterDetectorLength;
    m_initPos.angle = m_iOffsetView * m_rotInc;
  } else if (m_idGeometry == GEOMETRY_EQUIANGULAR) {
    if (m_dScanDiameter / 2 > m_dFocalLength) {
      m_fail = true;
      m_failMessage = "Invalid geometry: Focal length must be larger than scan length";
      return;
    }
    const double dAngle = asin ((m_dScanDiameter / 2) / m_dFocalLength);

    m_detLen = 2 * dAngle;
    m_detStart = -dAngle;
    m_detInc = m_detLen / m_nDet;
    double dDetectorArrayEndOffset = 0;
    if (isEven (m_nDet)) { // Adjust for Even number of detectors
      m_detInc = m_detLen / (m_nDet - 1); // center detector = (nDet/2)
      dDetectorArrayEndOffset = m_detInc;
    }
    // adjust for center-detector length
    double dA1 = acos ((m_dScanDiameter / 2) / m_dCenterDetectorLength);
    double dAngularScale = 2 * (HALFPI + dAngle - dA1) / m_detLen;

    m_dAngularDetLen = dAngularScale * (m_detLen + dDetectorArrayEndOffset);
    m_dAngularDetIncrement = dAngularScale * m_detInc;
    m_initPos.dAngularDet = -m_dAngularDetLen / 2;

    m_dFanBeamAngle = dAngle * 2;
    m_initPos.angle = m_iOffsetView * m_rotInc;
    m_initPos.xs1 = m_dXCenter;
    m_initPos.ys1 = m_dYCenter + m_dFocalLength;;
    m_initPos.xs2 = m_dXCenter;
    m_initPos.ys2 = m_dYCenter + m_dFocalLength;
    m_detLen += dDetectorArrayEndOffset;
  }

  // Calculate incrementatal rotation matrix
  GRFMTX_2D temp;
  xlat_mtx2 (m_rotmtxIncrement, -m_dXCenter, -m_dYCenter);
  rot_mtx2 (temp, m_rotInc);
  mult_mtx2 (m_rotmtxIncrement, temp, m_rotmtxIncrement);
  xlat_mtx2 (temp, m_dXCenter, m_dYCenter);
  mult_mtx2 (m_rotmtxIncrement, temp, m_rotmtxIncrement);

}

Scanner::~Scanner (void)
{
}


const char*
Scanner::convertGeometryIDToName (const int geomID)
{
  const char *name = "";

  if (geomID >= 0 && geomID < s_iGeometryCount)
    return (s_aszGeometryName[geomID]);

  return (name);
}

const char*
Scanner::convertGeometryIDToTitle (const int geomID)
{
  const char *title = "";

  if (geomID >= 0 && geomID < s_iGeometryCount)
    return (s_aszGeometryName[geomID]);

  return (title);
}

int
Scanner::convertGeometryNameToID (const char* const geomName)
{
  int id = GEOMETRY_INVALID;

  for (int i = 0; i < s_iGeometryCount; i++)
    if (strcasecmp (geomName, s_aszGeometryName[i]) == 0) {
      id = i;
      break;
    }

    return (id);
}


/* NAME
*   collectProjections          Calculate projections for a Phantom
*
* SYNOPSIS
*   collectProjections (proj, phm, start_view, nView, bStoreViewPos, trace)
*   Projectrions& proj      Projection storage
*   Phantom& phm             Phantom for which we collect projections
*   bool bStoreViewPos      TRUE then storage proj at normal view position
*   int trace                Trace level
*/


void
Scanner::collectProjections (Projections& proj, const Phantom& phm, const int trace, SGP* pSGP)
{
  collectProjections (proj, phm, m_startView, proj.nView(), m_iOffsetView, true, trace, pSGP);
}

void
Scanner::collectProjections (Projections& proj, const Phantom& phm, const int iStartView,
                             const int iNumViews, const int iOffsetView,  bool bStoreAtViewPosition,
                             const int trace, SGP* pSGP)
{
  int iStorageOffset = (bStoreAtViewPosition ? iStartView : 0);
  collectProjections (proj, phm, iStartView, iNumViews, iOffsetView, iStorageOffset, trace, pSGP);
}

void
Scanner::collectProjections (Projections& proj, const Phantom& phm, const int iStartView,
                             const int iNumViews, const int iOffsetView, int iStorageOffset,
                             const int trace, SGP* pSGP)
{
  m_trace = trace;
  double start_angle = (iStartView + iOffsetView) * proj.rotInc();

  // Calculate initial rotation matrix
  GRFMTX_2D rotmtx_initial, temp;
  xlat_mtx2 (rotmtx_initial, -m_dXCenter, -m_dYCenter);
  rot_mtx2 (temp, start_angle);
  mult_mtx2 (rotmtx_initial, temp, rotmtx_initial);
  xlat_mtx2 (temp, m_dXCenter, m_dYCenter);
  mult_mtx2 (rotmtx_initial, temp, rotmtx_initial);

  double xd1=0, yd1=0, xd2=0, yd2=0;
  if (m_idGeometry != GEOMETRY_EQUIANGULAR) {
    xd1 = m_initPos.xd1;
    yd1 = m_initPos.yd1;
    xd2 = m_initPos.xd2;
    yd2 = m_initPos.yd2;
    xform_mtx2 (rotmtx_initial, xd1, yd1);      // rotate detector endpoints
    xform_mtx2 (rotmtx_initial, xd2, yd2);      // to initial view_angle
  }

  double xs1 = m_initPos.xs1;
  double ys1 = m_initPos.ys1;
  double xs2 = m_initPos.xs2;
  double ys2 = m_initPos.ys2;
  xform_mtx2 (rotmtx_initial, xs1, ys1);      // rotate source endpoints to
  xform_mtx2 (rotmtx_initial, xs2, ys2);      // initial view angle

  int iView;
  double viewAngle;
  for (iView = 0, viewAngle = start_angle;  iView < iNumViews; iView++, viewAngle += proj.rotInc()) {
    int iStoragePosition = iView + iStorageOffset;

    DetectorArray& detArray = proj.getDetectorArray( iStoragePosition );

#ifdef HAVE_SGP
    if (pSGP && m_trace >= Trace::TRACE_PHANTOM) {
      m_pSGP = pSGP;
      double dWindowSize = dmax (m_detLen, m_dSourceDetectorLength) * 2;
      double dHalfWindowSize = dWindowSize / 2;
      m_dXMinWin = m_dXCenter - dHalfWindowSize;
      m_dXMaxWin = m_dXCenter + dHalfWindowSize;
      m_dYMinWin = m_dYCenter - dHalfWindowSize;
      m_dYMaxWin = m_dYCenter + dHalfWindowSize;

      m_pSGP->setWindow (m_dXMinWin, m_dYMinWin, m_dXMaxWin, m_dYMaxWin);
      m_pSGP->setRasterOp (RO_COPY);

      m_pSGP->setColor (C_RED);
      m_pSGP->moveAbs (0., 0.);
      m_pSGP->drawCircle (m_dViewDiameter / 2);

      m_pSGP->moveAbs (0., 0.);
      m_pSGP->setColor (C_GREEN);
      m_pSGP->drawCircle (m_dFocalLength);
      m_pSGP->setColor (C_BLUE);
      m_pSGP->setTextPointSize (9);
      phm.draw (*m_pSGP);
      m_dTextHeight = m_pSGP->getCharHeight ();

      traceShowParam ("Phantom:",       "%s", PROJECTION_TRACE_ROW_PHANT_ID, C_BLACK, phm.name().c_str());
      traceShowParam ("Geometry:", "%s", PROJECTION_TRACE_ROW_GEOMETRY, C_BLUE, convertGeometryIDToName(m_idGeometry));
      traceShowParam ("Focal Length Ratio:", "%.2f", PROJECTION_TRACE_ROW_FOCAL_LENGTH, C_BLUE, m_dFocalLengthRatio);
      //      traceShowParam ("Field Of View Ratio:", "%.2f", PROJECTION_TRACE_ROW_FIELD_OF_VIEW, C_BLUE, m_dFieldOfViewRatio);
      traceShowParam ("Num Detectors:", "%d", PROJECTION_TRACE_ROW_NDET, C_BLUE, proj.nDet());
      traceShowParam ("Num Views:", "%d", PROJECTION_TRACE_ROW_NVIEW, C_BLUE, proj.nView());
      traceShowParam ("Samples / Ray:", "%d", PROJECTION_TRACE_ROW_SAMPLES, C_BLUE, m_nSample);

      m_pSGP->setMarker (SGP::MARKER_BDIAMOND);
    }
#endif

#ifdef HAVE_SGP
    if (m_pSGP && m_trace >= Trace::TRACE_PHANTOM) {
      m_pSGP->setColor (C_BLACK);
      m_pSGP->setPenWidth (2);
      if (m_idGeometry == GEOMETRY_PARALLEL) {
        m_pSGP->moveAbs (xs1, ys1);
        m_pSGP->lineAbs (xs2, ys2);
        m_pSGP->moveAbs (xd1, yd1);
        m_pSGP->lineAbs (xd2, yd2);
      } else if (m_idGeometry == GEOMETRY_EQUILINEAR) {
        m_pSGP->setPenWidth (4);
        m_pSGP->moveAbs (xs1, ys1);
        m_pSGP->lineAbs (xs2, ys2);
        m_pSGP->setPenWidth (2);
        m_pSGP->moveAbs (xd1, yd1);
        m_pSGP->lineAbs (xd2, yd2);
      } else if (m_idGeometry == GEOMETRY_EQUIANGULAR) {
        m_pSGP->setPenWidth (4);
        m_pSGP->moveAbs (xs1, ys1);
        m_pSGP->lineAbs (xs2, ys2);
        m_pSGP->setPenWidth (2);
        m_pSGP->moveAbs (0., 0.);
        m_pSGP->drawArc (m_dCenterDetectorLength, viewAngle + 3 * HALFPI - (m_dAngularDetLen/2), viewAngle + 3 * HALFPI + (m_dAngularDetLen/2));
      }
      m_pSGP->setPenWidth (1);
    }
    if (m_trace > Trace::TRACE_CONSOLE)
      traceShowParam ("Current View:", "%d (%.0f%%)", PROJECTION_TRACE_ROW_CURR_VIEW, C_RED, iView + iStartView, (iView + iStartView) / static_cast<double>(m_nView) * 100.);
#endif
    if (m_trace == Trace::TRACE_CONSOLE)
      std::cout << "Current View: " << iView+iStartView << std::endl;

    projectSingleView (phm, detArray, xd1, yd1, xd2, yd2, xs1, ys1, xs2, ys2, viewAngle + 3 * HALFPI);
    detArray.setViewAngle (viewAngle);

#ifdef HAVE_SGP
    if (m_pSGP && m_trace >= Trace::TRACE_PHANTOM) {
      //        rs_plot (detArray, xd1, yd1, dXCenter, dYCenter, theta);
    }
#endif
    xform_mtx2 (m_rotmtxIncrement, xs1, ys1);
    xform_mtx2 (m_rotmtxIncrement, xs2, ys2);
    if (m_idGeometry != GEOMETRY_EQUIANGULAR) {
      xform_mtx2 (m_rotmtxIncrement, xd1, yd1);  // rotate detector endpoints
      xform_mtx2 (m_rotmtxIncrement, xd2, yd2);
    }
  } /* for each iView */
}


/* NAME
*    rayview                    Calculate raysums for a view at any angle
*
* SYNOPSIS
*    rayview (phm, detArray, xd1, nSample, yd1, xd2, yd2, xs1, ys1, xs2, ys2)
*    Phantom& phm               Phantom to scan
*    DETARRAY *detArray         Storage of values for detector array
*    Scanner& det               Scanner parameters
*    double xd1, yd1, xd2, yd2  Beginning & ending detector positions
*    double xs1, ys1, xs2, ys2  Beginning & ending source positions
*
* RAY POSITIONING
*         For each detector, have there are a variable number of rays traced.
*     The source of each ray is the center of the source x-ray cell. The
*     detector positions are equally spaced within the cell
*
*         The increments between rays are calculated so that the cells start
*     at the beginning of a detector cell and they end on the endpoint
*     of the cell.  Thus, the last cell starts at (xd2-ddx),(yd2-ddy).
*         The exception to this is if there is only one ray per detector.
*     In that case, the detector position is the center of the detector cell.
*/

void
Scanner::projectSingleView (const Phantom& phm, DetectorArray& detArray, const double xd1, const double yd1, const double xd2, const double yd2, const double xs1, const double ys1, const double xs2, const double ys2, const double dDetAngle)
{

  double sdx = (xs2 - xs1) / detArray.nDet();  // change in coords
  double sdy = (ys2 - ys1) / detArray.nDet();  // between source
  double xs_maj = xs1 + (sdx / 2);      // put ray source in center of cell
  double ys_maj = ys1 + (sdy / 2);

  double ddx=0, ddy=0, ddx2=0, ddy2=0, ddx2_ofs=0, ddy2_ofs=0, xd_maj=0, yd_maj=0;
  double dAngleInc=0, dAngleSampleInc=0, dAngleSampleOffset=0, dAngleMajor=0;
  if (m_idGeometry == GEOMETRY_EQUIANGULAR) {
    dAngleInc = m_dAngularDetIncrement;
    dAngleSampleInc = dAngleInc / m_nSample;
    dAngleSampleOffset = dAngleSampleInc / 2;
    dAngleMajor = dDetAngle - (m_dAngularDetLen/2) + dAngleSampleOffset;
  } else {
    ddx = (xd2 - xd1) / detArray.nDet();  // change in coords
    ddy = (yd2 - yd1) / detArray.nDet();  // between detectors
    ddx2 = ddx / m_nSample;     // Incr. between rays with detector cell
    ddy2 = ddy / m_nSample;  // Doesn't include detector endpoints
    ddx2_ofs = ddx2 / 2;    // offset of 1st ray from start of detector cell
    ddy2_ofs = ddy2 / 2;

    xd_maj = xd1 + ddx2_ofs;       // Incr. between detector cells
    yd_maj = yd1 + ddy2_ofs;
  }

  DetectorValue* detval = detArray.detValues();

  if (phm.getComposition() == P_UNIT_PULSE) {  // put unit pulse in center of view
    for (int d = 0; d < detArray.nDet(); d++)
        detval[d] = 0;
    detval[ detArray.nDet() / 2 ] = 1;
  } else {
    for (int d = 0; d < detArray.nDet(); d++) {
      double xs = xs_maj;
      double ys = ys_maj;
      double xd=0, yd=0, dAngle=0;
      if (m_idGeometry == GEOMETRY_EQUIANGULAR) {
        dAngle = dAngleMajor;
      } else {
        xd = xd_maj;
        yd = yd_maj;
      }
      double sum = 0.0;
      for (unsigned int i = 0; i < m_nSample; i++) {
        if (m_idGeometry == GEOMETRY_EQUIANGULAR) {
          xd = m_dCenterDetectorLength * cos (dAngle);
          yd = m_dCenterDetectorLength * sin (dAngle);
        }

#ifdef HAVE_SGP
        if (m_pSGP && m_trace >= Trace::TRACE_PROJECTIONS) {
          m_pSGP->setColor (C_YELLOW);
          m_pSGP->setRasterOp (RO_AND);
          m_pSGP->moveAbs (xs, ys);
          m_pSGP->lineAbs (xd, yd);
        }
#endif

        sum += projectSingleLine (phm, xd, yd, xs, ys);

#ifdef HAVE_SGP
        //      if (m_trace >= Trace::TRACE_CLIPPING) {
        //        traceShowParam ("Attenuation:", "%s", PROJECTION_TRACE_ROW_ATTEN, C_LTMAGENTA, "        ");
        //        traceShowParam ("Attenuation:", "%.3f", PROJECTION_TRACE_ROW_ATTEN, C_LTMAGENTA, sum);
        //      }
#endif
        if (m_idGeometry == GEOMETRY_EQUIANGULAR)
          dAngle += dAngleSampleInc;
        else {
          xd += ddx2;
          yd += ddy2;
        }
      } // for each sample in detector

      detval[d] = sum / m_nSample;
      xs_maj += sdx;
      ys_maj += sdy;
      if (m_idGeometry == GEOMETRY_EQUIANGULAR)
        dAngleMajor += dAngleInc;
      else {
        xd_maj += ddx;
        yd_maj += ddy;
      }
    } /* for each detector */
  } /* if not unit pulse */
}


void
Scanner::traceShowParam (const char *szLabel, const char *fmt, int row, int color, ...)
{
  va_list arg;
  va_start(arg, color);
#ifdef HAVE_SGP
  traceShowParamRasterOp (RO_COPY, szLabel, fmt, row, color, arg);
#else
  traceShowParamRasterOp (0, szLabel, fmt, row, color, arg);
#endif
  va_end(arg);
}

void
Scanner::traceShowParamXOR (const char *szLabel, const char *fmt, int row, int color, ...)
{
  va_list arg;
  va_start(arg, color);
#ifdef HAVE_SGP
  traceShowParamRasterOp (RO_XOR, szLabel, fmt, row, color, arg);
#else
  traceShowParamRasterOp (0, szLabel, fmt, row, color, arg);
#endif
  va_end(arg);
}

void
Scanner::traceShowParamRasterOp (int iRasterOp, const char *szLabel, const char *fmt, int row, int color, va_list args)
{
  char szValue[256];

  vsnprintf (szValue, sizeof(szValue), fmt, args);

#ifdef HAVE_SGP
  if (m_pSGP) {
    m_pSGP->setRasterOp (iRasterOp);
    m_pSGP->setTextColor (color, -1);
    double dValueOffset = (m_dXMaxWin - m_dXMinWin) / 4;
    if (row < 4) {
      double dYPos = m_dYMaxWin - (row * m_dTextHeight);
      double dXPos = m_dXMinWin;
      m_pSGP->moveAbs (dXPos, dYPos);
      m_pSGP->drawText (szLabel);
      m_pSGP->moveAbs (dXPos + dValueOffset, dYPos);
      m_pSGP->drawText (szValue);
    } else {
      row -= 4;
      double dYPos = m_dYMaxWin - (row * m_dTextHeight);
      double dXPos = m_dXMinWin + (m_dXMaxWin - m_dXMinWin) * 0.5;
      m_pSGP->moveAbs (dXPos, dYPos);
      m_pSGP->drawText (szLabel);
      m_pSGP->moveAbs (dXPos + dValueOffset, dYPos);
      m_pSGP->drawText (szValue);
    }
  } else
#endif
  {
    cio_put_str (szLabel);
    cio_put_str (szValue);
    cio_put_str ("\n");
  }
}



/* NAME
*    projectSingleLine                  INTERNAL: Calculates raysum along a line for a Phantom
*
* SYNOPSIS
*    rsum = phm_ray_attenuation (phm, x1, y1, x2, y2)
*    double rsum                Ray sum of Phantom along given line
*    Phantom& phm;              Phantom from which to calculate raysum
*    double *x1, *y1, *x2, y2   Endpoints of ray path (in Phantom coords)
*/

double
Scanner::projectSingleLine (const Phantom& phm, const double x1, const double y1, const double x2, const double y2)
{
  // check ray against each pelem in Phantom
  double rsum = 0.0;
  for (PElemConstIterator i = phm.listPElem().begin(); i != phm.listPElem().end(); i++)
    rsum += projectLineAgainstPElem (**i, x1, y1, x2, y2);

  return (rsum);
}


/* NAME
*   pelem_ray_attenuation               Calculate raysum of an pelem along one line
*
* SYNOPSIS
*   rsum = pelem_ray_attenuation (pelem, x1, y1, x2, y2)
*   double rsum         Computed raysum
*   PhantomElement& pelem               Pelem to scan
*   double x1, y1, x2, y2       Endpoints of raysum line
*/

double
Scanner::projectLineAgainstPElem (const PhantomElement& pelem, double x1, double y1, double x2, double y2)
{
  if (! pelem.clipLineWorldCoords (x1, y1, x2, y2)) {
    if (m_trace == Trace::TRACE_CLIPPING)
      cio_tone (1000., 0.05);
    return (0.0);
  }

#ifdef HAVE_SGP
  if (m_pSGP && m_trace == Trace::TRACE_CLIPPING) {
    m_pSGP->setRasterOp (RO_XOR);
    m_pSGP->moveAbs (x1, y1);
    m_pSGP->lineAbs (x2, y2);
    cio_tone (8000., 0.05);
    m_pSGP->moveAbs (x1, y1);
    m_pSGP->lineAbs (x2, y2);
    m_pSGP->setRasterOp (RO_SET);
  }
#endif

  double len = lineLength (x1, y1, x2, y2);
  return (len * pelem.atten());
}

