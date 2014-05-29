/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:         reconstruct.cpp         Reconstruction class
**   Programmer:   Kevin Rosenberg
**   Date Started: Aug 84
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


/* NAME
 *   Reconstructor::Reconstructor      Reconstruct Image from Projections
 *
 * SYNOPSIS
 *   im = proj.reconstruct (im, filt_type, filt_param, interp_type)
 *   IMAGE *im                  Output image
 *   int filt_type              Type of convolution filter to use
 *   double filt_param          Filter specific parameter
 *                              Currently, used only with Hamming filters
 *   int interp_type            Type of interpolation method to use
 *
 * ALGORITHM
 *
 *      Calculate one-dimensional filter in spatial domain
 *      Allocate & clear (zero) the 2d output image array
 *      For each projection view
 *          Convolve raysum array with filter
 *          Backproject raysums and add (summate) to image array
 *      end
 */


Reconstructor::Reconstructor (const Projections& rProj, ImageFile& rIF, const char* const filterName,
                              double filt_param, const char* const filterMethodName, const int zeropad,
                              const char* filterGenerationName, const char* const interpName,
                              int interpFactor, const char* const backprojectName, const int iTrace,
                              ReconstructionROI* pROI, bool bRebinToParallel, SGP* pSGP)
  : m_rOriginalProj(rProj),
    m_pProj(bRebinToParallel ? m_rOriginalProj.interpolateToParallel() : &m_rOriginalProj),
    m_rImagefile(rIF), m_pProcessSignal(0), m_pBackprojector(0),
    m_iTrace(iTrace), m_bRebinToParallel(bRebinToParallel), m_bFail(false), m_adPlotXAxis(0)
{
  m_nFilteredProjections = m_pProj->nDet() * interpFactor;

#ifdef HAVE_BSPLINE_INTERP
  int spline_order = 0, zoom_factor = 0;
  if (interp_type == I_BSPLINE) {
    zoom_factor = interpFactor;
    spline_order = 3;
    zoom_factor = 3;
    m_nFilteredProjections = (m_nDet - 1) * (zoom_factor + 1) + 1;
  }
#endif

  double filterBW = 1. / m_pProj->detInc();
  m_pProcessSignal = new ProcessSignal (filterName, filterMethodName, filterBW, m_pProj->detInc(),
    m_pProj->nDet(), filt_param, "spatial", filterGenerationName, zeropad, interpFactor, iTrace,
    m_pProj->geometry(), m_pProj->focalLength(), m_pProj->sourceDetectorLength(), pSGP);

  if (m_pProcessSignal->fail()) {
    m_bFail = true;
    m_strFailMessage = "Error creating ProcessSignal: ";
    m_strFailMessage += m_pProcessSignal->failMessage();
    delete m_pProcessSignal; m_pProcessSignal = NULL;
    return;
  }

  m_pBackprojector = new Backprojector (*m_pProj, m_rImagefile, backprojectName, interpName, interpFactor, pROI);
  if (m_pBackprojector->fail()) {
    m_bFail = true;
    m_strFailMessage = "Error creating backprojector: ";
    m_strFailMessage += m_pBackprojector->failMessage();
    delete m_pBackprojector; m_pBackprojector = NULL;
    delete m_pProcessSignal; m_pProcessSignal = NULL;
    return;
  }

#ifdef HAVE_SGP
  m_adPlotXAxis = new double [m_pProj->nDet()];
  double x = - ((m_pProj->nDet() - 1) / 2) * m_pProj->detInc();
  double xInc = m_pProj->detInc();

  for (int i = 0; i < m_pProj->nDet(); i++, x += xInc)
    m_adPlotXAxis[i] = x;
#endif
}

Reconstructor::~Reconstructor ()
{
  if (m_bRebinToParallel)
    delete m_pProj;

  delete m_pBackprojector;
  delete m_pProcessSignal;
  delete m_adPlotXAxis;
}


void
Reconstructor::plotFilter (SGP* pSGP)
{
#ifdef HAVE_SGP
  int nVecFilter = m_pProcessSignal->getNFilterPoints();
  double* adPlotXAxis = new double [nVecFilter];

  if (nVecFilter > 0 && pSGP)  {
    double f = m_pProcessSignal->getFilterMin();
    double filterInc = m_pProcessSignal->getFilterIncrement();
    for (int i = 0; i < nVecFilter; i++, f += filterInc)
      adPlotXAxis[i] = f;

    if (m_pProcessSignal->getFilter()) {
      EZPlot ezplot;

      ezplot.ezset ("title Filter Response");
      ezplot.addCurve (adPlotXAxis, m_pProcessSignal->getFilter(), nVecFilter);
      ezplot.plot (pSGP);
    }
  }
  delete adPlotXAxis;
#endif
}


