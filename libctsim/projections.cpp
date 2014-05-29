/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:         projections.cpp         Projection data classes
**   Programmer:   Kevin Rosenberg
**   Date Started: Aug 84
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
#include <ctime>
#include "interpolator.h"

const kuint16 Projections::m_signature = ('P'*256 + 'J');

const int Projections::POLAR_INTERP_INVALID = -1;
const int Projections::POLAR_INTERP_NEAREST = 0;
const int Projections::POLAR_INTERP_BILINEAR = 1;
const int Projections::POLAR_INTERP_BICUBIC = 2;

const char* const Projections::s_aszInterpName[] =
{
  "nearest",
  "bilinear",
//  {"bicubic"},
};

const char* const Projections::s_aszInterpTitle[] =
{
  "Nearest",
  "Bilinear",
//  {"Bicubic"},
};

const int Projections::s_iInterpCount = sizeof(s_aszInterpName) / sizeof(char*);



/* NAME
*    Projections                Constructor for projections matrix storage
*
* SYNOPSIS
*    proj = projections_create (filename, nView, nDet)
*    Projections& proj          Allocated projections structure & matrix
*    int nView                  Number of rotated view
*    int nDet                   Number of detectors
*
*/

Projections::Projections (const Scanner& scanner)
: m_projData(0)
{
  initFromScanner (scanner);
}


Projections::Projections (const int nView, const int nDet)
: m_projData(0)
{
  init (nView, nDet);
}

Projections::Projections (void)
: m_projData(0)
{
  init (0, 0);
}

Projections::~Projections (void)
{
  deleteProjData();
}

int
Projections::convertInterpNameToID (const char* const interpName)
{
  int interpID = POLAR_INTERP_INVALID;

  for (int i = 0; i < s_iInterpCount; i++)
    if (strcasecmp (interpName, s_aszInterpName[i]) == 0) {
      interpID = i;
      break;
    }

    return (interpID);
}

const char*
Projections::convertInterpIDToName (const int interpID)
{
  static const char *interpName = "";

  if (interpID >= 0 && interpID < s_iInterpCount)
    return (s_aszInterpName[interpID]);

  return (interpName);
}

const char*
Projections::convertInterpIDToTitle (const int interpID)
{
  static const char *interpTitle = "";

  if (interpID >= 0 && interpID < s_iInterpCount)
    return (s_aszInterpTitle[interpID]);

  return (interpTitle);
}



void
Projections::init (const int nView, const int nDet)
{
  m_label.setLabelType (Array2dFileLabel::L_HISTORY);
  m_nView = nView;
  m_nDet = nDet;
  newProjData ();

  time_t t = time (NULL);
  tm* lt = localtime (&t);
  m_year = lt->tm_year;
  m_month = lt->tm_mon;
  m_day = lt->tm_mday;
  m_hour = lt->tm_hour;
  m_minute = lt->tm_min;
  m_second = lt->tm_sec;
}

void
Projections::initFromScanner (const Scanner& scanner)
{
  m_label.setLabelType (Array2dFileLabel::L_HISTORY);
  deleteProjData();
  init (scanner.nView(), scanner.nDet());

  m_rotInc = scanner.rotInc();
  m_detInc = scanner.detInc();
  m_detStart =  scanner.detStart();
  m_geometry = scanner.geometry();
  m_dFocalLength = scanner.focalLength();
  m_dSourceDetectorLength = scanner.sourceDetectorLength();
  m_dViewDiameter = scanner.viewDiameter();
  m_rotStart = scanner.offsetView()*scanner.rotInc();
  m_dFanBeamAngle = scanner.fanBeamAngle();
}

void
Projections::setNView (int nView)  // used by MPI to reduce # of views
{
  deleteProjData();
  init (nView, m_nDet);
}

