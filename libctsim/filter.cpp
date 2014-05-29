/*****************************************************************************
** File IDENTIFICATION
**
**     Name:                   filter.cpp
**     Purpose:                Routines for signal-procesing filters
**     Progammer:              Kevin Rosenberg
**     Date Started:           Aug 1984
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

#include "ct.h"

int SignalFilter::N_INTEGRAL=500;  //static member

const int SignalFilter::FILTER_INVALID = -1 ;
const int SignalFilter::FILTER_ABS_BANDLIMIT = 0;       // filter times |x|
const int SignalFilter::FILTER_ABS_G_HAMMING = 1;
const int SignalFilter::FILTER_ABS_HANNING = 2;
const int SignalFilter::FILTER_ABS_COSINE = 3;
const int SignalFilter::FILTER_ABS_SINC = 4;
const int SignalFilter::FILTER_SHEPP = 5;
const int SignalFilter::FILTER_BANDLIMIT = 6;
const int SignalFilter::FILTER_SINC = 7;
const int SignalFilter::FILTER_G_HAMMING = 8;
const int SignalFilter::FILTER_HANNING = 9;
const int SignalFilter::FILTER_COSINE = 10;
const int SignalFilter::FILTER_TRIANGLE = 11;

const int SignalFilter::s_iReconstructFilterCount = 4;

const char* const SignalFilter::s_aszFilterName[] = {
  "abs_bandlimit",
  "abs_hamming",
  "abs_hanning",
  "abs_cosine",
  "shepp",
  "abs_sinc",
  "bandlimit",
  "sinc",
  "hamming",
  "hanning",
  "cosine",
  "triangle"
};

const char* const SignalFilter::s_aszFilterTitle[] = {
  "Abs(w) * Bandlimit",
  "Abs(w) * Hamming",
  "Abs(w) * Hanning",
  "Abs(w) * Cosine",
  "Shepp",
  "Abs(w) * Sinc",
  "Bandlimit",
  "Sinc",
  "Hamming",
  "Hanning",
  "Cosine",
  "Triangle"
};

const int SignalFilter::s_iFilterCount = sizeof(s_aszFilterName) / sizeof(const char*);


const int SignalFilter::DOMAIN_INVALID = -1;
const int SignalFilter::DOMAIN_FREQUENCY = 0;
const int SignalFilter::DOMAIN_SPATIAL = 1;

const char* const SignalFilter::s_aszDomainName[] = {
  "frequency",
  "spatial",
};

const char* const SignalFilter::s_aszDomainTitle[] = {
  "Frequency",
  "Spatial",
};

const int SignalFilter::s_iDomainCount = sizeof(s_aszDomainName) / sizeof(const char*);


/* NAME
*   SignalFilter::SignalFilter     Construct a signal
*
* SYNOPSIS
*   f = SignalFilter (filt_type, bw, filterMin, filterMax, n, param, domain, analytic)
*   double f            Generated filter vector
*   int filt_type       Type of filter wanted
*   double bw           Bandwidth of filter
*   double filterMin, filterMax Filter limits
*   int nFilterPoints   Number of points in signal
*   double param        General input parameter to filters
*   int domain          FREQUENCY or SPATIAL domain wanted
*/

SignalFilter::SignalFilter (const char* szFilterName, double dFilterMinimum, double dFilterMaximum, int nFilterPoints, double dBandwidth, double dFilterParam, const char* szDomainName)
: m_adFilter(NULL), m_fail(false)
{
  m_idFilter = convertFilterNameToID (szFilterName);
  if (m_idFilter == FILTER_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid Filter name ";
    m_failMessage += szFilterName;
    return;
  }
  m_idDomain = convertDomainNameToID (szDomainName);
  if (m_idDomain == DOMAIN_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid domain name ";
    m_failMessage += szDomainName;
    return;
  }
  init (m_idFilter, dFilterMinimum, dFilterMaximum, nFilterPoints, dBandwidth, dFilterParam, m_idDomain);
}

SignalFilter::SignalFilter (const int idFilter, double dFilterMinimum, double dFilterMaximum, int nFilterPoints, double dBandwidth, double dFilterParam, const int idDomain)
: m_adFilter(NULL), m_fail(false)
{
  init (idFilter, dFilterMinimum, dFilterMaximum, nFilterPoints, dBandwidth, dFilterParam, idDomain);
}

