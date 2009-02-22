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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/
/*
 * Module that "owns" the 'cur_term' variable:
 *
 *	TERMINAL *set_curterm(TERMINAL *)
 *	int del_curterm(TERMINAL *)
 */

#include <curses.priv.h>
#include <term_entry.h>		/* TTY, cur_term */
#include <termcap.h>		/* ospeed */

MODULE_ID("$Id: lib_cur_term.c,v 1.18.1.4 2009/02/21 15:11:29 tom Exp $")

#undef CUR
#define CUR termp->type.

NCURSES_EXPORT(TERMINAL *)
NCURSES_SP_NAME(cur_term) (SCREEN *sp)
{
    return ((0 != TerminalOf(sp)) ?
	    TerminalOf(sp) :
#if BROKEN_LINKER || USE_REENTRANT
	    _nc_prescreen._cur_term
#else
	    cur_term
#endif
	);
}

#if BROKEN_LINKER || USE_REENTRANT
NCURSES_EXPORT(TERMINAL *)
NCURSES_PUBLIC_VAR(cur_term) (void)
{
    return NCURSES_SP_NAME(cur_term) (CURRENT_SCREEN);
}
#else
NCURSES_EXPORT_VAR(TERMINAL *) cur_term = 0;
#endif

NCURSES_EXPORT(TERMINAL *)
NCURSES_SP_NAME(_nc_set_curterm) (SCREEN *sp, TERMINAL * termp)
{
    TERMINAL *oldterm;

    T((T_CALLED("set_curterm(%p)"), termp));

    _nc_lock_global(curses);
    oldterm = cur_term;
    if (sp)
	sp->_term = termp;
#if BROKEN_LINKER || USE_REENTRANT
    _nc_prescreen._cur_term = termp;
#else
    cur_term = termp;
#endif
    if (termp != 0) {
	TERMINAL_CONTROL_BLOCK *TCB = (TERMINAL_CONTROL_BLOCK *) termp;
	ospeed = _nc_ospeed(termp->_baudrate);
	if (TCB->drv->isTerminfo && termp->type.Strings) {
	    PC = (char) ((pad_char != NULL) ? pad_char[0] : 0);
	}
	TCB->csp = sp;
    }
    _nc_unlock_global(curses);

    T((T_RETURN("%p"), oldterm));
    return (oldterm);
}

NCURSES_EXPORT(TERMINAL *)
set_curterm(TERMINAL * termp)
{
    return NCURSES_SP_NAME(_nc_set_curterm) (CURRENT_SCREEN, termp);
}

NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_del_curterm) (SCREEN *sp, TERMINAL * termp)
{
    int rc = ERR;

    T((T_CALLED("del_curterm(%p, %p)"), sp, termp));

    if (termp != 0) {
	TERMINAL_CONTROL_BLOCK *TCB = (TERMINAL_CONTROL_BLOCK *) termp;
	TERMINAL *cur =
#if BROKEN_LINKER || USE_REENTRANT
	NCURSES_SP_NAME(cur_term) (sp);
#else
	cur_term;
#endif

	_nc_free_termtype(&(termp->type));
	if (termp == cur)
	    NCURSES_SP_NAME(_nc_set_curterm) (sp, 0);

	FreeIfNeeded(termp->_termname);
	if (TCB->drv)
	    TCB->drv->release(TCB);
	free(termp);

	rc = OK;
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
del_curterm(TERMINAL * termp)
{
    int rc = ERR;

    _nc_lock_global(curses);
    rc = NCURSES_SP_NAME(_nc_del_curterm) (CURRENT_SCREEN, termp);
    _nc_unlock_global(curses);

    return (rc);
}