//  Helical 180 Linear Interpolation.
//  This member function takes a set of helical scan projections and
//  performs a linear interpolation between pairs of complementary rays
//  to produce a single projection data set approximating what would be
//  measured at a single axial plane.
//  Complementary rays are rays which traverse the same path through the
//  phantom in opposite directions.
//
//  For parallel beam geometry, a ray with a given gantry angle beta and a
//  detector iDet will have a complementary ray at beta + pi and nDet-iDet
//
//  For equiangular or equilinear beam geometry the complementary ray to
//  gantry angle beta and fan-beam angle gamma is at
//  beta-hat = beta +2*gamma + pi, and gamma-hat =  -gamma.
//  Note that beta-hat - beta depends on gamma and is not constant.
//
//  The algorithm used here is from Crawford and King, Med. Phys. 17(6)
//  1990 p967; what they called method "C", CSH-HH.  It uses interpolation only
//  between pairs of complementary rays on either side of an image plane.
//  Input data must sample gantry angles from zero to
//  (2*pi + 2* fan-beam-angle).  The data set produced contains gantry
//  angles from 0 to Pi+fan-beam-angle.  This is a "halfscan" data set,
//  which still contains redundant data, and can be used with a half scan
//  reconstruction to produce an image.
//  In this particular implementation a lower triangle from (beta,gamma) =
//  (0,-fanAngle/2)->(2*fanAngle,-fanAngle/2)->(0,fanAngle/2) contains
//  zeros, but is actually redundant with data contained in the region
//  (pi+fanAngle,-fanAngle/2)->(pi+fanAngle, fanAngle/2) ->(pi-fanAngle,
//  fanAngle/2).
//
int
Projections::Helical180LI(int interpolation_view)
{
   if (m_geometry == Scanner::GEOMETRY_INVALID)
   {
       std::cerr << "Invalid geometry " << m_geometry << std::endl;
       return (2);
   }
   else if (m_geometry == Scanner::GEOMETRY_PARALLEL)
   {
       std::cerr << "Helical 180LI not yet implemented for PARALLEL geometry"
                   << std::endl;
       return (2);
   }
   else if (m_geometry == Scanner::GEOMETRY_EQUILINEAR)
   {
       std::cerr << "Helical 180LI not yet implemented for EQUILINEAR geometry"
                   << std::endl;
       return (2);
   }
   else if (m_geometry == Scanner::GEOMETRY_EQUIANGULAR)
   {
           return Helical180LI_Equiangular(interpolation_view);
   }
   else
   {
       std::cerr << "Invalid geometry  in projection data file" << m_geometry
                   << std::endl;
       return (2);
   }
}
int
Projections::Helical180LI_Equiangular(int interpView)
{
   double dbeta = m_rotInc;
   double dgamma =  m_detInc;
   double fanAngle = m_dFanBeamAngle;
   int offsetView=0;

   // is there enough data in the data set?  Should have 2(Pi+fanAngle)
   // coverage minimum
   if ( m_nView <  static_cast<int>((2*( PI + fanAngle ) ) / dbeta) -1 ){
       std::cerr   << "Data set does not include 360 +2*FanBeamAngle views"
                   << std::endl;
       return (1);
   }

   if (interpView < 0)   // use default position at PI+fanAngle
   {
       interpView = static_cast<int> ((PI+fanAngle)/dbeta);
   }
   else
   {
       // check if there is PI+fanAngle data on either side of the
       // of the specified image plane
       if ( interpView*dbeta < PI+fanAngle ||
            interpView*dbeta + PI + fanAngle > m_nView*dbeta)
       {
           std::cerr << "There isn't PI+fanAngle of data on either side of the requested interpolation view" << std::endl;
           return(1);
       }
       offsetView = interpView - static_cast<int>((PI+fanAngle)/dbeta);

   }
   int last_interp_view = static_cast<int> ((PI+fanAngle)/dbeta);


// make a new array for data...
   class DetectorArray ** newdetarray = new DetectorArray * [last_interp_view+1];
   for ( int i=0 ; i <= last_interp_view ; i++ ){
       newdetarray[i] = new DetectorArray (m_nDet);
       newdetarray[i]->setViewAngle((i+offsetView)*dbeta);
       DetectorValue* newdetval = (newdetarray[i])->detValues();
       // and initialize the data to zero
       for (int j=0; j < m_nDet; j++)
           newdetval[j] = 0.;
   }

   int last_acq_view = 2*last_interp_view;
   for ( int iView = 0 ; iView <= last_acq_view; iView++) {
       double beta = iView * dbeta;

       for ( int iDet = 0; iDet < m_nDet; iDet++) {
           double gamma = (iDet -(m_nDet-1)/2)* dgamma ;
           int newiView, newiDet;
           if (beta < PI+fanAngle) { //if (PI +fanAngle - beta > dbeta )
               //newbeta = beta;
               //newgamma = gamma;
               newiDet = iDet;
               newiView = iView;
           }
           else // (beta > PI+fanAngle)
           {
               //newbeta = beta +2*gamma - 180;
               //newgamma = -gamma;
               newiDet = -iDet + (m_nDet -1);
               // newiView = nearest<int>((beta + 2*gamma - PI)/dbeta);
               //newiView = static_cast<int>(( (iView*dbeta) + 2*(iDet-(m_nDet-1)/2)*dgamma - PI)/dbeta);
               newiView = nearest<int>(( (iView*dbeta) + 2*(iDet-(m_nDet-1)/2)*dgamma - PI)/dbeta);
           }

#ifdef DEBUG
//std::cout << beta << " "<< gamma << " " << newbeta << " " << newgamma <<"    " << iView-offsetView << " " << iDet << " " << newiView << " " << newiDet << std::endl;
//std::cout << iView-offsetView << " " << iDet << " " << newiView << " " << newiDet << std::endl;
#endif

           if (   ( beta > fanAngle - 2*gamma)
               && ( beta < 2*PI + fanAngle -2*gamma)  )
          {  // not in region  1 or 8
               DetectorValue* detval = (m_projData[iView+offsetView])->detValues();
               DetectorValue* newdetval = (newdetarray[newiView])->detValues();
               if (   beta > fanAngle - 2*gamma
                   && beta <= 2*fanAngle ) {  // in region 2
                   newdetval[newiDet] +=
                       (beta +2*gamma - fanAngle)/(PI+2*gamma)
                               * detval[iDet];
               } else if ( beta > 2*fanAngle
                          && beta <= PI - 2*gamma) {  // in region 3
                   newdetval[newiDet] +=
                       (beta +2*gamma - fanAngle)/(PI+2*gamma)
                               * detval[iDet];
               }
               else if (   beta > PI -2*gamma
                        && beta <= PI + fanAngle ) {  // in region 4
                   newdetval[newiDet] +=
                       (beta +2*gamma - fanAngle)/(PI+2*gamma)
                               * detval[iDet];
               }
               else if (   beta > PI + fanAngle
                        && beta <= PI +2*fanAngle -2*gamma) { // in region 5
                   newdetval[newiDet] +=
                       (2*PI - beta - 2*gamma + fanAngle)/(PI-2*gamma)
                               *detval[iDet];
               }
               else if (   beta > PI +2*fanAngle -2*gamma
                        && beta <= 2*PI) {  // in region 6
                   newdetval[newiDet] +=
                       (2*PI - beta - 2*gamma + fanAngle)/(PI-2*gamma)
                       *detval[iDet];
               }
               else if (   beta > 2*PI
                        && beta <= 2*PI + fanAngle -2*gamma){ // in region 7
                   newdetval[newiDet] +=
                       (2*PI - beta -2*gamma + fanAngle)/(PI-2*gamma)
                       *detval[iDet];
               }
               else
               {
                   ; // outside region of interest
               }
           }
       }
   }
   deleteProjData();
   m_projData = newdetarray;
   m_nView = last_interp_view+1;

   return (0);
}
// HalfScanFeather:
// A HalfScan Projection Data Set  for equiangular geometry,
// covering gantry angles from 0 to  pi+fanBeamAngle
// and fan angle gamma from -fanBeamAngle/2 to fanBeamAngle/2
// contains redundant information.  If one copy of this data is left as
// zero, (as in the Helical180LI routine above) overweighting is avoided,
// but the discontinuity in the data introduces ringing in the image.
// This routine makes a copy of the data and applies a weighting to avoid
// over-representation, as given in Appendix C of Crawford and King, Med
// Phys 17 1990, p967.
int
Projections::HalfScanFeather(void)
{
   double dbeta = m_rotInc;
   double dgamma =  m_detInc;
   double fanAngle = m_dFanBeamAngle;

// is there enough data?
   if ( m_nView !=  static_cast<int>(( PI+fanAngle ) / dbeta) +1 ){
       std::cerr   << "Data set does seem have enough data to be a halfscan data set"  << std::endl;
       return (1);
   }
   if (m_geometry == Scanner::GEOMETRY_INVALID) {
       std::cerr << "Invalid geometry " << m_geometry << std::endl;
       return (2);
   }

   if (m_geometry == Scanner::GEOMETRY_PARALLEL) {
       std::cerr << "HalfScanFeather not yet implemented for PARALLEL geometry"<< std::endl;
       return (2);
   }

   for ( int iView2 = 0 ; iView2 < m_nView; iView2++) {
       double beta2 = iView2 * dbeta;
       for ( int iDet2 = 0; iDet2 < m_nDet; iDet2++) {
           double gamma2 = (iDet2 -(m_nDet-1)/2)* dgamma ;
           if ( ( beta2 >= PI  - 2*gamma2) ) {  // in redundant data region
               int iView1, iDet1;
               iDet1 =  (m_nDet -1) - iDet2;
               //iView1 = nearest<int>((beta2 + 2*gamma2 - PI)/dbeta);
               iView1 = nearest<int>(( (iView2*dbeta)
                               + 2*(iDet2-(m_nDet-1)/2)*dgamma - PI)/dbeta);


               DetectorValue* detval2 = (m_projData[iView2])->detValues();
               DetectorValue* detval1 = (m_projData[iView1])->detValues();

               detval1[iDet1] = detval2[iDet2] ;

               double x, w1,w2,beta1, gamma1;
               beta1= iView1*dbeta;
               gamma1 = -gamma2;
               if ( beta1 <= (fanAngle - 2*gamma1) )
                   x = beta1 / ( fanAngle - 2*gamma1);
               else if ( (fanAngle  - 2*gamma1 <= beta1 ) && beta1 <= PI - 2*gamma1)
                   x = 1;
               else if ( (PI - 2*gamma1 <= beta1 ) && ( beta1 <=PI + fanAngle) )
                   x = (PI +fanAngle - beta1)/(fanAngle + 2*gamma1);
               else {
                   std::cerr << "Shouldn't be here!"<< std::endl;
                   return(4);
               }
               w1 = (3*x - 2*x*x)*x;
               w2 = 1-w1;
               detval1[iDet1] *= w1;
               detval2[iDet2] *= w2;

           }
       }
   }
   // heuristic scaling, why this factor?
   double scalefactor = m_nView * m_rotInc / PI;
   for ( int iView = 0 ; iView < m_nView; iView++) {
       DetectorValue* detval = (m_projData[iView])->detValues();
       for ( int iDet = 0; iDet < m_nDet; iDet++) {
           detval[iDet] *= scalefactor;
       }
   }

   return (0);
}

