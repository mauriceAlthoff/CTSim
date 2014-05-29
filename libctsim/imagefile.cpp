/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:           imagefile.cpp
**  Purpose:      Imagefile classes
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

#include "ct.h"
#ifdef HAVE_CTN_DICOM
#include "ctndicom.h"
#endif
#include "interpolator.h"

const double ImageFile::s_dRedGrayscaleFactor = 0.299;
const double ImageFile::s_dGreenGrayscaleFactor = 0.587;
const double ImageFile::s_dBlueGrayscaleFactor = 0.114;


const int ImageFile::EXPORT_FORMAT_INVALID = -1;
const int ImageFile::EXPORT_FORMAT_TEXT = 0;
const int ImageFile::EXPORT_FORMAT_PGM = 1;
const int ImageFile::EXPORT_FORMAT_PGMASCII = 2;
#ifdef HAVE_PNG
const int ImageFile::EXPORT_FORMAT_PNG = 3;
const int ImageFile::EXPORT_FORMAT_PNG16 = 4;
#endif
#ifdef HAVE_CTN_DICOM
const int ImageFile::EXPORT_FORMAT_DICOM = 5;
#endif
const int ImageFile::EXPORT_FORMAT_RAW = 6;

const char* ImageFile::s_aszExportFormatName[] =
{
  "text",
  "pgm",
  "pgmascii",
#ifdef HAVE_PNG
  "png",
  "png16",
#endif
#ifdef HAVE_CTN_DICOM
  "dicom",
#endif
};

const char* ImageFile::s_aszExportFormatTitle[] =
{
  "Text",
  "PGM",
  "PGM ASCII",
#ifdef HAVE_PNG
  "PNG",
  "PNG 16-bit",
#endif
#ifdef HAVE_CTN_DICOM
  "Dicom",
#endif
};
const int ImageFile::s_iExportFormatCount = sizeof(s_aszExportFormatName) / sizeof(const char*);


const int ImageFile::IMPORT_FORMAT_INVALID = -1;
const int ImageFile::IMPORT_FORMAT_PPM = 0;
#ifdef HAVE_PNG
const int ImageFile::IMPORT_FORMAT_PNG = 1;
#endif
#ifdef HAVE_CTN_DICOM
const int ImageFile::IMPORT_FORMAT_DICOM = 2;
#endif


const char* ImageFile::s_aszImportFormatName[] =
{
  "ppm",
#ifdef HAVE_PNG
  "png",
#endif
#ifdef HAVE_CTN_DICOM
  "dicom",
#endif
};

const char* ImageFile::s_aszImportFormatTitle[] =
{
  "PPM",
#ifdef HAVE_PNG
  "PNG",
#endif
#ifdef HAVE_CTN_DICOM
  "Dicom",
#endif
};
const int ImageFile::s_iImportFormatCount = sizeof(s_aszImportFormatName) / sizeof(const char*);



F32Image::F32Image (int nx, int ny, int dataType)
: Array2dFile (nx, ny, sizeof(kfloat32), Array2dFile::PIXEL_FLOAT32, dataType)
{
}

F32Image::F32Image (void)
: Array2dFile()
{
  setPixelFormat (Array2dFile::PIXEL_FLOAT32);
  setPixelSize (sizeof(kfloat32));
  setDataType (Array2dFile::DATA_TYPE_REAL);
}

F64Image::F64Image (int nx, int ny, int dataType)
: Array2dFile (nx, ny, sizeof(kfloat64), Array2dFile::PIXEL_FLOAT64, dataType)
{
}

F64Image::F64Image (void)
: Array2dFile ()
{
  setPixelFormat (PIXEL_FLOAT64);
  setPixelSize (sizeof(kfloat64));
  setDataType (Array2dFile::DATA_TYPE_REAL);
}

void
ImageFile::getCenterCoordinates (unsigned int& iXCenter, unsigned int& iYCenter)
{
  if (isEven (m_nx))
    iXCenter = m_nx / 2;
  else
    iXCenter = (m_nx - 1) / 2;

  if (isEven (m_ny))
    iYCenter = m_ny / 2;
  else
    iYCenter = (m_ny - 1) / 2;
}


void
ImageFile::filterResponse (const char* const domainName, double bw, const char* const filterName,
                           double filt_param, double dInputScale, double dOutputScale)
{
  ImageFileArray v = getArray();
  SignalFilter filter (filterName, domainName, bw, filt_param);

  unsigned int iXCenter, iYCenter;
  getCenterCoordinates (iXCenter, iYCenter);

  for (unsigned int ix = 0; ix < m_nx; ix++)
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      long lD2 = ((ix - iXCenter) * (ix - iXCenter)) + ((iy - iYCenter) * (iy - iYCenter));
      double r = ::sqrt (static_cast<double>(lD2)) * dInputScale;
      v[ix][iy] = filter.response (r) * dOutputScale;
    }
}


// ImageFile::comparativeStatistics    Calculate comparative stats
//
// OUTPUT
//   d   Normalized root mean squared distance measure
//   r   Normalized mean absolute distance measure
//   e   Worst case distance measure
//
// REFERENCES
//  G.T. Herman, Image Reconstruction From Projections, 1980

bool
ImageFile::comparativeStatistics (const ImageFile& imComp, double& d, double& r, double& e) const
{
  if (imComp.nx() != m_nx && imComp.ny() != m_ny) {
    sys_error (ERR_WARNING, "Image sizes differ [ImageFile::comparativeStatistics]");
    return false;
  }
  ImageFileArrayConst v = getArray();
  if (v == NULL || m_nx == 0 || m_ny == 0)
    return false;

  ImageFileArrayConst vComp = imComp.getArray();

  double myMean = 0.;
  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      myMean += v[ix][iy];
    }
  }
  myMean /= (m_nx * m_ny);

  double sqErrorSum = 0.;
  double absErrorSum = 0.;
  double sqDiffFromMeanSum = 0.;
  double absValueSum = 0.;
  for (unsigned int ix2 = 0; ix2 < m_nx; ix2++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      double diff = v[ix2][iy] - vComp[ix2][iy];
      sqErrorSum += diff * diff;
      absErrorSum += fabs(diff);
      double diffFromMean = v[ix2][iy] - myMean;
      sqDiffFromMeanSum += diffFromMean * diffFromMean;
      absValueSum += fabs(v[ix2][iy]);
    }
  }

  d = ::sqrt (sqErrorSum / sqDiffFromMeanSum);
  r = absErrorSum / absValueSum;

  int hx = m_nx / 2;
  int hy = m_ny / 2;
  double eMax = -1;
  for (int ix3 = 0; ix3 < hx; ix3++) {
    for (int iy = 0; iy < hy; iy++) {
      double avgPixel = 0.25 * (v[2*ix3][2*iy] + v[2*ix3+1][2*iy] + v[2*ix3][2*iy+1] + v[2*ix3+1][2*iy+1]);
      double avgPixelComp = 0.25 * (vComp[2*ix3][2*iy] + vComp[2*ix3+1][2*iy] + vComp[2*ix3][2*iy+1] + vComp[2*ix3+1][2*iy+1]);
      double error = fabs (avgPixel - avgPixelComp);
      if (error > eMax)
        eMax = error;
    }
  }

  e = eMax;

  return true;
}


