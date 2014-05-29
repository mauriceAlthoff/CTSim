/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dialogs.cpp
**   Purpose:       Dialog routines for CTSim program
**   Programmer:    Kevin Rosenberg
**   Date Started:  July 2000
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "wx/statline.h"
#include "wx/sizer.h"
#include "dialogs.h"
#include "ctsim.h"
#include "ct.h"
#include "docs.h"
#include "views.h"
#include "imagefile.h"
#include "projections.h"

#if defined(MSVC) || HAVE_SSTREAM
#include <sstream>
#else
#include <sstream_subst>
#endif


///////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//    StringValueAndTitleListBox
///////////////////////////////////////////////////////////////////////

#if 0
StringValueAndTitleListBox::StringValueAndTitleListBox (wxDialog* pParent, int nChoices, 
                                                        wxChar const* const* asTitle, 
                                                        char const* const* aszValue)
: wxListBox ()
{
  wxString* psTitle = new wxString [nChoices];
  for (int i = 0; i < nChoices; i++)
    psTitle[i] = asTitle[i];

  Create (pParent, -1, wxDefaultPosition, wxSize(-1,-1), nChoices, psTitle, wxLB_SINGLE | wxLB_NEEDED_SB);

  m_ppszValues = aszValue;
  delete [] psTitle;
};
#endif

const char*
StringValueAndTitleListBox::getSelectionStringValue () const
{
  return m_ppszValues[GetSelection()];
}

StringValueAndTitleRadioBox::StringValueAndTitleRadioBox (wxDialog* pParent, 
                                                          wxChar const* strTitle, 
                                                          int nChoices, 
                                                          char const* const* aszTitle, 
                                                          char const* const* aszValue)
: wxRadioBox ()
{
  wxString* psTitle = new wxString [nChoices];
  for (int i = 0; i < nChoices; i++)
    psTitle[i] = wxConvUTF8.cMB2WX(aszTitle[i]);

  Create (pParent, -1, strTitle, wxDefaultPosition, wxDefaultSize, nChoices, psTitle, 1, wxRA_SPECIFY_COLS);

  m_ppszValues = aszValue;
  delete [] psTitle;
};



const char*
StringValueAndTitleRadioBox::getSelectionStringValue () const
{
  return m_ppszValues[GetSelection()];
}

///////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//    DialogGetPhantom
///////////////////////////////////////////////////////////////////////

DialogGetPhantom::DialogGetPhantom (wxWindow* pParent, int iDefaultPhantom)
: wxDialog (pParent, -1, _T("Select Phantom"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Select Phantom")), 0, wxCENTER | wxALL, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  m_pRadioBoxPhantom = new StringValueAndTitleRadioBox (this, _T("Phantom"), Phantom::getPhantomCount(), Phantom::getPhantomTitleArray(), Phantom::getPhantomNameArray());
  m_pRadioBoxPhantom->SetSelection (iDefaultPhantom);
  pTopSizer->Add (m_pRadioBoxPhantom, 0, wxALL | wxALIGN_CENTER);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_PHANTOM);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

const char*
DialogGetPhantom::getPhantom()
{
  return m_pRadioBoxPhantom->getSelectionStringValue();
}


///////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//    DialogGetThetaRange
///////////////////////////////////////////////////////////////////////

DialogGetThetaRange::DialogGetThetaRange (wxWindow* pParent, int iDefaultThetaRange)
: wxDialog (pParent, -1, _T("Select Phantom"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Select Theta Range")), 0, wxCENTER | wxALL, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxString asTitle[] = {_T("Unconstrained"), _T("Normalized to 2pi"), _T("Fold to pi")};

  m_pRadioBoxThetaRange = new wxRadioBox (this, -1, _T("Theta Range"), wxDefaultPosition, wxDefaultSize, 3, asTitle, 1, wxRA_SPECIFY_COLS);
  if (iDefaultThetaRange == ParallelRaysums::THETA_RANGE_UNCONSTRAINED)
    m_pRadioBoxThetaRange->SetSelection (0);
  else if (iDefaultThetaRange == ParallelRaysums::THETA_RANGE_NORMALIZE_TO_TWOPI)
    m_pRadioBoxThetaRange->SetSelection (1);
  else if (iDefaultThetaRange == ParallelRaysums::THETA_RANGE_FOLD_TO_PI)
    m_pRadioBoxThetaRange->SetSelection (2);

  pTopSizer->Add (m_pRadioBoxThetaRange, 0, wxALL | wxALIGN_CENTER);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_THETA_RANGE);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

int
DialogGetThetaRange::getThetaRange()
{
  int iSelection = m_pRadioBoxThetaRange->GetSelection();
  if (iSelection == 0)
    return ParallelRaysums::THETA_RANGE_UNCONSTRAINED;
  else if (iSelection == 1)
    return ParallelRaysums::THETA_RANGE_NORMALIZE_TO_TWOPI;
  else
    return ParallelRaysums::THETA_RANGE_FOLD_TO_PI;
}


///////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//    DialogGetComparisonImage
///////////////////////////////////////////////////////////////////////

DialogGetComparisonImage::DialogGetComparisonImage (wxWindow* pParent, wxChar const* pwszTitle, 
                                                    const std::vector<ImageFileDocument*>& rVecIF, bool bShowMakeDifference)
: wxDialog (pParent, -1, pwszTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION), m_rVecIF(rVecIF)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, pwszTitle), 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxALL, 5);

  int iNImages = m_rVecIF.size();
  wxString* pstrImages = new wxString [iNImages];
  for (int i = 0; i < iNImages; i++) {
    ImageFileView* pView = dynamic_cast<ImageFileView*>(m_rVecIF[i]->GetFirstView());
    if (pView)
      pstrImages[i] = pView->getFrame()->GetTitle();
  }

  m_pListBoxImageChoices = new wxListBox (this, -1, wxDefaultPosition, wxDefaultSize, iNImages, pstrImages, wxLB_SINGLE);
  delete [] pstrImages;

  m_pListBoxImageChoices->SetSelection (0);
  pTopSizer->Add (m_pListBoxImageChoices, 0, wxALL | wxALIGN_CENTER | wxEXPAND);

  if (bShowMakeDifference) {
    m_pMakeDifferenceImage = new wxCheckBox (this, -1, _T("Make Difference Image"));
    m_pMakeDifferenceImage->SetValue (FALSE);
    pTopSizer->Add (m_pMakeDifferenceImage, 0, wxALL | wxALIGN_CENTER | wxEXPAND);
  } else
    m_pMakeDifferenceImage = NULL;

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_COMPARISON);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