// NAME
// newProjData

void
Projections::newProjData (void)
{
  if (m_projData)
    sys_error(ERR_WARNING, "m_projData != NULL [newProjData]");

  if (m_nView > 0 && m_nDet) {
    m_projData = new DetectorArray* [m_nView];

    for (int i = 0; i < m_nView; i++)
      m_projData[i] = new DetectorArray (m_nDet);
  }
}


/* NAME
*   projections_free                    Free memory allocated to projections
*
* SYNOPSIS
*   projections_free(proj)
*   Projections& proj                           Projectionss to be deallocated
*/

void
Projections::deleteProjData (void)
{
  if (m_projData != NULL) {
    for (int i = 0; i < m_nView; i++)
      delete m_projData[i];

    delete m_projData;
    m_projData = NULL;
  }
}


/* NAME
*    Projections::headerWwrite         Write data header for projections file
*
*/

bool
Projections::headerWrite (fnetorderstream& fs)
{
  kuint16 _hsize = m_headerSize;
  kuint16 _signature = m_signature;
  kuint32 _nView = m_nView;
  kuint32 _nDet = m_nDet;
  kuint32 _geom = m_geometry;
  kuint16 _remarksize = m_remark.length();
  kuint16 _year = m_year;
  kuint16 _month = m_month;
  kuint16 _day = m_day;
  kuint16 _hour = m_hour;
  kuint16 _minute = m_minute;
  kuint16 _second = m_second;

  kfloat64 _calcTime = m_calcTime;
  kfloat64 _rotStart = m_rotStart;
  kfloat64 _rotInc = m_rotInc;
  kfloat64 _detStart = m_detStart;
  kfloat64 _detInc = m_detInc;
  kfloat64 _viewDiameter = m_dViewDiameter;
  kfloat64 _focalLength = m_dFocalLength;
  kfloat64 _sourceDetectorLength = m_dSourceDetectorLength;
  kfloat64 _fanBeamAngle = m_dFanBeamAngle;

  fs.seekp(0);
  if (! fs)
    return false;

  fs.writeInt16 (_hsize);
  fs.writeInt16 (_signature);
  fs.writeInt32 (_nView);
  fs.writeInt32 (_nDet);
  fs.writeInt32 (_geom);
  fs.writeFloat64 (_calcTime);
  fs.writeFloat64 (_rotStart);
  fs.writeFloat64 (_rotInc);
  fs.writeFloat64 (_detStart);
  fs.writeFloat64 (_detInc);
  fs.writeFloat64 (_viewDiameter);
  fs.writeFloat64 (_focalLength);
  fs.writeFloat64 (_sourceDetectorLength);
  fs.writeFloat64 (_fanBeamAngle);
  fs.writeInt16 (_year);
  fs.writeInt16 (_month);
  fs.writeInt16 (_day);
  fs.writeInt16 (_hour);
  fs.writeInt16 (_minute);
  fs.writeInt16 (_second);
  fs.writeInt16 (_remarksize);
  fs.write (m_remark.c_str(), _remarksize);

  m_headerSize = fs.tellp();
  _hsize = m_headerSize;
  fs.seekp(0);
  fs.writeInt16 (_hsize);
  if (! fs)
    return false;

  return true;
}

/* NAME
*    projections_read_header         Read data header for projections file
*
*/
bool
Projections::headerRead (fnetorderstream& fs)
{
  kuint16 _hsize, _signature, _year, _month, _day, _hour, _minute, _second, _remarksize = 0;
  kuint32 _nView, _nDet, _geom;
  kfloat64 _calcTime, _rotStart, _rotInc, _detStart, _detInc, _focalLength, _sourceDetectorLength, _viewDiameter, _fanBeamAngle;

  fs.seekg(0);
  if (! fs)
    return false;

  fs.readInt16 (_hsize);
  fs.readInt16 (_signature);
  fs.readInt32 (_nView);
  fs.readInt32 (_nDet);
  fs.readInt32 (_geom);
  fs.readFloat64 (_calcTime);
  fs.readFloat64 (_rotStart);
  fs.readFloat64 (_rotInc);
  fs.readFloat64 (_detStart);
  fs.readFloat64 (_detInc);
  fs.readFloat64 (_viewDiameter);
  fs.readFloat64 (_focalLength);
  fs.readFloat64 (_sourceDetectorLength);
  fs.readFloat64 (_fanBeamAngle);
  fs.readInt16 (_year);
  fs.readInt16 (_month);
  fs.readInt16 (_day);
  fs.readInt16 (_hour);
  fs.readInt16 (_minute);
  fs.readInt16 (_second);
  fs.readInt16 (_remarksize);

  if (! fs) {
    sys_error (ERR_SEVERE, "Error reading header information , _remarksize=%d [projections_read_header]", _remarksize);
    return false;
  }

  if (_signature != m_signature) {
    sys_error (ERR_SEVERE, "File %s does not have a valid projection file signature", m_filename.c_str());
    return false;
  }

  char* pszRemarkStorage = new char [_remarksize+1];
  fs.read (pszRemarkStorage, _remarksize);
  if (! fs) {
    sys_error (ERR_SEVERE, "Error reading remark, _remarksize = %d", _remarksize);
    return false;
  }
  pszRemarkStorage[_remarksize] = 0;
  m_remark = pszRemarkStorage;
  delete pszRemarkStorage;

  off_t _hsizeread = fs.tellg();
  if (!fs || _hsizeread != _hsize) {
    sys_error (ERR_WARNING, "File header size read %ld != file header size stored %ld [read_projections_header]\n_remarksize=%ld", (long int) _hsizeread, _hsize, _remarksize);
    return false;
  }

  m_headerSize = _hsize;
  m_nView = _nView;
  m_nDet = _nDet;
  m_geometry = _geom;
  m_calcTime = _calcTime;
  m_rotStart = _rotStart;
  m_rotInc = _rotInc;
  m_detStart = _detStart;
  m_detInc = _detInc;
  m_dFocalLength = _focalLength;
  m_dSourceDetectorLength = _sourceDetectorLength;
  m_dViewDiameter = _viewDiameter;
  m_dFanBeamAngle = _fanBeamAngle;
  m_year = _year;
  m_month = _month;
  m_day = _day;
  m_hour = _hour;
  m_minute = _minute;
  m_second = _second;

  m_label.setLabelType (Array2dFileLabel::L_HISTORY);
  m_label.setLabelString (m_remark);
  m_label.setCalcTime (m_calcTime);
  m_label.setDateTime (m_year, m_month, m_day, m_hour, m_minute, m_second);

  return true;
}

