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

/*
 * Terminfo-only terminal setup routines:
 *
 *		int restartterm(const char *, int, int *)
 */

#include <curses.priv.h>

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

MODULE_ID("$Id: lib_restart.c,v 1.10.1.3 2009/02/14 20:57:17 tom Exp $")

NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_restartterm) (SCREEN *sp,
				  NCURSES_CONST char *termp,
				  int filenum,
				  int *errret)
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
	if (saveecho) {
	    NCURSES_SP_NAME(echo) (sp);
	} else {
	    NCURSES_SP_NAME(noecho) (sp);
	}

	if (savecbreak) {
	    NCURSES_SP_NAME(cbreak) (sp);
	    NCURSES_SP_NAME(noraw) (sp);
	} else if (saveraw) {
	    NCURSES_SP_NAME(nocbreak) (sp);
	    NCURSES_SP_NAME(raw) (sp);
	} else {
	    NCURSES_SP_NAME(nocbreak) (sp);
	    NCURSES_SP_NAME(noraw) (sp);
	}
	if (savenl) {
	    NCURSES_SP_NAME(nl) (sp);
	} else {
	    NCURSES_SP_NAME(nonl) (sp);
	}

	NCURSES_SP_NAME(reset_prog_mode) (sp);

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
    return NCURSES_SP_NAME(_nc_restartterm) (CURRENT_SCREEN, termp, filenum, errret);
}