bool
ImageFile::printComparativeStatistics (const ImageFile& imComp, std::ostream& os) const
{
  double d, r, e;

  if (comparativeStatistics (imComp, d, r, e)) {
    os << "  Normalized root mean squared distance (d): " << d << std::endl;
    os << "      Normalized mean absolute distance (r): " << r << std::endl;
    os << "Worst case distance (2x2 pixel average) (e): " << e << std::endl;
    return true;
  }
  return false;
}


void
ImageFile::printStatistics (std::ostream& os) const
{
  double min, max, mean, mode, median, stddev;

  statistics (min, max, mean, mode, median, stddev);
  if (isComplex())
    os << "Real Component Statistics" << std::endl;

  os << "   min: " << min << std::endl;
  os << "   max: " << max << std::endl;
  os << "  mean: " << mean << std::endl;
  os << "  mode: " << mode << std::endl;
  os << "median: " << median << std::endl;
  os << "stddev: " << stddev << std::endl;

  if (isComplex()) {
    statistics (getImaginaryArray(), min, max, mean, mode, median, stddev);
    os << std::endl << "Imaginary Component Statistics" << std::endl;
    os << "   min: " << min << std::endl;
    os << "   max: " << max << std::endl;
    os << "  mean: " << mean << std::endl;
    os << "  mode: " << mode << std::endl;
    os << "median: " << median << std::endl;
    os << "stddev: " << stddev << std::endl;
  }
}


void
ImageFile::statistics (double& min, double& max, double& mean, double& mode, double& median, double& stddev) const
{
  ImageFileArrayConst v = getArray();
  statistics (v, min, max, mean, mode, median, stddev);
}


void
ImageFile::statistics (ImageFileArrayConst v, double& min, double& max, double& mean, double& mode, double& median, double& stddev) const
{
  int nx = m_nx;
  int ny = m_ny;

  if (v == NULL || nx == 0 || ny == 0)
    return;

  std::vector<double> vecImage;
  int iVec = 0;
  vecImage.resize (nx * ny);
  for (int ix = 0; ix < nx; ix++) {
    for (int iy = 0; iy < ny; iy++)
      vecImage[iVec++] = v[ix][iy];
  }

  vectorNumericStatistics (vecImage, nx * ny, min, max, mean, mode, median, stddev);
}

void
ImageFile::getMinMax (double& min, double& max) const
{
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArrayConst v = getArray();

  if (v == NULL || nx == 0 || ny == 0)
    return;

  min = v[0][0];
  max = v[0][0];
  for (int ix = 0; ix < nx; ix++) {
    for (int iy = 0; iy < ny; iy++) {
      if (v[ix][iy] > max)
        max = v[ix][iy];
      if (v[ix][iy] < min)
        min = v[ix][iy];
    }
  }
}

bool
ImageFile::convertRealToComplex ()
{
  if (dataType() != Array2dFile::DATA_TYPE_REAL)
    return false;

  if (! reallocRealToComplex())
    return false;

  ImageFileArray vImag = getImaginaryArray();
  for (unsigned int ix = 0; ix < m_nx; ix++) {
    ImageFileColumn vCol = vImag[ix];
    for (unsigned int iy = 0; iy < m_ny; iy++)
      *vCol++ = 0;
  }

  return true;
}

bool
ImageFile::convertComplexToReal ()
{
  if (dataType() != Array2dFile::DATA_TYPE_COMPLEX)
    return false;

  ImageFileArray vReal = getArray();
  ImageFileArray vImag = getImaginaryArray();
  for (unsigned int ix = 0; ix < m_nx; ix++) {
    ImageFileColumn vRealCol = vReal[ix];
    ImageFileColumn vImagCol = vImag[ix];
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      CTSimComplex c (*vRealCol, *vImagCol);
      *vRealCol++ = std::abs (c);
      vImagCol++;
    }
  }

  return reallocComplexToReal();
}

bool
ImageFile::subtractImages (const ImageFile& rRHS, ImageFile& result) const
{
  if (m_nx != rRHS.nx() || m_ny != rRHS.ny() || m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::subtractImage]");
    return false;
  }

  if (isComplex() || (rRHS.isComplex() && ! result.isComplex()))
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArrayConst vRHS = rRHS.getArray();
  ImageFileArrayConst vRHSImag = rRHS.getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      vResult[ix][iy] = vLHS[ix][iy] - vRHS[ix][iy];
      if (result.isComplex()) {
        vResultImag[ix][iy] = 0;
        if (isComplex())
          vResultImag[ix][iy] += vLHSImag[ix][iy];
        if (rRHS.isComplex())
          vResultImag[ix][iy] -= vRHSImag[ix][iy];
      }
    }
  }

  return true;
}

bool
ImageFile::addImages (const ImageFile& rRHS, ImageFile& result) const
{
  if (m_nx != rRHS.nx() || m_ny != rRHS.ny() || m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::subtractImage]");
    return false;
  }

  if (isComplex() || (rRHS.isComplex() && ! result.isComplex()))
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArrayConst vRHS = rRHS.getArray();
  ImageFileArrayConst vRHSImag = rRHS.getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      vResult[ix][iy] = vLHS[ix][iy] + vRHS[ix][iy];
      if (result.isComplex()) {
        vResultImag[ix][iy] = 0;
        if (isComplex())
          vResultImag[ix][iy] += vLHSImag[ix][iy];
        if (rRHS.isComplex())
          vResultImag[ix][iy] += vRHSImag[ix][iy];
      }
    }
  }

  return true;
}

