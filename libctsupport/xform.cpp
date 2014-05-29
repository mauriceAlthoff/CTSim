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
 *   rotate2d           Rotates an array of 2 dimensional vectors
 *
 * SYNOPSIS
 *   rotate2d (x, y, n, angle)
 *   double x[], y[]    Array of points
 *   int n              Number of points in array
 *   double angle       Rotation angle (counter-clockwise)
 */

void
rotate2d (double x[], double y[], int n, double angle)
{
  double cos_theta = cos (angle);
  double sin_theta = sin (angle);

  for (int i = 0; i < n; i++) {
    double xrot = x[i] * cos_theta - y[i] * sin_theta;
    double yrot = x[i] * sin_theta + y[i] * cos_theta;
    x[i] = xrot;
    y[i] = yrot;
  }
}


/* NAME
 *   xlat2d                     Translates an array of 2 dimensional vectors
 *
 * SYNOPSIS
 *   xlat2d (x, y, n, xoffset, yoffset)
 *   double x[], y[]            Array of points
 *   int n                      Number of points in array
 *   double xoffset, yoffset    Offset to translate points by
 */

void
xlat2d (double x[], double y[], int n, double xoffset, double yoffset)
{
  for (int i = 0; i < n; i++) {
    x[i] += xoffset;
    y[i] += yoffset;
  }
}


/* NAME
 *   scale2d                    Scale an array of 2 dimensional vectors
 *
 * SYNOPSIS
 *   scale2d (x, y, n, xoffset, yoffset)
 *   double x[], y[]            Array of points
 *   int n                      Number of points in array
 *   double xfact, yfact        x & y scaling factors
 */

void
scale2d (double x[], double y[], int n, double xfact, double yfact)
{
  for (int i = 0; i < n; i++) {
    x[i] *= xfact;
    y[i] *= yfact;
  }
}


void
indent_mtx2 (GRFMTX_2D m)
{
  m[0][0] = 1.0;  m[0][1] = 0.0;  m[0][2] = 0.0;
  m[1][0] = 0.0;  m[1][1] = 1.0;  m[1][2] = 0.0;
  m[2][0] = 0.0;  m[2][1] = 0.0;  m[2][2] = 1.0;
}

void
xlat_mtx2 (GRFMTX_2D m, const double x, const double y)
{
  indent_mtx2 (m);
  m[2][0] = x;
  m[2][1] = y;
}

void
scale_mtx2 (GRFMTX_2D m, const double sx, const double sy)
{
  indent_mtx2 (m);
  m[0][0] = sx;
  m[1][1] = sy;
}

void
rot_mtx2 (GRFMTX_2D m, const double theta)
{
  double c = cos(theta);
  double s = sin(theta);

  indent_mtx2 (m);
  m[0][0] =  c;  m[0][1] = s;
  m[1][0] = -s;  m[1][1] = c;
}

void
mult_mtx2 (const GRFMTX_2D m1, const GRFMTX_2D m2, GRFMTX_2D result)
{
  GRFMTX_2D temp;

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      temp[row][col] = 0;
      for (int calc = 0; calc < 3; calc++)
            temp[row][col] += m1[row][calc] * m2[calc][col];
    }
  }

  for (int r = 0; r < 3; r++)
    for (int col = 0; col < 3; col++)
      result[r][col] = temp[r][col];
}

void
xform_mtx2 (const GRFMTX_2D m, double& x, double& y)
{
  double xt = x * m[0][0] + y * m[1][0] + m[2][0];
  double yt = x * m[0][1] + y * m[1][1] + m[2][1];

  x = xt;
  y = yt;
}
