/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dlgprojections.h
**   Purpose:       Headers for Projection Collection Animation Dialog
**   Programmer:    Kevin Rosenberg
**   Date Started:  August 2000
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

#ifndef __DLGPROJECTIONS_H_
#define __DLGPROJECTIONS_H_

#include "wx/setup.h"
#include "wx/dialog.h"
#include "wx/dcmemory.h"

class wxButton;
class wxStaticText;
class Projections;
class Phantom;
class SGP;
class Scanner;
class SGPDriver;

class ProjectionsDialog : public wxDialog
{
  DECLARE_DYNAMIC_CLASS(ProjectionsDialog)

public:
  ProjectionsDialog (Scanner& rScanner, Projections& rProj, const Phantom& rPhantom, const int iTrace, wxWindow *parent);

   ~ProjectionsDialog();

   /* Perform projection on view number
       return true if ABORT button has not been pressed
   */
   bool projectView(int iViewNumber);

   /* Can be called to continue after the cancel button has been pressed, but
       the program decided to continue the operation (e.g., user didn't
       confirm it)
   */
   void Resume() { m_state = Continue; }

   // implementation from now on
       // callback for optional abort button
   void OnCancel(wxCommandEvent& event);
       // callback to disable "hard" window closing
   void OnClose(wxCloseEvent& event);
   void OnPaint(wxPaintEvent& event);

   void OnPause(wxCommandEvent& event);
   void OnStep(wxCommandEvent& event);

   bool isPaused() const {return m_state == Paused;}

   bool isCancelled() const {return m_state == Cancelled;}

private:
   // parent top level window (may be NULL)
   wxWindow *m_parentTop;
   int m_iLastView;
   int m_iClientX;   // size of client window
   int m_iClientY;

   Scanner& m_rScanner;
   Projections& m_rProjections;
   const Phantom& m_rPhantom;
   SGPDriver* m_pSGPDriver;
   SGP* m_pSGP;
   const int m_iTrace;
   wxDC* m_pDC;

   wxButton *m_btnAbort;    // the abort button (or NULL if none)
   wxButton *m_btnPause;
   wxButton *m_btnStep;

   wxMemoryDC m_memoryDC;  // for restoring image on OnPaint
   wxBitmap m_bitmap;

   // continue processing or not (return value for Update())
   enum
   {
      Uncancellable = -1,   // dialog can't be canceled
      Paused,
      Cancelled,            // can be cancelled and, in fact, was
      Continue,            // can be cancelled but wasn't
      Finished             // finished, waiting to be removed from screen
   } m_state;

   enum { ID_BTN_PAUSE = 19996, ID_BTN_STEP = 19997 };

   void showView (int iViewNumber);

   DECLARE_EVENT_TABLE()
};
#endif

