/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          threadrecon.h
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

#ifndef _THREADRECON_H
#define _THREADRECON_H

#ifdef HAVE_WXTHREADS


#include <vector>
#include <wx/thread.h>
#include <wx/progdlg.h>
#include "timer.h"
#include "backgroundsupr.h"


class Reconstructor;
class ImageFile;
class ProjectionFileDocument;
class ReconstructorWorker;
class ProjectionFileView;
struct ReconstructionROI;

class ReconstructorSupervisorThread : public SupervisorThread {
private:
  ProjectionFileView* m_pProjView;
  ImageFile* m_pImageFile;
  const int m_iNX;
  const int m_iNY;
  const std::string m_strFilterName;
  const double m_dFilterParam;
  const std::string m_strFilterMethod;
  const int m_iZeropad;
  const std::string m_strFilterGenerationName;
  const std::string m_strInterpName;
  const int m_iInterpParam;
  const std::string m_strBackprojectName;
  const wxString m_strLabel;
  ReconstructionROI m_reconROI;
  const bool m_bRebinToParallel;

public:
  ReconstructorSupervisorThread(ProjectionFileView* pProjView, int iNX, int iNY, const char* pszFilterName,
   double dFilterParam, const char* pszFilterMethod, int iZeropad, const char* pszFilterGenerationName,
   const char* pszInterpName, int iInterpParam, const char* pszBackprojectName, wxChar const* pszLabel,
   ReconstructionROI* pROI, bool bRebinToParallel);

  virtual wxThread::ExitCode Entry();

  virtual void OnExit();
};



class ReconstructorSupervisor : public BackgroundSupervisor {
private:

  std::vector<ImageFile*> m_vecpChildImageFile;
  const Projections* m_pProj;
  ProjectionFileView* m_pProjView;
  ProjectionFileDocument* m_pProjDoc;

  const int m_iImageNX;
  const int m_iImageNY;

  const char* const m_pszFilterName;
  const double m_dFilterParam;
  const char* const m_pszFilterMethod;
  const int m_iZeropad;
  const char* const m_pszFilterGenerationName;
  const char* const m_pszInterpName;
  const int m_iInterpParam;
  const char* const m_pszBackprojectName;
  const wxString m_strLabel;
  ReconstructionROI* m_pReconROI;

public:
   ReconstructorSupervisor (SupervisorThread* pMyThread, Projections* pProj, ProjectionFileView* pProjView,
   int iNX, int iNY, const char* pszFilterName, double dFilterParam, const char* pszFilterMethod, int iZeropad,
   const char* pszFilterGenerationName, const char* pszInterpName, int iInterpParam,
   const char* pszBackprojectName, wxChar const* pszLabel, ReconstructionROI* pReconROI);

   virtual BackgroundWorkerThread* createWorker (int iThread, int iStartUnit, int iNumUnits);

   virtual ~ReconstructorSupervisor ();

  void onDone();

  ImageFile* getImageFile();

};


class ReconstructorWorker : public BackgroundWorkerThread {
private:
  const Projections* m_pProj;
  ProjectionFileView* m_pProjView;
  ImageFile* m_pImageFile;
  const char* m_pszFilterName;
  double m_dFilterParam;
  const char* m_pszFilterMethod;
  int m_iZeropad;
  const char* m_pszFilterGenerationName;
  const char* m_pszInterpName;
  int m_iInterpParam;
  const char* m_pszBackprojectName;
  ReconstructionROI* m_pReconROI;

public:
  ReconstructorWorker (ReconstructorSupervisor* pSupervisor, int iThread, int iStartView, int iNumViews)
    : BackgroundWorkerThread (pSupervisor, iThread, iStartView, iNumViews)
  {}

  void SetParameters (const Projections* pProj, ProjectionFileView* pProjFile, ImageFile* pImageFile,
   const char* const pszFilterName, double dFilterParam, const char* const pszFilterMethod,
   int iZeropad, const char* const pszFilterGenerationName, const char* const pszInterpName, int iInterpParam,
   const char* pszBackprojectName, ReconstructionROI* pROI);

  virtual wxThread::ExitCode Entry();      // thread execution starts here

  virtual void OnExit();
};

#endif // HAVE_WXTHREADS
#endif // _THREADRECON_H_

