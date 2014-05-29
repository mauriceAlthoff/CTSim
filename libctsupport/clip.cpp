/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:         clip.c
**   Purpose:      Line clipping routines
**   Programmer:   Kevin Rosenberg
**   Date Started: 1984
**
** OVERVIEW
**      Routines to clip lines against objects
**      All routines get the endpoints of the line, and
**      the SNARK size of the object (u,v)
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

#include "ctsupport.h"


/* NAME
 *    clip_segment              Clip against a segment of a circle
 *
 * SYNOPSIS
 *    clip_segment (x1, y1, x2, y2, u, v)
 *    double& x1,*y1,*x2,*y2    Endpoints of line
 *    double u,v                Dimensions of segment
 */

bool
clip_segment (double& x1, double& y1, double& x2, double& y2, const double u, const double v)
{
  double xc1 = x1 * u;
  double yc1 = y1 * v;
  double xc2 = x2 * u;
  double yc2 = y2 * v;

  if (yc1 > 0. && yc2 > 0.)     // reject lines above y-axis
    return false;

  double radius = sqrt (u * u + v * v);

  if (clip_circle (xc1, yc1, xc2, yc2, 0.0, v, radius, 0.0, 0.0) == false)
    return false;

  if (yc1 > 0. && yc2 > 0.)     // trivial reject above y-axis
    return false;

  // clip above x-axis
  if (yc1 > 0.) {
    xc1 = xc1 + (xc2-xc1)*(0.0-yc1)/(yc2-yc1);
    yc1 = 0.0;
  } else if (yc2 > 0.) {
    xc2 = xc1 + (xc2-xc1)*(0.0-yc1)/(yc2-yc1);
    yc2 = 0.0;
  }

  x1 = xc1 / u;
  y1 = yc1 / v;
  x2 = xc2 / u;
  y2 = yc2 / v;

  return true;
}


/* NAME
 *    clip_sector               Clip a line against a sector of a circle
 *
 * SYNOPSIS
 *    clip_sector (x1, y1, x2, y2, u, v)
 *    double& x1,*y1,*x2,*y2    Endpoints of line
 *    double u,v                Size of sector
 */

bool
clip_sector (double& x1, double& y1, double& x2, double& y2, const double u, const double v)
{
  double xc1 = x1 * u;
  double yc1 = y1 * v;
  double xc2 = x2 * u;
  double yc2 = y2 * v;

  double radius = sqrt (u * u + v * v);

  if (clip_circle (xc1, yc1, xc2, yc2, 0.0, v, radius, 0.0, 0.0) == false)
    return false;

  if (clip_triangle (xc1, yc1, xc2, yc2, u, v, false) == false)
    return false;

  x1 = xc1 / u;
  y1 = yc1 / v;
  x2 = xc2 / u;
  y2 = yc2 / v;

  return true;
}


/* NAME
 *    clip_circle               Clip a line against a circle
 *
 * SYNOPSIS
 *    clip_circle (x1,y1,x2,y2,cx,cy,radius,t1,t2)
 *    double& x1,*y1,*x2,*y2    Endpoints of line to be clipped
 *    double cx,cy              Center of circle
 *    double radius             Radius of circle
 *    double t1,t2              Starting & stopping angles of clipping
 */

