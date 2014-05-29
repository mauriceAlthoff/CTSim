/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          tips.cpp
**   Purpose:       User tips for CTSim
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "tips.h"

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif


const char* const CTSimTipProvider::s_aszTips[] = {
  "You can create a geometric phantom using the \"File - Create Phantom\" menu command.",
  "You can create a gray-scale, rasterized image of a phantom by using the \"Process - Rasterize\" menu command on a geometric phantom.",
  "You can simulate the x-ray process by using the \"Process - Projections\" menu command on a geometric phantom.",
  "You can simulate first, second, and third-fourth-fifth generation CT scanners by using different scanner geometries.",
  "You can reconstruct an image from the x-ray data by using the \"Reconstruction\" menu on a projection file.",
  "You can specify different levels of smoothing by using different filters in the \"Reconstruction\" dialog.",
  "You can select a row and column of an image by left-mouse button clicking on an image.",
  "You can plot a column or row of an image by using the \"Analyze - Plot\" command.",
  "You can save your plots to a disk file by using the \"File - Save\" command.",
  "You can create your own phantoms using a text editor. Please see the manual for the simple file format.",
  "You can perform 2-dimension Fourier transform of images using the \"Filter\" menu commands.",
  "You can create an image of a filter by using the \"File - Create Filter\" menu command.",
  "You can add two images by using the \"Image - Add\" menu command.",
  "You can display the value of a pixel in an image by right-mouse button clicking on an image.",
  "You can view a 3-dimensional view of an image using the \"Image - 3D\" menu command.",
  "You can scale an image to any size using the \"Image - Scale Size\" menu command.",
  "You can display context-sensitive help by using the \"Help\" button on dialog boxes.",
  "You can compare two images by using the \"Analyze - Compare Images\" menu command.",
  "You can display these tips at any time by using the \"Help - Tips\" menu command.",
  "You can control CTSim's operation using the \"File - Preferences\" menu command.",
};

const size_t CTSimTipProvider::s_iNumTips = sizeof(s_aszTips) / sizeof(const char *);


CTSimTipProvider::CTSimTipProvider (size_t iCurrentTip)
: wxTipProvider (iCurrentTip)
{
  if (iCurrentTip >= s_iNumTips)
    iCurrentTip = 0;

  m_iCurrentTip = iCurrentTip;
}

wxString
CTSimTipProvider::GetTip()
{
  if (m_iCurrentTip >= s_iNumTips)
    m_iCurrentTip = 0;

  size_t iThisTip = m_iCurrentTip;
  ++m_iCurrentTip;

  return wxString (wxConvUTF8.cMB2WX(s_aszTips[iThisTip]));
}

size_t
CTSimTipProvider::GetCurrentTip()
{
  if (m_iCurrentTip >= s_iNumTips)
    m_iCurrentTip = 0;

  return m_iCurrentTip;
}

