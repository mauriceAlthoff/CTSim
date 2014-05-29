/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          threadproj.h
**   Purpose:       Header file for thread reconstructions
**   Programmer:    Kevin Rosenberg
**   Date Started:  February 2001
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

#ifndef _THREADPROJ_H
#define _THREADPROJ_H

#ifdef HAVE_WXTHREADS

#include <vector>
#include <wx/thread.h>
#include <wx/progdlg.h>
#include "timer.h"
#include "backgroundsupr.h"


class Reconstructor;
class ImageFile;
class PhantomFileDocument;
class ProjectorWorker;
class ProjectionFileView;

class ProjectorSupervisorThread : public SupervisorThread {
private:
  PhantomFileView* m_pPhantomView;
  const int m_iNDet;
  const int m_iNView;
  const int m_iOffsetView;
  const std::string m_strGeometry;
  const int m_iNSample;
  const double m_dRotation;
  const double m_dFocalLength;
  const double m_dCenterDetectorLength;
  const double m_dViewRatio;
  const double m_dScanRatio;
  const wxString m_strLabel;

public:
  ProjectorSupervisorThread(PhantomFileView* pProjView, int iNDet, int iNView, int iOffsetView,
   const char* pszGeometry, int iNSample, double dRotation, double dFocalLength, double dCenterDetectorLength,
   double dViewRatio, double dScanRatio, wxChar const* strLabel);

  virtual wxThread::ExitCode Entry();

  virtual void OnExit();
};



class ProjectorSupervisor : public BackgroundSupervisor {
private:

  std::vector<Projections*> m_vecpChildProjections;
  PhantomFileView* m_pPhantomView;
  PhantomFileDocument* m_pPhantomDoc;
  Scanner* m_pScanner;

  const int m_iNDet;
  const int m_iNView;
  const int m_iOffsetView;
  const char* const m_pszGeometry;
  const int m_iNSample;
  const double m_dRotation;
  const double m_dFocalLength;
  const double m_dCenterDetectorLength;
  const double m_dViewRatio;
  const double m_dScanRatio;
  const wxString m_strLabel;


public:
   ProjectorSupervisor (SupervisorThread* pThread, PhantomFileView* pProjView, int iNDet, int iNView,  int iOffsetView,
   const char* pszGeometry, int iNSample, double dRotation, double dFocalLength, double dCenterDetectorLength,
   double dViewRatio, double dScanRatio, wxChar const* pszLabel);

   virtual BackgroundWorkerThread* createWorker (int iThread, int iStartUnit, int iNumUnits);

   virtual ~ProjectorSupervisor ();

  void onDone();

  Projections* getProjections();

};




class ProjectorWorker : public BackgroundWorkerThread {
private:
  PhantomFileView* m_pPhantomView;
  Projections* m_pProjections;
  Scanner* m_pScanner;
  int m_iNDet;
  int m_iNView;
  int m_iOffsetView;
  const char* m_pszGeometry;
  int m_iNSample;
  double m_dRotation;
  double m_dFocalLength;
  double m_dCenterDetectorLength;
  double m_dViewRatio;
  double m_dScanRatio;


public:
  ProjectorWorker (ProjectorSupervisor* pSupervisor, int iThread, int iStartView, int iNumViews)
    : BackgroundWorkerThread (pSupervisor, iThread, iStartView, iNumViews)
  {}

  void SetParameters (PhantomFileView* pPhantomFile, Projections* pProjections, Scanner* pScanner,
   int iNDet, int iView, int iOffsetView,
   const char* const pszGeometry, int iNSample, double dRotation, double dFocalLength, double dCenterDetectorLength,
   double dViewRatio, double dScanRatio);

  virtual wxThread::ExitCode Entry();      // thread execution starts here

  virtual void OnExit();
};


#endif // HAVE_WXTHREADS
#endif // _THREADPROJ_H_

