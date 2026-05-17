/****************************************************************************
 * Copyright 2026 Thomas E. Dickey                                          *
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

#include <curses.priv.h>
#include <tic.h>

#ifndef CUR
#define CUR SP_TERMTYPE
#endif

MODULE_ID("$Id: cookie_updates.c,v 1.4 2026/05/16 23:40:00 tom Exp $")

static bool
check_attrs(const SCREEN *sp, WINDOW *win)
{
    bool found = FALSE;
    if (win != NULL) {
	int row;
	for (row = 0; row < sp->_lines; ++row) {
	    NCURSES_CH_T *text = win->_line[row].text;
	    int col;
	    for (col = 0; col < sp->_columns; ++col) {
		attr_t attr = AttrOf(text[col]);
		/*
		 * Note: magic cookies predate hardware support for dim,
		 * italics, and (likely) alternate character set.
		 */
		if ((attr & (A_STANDOUT |
			     A_UNDERLINE |
			     A_REVERSE |
			     A_BLINK |
			     A_DIM |
			     A_BOLD)) != 0) {
		    found = TRUE;
		    break;
		}
	    }
	    if (found)
		break;
	}
    }
    return found;
}

NCURSES_EXPORT(void)
_nc_cookie_updates(SCREEN *sp)
{
    if (magic_cookie_glitch >= 0 &&
	sp != NULL &&
	!sp->cookie_initialized) {
	if (_nc_cookie_allowed(sp)) {
	    bool found = (check_attrs(sp, sp->_curscr) ||
			  check_attrs(sp, sp->_newscr) ||
			  check_attrs(sp, sp->_stdscr));
	    if (found) {
		_nc_cookie_init(sp);
		sp->cookie_active = TRUE;
	    }
	} else {
	    sp->cookie_initialized = TRUE;
	    magic_cookie_glitch = ABSENT_NUMERIC;
	}
    }
}
