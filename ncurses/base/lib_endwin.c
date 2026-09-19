/****************************************************************************
 * Copyright 2020-2025,2026 Thomas E. Dickey                                *
 * Copyright 1998-2014,2017 Free Software Foundation, Inc.                  *
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
 *     and: Juergen Pfeifer                         2009                    *
 ****************************************************************************/

/*
**	lib_endwin.c
**
**	The routine endwin().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_endwin.c,v 1.34 2026/09/19 23:04:39 tom Exp $")

NCURSES_EXPORT(int)
NCURSES_SP_NAME(endwin)(NCURSES_SP_DCL0)
{
    int code = ERR;

    T((T_CALLED("endwin(%p)"), (void *) SP_PARM));

    if (SP_PARM != NULL) {
	bool save_keypad = (stdscr != NULL) ? stdscr->_use_keypad : FALSE;
	bool save_meta = SP_PARM->_use_meta;
	bool suspended = FALSE;

	if (SP_PARM->_endwin != ewSuspend) {
#if USE_TERM_DRIVER
	    TERMINAL_CONTROL_BLOCK *TCB = TCBOf(SP_PARM);

	    SP_PARM->_endwin = ewSuspend;
	    if (TCB && TCB->drv && TCB->drv->td_scexit)
		TCB->drv->td_scexit(SP_PARM);
#else
	    SP_PARM->_endwin = ewSuspend;
	    SP_PARM->_mouse_wrap(SP_PARM);

	    NCURSES_SP_NAME(_nc_screen_wrap)(NCURSES_SP_ARG);
	    NCURSES_SP_NAME(_nc_mvcur_wrap)(NCURSES_SP_ARG);
#endif
	    suspended = TRUE;
	    code = OK;
	}
	if (NCURSES_SP_NAME(reset_shell_mode)(NCURSES_SP_ARG) == ERR)
	    code = ERR;
	/*
	 * reset_shell_mode resets the terminal's keypad and meta modes.
	 * If we did this as part of suspending screen mode, keep the
	 * "use" flags in the data structure, for reference when restoring
	 * screen mode in doupdate(), e.g., as called from refresh().
	 */
	if (suspended) {
	    if (stdscr != NULL)
		stdscr->_use_keypad = save_keypad;

	    SP_PARM->_use_meta = save_meta;
	}
    }

    returnCode(code);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
endwin(void)
{
    return NCURSES_SP_NAME(endwin)(CURRENT_SCREEN);
}
#endif