bool
ImageFile::multiplyImages (const ImageFile& rRHS, ImageFile& result) const
{
  if (m_nx != rRHS.nx() || m_ny != rRHS.ny() || m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::subtractImage]");
    return false;
  }

  if (isComplex() || (rRHS.isComplex() && ! result.isComplex()))
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArrayConst vRHS = rRHS.getArray();
  ImageFileArrayConst vRHSImag = rRHS.getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (result.isComplex()) {
        double dImag = 0;
        if (isComplex())
          dImag = vLHSImag[ix][iy];
        std::complex<double> cLHS (vLHS[ix][iy], dImag);
        dImag = 0;
        if (rRHS.isComplex())
          dImag = vRHSImag[ix][iy];
        std::complex<double> cRHS (vRHS[ix][iy], dImag);
        std::complex<double> cResult = cLHS * cRHS;
        vResult[ix][iy] = cResult.real();
        vResultImag[ix][iy] = cResult.imag();
      } else
        vResult[ix][iy] = vLHS[ix][iy] * vRHS[ix][iy];
    }
  }


  return true;
}

bool
ImageFile::divideImages (const ImageFile& rRHS, ImageFile& result) const
{
  if (m_nx != rRHS.nx() || m_ny != rRHS.ny() || m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::subtractImage]");
    return false;
  }

  if (isComplex() || (rRHS.isComplex() && ! result.isComplex()))
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArrayConst vRHS = rRHS.getArray();
  ImageFileArrayConst vRHSImag = rRHS.getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (result.isComplex()) {
        double dImag = 0;
        if (isComplex())
          dImag = vLHSImag[ix][iy];
        std::complex<double> cLHS (vLHS[ix][iy], dImag);
        dImag = 0;
        if (rRHS.isComplex())
          dImag = vRHSImag[ix][iy];
        std::complex<double> cRHS (vRHS[ix][iy], dImag);
        std::complex<double> cResult = cLHS / cRHS;
        vResult[ix][iy] = cResult.real();
        vResultImag[ix][iy] = cResult.imag();
      } else {
        if (vRHS != 0)
          vResult[ix][iy] = vLHS[ix][iy] / vRHS[ix][iy];
        else
          vResult[ix][iy] = 0;
      }
    }
  }

  return true;
}


bool
ImageFile::invertPixelValues (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (isComplex() && ! result.isComplex())
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArray vResult = result.getArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    ImageFileColumnConst in = vLHS[ix];
    ImageFileColumn out = vResult[ix];
    for (unsigned int iy = 0; iy < m_ny; iy++)
      *out++ = - *in++;
  }

  return true;
}

bool
ImageFile::sqrt (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (isComplex() && ! result.isComplex())
    result.convertRealToComplex();

  bool bComplexOutput = result.isComplex();
  ImageFileArrayConst vLHS = getArray();
  if (! bComplexOutput)   // check if should convert to complex output
    for (unsigned int ix = 0; ix < m_nx; ix++)
      for (unsigned int iy = 0; iy < m_ny; iy++)
        if (! bComplexOutput && vLHS[ix][iy] < 0) {
          result.convertRealToComplex();
          bComplexOutput = true;
          break;
        }

        ImageFileArrayConst vLHSImag = getImaginaryArray();
        ImageFileArray vResult = result.getArray();
        ImageFileArray vResultImag = result.getImaginaryArray();

        for (unsigned int ix = 0; ix < m_nx; ix++) {
          for (unsigned int iy = 0; iy < m_ny; iy++) {
            if (result.isComplex()) {
              double dImag = 0;
              if (isComplex())
                dImag = vLHSImag[ix][iy];
              std::complex<double> cLHS (vLHS[ix][iy], dImag);
              std::complex<double> cResult = std::sqrt(cLHS);
              vResult[ix][iy] = cResult.real();
              vResultImag[ix][iy] = cResult.imag();
            } else
              vResult[ix][iy] = ::sqrt (vLHS[ix][iy]);
          }
        }


        return true;
}

bool
ImageFile::log (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::log]");
    return false;
  }

  if (isComplex() && ! result.isComplex())
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (result.isComplex()) {
        double dImag = 0;
        if (isComplex())
          dImag = vLHSImag[ix][iy];
        std::complex<double> cLHS (vLHS[ix][iy], dImag);
        std::complex<double> cResult = std::log (cLHS);
        vResult[ix][iy] = cResult.real();
        vResultImag[ix][iy] = cResult.imag();
      } else {
        if (vLHS[ix][iy] > 0)
          vResult[ix][iy] = ::log (vLHS[ix][iy]);
        else
          vResult[ix][iy] = 0;
      }
    }
  }


  return true;
}

bool
ImageFile::exp (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (isComplex() && ! result.isComplex())
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (result.isComplex()) {
        double dImag = 0;
        if (isComplex())
          dImag = vLHSImag[ix][iy];
        std::complex<double> cLHS (vLHS[ix][iy], dImag);
        std::complex<double> cResult = std::exp (cLHS);
        vResult[ix][iy] = cResult.real();
        vResultImag[ix][iy] = cResult.imag();
      } else
        vResult[ix][iy] = ::exp (vLHS[ix][iy]);
    }
  }


  return true;
}

bool
ImageFile::scaleImage (ImageFile& result) const
{
  unsigned int nx = m_nx;
  unsigned int ny = m_ny;
  unsigned int newNX = result.nx();
  unsigned int newNY = result.ny();

  double dXScale = static_cast<double>(newNX) / static_cast<double>(nx);
  double dYScale = static_cast<double>(newNY) / static_cast<double>(ny);

  if (isComplex() && ! result.isComplex())
    result.convertRealToComplex();

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  BilinearInterpolator<ImageFileValue> realInterp (vReal, nx, ny);
  BilinearInterpolator<ImageFileValue> imagInterp (vImag, nx, ny);

  for (unsigned int ix = 0; ix < newNX; ix++) {
    for (unsigned int iy = 0; iy < newNY; iy++) {
      double dXPos = ix / dXScale;
      double dYPos = iy / dYScale;
      vResult[ix][iy] = realInterp.interpolate (dXPos, dYPos);
      if (result.isComplex()) {
        if (isComplex()) {
          vResultImag[ix][iy] = imagInterp.interpolate (dXPos, dYPos);
        } else {
          vResultImag[ix][iy] = 0;
        }
      }
    }
  }

  return true;
}

