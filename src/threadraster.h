/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          threadraster.h
**   Purpose:       Header file for threaded rasterizations
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

#ifndef _THREADRASTER_H
#define _THREADRASTER_H

#ifdef HAVE_WXTHREADS

#include <vector>
#include <wx/thread.h>
#include <wx/progdlg.h>
#include "timer.h"
#include "backgroundsupr.h"


class Reconstructor;
class ImageFile;
class PhantomFileDocument;
class RasterizerWorker;
class ImageFileView;

class RasterizerSupervisorThread : public SupervisorThread {
private:
  PhantomFileView* m_pPhantomView;
  const int m_iNX;
  const int m_iNY;
  const int m_iNSample;
  const double m_dViewRatio;
  const wxString m_strLabel;

public:
  RasterizerSupervisorThread(PhantomFileView* pProjView, int iNX, int iNY, int iNSample, double dViewRatio, wxChar const* strLabel);

  virtual wxThread::ExitCode Entry();
  virtual void OnExit();
};



class RasterizerSupervisor : public BackgroundSupervisor {
private:

  PhantomFileView* m_pPhantomView;
  std::vector<ImageFile*> m_vecpChildImageFiles;
  PhantomFileDocument* m_pPhantomDoc;

  const int m_iNX;
  const int m_iNY;
  const int m_iNSample;
  const double m_dViewRatio;
  const wxString m_strLabel;


public:
   RasterizerSupervisor (SupervisorThread* pThread, PhantomFileView* pProjView, int iNX, int iNY,
   int iNSample, double dViewRatio, wxChar const* pszLabel);

   virtual BackgroundWorkerThread* createWorker (int iThread, int iStartUnit, int iNumUnits);

   virtual ~RasterizerSupervisor ();

  void onDone();

  ImageFile* getImageFile();

};




class RasterizerWorker : public BackgroundWorkerThread {
private:
  PhantomFileView* m_pPhantomView;
  ImageFile* m_pImageFile;
  int m_iNX;
  int m_iNY;
  int m_iNSample;
  double m_dViewRatio;


public:
  RasterizerWorker (RasterizerSupervisor* pSupervisor, int iThread, int iStartView, int iNumViews)
    : BackgroundWorkerThread (pSupervisor, iThread, iStartView, iNumViews)
  {}

  void SetParameters (PhantomFileView* pPhantomFile, ImageFile* pImageFile, int iNX, int iY,
   int iNSample, double dViewRatio);

  virtual wxThread::ExitCode Entry();      // thread execution starts here

  virtual void OnExit();
};

#endif // HAVE_WXTHREADS
#endif // _THREADRASTER_H_