bool
clip_circle (double& x1, double& y1, double& x2, double& y2, const double cx, const double cy, const double radius, double t1, double t2)
{
  double xc1 = x1;
  double yc1 = y1;
  double xc2 = x2;
  double yc2 = y2;
  double ccx = cx;
  double ccy = cy;

  double xtrans = -xc1;                 // move (x1,y1) to origin
  double ytrans = -yc1;

  xc1 += xtrans;
  yc1 += ytrans;
  xc2 += xtrans;
  yc2 += ytrans;
  ccx += xtrans;
  ccy += ytrans;

  double theta = -atan2 (yc2, xc2);     // rotate line to lie on x-axis
  GRFMTX_2D rotmtx;
  rot_mtx2 (rotmtx, theta);     // xc1,yc1 is at origin, no need to rot
  xform_mtx2 (rotmtx, xc2, yc2);
  xform_mtx2 (rotmtx, ccx, ccy);
  t1 += theta;                  // rotate start and stop angles
  t2 += theta;
  t1 = normalizeAngle (t1);
  t2 = normalizeAngle (t2);

  if (xc2 < -D_EPSILON || fabs(yc2) > F_EPSILON) {
    sys_error (ERR_SEVERE, "Internal error in clip_circle\n x1=%6.2f, y1=%6.2f, x2=%6.2f, y2=%6.2f, xc2=%6.2f, yc2=%6.2f, theta=%6.2f", x1, y1, x2, y2, xc2, yc2, theta);
    return false;
  }

  if (fabs(ccy) > radius)               /* check if can reject */
    return false;

  double temp = sqrt (radius * radius - ccy * ccy);
  double xcmin = ccx - temp;
  double xcmax = ccx + temp;

  if (fabs(t2 - t1) < D_EPSILON) {
    if (xc1 < xcmin)
      xc1 = xcmin;
    if (xc2 > xcmax)
      xc2 = xcmax;
  } else if (t1 < t2) {
    if (t1 < PI && t2 > PI)
      if (xc1 < xcmin)
        xc1 = xcmin;
  } else if (t1 > t2) {
    if (t1 < PI)
      if (xc1 < xcmin)
        xc1 = xcmin;
    if (xc2 > xcmax)
      xc2 = xcmax;
  }

  rot_mtx2 (rotmtx, -theta);
  xform_mtx2 (rotmtx, xc1, yc1);
  xform_mtx2 (rotmtx, xc2, yc2);
  xc1 += -xtrans;
  yc1 += -ytrans;
  xc2 += -xtrans;
  yc2 += -ytrans;

  x1 = xc1;
  y1 = yc1;
  x2 = xc2;
  y2 = yc2;

  return true;
}


/* NAME
 *    clip_triangle             Clip a line against a triangle
 *
 * SYNOPSIS
 *    clip_triangle (x1, y1, x2, y2, u, v, clip_xaxis)
 *    double& x1, *y1, *x2, *y2 Endpoints of line
 *    double u, v               Size of 1/2 base len & height
 *    int clip_xaxis            Boolean flag whether to clip against x axis
 *                              (Use true for all triangles)
 *                              (false if used internally by sector clipping routine)
 *
 * DESCRIPTION
 *              x
 *             /|\              Note that vertices of triangle are
 *            / | \                 (-u, 0)
 *           /  |  \                (u, 0)
 *          /   |   \               (0, v)
 *         /    | v  \
 *        /     |     \
 *       +------+------+
 *            (0,0)  u
 *
 * NOTES
 *   1) Inside of this routine, values of (u,v) are assumed to be (1,1)
 *
 *   2) Derivation of clipping equations:
 *      Using parametric equations for the line
 *          xv = x1 + t * (x2 - x1)
 *          yv = y1 + t * (y2 - y1)
 *      so,
 *          t  = (xv - x1) / (x2 - x1)
 *          yv = y1 + (xv - x1) * (y2 - y1) / (x2 - x1)
 *          yv = y1 + (xv - x1) * dy / dx
 *
 *      Now, find the intersections with the following clipping boundries:
 *          yv = v - (v/u) * xv         (yv = mx + b)
 *          yv = v + (v/u) * xv         (m = v/u, b = v);
 */

static int tcode (const double x, const double y, const double m, const double b, const int clip_xaxis);

