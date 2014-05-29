/*****************************************************************************
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "ctsupport.h"
#include "fnetorderstream.h"


void
ConvertNetworkOrder (void* buffer, size_t bytes)
{
#ifndef WORDS_BIGENDIAN
    if (bytes < 2)
        return;

    char* start = static_cast<char*>(buffer);
    char* end = start + bytes - 1;   // last byte
    size_t nSwap = bytes / 2;

    while (nSwap-- > 0) {
        unsigned char c = *start;
        *start++ = *end;
        *end-- = c;
    }
#endif
}

void
ConvertReverseNetworkOrder (void* buffer, size_t bytes)
{
#ifdef WORDS_BIGENDIAN
    if (bytes < 2)
        return;

    char* start = static_cast<char*>(buffer);
    char* end = start + bytes - 1;  // last byte
    size_t nSwap = bytes / 2;

    while (nSwap-- > 0) {
        unsigned char c = *start;
        *start++ = *end;
        *end-- = c;
    }
#endif
}

void
fnetorderstream::writeInt16 (kuint16 n) {
#ifndef WORDS_BIGENDIAN
  SwapBytes2 (&n);
#endif
  write (reinterpret_cast<const char*>(&n), 2);
}

void
fnetorderstream::writeInt32 (kuint32 n) {
#ifndef WORDS_BIGENDIAN
  SwapBytes4(&n);
#endif
  write (reinterpret_cast<const char*>(&n), 4);
}

void
fnetorderstream::writeFloat32 (kfloat32 n) {
#ifndef WORDS_BIGENDIAN
  SwapBytes4 (&n);
#endif
  write (reinterpret_cast<const char*>(&n), 4);
}

void
fnetorderstream::writeFloat64 (kfloat64 n) {
#ifndef WORDS_BIGENDIAN
  SwapBytes8 (&n);
#endif
  write (reinterpret_cast<const char*>(&n), 8);
}

void
fnetorderstream::readInt16 (kuint16& n) {
  read (reinterpret_cast<char*>(&n), 2);
#ifndef WORDS_BIGENDIAN
  SwapBytes2 (&n);
#endif
}

void
fnetorderstream::readInt32 (kuint32& n) {
  read (reinterpret_cast<char*>(&n), 4);
#ifndef WORDS_BIGENDIAN
  SwapBytes4 (&n);
#endif
}

void
fnetorderstream::readFloat32 (kfloat32& n) {
  read (reinterpret_cast<char*>(&n), 4);
#ifndef WORDS_BIGENDIAN
  SwapBytes4 (&n);
#endif
}

void
fnetorderstream::readFloat64 (kfloat64& n) {
  read (reinterpret_cast<char*>(&n), 8);
#ifndef WORDS_BIGENDIAN
  SwapBytes8 (&n);
#endif
}



void
frnetorderstream::writeInt16 (kuint16 n) {
#ifdef WORDS_BIGENDIAN
  SwapBytes2 (&n);
#endif
  write (reinterpret_cast<char*>(&n), 2);
}

void
frnetorderstream::writeInt32 (kuint32 n) {
#ifdef WORDS_BIGENDIAN
  SwapBytes4(&n);
#endif
  write (reinterpret_cast<char*>(&n), 4);
}

void
frnetorderstream::writeFloat32 (kfloat32 n) {
#ifdef WORDS_BIGENDIAN
  SwapBytes4 (&n);
#endif
  write (reinterpret_cast<char*>(&n), 4);
}

void
frnetorderstream::writeFloat64 (kfloat64 n) {
#ifdef WORDS_BIGENDIAN
  SwapBytes8 (&n);
#endif
  write (reinterpret_cast<char*>(&n), 8);
}

void
frnetorderstream::readInt16 (kuint16& n) {
  read (reinterpret_cast<char*>(&n), 2);
#ifdef WORDS_BIGENDIAN
  SwapBytes2 (&n);
#endif
}

void
frnetorderstream::readInt32 (kuint32& n) {
  read (reinterpret_cast<char*>(&n), 4);
#ifdef WORDS_BIGENDIAN
  SwapBytes4 (&n);
#endif
}

void
frnetorderstream::readFloat32 (kfloat32& n) {
  read (reinterpret_cast<char*>(&n), 4);
#ifdef WORDS_BIGENDIAN
  SwapBytes4 (&n);
#endif
}

void
frnetorderstream::readFloat64 (kfloat64& n) {
  read (reinterpret_cast<char*>(&n), 8);
#ifdef WORDS_BIGENDIAN
  SwapBytes8 (&n);
#endif
}

