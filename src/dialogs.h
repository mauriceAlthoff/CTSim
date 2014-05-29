/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          dialogs.h
**   Purpose:       Header file for Dialogs of CTSim program
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


#ifndef __DIALOGSH__
#define __DIALOGSH__

#include "wx/wx.h"
#include <string>
#include "ctsupport.h"
#include "scanner.h"
#include "phantom.h"
#include "procsignal.h"
#include "filter.h"
#include "projections.h"
#include "ctsim-map.h"


class CTSimHelpButton : public wxButton
{
public:
  CTSimHelpButton (wxWindow* parent, int id)
    : wxButton (parent, id, _T("Help"))
  {}
};


// CLASS StringValueAndTitleListBox
//
// A superclass of wxListBox that can handle string values and titles
// and by displaying the title in the list box and returning the string value

class StringValueAndTitleListBox : public wxListBox
{
 public:
  StringValueAndTitleListBox (wxDialog* pParent, int nChoices, wxChar const* const* asTitle,
                              char const* const aszValue[]);
#if 0
  StringValueAndTitleListBox (wxDialog* pParent, int nChoices, wxChar const* title,
                              char const* const* aszTitle, char const* const* aszValue);
#endif
  StringValueAndTitleListBox (wxDialog* pParent, int nChoices, wxChar const* title,
                              wxChar const* const* asTitle, char const* const* aszValue);

    const char* getSelectionStringValue () const;

 private:
    const char* const* m_ppszValues;
};


class StringValueAndTitleRadioBox : public wxRadioBox
{
 public:
  StringValueAndTitleRadioBox (wxDialog* pParent, wxChar const* strTitle, int nChoices, const char* const aszTitle[], const char* const aszValue[]);

  const char* getSelectionStringValue () const;

 private:
  const char* const* m_ppszValues;
};


class DialogGetPhantom : public wxDialog
{
 public:
    DialogGetPhantom (wxWindow* pParent, int iDefaultPhantom = Phantom::PHM_HERMAN);
    virtual ~DialogGetPhantom () {}

    const char* getPhantom ();

 private:
    StringValueAndTitleRadioBox* m_pRadioBoxPhantom;
};

class DialogGetThetaRange : public wxDialog
{
 public:
   DialogGetThetaRange (wxWindow* pParent, int iDefaultThetaRange = ParallelRaysums::THETA_RANGE_UNCONSTRAINED);
    virtual ~DialogGetThetaRange () {}

    int getThetaRange ();

 private:
    wxRadioBox* m_pRadioBoxThetaRange;
};


#include <vector>
class ImageFileDocument;
class DialogGetComparisonImage : public wxDialog
{
 public:
   DialogGetComparisonImage (wxWindow* pParent, wxChar const * pwszTitle, const std::vector<ImageFileDocument*>& rVecIF, bool bShowMakeDifference);
    virtual ~DialogGetComparisonImage () {}

    ImageFileDocument* getImageFileDocument ();

    bool getMakeDifferenceImage();

 private:
    wxListBox* m_pListBoxImageChoices;
    wxCheckBox* m_pMakeDifferenceImage;
    const std::vector<ImageFileDocument*>& m_rVecIF;
};


class DialogPreferences : public wxDialog
{
 public:
  DialogPreferences (wxWindow* pParent, wxChar const* pszTitle, bool bAdvanced, bool bAskNewDocs,
      bool bVerboseLogging, bool bStartupTips, bool bUseBackgroundTasks);
    virtual ~DialogPreferences ();

    bool getAdvancedOptions ();
    bool getAskDeleteNewDocs ();
    bool getVerboseLogging ();
    bool getStartupTips ();
    bool getUseBackgroundTasks();

 private:
    wxCheckBox* m_pCBAdvancedOptions;
    wxCheckBox* m_pCBAskDeleteNewDocs;
    wxCheckBox* m_pCBVerboseLogging;
    wxCheckBox* m_pCBStartupTips;
    wxCheckBox* m_pCBUseBackgroundTasks;
};


class ImageFile;
class DialogGetMinMax : public wxDialog
{
 public:
    DialogGetMinMax (wxWindow* pParent, wxChar const* pszTitle, double dDefaultMin = 0., double dDefaultMax = 0.);
    virtual ~DialogGetMinMax ();

    double getMinimum ();
    double getMaximum ();

 private:
    wxTextCtrl* m_pTextCtrlMin;
    wxTextCtrl* m_pTextCtrlMax;

    double m_dDefaultMin;
    double m_dDefaultMax;
};


class DialogGetRasterParameters : public wxDialog
{
 public:
    DialogGetRasterParameters (wxWindow* pParent, int iDefaultXSize = 0, int iDefaultYSize = 0,
      int iDefaultNSamples = 1, double dDefaultViewRatio = 1);
    virtual ~DialogGetRasterParameters ();

    unsigned int getXSize ();
    unsigned int getYSize ();
    unsigned int getNSamples ();
    double getViewRatio();

