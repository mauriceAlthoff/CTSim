/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         array2d.h
**      Purpose:      2-dimension array classes
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

#ifndef ARRAY2D_H
#define ARRAY2D_H

#include "ctsupport.h"


template<class T>
class Array2d {
 public:
    Array2d (unsigned int x, unsigned int y)
        : m_nx(x), m_ny(y), array_data(0)
        {
            allocArray();
        }

    Array2d ()
        : m_nx(0), m_ny(0), array_data(0)
        {}

    ~Array2d ()
        {
            deleteArray();
        }

    void initSetSize (unsigned int x, unsigned int y)
        {
            m_nx = x;
            m_ny = y;
            deleteArray();
            allocArray();
        }

    T** getArray () const
        { return array_data; }

    T* getColumn (unsigned int x) const
        { return (array_data ? array_data[x] : NULL); }

    T getPoint (unsigned int x, unsigned int y) const
        { return (array_data ? array_data[x][y] : NULL); }

    unsigned int sizeofPixel () const
        {  return sizeof(T); }

    unsigned int sizeofColumn () const
        { return (sizeof(T) * m_ny); }

    unsigned int sizeofArray () const
        { return (sizeof(T) * m_nx * m_ny); }


 private:
    unsigned int m_nx;
    unsigned int m_ny;
    T** array_data;

    void allocArray ()
        {
            if (array_data)
                deleteArray();

            array_data = new T*[m_nx];

            for (unsigned int i = 0; i < m_nx; i++)
                array_data[i] = new T[m_ny];
        }

    void deleteArray ()
        {
            if (array_data) {
                for (unsigned int i = 0; i < m_nx; i++)
                    delete array_data [i];
                delete array_data;
                array_data = NULL;
            }
        }


    Array2d& operator= (const Array2d& rhs); //assignment operator
    Array2d (const Array2d& rhs);  // copy constructor
};


#endif
