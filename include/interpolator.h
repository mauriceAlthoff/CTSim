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


class CubicSplineInterpolator {
private:
  double *m_pdY2;  // second differential of y data

  const double* const m_pdY;
  const int m_n;

public:
  CubicSplineInterpolator (const double* const y, int n);

  ~CubicSplineInterpolator ();

  double interpolate (double x);
};


class CubicPolyInterpolator {
private:
  const double* const m_pdY;
  const int m_n;

public:
  CubicPolyInterpolator (const double* const y, int n);

  ~CubicPolyInterpolator ();

  double interpolate (double x);
};


template<class T>
class BilinearInterpolator {
private:
  T** const m_ppMatrix;
  const int m_nx;
  const int m_ny;

public:
  BilinearInterpolator (T** ppMatrix, unsigned int nx, unsigned int ny)
  : m_ppMatrix(ppMatrix), m_nx(nx), m_ny(ny)
  {}

  T interpolate (double dXPos, double dYPos)
{
  int iFloorX = static_cast<int>(floor(dXPos));
  int iFloorY = static_cast<int>(floor (dYPos));
  double dXFrac = dXPos - iFloorX;
  double dYFrac = dYPos - iFloorY;

  T result = 0;

  if (iFloorX < 0 || iFloorY < 0 || iFloorX > m_nx-1 || iFloorY > m_ny-1)
    result = 0;
  else if (iFloorX == m_nx - 1 && iFloorY == m_ny - 1)
      result = static_cast<T>(m_ppMatrix[m_nx-1][m_ny-1]);
  else if (iFloorX == m_nx - 1)
    result = static_cast<T>(m_ppMatrix[iFloorX][iFloorY] + dYFrac * (m_ppMatrix[iFloorX][iFloorY+1] - m_ppMatrix[iFloorX][iFloorY]));
    else if (iFloorY == m_ny - 1)
      result = static_cast<T>(m_ppMatrix[iFloorX][iFloorY] + dXFrac * (m_ppMatrix[iFloorX+1][iFloorY] - m_ppMatrix[iFloorX][iFloorY]));
  else
    result = static_cast<T>
      ((1 - dXFrac) * (1 - dYFrac) * m_ppMatrix[iFloorX][iFloorY] +
       dXFrac * (1 - dYFrac) * m_ppMatrix[iFloorX+1][iFloorY] +
       dYFrac * (1 - dXFrac) * m_ppMatrix[iFloorX][iFloorY+1] +
       dXFrac * dYFrac * m_ppMatrix[iFloorX+1][iFloorY+1]);

  return result;
}
    };


template<class T>
class BilinearPolarInterpolator {
private:
  T** const m_ppMatrix;
  const int m_nAngle;
  const int m_nPos;
  int m_nCenterPos;

public:
  BilinearPolarInterpolator (T** ppMatrix, unsigned int nAngle,
                             unsigned int nPos)
  : m_ppMatrix(ppMatrix), m_nAngle(nAngle), m_nPos(nPos)
  {
    if (m_nPos %2)
      m_nCenterPos = (m_nPos - 1) / 2;
    else
      m_nCenterPos = m_nPos / 2;
  }

  T interpolate (double dAngle, double dPos)
{
  int iFloorAngle = static_cast<int>(floor(dAngle));
  int iFloorPos = static_cast<int>(floor (dPos));
  double dAngleFrac = dAngle - iFloorAngle;
  double dPosFrac = dPos - iFloorPos;

  T result = 0;

  if (iFloorAngle < -1 || iFloorPos < 0 || iFloorAngle > m_nAngle-1 || iFloorPos > m_nPos-1)
    result = 0;
  else if (iFloorAngle == -1 && iFloorPos == m_nPos-1)
    result = static_cast<T>(m_ppMatrix[0][m_nPos-1] + dAngleFrac * (m_ppMatrix[m_nAngle-1][iFloorPos] - m_ppMatrix[0][iFloorPos]));
  else if (iFloorAngle == m_nAngle - 1 && iFloorPos == m_nPos-1)
    result = static_cast<T>(m_ppMatrix[m_nAngle-1][m_nPos-1] + dAngleFrac * (m_ppMatrix[0][iFloorPos] - m_ppMatrix[m_nAngle-1][iFloorPos]));
  else if (iFloorPos == m_nPos - 1)
    result = static_cast<T>(m_ppMatrix[iFloorAngle][iFloorPos] + dAngleFrac * (m_ppMatrix[iFloorAngle+1][iFloorPos] - m_ppMatrix[iFloorAngle][iFloorPos]));
  else {
    if (iFloorAngle == m_nAngle-1) {
      int iUpperAngle = 0;
      int iLowerPos = (m_nPos-1) - iFloorPos;
      int iUpperPos = (m_nPos-1) - (iFloorPos+1);
      result = static_cast<T>
        ((1-dAngleFrac) * (1-dPosFrac) * m_ppMatrix[iFloorAngle][iFloorPos] +
         dAngleFrac * (1-dPosFrac) * m_ppMatrix[iUpperAngle][iLowerPos] +
         dPosFrac * (1-dAngleFrac) * m_ppMatrix[iFloorAngle][iFloorPos+1] +
         dAngleFrac * dPosFrac * m_ppMatrix[iUpperAngle][iUpperPos]);
    } else if (iFloorAngle == -1) {
      int iLowerAngle = m_nAngle - 1;
      int iLowerPos = (m_nPos-1) - iFloorPos;
      int iUpperPos = (m_nPos-1) - (iFloorPos+1);
      result = static_cast<T>
        ((1-dAngleFrac) * (1-dPosFrac) * m_ppMatrix[iLowerAngle][iLowerPos] +
         dAngleFrac * (1-dPosFrac) * m_ppMatrix[iFloorAngle+1][iFloorPos] +
         dPosFrac * (1-dAngleFrac) * m_ppMatrix[iLowerAngle][iUpperPos] +
         dAngleFrac * dPosFrac * m_ppMatrix[iFloorAngle+1][iFloorPos+1]);
    } else
      result = static_cast<T>
        ((1-dAngleFrac) * (1-dPosFrac) * m_ppMatrix[iFloorAngle][iFloorPos] +
         dAngleFrac * (1-dPosFrac) * m_ppMatrix[iFloorAngle+1][iFloorPos] +
         dPosFrac * (1-dAngleFrac) * m_ppMatrix[iFloorAngle][iFloorPos+1] +
         dAngleFrac * dPosFrac * m_ppMatrix[iFloorAngle+1][iFloorPos+1]);
  }
  return result;
}
};


