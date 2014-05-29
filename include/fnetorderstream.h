/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          fnetorderstream.h
**   Purpose:       Network-order file stream header
**   Programmer:    Kevin Rosenberg
**   Date Started:  Sep 2000
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


#ifndef NETORDER_H
#define NETORDER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>
#include <iostream>
#include <string>


inline bool NativeBigEndian (void)
{
#ifdef WORDS_BIGENDIAN
    return true;
#else
    return false;
#endif
}

inline void
SwapBytes2 (void* buffer)
{
  unsigned char* p = static_cast<unsigned char*>(buffer);
  unsigned char temp = p[0];
  p[0] = p[1];
  p[1] = temp;
}

// 0<->3  1<->2 = 0123 -> 3210
inline void
SwapBytes4 (void* buffer)
{
  unsigned char* p = static_cast<unsigned char*>(buffer);
  unsigned char temp = p[0];
  p[0] = p[3];
  p[3] = temp;
  temp = p[1];
  p[1] = p[2];
  p[2] = temp;
}

// 0<->7 1<->6 2<->5 3<->4 = 01234567 -> 76543210
inline void
SwapBytes8 (void* buffer)
{
  unsigned char* p = static_cast<unsigned char*>(buffer);
  unsigned char temp = p[0];
  p[0] = p[7];
  p[7] = temp;
  temp = p[1];
  p[1] = p[6];
  p[6] = temp;
  temp = p[2];
  p[2] = p[5];
  p[5] = temp;
  temp = p[3];
  p[3] = p[4];
  p[4] = temp;
}

inline void
SwapBytes2IfLittleEndian (void* buffer)
{
#ifndef WORDS_BIGENDIAN
  SwapBytes2 (buffer);
#endif
}

inline void
SwapBytes4IfLittleEndian (void* buffer)
{
#ifndef WORDS_BIGENDIAN
  SwapBytes4 (buffer);
#endif
}

inline void
SwapBytes8IfLittleEndian (void* buffer)
{
#ifndef WORDS_BIGENDIAN
  SwapBytes8 (buffer);
#endif
}

void ConvertNetworkOrder (void* buffer, size_t bytes);
void ConvertReverseNetworkOrder (void* buffer, size_t bytes);

using std::fstream;
class fnetorderstream : public fstream {
 public:
  fnetorderstream (const char* filename, std::ios::openmode mode)
          : fstream (filename, mode) {}

  ~fnetorderstream (void)
      {}

  virtual void writeInt16 (kuint16 n);
  virtual void writeInt32 (kuint32 n);
  virtual void  writeFloat32 (kfloat32 n);
  virtual void  writeFloat64 (kfloat64 n);

  virtual void  readInt16 (kuint16& n);
  virtual void  readInt32 (kuint32& n);
  virtual void  readFloat32 (kfloat32& n);
  virtual void  readFloat64 (kfloat64& n);
};


class frnetorderstream : public fnetorderstream {
 public:
  frnetorderstream (const char* filename, std::ios::openmode mode)
    : fnetorderstream (filename, mode) {}

  virtual void writeInt16 (kuint16 n);
  virtual void writeInt32 (kuint32 n);
  virtual void writeFloat32 (kfloat32 n);
  virtual void writeFloat64 (kfloat64 n);

  virtual void readInt16 (kuint16& n);
  virtual void readInt32 (kuint32& n);
  virtual void readFloat32 (kfloat32& n);
  virtual void readFloat64 (kfloat64& n);
};

#endif
