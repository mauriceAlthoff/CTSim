/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:           ctndicomp.cpp
**  Purpose:      Interface to CTN Dicom classes
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CTN_DICOM

#include "ctndicom.h"
#include "imagefile.h"
#include "projections.h"


DicomImporter::DicomImporter (const char* const pszFile)
  : m_strFilename(pszFile), m_bFail(false), m_iContents(DICOM_CONTENTS_INVALID),
    m_pImageFile(NULL), m_pProjections(NULL), m_pFile(NULL)
{
  unsigned long lOptions = DCM_ORDERLITTLEENDIAN | DCM_FORMATCONVERSION;
  DCM_Debug (FALSE);

  CONDITION cond = DCM_OpenFile (pszFile, lOptions, &m_pFile);
  if (cond != DCM_NORMAL) {
    m_bFail = true;
    char textbuf [2048];
    CONDITION cond2 = COND_TopCondition (&cond, textbuf, sizeof(textbuf));
    cond2 = DCM_NORMAL; // testing
    if (cond2 != DCM_NORMAL) {
      m_strFailMessage = "DCM_OpenFile failure: ";
      m_strFailMessage += m_strFilename;
    } else
      m_strFailMessage = textbuf;

    return;
  }

  unsigned short iNRows, iNCols, iBitsAllocated, iBitsStored, iHighBit, iPixRep;
  DCM_ELEMENT aElemRequired[] = {
    {DCM_IMGROWS, DCM_US, "", 1, sizeof(iNRows),
     {reinterpret_cast<char*>(&iNRows)}},
    {DCM_IMGCOLUMNS, DCM_US, "", 1, sizeof(iNCols),
     {reinterpret_cast<char*>(&iNCols)}},
    {DCM_IMGBITSALLOCATED, DCM_US, "", 1, sizeof(iBitsAllocated),
     {reinterpret_cast<char*>(&iBitsAllocated)}},
    {DCM_IMGBITSSTORED, DCM_US, "", 1, sizeof(iBitsStored),
     {reinterpret_cast<char*>(&iBitsStored)}},
    {DCM_IMGHIGHBIT, DCM_US, "", 1, sizeof(iHighBit),
     {reinterpret_cast<char*>(&iHighBit)}},
    {DCM_IMGPIXELREPRESENTATION, DCM_US, "", 1, sizeof(iPixRep),
     {reinterpret_cast<char*>(&iPixRep)}},
  };
  int nElemRequired = sizeof (aElemRequired) / sizeof(DCM_ELEMENT);

  if (DCM_ParseObject (&m_pFile, aElemRequired, nElemRequired, NULL, 0, NULL) == DCM_NORMAL) {
    loadImage (iNRows, iNCols, iBitsAllocated, iBitsStored, iHighBit, iPixRep);
    return;
  }
  U32 lRtnLength;
  DCM_TAG somatomTag = DCM_MAKETAG(TAG_GROUP_SOMATOM, TAG_MEMBER_SOMATOM_DATA);
  if (DCM_GetElementSize (&m_pFile, somatomTag, &lRtnLength) == DCM_NORMAL)
    loadProjections();
}

