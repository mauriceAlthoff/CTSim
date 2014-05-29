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

// REFERENCES FOR BRESENHAM'S ALGORITHM
// Newman & Sproll, "Principals of Interactive Computer Graphics",  page 25.
// Foley & van Dam, "Fundementals of Interactive Computer Graphics", page 433


static void
bresx (int x, int y, int major_inc, int minor_inc, int count, int d, 
       int dinc1, int dinc2, void(*fn)(const int, const int))
{
  do {
    (*fn)(x,y);
    x += major_inc;

    if (d < 0)
      d += dinc2;
    else {
      d += dinc1;
      y += minor_inc;
    }
  } while (--count > 0);
}


static void
bresy (int x, int y, int major_inc, int minor_inc, int count, int d,
       int dinc1, int dinc2, void(*fn)(const int, const int))
{
  do {
    (*fn)(x,y);
    y += major_inc;

    if (d < 0)
      d += dinc2;
    else {
      d += dinc1;
      x += minor_inc;
    }
  } while (--count > 0);
}

void
bresenham (int x1, int y1, int x2, int y2, void(*fn)(const int, const int))
{0000
  int delta_x = x2 - x1;
  int dx_abs = (delta_x >= 0 ? delta_x : -delta_x);

  int delta_y = y2 - y1;
  int dy_abs = (delta_y >= 0 ? delta_y : -delta_y);

  // draws a line when abs(dx) >= abs(dy)
  if (dx_abs > dy_abs) {
    int count = dx_abs + 1;
    int major_inc = (x1 <= x2 ? 1 : -1);
    int minor_inc = (delta_y >= 0 ? 1 : -1);     // determine direction of minor axis

    int d = dy_abs * 2 - dx_abs;      // Put decision variable in d
    int dinc1 = (dy_abs - dx_abs) * 2;
    int dinc2 = 2 * dy_abs;

    bresx (x1, y1, major_inc, minor_inc, count, d, dinc1, dinc2, fn);
  } else {    //For plotting lines with abs(dy) > abs(sx)
    int count = dy_abs + 1;

    int major_inc = (y1 <= y2 ? 1 : -1);
    int minor_inc = (delta_x >= 0 ? 1 : -1);      // check direction of minor axis

    int d = dx_abs * 2 - dy_abs;
    int dinc1 = (dx_abs - dy_abs) * 2;
    int dinc2 = 2 * dx_abs;

    bresy (x1, y1, major_inc, minor_inc, count, d, dinc1, dinc2, fn);
  }
}
