/****************************************************************************
 * Copyright (c) 1998-2000,2003 Free Software Foundation, Inc.              *
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
 *     and: Thomas E. Dickey                        1996-2003               *
 ****************************************************************************/

/*
**	lib_has_cap.c
**
**	The routines to query terminal capabilities
**
*/

#include <curses.priv.h>
#define CUR TerminalOf(sp)->type.

MODULE_ID("$Id: lib_has_cap.c,v 1.4.1.1 2008/11/16 00:19:59 juergen Exp $")

NCURSES_EXPORT(bool)
NC_SNAME(has_ic)(SCREEN *sp)
{
    bool code = FALSE;
  
    T((T_CALLED("has_ic(%p)"),sp));

    if (IsValidTIScreen(sp) && IsTermInfo(sp))
    {
      code = ((insert_character || parm_ich
		      || (enter_insert_mode && exit_insert_mode))
	      && (delete_character || parm_dch)) ? TRUE : FALSE;
    }

    returnCode(code);
}

NCURSES_EXPORT(bool)
has_ic (void)
{
    return NC_SNAME(has_ic)(CURRENT_SCREEN);
}

NCURSES_EXPORT(bool)
NC_SNAME(has_il)(SCREEN *sp)
{
    bool code = FALSE;
    T((T_CALLED("has_il(%p)"),sp));
    if (IsValidTIScreen(sp) && IsTermInfo(sp))
    {
      code = ((insert_line || parm_insert_line)
	      && (delete_line || parm_delete_line)) ? TRUE : FALSE;
    }

    returnCode(code);
}

NCURSES_EXPORT(bool)
has_il (void)
{
    return NC_SNAME(has_il)(CURRENT_SCREEN);
}