#ifdef HAVE_FFTW
bool
ImageFile::fft (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  fftw_complex* in = static_cast<fftw_complex*> (fftw_malloc (sizeof(fftw_complex) * m_nx * m_ny));

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();

  unsigned int ix, iy;
  unsigned int iArray = 0;
  for (ix = 0; ix < m_nx; ix++) {
    for (iy = 0; iy < m_ny; iy++) {
      in[iArray][0] = vReal[ix][iy];
      if (isComplex())
        in[iArray][1] = vImag[ix][iy];
      else
        in[iArray][1] = 0;
      iArray++;
    }
  }

  fftw_plan plan = fftw_plan_dft_2d (m_nx, m_ny, in, in, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute (plan);

  ImageFileArray vRealResult = result.getArray();
  ImageFileArray vImagResult = result.getImaginaryArray();
  iArray = 0;
  unsigned int iScale = m_nx * m_ny;
  for (ix = 0; ix < m_nx; ix++) {
    for (iy = 0; iy < m_ny; iy++) {
      vRealResult[ix][iy] = in[iArray][0] / iScale;
      vImagResult[ix][iy] = in[iArray][1] / iScale;
      iArray++;
    }
  }
  fftw_free(in);
  fftw_destroy_plan (plan);

  Fourier::shuffleFourierToNaturalOrder (result);

  return true;
}


bool
ImageFile::ifft (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();
  ImageFileArray vRealResult = result.getArray();
  ImageFileArray vImagResult = result.getImaginaryArray();
  unsigned int ix, iy;
  for (ix = 0; ix < m_nx; ix++) {
    for (iy = 0; iy < m_ny; iy++) {
      vRealResult[ix][iy] = vReal[ix][iy];
      if (isComplex())
        vImagResult[ix][iy] = vImag[ix][iy];
      else
        vImagResult[ix][iy] = 0;
    }
  }

  Fourier::shuffleNaturalToFourierOrder (result);

  fftw_complex* in = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_nx * m_ny));

  unsigned int iArray = 0;
  for (ix = 0; ix < m_nx; ix++) {
    for (iy = 0; iy < m_ny; iy++) {
      in[iArray][0] = vRealResult[ix][iy];
      in[iArray][1] = vImagResult[ix][iy];
      iArray++;
    }
  }

  fftw_plan plan = fftw_plan_dft_2d (m_nx, m_ny, in, in, FFTW_BACKWARD, FFTW_ESTIMATE);

  fftw_execute (plan);

  iArray = 0;
  for (ix = 0; ix < m_nx; ix++) {
    for (iy = 0; iy < m_ny; iy++) {
      vRealResult[ix][iy] = in[iArray][0];
      vImagResult[ix][iy] = in[iArray][1];
      iArray++;
    }
  }
  fftw_destroy_plan (plan);
  fftw_free(in);

  return true;
}

bool
ImageFile::fftRows (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::fftRows]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();

  fftw_complex* in = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_nx));
  fftw_plan plan = fftw_plan_dft_1d (m_nx, in, in, FFTW_FORWARD, FFTW_ESTIMATE);

  std::complex<double>* pcRow = new std::complex<double> [m_nx];
  for (unsigned int iy = 0; iy < m_ny; iy++) {
    unsigned int ix;
    for (ix = 0; ix < m_nx; ix++) {
      in[ix][0] = vReal[ix][iy];
      if (isComplex())
        in[ix][1] = vImag[ix][iy];
      else
        in[ix][1] = 0;
    }

    fftw_execute (plan);

    for (ix = 0; ix < m_nx; ix++)
      pcRow[ix] = std::complex<double>(in[ix][0], in[ix][1]);

    Fourier::shuffleFourierToNaturalOrder (pcRow, m_nx);
    for (ix = 0; ix < m_nx; ix++) {
      vReal[ix][iy] = pcRow[ix].real() / m_nx;
      vImag[ix][iy] = pcRow[ix].imag() / m_nx;
    }
  }
  delete [] pcRow;

  fftw_destroy_plan (plan);
  fftw_free(in);

  return true;
}

bool
ImageFile::ifftRows (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::fftRows]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();

  fftw_complex* in = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_nx));
  fftw_plan plan = fftw_plan_dft_1d (m_nx, in, in, FFTW_BACKWARD, FFTW_ESTIMATE);
  std::complex<double>* pcRow = new std::complex<double> [m_nx];

  unsigned int ix, iy;
  // unsigned int iArray = 0;
  for (iy = 0; iy < m_ny; iy++) {
    for (ix = 0; ix < m_nx; ix++) {
      double dImag = 0;
      if (isComplex())
        dImag = vImag[ix][iy];
      pcRow[ix] = std::complex<double> (vReal[ix][iy], dImag);
    }

    Fourier::shuffleNaturalToFourierOrder (pcRow, m_nx);

    for (ix = 0; ix < m_nx; ix++) {
      in[ix][0] = pcRow[ix].real();
      in[ix][1] = pcRow[ix].imag();
    }

    fftw_execute (plan);

    for (ix = 0; ix < m_nx; ix++) {
      vReal[ix][iy] = in[ix][0];
      vImag[ix][iy] = in[ix][1];
    }
  }
  delete [] pcRow;

  fftw_destroy_plan (plan);
  fftw_free(in);

  return true;
}

bool
ImageFile::fftCols (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::fftRows]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();

  fftw_complex* in = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_ny));
  fftw_plan plan = fftw_plan_dft_1d (m_ny, in, in, FFTW_FORWARD, FFTW_ESTIMATE);

  std::complex<double>* pcCol = new std::complex<double> [m_ny];
  for (unsigned int ix = 0; ix < m_nx; ix++) {
    unsigned int iy;
    for (iy = 0; iy < m_ny; iy++) {
      in[iy][0] = vReal[ix][iy];
      if (isComplex())
        in[iy][1] = vImag[ix][iy];
      else
        in[iy][1] = 0;
    }

    fftw_execute (plan);

    for (iy = 0; iy < m_ny; iy++)
      pcCol[iy] = std::complex<double>(in[iy][0], in[iy][1]);

    Fourier::shuffleFourierToNaturalOrder (pcCol, m_ny);
    for (iy = 0; iy < m_ny; iy++) {
      vReal[ix][iy] = pcCol[iy].real() / m_ny;
      vImag[ix][iy] = pcCol[iy].imag() / m_ny;
    }
  }
  delete [] pcCol;

  fftw_destroy_plan (plan);
  fftw_free(in);

  return true;
}

