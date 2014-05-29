/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          fourier.h
**   Purpose:       Header for Fourier transform functions
**   Programmer:    Kevin Rosenberg
**   Date Started:  Dec 2000
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

#include <complex>
#ifdef HAVE_FFTW
#include <fftw3.h>
#endif

class ImageFile;

class Fourier {
public:
    static void shuffleFourierToNaturalOrder (ImageFile& im);
    static void shuffleNaturalToFourierOrder (ImageFile& im);

#ifdef HAVE_FFTW
    static void shuffleFourierToNaturalOrder (fftw_complex* pc, const int n);
    static void shuffleNaturalToFourierOrder (fftw_complex* pc, const int n);
#endif

// Odd Number of Points
//   Natural Frequency Order: -(n-1)/2...-1,0,1...(n-1)/2
//   Fourier Frequency Order: 0, 1..(n-1)/2,-(n-1)/2...-1
// Even Number of Points
//   Natural Frequency Order: -n/2...-1,0,1...((n/2)-1)
//   Fourier Frequency Order: 0,1...((n/2)-1),-n/2...-1
    template<class T>
    static void shuffleNaturalToFourierOrder (T* pVector, const int n)
    {
      T* pTemp = new T [n];
      int i;
      if (isOdd(n)) { // Odd
        int iHalfN = (n - 1) / 2;

        pTemp[0] = pVector[iHalfN];
        for (i = 0; i < iHalfN; i++)
          pTemp[i + 1] = pVector[i + 1 + iHalfN];
        for (i = 0; i < iHalfN; i++)
          pTemp[i + iHalfN + 1] = pVector[i];
      } else {     // Even
        int iHalfN = n / 2;
        pTemp[0] = pVector[iHalfN];
        for (i = 0; i < iHalfN - 1; i++)
        pTemp[i + 1] = pVector[i + iHalfN + 1];
        for (i = 0; i < iHalfN; i++)
          pTemp[i + iHalfN] = pVector[i];
      }

    for (i = 0; i < n; i++)
      pVector[i] = pTemp[i];
    delete pTemp;
  }

  template<class T>
  static void shuffleFourierToNaturalOrder (T* pVector, const int n)
  {
    T* pTemp = new T [n];
    int i;
    if (isOdd(n)) { // Odd
      int iHalfN = (n - 1) / 2;

      pTemp[iHalfN] = pVector[0];
      for (i = 0; i < iHalfN; i++)
        pTemp[i + 1 + iHalfN] = pVector[i + 1];
      for (i = 0; i < iHalfN; i++)
        pTemp[i] = pVector[i + iHalfN + 1];
    } else {     // Even
      int iHalfN = n / 2;
      pTemp[iHalfN] = pVector[0];
      for (i = 0; i < iHalfN; i++)
        pTemp[i] = pVector[i + iHalfN];
      for (i = 0; i < iHalfN - 1; i++)
        pTemp[i + iHalfN + 1] = pVector[i+1];
    }

    for (i = 0; i < n; i++)
      pVector[i] = pTemp[i];
    delete pTemp;
  }
#if 0
    static void shuffleNaturalToFourierOrder (float* pdVector, const int n);
    static void shuffleNaturalToFourierOrder (double* pdVector, const int n);
    static void shuffleNaturalToFourierOrder (std::complex<double>* pdVector, const int n);
    static void shuffleFourierToNaturalOrder (float* pdVector, const int n);
    static void shuffleFourierToNaturalOrder (double* pdVector, const int n);
    static void shuffleFourierToNaturalOrder (std::complex<double>* pdVector, const int n);
#endif
};