void
Reconstructor::reconstructAllViews ()
{
  reconstructView (0, m_pProj->nView());
  postProcessing();
}

void
Reconstructor::postProcessing()
{
  m_pBackprojector->PostProcessing();
}


void
Reconstructor::reconstructView (int iStartView, int iViewCount, SGP* pSGP, bool bBackprojectView, double dGraphWidth)
{
  double* adFilteredProj = new double [m_nFilteredProjections];   // filtered projections

  if (iViewCount <= 0)
    iViewCount = m_pProj->nView() - iStartView;

  for (int iView = iStartView; iView < (iStartView + iViewCount); iView++)  {
    if (m_iTrace == Trace::TRACE_CONSOLE)
                std::cout <<"Reconstructing view " << iView << " (last = " << m_pProj->nView() - 1 << ")\n";

    const DetectorArray& rDetArray = m_pProj->getDetectorArray (iView);
    const DetectorValue* detval = rDetArray.detValues();

    m_pProcessSignal->filterSignal (detval, adFilteredProj);

#ifdef HAVE_BSPLINE_INTERP
    if (interp_type == I_BSPLINE)
        bspline (m_pProj->nDet(), zoom_factor, spline_order, adFilteredProj, adFilteredProj);

#ifdef HAVE_SGP
    if (trace >= Trace::TRACE_PLOT && interp_type == I_BSPLINE && pSGP) {
        bspline (m_pProj->nDet(), zoom_factor, spline_order, adFilteredProj, adFilteredProj);
      ezplot_1d (adFilteredProj, m_nFilteredProjections);
    }
#endif
#endif

        if (bBackprojectView)
      m_pBackprojector->BackprojectView (adFilteredProj, rDetArray.viewAngle());

#ifdef HAVE_SGP
    if (m_iTrace >= Trace::TRACE_PLOT && pSGP) {
      EZPlot ezplotProj;

      std::ostringstream osXLength;
      osXLength << "xlength " << dGraphWidth;

      ezplotProj.ezset ("clear");
      ezplotProj.ezset ("title Raw Projection");
      ezplotProj.ezset ("xticks major 5");
      ezplotProj.ezset ("yticks major 5");
      ezplotProj.ezset ("xlabel ");
      ezplotProj.ezset ("ylabel ");
      ezplotProj.ezset ("yporigin 0.55");
      ezplotProj.ezset ("ylength 0.45");
      ezplotProj.ezset (osXLength.str().c_str());
      ezplotProj.ezset ("box.");
      ezplotProj.ezset ("grid.");
#if 0  // workaround c++ optimizer bug, now disabled by using /O1 in code
      double* pdDetval = new double [m_pProj->nDet()];
      for (unsigned int id = 0; id < m_pProj->nDet(); id++) {
        pdDetval[id] = detval[id];
      }
      ezplotProj.addCurve (m_adPlotXAxis, pdDetval, m_pProj->nDet());
      delete pdDetval;
#else
      ezplotProj.addCurve (m_adPlotXAxis, detval, m_pProj->nDet());
#endif
      pSGP->setTextPointSize (9);
      ezplotProj.plot (pSGP);

      ezplotProj.ezset ("clear");
      ezplotProj.ezset ("title Filtered Projection");
      ezplotProj.ezset ("xticks major 5");
      ezplotProj.ezset ("xlabel ");
      ezplotProj.ezset ("ylabel ");
      ezplotProj.ezset ("yticks major 5");
      ezplotProj.ezset ("yporigin 0.10");
      ezplotProj.ezset ("ylength 0.45");
      ezplotProj.ezset (osXLength.str().c_str());
      ezplotProj.ezset ("box");
      ezplotProj.ezset ("grid");
      ezplotProj.addCurve (m_adPlotXAxis, adFilteredProj,  m_nFilteredProjections);
      pSGP->setTextPointSize (9);
      ezplotProj.plot (pSGP);

}
#endif  //HAVE_SGP
  }

  delete adFilteredProj;
}