ImageFileDocument*
DialogGetComparisonImage::getImageFileDocument()
{
  return m_rVecIF[ m_pListBoxImageChoices->GetSelection() ];
}

bool
DialogGetComparisonImage::getMakeDifferenceImage()
{
  if (m_pMakeDifferenceImage)
    return m_pMakeDifferenceImage->GetValue();
  else
    return false;
}


/////////////////////////////////////////////////////////////////////
// CLASS DiaglogPreferences Implementation
/////////////////////////////////////////////////////////////////////

DialogPreferences::DialogPreferences (wxWindow* pParent, wxChar const* pwszTitle,
                                      bool bAdvancedOptions, bool bAskDeleteNewDocs, bool bVerboseLogging, bool bStartupTips, 
                                      bool bUseBackgroundTasks)
: wxDialog (pParent, -1, pwszTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, pwszTitle), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  m_pCBAdvancedOptions = new wxCheckBox (this, -1, _T("Advanced Options"), wxDefaultPosition, wxSize(250, 25), 0);
  m_pCBAdvancedOptions->SetValue (bAdvancedOptions);
  pTopSizer->Add (m_pCBAdvancedOptions, 0, wxALIGN_CENTER_VERTICAL);

  m_pCBAskDeleteNewDocs = new wxCheckBox (this, -1, _T("Ask \"Save New Documents\" Before Closing"), wxDefaultPosition, wxSize(250, 25), 0);
  m_pCBAskDeleteNewDocs->SetValue (bAskDeleteNewDocs);
  pTopSizer->Add (m_pCBAskDeleteNewDocs, 0, wxALIGN_CENTER_VERTICAL);

  m_pCBVerboseLogging = new wxCheckBox (this, -1, _T("Verbose Logging"), wxDefaultPosition, wxSize(250, 25), 0);
  m_pCBVerboseLogging->SetValue (bVerboseLogging);
  pTopSizer->Add (m_pCBVerboseLogging, 0, wxALIGN_CENTER_VERTICAL);

  m_pCBStartupTips = new wxCheckBox (this, -1, _T("Show Tips at Start"), wxDefaultPosition, wxSize(250, 25), 0);
  m_pCBStartupTips->SetValue (bStartupTips);
  pTopSizer->Add (m_pCBStartupTips, 0, wxALIGN_CENTER_VERTICAL);

#if HAVE_WXTHREADS && MSVC
  m_pCBUseBackgroundTasks = new wxCheckBox (this, -1, _T("Put Tasks in Background"), wxDefaultPosition, wxSize(250, 25), 0);
  m_pCBUseBackgroundTasks->SetValue (bUseBackgroundTasks);
  pTopSizer->Add (m_pCBUseBackgroundTasks, 0, wxALIGN_CENTER_VERTICAL);
#endif

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_PREFERENCES);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

DialogPreferences::~DialogPreferences ()
{
}

bool
DialogPreferences::getAdvancedOptions ()
{
  return static_cast<bool>(m_pCBAdvancedOptions->GetValue());
}

bool
DialogPreferences::getAskDeleteNewDocs ()
{
  return static_cast<bool>(m_pCBAskDeleteNewDocs->GetValue());
}

bool
DialogPreferences::getVerboseLogging ()
{
  return static_cast<bool>(m_pCBVerboseLogging->GetValue());
}

bool
DialogPreferences::getStartupTips ()
{
  return static_cast<bool>(m_pCBStartupTips->GetValue());
}

bool
DialogPreferences::getUseBackgroundTasks ()
{
#if HAVE_WXTHREADS && MSVC
  return static_cast<bool>(m_pCBUseBackgroundTasks->GetValue());
#else
  return false;
#endif
}


/////////////////////////////////////////////////////////////////////
// CLASS DiaglogGetMinMax Implementation
/////////////////////////////////////////////////////////////////////

DialogGetMinMax::DialogGetMinMax (wxWindow* pParent, wxChar const* pwszTitle, double dDefaultMin, double dDefaultMax)
: wxDialog (pParent, -1, pwszTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, pwszTitle), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxString sMin;
  sMin << dDefaultMin;
  m_pTextCtrlMin = new wxTextCtrl (this, -1, sMin, wxDefaultPosition, wxSize(100, 25), 0);
  wxString sMax;
  sMax << dDefaultMax;
  m_pTextCtrlMax = new wxTextCtrl (this, -1, sMax, wxDefaultPosition, wxSize(100, 25), 0);

  wxFlexGridSizer *pGridSizer = new wxFlexGridSizer (2);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Minimum")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlMin, 0, wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Maximum")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlMax, 0, wxALIGN_CENTER_VERTICAL);
  pTopSizer->Add (pGridSizer, 1, wxALL, 10);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_MINMAX);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

DialogGetMinMax::~DialogGetMinMax ()
{
}

double
DialogGetMinMax::getMinimum ()
{
  wxString strCtrl = m_pTextCtrlMin->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return dValue;
  else
    return (m_dDefaultMin);
}

double
DialogGetMinMax::getMaximum ()
{
  wxString strCtrl = m_pTextCtrlMax->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return dValue;
  else
    return (m_dDefaultMax);
}


/////////////////////////////////////////////////////////////////////
// CLASS DialogAutoScaleParameters IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

DialogAutoScaleParameters::DialogAutoScaleParameters (wxWindow *pParent, double mean, double mode, double median, double stddev, double dDefaultScaleFactor)
: wxDialog (pParent, -1, _T("Auto Scale Parameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION), m_dMean(mean), m_dMode(mode), m_dMedian(median), m_dStdDev(stddev)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Auto Scale Parameters")), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxString asTitle[] = {_T("Mode"), _T("Median"), _T("Mean")};

  m_pRadioBoxCenter = new wxRadioBox (this, -1, _T("Center"), wxDefaultPosition, wxDefaultSize, 3, asTitle, 1, wxRA_SPECIFY_COLS);
  m_pRadioBoxCenter->SetSelection (0);
  pTopSizer->Add (m_pRadioBoxCenter, 0, wxALL | wxALIGN_CENTER);

  wxGridSizer *pGridSizer = new wxGridSizer (2);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Standard Deviation Factor")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  wxString sDefaultFactor;
  sDefaultFactor << dDefaultScaleFactor;
  m_pTextCtrlStdDevFactor = new wxTextCtrl (this, -1, sDefaultFactor, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (m_pTextCtrlStdDevFactor, 0, wxALIGN_CENTER_VERTICAL);
  pTopSizer->Add (pGridSizer, 1, wxALL, 10);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_AUTOSCALE);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

