/****************************************************************************
 * Copyright (c) 1998-2007,2008 Free Software Foundation, Inc.              *
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

/*
 * Terminfo-only terminal setup routines:
 *
 *		int restartterm(const char *, int, int *)
 */

#include <curses.priv.h>

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

MODULE_ID("$Id: lib_restart.c,v 1.10.1.2 2009/02/07 23:09:40 tom Exp $")

NCURSES_EXPORT(int)
NC_SNAME(_nc_restartterm) (SCREEN *sp, NCURSES_CONST char *termp, int
			   filenum, int *errret)
{
    int result;
    TERMINAL *new_term;
    T((T_CALLED("restartterm(%p,%s,%d,%p)"), sp, termp, filenum, errret));

    if (_nc_setupterm_ex(&new_term, termp, filenum, errret, FALSE) != OK) {
	result = ERR;
    } else if (sp != 0) {
	int saveecho = sp->_echo;
	int savecbreak = sp->_cbreak;
	int saveraw = sp->_raw;
	int savenl = sp->_nl;

	sp->_term = new_term;
	NC_SNAME(echo) (sp, saveecho);

	if (savecbreak) {
	    NC_SNAME(cbreak) (sp, TRUE);
	    NC_SNAME(raw) (sp, FALSE);
	} else if (saveraw) {
	    NC_SNAME(cbreak) (sp, FALSE);
	    NC_SNAME(raw) (sp, TRUE);
	} else {
	    NC_SNAME(cbreak) (sp, FALSE);
	    NC_SNAME(raw) (sp, FALSE);
	}
	NC_SNAME(nl) (sp, savenl);

	NC_SNAME(reset_prog_mode) (sp);

#if USE_SIZECHANGE
	_nc_update_screensize(sp);
#endif

	result = OK;
    } else {
	result = ERR;
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
restartterm(NCURSES_CONST char *termp, int filenum, int *errret)
{
    return NC_SNAME(_nc_restartterm) (CURRENT_SCREEN, termp, filenum, errret);
}