bool
Projections::read (const std::string& filename)
{
  return read (filename.c_str());
}


bool
Projections::read (const char* filename)
{
  m_filename = filename;
#ifdef MSVC
  frnetorderstream fileRead (m_filename.c_str(), std::ios::in | std::ios::binary);
#else
  frnetorderstream fileRead (m_filename.c_str(), std::ios::in | std::ios::binary); // | std::ios::nocreate);
#endif

  if (fileRead.fail())
    return false;

  if (! headerRead (fileRead))
    return false;

  deleteProjData ();
  newProjData();

  for (int i = 0; i < m_nView; i++) {
    if (! detarrayRead (fileRead, *m_projData[i], i))
      break;
  }

  fileRead.close();
  return true;
}


bool
Projections::copyViewData (const std::string& filename, std::ostream& os, int startView, int endView)
{
  return copyViewData (filename.c_str(), os, startView, endView);
}

bool
Projections::copyViewData (const char* const filename, std::ostream& os, int startView, int endView)
{
  frnetorderstream is (filename, std::ios::in | std::ios::binary);
  kuint16 sizeHeader, signature;
  kuint32 _nView, _nDet;

  is.seekg (0);
  if (is.fail()) {
    sys_error (ERR_SEVERE, "Unable to read projection file %s", filename);
    return false;
  }

  is.readInt16 (sizeHeader);
  is.readInt16 (signature);
  is.readInt32 (_nView);
  is.readInt32 (_nDet);
  int nView = _nView;
  int nDet = _nDet;

  if (signature != m_signature) {
    sys_error (ERR_SEVERE, "Illegal signature in projection file %s", filename);
    return false;
  }

  if (startView < 0)
    startView = 0;
  if (startView > nView - 1)
    startView = nView;
  if (endView < 0 || endView > nView - 1)
    endView = nView - 1;

  if (startView > endView) { // swap if start > end
    int tempView = endView;
    endView = startView;
    startView = tempView;
  }

  int sizeView = 8 /* view_angle */ + 4 /* nDet */ + (4 * nDet);
  unsigned char* pViewData = new unsigned char [sizeView];

  for (int i = startView; i <= endView; i++) {
    is.seekg (sizeHeader + i * sizeView);
    is.read (reinterpret_cast<char*>(pViewData), sizeView);
    os.write (reinterpret_cast<char*>(pViewData), sizeView);
    if (is.fail() || os.fail())
      break;
  }

  delete pViewData;
  if (is.fail())
    sys_error (ERR_SEVERE, "Error reading projection file");
  if (os.fail())
    sys_error (ERR_SEVERE, "Error writing projection file");

  return (! (is.fail() | os.fail()));
}

bool
Projections::copyHeader (const std::string& filename, std::ostream& os)
{
  return copyHeader (filename.c_str(), os);
}

bool
Projections::copyHeader (const char* const filename, std::ostream& os)
{
  frnetorderstream is (filename, std::ios::in | std::ios::binary);
  kuint16 sizeHeader, signature;
  is.readInt16 (sizeHeader);
  is.readInt16 (signature);
  is.seekg (0);
  if (signature != m_signature) {
    sys_error (ERR_SEVERE, "Illegal signature in projection file %s", filename);
    return false;
  }

  unsigned char* pHdrData = new unsigned char [sizeHeader];
  is.read (reinterpret_cast<char*>(pHdrData), sizeHeader);
  if (is.fail()) {
    sys_error (ERR_SEVERE, "Error reading header");
    return false;
  }

  os.write (reinterpret_cast<char*>(pHdrData), sizeHeader);
  if (os.fail()) {
    sys_error (ERR_SEVERE, "Error writing header");
    return false;
  }

  return true;
}

bool
Projections::write (const std::string& filename)
{
  return write (filename.c_str());
}

bool
Projections::write (const char* filename)
{
  frnetorderstream fs (filename, std::ios::out | std::ios::binary | std::ios::trunc | std::ios::ate);
  m_filename = filename;
  if (! fs) {
    sys_error (ERR_SEVERE, "Error opening file %s for output [projections_create]", filename);
    return false;
  }

  if (! headerWrite (fs))
    return false;

  if (m_projData != NULL) {
    for (int i = 0; i < m_nView; i++) {
      if (! detarrayWrite (fs, *m_projData[i], i))
        break;
    }
  }
  if (! fs)
    return false;

  fs.close();

  return true;
}

/* NAME
*   detarrayRead                Read a Detector Array structure from the disk
*
* SYNOPSIS
*   detarrayRead (proj, darray, view_num)
*   DETARRAY *darray            Detector array storage location to be filled
*   int      view_num           View number to read
*/

bool
Projections::detarrayRead (fnetorderstream& fs, DetectorArray& darray, const int iview)
{
  const int detval_bytes = darray.nDet() * sizeof(kfloat32);
  const int detheader_bytes = sizeof(kfloat64) /* view_angle */ + sizeof(kint32) /* nDet */;
  const int view_bytes = detheader_bytes + detval_bytes;
  const off_t start_data = m_headerSize + (iview * view_bytes);
  DetectorValue* detval_ptr = darray.detValues();
  kfloat64 view_angle;
  kuint32 nDet;

  fs.seekg (start_data);

  fs.readFloat64 (view_angle);
  fs.readInt32 (nDet);
  darray.setViewAngle (view_angle);
  //  darray.setNDet ( nDet);

  for (unsigned int i = 0; i < nDet; i++) {
    kfloat32 detval;
    fs.readFloat32 (detval);
    detval_ptr[i] = detval;
  }
  if (! fs)
    return false;

  return true;
}


/* NAME
*   detarrayWrite                       Write detector array data to the disk
*
* SYNOPSIS
*   detarrayWrite (darray, view_num)
*   DETARRAY *darray                    Detector array data to be written
*   int      view_num                   View number to write
*
* DESCRIPTION
*       This routine writes the detarray data from the disk sequentially to
*    the file that was opened with open_projections().  Data is written in
*    binary format.
*/

