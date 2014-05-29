/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:           ctndicomp.cpp
**  Purpose:      Interface to CTN Dicom header
**      Programmer:   Kevin Rosenberg
**      Date Started: March 2001
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

#ifndef _CTNDICOM_H_
#define _CTNDICOM_H_
#if HAVE_CTN_DICOM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef GCCSUNOS
#include <sys/types.h>
#endif

#include "ctsupport.h"

#if SIZEOF_LONG == 4
#define LONGSIZE 32
#elif SIZEOF_LONG ==8
#define LONGSIZE 64
#endif

#if SIZEOF_INT == 2
#define INTSIZE 16
#elif SIZEOF_INT == 4
#define INTSIZE 32
#elif SIZEOF_INT == 8
#define INTSIZE 64
#endif

#if SIZEOF_SHORT == 2
#define SHORTSIZE 16
#elif SIZEOF_SHORT == 4
#define SHORTSIZE 32
#endif

#include "dicom.h"
#include "ctnthread.h"
#include "lst.h"
#include "condition.h"
#include "dicom_objects.h"

#include <string>
class ImageFile;
class Projections;

class DicomImporter {
private:
  std::string m_strFilename;
  bool m_bFail;
  std::string m_strFailMessage;
  int m_iContents;
  ImageFile* m_pImageFile;
  Projections* m_pProjections;
  DCM_OBJECT* m_pFile;

  void loadImage(unsigned short iNRows, unsigned short iNCols, unsigned short iBitsAllocated,
            unsigned short iBitsStored, unsigned short iHighBit, unsigned short iPixRep);

  void loadProjections();

  enum {
    TAG_GROUP_SOMATOM = 0x7fe1,
    TAG_MEMBER_SOMATOM_DATA = 0x1000,
  };

public:
  enum {
    DICOM_CONTENTS_INVALID = -1,
    DICOM_CONTENTS_IMAGE,
    DICOM_CONTENTS_PROJECTIONS,
  };

  DicomImporter (const char* const pszFile);
  ~DicomImporter();

  bool testImage() const {return m_iContents == DICOM_CONTENTS_IMAGE;}
  bool testProjections() const {return m_iContents == DICOM_CONTENTS_PROJECTIONS;}
  bool fail() const {return m_bFail;}
  const std::string& failMessage() const {return m_strFailMessage;}

  ImageFile* getImageFile() const {return m_pImageFile;}
  Projections* getProjections() const {return m_pProjections;}
};


class DicomExporter {
private:
  const ImageFile* m_pImageFile;
  std::string m_strFilename;
  bool m_bFail;
  std::string m_strFailMessage;
  DCM_OBJECT* m_pObject;

  bool createDicomObject();

public:

  DicomExporter (ImageFile* pImageFile);
  ~DicomExporter();

  bool writeFile (const char* const pszFilename);
  bool fail() const {return m_bFail;}
  const std::string& failMessage() const {return m_strFailMessage;}
};

#endif // HAVE_CTN_DICOM
#endif // _CTNDICOM_H_
