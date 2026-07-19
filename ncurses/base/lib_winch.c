/****************************************************************************
 * Copyright 2020-2024,2026 Thomas E. Dickey                                *
 * Copyright 1998-2009,2010 Free Software Foundation, Inc.                  *
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/

/*
**	lib_winch.c
**
**	The routine winch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_winch.c,v 1.13 2026/07/19 00:52:24 tom Exp $")

NCURSES_EXPORT(chtype)
winch(WINDOW *win)
{
    chtype result = 0;

    T((T_CALLED("winch(%p)"), (void *) win));
    if (win != NULL) {
	NCURSES_CH_T *cell = &win->_line[win->_cury].text[win->_curx];
#if USE_WIDEC_SUPPORT
	if (!_nc_unicode_locale()) {

	    if (!isWidecExt(*cell)) {
		attr_t attrs;
		NCURSES_PAIRS_T pair;
		wchar_t wch[CCHARW_MAX + 2];
		int n2;

		n2 = getcchar(cell, NULL, NULL, NULL, NULL);
		if (n2 > 0 && n2 <= CCHARW_MAX) {

		    if (getcchar(cell, wch, &attrs, &pair, NULL) == OK) {
			mbstate_t state;
			size_t n3;

			init_mb(state);
			n3 = wcstombs(NULL, wch, (size_t) 0);
			if (OkWcsToMbs(n3) && n3 == 1) {
			    char tmp[2];

			    init_mb(state);
			    n3 = wcstombs(tmp, wch, 1);
			    if (OkWcsToMbs(n3)) {
				result = UChar(tmp[0]);
			    }
			}
		    }
		}
	    }
	} else
#endif
	{
	    result = ((chtype) CharOf(*cell) | AttrOf(*cell));
	}
    }
    returnChtype(result);
}
