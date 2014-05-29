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


/* NAME
*    integrateSimpson         Integrate array of data by Simpson's rule
*
* SYNOPSIS
*    double integrateSimpson (xmin, xmax, y, np)
*    double xmin, xmax          Extent of integration
*    double y[]         Function values to be integrated
*    int np                     number of data points
*                               (must be an odd number and at least 3)
*
* RETURNS
*    integrand of function
*/

double
integrateSimpson (const double xmin, const double xmax, const double *y, const int np)
{
  if (np < 2)
    return (0.);
  else if (np == 2)
    return ((xmax - xmin) * (y[0] + y[1]) / 2);

  double area = 0;
  int nDiv = (np - 1) / 2;  // number of divisions
  double width = (xmax - xmin) / (double) (np - 1);     // width of cells

  for (int i = 1; i <= nDiv; i++) {
    int xr = 2 * i;
    int xl = xr - 2;       // 2 * (i - 1) == 2 * i - 2 == xr - 2
    int xm = xr - 1;       // (xl+xr)/2 == (xr+xr-2)/2 == (2*xr-2)/2 = xr-1

    area += (width / 3.0) * (y[xl] + 4.0 * y[xm] + y[xr]);
  }

  if ((np & 1) == 0)            /* do last trapazoid */
    area += width * (y[np-2] + y[np-1]) / 2;

  return (area);
}


/* NAME
*    normalizeAngle       Normalize angle to 0 to 2 * PI range
*
* SYNOPSIS
*    t = normalizeAngle (theta)
*    double t          Normalized angle
*    double theta     Input angle
*/

double
normalizeAngle (double theta)
{
  while (theta < 0.)
    theta += TWOPI;
  while (theta >= TWOPI)
    theta -= TWOPI;

  return (theta);
}


void
vectorNumericStatistics (std::vector<double> vec, const int nPoints, double& min, double& max, double& mean, double& mode, double& median, double& stddev)
{
  if (nPoints <= 0)
    return;

  mean = 0;
  min = vec[0];
  max = vec[0];
  int i;
  for (i = 0; i < nPoints; i++) {
    double v = vec[i];
    if (v > max)
      max = v;
    if (v < min)
      min = v;
    mean += v;
  }
  mean /= nPoints;

  static const int nbin = 1024;
  int hist[ nbin ] = {0};
  double spread = max - min;
  mode = 0;
  stddev = 0;
  for (i = 0; i < nPoints; i++) {
    double v = vec[i];
    int b = static_cast<int>((((v - min) / spread) * (nbin - 1)) + 0.5);
    hist[b]++;
    double diff = (v - mean);
    stddev += diff * diff;
  }
  stddev = sqrt (stddev / nPoints);

  int max_binindex = 0;
  int max_bin = -1;
  for (int ibin = 0; ibin < nbin; ibin++) {
    if (hist[ibin] > max_bin) {
      max_bin = hist[ibin];
      max_binindex = ibin;
    }
  }

  mode = (max_binindex * spread / (nbin - 1)) + min;

  std::sort(vec.begin(), vec.end());

  if (nPoints % 2)  // Odd
    median = vec[((nPoints - 1) / 2)];
  else        // Even
    median = (vec[ (nPoints / 2) - 1 ] + vec[ nPoints / 2 ]) / 2;
}


