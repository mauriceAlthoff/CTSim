/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         imagefile.h
**      Purpose:      imagefile class header
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

#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#ifndef MSVC
#include <unistd.h>
#endif
#include <string>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include "ctsupport.h"
#include "fnetorderstream.h"
#include "array2dfile.h"

#ifdef HAVE_MPI
#include <mpi++.h>
#endif

class F32Image : public Array2dFile
{
public:
  F32Image (int nx, int ny, int dataType = Array2dFile::DATA_TYPE_REAL);
  F32Image (void);

  kfloat32** getArray (void)
      { return (kfloat32**) (m_arrayData); }

  kfloat32** const getArray (void) const
       { return (kfloat32** const) (m_arrayData); }

  kfloat32** getImaginaryArray (void)
      { return (kfloat32**) (m_imaginaryArrayData); }

  kfloat32** const getImaginaryArray (void) const
       { return (kfloat32** const) (m_imaginaryArrayData); }

#ifdef HAVE_MPI
  MPI::Datatype getMPIDataType (void) const
      { return MPI::FLOAT; }
#endif

 private:
  F32Image (const F32Image& rhs);             //copy constructor
  F32Image& operator= (const F32Image& rhs);  // assignment operator
};


class F64Image : public Array2dFile
{
 public:

   F64Image (int nx, int ny, int dataType = Array2dFile::DATA_TYPE_REAL);
  F64Image (void);

  kfloat64** getArray (void)
      { return (kfloat64**) (m_arrayData); }

  kfloat64** const getArray (void) const
      { return (kfloat64** const) (m_arrayData); }

  kfloat64** getImaginaryArray (void)
      { return (kfloat64**) (m_imaginaryArrayData); }

  kfloat64** const getImaginaryArray (void) const
      { return (kfloat64** const) (m_imaginaryArrayData); }

#ifdef HAVE_MPI
  MPI::Datatype getMPIDataType (void) const
      { return MPI::DOUBLE; }
#endif
 private:
  F64Image (const F64Image& rhs);             //copy constructor
  F64Image& operator= (const F64Image& rhs);  // assignment operator
};

#undef IMAGEFILE_64_BITS
#ifdef IMAGEFILE_64_BITS
typedef F64Image   ImageFileBase;
typedef kfloat64   ImageFileValue;
typedef kfloat64*  ImageFileColumn;
typedef kfloat64** ImageFileArray;
typedef kfloat64** const ImageFileArrayConst;
typedef const kfloat64* ImageFileColumnConst;
#else
typedef F32Image   ImageFileBase;
typedef kfloat32   ImageFileValue;
typedef kfloat32*  ImageFileColumn;
typedef kfloat32** ImageFileArray;
typedef kfloat32** const ImageFileArrayConst;
typedef const kfloat32* ImageFileColumnConst;
#endif



class ImageFile : public ImageFileBase
{
private:

  static const char* s_aszExportFormatName[];
  static const char* s_aszExportFormatTitle[];
  static const int s_iExportFormatCount;
  static const char* s_aszImportFormatName[];
  static const char* s_aszImportFormatTitle[];
  static const int s_iImportFormatCount;

  static void skipSpacePPM (FILE* fp); // skip space in a ppm file

public:

  static const int EXPORT_FORMAT_INVALID;
  static const int IMPORT_FORMAT_INVALID;
  static const int EXPORT_FORMAT_TEXT;
  static const int EXPORT_FORMAT_PGM;
  static const int EXPORT_FORMAT_PGMASCII;
  static const int IMPORT_FORMAT_PPM;
#if HAVE_PNG
  static const int EXPORT_FORMAT_PNG;
  static const int EXPORT_FORMAT_PNG16;
  static const int IMPORT_FORMAT_PNG;
#endif
#if HAVE_CTN_DICOM
  static const int EXPORT_FORMAT_DICOM;
  static const int IMPORT_FORMAT_DICOM;
#endif
  static const int EXPORT_FORMAT_RAW;

  static const int getExportFormatCount() {return s_iExportFormatCount;}
  static const char** getExportFormatNameArray() {return s_aszExportFormatName;}
  static const char** getExportFormatTitleArray() {return s_aszExportFormatTitle;}
  static int convertExportFormatNameToID (const char* const ExportFormatName);
  static const char* convertExportFormatIDToName (const int ExportFormatID);
  static const char* convertExportFormatIDToTitle (const int ExportFormatID);

