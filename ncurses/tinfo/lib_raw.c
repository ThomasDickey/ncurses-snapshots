/****************************************************************************
 * Copyright (c) 1998-2002,2007 Free Software Foundation, Inc.              *
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
 *     and: Thomas E. Dickey 1998 on                                        *
 ****************************************************************************/

/*
 *	raw.c
 *
 *	Routines:
 *		raw()
 *		cbreak()
 *		noraw()
 *		nocbreak()
 *		qiflush()
 *		noqiflush()
 *		intrflush()
 *
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_raw.c,v 1.14.1.1 2008/11/16 00:19:59 juergen Exp $")

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

#if HAVE_SYS_TERMIO_H
#include <sys/termio.h>		/* needed for ISC */
#endif

#ifdef __EMX__
#include <io.h>
#define _nc_setmode(sp,mode) setmode(sp->_ifd, mode)
#else
#define _nc_setmode(sp,mode)	/* nothing */
#endif

#define COOKED_INPUT	(IXON|BRKINT|PARMRK)

#ifdef TRACE
#define BEFORE(N)	if (USE_TRACEF(TRACE_BITS)) _nc_locked_tracef("%s before bits: %s", N, _nc_tracebits())
#define AFTER(N)	if (USE_TRACEF(TRACE_BITS)) _nc_locked_tracef("%s after bits: %s", N, _nc_tracebits())
#else
#define BEFORE(s)
#define AFTER(s)
#endif /* TRACE */

NCURSES_EXPORT(int)
NC_SNAME(raw)(SCREEN *sp, bool flag)
{
    int result = ERR;
    T((T_CALLED("raw(%p,%d)"), sp, flag));

    if (0!=TerminalOf(sp)) {
	TTY buf;

	BEFORE("raw");
	_nc_setmode(sp, flag ? O_BINARY : O_TEXT);

	buf = TerminalOf(sp)->Nttyb;
	if (flag) {
#ifdef TERMIOS
	    buf.c_lflag &= ~(ICANON | ISIG | IEXTEN);
	    buf.c_iflag &= ~(COOKED_INPUT);
	    buf.c_cc[VMIN] = 1;
	    buf.c_cc[VTIME] = 0;
#else
	    buf.sg_flags |= RAW;
#endif
	} else {
#ifdef TERMIOS
	    buf.c_lflag |= ISIG | ICANON |
	      (TerminalOf(sp)->Ottyb.c_lflag & IEXTEN);
	    buf.c_iflag |= COOKED_INPUT;
#else
	    buf.sg_flags &= ~(RAW | CBREAK);
#endif
	}
	if ((result = NC_SNAME(_nc_set_tty_mode)(sp, &buf)) == OK) {
	    sp->_raw = flag ? TRUE : FALSE;
	    sp->_cbreak = flag ? 1 : 0;
	    TerminalOf(sp)->Nttyb = buf;
	}
	AFTER("raw");
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
raw (void)
{
    return NC_SNAME(raw)(CURRENT_SCREEN, TRUE);
}

NCURSES_EXPORT(int)
noraw (void)
{
    return NC_SNAME(raw)(CURRENT_SCREEN, FALSE);
}

NCURSES_EXPORT(int)
NC_SNAME(cbreak)(SCREEN *sp, bool flag)
{
    int result = ERR;
    T((T_CALLED("cbreak(%p,%d)"), sp, flag));

    if (0!=TerminalOf(sp)) {
	TTY buf;

	BEFORE("cbreak");
	_nc_setmode(sp, flag ? O_BINARY : O_TEXT);

	buf = TerminalOf(sp)->Nttyb;
	if (flag) {
#ifdef TERMIOS
	    buf.c_lflag &= ~ICANON;
	    buf.c_iflag &= ~ICRNL;
	    buf.c_lflag |= ISIG;
	    buf.c_cc[VMIN] = 1;
	    buf.c_cc[VTIME] = 0;
#else
	    buf.sg_flags |= CBREAK;
#endif
	} else {
#ifdef TERMIOS
	    buf.c_lflag |= ICANON;
	    buf.c_iflag |= ICRNL;
#else
	    buf.sg_flags &= ~CBREAK;
#endif
	}
	if ((result = NC_SNAME(_nc_set_tty_mode)(sp, &buf)) == OK) {
	    sp->_cbreak = flag ? 1 : 0;
	    TerminalOf(sp)->Nttyb = buf;
	}
	AFTER("cbreak");
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
cbreak (void)
{
    return NC_SNAME(cbreak)(CURRENT_SCREEN, TRUE);
}

NCURSES_EXPORT(int)
nocbreak (void)
{
    return NC_SNAME(cbreak)(CURRENT_SCREEN, FALSE);
}

/*
 * Note:
 * this implementation may be wrong.  See the comment under intrflush().
 */
NCURSES_EXPORT(void)
NC_SNAME(qiflush)(SCREEN *sp, bool flag)
{
    int result = ERR;
    T((T_CALLED("qiflush(%p,%d)"), sp, flag));

    if (0!=TerminalOf(sp)) {
	TTY buf;

	BEFORE("qiflush");
	buf = TerminalOf(sp)->Nttyb;
	if (flag) {
#ifdef TERMIOS
	    buf.c_lflag &= ~(NOFLSH);
	    result = NC_SNAME(_nc_set_tty_mode)(sp, &buf);
#else
	    /* FIXME */
#endif
	} else {
#ifdef TERMIOS
	    buf.c_lflag |= NOFLSH;
	    result = NC_SNAME(_nc_set_tty_mode)(sp, &buf);
#else
	    /* FIXME */
#endif
	}
	if (result == OK)
  	    TerminalOf(sp)->Nttyb = buf;
	AFTER("qiflush");
    }
    returnVoid;
}

NCURSES_EXPORT(void)
qiflush (void)
{
    NC_SNAME(qiflush)(CURRENT_SCREEN, TRUE);
}

NCURSES_EXPORT(void)
noqiflush (void)
{
    NC_SNAME(qiflush)(CURRENT_SCREEN, FALSE);
}

/*
 * This call does the same thing as the qiflush()/noqiflush() pair.  We know
 * for certain that SVr3 intrflush() tweaks the NOFLSH bit; on the other hand,
 * the match (in the SVr4 man pages) between the language describing NOFLSH in
 * termio(7) and the language describing qiflush()/noqiflush() in
 * curs_inopts(3x) is too exact to be coincidence.
 */
NCURSES_EXPORT(int)
NC_SNAME(intrflush)(SCREEN *sp, WINDOW *win GCC_UNUSED, bool flag)
{
    int result = ERR;

    T((T_CALLED("intrflush(%p,%d)"), sp, flag));
    if (sp == 0)
        returnCode(ERR);

    if (0!=TerminalOf(sp)) {
	TTY buf;

	BEFORE("intrflush");
	buf = TerminalOf(sp)->Nttyb;
#ifdef TERMIOS
	if (flag)
	    buf.c_lflag &= ~(NOFLSH);
	else
	    buf.c_lflag |= (NOFLSH);
	result = NC_SNAME(_nc_set_tty_mode)(sp, &buf);
#else
	/* FIXME */
#endif
	if (result == OK) {
  	    TerminalOf(sp)->Nttyb = buf;
	}
	AFTER("intrflush");
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
intrflush(WINDOW *win GCC_UNUSED, bool flag)
{
  return NC_SNAME(intrflush)(CURRENT_SCREEN, win, flag);
}
