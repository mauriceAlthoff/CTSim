/*****************************************************************************
** FILE IDENTIFICATION
**
**     Name:                   phm.cpp
**     Purpose:                Routines for phantom objects
**     Progammer:              Kevin Rosenberg
**     Date Started:           Aug 1984
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

const int PhantomElement::POINTS_PER_CIRCLE = 360;
const double PhantomElement::SCALE_PELEM_EXTENT=0.000;  // increase pelem limits by 0.5%
//const double PhantomElement::SCALE_PELEM_EXTENT=0.005;  // increase pelem limits by 0.5%

const int Phantom::PHM_INVALID = -1;
const int Phantom::PHM_HERMAN = 0;
const int Phantom::PHM_SHEPP_LOGAN = 1;
const int Phantom::PHM_UNITPULSE = 2;

const char* Phantom::s_aszPhantomName[] =
{
  "herman",
  "shepp-logan",
  "unit-pulse",
};

const char* Phantom::s_aszPhantomTitle[] =
{
  "Herman Head",
  "Shepp-Logan",
  "Unit Pulse",
};

const int Phantom::s_iPhantomCount = sizeof(s_aszPhantomName) / sizeof(const char*);


// CLASS IDENTIFICATION
//   Phantom
//

Phantom::Phantom ()
{
  init ();
}


Phantom::Phantom (const char* const phmName)
{
  init ();
  createFromPhantom (phmName);
}

void
Phantom::init ()
{
  m_nPElem = 0;
  m_xmin = 1E30;
  m_xmax = -1E30;
  m_ymin = 1E30;
  m_ymax = -1E30;
  m_composition = P_PELEMS;
  m_fail = false;
  m_id = PHM_INVALID;
}

Phantom::~Phantom ()
{
  for (PElemIterator i = m_listPElem.begin(); i != m_listPElem.end(); i++) {
    delete *i;
  }
}


const char*
Phantom::convertPhantomIDToName (int phmID)
{
  static const char *name = "";

  if (phmID >= 0 && phmID < s_iPhantomCount)
    return (s_aszPhantomName[phmID]);

  return (name);
}

const char*
Phantom::convertPhantomIDToTitle (int phmID)
{
  static const char *title = "";

  if (phmID >= 0 && phmID < s_iPhantomCount)
    return (s_aszPhantomName[phmID]);

  return (title);
}

int
Phantom::convertNameToPhantomID (const char* const phmName)
{
  int id = PHM_INVALID;

  for (int i = 0; i < s_iPhantomCount; i++)
    if (strcasecmp (phmName, s_aszPhantomName[i]) == 0) {
      id = i;
      break;
    }

    return (id);
}


bool
Phantom::createFromPhantom (const char* const phmName)
{
  int phmid = convertNameToPhantomID (phmName);
  if (phmid == PHM_INVALID) {
    m_fail = true;
    m_failMessage = "Invalid phantom name ";
    m_failMessage += phmName;
    return false;
  }

  m_name = phmName;
  createFromPhantom (phmid);
  return true;
}

bool
Phantom::createFromPhantom (const int phmid)
{
  switch (phmid)
  {
  case PHM_HERMAN:
    addStdHerman();
    break;
  case PHM_SHEPP_LOGAN:
    addStdSheppLogan();
    break;
  case PHM_UNITPULSE:
    m_composition = P_UNIT_PULSE;
    addPElem ("rectangle", 0., 0., 100., 100., 0., 0.);     // outline
    addPElem ("ellipse", 0., 0., 1., 1., 0., 1.);             // pulse
    break;
  default:
    m_fail = true;
    m_failMessage = "Illegal phantom id ";
    m_failMessage += phmid;
    return false;
  }

  m_id = phmid;

  return true;
}


/* METHOD IDENTIFICATION
*   createFromFile          Add PhantomElements from file
*
* SYNOPSIS
*   createFromFile (filename)
*
* RETURNS
*   true if pelem were added
*   false if an pelem not added
*/