 private:
    wxTextCtrl* m_pTextCtrlXSize;
    wxTextCtrl* m_pTextCtrlYSize;
    wxTextCtrl* m_pTextCtrlNSamples;
    wxTextCtrl* m_pTextCtrlViewRatio;

    int m_iDefaultXSize;
    int m_iDefaultYSize;
    int m_iDefaultNSamples;
    double m_dDefaultViewRatio;
};


class DialogGetProjectionParameters : public wxDialog
{
 public:
    DialogGetProjectionParameters (wxWindow* pParent, int iDefaultNDet = 0,
      int iDefaultNView = 0, int iDefaultOffsetView = 0, int iDefaultNSamples = 1, double dDefaultRotAngle = 1.,
      double dDefaultFocalLength = 1, double dDefaultCenterDetectorLength = 1, double dDefaultViewRatio = 1.,
      double dDefaultScanRatio = 1., int iDefaultGeometry = Scanner::GEOMETRY_PARALLEL, int iDefaultTrace = Trace::TRACE_NONE);
    ~DialogGetProjectionParameters ();

    unsigned int getNDet ();
    unsigned int getNView ();
          unsigned int getOffsetView ();
    unsigned int getNSamples ();
    int getTrace ();

    double getRotAngle ();
    double getViewRatio ();
    double getScanRatio ();
    double getFocalLengthRatio ();
    double getCenterDetectorLengthRatio ();
    const char* getGeometry();

 private:
    wxTextCtrl* m_pTextCtrlNDet;
    wxTextCtrl* m_pTextCtrlNView;
    wxTextCtrl* m_pTextCtrlOffsetView;
    wxTextCtrl* m_pTextCtrlNSamples;
    wxTextCtrl* m_pTextCtrlRotAngle;
    wxTextCtrl* m_pTextCtrlFocalLength;
    wxTextCtrl* m_pTextCtrlCenterDetectorLength;
    wxTextCtrl* m_pTextCtrlViewRatio;
    wxTextCtrl* m_pTextCtrlScanRatio;
    StringValueAndTitleRadioBox* m_pRadioBoxGeometry;
    StringValueAndTitleRadioBox* m_pRadioBoxTrace;

    int m_iDefaultNDet;
    int m_iDefaultNView;
          int m_iDefaultOffsetView;
    int m_iDefaultNSamples;
    int m_iDefaultTrace;
    int m_iDefaultGeometry;
    double m_dDefaultRotAngle;
    double m_dDefaultFocalLength;
    double m_dDefaultCenterDetectorLength;
    double m_dDefaultViewRatio;
    double m_dDefaultScanRatio;
};


#include "backprojectors.h"
class DialogGetReconstructionParameters : public wxDialog
{
 public:
    DialogGetReconstructionParameters (wxWindow* pParent, int iDefaultXSize = 0, int iDefaultYSize = 0,
      int iDefaultFilterID = SignalFilter::FILTER_ABS_BANDLIMIT, double dDefaultFilterParam = 1.,
      int iDefaultFilterMethodID = ProcessSignal::FILTER_METHOD_CONVOLUTION,
      int iDefaultFilterGeneration = ProcessSignal::FILTER_GENERATION_DIRECT,
      int iDefaultZeropad = 3, int iDefaultInterpID = Backprojector::INTERP_LINEAR,
      int iDefaultInterpParam = 1, int iDefaultBackprojectID = Backprojector::BPROJ_IDIFF,
      int iDefaultTrace = Trace::TRACE_NONE, ReconstructionROI* pROI = NULL);
    virtual ~DialogGetReconstructionParameters ();

    unsigned int getXSize();
    unsigned int getYSize();
    const char* getFilterName();
    double getFilterParam();
    const char* getFilterMethodName();
    unsigned int getZeropad();
    const char* getFilterGenerationName();
    const char* getInterpName();
    unsigned int getInterpParam();
    const char* getBackprojectName();
    void getROI (ReconstructionROI* pROI);
    int getTrace ();

 private:
    wxTextCtrl* m_pTextCtrlXSize;
    wxTextCtrl* m_pTextCtrlYSize;
    wxTextCtrl* m_pTextCtrlZeropad;
    wxTextCtrl* m_pTextCtrlFilterParam;
    wxTextCtrl* m_pTextCtrlInterpParam;
    wxTextCtrl* m_pTextCtrlRoiXMin;
    wxTextCtrl* m_pTextCtrlRoiXMax;
    wxTextCtrl* m_pTextCtrlRoiYMin;
    wxTextCtrl* m_pTextCtrlRoiYMax;
    StringValueAndTitleRadioBox* m_pRadioBoxFilter;
    StringValueAndTitleRadioBox* m_pRadioBoxFilterMethod;
    StringValueAndTitleRadioBox* m_pRadioBoxFilterGeneration;
    StringValueAndTitleRadioBox* m_pRadioBoxInterp;
    StringValueAndTitleRadioBox* m_pRadioBoxBackproject;
    StringValueAndTitleRadioBox* m_pRadioBoxTrace;