SignalFilter::SignalFilter (const char* szFilterName, const char* szDomainName, double dBandwidth, double dFilterParam)
: m_adFilter(NULL), m_fail(false)
{
  m_nFilterPoints = 0;
  m_dBandwidth = dBandwidth;
  m_dFilterParam = dFilterParam;
  m_idFilter = convertFilterNameToID (szFilterName);
  if (m_idFilter == FILTER_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid Filter name ";
    m_failMessage += szFilterName;
    return;
  }
  m_idDomain = convertDomainNameToID (szDomainName);
  if (m_idDomain == DOMAIN_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid domain name ";
    m_failMessage += szDomainName;
    return;
  }
}

void
SignalFilter::init (const int idFilter, double dFilterMinimum, double dFilterMaximum, int nFilterPoints, double dBandwidth, double dFilterParam, const int idDomain)
{
  m_idFilter = idFilter;
  m_idDomain = idDomain;
  if (m_idFilter == FILTER_INVALID || m_idDomain == DOMAIN_INVALID) {
    m_fail = true;
    return;
  }
  if (nFilterPoints < 2) {
    m_fail = true;
    m_failMessage = "Number of filter points ";
    m_failMessage += nFilterPoints;
    m_failMessage = " less than 2";
    return;
  }

  m_nameFilter = convertFilterIDToName (m_idFilter);
  m_nameDomain = convertDomainIDToName (m_idDomain);
  m_nFilterPoints = nFilterPoints;
  m_dFilterParam = dFilterParam;
  m_dBandwidth = dBandwidth;
  m_dFilterMin = dFilterMinimum;
  m_dFilterMax = dFilterMaximum;

  m_dFilterInc = (m_dFilterMax - m_dFilterMin) / (m_nFilterPoints - 1);
  m_adFilter = new double [m_nFilterPoints];

  if (m_idDomain == DOMAIN_FREQUENCY)
    createFrequencyFilter (m_adFilter);
  else if (m_idDomain == DOMAIN_SPATIAL)
    createSpatialFilter (m_adFilter);
}


SignalFilter::~SignalFilter (void)
{
  delete [] m_adFilter;
}

void
SignalFilter::createFrequencyFilter (double* adFilter) const
{
  double x;
  int i;
  for (x = m_dFilterMin, i = 0; i < m_nFilterPoints; x += m_dFilterInc, i++)
    adFilter[i] = frequencyResponse (x);
}


void
SignalFilter::createSpatialFilter (double* adFilter) const
{
  if (m_idFilter == FILTER_SHEPP) {
    double a = 2 * m_dBandwidth;
    double c = - 4. / (a * a);
    int center = (m_nFilterPoints - 1) / 2;
    int sidelen = center;
    m_adFilter[center] = 4. / (a * a);

    for (int i = 1; i <= sidelen; i++ )
      m_adFilter [center + i] = m_adFilter [center - i] = c / (4 * (i * i) - 1);
  } else {
    double x = m_dFilterMin;
    for (int i = 0; i < m_nFilterPoints; i++, x += m_dFilterInc) {
      if (haveAnalyticSpatial(m_idFilter))
        m_adFilter[i] = spatialResponseAnalytic (x);
      else
        m_adFilter[i] = spatialResponseCalc (x);
    }
  }
}

int
SignalFilter::convertFilterNameToID (const char *filterName)
{
  int filterID = FILTER_INVALID;

  for (int i = 0; i < s_iFilterCount; i++)
    if (strcasecmp (filterName, s_aszFilterName[i]) == 0) {
      filterID = i;
      break;
    }

    return (filterID);
}

const char *
SignalFilter::convertFilterIDToName (const int filterID)
{
  static const char *name = "";

  if (filterID >= 0 && filterID < s_iFilterCount)
    return (s_aszFilterName [filterID]);

  return (name);
}

const char *
SignalFilter::convertFilterIDToTitle (const int filterID)
{
  static const char *title = "";

  if (filterID >= 0 && filterID < s_iFilterCount)
    return (s_aszFilterTitle [filterID]);

  return (title);
}

int
SignalFilter::convertDomainNameToID (const char* const domainName)
{
  int dID = DOMAIN_INVALID;

  for (int i = 0; i < s_iDomainCount; i++)
    if (strcasecmp (domainName, s_aszDomainName[i]) == 0) {
      dID = i;
      break;
    }

    return (dID);
}

const char *
SignalFilter::convertDomainIDToName (const int domainID)
{
  static const char *name = "";

  if (domainID >= 0 && domainID < s_iDomainCount)
    return (s_aszDomainName [domainID]);

  return (name);
}

