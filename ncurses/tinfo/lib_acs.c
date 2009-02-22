/****************************************************************************
 * Copyright (c) 1998-2008,2009 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: lib_acs.c,v 1.36.1.3 2009/02/21 15:11:29 tom Exp $")

#if BROKEN_LINKER || USE_REENTRANT
#define MyBuffer _nc_prescreen.real_acs_map
NCURSES_EXPORT_VAR (chtype *)
_nc_acs_map(void)
{
    if (MyBuffer == 0)
	MyBuffer = typeCalloc(chtype, ACS_LEN);
    return MyBuffer;
}
#undef MyBuffer
#else
NCURSES_EXPORT_VAR (chtype) acs_map[ACS_LEN] =
{
    0
};
#endif

NCURSES_EXPORT(chtype)
NCURSES_SP_NAME(_nc_acs_char) (SCREEN *sp, int c)
{
    chtype *map;
    if (c < 0 || c >= ACS_LEN)
	return (chtype) 0;
    map = (sp != 0) ? sp->_acs_map :
#if BROKEN_LINKER || USE_REENTRANT
	_nc_prescreen.real_acs_map
#else
	acs_map
#endif
	;
    return map[c];
}

NCURSES_EXPORT(void)
NCURSES_SP_NAME(_nc_init_acs) (SCREEN *sp)
{
    chtype *fake_map = acs_map;
    chtype *real_map = sp != 0 ? sp->_acs_map : fake_map;
    int j;

    T(("initializing ACS map"));

    /*
     * If we're using this from curses (rather than terminfo), we are storing
     * the mapping information in the SCREEN struct so we can decide how to
     * render it.
     */
    if (real_map != fake_map) {
	for (j = 1; j < ACS_LEN; ++j) {
	    real_map[j] = 0;
	    fake_map[j] = A_ALTCHARSET | j;
	    if (sp)
		sp->_screen_acs_map[j] = FALSE;
	}
    } else {
	for (j = 1; j < ACS_LEN; ++j) {
	    real_map[j] = 0;
	}
    }

    /*
     * Initializations for a UNIX-like multi-terminal environment.  Use
     * ASCII chars and count on the terminfo description to do better.
     */
    real_map['l'] = '+';	/* should be upper left corner */
    real_map['m'] = '+';	/* should be lower left corner */
    real_map['k'] = '+';	/* should be upper right corner */
    real_map['j'] = '+';	/* should be lower right corner */
    real_map['u'] = '+';	/* should be tee pointing left */
    real_map['t'] = '+';	/* should be tee pointing right */
    real_map['v'] = '+';	/* should be tee pointing up */
    real_map['w'] = '+';	/* should be tee pointing down */
    real_map['q'] = '-';	/* should be horizontal line */
    real_map['x'] = '|';	/* should be vertical line */
    real_map['n'] = '+';	/* should be large plus or crossover */
    real_map['o'] = '~';	/* should be scan line 1 */
    real_map['s'] = '_';	/* should be scan line 9 */
    real_map['`'] = '+';	/* should be diamond */
    real_map['a'] = ':';	/* should be checker board (stipple) */
    real_map['f'] = '\'';	/* should be degree symbol */
    real_map['g'] = '#';	/* should be plus/minus */
    real_map['~'] = 'o';	/* should be bullet */
    real_map[','] = '<';	/* should be arrow pointing left */
    real_map['+'] = '>';	/* should be arrow pointing right */
    real_map['.'] = 'v';	/* should be arrow pointing down */
    real_map['-'] = '^';	/* should be arrow pointing up */
    real_map['h'] = '#';	/* should be board of squares */
    real_map['i'] = '#';	/* should be lantern symbol */
    real_map['0'] = '#';	/* should be solid square block */
    /* these defaults were invented for ncurses */
    real_map['p'] = '-';	/* should be scan line 3 */
    real_map['r'] = '-';	/* should be scan line 7 */
    real_map['y'] = '<';	/* should be less-than-or-equal-to */
    real_map['z'] = '>';	/* should be greater-than-or-equal-to */
    real_map['{'] = '*';	/* should be greek pi */
    real_map['|'] = '!';	/* should be not-equal */
    real_map['}'] = 'f';	/* should be pound-sterling symbol */

    CallDriver_2(sp, initacs, real_map, fake_map);
}

NCURSES_EXPORT(void)
_nc_init_acs(void)
{
    NCURSES_SP_NAME(_nc_init_acs) (CURRENT_SCREEN);
}