    int m_iDefaultXSize;
    int m_iDefaultYSize;
    double m_dDefaultFilterParam;
    int m_iDefaultZeropad;
    int m_iDefaultInterpParam;
    double m_dDefaultRoiXMin;
    double m_dDefaultRoiXMax;
    double m_dDefaultRoiYMin;
    double m_dDefaultRoiYMax;
};


class DialogGetFilterParameters : public wxDialog
{
 public:
    DialogGetFilterParameters (wxWindow* pParent, int iDefaultXSize = 0, int iDefaultYSize = 0, int iDefaultFilterID = SignalFilter::FILTER_BANDLIMIT, double dDefaultFilterParam = 1., double dDefaultBandwidth = 1., int iDefaultDomain = SignalFilter::DOMAIN_SPATIAL, double dDefaultInputScale = 1., double dDefaultOutputScale = 1.);
    virtual ~DialogGetFilterParameters ();

    unsigned int getXSize();
    unsigned int getYSize();
    const char* getFilterName();
    const char* getDomainName();
    double getFilterParam();
    double getInputScale();
    double getOutputScale();
    double getBandwidth();

 private:
    wxTextCtrl* m_pTextCtrlXSize;
    wxTextCtrl* m_pTextCtrlYSize;
    wxTextCtrl* m_pTextCtrlFilterParam;
    wxTextCtrl* m_pTextCtrlOutputScale;
    wxTextCtrl* m_pTextCtrlInputScale;
    wxTextCtrl* m_pTextCtrlBandwidth;

    StringValueAndTitleRadioBox* m_pRadioBoxFilter;
    StringValueAndTitleRadioBox* m_pRadioBoxDomain;

    int m_iDefaultXSize;
    int m_iDefaultYSize;
    double m_dDefaultFilterParam;
    double m_dDefaultBandwidth;
    double m_dDefaultOutputScale;
    double m_dDefaultInputScale;
    int m_iDefaultDomain;
};

class DialogExportParameters : public wxDialog
{
 public:
    DialogExportParameters (wxWindow* pParent, int iDefaultFormatID);
    virtual ~DialogExportParameters () {}

    const char* getFormatName();

 private:
    StringValueAndTitleRadioBox* m_pRadioBoxFormat;
};

class DialogImportParameters : public wxDialog
{
 public:
    DialogImportParameters (wxWindow* pParent, int iDefaultFormatID);
    virtual ~DialogImportParameters () {}

    const char* getFormatName();

 private:
    StringValueAndTitleRadioBox* m_pRadioBoxFormat;
};

class DialogAutoScaleParameters : public wxDialog
{
 public:
    DialogAutoScaleParameters (wxWindow* pParent, double mean, double mode, double median, double stddev, double dDefaultScaleFactor = 1.);
    virtual ~DialogAutoScaleParameters() {}

    bool getMinMax (double* pMin, double* pMax);
    double getAutoScaleFactor ();

 private:
    const double m_dMean;
        const double m_dMode;
        const double m_dMedian;
        const double m_dStdDev;

    wxTextCtrl* m_pTextCtrlStdDevFactor;
    wxRadioBox* m_pRadioBoxCenter;
};

class DialogGetXYSize : public wxDialog
{
 public:
    DialogGetXYSize (wxWindow* pParent, wxChar const * pwszTitle, int iDefaultXSize = 1, int iDefaultYSize = 1);
    virtual ~DialogGetXYSize ();

    unsigned int getXSize ();
    unsigned int getYSize ();

 private:
    wxTextCtrl* m_pTextCtrlXSize;
    wxTextCtrl* m_pTextCtrlYSize;

    unsigned int m_iDefaultXSize;
    unsigned int m_iDefaultYSize;
};


class DialogGetConvertPolarParameters : public wxDialog
{
 public:
   DialogGetConvertPolarParameters (wxWindow* pParent, wxChar const * pwszTitle, int iDefaultXSize = 0,
     int iDefaultYSize = 0, int iDefaultInterpolationID = Projections::POLAR_INTERP_BILINEAR,
     int iDefaultZeropad = 3, int iHelpID = IDH_DLG_POLAR);
   virtual ~DialogGetConvertPolarParameters ();

    unsigned int getXSize();
    unsigned int getYSize();
    const char* getInterpolationName();
    unsigned int getZeropad();

 private:
    wxTextCtrl* m_pTextCtrlXSize;
    wxTextCtrl* m_pTextCtrlYSize;
    wxTextCtrl* m_pTextCtrlZeropad;

    StringValueAndTitleRadioBox* m_pRadioBoxInterpolation;

    int m_iDefaultXSize;
    int m_iDefaultYSize;
    int m_iDefaultZeropad;
};


#endif