const char *
SignalFilter::convertDomainIDToTitle (const int domainID)
{
  static const char *title = "";

  if (domainID >= 0 && domainID < s_iDomainCount)
    return (s_aszDomainTitle [domainID]);

  return (title);
}


double
SignalFilter::response (double x)
{
  double response = 0;

  if (m_idDomain == DOMAIN_SPATIAL)
    response = spatialResponse (m_idFilter, m_dBandwidth, x, m_dFilterParam);
  else if (m_idDomain == DOMAIN_FREQUENCY)
    response = frequencyResponse (m_idFilter, m_dBandwidth, x, m_dFilterParam);

  return (response);
}


double
SignalFilter::spatialResponse (int filterID, double bw, double x, double param)
{
  if (haveAnalyticSpatial(filterID))
    return spatialResponseAnalytic (filterID, bw, x, param);
  else
    return spatialResponseCalc (filterID, bw, x, param, N_INTEGRAL);
}

void
SignalFilter::copyFilterData (double* pdFilter, const int iStart, const int nPoints) const
{
  int iFirst = clamp (iStart, 0, m_nFilterPoints - 1);
  int iLast = clamp (iFirst + nPoints - 1, 0, m_nFilterPoints - 1);

  for (int i = iFirst; i <= iLast; i++)
    pdFilter[i - iFirst] = m_adFilter[i];
}

/* NAME
*   filter_spatial_response_calc        Calculate filter by discrete inverse fourier
*                                       transform of filters's frequency
*                                       response
*
* SYNOPSIS
*   y = filter_spatial_response_calc (filt_type, x, m_bw, param, n)
*   double y                    Filter's response in spatial domain
*   int filt_type               Type of filter (definitions in ct.h)
*   double x                    Spatial position to evaluate filter
*   double m_bw                 Bandwidth of window
*   double param                General parameter for various filters
*   int n                       Number of points to calculate integrations
*/

double
SignalFilter::spatialResponseCalc (double x) const
{
  return (spatialResponseCalc (m_idFilter, m_dBandwidth, x, m_dFilterParam, N_INTEGRAL));
}

double
SignalFilter::spatialResponseCalc (int filterID, double bw, double x, double param, int n)
{
  double zmin, zmax;

  if (filterID == FILTER_TRIANGLE) {
    zmin = 0;
    zmax = bw;
  } else {
    zmin = 0;
    zmax = bw / 2;
  }
  double zinc = (zmax - zmin) / (n - 1);

  double z = zmin;
  double* q = new double [n];
  for (int i = 0; i < n; i++, z += zinc)
    q[i] = frequencyResponse (filterID, bw, z, param) * cos (TWOPI * z * x);

  double y = 2 * integrateSimpson (zmin, zmax, q, n);
  delete q;

  return (y);
}


/* NAME
*    filter_frequency_response                  Return filter frequency response
*
* SYNOPSIS
*    h = filter_frequency_response (filt_type, u, m_bw, param)
*    double h                   Filters frequency response at u
*    int filt_type              Type of filter
*    double u                   Frequency to evaluate filter at
*    double m_bw                        Bandwidth of filter
*    double param               General input parameter for various filters
*/

double
SignalFilter::frequencyResponse (double u) const
{
  return frequencyResponse (m_idFilter, m_dBandwidth, u, m_dFilterParam);
}


double
SignalFilter::frequencyResponse (int filterID, double bw, double u, double param)
{
  double q;
  double au = fabs (u);
  double abw = fabs (bw);

  switch (filterID) {
  case FILTER_BANDLIMIT:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0.;
    else
      q = 1;
    break;
  case FILTER_ABS_BANDLIMIT:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0.;
    else
      q = au;
    break;
  case FILTER_TRIANGLE:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0;
    else
      q = 1 - au / abw;
    break;
  case FILTER_COSINE:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0;
    else
      q = cos(PI * au / abw);
    break;
  case FILTER_ABS_COSINE:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0;
    else
      q = au * cos(PI * au / abw);
    break;
  case FILTER_SINC:
    q = abw * sinc (PI * abw * au, 1.);
    break;
  case FILTER_ABS_SINC:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0;
    else
      q = au * abw * sinc (PI * abw * au, 1.);
    break;
  case FILTER_HANNING:
    param = 0.5;
    // follow through to G_HAMMING
  case FILTER_G_HAMMING:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0;
    else
      q = param + (1 - param) * cos (TWOPI * au / abw);
    break;
  case FILTER_ABS_HANNING:
    param = 0.5;
    // follow through to ABS_G_HAMMING
  case FILTER_ABS_G_HAMMING:
    if (au >= (abw / 2) + F_EPSILON)
      q = 0;
    else
      q = au * (param + (1 - param) * cos(TWOPI * au / abw));
    break;
  default:
    q = 0;
    sys_error (ERR_WARNING, "Frequency response for filter %d not implemented [filter_frequency_response]", filterID);
    break;
  }

  return (q);
}