bool
clip_triangle (double& x1, double& y1, double& x2, double& y2, const double u, const double v, const int clip_xaxis)
{
  double m = v / u;      // slope of triangle lines
  double b = v;          // y-intercept of triangle lines

  int c1 = tcode (x1, y1, m, b, clip_xaxis);
  int c2 = tcode (x2, y2, m, b, clip_xaxis);

#if 0
  printf ("x1:%6.2f  y1:%6.2f  code1:%2d  x2:%6.2f  y2:%6.2f code2:%2d\n", x1, y1, c1, x2, y2, c2);
#endif
  while ( c1 || c2 ) {
    if ( c1 & c2 ) {
      return false;                     // trivial reject
    }
    int c = c1;
    if (c1 == 0)
      c = c2;

    double x = 0, y = 0;
    if (c & 1) {                        // below
      x = x1 + (x2-x1)*(0.0-y1)/(y2-y1);
      y = 0.0;
    } else if (c & 2) {                 // right
      double dx, dy;
      dx = x2 - x1;
      dy = y2 - y1;
      if (fabs(dx) > D_EPSILON)
        x = (-y1 + b + x1 * dy/dx) / (m + dy/dx);
      else
        x = x1;
      y = -m * x + b;
    } else if (c & 4) {                 /* left */
      double dx, dy;
      dx = x2 - x1;
      dy = y2 - y1;
      if (fabs(dx) > D_EPSILON) {
        x = (y1 - b - x1 * dy/dx);
        x /= (m - dy/dx);
      } else
        x = x1;
      y = m * x + b;
    }

    if (c == c1) {
      x1=x; y1=y; c1=tcode (x1,y1,m,b,clip_xaxis);
    } else {
      x2=x; y2=y; c2=tcode (x2,y2,m,b,clip_xaxis);
    }
#if 0
    printf ("x1:%6.2f  y1:%6.2f  code1:%2d  x2:%6.2f  y2:%6.2f code2:%2d\n", x1, y1, c1, x2, y2, c2);
#endif
  }

  return true;          /* we have clipped the line, and it is good */
}


/* compute region code */
static int
tcode (const double x, const double y, const double m, const double b, const int clip_xaxis)
{
  int c = 0;

  if (clip_xaxis && y < 0.)     // below triange
    c = 1;

  if (y > -m * x + b + D_EPSILON)               // right of triangle
    c += 2;
  if (y > m * x + b + D_EPSILON)                // left of triangle
    c += 4;

  return (c);
}


/* NAME
 *    clip_rect                 Clip a line against a rectangle
 *
 * SYNOPSIS
 *    clip_rect (x1, y1, x2, y2, rect)
 *    double& x1, *y1, *x2, *y2 Endpoints of line
 *    double rect[4]            Rectangle to clip against
 *                              ordered xmin, ymin, xmax, ymax
 */

static int rectcode (double x, double y, const double rect[4]);

bool
clip_rect (double& x1, double& y1, double& x2, double& y2, const double rect[4])
{
  double x = 0, y = 0;

  int c1 = rectcode (x1, y1, rect);
  int c2 = rectcode (x2, y2, rect);

  while (c1 || c2) {
    if (c1 & c2)
      return false;                     // trivial reject
    int c = c1;
    if (c1 == 0)
      c = c2;
    if (c & 1) {                        // left
      y = y1 + (y2-y1)*(rect[0]-x1)/(x2-x1);
      x = rect[0];
    } else if (c & 2) {                 // right
      y = y1 + (y2-y1)*(rect[2]-x1)/(x2-x1);
      x = rect[2];
    } else if (c & 4) {                 // bottom
      x = x1 + (x2-x1)*(rect[1]-y1)/(y2-y1);
      y = rect[1];
    } else if (c & 8) {                 // top
      x = x1 + (x2-x1)*(rect[3]-y1)/(y2-y1);
      y = rect[3];
    }

    if (c == c1) {
      x1=x; y1=y; c1=rectcode(x1,y1,rect);
    } else {
      x2=x; y2=y; c2=rectcode(x2,y2,rect);
    }
  }
  return true;          // we have clipped the line, and it is good
}


/* NAME
 *   rectcode                   INTERNAL routine to return position of
 *                              point relative to a rectangle
 *
 * SYNOPSIS
 *    c = rectcode (x, y, rect)
 *    int c                     Position of point relative to the window
 *    double x, y               Point to test against window
 *    double rect[4]            Coordinates of rectangle extent
 *                              Ordered [xmin, ymin, xmax, ymax]
 */

static int
rectcode (double x, double y, const double rect[4])
{
  int c = 0;

  if (x < rect[0])
    c = 1;
  else if (x > rect[2])
    c = 2;
  if (y < rect[1])
    c += 4;
  else if (y > rect[3])
    c += 8;
  return (c);
}