bool
Phantom::createFromFile (const char* const fname)
{
  bool bGoodFile = true;
  FILE *fp;

  if ((fp = fopen (fname, "r")) == NULL)
    return (false);

  m_name = fname;

  while (1) {
    double cx, cy, u, v, rot, dens;
    char pelemtype[80];

    int status = fscanf (fp, "%79s %lf %lf %lf %lf %lf %lf", pelemtype, &cx, &cy, &u, &v, &rot, &dens);

    if (status == static_cast<int>(EOF))
      break;
    else if (status != 7) {
      sys_error (ERR_WARNING, "Insufficient fields reading phantom file %s [Phantom::createFromFile]", fname);
      bGoodFile = false;
    }
    addPElem (pelemtype, cx, cy, u, v, rot, dens);
  }

  fclose (fp);

  return (bGoodFile);
}

bool
Phantom::fileWrite (const char* const fname)
{
  fstream file (fname, std::ios::out);

  if (! file.fail())
    printDefinitions (file);
  return ! file.fail();
}

/* NAME
*   addPElem            Add pelem
*
* SYNOPSIS
*   addPElem (type, cx, cy, u, v, rot, atten)
*   char *type          type of pelem (box, ellipse, etc)
*   double cx, cy       pelem center
*   double u,v          pelem size
*   double rot          rotation angle of pelem (in degrees)
*   double atten        x-ray attenuation cooefficient
*/

void
Phantom::addPElem (const char *type, const double cx, const double cy, const double u, const double v, const double rot, const double atten)
{
  PhmElemType pe_type = PhantomElement::convertNameToType (type);
  if (pe_type == PELEM_INVALID) {
    sys_error (ERR_WARNING, "Unknown PhantomElement type %s [PhantomElement::PhantomElement]", type);
    return;
  }

  PhantomElement *pelem = new PhantomElement (type, cx, cy, u, v, rot, atten);
  m_listPElem.push_front (pelem);

  // update phantom limits
  if (m_xmin > pelem->xmin())    m_xmin = pelem->xmin();
  if (m_xmax < pelem->xmax())    m_xmax = pelem->xmax();
  if (m_ymin > pelem->ymin())    m_ymin = pelem->ymin();
  if (m_ymax < pelem->ymax())    m_ymax = pelem->ymax();

  m_nPElem++;
}


/*----------------------------------------------------------------------*/
/*                      Input-Output Routines                           */
/*----------------------------------------------------------------------*/


/* NAME
*   print                               Print vertices of Phantom pelems
*
* SYNOPSIS
*   print (phm)
*/

void
Phantom::print (std::ostream& os) const
{
  os << "Number of PElements: " << m_nPElem << "\n";
  os << "Limits: xmin=" << m_xmin << ", ymin=" << m_ymin << ", xmax=" << m_xmax << ", ymax=" << m_ymax << "\n";

  for (PElemConstIterator i = m_listPElem.begin(); i != m_listPElem.end(); i++) {
    const PhantomElement& rPE = **i;
    os << "PhantomElement: nPoints=" << rPE.nOutlinePoints();
    os << ", atten=" << rPE.atten() << " rot=" << convertRadiansToDegrees (rPE.rot()) << "\n";
    os << "xmin=" << rPE.xmin() << ", ymin=" << rPE.ymin() << ", xmax=" << rPE.xmax() << ", ymax=" << rPE.ymax() << "\n";

    if (false)
      for (int i = 0; i < rPE.nOutlinePoints(); i++)
        os << rPE.xOutline()[i] << "," << rPE.yOutline()[i] << "\n";
  }
}
void
Phantom::print (std::ostringstream& os) const
{
  os << "Number of PElements: " << m_nPElem << "\n";
  os << "Limits: xmin=" << m_xmin << ", ymin=" << m_ymin << ", xmax=" << m_xmax << ", ymax=" << m_ymax << "\n";

  for (PElemConstIterator i = m_listPElem.begin(); i != m_listPElem.end(); i++) {
    const PhantomElement& rPE = **i;
    os << "PhantomElement: nPoints=" << rPE.nOutlinePoints();
    os << ", atten=" << rPE.atten() << " rot=" << convertRadiansToDegrees (rPE.rot()) << "\n";
    os << "xmin=" << rPE.xmin() << ", ymin=" << rPE.ymin() << ", xmax=" << rPE.xmax() << ", ymax=" << rPE.ymax() << "\n";

    if (false)
      for (int i = 0; i < rPE.nOutlinePoints(); i++)
        os << rPE.xOutline()[i] << "," << rPE.yOutline()[i] << "\n";
  }
}

