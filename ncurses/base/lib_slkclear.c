/****************************************************************************
 * Copyright (c) 1998-2006,2007 Free Software Foundation, Inc.              *
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
 *     and: Juergen Pfeifer                         1996-1999               *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	lib_slkclear.c
 *	Soft key routines.
 *      Remove soft labels from the screen.
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkclear.c,v 1.10.1.1 2008/11/16 00:19:59 juergen Exp $")

NCURSES_EXPORT(int)
NC_SNAME(slk_clear)(SCREEN *sp)
{
    int rc = ERR;

    T((T_CALLED("slk_clear(%p)"), sp));

    if (sp != 0 && sp->_slk != 0) {
	sp->_slk->hidden = TRUE;
	/* For simulated SLK's it looks much more natural to
	   inherit those attributes from the standard screen */
	sp->_slk->win->_nc_bkgd = sp->_stdscr->_nc_bkgd;
	WINDOW_ATTRS(sp->_slk->win) = WINDOW_ATTRS(sp->_stdscr);
	if (sp->_slk->win == sp->_stdscr) {
	    rc = OK;
	} else {
	    werase(sp->_slk->win);
	    rc = wrefresh(sp->_slk->win);
	}
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
slk_clear (void)
{
    return NC_SNAME(slk_clear)(CURRENT_SCREEN);
}