bool
Projections::detarrayWrite (fnetorderstream& fs, const DetectorArray& darray, const int iview)
{
  const int detval_bytes = darray.nDet() * sizeof(float);
  const int detheader_bytes = sizeof(kfloat64) /* view_angle */ + sizeof(kint32) /* nDet */;
  const int view_bytes = detheader_bytes + detval_bytes;
  const off_t start_data = m_headerSize + (iview * view_bytes);
  const DetectorValue* const detval_ptr = darray.detValues();
  kfloat64 view_angle = darray.viewAngle();
  kuint32 nDet = darray.nDet();

  fs.seekp (start_data);
  if (! fs) {
    sys_error (ERR_SEVERE, "Error seeking detectory array [detarrayWrite]");
    return false;
  }

  fs.writeFloat64 (view_angle);
  fs.writeInt32 (nDet);

  for (unsigned int i = 0; i < nDet; i++) {
    kfloat32 detval = detval_ptr[i];
    fs.writeFloat32 (detval);
  }

  if (! fs)
    return (false);

  return true;
}

/* NAME
*   printProjectionData                 Print projections data
*
* SYNOPSIS
*   printProjectionData ()
*/

void
Projections::printProjectionData ()
{
  printProjectionData (0, nView() - 1);
}

void
Projections::printProjectionData (int startView, int endView)
{
  printf("Projections Data\n\n");
  printf("Description: %s\n", m_remark.c_str());
  printf("Geometry: %s\n", Scanner::convertGeometryIDToName (m_geometry));
  printf("nView       = %8d             nDet = %8d\n", m_nView, m_nDet);
  printf("focalLength = %8.4f   ViewDiameter = %8.4f\n", m_dFocalLength, m_dViewDiameter);
  printf("fanBeamAngle= %8.4f SourceDetector = %8.4f\n", convertRadiansToDegrees(m_dFanBeamAngle), m_dSourceDetectorLength);
  printf("rotStart    = %8.4f         rotInc = %8.4f\n", m_rotStart, m_rotInc);
  printf("detStart    = %8.4f         detInc = %8.4f\n", m_detStart, m_detInc);
  if (m_projData != NULL) {
    if (startView < 0)
      startView = 0;
    if (endView < 0)
      endView = m_nView - 1;
    if (startView > m_nView - 1)
      startView = m_nView - 1;
    if (endView > m_nView - 1)
      endView = m_nView - 1;
    for (int ir = startView; ir <= endView - 1; ir++) {
      printf("View %d: angle %f\n", ir, m_projData[ir]->viewAngle());
      DetectorValue* detval = m_projData[ir]->detValues();
      for (int id = 0; id < m_projData[ir]->nDet(); id++)
        printf("%8.4f  ", detval[id]);
      printf("\n");
    }
  }
}

void
Projections::printScanInfo (std::ostringstream& os) const
{
  os << "Number of detectors: " << m_nDet << "\n";
  os << "Number of views: " << m_nView<< "\n";
  os << "Description: " << m_remark.c_str()<< "\n";
  os << "Geometry: " << Scanner::convertGeometryIDToName (m_geometry)<< "\n";
  os << "Focal Length: " << m_dFocalLength<< "\n";
  os << "Source Detector Length: " << m_dSourceDetectorLength << "\n";
  os << "View Diameter: " << m_dViewDiameter<< "\n";
  os << "Fan Beam Angle: " << convertRadiansToDegrees(m_dFanBeamAngle) << "\n";
  os << "detStart: " << m_detStart<< "\n";
  os << "detInc: " << m_detInc<< "\n";
  os << "rotStart: " << m_rotStart<< "\n";
  os << "rotInc: " << m_rotInc<< "\n";
}


bool
Projections::convertPolar (ImageFile& rIF, int iInterpolationID)
{
  unsigned int nx = rIF.nx();
  unsigned int ny = rIF.ny();
  ImageFileArray v = rIF.getArray();
  ImageFileArray vImag = rIF.getImaginaryArray();

  if (! v || nx == 0 || ny == 0)
    return false;

  Projections* pProj = this;
  if (m_geometry == Scanner::GEOMETRY_EQUIANGULAR || m_geometry == Scanner::GEOMETRY_EQUILINEAR)
    pProj = interpolateToParallel();

  Array2d<double> adView (nx, ny);
  Array2d<double> adDet (nx, ny);
  double** ppdView = adView.getArray();
  double** ppdDet = adDet.getArray();

  std::complex<double>** ppcDetValue = new std::complex<double>* [pProj->m_nView];
  int iView;
  for (iView = 0; iView < pProj->m_nView; iView++) {
    ppcDetValue[iView] = new std::complex<double> [pProj->m_nDet];
    DetectorValue* detval = pProj->getDetectorArray (iView).detValues();
    for (int iDet = 0; iDet < pProj->m_nDet; iDet++)
      ppcDetValue[iView][iDet] = std::complex<double>(detval[iDet], 0);
  }

  pProj->calcArrayPolarCoordinates (nx, ny, ppdView, ppdDet, pProj->m_nDet, 1., pProj->m_detInc);

  pProj->interpolatePolar (v, vImag, nx, ny, ppcDetValue, ppdView, ppdDet, pProj->m_nView, pProj->m_nDet,
    pProj->m_nDet, iInterpolationID);

  for (iView = 0; iView < pProj->m_nView; iView++)
    delete [] ppcDetValue[iView];
  delete [] ppcDetValue;

  if (m_geometry == Scanner::GEOMETRY_EQUIANGULAR || m_geometry == Scanner::GEOMETRY_EQUILINEAR)
    delete pProj;

  return true;
}