void
Phantom::printDefinitions (std::ostream& os) const
{
  for (PElemConstIterator i = m_listPElem.begin(); i != m_listPElem.end(); i++) {
    const PhantomElement& rPE = **i;
    rPE.printDefinition (os);
  }
}

void
Phantom::printDefinitions (std::ostringstream& os) const
{
  for (PElemConstIterator i = m_listPElem.begin(); i != m_listPElem.end(); i++) {
    const PhantomElement& rPE = **i;
    rPE.printDefinition (os);
  }
}


/* NAME
*   show                Show vector outline of Phantom to user
*
* SYNOPSIS
*   show (pic)
*/

#ifdef HAVE_SGP
void
Phantom::show () const
{
  SGPDriver driverSGP ("Phantom Show");
  SGP sgp (driverSGP);

  show (sgp);

  std::cout << "Press return to continue";
  cio_kb_getc();
}

void
Phantom::show (SGP& sgp) const
{
  double wsize = m_xmax - m_xmin;
  if ((m_ymax - m_ymin) > wsize)
    wsize = m_ymax - m_ymin;
  wsize *= 1.01;
  double halfWindow = wsize / 2;

  double xcent = m_xmin + (m_xmax - m_xmin) / 2;
  double ycent = m_ymin + (m_ymax - m_ymin) / 2;

  sgp.setWindow (xcent - halfWindow, ycent - halfWindow, xcent + halfWindow, ycent + halfWindow);

  draw (sgp);
}
#endif


/* NAME
*   draw                Draw vector outline of Phantom
*
* SYNOPSIS
*   draw ()
*/

#ifdef HAVE_SGP
void
Phantom::draw (SGP& sgp) const
{
  for (PElemIterator i = m_listPElem.begin(); i != m_listPElem.end(); i++)
    sgp.polylineAbs ((*i)->xOutline(), (*i)->yOutline(), (*i)->nOutlinePoints());
}
#endif


/* NAME
*   addStdSheppLogan    Make head phantom of Shepp-Logan
*
* REFERENCES
*   S. W. Rowland, "Computer Implementation of Image Reconstruction
*       Formulas", in "Image Reconstruction from Projections: Implementation
*       and Applications", edited by G. T. Herman, 1978.
*/

void
Phantom::addStdSheppLogan ()
{
  addPElem ("ellipse",  0.0000,  0.0000, 0.6900,  0.9200,   0.0,  1.00);
  addPElem ("ellipse",  0.0000, -0.0184, 0.6624,  0.8740,   0.0, -0.98);
  addPElem ("ellipse",  0.2200,  0.0000, 0.1100,  0.3100, -18.0, -0.02);
  addPElem ("ellipse", -0.2200,  0.0000, 0.1600,  0.4100,  18.0, -0.02);
  addPElem ("ellipse",  0.0000,  0.3500, 0.2100,  0.2500,   0.0,  0.01);
  addPElem ("ellipse",  0.0000,  0.1000, 0.0460,  0.0460,   0.0,  0.01);
  addPElem ("ellipse",  0.0000, -0.1000, 0.0460,  0.0460,   0.0,  0.01);
  addPElem ("ellipse", -0.0800, -0.6050, 0.0460,  0.0230,   0.0,  0.01);
  addPElem ("ellipse",  0.0000, -0.6050, 0.0230,  0.0230,   0.0,  0.01);
  addPElem ("ellipse",  0.0600, -0.6050, 0.0230,  0.0230,   0.0,  0.01);
  addPElem ("ellipse",  0.5538, -0.3858, 0.0330,  0.2060, -18.0,  0.03);
}


/* NAME
*   addStdHerman                        Standard head phantom of G. T. Herman
*
* REFERENCES
*   G. T. Herman, "Image Reconstructions from Projections:  The Fundementals
*       of Computed Tomography", 1979.
*/

