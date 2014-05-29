/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         msvc_compat.h
**      Purpose:      Microsoft Visual C compatibility header
**      Author:       Kevin Rosenberg
**      Date Started: Dec 2000
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

#ifndef __MSVC_COMPAT_H
#define __MSVC_COMPAT_H

#ifdef MSVC
  typedef long off_t;
  #define HAVE_STRING_H 1
  #define HAVE_VSNPRINTF 1
  #include <fcntl.h>
  #define strdup _strdup
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #pragma warning(disable:4786)
#endif
#endif
