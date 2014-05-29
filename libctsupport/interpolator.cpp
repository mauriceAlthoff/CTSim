/*****************************************************************************
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


#include "ctsupport.h"
#include "interpolator.h"


CubicPolyInterpolator::CubicPolyInterpolator (const double* const y, const int n)
  : m_pdY(y), m_n(n)
{
  if (m_n < 2)
    sys_error (ERR_SEVERE, "Too few points (%d) in CubicPolyInterpolator", m_n);
}

CubicPolyInterpolator::~CubicPolyInterpolator ()
{
}


double
CubicPolyInterpolator::interpolate (double x)
{
  int lo = static_cast<int>(floor(x)) - 1;
  int hi = lo + 3;

  if (lo < -1) {
#ifdef DEBUG
    sys_error (ERR_WARNING, "x=%f, out of range [CubicPolyInterpolator]", x);
#endif
    return (0);
  } else if (lo == -1)  // linear interpolate at between x = 0 & 1
    return m_pdY[0] + x * (m_pdY[1] - m_pdY[0]);

  if (hi > m_n) {
#ifdef DEBUG
    sys_error (ERR_WARNING, "x=%f, out of range [CubicPolyInterpolator]", x);
#endif
    return (0);
  } else if (hi == m_n) {// linear interpolate between x = (n-2) and (n-1)
    double frac = x - (lo + 1);
    return m_pdY[m_n - 2] + frac * (m_pdY[m_n - 1] - m_pdY[m_n - 2]);
  }

  // Lagrange formula for N=4 (cubic)

  double xd_0 = x - lo;
  double xd_1 = x - (lo + 1);
  double xd_2 = x - (lo + 2);
  double xd_3 = x - (lo + 3);

  static double oneSixth = (1. / 6.);

  double y = xd_1 * xd_2 * xd_3 * -oneSixth * m_pdY[lo];
  y += xd_0 * xd_2 * xd_3 * 0.5 * m_pdY[lo+1];
  y += xd_0 * xd_1 * xd_3 * -0.5 * m_pdY[lo+2];
  y += xd_0 * xd_1 * xd_2 * oneSixth * m_pdY[lo+3];

  return (y);
}



CubicSplineInterpolator::CubicSplineInterpolator (const double* const y, const int n)
  : m_pdY(y), m_n(n)
{
  // Precalculate 2nd derivative of y and put in m_pdY2
  // Calculated by solving set of simultaneous CubicSpline spline equations
  // Only n-2 CubicSpline spline equations, but able to make two more
  // equations by setting second derivative to 0 at ends

  m_pdY2 = new double [n];
  m_pdY2[0] = 0;   // second deriviative = 0 at beginning and end
  m_pdY2[n-1] = 0;

  double* temp = new double [n - 1];
  temp[0] = 0;
  int i;
  for (i = 1; i < n - 1; i++) {
    double t = 2 + (0.5 * m_pdY2[i-1]);
    temp[i] = y[i+1] + y[i-1] - y[i] - y[i];
    temp[i] = (3 * temp[i] - 0.5 * temp[i-1]) / t;
    m_pdY2[i] = -0.5 / t;
  }

  for (i = n - 2; i >= 0; i--)
    m_pdY2[i] = temp[i] + m_pdY2[i] * m_pdY2[i + 1];

  delete temp;
}

CubicSplineInterpolator::~CubicSplineInterpolator ()
{
  delete m_pdY2;
}


double
CubicSplineInterpolator::interpolate (double x)
{
  const static double oneSixth = (1. / 6.);
  int lo = static_cast<int>(floor(x));
  int hi = lo + 1;

  if (lo < 0 || hi >= m_n) {
#ifdef DEBUG
    sys_error (ERR_SEVERE, "x out of bounds [CubicSplineInterpolator::interpolate]");
#endif
    return (0);
  }

  double loFr = hi - x;
  double hiFr = 1 - loFr;
  double y = loFr * m_pdY[lo] + hiFr * m_pdY[hi];
  y += oneSixth * ((loFr*loFr*loFr - loFr) * m_pdY2[lo] + (hiFr*hiFr*hiFr - hiFr) * m_pdY2[hi]);

  return y;
}