void
Phantom::addStdHerman ()
{
  addPElem ("ellipse",  0.000,  1.50,  0.375, 0.3000,  90.00, -0.003);
  addPElem ("ellipse",  0.675, -0.75,  0.225, 0.1500, 140.00,  0.010);
  addPElem ("ellipse",  0.750,  1.50,  0.375, 0.2250,  50.00,  0.003);
  addPElem ("segment",  1.375, -7.50,  1.100, 0.6250,  19.20, -0.204);
  addPElem ("segment",  1.375, -7.50,  1.100, 4.3200,  19.21,  0.204);
  addPElem ("segment",  0.000, -2.25,  1.125, 0.3750,   0.00, -0.003);
  addPElem ("segment",  0.000, -2.25,  1.125, 3.0000,   0.00,  0.003);
  addPElem ("segment", -1.000,  3.75,  1.000, 0.5000, 135.00, -0.003);
  addPElem ("segment", -1.000,  3.75,  1.000, 3.0000, 135.00,  0.003);
  addPElem ("segment",  1.000,  3.75,  1.000, 0.5000, 225.00, -0.003);
  addPElem ("segment",  1.000,  3.75,  1.000, 3.0000, 225.00,  0.003);
  addPElem ("triangle", 5.025,  3.75,  1.125, 0.5000, 110.75,  0.206);
  addPElem ("triangle",-5.025,  3.75,  1.125, 0.9000,-110.75,  0.206);
  addPElem ("ellipse",  0.000,  0.00,  8.625, 6.4687,  90.00,  0.416);
  addPElem ("ellipse",  0.000,  0.00,  7.875, 5.7187,  90.00, -0.206);
}



/* NAME
*    convertToImagefile         Make image array from Phantom
*
* SYNOPSIS
*    pic_to_imagefile (pic, im, nsample)
*    Phantom& pic               Phantom definitions
*    ImageFile  *im             Computed pixel array
*    int nsample                Number of samples along each axis for each pixel
*                               (total samples per pixel = nsample * nsample)
*/

void
Phantom::convertToImagefile (ImageFile& im, double dViewRatio, const int in_nsample, const int trace) const
{
  convertToImagefile (im, dViewRatio, in_nsample, trace, 0, im.nx(), true);
}

void
Phantom::convertToImagefile (ImageFile& im, const double dViewRatio, const int in_nsample, const int trace,
                             const int colStart, const int colCount, bool bStoreAtColumnPos) const
{
  int iStorageOffset = (bStoreAtColumnPos ? colStart : 0);
  convertToImagefile (im, im.nx(), dViewRatio, in_nsample, trace, colStart, colCount, iStorageOffset);
}