template<class T>
class BicubicPolyInterpolator {
private:
  T** const m_ppMatrix;
  const unsigned int m_nx;
  const unsigned int m_ny;

public:
  BicubicPolyInterpolator (T** ppMatrix, unsigned int nx, unsigned int ny)
  : m_ppMatrix(ppMatrix), m_nx(nx), m_ny(ny)
  {}

  T interpolate (double dXPos, double dYPos)
  {
    // int iFloorX = static_cast<int>(floor (dXPos));
    // int iFloorY = static_cast<int>(floor (dYPos));
    // double dXFrac = dXPos - iFloorX;
    // double dYFrac = dYPos - iFloorY;

    T result = 0;

    // Need to add code

    return result;
  }
};


template<class T>
class LinearInterpolator {
private:
  T* const m_pX;
  T* const m_pY;
  const int m_n;
  const bool m_bZeroOutsideRange;

public:
  LinearInterpolator (T* pY, unsigned int n, bool bZeroOutside = true)
  : m_pX(0), m_pY(pY), m_n(n), m_bZeroOutsideRange(bZeroOutside)
  {}

  LinearInterpolator (T* pX, T* pY, unsigned int n, bool bZeroOutside = true)
  : m_pX(pX), m_pY(pY), m_n(n), m_bZeroOutsideRange(bZeroOutside)
  {}

  double interpolate (double dX, int* piLastFloor = NULL)
  {
    double result = 0;

    if (! m_pX) {
      if (dX == 0)
        result = m_pY[0];
      else if (dX < 0) {
        if (m_bZeroOutsideRange)
          result = 0;
        else
          result = m_pY[0];
      } else if (dX == m_n - 1)
        result = m_pY[m_n-1];
      else if (dX > m_n - 1) {
        if (m_bZeroOutsideRange)
          result = 0;
        else
          result = m_pY[m_n - 1];
      } else {
       int iFloor = static_cast<int>(floor(dX));
       result = m_pY[iFloor] + (m_pY[iFloor+1] - m_pY[iFloor]) * (dX - iFloor);
      }
    } else {
      int iLower = -1;
      int iUpper = m_n;
      if (piLastFloor && *piLastFloor >= 0 && m_pX[*piLastFloor] < dX)
        iLower = *piLastFloor;

      while (iUpper - iLower > 1) {
        int iMiddle = (iUpper + iLower) >> 1;
        if (dX >= m_pX[iMiddle])
          iLower = iMiddle;
        else
          iUpper = iMiddle;
      }
      if (dX == m_pX[0])
        result = m_pY[0];
      else if (dX < m_pX[0]) {
        if (m_bZeroOutsideRange)
          result = 0;
        else
          result = m_pY[0];
      } else if (dX == m_pX[m_n-1])
        result = m_pY[m_n-1];
      else if (dX > m_pX[m_n - 1]) {
        if (m_bZeroOutsideRange)
          result = 0;
        else
          result = m_pY[m_n - 1];
      } else {
        if (iLower < 0 || iLower >= m_n) {
          sys_error (ERR_SEVERE, "Coordinate out of range [linearInterpolate]");
          return 0;
        }

       if (piLastFloor)
         *piLastFloor = iLower;
       result = m_pY[iLower] + (m_pY[iUpper] - m_pY[iLower]) * ((dX - m_pX[iLower]) / (m_pX[iUpper] - m_pX[iLower]));
      }
    }

    return result;
  }
};