  static const int getImportFormatCount() {return s_iImportFormatCount;}
  static const char** getImportFormatNameArray() {return s_aszImportFormatName;}
  static const char** getImportFormatTitleArray() {return s_aszImportFormatTitle;}
  static int convertImportFormatNameToID (const char* const ImportFormatName);
  static const char* convertImportFormatIDToName (const int ImportFormatID);
  static const char* convertImportFormatIDToTitle (const int ImportFormatID);

  static const double s_dRedGrayscaleFactor;
  static const double s_dGreenGrayscaleFactor;
  static const double s_dBlueGrayscaleFactor;

  ImageFile (int nx, int ny)
      : ImageFileBase (nx, ny)
  {}

  ImageFile (void)
      : ImageFileBase ()
  {}

  void getCenterCoordinates (unsigned int& iXCenter, unsigned int& iYCenter);

  bool convertRealToComplex ();
  bool convertComplexToReal ();

  void filterResponse (const char* const domainName, double bw, const char* const filterName, double filt_param, double dInputScale = 1., double dOutputScale = 1.);

  void statistics (double& min, double& max, double& mean, double& mode, double& median, double& stddev) const;
  void statistics (ImageFileArrayConst v, double& min, double& max, double& mean, double& mode, double& median, double& stddev) const;
  void getMinMax (double& min, double& max) const;
  void printStatistics (std::ostream& os) const;
  bool comparativeStatistics (const ImageFile& imComp, double& d, double& r, double& e) const;
  bool printComparativeStatistics (const ImageFile& imComp, std::ostream& os) const;

  bool subtractImages (const ImageFile& rRHS, ImageFile& result) const;
  bool addImages (const ImageFile& rRHS, ImageFile& result) const;
  bool multiplyImages (const ImageFile& rRHS, ImageFile& result) const;
  bool divideImages (const ImageFile& rRHS, ImageFile& result) const;

  bool scaleImage (ImageFile& result) const;

  bool invertPixelValues (ImageFile& result) const;
  bool sqrt (ImageFile& result) const;
  bool square (ImageFile& result) const;
  bool log (ImageFile& result) const;
  bool exp (ImageFile& result) const;
  bool fourier (ImageFile& result) const;
  bool inverseFourier (ImageFile& result) const;
#ifdef HAVE_FFTW
  bool fft (ImageFile& result) const;
  bool ifft (ImageFile& result) const;
  bool fftRows (ImageFile& result) const;
  bool ifftRows (ImageFile& result) const;
  bool fftCols (ImageFile& result) const;
  bool ifftCols (ImageFile& result) const;
#endif
  bool magnitude (ImageFile& result) const;
  bool phase (ImageFile& result) const;
  bool real (ImageFile& result) const;
  bool imaginary (ImageFile& result) const;

  bool exportImage (const char* const pszFormat, const char* const pszFilename, int nxcell, int nycell, double densmin, double densmax);

  bool importImage (const char* const pszFormat, const char* const pszFilename);

#ifdef HAVE_PNG
  bool writeImagePNG (const char* const outfile, int bitdepth, int nxcell, int nycell, double densmin, double densmax);
  bool readImagePNG (const char* const pszFile);
#endif
#ifdef HAVE_GD
  bool writeImageGIF (const char* const outfile, int nxcell, int nycell, double densmin, double densmax);
#endif
  bool writeImagePGM (const char* const outfile, int nxcell, int nycell, double densmin, double densmax);
  bool writeImagePGMASCII (const char* const outfile, int nxcell, int nycell, double densmin, double densmax);
  bool readImagePPM (const char* const pszFile);
  bool writeImageRaw(const char* const outfile, int nxcell, int nycell);
  bool writeImageText (const char* const outfile);

  static double redGrayscaleFactor() {return s_dRedGrayscaleFactor;}
  static double greenGrayscaleFactor() {return s_dGreenGrayscaleFactor;}
  static double blueGrayscaleFactor() {return s_dBlueGrayscaleFactor;}
  static double colorToGrayscale (double r, double g, double b)
  { return r * s_dRedGrayscaleFactor + g * s_dGreenGrayscaleFactor + b * s_dBlueGrayscaleFactor; }
};


#endif
