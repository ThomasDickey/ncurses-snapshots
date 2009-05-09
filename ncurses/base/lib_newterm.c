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
 *     and: Juergen Pfeifer                         2009                    *
 ****************************************************************************/

/*
**	lib_newterm.c
**
**	The newterm() function.
**
*/

#include <curses.priv.h>

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

#include <tic.h>

MODULE_ID("$Id: lib_newterm.c,v 1.75.1.1 2009/04/25 17:57:42 tom Exp $")

#ifndef ONLCR			/* Allows compilation under the QNX 4.2 OS */
#define ONLCR 0
#endif

/*
 * SVr4/XSI Curses specify that hardware echo is turned off in initscr, and not
 * restored during the curses session.  The library simulates echo in software.
 * (The behavior is unspecified if the application enables hardware echo).
 *
 * The newterm function also initializes terminal settings, and since initscr
 * is supposed to behave as if it calls newterm, we do it here.
 */
static NCURSES_INLINE int
_nc_initscr(NCURSES_SP_DCL0)
{
    int result = ERR;
    TERMINAL *term = TerminalOf(SP_PARM);

    /* for extended XPG4 conformance requires cbreak() at this point */
    /* (SVr4 curses does this anyway) */
    if (NCURSES_SP_NAME(cbreak) (NCURSES_SP_ARG) == OK) {
	TTY buf;

	buf = term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag &= ~(ECHO | ECHONL);
	buf.c_iflag &= ~(ICRNL | INLCR | IGNCR);
	buf.c_oflag &= ~(ONLCR);
#elif HAVE_SGTTY_H
	buf.sg_flags &= ~(ECHO | CRMOD);
#else
	memset(&buf, 0, sizeof(buf));
#endif
	result = NCURSES_SP_NAME(_nc_set_tty_mode) (NCURSES_SP_ARGx &buf);
	if (result == OK)
	    term->Nttyb = buf;
    }
    return result;
}

NCURSES_EXPORT(void)
NCURSES_SP_NAME(filter) (NCURSES_SP_DCL)
{
    START_TRACE();
    T((T_CALLED("filter(%p,%d)"), SP_PARM));
    if (IsPreScreen(SP_PARM)) {
	SP_PARM->_filtered = TRUE;
    }
    returnVoid;
}

/*
 * filter() has to be called before either initscr() or newterm(), so there is
 * apparently no way to make this flag apply to some terminals and not others,
 * aside from possibly delaying a filter() call until some terminals have been
 * initialized.
 */
#if NCURSES_SP_FUNCS
NCURSES_EXPORT(void)
filter(void)
{
    START_TRACE();
    T((T_CALLED("filter")));
    _nc_prescreen.filter_mode = TRUE;
    returnVoid;
}
#endif

#if NCURSES_EXT_FUNCS
/*
 * An extension, allowing the application to open a new screen without
 * requiring it to also be filtered.
 */
NCURSES_EXPORT(void)
nofilter(void)
{
    START_TRACE();
    T((T_CALLED("nofilter")));
    _nc_prescreen.filter_mode = FALSE;
    returnVoid;
}
#endif

NCURSES_EXPORT(SCREEN *)
NCURSES_SP_NAME(newterm) (NCURSES_SP_DCLx
			  NCURSES_CONST char *name,
			  FILE *ofp,
			  FILE *ifp)
{
    int value;
    int errret;
    SCREEN *result = 0;
    SCREEN *current;
    TERMINAL *its_term;
    TERMINAL *new_term;
    FILE *_ofp = ofp ? ofp : stdout;
    FILE *_ifp = ifp ? ifp : stdin;
    int cols;
    int numlab;
    bool code = FALSE;

    START_TRACE();
    T((T_CALLED("newterm(%p, \"%s\", %p,%p)"), SP_PARM, name, ofp, ifp));

    assert(SP_PARM != 0);
    if (SP_PARM == 0)
	returnSP(SP_PARM);

    _nc_init_pthreads();
    _nc_lock_global(curses);

    current = CURRENT_SCREEN;
    its_term = (current ? current->_term : 0);

    /* this loads the capability entry, then sets LINES and COLS */
    if (SP_PARM->_prescreen &&
	_nc_setupterm_ex(&new_term, name,
			 fileno(_ofp), &errret, FALSE) != ERR) {

	_nc_set_screen(0);
	assert(new_term != 0);
	/*
	 * This actually allocates the screen structure, and saves the original
	 * terminal settings.
	 */
	if (NCURSES_SP_NAME(_nc_setupscreen) (&SP_PARM,
					      *(ptrLines(SP_PARM)),
					      *(ptrCols(SP_PARM)),
					      _ofp,
					      SP_PARM->_filtered,
					      SP_PARM->slk_format) == ERR) {
	    _nc_set_screen(current);
	    result = 0;
	} else {
	    TERMINAL_CONTROL_BLOCK *TCB;
	    assert(SP_PARM != 0);
	    _nc_set_screen(SP_PARM);
	    cols = *(ptrCols(SP_PARM));
	    TCB = (TERMINAL_CONTROL_BLOCK *) new_term;
	    TCB->csp = SP_PARM;
	    numlab = InfoOf(SP_PARM).numlabels;

	    /*
	     * In setupterm() we did a set_curterm(), but it was before we set
	     * CURRENT_SCREEN.  So the "current" screen's terminal pointer was overwritten
	     * with a different terminal.  Later, in _nc_setupscreen(), we set
	     * CURRENT_SCREEN and the terminal pointer in the new screen.
	     *
	     * Restore the terminal-pointer for the pre-existing screen, if
	     * any.
	     */

	    if (current)
		current->_term = its_term;

	    SP_PARM->_term = new_term;

	    /* allow user to set maximum escape delay from the environment */
	    if ((value = _nc_getenv_num("ESCDELAY")) >= 0) {
#if USE_REENTRANT
		NCURSES_SP_NAME(set_escdelay) (SP_PARM, value);
#else
		ESCDELAY = value;
#endif
	    }

	    /* if the terminal type has real soft labels, set those up */
	    if (SP_PARM->slk_format && numlab > 0 && SLK_STDFMT(SP_PARM->slk_format))
		_nc_slk_initialize(SP_PARM->_stdscr, cols);

	    SP_PARM->_ifd = fileno(_ifp);
	    NCURSES_SP_NAME(typeahead) (SP_PARM, fileno(_ifp));
#ifdef TERMIOS
	    SP_PARM->_use_meta = ((SP_PARM->_term->Ottyb.c_cflag & CSIZE) ==
				  CS8 &&
				  !(SP_PARM->_term->Ottyb.c_iflag & ISTRIP));
#else
	    SP_PARM->_use_meta = FALSE;
#endif
	    SP_PARM->_endwin = FALSE;

	    NCURSES_SP_NAME(baudrate) (SP_PARM);	/* sets a field in the screen structure */

	    SP_PARM->_keytry = 0;

	    /* compute movement costs so we can do better move optimization */
	    TCBOf(SP_PARM)->drv->scinit(SP_PARM);

	    /* Initialize the terminal line settings. */
	    _nc_initscr(SP_PARM);

	    _nc_signal_handler(TRUE);
	    result = SP_PARM;
	    code = TRUE;
	}
    }
    _nc_unlock_global(curses);
    returnSP(SP_PARM);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(SCREEN *)
newterm(NCURSES_CONST char *name, FILE *ofp, FILE *ifp)
{
    SCREEN *sp = CURRENT_SCREEN_PRE;
    if (NCURSES_SP_NAME(newterm) (sp, name, ofp, ifp))
	return sp;
    else
	return (SCREEN *) 0;
}
#endif