void
DicomImporter::loadImage(unsigned short iNRows, unsigned short iNCols, unsigned short iBitsAllocated,
                         unsigned short iBitsStored, unsigned short iHighBit, unsigned short iPixRep)
{
  U32 lRtnLength;
  unsigned short iSamplesPerPixel, iPlanarConfig;

  DCM_ELEMENT elemPlanarConfig =
    {DCM_IMGPLANARCONFIGURATION, DCM_US, "", 1, sizeof(iPlanarConfig),
     {reinterpret_cast<char*>(&iPlanarConfig)}};
  DCM_ELEMENT elemSamplesPerPixel =
    {DCM_IMGSAMPLESPERPIXEL, DCM_US, "", 1,
     sizeof(iSamplesPerPixel), {reinterpret_cast<char*>(&iSamplesPerPixel)}};

  if (DCM_ParseObject (&m_pFile, &elemSamplesPerPixel, 1, NULL, 0, NULL) != DCM_NORMAL)
    iSamplesPerPixel = 1;  // default value

  if (iSamplesPerPixel > 1) {
    void* ctx = NULL;
    if (DCM_GetElementValue (&m_pFile, &elemPlanarConfig, &lRtnLength, &ctx) != DCM_NORMAL) {
      m_bFail = true;
      m_strFailMessage = "Planar Configuration not specified when iSamplesPerPixel > 1";
    }
  }

  char szRescaleSlope[17];
  char szRescaleIntercept[17];
  double dRescaleSlope = 1;
  double dRescaleIntercept = 0;
  DCM_ELEMENT elemRescaleSlope =
    {DCM_IMGRESCALESLOPE, DCM_DS, "", 1, strlen(szRescaleSlope),
     {szRescaleSlope}};
  DCM_ELEMENT elemRescaleIntercept =
    {DCM_IMGRESCALEINTERCEPT, DCM_DS, "", 1, strlen(szRescaleIntercept),
     {szRescaleIntercept}};
  if (DCM_ParseObject (&m_pFile, &elemRescaleSlope, 1, NULL, 0, NULL) == DCM_NORMAL) {
    if (sscanf (szRescaleSlope, "%lf", &dRescaleSlope) != 1)
      sys_error (ERR_SEVERE, "Error parsing rescale slope");
  }
  if (DCM_ParseObject (&m_pFile, &elemRescaleIntercept, 1, NULL, 0, NULL) == DCM_NORMAL) {
    if (sscanf (szRescaleIntercept, "%lf", &dRescaleIntercept) != 1)
      sys_error (ERR_SEVERE, "Error parsing rescale intercept");
  }

  DCM_ELEMENT elemPixelData = {DCM_PXLPIXELDATA, DCM_OT, "", 1, 0, {NULL}};
  // Get the actual  pixel data (the only other required element)
  if (DCM_GetElementSize (&m_pFile, elemPixelData.tag, &lRtnLength) != DCM_NORMAL) {
    m_bFail = true;
    m_strFailMessage = "Can't get pixel data size";
    return;
  }

  // Check the size of the pixel data to make sure we have the correct amount...
  unsigned long lRealLength = lRtnLength;
  unsigned long lCheckLengthInBytes = iNRows * iNCols * iSamplesPerPixel;
  if (iBitsAllocated == 16)
    lCheckLengthInBytes *= 2;
  if (lCheckLengthInBytes > lRealLength) {
    m_bFail = true;
    m_strFailMessage = "Too little pixel data supplied";
    return;
  }
  // Now allocate the memory to hold the pixel data and get it from the DICOM file...

  unsigned char* pRawPixels = new unsigned char [lCheckLengthInBytes];
  elemPixelData.length = lCheckLengthInBytes;
  elemPixelData.d.ot = pRawPixels;

  void* ctx = NULL;
  CONDITION cond = DCM_GetElementValue (&m_pFile, &elemPixelData, &lRtnLength, &ctx);
  if ((cond != DCM_NORMAL) && (cond != DCM_GETINCOMPLETE)) {
    m_bFail = true;
    m_strFailMessage = "Can't read pixel data";
    delete pRawPixels;
    return;
  }
  if ((lCheckLengthInBytes < lRealLength) && (cond != DCM_GETINCOMPLETE)) {
    m_bFail = true;
    m_strFailMessage = "Should have gooten incomplete message reading pixel data";
    delete pRawPixels;
    return;
  }

  m_pImageFile = new ImageFile (iNCols, iNRows);
  ImageFileArray v = m_pImageFile->getArray();
  double dScale = 1 << iBitsStored;
  unsigned int iMaskLength = iBitsStored;
  if (iMaskLength > 8)
    iMaskLength -= 8;
  unsigned int iMask = (1 << iMaskLength) - 1;
  for (int iy = 0; iy < iNRows; iy++) {
    for (int ix = 0; ix < iNCols; ix++) {
      if (iBitsAllocated == 8)  {
        unsigned char cV = pRawPixels[iy * iNRows + ix];
        v[ix][iy] = (cV & iMask) / dScale;
      } else if (iBitsAllocated == 16) {
        unsigned long lBase = (iy * iNRows + ix) * 2;
        unsigned char cV1 = pRawPixels[lBase];
        unsigned char cV2 = pRawPixels[lBase+1] & iMask;
        int iV = cV1 + (cV2 << 8);
        v[ix][iNRows - 1 - iy] = iV * dRescaleSlope + dRescaleIntercept;
      }
    }
  }
  m_iContents = DICOM_CONTENTS_IMAGE;
}