/* NAME
*   filter_spatial_response_analytic                    Calculate filter by analytic inverse fourier
*                               transform of filters's frequency
*                               response
*
* SYNOPSIS
*   y = filter_spatial_response_analytic (filt_type, x, m_bw, param)
*   double y                    Filter's response in spatial domain
*   int filt_type               Type of filter (definitions in ct.h)
*   double x                    Spatial position to evaluate filter
*   double m_bw                 Bandwidth of window
*   double param                General parameter for various filters
*/

double
SignalFilter::spatialResponseAnalytic (double x) const
{
  return spatialResponseAnalytic (m_idFilter, m_dBandwidth, x, m_dFilterParam);
}

const bool
SignalFilter::haveAnalyticSpatial (int filterID)
{
  bool haveAnalytic = false;

  switch (filterID) {
  case FILTER_BANDLIMIT:
  case FILTER_TRIANGLE:
  case FILTER_COSINE:
  case FILTER_G_HAMMING:
  case FILTER_HANNING:
  case FILTER_ABS_BANDLIMIT:
  case FILTER_ABS_COSINE:
  case FILTER_ABS_G_HAMMING:
  case FILTER_ABS_HANNING:
  case FILTER_SHEPP:
  case FILTER_SINC:
    haveAnalytic = true;
    break;
  default:
    break;
  }

  return (haveAnalytic);
}

double
SignalFilter::spatialResponseAnalytic (int filterID, double bw, double x, double param)
{
  double q, temp;
  double u = TWOPI * x;
  double w = bw / 2;
  double b = PI / bw;
  double b2 = TWOPI / bw;

  switch (filterID) {
  case FILTER_BANDLIMIT:
    q = bw * sinc(u * w, 1.0);
    break;
  case FILTER_TRIANGLE:
    temp = sinc (u * w, 1.0);
    q = bw * temp * temp;
    break;
  case FILTER_COSINE:
    q = sinc(b-u,w) + sinc(b+u,w);
    break;
  case FILTER_HANNING:
    param = 0.5;
    // follow through to G_HAMMING
  case FILTER_G_HAMMING:
    q = 2 * param * sin(u*w)/u + (1-param) * (sinc(b2-u, w) + sinc(b2+u, w));
    break;
  case FILTER_ABS_BANDLIMIT:
    q = 2 * integral_abscos (u, w);
    break;
  case FILTER_ABS_COSINE:
    q = integral_abscos(b-u,w) + integral_abscos(b+u,w);
    break;
  case FILTER_ABS_HANNING:
    param = 0.5;
    // follow through to ABS_G_HAMMING
  case FILTER_ABS_G_HAMMING:
    q = 2 * param * integral_abscos(u,w) +
      (1-param)*(integral_abscos(u-b2,w)+integral_abscos(u+b2,w));
    break;
  case FILTER_SHEPP:
    if (fabs (u) < 1E-7)
      q = 4. / (PI * bw * bw);
    else
      q = fabs ((2 / bw) * sin (u * w)) * sinc (u * w, 1.) * sinc (u * w, 1.);
    break;
  case FILTER_SINC:
    if (fabs (x) < bw / 2)
      q = 1.;
    else
      q = 0.;
    break;
  case FILTER_ABS_SINC:
  default:
    sys_error (ERR_WARNING, "Analytic filter type %d not implemented [filter_spatial_response_analytic]", filterID);
    q = 0;
    break;
  }

  return (q);
}



// Functions that are inline in filter.h


//  sinc                        Return sin(x)/x function
//   v = sinc (x, mult)
// Calculates sin(x * mult) / x;

//  integral_abscos     Returns integral of u*cos(u)
//
//   q = integral_abscos (u, w)
//   double q                   Integral value
//   double u                   Integration variable
//   double w                   Upper integration boundary
// Returns the value of integral of u*cos(u)*dV for V = 0 to w



