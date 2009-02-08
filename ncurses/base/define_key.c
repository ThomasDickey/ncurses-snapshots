/****************************************************************************
 * Copyright (c) 1998-2005,2006 Free Software Foundation, Inc.              *
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
 *  Author: Thomas E. Dickey                    1997-on                     *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: define_key.c,v 1.13.1.2 2009/02/07 23:09:39 tom Exp $")

NCURSES_EXPORT(int)
NC_SNAME(define_key) (SCREEN *sp, const char *str, int keycode)
{
    int code = ERR;

    T((T_CALLED("define_key(%p, %s,%d)"), sp, _nc_visbuf(str), keycode));
    if (sp == 0 || !HasTInfoTerminal(sp)) {
	code = ERR;
    } else if (keycode > 0) {
	unsigned ukey = (unsigned) keycode;

	if (str != 0) {
	    NC_SNAME(define_key) (sp, str, 0);
	} else if (NC_SNAME(has_key) (sp, keycode)) {
	    while (_nc_remove_key(&(sp->_keytry), ukey))
		code = OK;
	}
	if (str != 0) {
	    if (NC_SNAME(key_defined) (sp, str) == 0) {
		if (_nc_add_to_try(&(sp->_keytry), str, ukey) == OK) {
		    code = OK;
		} else {
		    code = ERR;
		}
	    } else {
		code = ERR;
	    }
	}
    } else {
	while (_nc_remove_string(&(sp->_keytry), str))
	    code = OK;
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
define_key(const char *str, int keycode)
{
    return NC_SNAME(define_key) (CURRENT_SCREEN, str, keycode);
}