void
DicomImporter::loadProjections()
{
  U32 lRtnLength;
  void* ctx = NULL;

  unsigned short iNViews, iNDets;
  DCM_ELEMENT aElemRequired[] = {
    {DCM_IMGROWS, DCM_US, "", 1, sizeof(iNViews),
     {reinterpret_cast<char*>(&iNViews)}},
    {DCM_IMGCOLUMNS, DCM_US, "", 1, sizeof(iNDets),
     {reinterpret_cast<char*>(&iNDets)}},
  };
  int nElemRequired = sizeof (aElemRequired) / sizeof(DCM_ELEMENT);

  if (DCM_ParseObject (&m_pFile, aElemRequired, nElemRequired, NULL, 0, NULL) != DCM_NORMAL) {
    m_bFail = true;
    m_strFailMessage = "Unable to read header for projections";
    return;
  }

  DCM_TAG somatomTag = DCM_MAKETAG(TAG_GROUP_SOMATOM, TAG_MEMBER_SOMATOM_DATA);
  DCM_ELEMENT elemProjections = {somatomTag, DCM_UN, "", 1, 0, {NULL}};
  if (DCM_GetElementSize (&m_pFile, elemProjections.tag, &lRtnLength) != DCM_NORMAL) {
    m_bFail = true;
    m_strFailMessage = "Can't find projection data";
    return;
  }

  unsigned char* pRawProjections = new unsigned char [lRtnLength];
  elemProjections.length = lRtnLength;
  elemProjections.d.ot = pRawProjections;

  ctx = NULL;
  CONDITION cond = DCM_GetElementValue (&m_pFile, &elemProjections, &lRtnLength, &ctx);
  if ((cond != DCM_NORMAL) && (cond != DCM_GETINCOMPLETE)) {
    m_bFail = true;
    m_strFailMessage = "Can't read projections data";
    delete pRawProjections;
    return;
  }
  m_iContents = DICOM_CONTENTS_PROJECTIONS;
  m_pProjections = new Projections;
  if (! m_pProjections->initFromSomatomAR_STAR (iNViews, iNDets, pRawProjections, lRtnLength)) {
    m_bFail = true;
    m_strFailMessage = "Error converting raw projection data";
    delete m_pProjections;
    m_pProjections = NULL;
  }

  delete pRawProjections;
}


DicomImporter::~DicomImporter()
{
  if (m_pFile)
    DCM_CloseObject (&m_pFile);
}


DicomExporter::DicomExporter (ImageFile* pImageFile)
: m_pImageFile(pImageFile), m_pObject(NULL)
{
  DCM_Debug (FALSE);
  if (! pImageFile) {
    m_bFail = true;
    m_strFailMessage = "Initialized DicomExported with NULL imagefile";
    return;
  }
  m_bFail = ! createDicomObject();
}

DicomExporter::~DicomExporter ()
{
  if (m_pObject)
    DCM_CloseObject (&m_pObject);
}