bool
Projections::convertFFTPolar (ImageFile& rIF, int iInterpolationID, int iZeropad)
{
#ifndef HAVE_FFTW
  rIF.arrayDataClear();
  return false;
#else
  unsigned int nx = rIF.nx();
  unsigned int ny = rIF.ny();
  ImageFileArray v = rIF.getArray();
  if (! rIF.isComplex())
    rIF.convertRealToComplex();
  ImageFileArray vImag = rIF.getImaginaryArray();

  if (! v || nx == 0 || ny == 0)
    return false;

  Projections* pProj = this;
  if (m_geometry == Scanner::GEOMETRY_EQUIANGULAR || m_geometry == Scanner::GEOMETRY_EQUILINEAR)
    pProj = interpolateToParallel();

  int iInterpDet = static_cast<int>(static_cast<double>(sqrt(nx*nx+ny*ny)));
  int iNumInterpDetWithZeros = ProcessSignal::addZeropadFactor (iInterpDet, iZeropad);
  double dProjScale = iInterpDet / (pProj->viewDiameter() * 0.05);
  double dZeropadRatio = static_cast<double>(iNumInterpDetWithZeros) / static_cast<double>(iInterpDet);

  fftw_complex* pcIn = static_cast<fftw_complex*> (fftw_malloc (sizeof(fftw_complex) * iNumInterpDetWithZeros));
  fftw_plan plan = fftw_plan_dft_1d (iNumInterpDetWithZeros, pcIn, pcIn, FFTW_FORWARD, FFTW_ESTIMATE);

  std::complex<double>** ppcDetValue = new std::complex<double>* [pProj->m_nView];
  //double dInterpScale = (pProj->m_nDet-1) / static_cast<double>(iInterpDet-1);
  double dInterpScale = pProj->m_nDet / static_cast<double>(iInterpDet);

  double dFFTScale = 1. / static_cast<double>(iInterpDet * iInterpDet);
  int iMidPoint = iInterpDet / 2;
  double dMidPoint = static_cast<double>(iInterpDet) / 2.;
  int iZerosAdded = iNumInterpDetWithZeros - iInterpDet;

  // For each view, interpolate, shift to center at origin, and FFT
  for (int iView = 0; iView < m_nView; iView++) {
    DetectorValue* detval = pProj->getDetectorArray(iView).detValues();
    LinearInterpolator<DetectorValue> projInterp (detval, pProj->m_nDet);
    for (int iDet = 0; iDet < iInterpDet; iDet++) {
      double dInterpPos = (m_nDet / 2.) + (iDet - dMidPoint) * dInterpScale;
      pcIn[iDet][0] = projInterp.interpolate (dInterpPos) * dProjScale;
      pcIn[iDet][1] = 0;
    }

    Fourier::shuffleFourierToNaturalOrder (pcIn, iInterpDet);
    if (iZerosAdded > 0) {
      for (int iDet1 = iInterpDet -1; iDet1 >= iMidPoint; iDet1--) {
        pcIn[iDet1+iZerosAdded][0] = pcIn[iDet1][0];
        pcIn[iDet1+iZerosAdded][1] = pcIn[iDet1][1];
      }
      for (int iDet2 = iMidPoint; iDet2 < iMidPoint + iZerosAdded; iDet2++)
        pcIn[iDet2][0] = pcIn[iDet2][1] = 0;
    }

    fftw_execute (plan);

    ppcDetValue[iView] = new std::complex<double> [iNumInterpDetWithZeros];
    for (int iD = 0; iD < iNumInterpDetWithZeros; iD++) {
      ppcDetValue[iView][iD] = std::complex<double> (pcIn[iD][0] * dFFTScale, pcIn[iD][1] * dFFTScale);
    }

    Fourier::shuffleFourierToNaturalOrder (ppcDetValue[iView], iNumInterpDetWithZeros);
  }
  fftw_free(pcIn) ;

  fftw_destroy_plan (plan);

  Array2d<double> adView (nx, ny);
  Array2d<double> adDet (nx, ny);
  double** ppdView = adView.getArray();
  double** ppdDet = adDet.getArray();
  pProj->calcArrayPolarCoordinates (nx, ny, ppdView, ppdDet, iNumInterpDetWithZeros, dZeropadRatio,
    pProj->m_detInc * dInterpScale);

  pProj->interpolatePolar (v, vImag, nx, ny, ppcDetValue, ppdView, ppdDet, pProj->m_nView, pProj->m_nDet,
    iNumInterpDetWithZeros, iInterpolationID);

  if (m_geometry == Scanner::GEOMETRY_EQUIANGULAR || m_geometry == Scanner::GEOMETRY_EQUILINEAR)
    delete pProj;

  for (int i = 0; i < m_nView; i++)
    delete [] ppcDetValue[i];
  delete [] ppcDetValue;

  return true;
#endif
}


void
Projections::calcArrayPolarCoordinates (unsigned int nx, unsigned int ny, double** ppdView, double** ppdDet,
                                        int iNumDetWithZeros, double dZeropadRatio, double dDetInc)
{
  double dLength = viewDiameter();
  double xMin = -dLength / 2;
  double xMax = xMin + dLength;
  double yMin = -dLength / 2;
  double yMax = yMin + dLength;
  double xCent = (xMin + xMax) / 2;
  double yCent = (yMin + yMax) / 2;

  xMin = (xMin - xCent) * dZeropadRatio + xCent;
  xMax = (xMax - xCent) * dZeropadRatio + xCent;
  yMin = (yMin - yCent) * dZeropadRatio + yCent;
  yMax = (yMax - yCent) * dZeropadRatio + yCent;

  double xInc = (xMax - xMin) / nx;     // size of cells
  double yInc = (yMax - yMin) / ny;

  double dDetCenter = (iNumDetWithZeros - 1) / 2.;      // index refering to L=0 projection
  // +1 is correct for frequency data, ndet-1 is correct for projections
  //  if (isEven (iNumDetWithZeros))
  //    dDetCenter = (iNumDetWithZeros + 0) / 2;

  // Calculates polar coordinates (view#, det#) for each point on phantom grid
  double x = xMin + xInc / 2;   // Rectang coords of center of pixel
  for (unsigned int ix = 0; ix < nx; x += xInc, ix++) {
    double y = yMin + yInc / 2;
    for (unsigned int iy = 0; iy < ny; y += yInc, iy++) {
      double r = ::sqrt (x * x + y * y);
      double phi = atan2 (y, x);

      if (phi <= -m_rotInc / 2)
        phi += TWOPI;
      if (phi >= PI - (m_rotInc / 2)) {
        phi -= PI;
        r = -r;
      }

      ppdView[ix][iy] = (phi - m_rotStart) / m_rotInc;
      ppdDet[ix][iy] = (r / dDetInc) + dDetCenter;
    }
  }
}

void
Projections::interpolatePolar (ImageFileArray& v, ImageFileArray& vImag,
     unsigned int nx, unsigned int ny, std::complex<double>** ppcDetValue, double** ppdView,
     double** ppdDet, unsigned int nView, unsigned int nDet, unsigned int nDetWithZeros, int iInterpolationID)
{
  typedef std::complex<double> complexValue;

  BilinearPolarInterpolator<complexValue>* pBilinear = NULL;
  BicubicPolyInterpolator<complexValue>* pBicubic = NULL;
  if (iInterpolationID == POLAR_INTERP_BILINEAR)
    pBilinear = new BilinearPolarInterpolator<complexValue> (ppcDetValue, nView, nDetWithZeros);
  else if (iInterpolationID == POLAR_INTERP_BICUBIC)
    pBicubic = new BicubicPolyInterpolator<complexValue> (ppcDetValue, nView, nDetWithZeros);

  for (unsigned int ix = 0; ix < ny; ix++) {
    for (unsigned int iy = 0; iy < ny; iy++) {
      if (iInterpolationID == POLAR_INTERP_NEAREST) {
        unsigned int iView = nearest<int> (ppdView[ix][iy]);
        unsigned int iDet = nearest<int> (ppdDet[ix][iy]);
        if (iView == nView)
          iView = 0;
        if (iDet >= 0 && iDet < nDetWithZeros && iView >= 0 && iView < nView) {
          v[ix][iy] = ppcDetValue[iView][iDet].real();
          if (vImag)
            vImag[ix][iy] = ppcDetValue[iView][iDet].imag();
        } else {
          v[ix][iy] = 0;
          if (vImag)
            vImag[ix][iy] = 0;
        }

      } else if (iInterpolationID == POLAR_INTERP_BILINEAR) {
        std::complex<double> vInterp = pBilinear->interpolate (ppdView[ix][iy], ppdDet[ix][iy]);
        v[ix][iy] = vInterp.real();
        if (vImag)
          vImag[ix][iy] = vInterp.imag();
      } else if (iInterpolationID == POLAR_INTERP_BICUBIC) {
        std::complex<double> vInterp = pBicubic->interpolate (ppdView[ix][iy], ppdDet[ix][iy]);
        v[ix][iy] = vInterp.real();
        if (vImag)
          vImag[ix][iy] = vInterp.imag();
      }
    }
  }
}

