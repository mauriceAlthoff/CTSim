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

#include <stdio.h>
#include <string.h>
#include "ctsupport.h"


/* NAME
 *   cio_put_c                          Put a character on screen
 *
 * SYNOPSIS
 *   cio_put_c (c)
 *   char c                             Character to write
 *
 * NOTES
 *   Color of character is determined by the global variable, crtv.text_attr.
 *
 * SIDE EFFECTS
 *   Cursor is advanced by one.  If necessary, the cursor will wrap around
 *   and maybe the screen will scroll
 */

void
cio_put_c (int c)
{
    fputc(c, stdout);
}



/* NAME
 *    cio_put_cc                        Put a char on screen count times
 *
 * SYNOPSIS
 *    cio_put_cc (c, count)
 *    char c                            Character to write
 *    int count                         Number of characters to write
 */

void
cio_put_cc (int c, int count)
{
    for (int i = 0; i < count; i++)
        cio_put_c (c);
}


void
cio_put_str (const char *str)
{
    fputs (str, stdout);
}



/* NAME
 *   kb_getc                    Get a character from the keyboard
 *
 * SYNOPSIS
 *   key = kb_getc()
 *
 * DESCRIPTION
 *      1. This routine returns an EXTENTED ASCII code,
 *         the extended codes have a low byte of 0 and a distinctive
 *         high byte, such as 0x2D00 and 0x3200
 *      2. This routine waits until a key has been typed
 *      2. The keystroke will not be echoed.
 */

unsigned int cio_kb_getc(void)
{
    return fgetc(stdin);
}

void
cio_kb_ungetc (unsigned int c)
{
    ungetc(c, stdin);
}

/* NAME
 *    kb_gets                           Get a string from the keyboard
 *
 * SYNOPSIS
 *    str = kb_gets (str, maxlen)
 *    char *str                         Space to store input string
 *    int maxlen                        Maximum number of characters to read
 *                                      (Not including EOS)
 * NOTES
 *    Backspace - erases character to the right
 *    Escape    - erases to beginning of line
 *    Return    - ends string (no not cause a linefeed)
 */

char *
cio_kb_gets (char *str, int maxlen)
{
    return fgets(str, maxlen, stdin);
}

/* NAME
 *   kb_waitc                   Wait for a character from the keyboard
 *
 * SYNOPSIS
 *   key = kb_waitc (astr, estr, beep)
 *   int key                    Keystroke entered
 *   char *astr                 String of valid ascii characters
 *   bool beep                  If TRUE, beep when user hits invalid key
 *
 */


unsigned int
cio_kb_waitc (const char *astr, int beep_on_error)
{
  unsigned int c;
  do {
    c = cio_kb_getc ();
    if (strchr (astr, c) != NULL)
        break;
    if (beep_on_error)
      cio_beep();
  } while (1);

  return (c);
}


/* NAME
 *    beep                              sound a beep to user
 *
 * SYNOPSIS
 *    beep()
 */

void cio_beep (void)
{
        cio_tone (2000.0, 0.3);
}

/* NAME
 *    tone              play a frequency sound for some duration
 *
 * SYNOPSIS
 *    tone (freq, length)
 *    double freq       frequency to play in Hertz
 *    double length     duration to play note in seconds
 */

void
cio_tone (double freq, double length)
{
#if 1
  fprintf(stdout, "\007");
#else
  cio_spkr_freq (freq);         /* Set frequency of tone */
  cio_spkr_on ();                       /* Turn on speaker */
  pause (length);                       /* Pause for length seconds */
  cio_spkr_off ();                      /* Turn off speaker */
#endif
}
