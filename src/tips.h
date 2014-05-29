/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          tips.h
**   Purpose:       Header file for user tips
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


#include <wx/tipdlg.h>


class CTSimTipProvider : public wxTipProvider
{
private:
  static const char* const s_aszTips[];
  static const size_t s_iNumTips;

  size_t m_iCurrentTip;

public:
  CTSimTipProvider(size_t currentTip);

  virtual wxString GetTip();
  size_t GetCurrentTip();
};