bool
Projections::initFromSomatomAR_STAR (int iNViews, int iNDets, unsigned char* pData, unsigned long lDataLength)
{
  init (iNViews, iNDets);
  m_geometry = Scanner::GEOMETRY_EQUIANGULAR;
  m_dFocalLength = 510;
  m_dSourceDetectorLength = 890;
  m_detInc = convertDegreesToRadians (3.06976 / 60);
  m_dFanBeamAngle = iNDets * m_detInc;
  m_detStart = -(m_dFanBeamAngle / 2);
  m_rotInc = TWOPI / static_cast<double>(iNViews);
  m_rotStart = 0;
  m_dViewDiameter = sin (m_dFanBeamAngle / 2) * m_dFocalLength * 2;

  if (! ((iNViews == 750 && lDataLength == 1560000L) || (iNViews == 950 && lDataLength == 1976000L)
                || (iNViews == 1500 && lDataLength == 3120000)))
    return false;

  double dCenter = (iNDets - 1.) / 2.; // change from (Nm+1)/2 because of 0 vs. 1 indexing
  double* pdCosScale = new double [iNDets];
  for (int i = 0; i < iNDets; i++)
    pdCosScale[i] = 1. / cos ((i - dCenter) * m_detInc);

  long lDataPos = 0;
  for (int iv = 0; iv < iNViews; iv++) {
    unsigned char* pArgBase = pData + lDataPos;
    unsigned char* p = pArgBase+0; SwapBytes4IfLittleEndian (p);
    // long lProjNumber = *reinterpret_cast<long*>(p);

    p = pArgBase+20;  SwapBytes4IfLittleEndian (p);
    long lEscale = *reinterpret_cast<long*>(p);

    p = pArgBase+28;  SwapBytes4IfLittleEndian (p);
    // long lTime = *reinterpret_cast<long*>(p);

    p = pArgBase + 4; SwapBytes4IfLittleEndian (p);
    double dAlpha = *reinterpret_cast<float*>(p) + HALFPI;

    p = pArgBase+12; SwapBytes4IfLittleEndian (p);
    // double dAlign = *reinterpret_cast<float*>(p);

    p = pArgBase + 16; SwapBytes4IfLittleEndian (p);
    // double dMaxValue = *reinterpret_cast<float*>(p);

    DetectorArray& detArray = getDetectorArray (iv);
    detArray.setViewAngle (dAlpha);
    DetectorValue* detval = detArray.detValues();

    double dViewScale = 1. / (2294.4871 * ::pow (2.0, -lEscale));
    lDataPos += 32;
    for (int id = 0; id < iNDets; id++) {
      int iV = pData[lDataPos+1] + (pData[lDataPos] << 8);
      if (iV > 32767)   // two's complement signed conversion
        iV = iV - 65536;
      detval[id] = iV * dViewScale * pdCosScale[id];
      lDataPos += 2;
    }
#if 1
    for (int k = iNDets - 2; k >= 0; k--)
      detval[k+1] = detval[k];
    detval[0] = 0;
#endif
  }

  delete pdCosScale;
  return true;
}

