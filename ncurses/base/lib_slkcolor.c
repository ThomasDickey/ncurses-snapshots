/****************************************************************************
 * Copyright (c) 1998-2005,2009 Free Software Foundation, Inc.              *
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
 *  Author:  Juergen Pfeifer, 1998                                          *
 *     and:  Thomas E. Dickey 2005-on                                       *
 ****************************************************************************/

/*
 *	lib_slkcolor.c
 *	Soft key routines.
 *	Set the label's color
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkcolor.c,v 1.13.1.2 2009/02/07 22:35:50 tom Exp $")

NCURSES_EXPORT(int)
NC_SNAME(slk_color) (SCREEN *sp, short color_pair_number)
{
    int code = ERR;

    T((T_CALLED("slk_color(%p,%d)"), sp, color_pair_number));

    if (sp != 0
	&& sp->_slk != 0
	&& color_pair_number >= 0
	&& color_pair_number < sp->_pair_limit) {
	TR(TRACE_ATTRS, ("... current is %s", _tracech_t(CHREF(sp->_slk->attr))));
	SetPair(sp->_slk->attr, color_pair_number);
	TR(TRACE_ATTRS, ("new attribute is %s", _tracech_t(CHREF(sp->_slk->attr))));
	code = OK;
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
slk_color(short color_pair_number)
{
    return NC_SNAME(slk_color) (CURRENT_SCREEN, color_pair_number);
}