bool
ImageFile::ifftCols (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::fftRows]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  ImageFileArrayConst vReal = getArray();
  ImageFileArrayConst vImag = getImaginaryArray();

  fftw_complex* in = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_ny));
  fftw_plan plan = fftw_plan_dft_1d (m_ny, in, in, FFTW_BACKWARD, FFTW_ESTIMATE);
  std::complex<double>* pcCol = new std::complex<double> [m_ny];

  unsigned int ix, iy;
  // unsigned int iArray = 0;
  for (ix = 0; ix < m_nx; ix++) {
    for (iy = 0; iy < m_ny; iy++) {
      double dImag = 0;
      if (isComplex())
        dImag = vImag[ix][iy];
      pcCol[iy] = std::complex<double> (vReal[ix][iy], dImag);
    }

    Fourier::shuffleNaturalToFourierOrder (pcCol, m_ny);

    for (iy = 0; iy < m_ny; iy++) {
      in[iy][0] = pcCol[iy].real();
      in[iy][1] = pcCol[iy].imag();
    }

    fftw_execute (plan);

    for (iy = 0; iy < m_ny; iy++) {
      vReal[ix][iy] = in[iy][0];
      vImag[ix][iy] = in[iy][1];
    }
  }
  delete [] pcCol;

  fftw_destroy_plan (plan);
  fftw_free(in);

  return true;
}

#endif // HAVE_FFTW



bool
ImageFile::fourier (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (! result.isComplex())
    if (! result.convertRealToComplex ())
      return false;

    ImageFileArrayConst vLHS = getArray();
    ImageFileArrayConst vLHSImag = getImaginaryArray();
    ImageFileArray vRealResult = result.getArray();
    ImageFileArray vImagResult = result.getImaginaryArray();

    unsigned int ix, iy;

    // alloc output matrix
    CTSimComplex** complexOut = new CTSimComplex* [m_nx];
    for (ix = 0; ix < m_nx; ix++)
      complexOut[ix] = new CTSimComplex [m_ny];

    // fourier each x column
    CTSimComplex* pY = new CTSimComplex [m_ny];
    for (ix = 0; ix < m_nx; ix++) {
      for (iy = 0; iy < m_ny; iy++) {
        double dImag = 0;
        if (isComplex())
          dImag = vLHSImag[ix][iy];
        pY[iy] = std::complex<double>(vLHS[ix][iy], dImag);
      }
      ProcessSignal::finiteFourierTransform (pY, complexOut[ix], m_ny,  ProcessSignal::FORWARD);
    }
    delete [] pY;

    // fourier each y row
    CTSimComplex* pX = new CTSimComplex [m_nx];
    CTSimComplex* complexOutRow = new CTSimComplex [m_nx];
    for (iy = 0; iy < m_ny; iy++) {
      for (ix = 0; ix < m_nx; ix++)
        pX[ix] = complexOut[ix][iy];
      ProcessSignal::finiteFourierTransform (pX, complexOutRow, m_nx, ProcessSignal::FORWARD);
      for (ix = 0; ix < m_nx; ix++)
        complexOut[ix][iy] = complexOutRow[ix];
    }
    delete [] pX;
    delete [] complexOutRow;

    for (ix = 0; ix < m_nx; ix++)
      for (iy = 0; iy < m_ny; iy++) {
        vRealResult[ix][iy] = complexOut[ix][iy].real();
        vImagResult[ix][iy] = complexOut[ix][iy].imag();
      }

      Fourier::shuffleFourierToNaturalOrder (result);

      // delete complexOut matrix
      for (ix = 0; ix < m_nx; ix++)
        delete [] complexOut[ix];
      delete [] complexOut;

      return true;
}

bool
ImageFile::inverseFourier (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (result.dataType() == Array2dFile::DATA_TYPE_REAL) {
    if (! result.convertRealToComplex ())
      return false;
  }

  ImageFileArrayConst vLHSReal = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArray vRealResult = result.getArray();
  ImageFileArray vImagResult = result.getImaginaryArray();

  unsigned int ix, iy;
  // alloc 2d complex output matrix
  CTSimComplex** complexOut = new CTSimComplex* [m_nx];
  for (ix = 0; ix < m_nx; ix++)
    complexOut[ix] = new CTSimComplex [m_ny];

  // put input image into result
  for (ix = 0; ix < m_nx; ix++)
    for (iy = 0; iy < m_ny; iy++) {
      vRealResult[ix][iy] = vLHSReal[ix][iy];
      if (isComplex())
        vImagResult[ix][iy] = vLHSImag[ix][iy];
      else
        vImagResult[ix][iy] = 0;
    }

    Fourier::shuffleNaturalToFourierOrder (result);

    // ifourier each x column
    CTSimComplex* pCol = new CTSimComplex [m_ny];
    for (ix = 0; ix < m_nx; ix++) {
      for (iy = 0; iy < m_ny; iy++) {
        pCol[iy] = std::complex<double> (vRealResult[ix][iy], vImagResult[ix][iy]);
      }
      ProcessSignal::finiteFourierTransform (pCol, complexOut[ix], m_ny,  ProcessSignal::BACKWARD);
    }
    delete [] pCol;

    // ifourier each y row
    CTSimComplex* complexInRow = new CTSimComplex [m_nx];
    CTSimComplex* complexOutRow = new CTSimComplex [m_nx];
    for (iy = 0; iy < m_ny; iy++) {
      for (ix = 0; ix < m_nx; ix++)
        complexInRow[ix] = complexOut[ix][iy];
      ProcessSignal::finiteFourierTransform (complexInRow, complexOutRow, m_nx, ProcessSignal::BACKWARD);
      for (ix = 0; ix < m_nx; ix++)
        complexOut[ix][iy] = complexOutRow[ix];
    }
    delete [] complexInRow;
    delete [] complexOutRow;

    for (ix = 0; ix < m_nx; ix++)
      for (iy = 0; iy < m_ny; iy++) {
        vRealResult[ix][iy] = complexOut[ix][iy].real();
        vImagResult[ix][iy] = complexOut[ix][iy].imag();
      }

      // delete complexOut matrix
      for (ix = 0; ix < m_nx; ix++)
        delete [] complexOut[ix];
      delete [] complexOut;

      return true;
}


bool
ImageFile::magnitude (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  ImageFileArray vReal = getArray();
  ImageFileArray vImag = getImaginaryArray();
  ImageFileArray vRealResult = result.getArray();

  for (unsigned int ix = 0; ix < m_nx; ix++)
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (isComplex())
        vRealResult[ix][iy] = ::sqrt (vReal[ix][iy] * vReal[ix][iy] + vImag[ix][iy] * vImag[ix][iy]);
      else
        vRealResult[ix][iy] = ::fabs(vReal[ix][iy]);
    }

    if (result.isComplex())
      result.reallocComplexToReal();

    return true;
}

bool
ImageFile::phase (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  ImageFileArray vReal = getArray();
  ImageFileArray vImag = getImaginaryArray();
  ImageFileArray vRealResult = result.getArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (isComplex())
        vRealResult[ix][iy] = ::atan2 (vImag[ix][iy], vReal[ix][iy]);
      else
        vRealResult[ix][iy] = 0;
    }
  }
  if (result.isComplex())
    result.reallocComplexToReal();

  return true;
}

