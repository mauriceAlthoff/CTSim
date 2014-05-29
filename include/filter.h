/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         filter.h
**      Purpose:      Signal filter header file
**      Programmer:   Kevin Rosenberg
**      Date Started: June 2000
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

#ifndef FILTER_H
#define FILTER_H


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FFTW
#include <fftw3.h>
#endif


// CLASS IDENTIFICATION
//    SignalFilter       A filter used to process signals
//
// CONTAINS
//    signal vector
//
//    Can create either a time/spatial waveform or a frequency signal
//    Waveforms can be created either by direct calculation or by inverse fourier transform

class SignalFilter {
 public:
    static const int FILTER_INVALID;
    static const int FILTER_ABS_BANDLIMIT;      // filter times |x|
    static const int FILTER_ABS_SINC;
    static const int FILTER_ABS_G_HAMMING;
    static const int FILTER_ABS_HANNING;
    static const int FILTER_ABS_COSINE;
    static const int FILTER_SHEPP;
    static const int FILTER_BANDLIMIT;
    static const int FILTER_SINC;
    static const int FILTER_G_HAMMING;
    static const int FILTER_HANNING;
    static const int FILTER_COSINE;
    static const int FILTER_TRIANGLE;

    static const int DOMAIN_INVALID;
    static const int DOMAIN_FREQUENCY;
    static const int DOMAIN_SPATIAL;

    SignalFilter (const char* szFilterName, double dFilterMinimum, double dFilterMaximum, int nFilterPoints, double dBandwidth, double dFilterParam, const char* szDomainName);

    SignalFilter (const int idFilter, double dFilterMinimum, double dFilterMaximum, int nFilterPoints, double dBandwidth, double dFilterParam, const int idDomain);

    SignalFilter (const char* szFilterName, const char* szDomainName, double dBandwidth, double dFilterParam);

    ~SignalFilter (void);

    double* getFilter (void) const
      { return m_adFilter; }

    bool fail(void) const       {return m_fail;}
    const std::string& failMessage(void) const {return m_failMessage;}

    const std::string& nameFilter(void) const   { return m_nameFilter;}
    const std::string& nameDomain(void) const   { return m_nameDomain;}
    const int idFilter(void) const      { return m_idFilter;}
    const int idDomain(void) const      { return m_idDomain;}

    int getNFilterPoints (void) const  { return m_nFilterPoints; }
    const double getFilterMin(void) const {return m_dFilterMin;}
    const double getFilterMax(void) const {return m_dFilterMax;}
    const double getFilterIncrement(void) const {return m_dFilterInc;}
    void copyFilterData(double *pdFilter, const int iStart, const int nPoints) const;

    double response (double x);

    static double spatialResponse (int fType, double bw, double x, double param);

    static double frequencyResponse (int fType, double bw, double u, double param);

    static double spatialResponseAnalytic (int fType, double bw, double x, double param);

    static double spatialResponseCalc (int fType, double bw, double x, double param, int nIntegral);

    static void setNumIntegral(int nIntegral) {N_INTEGRAL = nIntegral;}

  static const int getFilterCount() {return s_iFilterCount;}
  static const int getReconstructFilterCount() { return s_iReconstructFilterCount; }

  static const char* const* getFilterNameArray() {return s_aszFilterName;}
  static const char* const* getFilterTitleArray() {return s_aszFilterTitle;}
  static int convertFilterNameToID (const char* const filterName);
  static const char* convertFilterIDToName (const int idFilter);
  static const char* convertFilterIDToTitle (const int idFilter);

  static const int getDomainCount() {return s_iDomainCount;}
  static const char* const* getDomainNameArray() {return s_aszDomainName;}
  static const char* const* getDomainTitleArray() {return s_aszDomainTitle;}
  static int convertDomainNameToID (const char* const domainName);
  static const char* convertDomainIDToName (const int idDomain);
  static const char* convertDomainIDToTitle (const int idDomain);

  static double sinc (double x)
      { return (fabs(x) > F_EPSILON ? (sin (x) / x) : 1.0); }

  static double sinc (double x, double mult)
      { return (fabs(x) > F_EPSILON ? (sin (x * mult) / x) : 1.0); }

 private:
    int m_nFilterPoints;
    double m_dBandwidth;
    double m_dFilterParam;
    double m_dFilterInc;
    double m_dFilterMin;
    double m_dFilterMax;
    double* m_adFilter;

    std::string m_nameFilter;
    std::string m_nameDomain;
    int m_idFilter;
    int m_idDomain;

    bool m_fail;
    std::string m_failMessage;

    static const char* const s_aszFilterName[];
    static const char* const s_aszFilterTitle[];
    static const int s_iFilterCount;
    static const int s_iReconstructFilterCount;
    static const char* const s_aszDomainName[];
    static const char* const s_aszDomainTitle[];
    static const int s_iDomainCount;
    static int N_INTEGRAL;

    static const bool haveAnalyticSpatial (const int filterID);

    void init (const int idFilter, double dFilterMin, double dFilterMax, int nFilterPoints, double dBandwidth, double dFilterParam, const int idDomain);

    void createFrequencyFilter (double* x) const;
    void createSpatialFilter (double* x) const;

    double spatialResponseCalc (double x) const;
    double spatialResponseAnalytic (double x) const;
    double frequencyResponse (double u) const;

    static double integral_abscos (double u, double w)
    { return (fabs (u) > F_EPSILON ? (cos (u * w) - 1) / (u * u) + w / u * sin (u * w) : (w * w / 2)); }
};

#endif
