/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:         reconstruct.h          Header file for Reconstruction class
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

#ifndef __RECONSTRUCT_H
#define __RECONSTRUCT_H


class Projections;
class ImageFile;
class Backprojector;
class ProcessSignal;

#include <string>

struct ReconstructionROI {
  double m_dXMin;
  double m_dYMin;
  double m_dXMax;
  double m_dYMax;
};

class Reconstructor
{
 private:
    const Projections& m_rOriginalProj;
    const Projections* m_pProj;
    ImageFile& m_rImagefile;
    ProcessSignal* m_pProcessSignal;
    Backprojector* m_pBackprojector;
    int m_nFilteredProjections;
    int m_iTrace;
    const bool m_bRebinToParallel;
    bool m_bFail;
    std::string m_strFailMessage;

    double* m_adPlotXAxis;

 public:
    Reconstructor (const Projections& rProj, ImageFile& rIF, const char* const filterName, double filt_param,
      const char* const filterMethodName, const int zeropad, const char* filterGenerationName,
      const char* const interpName, int interpFactor, const char* const backprojectName, const int trace,
      ReconstructionROI* pROI = NULL, bool bRebinToParallel = false, SGP* pSGP = NULL);

    ~Reconstructor ();

    bool fail() const {return m_bFail;}
    const std::string& failMessage() const {return m_strFailMessage;}

    void plotFilter (SGP* pSGP = NULL);

    void reconstructAllViews ();

    void reconstructView (int iStartView = 0, int iViewCount = -1, SGP* pSGP = NULL, bool bBackprojectView = true, double dGraphWidth = 1.);
    void postProcessing ();
};

#endif