bool
DialogAutoScaleParameters::getMinMax (double* pMin, double* pMax)
{
  int iCenter = m_pRadioBoxCenter->GetSelection();
  double dCenter = m_dMode;
  if (iCenter == 1)
    dCenter = m_dMedian;
  else if (iCenter == 2)
    dCenter = m_dMean;

  wxString sStddevFactor = m_pTextCtrlStdDevFactor->GetValue();
  double dValue;
  if (! sStddevFactor.ToDouble (&dValue)) {
    *theApp->getLog() << _T("Error: Non-numeric Standard Deviation Factor of ") << sStddevFactor << _T("\n");
    return false;
  }
  double dHalfWidth = dValue * m_dStdDev / 2;
  *pMin = dCenter - dHalfWidth;
  *pMax = dCenter + dHalfWidth;
  *theApp->getLog() << _T("Setting minimum to ") << *pMin << _T(" and maximum to ") << *pMax << _T("\n");

  return true;
}

double
DialogAutoScaleParameters::getAutoScaleFactor ()
{
  wxString sStddevFactor = m_pTextCtrlStdDevFactor->GetValue();
  double dValue = 1.;
  if (! sStddevFactor.ToDouble (&dValue)) {
    *theApp->getLog() << _T("Error: Non-numeric Standard Deviation Factor of ") << sStddevFactor << _T("\n");
  }

  return dValue;
}



/////////////////////////////////////////////////////////////////////
// CLASS IDENTIFICATION
//
// DialogGetRasterParameters
/////////////////////////////////////////////////////////////////////

DialogGetRasterParameters::DialogGetRasterParameters
   (wxWindow* pParent, int iDefaultXSize, int iDefaultYSize, int iDefaultNSamples, double dDefaultViewRatio)
: wxDialog (pParent, -1, _T("Rasterization Parameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Rasterization Parameters")), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxFlexGridSizer *pGridSizer = new wxFlexGridSizer (2);
  wxString sXSize;
  sXSize << iDefaultXSize;
  m_pTextCtrlXSize = new wxTextCtrl (this, -1, sXSize, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("X Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlXSize, 0, wxALIGN_CENTER_VERTICAL);
  wxString sYSize;
  sYSize << iDefaultYSize;
  m_pTextCtrlYSize = new wxTextCtrl (this, -1, sYSize, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Y Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlYSize, 0, wxALIGN_CENTER_VERTICAL);
  wxString sViewRatio;
  sViewRatio << dDefaultViewRatio;
  m_pTextCtrlViewRatio = new wxTextCtrl (this, -1, sViewRatio, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("View Ratio")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlViewRatio, 0, wxALIGN_CENTER_VERTICAL);
  wxString sNSamples;
  sNSamples << iDefaultNSamples;
  m_pTextCtrlNSamples = new wxTextCtrl (this, -1, sNSamples, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Samples per Pixel")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlNSamples, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add (pGridSizer, 1, wxALL, 10);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_RASTERIZE);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

DialogGetRasterParameters::~DialogGetRasterParameters ()
{
}


unsigned int
DialogGetRasterParameters::getXSize ()
{
  wxString strCtrl = m_pTextCtrlXSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultXSize);
}

unsigned int
DialogGetRasterParameters::getYSize ()
{
  wxString strCtrl = m_pTextCtrlYSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultYSize);
}

unsigned int
DialogGetRasterParameters::getNSamples ()
{
  wxString strCtrl = m_pTextCtrlNSamples->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultNSamples);
}

double
DialogGetRasterParameters::getViewRatio ()
{
  wxString strCtrl = m_pTextCtrlViewRatio->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return dValue;
  else
    return (m_dDefaultViewRatio);
}


/////////////////////////////////////////////////////////////////////
// CLASS IDENTIFICATION
//
// DialogGetProjectionParameters
/////////////////////////////////////////////////////////////////////


DialogGetProjectionParameters::DialogGetProjectionParameters
   (wxWindow* pParent, int iDefaultNDet, int iDefaultNView, int iDefaultOffsetView, int iDefaultNSamples,
    double dDefaultRotAngle, double dDefaultFocalLength, double dDefaultCenterDetectorLength,
    double dDefaultViewRatio, double dDefaultScanRatio, int iDefaultGeometry, int iDefaultTrace)