bool
ImageFile::real (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  ImageFileArray vReal = getArray();
  ImageFileArray vRealResult = result.getArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
        vRealResult[ix][iy] = vReal[ix][iy];
    }
  }

  if (result.isComplex())
    result.reallocComplexToReal();

  return true;
}

bool
ImageFile::imaginary (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  ImageFileArray vImag = getArray();
  ImageFileArray vRealResult = result.getArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (isComplex())
        vRealResult[ix][iy] = vImag[ix][iy];
      else
        vRealResult[ix][iy] = 0;
    }
  }

  if (result.isComplex())
      result.reallocComplexToReal();

  return true;
}

bool
ImageFile::square (ImageFile& result) const
{
  if (m_nx != result.nx() || m_ny != result.ny()) {
    sys_error (ERR_WARNING, "Difference sizes of images [ImageFile::invertPixelValues]");
    return false;
  }

  if (isComplex() && ! result.isComplex())
    result.convertRealToComplex();

  ImageFileArrayConst vLHS = getArray();
  ImageFileArrayConst vLHSImag = getImaginaryArray();
  ImageFileArray vResult = result.getArray();
  ImageFileArray vResultImag = result.getImaginaryArray();

  for (unsigned int ix = 0; ix < m_nx; ix++) {
    for (unsigned int iy = 0; iy < m_ny; iy++) {
      if (result.isComplex()) {
        double dImag = 0;
        if (isComplex())
          dImag = vLHSImag[ix][iy];
        std::complex<double> cLHS (vLHS[ix][iy], dImag);
        std::complex<double> cResult = cLHS * cLHS;
        vResult[ix][iy] = cResult.real();
        vResultImag[ix][iy] = cResult.imag();
      } else
        vResult[ix][iy] = vLHS[ix][iy] * vLHS[ix][iy];
    }
  }

  return true;
}

int
ImageFile::convertExportFormatNameToID (const char* const formatName)
{
  int formatID = EXPORT_FORMAT_INVALID;

  for (int i = 0; i < s_iExportFormatCount; i++)
    if (strcasecmp (formatName, s_aszExportFormatName[i]) == 0) {
      formatID = i;
      break;
    }

    return (formatID);
}

const char*
ImageFile::convertExportFormatIDToName (int formatID)
{
  static const char *formatName = "";

  if (formatID >= 0 && formatID < s_iExportFormatCount)
    return (s_aszExportFormatName[formatID]);

  return (formatName);
}

const char*
ImageFile::convertExportFormatIDToTitle (const int formatID)
{
  static const char *formatTitle = "";

  if (formatID >= 0 && formatID < s_iExportFormatCount)
    return (s_aszExportFormatTitle[formatID]);

  return (formatTitle);
}

int
ImageFile::convertImportFormatNameToID (const char* const formatName)
{
  int formatID = IMPORT_FORMAT_INVALID;

  for (int i = 0; i < s_iImportFormatCount; i++)
    if (strcasecmp (formatName, s_aszImportFormatName[i]) == 0) {
      formatID = i;
      break;
    }

    return (formatID);
}

const char*
ImageFile::convertImportFormatIDToName (int formatID)
{
  static const char *formatName = "";

  if (formatID >= 0 && formatID < s_iImportFormatCount)
    return (s_aszImportFormatName[formatID]);

  return (formatName);
}

const char*
ImageFile::convertImportFormatIDToTitle (const int formatID)
{
  static const char *formatTitle = "";

  if (formatID >= 0 && formatID < s_iImportFormatCount)
    return (s_aszImportFormatTitle[formatID]);

  return (formatTitle);
}

bool
ImageFile::importImage (const char* const pszFormat, const char* const pszFilename)
{
  int iFormatID = convertImportFormatNameToID (pszFormat);

  if (iFormatID == IMPORT_FORMAT_PPM)
    return readImagePPM (pszFilename);
#ifdef HAVE_PNG
  else if (iFormatID == IMPORT_FORMAT_PNG)
    return readImagePNG (pszFilename);
#endif

  sys_error (ERR_SEVERE, "Invalid format %s [ImageFile::importImage]", pszFormat);
  return false;
}

void
ImageFile::skipSpacePPM (FILE* fp)
{
  int c = fgetc (fp);
  while (isspace (c) || c == '#') {
    if (c == '#') {   // comment until end of line
      c = fgetc(fp);
      while (c != 13 && c != 10)
        c = fgetc(fp);
    }
    else
      c = fgetc(fp);
  }

  ungetc (c, fp);
}

bool
ImageFile::readImagePPM (const char* const pszFile)
{
  FILE* fp = fopen (pszFile, "r");
  if ((fp = fopen (pszFile, "r")) == NULL)
    return false;
  char cSignature = toupper(fgetc(fp));
  if (cSignature != 'P') {
    fclose(fp);
    return false;
  }
  cSignature = fgetc(fp);
  if (cSignature == '5' || cSignature == '6') { // binary modes
    fclose(fp);
    fp = fopen(pszFile, "rb"); // reopen in binary mode
    fgetc(fp);
    fgetc(fp);
  } else if (cSignature != '2' && cSignature != '3') {
    fclose(fp);
    return false;
  }

  int nRows, nCols, iMaxValue;
  skipSpacePPM (fp);
  if (fscanf (fp, "%d", &nCols) != 1) {
    fclose(fp);
    return false;
  }
  skipSpacePPM (fp);
  if (fscanf (fp, "%d", &nRows) != 1) {
    fclose(fp);
    return false;
  }
  skipSpacePPM (fp);
  if (fscanf (fp, "%d", &iMaxValue) != 1) {
    fclose(fp);
    return false;
  }
  setArraySize (nRows, nCols);

  if (cSignature == '5' || cSignature == '6') { // binary modes
    int c = fgetc(fp);
    if (c == 13) {
      c = fgetc(fp);
      if (c != 10)  // read msdos 13-10 newline
        ungetc(c, fp);
    }
  } else
    skipSpacePPM (fp); // ascii may have comments

  bool bMonochromeImage = false;
  double dMaxValue = iMaxValue;
  double dMaxValue3 = iMaxValue * 3;

  ImageFileArray v = getArray();
  for (int iy = nRows - 1; iy >= 0; iy--) {
    for (int ix = 0; ix < nCols; ix++) {
      int iGS, iR, iG, iB;
      double dR, dG, dB;
      switch (cSignature) {
      case '2':
        if (fscanf(fp, "%d ", &iGS) != 1) {
          fclose(fp);
          return false;
        }
        v[ix][iy] = iGS / dMaxValue;
        break;
      case '5':
        iGS = fgetc(fp);
        if (iGS == EOF) {
          fclose(fp);
          return false;
        }
        v[ix][iy] = iGS / dMaxValue;
        break;
      case '3':
        if (fscanf (fp, "%d %d %d ", &iR, &iG, &iB) != 3) {
          fclose(fp);
          return false;
        }
        if (ix == 0 && iy == 0 && (iR == iG && iG == iB))
          bMonochromeImage = true;
        if (bMonochromeImage)
          v[ix][iy] = (iR + iG + iB) / dMaxValue3;
        else {
          dR = iR / dMaxValue;
          dG = iG / dMaxValue;
          dB = iB / dMaxValue;
          v[ix][iy] = colorToGrayscale (dR, dG, dB);
        }
        break;
      case '6':
        iR = fgetc(fp);
        iG = fgetc(fp);
        iB = fgetc(fp);

        if (iB == EOF) {
          fclose(fp);
          return false;
        }
        if (ix == 0 && iy == 0 && (iR == iG && iG == iB))
          bMonochromeImage = true;

        if (bMonochromeImage)
          v[ix][iy] = (iR + iG + iB) / dMaxValue3;
        else {
          dR = iR / dMaxValue;
          dG = iG / dMaxValue;
          dB = iB / dMaxValue;
          v[ix][iy] = colorToGrayscale (dR, dG, dB);
        }
        break;
      }
    }
  }

  fclose(fp);
  return true;
}