bool
DicomExporter::writeFile (const char* const pszFilename)
{
  if (! m_pObject)
    return false;

  m_strFilename = pszFilename;

  CONDITION cond = DCM_WriteFile (&m_pObject, DCM_ORDERLITTLEENDIAN, pszFilename);
  if (cond != DCM_NORMAL) {
    m_bFail = true;
    m_strFailMessage = "Error writing DICOM file ";
    m_strFailMessage += pszFilename;
    return false;
  }

  return true;
}

bool
DicomExporter::createDicomObject()
{
  CONDITION cond = DCM_CreateObject (&m_pObject, 0);
  if (cond != DCM_NORMAL) {
    m_bFail = true;
    m_strFailMessage = "Error creating DICOM object";
    return false;
  }

  double dMin, dMax;
  m_pImageFile->getMinMax (dMin, dMax);
  double dWidth = dMax - dMin;
  if (dWidth == 0.)
    dWidth = 1E-7;
  double dScale = 65535. / dWidth;

  double dRescaleIntercept = dMin;
  double dRescaleSlope = 1 / dScale;
  char szRescaleIntercept[17];
  char szRescaleSlope[17];
  snprintf (szRescaleIntercept, sizeof(szRescaleIntercept), "%e", dRescaleIntercept);
  snprintf (szRescaleSlope, sizeof(szRescaleIntercept), "%e", dRescaleSlope);

  char szCTSimRoot[] = "1.2.826.0.1.3680043.2.284.";
  char szModality[] = "CT";
  char szSOPClassUID[65] = "1.2.840.10008.5.4.1.1.2";
  char szImgPhotometricInterp[] = "MONOCHROME2";
  char szPixelSpacing[33] = "0\\0";
  char szRelImageOrientationPatient[100] = "1\\0\\0\\0\\1\\0";
  char szRelImagePositionPatient[49] = "0\\0\\0";
  char szAcqKvp[] = "0";
  char szRelAcquisitionNumber[] = "1";
  char szRelImageNumber[] = "1";
  char szIDSOPInstanceUID[65] = "";
  char szIDManufacturer[] = "CTSim";
  char szRelPositionRefIndicator[] = "0";
  char szRelFrameOfReferenceUID[65] = "";
  char szRelSeriesNumber[] = "1";
  char szIDAccessionNumber[] = "0";
  char szRelStudyID[] = "1";
  char szIDReferringPhysician[] = "NONE";
  char szIDStudyTime[] = "000000.0";
  char szIDStudyDate[] = "00000000";
  char szRelStudyInstanceUID[65] = "";
  char szPatSex[] = "O";
  char szPatBirthdate[] = "0000000";
  char szPatID[] = "NONE";
  char szPatName[] = "NONE";
  char szIDImageType[] = "ORIGINAL";
  char szIDManufacturerModel[65] = "";

  std::ostringstream osPatComments;
  m_pImageFile->printLabelsBrief (osPatComments);
  size_t sizePatComments = osPatComments.str().length();
  char* pszPatComments = new char [sizePatComments+1];
  strncpy (pszPatComments, osPatComments.str().c_str(), sizePatComments);

  snprintf (szIDSOPInstanceUID, sizeof(szIDSOPInstanceUID), "%s.2.1.6.1", szCTSimRoot);
  snprintf (szRelStudyInstanceUID, sizeof(szRelStudyInstanceUID), "%s.2.1.6.1.1", szCTSimRoot);
  snprintf (szRelFrameOfReferenceUID, sizeof(szRelFrameOfReferenceUID), "%s.99", szCTSimRoot);
#ifdef VERSION
  snprintf (szIDManufacturerModel, sizeof(szIDManufacturerModel), "VERSION %s", VERSION);
#endif
  snprintf (szPixelSpacing, sizeof(szPixelSpacing), "%e\\%e", m_pImageFile->axisIncrementX(), m_pImageFile->axisIncrementY());
  double minX, maxX, minY, maxY;
  if (m_pImageFile->getAxisExtent(minX, maxX, minY, maxY)) {
    minX += m_pImageFile->axisIncrementX() / 2;
    minY += m_pImageFile->axisIncrementY() / 2;
    snprintf(szRelImagePositionPatient, sizeof(szRelImagePositionPatient), "%e\\%e\\0", minX, minY);
  }

  unsigned short iNRows = m_pImageFile->ny();
  unsigned short iNCols = m_pImageFile->nx();
  unsigned short iBitsAllocated = 16;
  unsigned short iBitsStored = 16;
  unsigned short iHighBit = 15;
  unsigned short iPixRep = 0;
  unsigned short iSamplesPerPixel = 1;
  DCM_ELEMENT aElemRequired[] = {
    {DCM_IMGROWS, DCM_US, "", 1, sizeof(iNRows),
     {reinterpret_cast<char*>(&iNRows)}},
    {DCM_IMGCOLUMNS, DCM_US, "", 1, sizeof(iNCols),
     {reinterpret_cast<char*>(&iNCols)}},
    {DCM_IMGBITSALLOCATED, DCM_US, "", 1, sizeof(iBitsAllocated),
     {reinterpret_cast<char*>(&iBitsAllocated)}},
    {DCM_IMGBITSSTORED, DCM_US, "", 1, sizeof(iBitsStored),
     {reinterpret_cast<char*>(&iBitsStored)}},
    {DCM_IMGHIGHBIT, DCM_US, "", 1, sizeof(iHighBit),
     {reinterpret_cast<char*>(&iHighBit)}},
    {DCM_IMGPIXELREPRESENTATION, DCM_US, "", 1, sizeof(iPixRep),
     {reinterpret_cast<char*>(&iPixRep)}},
    {DCM_IMGSAMPLESPERPIXEL, DCM_US, "", 1, sizeof(iSamplesPerPixel),
     {reinterpret_cast<char*>(&iSamplesPerPixel)}},
    {DCM_IMGRESCALESLOPE, DCM_DS, "", 1, strlen(szRescaleSlope),
     {szRescaleSlope}},
    {DCM_IMGRESCALEINTERCEPT, DCM_DS, "", 1, strlen(szRescaleIntercept),
     {szRescaleIntercept}},
    {DCM_IMGPHOTOMETRICINTERP, DCM_CS, "", 1, strlen(szImgPhotometricInterp),
     {szImgPhotometricInterp}},
    {DCM_IMGPIXELSPACING, DCM_DS, "", 1, strlen(szPixelSpacing),
     {szPixelSpacing}},
    {DCM_RELIMAGEORIENTATIONPATIENT, DCM_DS, "", 1,
     strlen(szRelImageOrientationPatient), {szRelImageOrientationPatient}},
    {DCM_RELIMAGEPOSITIONPATIENT, DCM_DS, "", 1,
     strlen(szRelImagePositionPatient), {szRelImagePositionPatient}},
    {DCM_ACQKVP, DCM_DS, "", 1, strlen(szAcqKvp), {szAcqKvp}},
    {DCM_RELACQUISITIONNUMBER, DCM_IS, "", 1, strlen(szRelAcquisitionNumber),
     {szRelAcquisitionNumber}},
    {DCM_ACQSLICETHICKNESS, DCM_DS, "", 1, strlen(szRelAcquisitionNumber),
     {szRelAcquisitionNumber}},
    {DCM_RELIMAGENUMBER, DCM_IS, "", 1, strlen(szRelImageNumber),
     {szRelImageNumber}},
    {DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, strlen(szIDSOPInstanceUID),
     {szIDSOPInstanceUID}},
    {DCM_IDMANUFACTURER, DCM_LO, "", 1, strlen(szIDManufacturer),
     {szIDManufacturer}},
    {DCM_RELPOSITIONREFINDICATOR, DCM_LO, "", 1,
     strlen(szRelPositionRefIndicator), {szRelPositionRefIndicator}},
    {DCM_RELFRAMEOFREFERENCEUID, DCM_UI, "", 1,
     strlen(szRelFrameOfReferenceUID), {szRelFrameOfReferenceUID}},
    {DCM_RELSERIESNUMBER, DCM_IS, "", 1, strlen(szRelSeriesNumber),
     {szRelSeriesNumber}},
    {DCM_RELSERIESINSTANCEUID, DCM_UI, "", 1, strlen(szIDAccessionNumber),
     {szIDAccessionNumber}},
    {DCM_IDACCESSIONNUMBER, DCM_SH, "", 1, strlen(szIDAccessionNumber),
     {szIDAccessionNumber}},
    {DCM_RELSTUDYID, DCM_SH, "", 1, strlen(szRelStudyID), {szRelStudyID}},
    {DCM_IDREFERRINGPHYSICIAN, DCM_PN, "", 1, strlen(szIDReferringPhysician),
     {szIDReferringPhysician}},
    {DCM_IDSTUDYTIME, DCM_TM, "", 1, strlen(szIDStudyTime), {szIDStudyTime}},
    {DCM_IDSTUDYDATE, DCM_DA, "", 1, strlen(szIDStudyDate), {szIDStudyDate}},
    {DCM_RELSTUDYINSTANCEUID, DCM_UI, "", 1, strlen(szRelStudyInstanceUID),
     {szRelStudyInstanceUID}},
    {DCM_PATSEX, DCM_CS, "", 1, strlen(szPatSex), {szPatSex}},
    {DCM_PATBIRTHDATE, DCM_DA, "", 1, strlen(szPatBirthdate), {szPatBirthdate}},
    {DCM_PATID, DCM_LO, "", 1, strlen(szPatID), {szPatID}},
    {DCM_PATNAME, DCM_PN, "", 1, strlen(szPatName), {szPatName}},
    {DCM_IDIMAGETYPE, DCM_CS, "", 1, strlen(szIDImageType), {szIDImageType}},
    {DCM_IDMODALITY, DCM_CS, "", 1, strlen(szModality), {szModality}},
    {DCM_IDSOPCLASSUID, DCM_UI, "", 1, strlen(szSOPClassUID), {szSOPClassUID}},
    {DCM_IDMANUFACTURERMODEL, DCM_LO, "", 1, strlen(szIDManufacturerModel),
     {szIDManufacturerModel}},
    {DCM_PATCOMMENTS, DCM_LT, "", 1, strlen(pszPatComments), {pszPatComments}},
  };
  int nElemRequired = sizeof (aElemRequired) / sizeof(DCM_ELEMENT);

  int iUpdateCount;
  cond = DCM_ModifyElements (&m_pObject, aElemRequired, nElemRequired, NULL, 0, &iUpdateCount);

  DCM_ELEMENT elemPixelData = {DCM_PXLPIXELDATA, DCM_OT, "", 1, 0, {NULL}};

  unsigned long lRealLength = 2 * m_pImageFile->nx() * m_pImageFile->ny();

  unsigned char* pRawPixels = new unsigned char [lRealLength];
  elemPixelData.length = lRealLength;
  elemPixelData.d.ot = pRawPixels;

  ImageFileArray v = m_pImageFile->getArray();
  for (int iy = 0; iy < iNRows; iy++) {
    for (int ix = 0; ix < iNCols; ix++) {
        unsigned long lBase = (iy * iNRows + ix) * 2;
        unsigned int iValue = nearest<int>(dScale * (v[ix][iNRows - 1 - iy] - dMin));
        pRawPixels[lBase] = iValue & 0xFF;
        pRawPixels[lBase+1] = (iValue & 0xFF00) >> 8;
    }
  }
  cond = DCM_ModifyElements (&m_pObject, &elemPixelData, 1, NULL, 0, &iUpdateCount);
  delete pRawPixels;

  delete pszPatComments;

  if (cond != DCM_NORMAL || iUpdateCount != 1) {
    m_bFail = true;
    m_strFailMessage = "Error modifying pixel data";
    return false;
  }

  return true;
}

#endif // HAVE_CTN_DICOM


