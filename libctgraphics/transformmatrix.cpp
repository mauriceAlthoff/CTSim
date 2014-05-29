/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:       transformmatrix.cpp
**      Function:   Transform Matrix routine for graphic library
**      Programmer: Kevin Rosenberg
**      Date Started:   1-22-85
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

#include <iostream>
#include "ctsupport.h"
#include "transformmatrix.h"


TransformationMatrix2D::TransformationMatrix2D (double m[3][3])
{
  mtx[0][0] = m[0][0]; mtx[0][1] = m[0][1]; mtx[0][2] = m[0][2];
  mtx[1][0] = m[1][0]; mtx[1][1] = m[1][1]; mtx[1][2] = m[1][2];
  mtx[2][0] = m[2][0]; mtx[2][1] = m[2][1]; mtx[2][2] = m[2][2];
}

void
TransformationMatrix2D::setTranslate (double x, double y)
{
  setIdentity();
  mtx[2][0] = x;
  mtx[2][1] = y;
}


void
TransformationMatrix2D::setScale (double sx, double sy)
{
  setIdentity();
  mtx[0][0] = sx;
  mtx[1][1] = sy;
}


void
TransformationMatrix2D::setShear (double shrx, double shry)
{
  setIdentity();
  mtx[1][0] = shrx;
  mtx[0][1] = shry;
}

void
TransformationMatrix2D::setRotate (double theta)
{
    double s = sin (theta);
    double c = cos (theta);

    setIdentity();

    mtx[0][0] =  c;  mtx[0][1] = s;
    mtx[1][0] = -s;  mtx[1][1] = c;
}


void
TransformationMatrix2D::setIdentity ()
{
    mtx[0][0] = 1.;  mtx[0][1] = 0.;  mtx[0][2] = 0.;
    mtx[1][0] = 0.;  mtx[1][1] = 1.;  mtx[1][2] = 0.;
    mtx[2][0] = 0.;  mtx[2][1] = 0.;  mtx[2][2] = 1.;
}


const TransformationMatrix2D
TransformationMatrix2D::invert () const
{
  double b[3][3];

  double determ = determinant ();
  if (fabs(determ) < 1E-6) {
    sys_error (ERR_WARNING, "Determinant = %g [TransformationMatrix2D::invert]", determ);
        print (std::cout);
        std::cout << std::endl;
  }

  b[0][0] =  (mtx[1][1] * mtx[2][2] - mtx[2][1] * mtx[1][2]) / determ;
  b[1][0] = -(mtx[1][0] * mtx[2][2] - mtx[2][0] * mtx[1][2]) / determ;
  b[2][0] =  (mtx[1][0] * mtx[2][1] - mtx[2][0] * mtx[1][1]) / determ;

  b[0][1] = -(mtx[0][1] * mtx[2][2] - mtx[2][1] * mtx[0][2]) / determ;
  b[1][1] =  (mtx[0][0] * mtx[2][2] - mtx[2][0] * mtx[0][2]) / determ;
  b[2][1] = -(mtx[0][0] * mtx[2][1] - mtx[2][0] * mtx[0][1]) / determ;

  b[0][2] =  (mtx[0][1] * mtx[1][2] - mtx[1][1] * mtx[0][2]) / determ;
  b[1][2] = -(mtx[0][0] * mtx[1][2] - mtx[1][0] * mtx[0][2]) / determ;
  b[2][2] =  (mtx[0][0] * mtx[1][1] - mtx[1][0] * mtx[0][1]) / determ;

  return TransformationMatrix2D (b);
}


double
TransformationMatrix2D::determinant () const
{
  return
    (mtx[0][0] * mtx[1][1] * mtx[2][2] - mtx[0][0] * mtx[2][1] * mtx[1][2] -
     mtx[0][1] * mtx[1][0] * mtx[2][2] + mtx[0][1] * mtx[2][0] * mtx[1][2] +
     mtx[0][2] * mtx[1][0] * mtx[2][1] - mtx[0][2] * mtx[2][0] * mtx[1][1]);
}


void
TransformationMatrix2D::transformPoint (double* pX, double *pY) const
{
  double x = *pX * mtx[0][0] + *pY * mtx[1][0] + mtx[2][0];
  double y = *pX * mtx[0][1] + *pY * mtx[1][1] + mtx[2][1];

  *pX = x;
  *pY = y;
}

void
TransformationMatrix2D::print (std::ostream& ostr) const
{
        std::cout << mtx[0][0] << " " << mtx[0][1] << " " << mtx[0][2] << std::endl;
        std::cout << mtx[1][0] << " " << mtx[1][1] << " " << mtx[1][2] << std::endl;
        std::cout << mtx[2][0] << " " << mtx[2][1] << " " << mtx[2][2] << std::endl;
}


// Friend of TransformMatrix2D

const TransformationMatrix2D operator* (const TransformationMatrix2D& a, const TransformationMatrix2D& b)
{
    double c[3][3];

    for (int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++) {
        c[row][col] = 0.;
        for (int calc = 0; calc < 3; calc++)
          c[row][col] += a.mtx[row][calc] * b.mtx[calc][col];
      }

    return TransformationMatrix2D (c);
}
