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

#include <ctype.h>
#include "ctsupport.h"


/* NAME
 *      str_skip_head                   Skip leading characters of string
 *
 * SYNOPSIS
 *      shortened = str_skip_head (str, charlist)
 *  OUT shortened                       Start of shortened string
 *  IN  char *str                       String to have beginning skipped
 *  IN  char *charlist                  List of characters to skip over
 *
 * NOTES
 *          This routine returns the position in a string (str) of the
 *      first character that is not in an specified string of characters
 *      (charlist).
 */


char*
str_skip_head (const char* str, const char* const charlist)
{
  const char* p = str;

  while (*p && (strchr (charlist, *p) != NULL))
    p++;

  return (const_cast<char*>(p));
}

char*
str_skip_head (const char* str, char* charlist)
{
  const char* p = str;

  while (*p && (strchr (charlist, *p) != NULL))
    p++;

  return (const_cast<char*>(p));
}


/* NAME
 *      str_lower                       Convert a string to lower case
 *
 * SYNOPSIS
 *      str  = str_lower (str)
 *      char *str                       String to be converted
 */

char *
str_lower (char *s)
{
    char *p = s;

    while (*p) {                        /* while (*p != EOS) */
        *p = tolower(*p);
        ++p;
    }

    return (s);
}

/* NAME
 *      str_rm_tail                     Remove characters from end of string
 *
 * SYNOPSIS
 *      str = str_rm_tail (str, charlist)
 *      char *str                       String to have end removed
k *     char *charlist                  List of characters to remove from string
 *
 */


char *
str_rm_tail (char *str, const char* const charlist)
{
  int i;

  for (i = strlen(str) - 1; i >= 0; i--)
    if (strchr (charlist, str[i]) != NULL)
      str[i] = EOS;
    else
      break;            /* found non-specified char, all done */

  return (str);
}

/* NAME
 *      str_wrm_tail                    Remove white space from end of string
 *
 * SYNOPSIS
 *      str = str_wrm_tail (str)
 *      char *str                       String to have white space removed
 *
 */

char *
str_wrm_tail (char *str)
{
  return (str_rm_tail(str, "\b\t\n\r"));
}

/* NAME
 *      str_upper                       Convert a string to upper case
 *
 * SYNOPSIS
 *      str  = str_upper (str)
 *      char *str                       String to be converted
 */

char *
str_upper (char *s)
{
  char *p = s;

  while (*p) {                  /* while (*s != EOS) */
    *p = toupper(*p);
    p++;
  }

  return (s);
}


#ifdef TEST
int
main (void)
{
  string str, clist;
  char *skip;

  printf ("Test program for str_skip_head\n");
  printf ("\n");
  printf ("Enter string that will have its head skipped -- ");
  gets (str);
  printf ("Enter list of characters to be skipped -- ");
  gets (clist);
  printf ("\n");

  skip = str_skip_head (str, clist);

  printf ("Shortened string = '%s'\n", skip);
}
#endif


