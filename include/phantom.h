/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          phantom.h
**   Purpose:       Header file for Phantom objects
**   Programmer:    Kevin Rosenberg
**   Date Started:  July 1, 1984
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

#ifndef PHANTOM_H
#define PHANTOM_H

#include <list>
#include "ctsupport.h"

typedef enum {
    PELEM_INVALID,
    PELEM_RECTANGLE,
    PELEM_TRIANGLE,
    PELEM_ELLIPSE,
    PELEM_SECTOR,
    PELEM_SEGMENT
} PhmElemType;

/* Codes for Coordinate Types      */
/* Defines coords for isPointInside() */

typedef enum {
  PELEM_COORD,         /* Normalized PElem Coordinates */
  PHM_COORD           /* World phantom Coordinates */
} CoordType;


class PhantomElement
{
 public:
    PhantomElement (const char* const type, const double cx, const double cy, const double u, const double v, const double rot, const double atten);

    ~PhantomElement ();

    bool isPointInside (double x, double y, const CoordType coord_type) const;

    bool clipLineNormalizedCoords (double& x1, double& y1, double& x2, double& y2) const;

    bool clipLineWorldCoords (double& x1, double& y1, double& x2, double& y2) const;

    const int nOutlinePoints() const {return m_nPoints;}
    double* rectLimits() {return m_rectLimits;}
    double* xOutline() {return m_xOutline;}
    double* yOutline() {return m_yOutline;}
    double* const xOutline() const {return m_xOutline;}
    double* const yOutline() const {return m_yOutline;}
    const double atten() const {return m_atten;}
    const double xmin() const {return m_xmin;}
    const double xmax() const {return m_xmax;}
    const double ymin() const {return m_ymin;}
    const double ymax() const {return m_ymax;}
    const double rot() const {return m_rot;}
    const double cx() const {return m_cx;}
    const double cy() const {return m_cy;}
    const double u() const {return m_u;}
    const double v() const {return m_v;}

    static PhmElemType convertNameToType (const char* const typeName);

    void printDefinition (std::ostream& os) const;
    void printDefinition (std::ostringstream& os) const;

 private:
    PhmElemType m_type;      // pelem type (box, ellipse, etc)
    double m_cx, m_cy;       // center of pelem
    double m_u, m_v;                 // size of pelem
    double m_atten;          // X-ray attenuation coefficient
    double m_rot;                    // pelem rotation angle (in radians)
    double *m_x, *m_y;       // ptr to array of points in obj world coord
    int m_nPoints;                   // number of points in outline arrays
    double m_xmin, m_xmax, m_ymin, m_ymax;  // pelem limits
    GRFMTX_2D m_xformPhmToObj;        // map from phantom to normalized pelem coords
    GRFMTX_2D m_xformObjToPhm;        // map from normalized pelem coords to phantom coords
    double* m_xOutline;
    double* m_yOutline;
    double  m_rectLimits[4];

    static const int POINTS_PER_CIRCLE;
    static const double SCALE_PELEM_EXTENT;  // increase pelem limits by 0.5%

    static const char* const convertTypeToName (PhmElemType iType);

    void makeTransformMatrices ();

    void makeVectorOutline ();

    void calcArcPoints (double x[], double y[], const int pts, const double xcent, const double ycent, const double r, const double start, const double stop);

    void calcEllipsePoints (double x[], double y[], const int pts, const double u, const double v);

    static int numCirclePoints (double theta);

    PhantomElement (const PhantomElement& rhs);        // copy constructor
    PhantomElement& operator= (const PhantomElement&); // assignment operator
};


typedef enum {
    P_PELEMS,        // Phantom made of PElems
    P_UNIT_PULSE,   // Special phantom, not made of pelems
    P_FILTER      // defined only by this type
} PhantomComposition;


//////////////////////////////////////////////////////
// Phantom Class Declaration
//////////////////////////////////////////////////////

class SGP;
class ImageFile;
class Phantom
{
 public:
    static const int PHM_INVALID;
    static const int PHM_HERMAN;
    static const int PHM_SHEPP_LOGAN;
    static const int PHM_UNITPULSE;

    Phantom ();
    Phantom (const char* const phmName);

    ~Phantom ();

    void setComposition (PhantomComposition composition)
        { m_composition = composition; }

    const PhantomComposition getComposition () const
        { return m_composition; }

    bool createFromPhantom (const char* const phmName);

    bool createFromPhantom (const int phmid);

    bool createFromFile (const char* const fname);

    bool fileWrite (const char* const fname);

    void addPElem (const PhantomElement& pelem);

    void addPElem (const char* const composition, const double cx, const double cy, const double u, const double v, const double rot, const double atten);

    void convertToImagefile (ImageFile& im, double dViewRatio, const int in_nsample, const int trace) const;
    void convertToImagefile (ImageFile& im, double dViewRatio, const int in_nsample, const int trace,
      const int colStart, const int colCount, bool bStoreAtColumnPos) const;
    void convertToImagefile (ImageFile& im, int iNX, double dViewRatio, const int in_nsample, const int trace,
      const int colStart, const int colCount, int iStorageOffset) const;

    void printDefinitions (std::ostream& os) const;
    void printDefinitions (std::ostringstream& os) const;

    bool fail() const             {return m_fail;}
    const std::string& failMessage() const {return m_failMessage;}
    const std::string& name() const     {return m_name;}
    const int id() const     {return m_id;}

#ifdef HAVE_SGP
    void show () const;
    void show (SGP& sgp) const;
    void draw (SGP& sgp) const;
#endif

    void addStdHerman ();
    void addStdSheppLogan ();

    void print (std::ostream& os) const;
    void print (std::ostringstream& os) const;

    double maxAxisLength () const
    {  return maxValue<double> (m_xmax - m_xmin, m_ymax - m_ymin); }

    double getDiameterBoundaryCircle() const
    { return SQRT2 * maxAxisLength(); }

    const double xmin() const {return m_xmin;}
    const double xmax() const {return m_xmax;}
    const double ymin() const {return m_ymin;}
    const double ymax() const {return m_ymax;}
          std::list<PhantomElement*>& listPElem() {return m_listPElem;}
    const std::list<PhantomElement*>& listPElem() const {return m_listPElem;}
    const int nPElem() const {return m_nPElem;}

    static const int getPhantomCount() {return s_iPhantomCount;}
    static const char** getPhantomNameArray() {return s_aszPhantomName;}
    static const char** getPhantomTitleArray() {return s_aszPhantomTitle;}
    static int convertNameToPhantomID (const char* const phmName);
    static const char* convertPhantomIDToName (const int phmID);
    static const char* convertPhantomIDToTitle (const int phmID);

 private:
    PhantomComposition m_composition;
    int m_nPElem;                           // number of pelems in phantom
    double m_xmin, m_xmax, m_ymin, m_ymax;  // extent of pelems in pelem coordinates
    mutable std::list<PhantomElement*> m_listPElem;      // pelem lists
    std::string m_name;
    int m_id;
    bool m_fail;
    std::string m_failMessage;
    static const char* s_aszPhantomName[];
    static const char* s_aszPhantomTitle[];
    static const int s_iPhantomCount;

    void init();

    Phantom (const Phantom& rhs);        // copy constructor
    Phantom& operator= (const Phantom&); // assignment operator
};

typedef std::list<PhantomElement*>::iterator PElemIterator;
typedef std::list<PhantomElement*>::const_iterator PElemConstIterator;

#endif
