/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         sgp.h
**      Purpose:      Header file for Simple Graphics Package
**      Author:       Kevin Rosenberg
**      Date Started: 1984
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

#ifndef __H_TRANSFORMMATRIX
#define __H_TRANSFORMMATRIX

#include <iostream>

class TransformationMatrix2D {
public:
  double mtx[3][3];

  TransformationMatrix2D () {};
  TransformationMatrix2D (double m[3][3]);

  void setIdentity();
  void setTranslate (double x, double y);
  void setScale (double sx, double sy);
  void setShear (double shrx, double shry);
  void setRotate (double theta);

  double determinant () const;

  const TransformationMatrix2D invert () const;

  void transformPoint (double *pX, double *pY) const;

  void print (std::ostream& ostr) const;

  friend const TransformationMatrix2D operator* (const TransformationMatrix2D& lhs, const TransformationMatrix2D& rhs);

};


const TransformationMatrix2D  operator* (const TransformationMatrix2D& lhs, const TransformationMatrix2D& rhs);

#endif
