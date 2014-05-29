/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dlgreconstruct.h
**   Purpose:       Headers for Reconstruction Animation Dialog
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

#ifndef __DLGRECONSTRUCT_H_
#define __DLGRECONSTRUCT_H_

#include "wx/setup.h"
#include "wx/dialog.h"
#include "wx/dcmemory.h"

class wxButton;
class wxStaticText;
class Projections;
class ImageFile;
class SGP;
class SGPDriver;
class Reconstructor;

class ReconstructDialog : public wxDialog
{
  DECLARE_DYNAMIC_CLASS(ReconstructDialog)

public:
  ReconstructDialog (Reconstructor& rReconstruct, const Projections& rProj, ImageFile& rIF, const int iTrace, wxWindow *parent);

   ~ReconstructDialog();

   /* Perform projection on view number
       return true if ABORT button has not been pressed
   */
   bool reconstructView (int iViewNumber, bool bBackproject = true);

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

   Reconstructor& m_rReconstructor;
   const Projections& m_rProjections;
   ImageFile& m_rImageFile;
   SGPDriver* m_pSGPDriver;
   SGP* m_pSGP;
   const int m_iTrace;
   wxDC* m_pDC;

   wxButton *m_btnAbort;    // the abort button (or NULL if none)
   wxButton *m_btnPause;
   wxButton *m_btnStep;

   wxMemoryDC m_memoryDC;  // for restoring image on OnPaint
   wxBitmap m_bitmap;

   int m_nxImage;
   int m_nyImage;
   int m_nxGraph;
   int m_nyGraph;

   // continue processing or not (return value for Update())
   enum
   {
      Uncancellable = -1,   // dialog can't be canceled
      Paused,
      Cancelled,            // can be cancelled and, in fact, was
      Continue,            // can be cancelled but wasn't
      Finished             // finished, waiting to be removed from screen
   } m_state;

   const static int ID_BTN_PAUSE;
   const static int ID_BTN_STEP;
   const static int MAX_IMAGE_X;
   const static int MAX_IMAGE_Y;

   void showView (int iViewNumber, bool bBackprojectView = true);

   DECLARE_EVENT_TABLE()
};
#endif