: wxDialog (pParent, -1, _T("Projection Parameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  m_dDefaultRotAngle = dDefaultRotAngle;
  m_dDefaultFocalLength = dDefaultFocalLength;
  m_dDefaultCenterDetectorLength = dDefaultCenterDetectorLength;
  m_dDefaultViewRatio = dDefaultViewRatio;
  m_dDefaultScanRatio = dDefaultScanRatio;
  m_iDefaultNSamples = iDefaultNSamples;
  m_iDefaultNView = iDefaultNView;
  m_iDefaultNDet = iDefaultNDet;
  m_iDefaultTrace = iDefaultTrace;
  m_iDefaultGeometry = iDefaultGeometry;

  pTopSizer->Add (new wxStaticText (this, -1, _T("Projection Parameters")), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxFlexGridSizer* pGridSizer = new wxFlexGridSizer (2);
  m_pRadioBoxGeometry = new StringValueAndTitleRadioBox (this, _T("Geometry"), Scanner::getGeometryCount(), Scanner::getGeometryTitleArray(), Scanner::getGeometryNameArray());
  m_pRadioBoxGeometry->SetSelection (iDefaultGeometry);

  pGridSizer->Add (m_pRadioBoxGeometry, 0, wxALL | wxALIGN_CENTER | wxEXPAND);

  m_pRadioBoxTrace = new StringValueAndTitleRadioBox (this, _T("Trace Level"), Trace::getTraceCount(), Trace::getTraceTitleArray(), Trace::getTraceNameArray());
  m_pRadioBoxTrace->SetSelection (iDefaultTrace);
  pGridSizer->Add (m_pRadioBoxTrace, 0, wxALL | wxALIGN_CENTER | wxEXPAND);

          wxFlexGridSizer* pText1Sizer = new wxFlexGridSizer(2);
  wxString sNDet;
  sNDet << iDefaultNDet;
  m_pTextCtrlNDet = new wxTextCtrl (this, -1, sNDet, wxDefaultPosition, wxSize(100, 25), 0);
  pText1Sizer->Add (new wxStaticText (this, -1, _T("Detectors")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pText1Sizer->Add (m_pTextCtrlNDet, 0, wxALIGN_CENTER_VERTICAL);
  wxString sNView;
  sNView << iDefaultNView;
  m_pTextCtrlNView = new wxTextCtrl (this, -1, sNView, wxDefaultPosition, wxSize(100, 25), 0);
  pText1Sizer->Add (new wxStaticText (this, -1, _T("Views")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pText1Sizer->Add (m_pTextCtrlNView, 0, wxALIGN_CENTER_VERTICAL);
  wxString sNSamples;
  sNSamples << iDefaultNSamples;
  m_pTextCtrlNSamples = new wxTextCtrl (this, -1, sNSamples, wxDefaultPosition, wxSize(100, 25), 0);
  pText1Sizer->Add (new wxStaticText (this, -1, _T("Samples per Detector")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pText1Sizer->Add (m_pTextCtrlNSamples, 0, wxALIGN_CENTER_VERTICAL);

  pGridSizer->Add (pText1Sizer);

  wxFlexGridSizer* pText2Sizer = new wxFlexGridSizer(2);
  wxString sViewRatio;
  sViewRatio << dDefaultViewRatio;
  m_pTextCtrlViewRatio = new wxTextCtrl (this, -1, sViewRatio, wxDefaultPosition, wxSize(100, 25), 0);
  pText2Sizer->Add (new wxStaticText (this, -1, _T("View Ratio")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pText2Sizer->Add (m_pTextCtrlViewRatio, 0, wxALIGN_CENTER_VERTICAL);
  wxString sScanRatio;
  sScanRatio << dDefaultScanRatio;
  m_pTextCtrlScanRatio = new wxTextCtrl (this, -1, sScanRatio, wxDefaultPosition, wxSize(100, 25), 0);
  pText2Sizer->Add (new wxStaticText (this, -1, _T("Scan Ratio")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pText2Sizer->Add (m_pTextCtrlScanRatio, 0, wxALIGN_CENTER_VERTICAL);
  wxString sFocalLength;
  sFocalLength << dDefaultFocalLength;
  m_pTextCtrlFocalLength = new wxTextCtrl (this, -1, sFocalLength, wxDefaultPosition, wxSize(100, 25), 0);
  pText2Sizer->Add (new wxStaticText (this, -1, _T("Focal Length Ratio")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pText2Sizer->Add (m_pTextCtrlFocalLength, 0, wxALIGN_CENTER_VERTICAL);

  if (theApp->getAdvancedOptions()) {
    wxString sCenterDetectorLength;
    sCenterDetectorLength << dDefaultCenterDetectorLength;
    m_pTextCtrlCenterDetectorLength = new wxTextCtrl (this, -1, sCenterDetectorLength, wxDefaultPosition, wxSize(100, 25), 0);
    pText2Sizer->Add (new wxStaticText (this, -1, _T("Center-Detector Length Ratio")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pText2Sizer->Add (m_pTextCtrlCenterDetectorLength, 0, wxALIGN_CENTER_VERTICAL);

    wxString sRotAngle;
    sRotAngle << dDefaultRotAngle;
    m_pTextCtrlRotAngle = new wxTextCtrl (this, -1, sRotAngle, wxDefaultPosition, wxSize(100, 25), 0);
    pText2Sizer->Add (new wxStaticText (this, -1, _T("Rotation Angle (Fraction of circle)")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pText2Sizer->Add (m_pTextCtrlRotAngle, 0, wxALIGN_CENTER_VERTICAL);

    wxString sOffsetView;
    sOffsetView << iDefaultOffsetView;
    m_pTextCtrlOffsetView = new wxTextCtrl (this, -1, sOffsetView, wxDefaultPosition, wxSize(100, 25), 0);
    pText2Sizer->Add (new wxStaticText (this, -1, _T("Gantry offset in units of 'views' ")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pText2Sizer->Add (m_pTextCtrlOffsetView, 0, wxALIGN_CENTER_VERTICAL);

  }
  pGridSizer->Add (pText2Sizer);

  pTopSizer->Add (pGridSizer, 1, wxALL, 10);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_PROJECTIONS);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);

  m_pRadioBoxGeometry->SetFocus();
}

DialogGetProjectionParameters::~DialogGetProjectionParameters ()
{
}


unsigned int
DialogGetProjectionParameters::getNDet ()
{
  wxString strCtrl = m_pTextCtrlNDet->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultNDet);
}

unsigned int
DialogGetProjectionParameters::getNView ()
{
  wxString strCtrl = m_pTextCtrlNView->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultNView);
}

unsigned int
DialogGetProjectionParameters::getOffsetView ()
{
  if (theApp->getAdvancedOptions()) {
          wxString strCtrl = m_pTextCtrlOffsetView->GetValue();
          unsigned long lValue;
          if (strCtrl.ToULong (&lValue))
            return lValue;
          else
            return (m_iDefaultOffsetView);
  }
  else
    return 0;
}

unsigned int
DialogGetProjectionParameters::getNSamples ()
{
  wxString strCtrl = m_pTextCtrlNSamples->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultNSamples);
}

double
DialogGetProjectionParameters::getRotAngle ()
{
  if (theApp->getAdvancedOptions()) {
    wxString strCtrl = m_pTextCtrlRotAngle->GetValue();
    double dValue;
    if (strCtrl.ToDouble (&dValue))
      return (dValue * TWOPI);
    else
      return (m_dDefaultRotAngle);
  } else {
    if (Scanner::convertGeometryNameToID (m_pRadioBoxGeometry->getSelectionStringValue()) ==
          Scanner::GEOMETRY_PARALLEL)
      return (PI);
    else
      return (TWOPI);
  }
}

double
DialogGetProjectionParameters::getFocalLengthRatio ()
{
  wxString strCtrl = m_pTextCtrlFocalLength->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return (dValue);
  else
    return (m_dDefaultFocalLength);
}

double
DialogGetProjectionParameters::getCenterDetectorLengthRatio ()
{
  if (theApp->getAdvancedOptions()) {
    wxString strCtrl = m_pTextCtrlCenterDetectorLength->GetValue();
    double dValue;
    if (strCtrl.ToDouble (&dValue))
      return (dValue);
    else
      return (m_dDefaultCenterDetectorLength);
  } else
    return getFocalLengthRatio(); // default is to set equal to focal-length
}

double
DialogGetProjectionParameters::getViewRatio ()
{
  wxString strCtrl = m_pTextCtrlViewRatio->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return (dValue);
  else
    return (m_dDefaultViewRatio);
}

double
DialogGetProjectionParameters::getScanRatio ()
{
  wxString strCtrl = m_pTextCtrlScanRatio->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return (dValue);
  else
    return (m_dDefaultScanRatio);
}

const char*
DialogGetProjectionParameters::getGeometry ()
{
  return m_pRadioBoxGeometry->getSelectionStringValue();
}

int
DialogGetProjectionParameters::getTrace ()
{
  return Trace::convertTraceNameToID(m_pRadioBoxTrace->getSelectionStringValue());
}



/////////////////////////////////////////////////////////////////////
// CLASS IDENTIFICATION
//
// DialogGetReconstructionParameters
/////////////////////////////////////////////////////////////////////


DialogGetReconstructionParameters::DialogGetReconstructionParameters (wxWindow* pParent, int iDefaultXSize,
                     int iDefaultYSize, int iDefaultFilterID, double dDefaultHammingParam,
                     int iDefaultFilterMethodID, int iDefaultFilterGenerationID, int iDefaultZeropad,
                     int iDefaultInterpID, int iDefaultInterpParam, int iDefaultBackprojectID, int iTrace,
                     ReconstructionROI* pDefaultROI)
: wxDialog (pParent, -1, _T("Reconstruction Parameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  m_iDefaultXSize = iDefaultXSize;
  m_iDefaultYSize = iDefaultYSize;
  m_dDefaultFilterParam = dDefaultHammingParam;
  m_iDefaultZeropad = iDefaultZeropad;
  m_iDefaultInterpParam = iDefaultInterpParam;
  m_dDefaultRoiXMin = pDefaultROI->m_dXMin;
  m_dDefaultRoiXMax = pDefaultROI->m_dXMax;
  m_dDefaultRoiYMin = pDefaultROI->m_dYMin;
  m_dDefaultRoiYMax = pDefaultROI->m_dYMax;

  pTopSizer->Add (new wxStaticText (this, -1, _T("Filtered Backprojection Parameters")), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);
  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxFlexGridSizer* pGridSizer = NULL;
  if (theApp->getAdvancedOptions())
    pGridSizer = new wxFlexGridSizer (4);
  else
    pGridSizer = new wxFlexGridSizer (3);

  if (theApp->getAdvancedOptions())
    m_pRadioBoxFilter = new StringValueAndTitleRadioBox (this, _T("Filter"), SignalFilter::getFilterCount(), SignalFilter::getFilterTitleArray(), SignalFilter::getFilterNameArray());
  else
    m_pRadioBoxFilter = new StringValueAndTitleRadioBox (this, _T("Filter"), SignalFilter::getReconstructFilterCount(), SignalFilter::getFilterTitleArray(), SignalFilter::getFilterNameArray());
  m_pRadioBoxFilter->SetSelection (iDefaultFilterID);
  pGridSizer->Add (m_pRadioBoxFilter, 0, wxALL | wxALIGN_LEFT | wxEXPAND);

  if (theApp->getAdvancedOptions()) {
    m_pRadioBoxFilterMethod = new StringValueAndTitleRadioBox (this, _T("Filter Method"), ProcessSignal::getFilterMethodCount(), ProcessSignal::getFilterMethodTitleArray(), ProcessSignal::getFilterMethodNameArray());
    m_pRadioBoxFilterMethod->SetSelection (iDefaultFilterMethodID);
    pGridSizer->Add (m_pRadioBoxFilterMethod, 0, wxALL | wxALIGN_LEFT | wxEXPAND);
  } else {
#if HAVE_FFTW
    static const char* aszFilterMethodTitle[] = {"Convolution", "FFT"};
    static const char* aszFilterMethodName[] = {"convolution", "rfftw"};
#else
    static const char* aszFilterMethodTitle[] = {"Convolution", "Fourier"};
    static const char* aszFilterMethodName[] = {"convolution", "fourier-table"};
#endif
      m_pRadioBoxFilterMethod = new StringValueAndTitleRadioBox (this, _T("Filter Method"), 2, aszFilterMethodTitle, aszFilterMethodName);
#if HAVE_FFTW
      m_pRadioBoxFilterMethod->SetSelection (1);
#else
      m_pRadioBoxFilterMethod->SetSelection (0);
#endif
      pGridSizer->Add (m_pRadioBoxFilterMethod, 0, wxALL | wxALIGN_LEFT | wxEXPAND);
  }

  if (theApp->getAdvancedOptions()) {
    m_pRadioBoxFilterGeneration = new StringValueAndTitleRadioBox (this, _T("Filter Generation"), ProcessSignal::getFilterGenerationCount(), ProcessSignal::getFilterGenerationTitleArray(), ProcessSignal::getFilterGenerationNameArray());
    m_pRadioBoxFilterGeneration->SetSelection (iDefaultFilterGenerationID);
    pGridSizer->Add (m_pRadioBoxFilterGeneration, 0, wxALL | wxALIGN_LEFT | wxEXPAND);

    m_pRadioBoxBackproject = new StringValueAndTitleRadioBox (this, _T("Backprojection"), Backprojector::getBackprojectCount(), Backprojector::getBackprojectTitleArray(), Backprojector::getBackprojectNameArray());
    m_pRadioBoxBackproject->SetSelection (iDefaultBackprojectID);
    pGridSizer->Add (m_pRadioBoxBackproject, 0, wxALL | wxALIGN_RIGHT | wxEXPAND);
  }

  m_pRadioBoxInterp = new StringValueAndTitleRadioBox (this, _T("Interpolation"), Backprojector::getInterpCount(), Backprojector::getInterpTitleArray(), Backprojector::getInterpNameArray());
  m_pRadioBoxInterp->SetSelection (iDefaultInterpID);
  pGridSizer->Add (m_pRadioBoxInterp, 0, wxALL | wxALIGN_RIGHT | wxEXPAND);

  static const char* aszTraceTitle[] = {"None", "Full"};
  static const char* aszTraceName[] = {"none", "full"};
  m_pRadioBoxTrace = new StringValueAndTitleRadioBox (this, _T("Trace Level"), 2, aszTraceTitle, aszTraceName);
  iTrace = clamp(iTrace, 0, 1);
  m_pRadioBoxTrace->SetSelection (iTrace);
  pGridSizer->Add (m_pRadioBoxTrace);

  wxFlexGridSizer* pTextGridSizer = new wxFlexGridSizer (2);
  wxString sXSize;
  sXSize << iDefaultXSize;
  m_pTextCtrlXSize = new wxTextCtrl (this, -1, sXSize, wxDefaultPosition, wxSize(100, 25), 0);
  pTextGridSizer->Add (new wxStaticText (this, -1, _T("X Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pTextGridSizer->Add (m_pTextCtrlXSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  wxString sYSize;
  sYSize << iDefaultYSize;
  m_pTextCtrlYSize = new wxTextCtrl (this, -1, sYSize, wxDefaultPosition, wxSize(100, 25), 0);
  pTextGridSizer->Add (new wxStaticText (this, -1, _T("Y Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pTextGridSizer->Add (m_pTextCtrlYSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  wxString sHammingParam;
  sHammingParam << dDefaultHammingParam;
  m_pTextCtrlFilterParam = new wxTextCtrl (this, -1, sHammingParam, wxDefaultPosition, wxSize(100, 25), 0);
  pTextGridSizer->Add (new wxStaticText (this, -1, _T("Hamming Parameter")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pTextGridSizer->Add (m_pTextCtrlFilterParam, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  if (theApp->getAdvancedOptions()) {
    wxString sZeropad;
    sZeropad << iDefaultZeropad;
    m_pTextCtrlZeropad = new wxTextCtrl (this, -1, sZeropad, wxDefaultPosition, wxSize(100, 25), 0);
    pTextGridSizer->Add (new wxStaticText (this, -1, _T("Zeropad")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pTextGridSizer->Add (m_pTextCtrlZeropad, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  }
  pGridSizer->Add (pTextGridSizer);

#if HAVE_FREQ_PREINTERP
  wxString sInterpParam;
  sInterpParam << iDefaultInterpParam;
  m_pTextCtrlInterpParam = new wxTextCtrl (this, -1, sInterpParam, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Interpolation Parameter")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlInterpParam, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
#endif

  if (theApp->getAdvancedOptions()) {
    wxFlexGridSizer* pROIGridSizer = new wxFlexGridSizer (2);
    wxString sRoiXMin;
    sRoiXMin << m_dDefaultRoiXMin;
    m_pTextCtrlRoiXMin = new wxTextCtrl (this, -1, sRoiXMin, wxDefaultPosition, wxSize(100, 25), 0);
    pROIGridSizer->Add (new wxStaticText (this, -1, _T("ROI XMin")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pROIGridSizer->Add (m_pTextCtrlRoiXMin, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    wxString sRoiXMax;
    sRoiXMax << m_dDefaultRoiXMax;
    m_pTextCtrlRoiXMax = new wxTextCtrl (this, -1, sRoiXMax, wxDefaultPosition, wxSize(100, 25), 0);
    pROIGridSizer->Add (new wxStaticText (this, -1, _T("ROI XMax")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pROIGridSizer->Add (m_pTextCtrlRoiXMax, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    wxString sRoiYMin;
    sRoiYMin << m_dDefaultRoiYMin;
    m_pTextCtrlRoiYMin = new wxTextCtrl (this, -1, sRoiYMin, wxDefaultPosition, wxSize(100, 25), 0);
    pROIGridSizer->Add (new wxStaticText (this, -1, _T("ROI YMin")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pROIGridSizer->Add (m_pTextCtrlRoiYMin, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    wxString sRoiYMax;
    sRoiYMax << m_dDefaultRoiYMax;
    m_pTextCtrlRoiYMax = new wxTextCtrl (this, -1, sRoiYMax, wxDefaultPosition, wxSize(100, 25), 0);
    pROIGridSizer->Add (new wxStaticText (this, -1, _T("ROI YMax")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pROIGridSizer->Add (m_pTextCtrlRoiYMax, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    pGridSizer->Add (pROIGridSizer);
  }

  pTopSizer->Add (pGridSizer, 1, wxALL, 3);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_RECONSTRUCTION);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Layout();
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

DialogGetReconstructionParameters::~DialogGetReconstructionParameters ()
{
}


unsigned int
DialogGetReconstructionParameters::getXSize ()
{
  wxString strCtrl = m_pTextCtrlXSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultXSize);
}

unsigned int
DialogGetReconstructionParameters::getYSize ()
{
  wxString strCtrl = m_pTextCtrlYSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultYSize);
}

unsigned int
DialogGetReconstructionParameters::getZeropad ()
{
  if (theApp->getAdvancedOptions()) {
    wxString strCtrl = m_pTextCtrlZeropad->GetValue();
    unsigned long lValue;
    if (strCtrl.ToULong (&lValue))
      return lValue;
    else
      return (m_iDefaultZeropad);
  } else
    return 1;
}


unsigned int
DialogGetReconstructionParameters::getInterpParam ()
{
#if HAVE_FREQ_PREINTERP
  wxString strCtrl = m_pTextCtrlInterpParam->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultInterpParam);
#else
  return 1;
#endif
}

double
DialogGetReconstructionParameters::getFilterParam ()
{
  wxString strCtrl = m_pTextCtrlFilterParam->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return (dValue);
  else
    return (m_dDefaultFilterParam);
}

const char*
DialogGetReconstructionParameters::getFilterName ()
{
  return m_pRadioBoxFilter->getSelectionStringValue();
}

const char*
DialogGetReconstructionParameters::getFilterMethodName ()
{
  return m_pRadioBoxFilterMethod->getSelectionStringValue();
}

const char*
DialogGetReconstructionParameters::getInterpName ()
{
  return m_pRadioBoxInterp->getSelectionStringValue();
}

int
DialogGetReconstructionParameters::getTrace ()
{
  int iTrace = 0;
  if (strcmp("full", m_pRadioBoxTrace->getSelectionStringValue()) == 0)
    iTrace = Trace::TRACE_PLOT;
  return iTrace;
}

const char*
DialogGetReconstructionParameters::getBackprojectName ()
{
  if (theApp->getAdvancedOptions()) {
    return m_pRadioBoxBackproject->getSelectionStringValue();
  } else
    return "idiff";
}

const char*
DialogGetReconstructionParameters::getFilterGenerationName ()
{
  if (theApp->getAdvancedOptions()) {
    return m_pRadioBoxFilterGeneration->getSelectionStringValue();
  } else {
    if (ProcessSignal::convertFilterMethodNameToID(m_pRadioBoxFilterMethod->getSelectionStringValue())
        == ProcessSignal::FILTER_METHOD_CONVOLUTION)
      return "direct";
    else
      return "inverse-fourier";
  }
}

void
DialogGetReconstructionParameters::getROI (ReconstructionROI* pROI)
{
  if (theApp->getAdvancedOptions()) {
    double dValue;
    if (m_pTextCtrlRoiXMin->GetValue().ToDouble (&dValue))
      pROI->m_dXMin = dValue;
    else
      pROI->m_dXMin = m_dDefaultRoiXMin;

    if (m_pTextCtrlRoiXMax->GetValue().ToDouble (&dValue))
      pROI->m_dXMax = dValue;
    else
      pROI->m_dXMax = m_dDefaultRoiXMax;

    if (m_pTextCtrlRoiYMin->GetValue().ToDouble (&dValue))
      pROI->m_dYMin = dValue;
    else
      pROI->m_dYMin = m_dDefaultRoiYMin;

    if (m_pTextCtrlRoiYMax->GetValue().ToDouble (&dValue))
      pROI->m_dYMax = dValue;
    else
      pROI->m_dYMax = m_dDefaultRoiYMax;
  } else {
    pROI->m_dXMin = m_dDefaultRoiXMin;
    pROI->m_dXMax = m_dDefaultRoiXMax;
    pROI->m_dYMin = m_dDefaultRoiYMin;
    pROI->m_dYMax = m_dDefaultRoiYMax;
  }
}

/////////////////////////////////////////////////////////////////////
// CLASS IDENTIFICATION
//
// DialogGetFilterParameters
/////////////////////////////////////////////////////////////////////



DialogGetFilterParameters::DialogGetFilterParameters (wxWindow* pParent, int iDefaultXSize, int iDefaultYSize, int iDefaultFilterID, double dDefaultFilterParam,  double dDefaultBandwidth, int iDefaultDomainID, double dDefaultInputScale, double dDefaultOutputScale)
: wxDialog (pParent, -1, _T("Filter Parameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Filter Parameters")), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);
  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxFlexGridSizer* pGridSizer = new wxFlexGridSizer (2);

  m_pRadioBoxFilter = new StringValueAndTitleRadioBox (this, _T("Filter"), SignalFilter::getFilterCount(), SignalFilter::getFilterTitleArray(), SignalFilter::getFilterNameArray());
  m_pRadioBoxFilter->SetSelection (iDefaultFilterID);
  pGridSizer->Add (m_pRadioBoxFilter, 0, wxALL | wxALIGN_LEFT | wxEXPAND);

  m_pRadioBoxDomain = new StringValueAndTitleRadioBox (this, _T("Domain"), SignalFilter::getDomainCount(), SignalFilter::getDomainTitleArray(), SignalFilter::getDomainNameArray());
  m_pRadioBoxDomain->SetSelection (iDefaultDomainID);
  pGridSizer->Add (m_pRadioBoxDomain, 0, wxALL | wxALIGN_LEFT | wxEXPAND);

  wxString sXSize;
  sXSize << iDefaultXSize;
  m_pTextCtrlXSize = new wxTextCtrl (this, -1, sXSize, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("X Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlXSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  wxString sYSize;
  sYSize << iDefaultYSize;
  m_pTextCtrlYSize = new wxTextCtrl (this, -1, sYSize, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Y Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlYSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  wxString sFilterParam;
  sFilterParam << dDefaultFilterParam;
  m_pTextCtrlFilterParam = new wxTextCtrl (this, -1, sFilterParam, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Filter Parameter")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlFilterParam, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  wxString sBandwidth;
  sBandwidth << dDefaultBandwidth;
  m_pTextCtrlBandwidth = new wxTextCtrl (this, -1, sBandwidth, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Bandwidth")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlBandwidth, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  wxString sInputScale;
  sInputScale << dDefaultInputScale;
  m_pTextCtrlInputScale = new wxTextCtrl (this, -1, sInputScale, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Axis (input) Scale")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlInputScale, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  wxString sOutputScale;
  sOutputScale << dDefaultOutputScale;
  m_pTextCtrlOutputScale = new wxTextCtrl (this, -1, sOutputScale, wxDefaultPosition, wxSize(100, 25), 0);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Filter Output Scale")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlOutputScale, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add (pGridSizer, 1, wxALL, 3);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_FILTER);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Layout();
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

DialogGetFilterParameters::~DialogGetFilterParameters ()
{
}


unsigned int
DialogGetFilterParameters::getXSize ()
{
  wxString strCtrl = m_pTextCtrlXSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultXSize);
}

unsigned int
DialogGetFilterParameters::getYSize ()
{
  wxString strCtrl = m_pTextCtrlYSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultYSize);
}

double
DialogGetFilterParameters::getBandwidth ()
{
  wxString strCtrl = m_pTextCtrlBandwidth->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return dValue;
  else
    return (m_dDefaultBandwidth);
}

double
DialogGetFilterParameters::getFilterParam ()
{
  wxString strCtrl = m_pTextCtrlFilterParam->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return (dValue);
  else
    return (m_dDefaultFilterParam);
}

double
DialogGetFilterParameters::getInputScale ()
{
  wxString strCtrl = m_pTextCtrlInputScale->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return dValue;
  else
    return (m_dDefaultInputScale);
}

double
DialogGetFilterParameters::getOutputScale ()
{
  wxString strCtrl = m_pTextCtrlOutputScale->GetValue();
  double dValue;
  if (strCtrl.ToDouble (&dValue))
    return dValue;
  else
    return (m_dDefaultOutputScale);
}

const char*
DialogGetFilterParameters::getFilterName ()
{
  return m_pRadioBoxFilter->getSelectionStringValue();
}

const char*
DialogGetFilterParameters::getDomainName ()
{
  return m_pRadioBoxDomain->getSelectionStringValue();
}


///////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//    DialogExportParameters
///////////////////////////////////////////////////////////////////////

DialogExportParameters::DialogExportParameters (wxWindow* pParent, int iDefaultFormatID)
: wxDialog (pParent, -1, _T("Select ExportParameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Select Export Format")), 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxALL, 5);

  m_pRadioBoxFormat = new StringValueAndTitleRadioBox (this, _T("File Type"),
    ImageFile::getExportFormatCount(), ImageFile::getExportFormatTitleArray(), ImageFile::getExportFormatNameArray());
  m_pRadioBoxFormat->SetSelection (iDefaultFormatID);
  pTopSizer->Add (m_pRadioBoxFormat, 0, wxALL | wxALIGN_CENTER);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_EXPORT);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

const char*
DialogExportParameters::getFormatName()
{
  return m_pRadioBoxFormat->getSelectionStringValue();
}


///////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION
//    DialogImportParameters
///////////////////////////////////////////////////////////////////////

DialogImportParameters::DialogImportParameters (wxWindow* pParent, int iDefaultFormatID)
: wxDialog (pParent, -1, _T("Select Import Parameters"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, _T("Select Import Format")), 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxALL, 5);

  m_pRadioBoxFormat = new StringValueAndTitleRadioBox (this, _T("File Type"),
    ImageFile::getImportFormatCount(), ImageFile::getImportFormatTitleArray(), ImageFile::getImportFormatNameArray());
  m_pRadioBoxFormat->SetSelection (iDefaultFormatID);
  pTopSizer->Add (m_pRadioBoxFormat, 0, wxALL | wxALIGN_CENTER);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, IDH_DLG_IMPORT);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

const char*
DialogImportParameters::getFormatName()
{
  return m_pRadioBoxFormat->getSelectionStringValue();
}


/////////////////////////////////////////////////////////////////////
// CLASS DiaglogGetXYSize Implementation
/////////////////////////////////////////////////////////////////////

DialogGetXYSize::DialogGetXYSize (wxWindow* pParent, wxChar const * pwszTitle, int iDefaultXSize, int iDefaultYSize)
: wxDialog (pParent, -1, pwszTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  m_iDefaultXSize = iDefaultXSize;
  m_iDefaultYSize = iDefaultYSize;

  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, pwszTitle), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxString sXSize;
  sXSize << iDefaultXSize;
  m_pTextCtrlXSize = new wxTextCtrl (this, -1, sXSize, wxDefaultPosition, wxSize(100, 25), 0);
  wxString sYSize;
  sYSize << iDefaultYSize;
  m_pTextCtrlYSize = new wxTextCtrl (this, -1, sYSize, wxDefaultPosition, wxSize(100, 25), 0);

  wxFlexGridSizer *pGridSizer = new wxFlexGridSizer (2);
  pGridSizer->Add (new wxStaticText (this, -1, _T("X Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlXSize, 0, wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (new wxStaticText (this, -1, _T("Y Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pGridSizer->Add (m_pTextCtrlYSize, 0, wxALIGN_CENTER_VERTICAL);
  pTopSizer->Add (pGridSizer, 1, wxALL, 10);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();

  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}

DialogGetXYSize::~DialogGetXYSize ()
{
}

unsigned int
DialogGetXYSize::getXSize ()
{
  wxString strCtrl = m_pTextCtrlXSize->GetValue();
  long lValue;
  if (strCtrl.ToLong (&lValue))
    return lValue;
  else
    return (m_iDefaultXSize);
}

unsigned int
DialogGetXYSize::getYSize ()
{
  wxString strCtrl = m_pTextCtrlYSize->GetValue();
  long lValue;
  if (strCtrl.ToLong (&lValue))
    return lValue;
  else
    return (m_iDefaultYSize);
}



/////////////////////////////////////////////////////////////////////
// CLASS IDENTIFICATION
//
// DialogGetConvertPolarParameters
/////////////////////////////////////////////////////////////////////

DialogGetConvertPolarParameters::DialogGetConvertPolarParameters (wxWindow* pParent, wxChar const * pwszTitle,
       int iDefaultXSize, int iDefaultYSize, int iDefaultInterpolationID, int iDefaultZeropad, int iHelpID)
: wxDialog (pParent, -1, pwszTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCAPTION)
{
  m_iDefaultXSize = iDefaultXSize;
  m_iDefaultYSize = iDefaultYSize;
  m_iDefaultZeropad = iDefaultZeropad;

  wxBoxSizer* pTopSizer = new wxBoxSizer (wxVERTICAL);

  pTopSizer->Add (new wxStaticText (this, -1, pwszTitle), 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 5);
  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxFlexGridSizer* pGridSizer = new wxFlexGridSizer (1);

  m_pRadioBoxInterpolation = new StringValueAndTitleRadioBox (this, _T("Interpolation"), Projections::getInterpCount(), Projections::getInterpTitleArray(), Projections::getInterpNameArray());
  m_pRadioBoxInterpolation->SetSelection (iDefaultInterpolationID);
  pGridSizer->Add (m_pRadioBoxInterpolation, 0, wxALL | wxALIGN_CENTER);

  wxFlexGridSizer* pTextGridSizer = new wxFlexGridSizer (2);
  wxString sXSize;
  sXSize << iDefaultXSize;
  m_pTextCtrlXSize = new wxTextCtrl (this, -1, sXSize, wxDefaultPosition, wxSize(100, 25), 0);
  pTextGridSizer->Add (new wxStaticText (this, -1, _T("X Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pTextGridSizer->Add (m_pTextCtrlXSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  wxString sYSize;
  sYSize << iDefaultYSize;
  m_pTextCtrlYSize = new wxTextCtrl (this, -1, sYSize, wxDefaultPosition, wxSize(100, 25), 0);
  pTextGridSizer->Add (new wxStaticText (this, -1, _T("Y Size")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pTextGridSizer->Add (m_pTextCtrlYSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  if (m_iDefaultZeropad >= 0) {
    wxString sZeropad;
    sZeropad << iDefaultZeropad;
    m_pTextCtrlZeropad = new wxTextCtrl (this, -1, sZeropad, wxDefaultPosition, wxSize(100, 25), 0);
    pTextGridSizer->Add (new wxStaticText (this, -1, _T("Zeropad")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pTextGridSizer->Add (m_pTextCtrlZeropad, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  }

  pGridSizer->Add (pTextGridSizer, 0, wxALIGN_CENTER | wxALL);

  pTopSizer->Add (pGridSizer, 1, wxALL | wxALIGN_CENTER, 3);

  pTopSizer->Add (new wxStaticLine (this, -1, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL), 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer (wxHORIZONTAL);
  wxButton* pButtonOk = new wxButton (this, wxID_OK, _T("Okay"));
  pButtonSizer->Add (pButtonOk, 0, wxEXPAND | wxALL, 10);
  wxButton* pButtonCancel = new wxButton (this, wxID_CANCEL, _T("Cancel"));
  pButtonSizer->Add (pButtonCancel, 0, wxEXPAND | wxALL, 10);
  CTSimHelpButton* pButtonHelp = new CTSimHelpButton (this, iHelpID);
  pButtonSizer->Add (pButtonHelp, 0, wxEXPAND | wxALL, 10);

  pTopSizer->Add (pButtonSizer, 0, wxALIGN_CENTER);
  pButtonOk->SetDefault();
  SetAutoLayout (true);
  SetSizer (pTopSizer);
  pTopSizer->Layout();
  pTopSizer->Fit (this);
  pTopSizer->SetSizeHints (this);
}


DialogGetConvertPolarParameters::~DialogGetConvertPolarParameters ()
{
}


unsigned int
DialogGetConvertPolarParameters::getXSize ()
{
  wxString strCtrl = m_pTextCtrlXSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultXSize);
}

unsigned int
DialogGetConvertPolarParameters::getYSize ()
{
  wxString strCtrl = m_pTextCtrlYSize->GetValue();
  unsigned long lValue;
  if (strCtrl.ToULong (&lValue))
    return lValue;
  else
    return (m_iDefaultYSize);
}

unsigned int
DialogGetConvertPolarParameters::getZeropad ()
{
  if (m_iDefaultZeropad >= 0) {
    wxString strCtrl = m_pTextCtrlZeropad->GetValue();
    unsigned long lValue;
    if (strCtrl.ToULong (&lValue))
      return lValue;
    else
      return (m_iDefaultZeropad);
  } else
    return 0;
}

const char*
DialogGetConvertPolarParameters::getInterpolationName ()
{
  return m_pRadioBoxInterpolation->getSelectionStringValue();
}