Projections*
Projections::interpolateToParallel () const
{
  if (m_geometry == Scanner::GEOMETRY_PARALLEL)
    return const_cast<Projections*>(this);

  int nDet = m_nDet;
  int nView = m_nView;
  Projections* pProjNew = new Projections (nView, nDet);
  pProjNew->m_geometry = Scanner::GEOMETRY_PARALLEL;
  pProjNew->m_dFocalLength = m_dFocalLength;
  pProjNew->m_dSourceDetectorLength = m_dSourceDetectorLength;
  pProjNew->m_dViewDiameter = m_dViewDiameter;
  pProjNew->m_dFanBeamAngle = m_dFanBeamAngle;
  pProjNew->m_calcTime  = 0;
  pProjNew->m_remark = m_remark;
  pProjNew->m_remark += "; Interpolate to Parallel";
  pProjNew->m_label.setLabelType (Array2dFileLabel::L_HISTORY);
  pProjNew->m_label.setLabelString (pProjNew->m_remark);
  pProjNew->m_label.setCalcTime (pProjNew->m_calcTime);
  pProjNew->m_label.setDateTime (pProjNew->m_year, pProjNew->m_month, pProjNew->m_day, pProjNew->m_hour, pProjNew->m_minute, pProjNew->m_second);

  pProjNew->m_rotStart = 0;
#ifdef CONVERT_PARALLEL_PI
  pProjNew->m_rotInc = PI / nView;;
#else
  pProjNew->m_rotInc = TWOPI / nView;
#endif
  pProjNew->m_detStart = -m_dViewDiameter / 2;
  pProjNew->m_detInc = m_dViewDiameter / nDet;
  if (isEven (nDet)) // even
    pProjNew->m_detInc = m_dViewDiameter / (nDet - 1);

  ParallelRaysums parallel (this, ParallelRaysums::THETA_RANGE_NORMALIZE_TO_TWOPI);

  double* pdThetaValuesForT = new double [pProjNew->nView()];
  double* pdRaysumsForT = new double [pProjNew->nView()];

  // interpolate to evenly spaced theta (views)
  double dDetPos = pProjNew->m_detStart;
  for (int iD = 0; iD < pProjNew->nDet(); iD++, dDetPos += pProjNew->m_detInc) {
      parallel.getThetaAndRaysumsForT (iD, pdThetaValuesForT, pdRaysumsForT);

    double dViewAngle = m_rotStart;
    int iLastFloor = -1;
    LinearInterpolator<double> interp (pdThetaValuesForT, pdRaysumsForT, pProjNew->nView(), false);
    for (int iV = 0; iV < pProjNew->nView(); iV++, dViewAngle += pProjNew->m_rotInc) {
      DetectorValue* detValues = pProjNew->getDetectorArray (iV).detValues();
      detValues[iD] = interp.interpolate (dViewAngle, &iLastFloor);
    }
  }
  delete pdThetaValuesForT;
  delete pdRaysumsForT;

  // interpolate to evenly space t (detectors)
  double* pdOriginalDetPositions = new double [pProjNew->nDet()];
  parallel.getDetPositions (pdOriginalDetPositions);

  double* pdDetValueCopy = new double [pProjNew->nDet()];
  double dViewAngle = m_rotStart;
  for (int iV = 0; iV < pProjNew->nView(); iV++, dViewAngle += pProjNew->m_rotInc) {
    DetectorArray& detArray = pProjNew->getDetectorArray (iV);
    DetectorValue* detValues = detArray.detValues();
    detArray.setViewAngle (dViewAngle);

    for (int i = 0; i < pProjNew->nDet(); i++)
      pdDetValueCopy[i] =   detValues[i];

    double dDetPos = pProjNew->m_detStart;
    int iLastFloor = -1;
    LinearInterpolator<double> interp (pdOriginalDetPositions, pdDetValueCopy, pProjNew->nDet(), false);
    for (int iD = 0; iD < pProjNew->nDet(); iD++, dDetPos += pProjNew->m_detInc)
      detValues[iD] = interp.interpolate (dDetPos, &iLastFloor);
  }
  delete pdDetValueCopy;
  delete pdOriginalDetPositions;

  return pProjNew;
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ParallelRaysums
//
// Used for converting divergent beam raysums into Parallel raysums
//
///////////////////////////////////////////////////////////////////////////////

ParallelRaysums::ParallelRaysums (const Projections* pProjections, int iThetaRange)
: m_pCoordinates(NULL), m_iNumCoordinates(0), m_iNumView(pProjections->nView()), m_iNumDet(pProjections->nDet()),
  m_iThetaRange (iThetaRange)
{
  int iGeometry = pProjections->geometry();
  double dDetInc = pProjections->detInc();
  double dDetStart = pProjections->detStart();
  double dFocalLength = pProjections->focalLength();

  m_iNumCoordinates =  m_iNumView * m_iNumDet;
  m_pCoordinates = new ParallelRaysumCoordinate [m_iNumCoordinates];
  m_vecpCoordinates.reserve (m_iNumCoordinates);
  for (int i = 0; i < m_iNumCoordinates; i++)
    m_vecpCoordinates[i] = m_pCoordinates + i;

  int iCoordinate = 0;
  for (int iV = 0; iV < m_iNumView; iV++) {
    double dViewAngle = pProjections->getDetectorArray(iV).viewAngle();
    const DetectorValue* detValues = pProjections->getDetectorArray(iV).detValues();

    double dDetPos = dDetStart;
    for (int iD = 0; iD < m_iNumDet; iD++) {
      ParallelRaysumCoordinate* pC = m_vecpCoordinates[iCoordinate++];

      if (iGeometry == Scanner::GEOMETRY_PARALLEL) {
        pC->m_dTheta = dViewAngle;
        pC->m_dT = dDetPos;
      } else if (iGeometry == Scanner::GEOMETRY_EQUILINEAR) {
        double dFanAngle = atan (dDetPos / pProjections->sourceDetectorLength());
        pC->m_dTheta = dViewAngle + dFanAngle;
        pC->m_dT = dFocalLength * sin(dFanAngle);

      } else if (iGeometry == Scanner::GEOMETRY_EQUIANGULAR) {
        // fan angle is same as dDetPos
        pC->m_dTheta = dViewAngle + dDetPos;
        pC->m_dT = dFocalLength * sin (dDetPos);
      }
      if (m_iThetaRange != THETA_RANGE_UNCONSTRAINED) {
        pC->m_dTheta = normalizeAngle (pC->m_dTheta);
        if (m_iThetaRange == THETA_RANGE_FOLD_TO_PI && pC->m_dTheta >= PI) {
          pC->m_dTheta -= PI;
          pC->m_dT = -pC->m_dT;
        }
      }
      pC->m_dRaysum = detValues[iD];
      dDetPos += dDetInc;
    }
  }
}

ParallelRaysums::~ParallelRaysums()
{
  delete m_pCoordinates;
}

ParallelRaysums::CoordinateContainer&
ParallelRaysums::getSortedByTheta()
{
  if (m_vecpSortedByTheta.size() == 0) {
    m_vecpSortedByTheta.resize (m_iNumCoordinates);
    for (int i = 0; i < m_iNumCoordinates; i++)
      m_vecpSortedByTheta[i] = m_vecpCoordinates[i];
    std::sort (m_vecpSortedByTheta.begin(), m_vecpSortedByTheta.end(), ParallelRaysumCoordinate::compareByTheta);
  }

  return m_vecpSortedByTheta;
}

ParallelRaysums::CoordinateContainer&
ParallelRaysums::getSortedByT()
{
  if (m_vecpSortedByT.size() == 0) {
    m_vecpSortedByT.resize (m_iNumCoordinates);
    for (int i = 0; i < m_iNumCoordinates; i++)
      m_vecpSortedByT[i] = m_vecpCoordinates[i];
    std::sort (m_vecpSortedByT.begin(), m_vecpSortedByT.end(), ParallelRaysumCoordinate::compareByT);
  }

  return m_vecpSortedByT;
}


void
ParallelRaysums::getLimits (double* dMinT, double* dMaxT, double* dMinTheta, double* dMaxTheta) const
{
  if (m_iNumCoordinates <= 0)
    return;

  *dMinT = *dMaxT = m_vecpCoordinates[0]->m_dT;
  *dMinTheta = *dMaxTheta = m_vecpCoordinates[0]->m_dTheta;

  for (int i = 0; i < m_iNumCoordinates; i++) {
    double dT = m_vecpCoordinates[i]->m_dT;
    double dTheta = m_vecpCoordinates[i]->m_dTheta;

    if (dT < *dMinT)
      *dMinT = dT;
    else if (dT > *dMaxT)
      *dMaxT = dT;

    if (dTheta < *dMinTheta)
      *dMinTheta = dTheta;
    else if (dTheta > *dMaxTheta)
      *dMaxTheta = dTheta;
  }
}

void
ParallelRaysums::getThetaAndRaysumsForT (int iTheta, double* pTheta, double* pRaysum)
{
  const CoordinateContainer& coordsT = getSortedByT();

  int iBase = iTheta * m_iNumView;
  for (int i = 0; i < m_iNumView; i++) {
    int iPos = iBase + i;
    pTheta[i] = coordsT[iPos]->m_dTheta;
    pRaysum[i] = coordsT[iPos]->m_dRaysum;
  }
}

void
ParallelRaysums::getDetPositions (double* pdDetPos)
{
  const CoordinateContainer& coordsT = getSortedByT();

  int iPos = 0;
  for (int i = 0; i < m_iNumDet; i++) {
    pdDetPos[i] = coordsT[iPos]->m_dT;
    iPos += m_iNumView;
  }
}