#ifdef HAVE_PNG
bool
ImageFile::readImagePNG (const char* const pszFile)
{
  FILE* fp = fopen(pszFile, "rb");
  if (!fp)
    return false;
  unsigned char header[8];
  fread (header, 1, 8, fp);
  if (png_sig_cmp (header, 0, 8)) {
    fclose (fp);
    return false;
  }

  png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fclose(fp);
    return false;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fp);
    return false;
  }

  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return false;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  int width = png_get_image_width (png_ptr, info_ptr);
  int height = png_get_image_height (png_ptr, info_ptr);
  int bit_depth = png_get_bit_depth (png_ptr, info_ptr);
  int color_type = png_get_color_type (png_ptr, info_ptr);

  if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
    png_set_expand(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);

  if (bit_depth < 8)
    png_set_packing(png_ptr);

  if (color_type & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(png_ptr);

  if (bit_depth == 16)
    png_set_swap(png_ptr); // convert to little-endian format

  png_read_update_info(png_ptr, info_ptr); // update with transformations
  int rowbytes = png_get_rowbytes (png_ptr, info_ptr);
  bit_depth = png_get_bit_depth (png_ptr, info_ptr);
  color_type = png_get_color_type (png_ptr, info_ptr);

  png_bytep* row_pointers = new png_bytep [height];
  int i;
  for (i = 0; i < height; i++)
    row_pointers[i] = new unsigned char [rowbytes];

  png_read_image(png_ptr, row_pointers);

  setArraySize (width, height);
  ImageFileArray v = getArray();
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      double dV = 0;
      if (color_type == PNG_COLOR_TYPE_GRAY) {
        if (bit_depth == 8)
          dV = row_pointers[iy][ix] / 255.;
        else if (bit_depth == 16) {
          int iBase = ix * 2;
          dV = (row_pointers[iy][iBase] + (row_pointers[iy][iBase+1] << 8)) / 65536.;
        } else
          dV = 0;
      } else if (color_type == PNG_COLOR_TYPE_RGB) {
        if (bit_depth == 8) {
          int iBase = ix * 3;
          double dR = row_pointers[iy][iBase] / 255.;
          double dG = row_pointers[iy][iBase+1] / 255.;
          double dB = row_pointers[iy][iBase+2] / 255.;
          dV = colorToGrayscale (dR, dG, dB);
        } else
          dV = 0;
      }
      v[ix][height-iy-1] = dV;
    }
  }

  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  for (i = 0; i < height; i++)
    delete row_pointers[i];
  delete row_pointers;

  fclose (fp);
  return true;
}
#endif

bool
ImageFile::exportImage (const char* const pszFormat, const char* const pszFilename, int nxcell, int nycell, double densmin, double densmax)
{
  int iFormatID = convertExportFormatNameToID (pszFormat);

  if (iFormatID == EXPORT_FORMAT_PGM)
    return writeImagePGM (pszFilename, nxcell, nycell, densmin, densmax);
  else if (iFormatID == EXPORT_FORMAT_PGMASCII)
    return writeImagePGMASCII (pszFilename, nxcell, nycell, densmin, densmax);
  else if (iFormatID == EXPORT_FORMAT_TEXT)
    return writeImageText (pszFilename);
#ifdef HAVE_PNG
  else if (iFormatID == EXPORT_FORMAT_PNG)
    return writeImagePNG (pszFilename, 8, nxcell, nycell, densmin, densmax);
  else if (iFormatID == EXPORT_FORMAT_PNG16)
    return writeImagePNG (pszFilename, 16, nxcell, nycell, densmin, densmax);
#endif
#ifdef HAVE_CTN_DICOM
  else if (iFormatID == EXPORT_FORMAT_DICOM) {
    DicomExporter dicomExport (this);
    bool bSuccess = dicomExport.writeFile (pszFilename);
    if (! bSuccess)
      sys_error (ERR_SEVERE, dicomExport.failMessage().c_str());
    return bSuccess;
  }
#endif
  else if (iFormatID == EXPORT_FORMAT_RAW)
         return writeImageRaw(pszFilename, nxcell, nycell);


  sys_error (ERR_SEVERE, "Invalid format %s [ImageFile::exportImage]", pszFormat);
  return false;
}


bool
ImageFile::writeImagePGM (const char* const outfile, int nxcell, int nycell, double densmin, double densmax)
{
  FILE *fp;
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArray v = getArray();

  unsigned char* rowp = new unsigned char [nx * nxcell];

  if ((fp = fopen (outfile, "wb")) == NULL)
    return false;

  fprintf(fp, "P5\n");
  fprintf(fp, "%d %d\n", nx, ny);
  fprintf(fp, "255\n");

  for (int irow = ny - 1; irow >= 0; irow--) {
    for (int icol = 0; icol < nx; icol++) {
      int pos = icol * nxcell;
      double dens = (v[icol][irow] - densmin) / (densmax - densmin);
      dens = clamp (dens, 0., 1.);
      for (int p = pos; p < pos + nxcell; p++) {
        rowp[p] = static_cast<unsigned int> (dens * 255.);
      }
    }
    for (int ir = 0; ir < nycell; ir++) {
      for (int ic = 0; ic < nx * nxcell; ic++)
        fputc( rowp[ic], fp );
    }
  }

  delete rowp;
  fclose(fp);

  return true;
}

