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

/*
 *		def_prog_mode()
 *		def_shell_mode()
 *		reset_prog_mode()
 *		reset_shell_mode()
 *		savetty()
 *		resetty()
 */

#include <curses.priv.h>
#define CUR TerminalOf(sp)->type.

MODULE_ID("$Id: lib_ttyflags.c,v 1.18.1.1 2008/11/16 00:19:59 juergen Exp $")

NCURSES_EXPORT(int)
NC_SNAME(_nc_get_tty_mode)(SCREEN *sp, TTY * buf)
{
    int result = OK;

    if (buf == 0 || sp==0) {
	result = ERR;
    } else {
	if (0==TerminalOf(sp)) {
	    result = ERR;
	} else {
	    result = CallDriver_2(sp,sgmode,FALSE,buf);
	}

	if (result == ERR)
	    memset(buf, 0, sizeof(*buf));

	TR(TRACE_BITS, ("_nc_get_tty_mode(%d): %s",
			sp->_term ? sp->_term->Filedes : -1,
			_nc_trace_ttymode(buf)));
    }
    return (result);
}

NCURSES_EXPORT(int)
_nc_get_tty_mode (TTY * buf)
{
    return NC_SNAME(_nc_get_tty_mode)(CURRENT_SCREEN, buf);
}

NCURSES_EXPORT(int)
NC_SNAME(_nc_set_tty_mode)(SCREEN *sp, TTY * buf)
{
    int result = OK;

    if (buf == 0 || sp==0) {
	result = ERR;
    } else {
	if (0==TerminalOf(sp)) {
	    result = ERR;
	} else {
	    result = CallDriver_2(sp,sgmode,TRUE,buf);
	}
	TR(TRACE_BITS, ("_nc_set_tty_mode(%d): %s",
			sp->_term ? sp->_term->Filedes : -1,
			_nc_trace_ttymode(buf)));
    }
    return (result);
}

NCURSES_EXPORT(int)
_nc_set_tty_mode (TTY * buf)
{
    return NC_SNAME(_nc_set_tty_mode)(CURRENT_SCREEN, buf);
}

NCURSES_EXPORT(int)
NC_SNAME(def_shell_mode)(SCREEN *sp)
{
    int rc = ERR;

    T((T_CALLED("def_shell_mode(%p)"),sp));

    if (0==sp)
      returnCode(rc);

    if (0!=TerminalOf(sp)) {
        rc = CallDriver_2(sp,mode,FALSE,TRUE);
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
def_shell_mode (void)
{
    return NC_SNAME(def_shell_mode)(CURRENT_SCREEN);
}

NCURSES_EXPORT(int)
NC_SNAME(def_prog_mode)(SCREEN *sp)
{
  int rc = ERR;
  
  T((T_CALLED("def_prog_mode(%p)"),sp));
  
    if (0==sp)
      returnCode(rc);

    if (0!=TerminalOf(sp)) {
        rc = CallDriver_2(sp,mode,TRUE,TRUE);
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
def_prog_mode (void)
{
    return NC_SNAME(def_prog_mode)(CURRENT_SCREEN);
}

NCURSES_EXPORT(int)
NC_SNAME(reset_prog_mode)(SCREEN *sp)
{
    int rc = ERR;

    T((T_CALLED("reset_prog_mode(%p)"), sp));

    if (0==sp)
      returnCode(rc);

    if (0!=TerminalOf(sp)) {
        rc = CallDriver_2(sp,mode,TRUE,FALSE);
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
reset_prog_mode (void)
{
    return NC_SNAME(reset_prog_mode)(CURRENT_SCREEN);
}

NCURSES_EXPORT(int)
NC_SNAME(reset_shell_mode)(SCREEN *sp)
{
    int rc = ERR;

    T((T_CALLED("reset_shell_mode(%p)"), sp));

    if (0==sp)
      returnCode(rc);

    if (0!=TerminalOf(sp)) {
        rc = CallDriver_2(sp,mode,FALSE,FALSE);
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
reset_shell_mode (void)
{
    return NC_SNAME(reset_shell_mode)(CURRENT_SCREEN);
}

static TTY *
saved_tty(SCREEN *sp)
{
    TTY *result = 0;

    if (sp != 0) {
      result = (TTY*)&(sp->_saved_tty);
    } else {
	if (_nc_prescreen.saved_tty == 0) {
	    _nc_prescreen.saved_tty = typeCalloc(TTY, 1);
	}
	result = _nc_prescreen.saved_tty;
    }
    return result;
}

/*
**	savetty()  and  resetty()
**
*/

NCURSES_EXPORT(int)
NC_SNAME(_nc_savetty)(SCREEN *sp)
{
    T((T_CALLED("savetty(%p)"), sp));
    returnCode(NC_SNAME(_nc_get_tty_mode)(sp, saved_tty(sp)));
}

NCURSES_EXPORT(int)
savetty (void)
{
    return NC_SNAME(_nc_savetty)(CURRENT_SCREEN);
}

NCURSES_EXPORT(int)
NC_SNAME(_nc_resetty)(SCREEN *sp)
{
    T((T_CALLED("resetty(%p)"), sp));
    returnCode(NC_SNAME(_nc_set_tty_mode)(sp, saved_tty(sp)));
}

NCURSES_EXPORT(int)
resetty (void)
{
    return NC_SNAME(_nc_resetty)(CURRENT_SCREEN);
}