void
Phantom::convertToImagefile (ImageFile& im, const int iTotalRasterCols, const double dViewRatio,
            const int in_nsample, const int trace, const int colStart, const int colCount, int iStorageOffset) const
{
  const int nx = im.nx();
  const int ny = im.ny();
  if (nx < 2 || ny < 2)
    return;

  int nsample = in_nsample;
  if (nsample < 1)
    nsample = 1;

  double dx = m_xmax - m_xmin;
  double dy = m_ymax - m_ymin;
  double xcent = m_xmin + dx / 2;
  double ycent = m_ymin + dy / 2;
  double dHalflen = dViewRatio * (getDiameterBoundaryCircle() / SQRT2 / 2);

  double xmin = xcent - dHalflen;
  double xmax = xcent + dHalflen;
  double ymin = ycent - dHalflen;
  double ymax = ycent + dHalflen;

  // Each pixel holds the average of the intensity of the cell with (ix,iy) at the center of the pixel
  // Set major increments so that the last cell v[nx-1][ny-1] will start at xmax - xinc, ymax - yinc).
  // Set minor increments so that sample points are centered in cell

  double xinc = (xmax - xmin) / (iTotalRasterCols);
  double yinc = (ymax - ymin) / ny;

  double kxinc = xinc / nsample;                /* interval between samples */
  double kyinc = yinc / nsample;
  double kxofs = kxinc / 2;             /* offset of 1st point */
  double kyofs = kyinc / 2;

  im.setAxisExtent (xmin, xmax, ymin, ymax);
  im.setAxisIncrement (xinc, yinc);

  ImageFileArray v = im.getArray();

  for (int ix = 0; ix < colCount; ix++) {
    int iColStore = ix + iStorageOffset;
    ImageFileColumn vCol = v[iColStore];
    for (int iy = 0; iy < ny; iy++)
      *vCol++ = 0;
  }

  double x_start = xmin + (colStart * xinc);
  for (PElemConstIterator pelem = m_listPElem.begin(); pelem != m_listPElem.end(); pelem++) {
    const PhantomElement& rPElem = **pelem;
    double x, y, xi, yi;
    int ix, iy, kx, ky;
    for (ix = 0, x = x_start; ix < colCount; ix++, x += xinc) {
      int iColStore = ix + iStorageOffset;
      ImageFileColumn vCol = v[iColStore];
      for (iy = 0, y = ymin; iy < ny; iy++, y += yinc) {
        double dAtten = 0;
        for (kx = 0, xi = x + kxofs; kx < nsample; kx++, xi += kxinc) {
          for (ky = 0, yi = y + kyofs; ky < nsample; ky++, yi += kyinc)
            if (rPElem.isPointInside (xi, yi, PHM_COORD))
              dAtten += rPElem.atten();
        } // for kx
        *vCol++ += dAtten;
      } /* for iy */
    }  /* for ix */
  }  /* for pelem */


  if (nsample > 1) {
    double factor = 1.0 / static_cast<double>(nsample * nsample);


    for (int ix = 0; ix < colCount; ix++) {
      int iColStore = ix + iStorageOffset;
      ImageFileColumn vCol = v[iColStore];
      for (int iy = 0; iy < ny; iy++)
        *vCol++ *= factor;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS IDENTIFICATION
//
//      PhantomElement
//
// PURPOSE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////


PhantomElement::PhantomElement (const char *type, const double cx, const double cy, const double u, const double v, const double rot, const double atten)
: m_cx(cx), m_cy(cy), m_u(u), m_v(v), m_atten(atten), m_nPoints(0), m_xOutline(0), m_yOutline(0)
{
  m_rot = convertDegreesToRadians (rot);   // convert angle to radians

  m_type = convertNameToType (type);

  makeTransformMatrices ();     // calc transform matrices between phantom and normalized phantomelement
  makeVectorOutline ();         // calculate vector outline of pelem

  m_rectLimits[0] = m_xmin;   m_rectLimits[1] = m_ymin;
  m_rectLimits[2] = m_xmax;   m_rectLimits[3] = m_ymax;
}



PhantomElement::~PhantomElement ()
{
  delete m_xOutline;
  delete m_yOutline;
}

void
PhantomElement::printDefinition (std::ostream& os) const
{
  os << convertTypeToName (m_type) << " " << m_cx << " " << m_cy << " " << m_u << " "
    << m_v << " " << convertRadiansToDegrees (m_rot) << " " << m_atten << "\n";
}

void
PhantomElement::printDefinition (std::ostringstream& os) const
{
  os << convertTypeToName (m_type) << " " << m_cx << " " << m_cy << " " << m_u << " "
    << m_v << " " << convertRadiansToDegrees (m_rot) << " " << m_atten << "\n";
}

PhmElemType
PhantomElement::convertNameToType (const char* const typeName)
{
  PhmElemType type = PELEM_INVALID;

  if (strcasecmp (typeName, "rectangle") == 0)
    type = PELEM_RECTANGLE;
  else if (strcasecmp (typeName, "triangle") == 0)
    type = PELEM_TRIANGLE;
  else if (strcasecmp (typeName, "ellipse") == 0)
    type = PELEM_ELLIPSE;
  else if (strcasecmp (typeName, "sector") == 0)
    type = PELEM_SECTOR;
  else if (strcasecmp (typeName, "segment") == 0)
    type = PELEM_SEGMENT;

  return (type);
}

const char* const
PhantomElement::convertTypeToName (PhmElemType iType)
{
  static const char* pszType = "Unknown";

  if (iType == PELEM_RECTANGLE)
    pszType = "rectangle";
  else if (iType == PELEM_TRIANGLE)
    pszType = "triangle";
  else if (iType == PELEM_ELLIPSE)
    pszType = "ellipse";
  else if (iType == PELEM_SECTOR)
    pszType = "sector";
  else if (iType == PELEM_SEGMENT)
    pszType = "segment";

  return pszType;
}


void
PhantomElement::makeTransformMatrices ()
{
  GRFMTX_2D temp;

  // To map normalized Pelem coords to world Phantom
  //     scale by (u, v)
  //     rotate by rot
  //     translate by (cx, cy)

  scale_mtx2 (m_xformObjToPhm, m_u, m_v);
  rot_mtx2  (temp, m_rot);
  mult_mtx2 (m_xformObjToPhm, temp, m_xformObjToPhm);
  xlat_mtx2 (temp, m_cx, m_cy);
  mult_mtx2 (m_xformObjToPhm, temp, m_xformObjToPhm);

  // to map world Phantom coodinates to normalized PElem coords
  //     translate by (-cx, -cy)
  //     rotate by -rot
  //     scale by (1/u, 1/v)

  xlat_mtx2 (m_xformPhmToObj, -m_cx, -m_cy);
  rot_mtx2  (temp, -m_rot);
  mult_mtx2 (m_xformPhmToObj, temp, m_xformPhmToObj);
  scale_mtx2 (temp, 1 / m_u, 1 / m_v);
  mult_mtx2 (m_xformPhmToObj, temp, m_xformPhmToObj);
}


/* NAME
*   pelem_make_points           INTERNAL routine to calculate point array for an pelem
*
* SYNOPSIS
*   makepelempts (pelem)
*   PELEM *pelem        pelem whose points we are calculating
*
* NOTES
*   Called by phm_add_pelem()
*/

void
PhantomElement::makeVectorOutline ()
{
  double radius, theta, start, stop;
  double xfact, yfact;
  int cpts;

  m_nPoints = 0;
  switch (m_type) {
  case PELEM_RECTANGLE:
    m_nPoints = 5;
    m_xOutline = new double [m_nPoints];
    m_yOutline = new double [m_nPoints];
    m_xOutline[0] =-m_u;        m_yOutline[0] =-m_v;
    m_xOutline[1] = m_u;        m_yOutline[1] =-m_v;
    m_xOutline[2] = m_u;        m_yOutline[2] = m_v;
    m_xOutline[3] =-m_u;        m_yOutline[3] = m_v;
    m_xOutline[4] =-m_u;        m_yOutline[4] =-m_v;
    break;
  case PELEM_TRIANGLE:
    m_nPoints = 4;
    m_xOutline = new double [m_nPoints];
    m_yOutline = new double [m_nPoints];
    m_xOutline[0] =-m_u;        m_yOutline[0] = 0.0;
    m_xOutline[1] = m_u;        m_yOutline[1] = 0.0;
    m_xOutline[2] = 0.0;        m_yOutline[2] = m_v;
    m_xOutline[3] =-m_u;        m_yOutline[3] = 0.0;
    break;
  case PELEM_ELLIPSE:
    cpts = numCirclePoints (TWOPI);
    m_nPoints = cpts;
    m_xOutline = new double [m_nPoints];
    m_yOutline = new double [m_nPoints];
    calcEllipsePoints (m_xOutline, m_yOutline, cpts, m_u, m_v);
    break;
  case PELEM_SECTOR:
    radius = sqrt(m_u * m_u + m_v * m_v);
    theta = atan(m_u / m_v);            // angle with y-axis
    start = 3.0 * HALFPI - theta;
    stop  = 3.0 * HALFPI + theta;
    cpts = numCirclePoints (stop - start);
    m_nPoints = 3 + cpts;
    m_xOutline = new double [m_nPoints];
    m_yOutline = new double [m_nPoints];

    m_xOutline[0] = 0.0;                m_yOutline[0] = m_v;
    m_xOutline[1] =-m_u;                m_yOutline[1] = 0.0;
    calcArcPoints (&m_xOutline[2], &m_yOutline[2], cpts, 0.0, m_v, radius, start, stop);
    m_xOutline[cpts + 2] = 0.0;
    m_yOutline[cpts + 2] = m_v;
    break;
  case PELEM_SEGMENT:
    radius = sqrt(m_u * m_u + m_v * m_v);
    theta = atan (m_u / m_v);           // angle with y-axis
    start = 3.0 * HALFPI - theta;
    stop  = 3.0 * HALFPI + theta;

    cpts = numCirclePoints (stop - start);
    m_nPoints = cpts + 1;
    m_xOutline = new double [m_nPoints];
    m_yOutline = new double [m_nPoints];

    calcArcPoints (m_xOutline, m_yOutline, cpts, 0.0, m_v, radius, start, stop);
    m_xOutline[cpts] = -m_u;
    m_yOutline[cpts] = 0.0;
    break;
  default:
    sys_error(ERR_WARNING, "Illegal phantom element type %d [makeVectorOutline]", m_type);
    return;
  }

  rotate2d (m_xOutline, m_yOutline, m_nPoints, m_rot);
  xlat2d (m_xOutline, m_yOutline, m_nPoints, m_cx, m_cy);

  minmax_array (m_xOutline, m_nPoints, m_xmin, m_xmax);
  minmax_array (m_yOutline, m_nPoints, m_ymin, m_ymax);

  // increase pelem extent by SCALE_PELEM_EXTENT to eliminate chance of
  //   missing actual pelem maximum due to polygonal sampling

  xfact = (m_xmax - m_xmin) * SCALE_PELEM_EXTENT;
  yfact = (m_ymax - m_ymin) * SCALE_PELEM_EXTENT;

  m_xmin -= xfact;
  m_ymin -= yfact;
  m_xmax += xfact;
  m_ymax += yfact;
}


/* NAME
*   calc_arc                    Calculate outline of a arc of a circle
*
* SYNOPSIS
*   calc_arc (x, y, xcent, ycent, pts, r, start, stop)
*   double x[], y[];            Array of points
*   int pts                     Number of points in array
*   double xcent, ycent Center of cirlce
*   double r                    Radius of circle
*   double start, stop          Beginning & ending angles
*/

void
PhantomElement::calcArcPoints (double x[], double y[], const int pts, const double xcent, const double ycent, const double r, const double start, const double stop)
{
  if (r <= 0.0)
    sys_error (ERR_WARNING, "negative or zero radius in calc_arc()");

  double theta = (stop - start) / (pts - 1);    // angle incr. between points
  double c = cos(theta);
  double s = sin(theta);

  x[0] = r * cos (start) + xcent;
  y[0] = r * sin (start) + ycent;

  double xp = x[0] - xcent;
  double yp = y[0] - ycent;
  for (int i = 1; i < pts; i++) {
    double xc = c * xp - s * yp;
    double yc = s * xp + c * yp;
    x[i] = xc + xcent;
    y[i] = yc + ycent;
    xp = xc;  yp = yc;
  }
}


// NAME
//   PhantomElement::calcEllipsePoints    Calculate outline of a ellipse
//
// SYNOPSIS
//   calcEllipsePoints ()
//


void
PhantomElement::calcEllipsePoints (double x[], double y[], const int pts, const double u, const double v)
{
  calcArcPoints (x, y, m_nPoints, 0.0, 0.0, 1.0, 0.0, TWOPI);   // make a unit circle
  scale2d (x, y, m_nPoints, m_u, m_v);                       // scale to ellipse
}


/* NAME
*   circle_pts          Calculate number of points to use for circle segment
*
* SYNOPSIS
*   n = circle_pts (theta)
*   int n               Number of points to use for arc
*   double theta        Length of arc in radians
*/

int
PhantomElement::numCirclePoints (double theta)
{
  theta = clamp (theta, 0., TWOPI);

  return static_cast<int> (POINTS_PER_CIRCLE * theta / TWOPI + 1.5);
}


bool
PhantomElement::clipLineWorldCoords (double& x1, double& y1, double& x2, double &y2) const
{
  /* check if ray is outside of pelem extents */
  double cx1 = x1, cy1 = y1, cx2 = x2, cy2 = y2;
  if (! clip_rect (cx1, cy1, cx2, cy2, m_rectLimits))
    return false;

  // convert phantom coordinates to pelem coordinates
  xform_mtx2 (m_xformPhmToObj, x1, y1);
  xform_mtx2 (m_xformPhmToObj, x2, y2);

  if (! clipLineNormalizedCoords (x1, y1, x2, y2))
    return false;

  // convert standard pelem coordinates back to phantom coordinates
  xform_mtx2 (m_xformObjToPhm, x1, y1);
  xform_mtx2 (m_xformObjToPhm, x2, y2);

  return true;
}


/* NAME
*   pelem_clip_line                     Clip pelem against an arbitrary line
*
* SYNOPSIS
*   pelem_clip_line (pelem, x1, y1, x2, y2)
*   PhantomElement& pelem;              Pelem to be clipped
*   double *x1, *y1, *x2, *y2   Endpoints of line to be clipped
*
* RETURNS
*   true   if line passes through pelem
*               (x1, y1, x2, y2 hold coordinates of new line)
*   false  if line do not pass through pelem
*               (x1, y1, x2, y2 are undefined)
*/

bool
PhantomElement::clipLineNormalizedCoords (double& x1, double& y1, double& x2, double& y2) const
{
  bool accept = false;

  switch (m_type) {
  case PELEM_RECTANGLE:
    double rect[4];
    rect[0] = -1.0;  rect[1] = -1.0;
    rect[2] = 1.0;  rect[3] = 1.0;
    accept = clip_rect (x1, y1, x2, y2, rect);
    break;
  case PELEM_ELLIPSE:
    accept = clip_circle (x1, y1, x2, y2, 0.0, 0.0, 1.0, 0.0, 0.0);
    break;
  case PELEM_TRIANGLE:
    accept = clip_triangle (x1, y1, x2, y2, 1.0, 1.0, true);
    break;
  case PELEM_SEGMENT:
    accept = clip_segment (x1, y1, x2, y2, m_u, m_v);
    break;
  case PELEM_SECTOR:
    accept = clip_sector (x1, y1, x2, y2, m_u, m_v);
    break;
  default:
    sys_error (ERR_WARNING, "Illegal pelem type %d [pelem_clip_line]", m_type);
    break;
  }

  return(accept);
}


// METHOD IDENTIFICATION
//    PhantomElement::isPointInside             Check if point is inside pelem
//
// SYNOPSIS
//    is_point_inside (pelem, x, y, coord_type)
//    double x, y               Point to see if lies in pelem
//    int coord_type            Coordinate type (PELEM_COORD or PHM_COORD)
//
// RETURNS
//    true if point lies within pelem
//    false if point lies outside of pelem

bool
PhantomElement::isPointInside (double x, double y, const CoordType coord_type) const
{
  if (coord_type == PHM_COORD) {
    xform_mtx2 (m_xformPhmToObj, x, y);
  } else if (coord_type != PELEM_COORD) {
    sys_error(ERR_WARNING, "Illegal coordinate type in pelem_is_point_inside");
    return (false);
  }

  switch (m_type) {
  case PELEM_RECTANGLE:
    if (x > 1. || x < -1. || y > 1. || y < -1.)
      return (false);
    else
      return (true);
    break;
  case PELEM_TRIANGLE:
    if (y < 0. || y > 1. - x || y > 1. + x)
      return (false);
    else
      return (true);
    break;
  case PELEM_ELLIPSE:
    if (x > 1. || x < -1. || y > 1. || y < -1.)
      return (false);
    if (x * x + y * y > 1.)             // check if inside unit circle
      return (false);
    else
      return (true);
    break;

    // for clipping segments & sectors, must NOT scale by (1/u, 1/v)
    // because this destroys information about size of arc component

  case PELEM_SEGMENT:
    if (x > 1. || x < -1. || y > 0.)
      return (false);           // clip against y > 0
    x *= m_u;                   // put back u & v scale
    y *= m_v;
    if (x * x + (y-m_v) * (y-m_v) > m_u * m_u + m_v * m_v)
      return (false);           // clip against circle, r = sqrt(@)
    else
      return (true);
    break;
  case PELEM_SECTOR:
    if (x > 1. || x < -1. || y > 1.)   // extent
      return (false);
    if (y > 1. - x || y > 1. + x)      // triangle
      return (false);                  // clip against triangle
    x *= m_u;                  // circle: put back u & v scale
    y *= m_v;
    if (x * x + (y-m_v) * (y-m_v) > m_u * m_u + m_v * m_v)
      return (false);                  // clip against circle
    else
      return (true);
    break;
  default:
    sys_error (ERR_WARNING, "Illegal pelem type in pelem_is_point_inside()");
    break;
  }

  return (false);
}