bool
ImageFile::writeImagePGMASCII (const char* const outfile, int nxcell, int nycell, double densmin, double densmax)
{
  FILE *fp;
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArray v = getArray();

  unsigned char* rowp = new unsigned char [nx * nxcell];

  if ((fp = fopen (outfile, "wb")) == NULL)
    return false;

  fprintf(fp, "P2\n");
  fprintf(fp, "%d %d\n", nx, ny);
  fprintf(fp, "255\n");

  for (int irow = ny - 1; irow >= 0; irow--) {
    for (int icol = 0; icol < nx; icol++) {
      int pos = icol * nxcell;
      double dens = (v[icol][irow] - densmin) / (densmax - densmin);
      dens = clamp (dens, 0., 1.);
      for (int p = pos; p < pos + nxcell; p++) {
        rowp[p] = static_cast<unsigned int> (dens * 255.);
      }
    }
    for (int ir = 0; ir < nycell; ir++) {
      for (int ic = 0; ic < nx * nxcell; ic++)
        fprintf(fp, "%d ", rowp[ic]);
      fprintf(fp, "\n");
    }
  }

  delete rowp;
  fclose(fp);

  return true;
}

bool
ImageFile::writeImageText (const char* const outfile)
{
  FILE *fp;
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArray v = getArray();
  ImageFileArray vImag = getImaginaryArray();

  if ((fp = fopen (outfile, "w")) == NULL)
    return false;

  for (int irow = ny - 1; irow >= 0; irow--) {
    for (int icol = 0; icol < nx; icol++) {
      if (isComplex()) {
        if (vImag[icol][irow] >= 0)
          fprintf (fp, "%.9g+%.9gi ", v[icol][irow], vImag[icol][irow]);
        else
          fprintf (fp, "%.9g-%.9gi ", v[icol][irow], -vImag[icol][irow]);
      } else
        fprintf (fp, "%12.8g ", v[icol][irow]);
    }
    fprintf(fp, "\n");
  }

  fclose(fp);

  return true;
}


#ifdef HAVE_PNG
bool
ImageFile::writeImagePNG (const char* const outfile, int bitdepth, int nxcell, int nycell, double densmin, double densmax)
{
  double max_out_level = (1 << bitdepth) - 1;
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArray v = getArray();

  unsigned char* rowp = new unsigned char [nx * nxcell * (bitdepth / 8)];

  FILE *fp = fopen (outfile, "wb");
  if (! fp)
    return false;

  png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (! png_ptr)
    return false;

  png_infop info_ptr = png_create_info_struct (png_ptr);
  if (! info_ptr) {
    png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
    fclose (fp);
    return false;
  }

  if (setjmp (png_jmpbuf(png_ptr))) {
    png_destroy_write_struct (&png_ptr, &info_ptr);
    fclose (fp);
    return false;
  }

  png_init_io(png_ptr, fp);

  png_set_IHDR (png_ptr, info_ptr, nx * nxcell, ny * nycell, bitdepth, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);
  for (int irow = ny - 1; irow >= 0; irow--) {
    png_bytep row_pointer = rowp;

    for (int icol = 0; icol < nx; icol++) {
      int pos = icol * nxcell;
      double dens = (v[icol][irow] - densmin) / (densmax - densmin);
      dens = clamp (dens, 0., 1.);
      unsigned int outval = static_cast<unsigned int> (dens * max_out_level);

      for (int p = pos; p < pos + nxcell; p++) {
        if (bitdepth == 8)
          rowp[p] = outval;
        else {
          int rowpos = p * 2;
          rowp[rowpos+1] = (outval >> 8) & 0xFF;
          rowp[rowpos] = (outval & 0xFF);
        }
      }
    }
    for (int ir = 0; ir < nycell; ir++)
      png_write_rows (png_ptr, &row_pointer, 1);
  }

  png_write_end (png_ptr, info_ptr);
  png_destroy_write_struct (&png_ptr, &info_ptr);
  delete rowp;

  fclose(fp);

  return true;
}
#endif

#ifdef HAVE_GD
#include "gd.h"
static const int N_GRAYSCALE=256;

bool
ImageFile::writeImageGIF (const char* const outfile, int nxcell, int nycell, double densmin, double densmax)
{
  int gs_indices[N_GRAYSCALE];
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArray v = getArray();

  unsigned char* rowp = new unsigned char [nx * nxcell];

  gdImagePtr gif = gdImageCreate(nx * nxcell, ny * nycell);
  for (int i = 0; i < N_GRAYSCALE; i++)
    gs_indices[i] = gdImageColorAllocate(gif, i, i, i);

  int lastrow = ny * nycell - 1;
  for (int irow = 0; irow < ny; irow++) {
    int rpos = irow * nycell;
    for (int ir = rpos; ir < rpos + nycell; ir++) {
      for (int icol = 0; icol < nx; icol++) {
        int cpos = icol * nxcell;
        double dens = (v[icol][irow] - densmin) / (densmax - densmin);
        dens = clamp(dens, 0., 1.);
        for (int ic = cpos; ic < cpos + nxcell; ic++) {
          rowp[ic] = (unsigned int) (dens * (double) (N_GRAYSCALE - 1));
          gdImageSetPixel(gif, ic, lastrow - ir, gs_indices[rowp[ic]]);
        }
      }
    }
  }

  FILE *out;
  if ((out = fopen (outfile,"w")) == NULL) {
    sys_error(ERR_SEVERE, "Error opening output file %s for writing", outfile);
    return false;
  }
  gdImageGif(gif,out);
  fclose(out);
  gdImageDestroy(gif);

  delete rowp;

  return true;
}
#endif

bool
ImageFile::writeImageRaw (const char* const outfile, int nxcell, int nycell)
{
  FILE *fp;
  int nx = m_nx;
  int ny = m_ny;
  ImageFileArray v = getArray();

  if ((fp = fopen (outfile, "wb")) == NULL)
    return false;

  for (int irow = ny - 1; irow >= 0; irow--) {
    for (int icol = 0; icol < nx; icol++) {
      float dens = v[icol][irow];
     fwrite(&dens, sizeof(float), 1, fp);
    }
  }

  fclose(fp);
  return true;
}


