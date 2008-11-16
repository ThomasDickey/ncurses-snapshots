/****************************************************************************
 * Copyright (c) 1998-2000,2005 Free Software Foundation, Inc.              *
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
 *  Author:  Juergen Pfeifer, 1997                                          *
 *     and:  Thomas E. Dickey 2005                                          *
 ****************************************************************************/

/*
 *	lib_slkatron.c
 *	Soft key routines.
 *      Switch on labels attributes
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkatron.c,v 1.8.1.1 2008/11/16 00:19:59 juergen Exp $")

NCURSES_EXPORT(int)
NC_SNAME(slk_attron)(SCREEN *sp, const chtype attr)
{
    T((T_CALLED("slk_attron(%p,%s)"), sp, _traceattr(attr)));

    if (sp != 0 && sp->_slk != 0) {
	TR(TRACE_ATTRS, ("... current %s", _tracech_t(CHREF(sp->_slk->attr))));
	AddAttr(sp->_slk->attr, attr);
	if ((attr & A_COLOR) != 0) {
	    SetPair(sp->_slk->attr, PAIR_NUMBER(attr));
	}
	TR(TRACE_ATTRS, ("new attribute is %s", _tracech_t(CHREF(sp->_slk->attr))));
	returnCode(OK);
    } else
	returnCode(ERR);
}

NCURSES_EXPORT(int)
slk_attron (const chtype attr)
{
    return NC_SNAME(slk_attron)(CURRENT_SCREEN, attr);
}
