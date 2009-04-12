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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 * This is an extension to the curses library.  It provides callers with a hook
 * into the NCURSES data to resize windows, primarily for use by programs
 * running in an X Window terminal (e.g., xterm).  I abstracted this module
 * from my application library for NCURSES because it must be compiled with
 * the private data structures -- T.Dickey 1995/7/4.
 */

#include <curses.priv.h>

MODULE_ID("$Id: resizeterm.c,v 1.34.1.3 2009/02/21 15:46:16 tom Exp $")

/*
 * If we're trying to be reentrant, do not want any local statics.
 */
#if USE_REENTRANT
#define EXTRA_ARGS ,     CurLines,     CurCols
#define EXTRA_DCLS , int CurLines, int CurCols
#else
static int current_lines;
static int current_cols;
#define CurLines current_lines
#define CurCols  current_cols
#define EXTRA_ARGS		/* nothing */
#define EXTRA_DCLS		/* nothing */
#endif

#ifdef TRACE
static void
show_window_sizes(const char *name)
{
    SCREEN *sp;
    WINDOWLIST *wp;

    _nc_lock_global(curses);
    for (each_screen(sp)) {
	_tracef("%s resizing: %p: %2d x %2d (%2d x %2d)", name, sp,
		*(ptrLines(sp)),
		*(ptrCols(sp)),
		screen_lines(sp), screen_columns(sp));
	for (each_window(sp, wp)) {
	    _tracef("  window %p is %2ld x %2ld at %2ld,%2ld",
		    &(wp->win),
		    (long) wp->win._maxy + 1,
		    (long) wp->win._maxx + 1,
		    (long) wp->win._begy,
		    (long) wp->win._begx);
	}
    }
    _nc_unlock_global(curses);
}
#endif

/*
 * Return true if the given dimensions do not match the internal terminal
 * structure's size.
 */
NCURSES_EXPORT(bool)
NCURSES_SP_NAME(_nc_is_term_resized) (SCREEN *sp, int ToLines, int ToCols)
{
    T((T_CALLED("_nc_is_term_resized(%p, %d, %d)"), sp, ToLines, ToCols));
    returnCode(ToLines > 0
	       && ToCols > 0
	       && (ToLines != screen_lines(sp)
		   || ToCols != screen_columns(sp)));
}

NCURSES_EXPORT(bool)
is_term_resized(int ToLines, int ToCols)
{
    return NCURSES_SP_NAME(_nc_is_term_resized) (CURRENT_SCREEN, ToLines, ToCols);
}

/*
 */
static ripoff_t *
ripped_window(WINDOW *win)
{
    ripoff_t *result = 0;
    ripoff_t *rop;
    if (win != 0) {
	SCREEN *sp = _nc_screen_of(win);
	for (each_ripoff(rop)) {
	    if (rop->win == win && rop->line != 0) {
		result = rop;
		break;
	    }
	}
    }
    return result;
}

/*
 * Returns the number of lines from the bottom for the beginning of a ripped
 * off window.
 */
static int
ripped_bottom(WINDOW *win)
{
    int result = 0;
    ripoff_t *rop;

    if (win != 0) {
	SCREEN *sp = _nc_screen_of(win);
	for (each_ripoff(rop)) {
	    if (rop->line < 0) {
		result -= rop->line;
		if (rop->win == win) {
		    break;
		}
	    }
	}
    }
    return result;
}

/*
 * Return the number of levels of child-windows under the current window.
 */
static int
child_depth(WINDOW *cmp)
{
    int depth = 0;

    if (cmp != 0) {
	SCREEN *sp = _nc_screen_of(cmp);
	WINDOWLIST *wp;

	for (each_window(sp, wp)) {
	    WINDOW *tst = &(wp->win);
	    if (tst->_parent == cmp) {
		depth = 1 + child_depth(tst);
		break;
	    }
	}
    }
    return depth;
}

/*
 * Return the number of levels of parent-windows above the current window.
 */
static int
parent_depth(WINDOW *cmp)
{
    int depth = 0;

    if (cmp != 0) {
	WINDOW *tst;
	while ((tst = cmp->_parent) != 0) {
	    ++depth;
	    cmp = tst;
	}
    }
    return depth;
}

/*
 * FIXME: must adjust position so it's within the parent!
 */
static int
adjust_window(WINDOW *win, int ToLines, int ToCols, int stolen EXTRA_DCLS)
{
    int result;
    int bottom = CurLines + _nc_screen_of(win)->_topstolen - stolen;
    int myLines = win->_maxy + 1;
    int myCols = win->_maxx + 1;
    ripoff_t *rop = ripped_window(win);

    T((T_CALLED("adjust_window(%p,%d,%d)%s depth %d/%d currently %ldx%ld at %ld,%ld"),
       win, ToLines, ToCols,
       (rop != 0) ? " (rip)" : "",
       parent_depth(win),
       child_depth(win),
       (long) getmaxy(win), (long) getmaxx(win),
       (long) getbegy(win) + win->_yoffset, (long) getbegx(win)));

    if (rop != 0 && rop->line < 0) {
	/*
	 * If it is a ripped-off window at the bottom of the screen, simply
	 * move it to the same relative position.
	 */
	win->_begy = ToLines - ripped_bottom(win) - 0 - win->_yoffset;
	if (rop->hook == _nc_slk_initialize)
	    _nc_format_slks(_nc_screen_of(win), ToCols);
    } else if (win->_begy >= bottom) {
	/*
	 * If it is below the bottom of the new screen, move up by the same
	 * amount that the screen shrank.
	 */
	win->_begy += (ToLines - CurLines);
    } else {
	if (myLines == (CurLines - stolen)
	    && ToLines != CurLines) {
	    myLines = ToLines - stolen;
	} else if (myLines == CurLines
		   && ToLines != CurLines) {
	    myLines = ToLines;
	}
    }

    if (myLines > ToLines) {
	myLines = ToLines;
    }

    if (myCols > ToCols)
	myCols = ToCols;

    if (myCols == CurCols
	&& ToCols != CurCols)
	myCols = ToCols;

    result = wresize(win, myLines, myCols);
    returnCode(result);
}

/*
 * If we're decreasing size, recursively search for windows that have no
 * children, decrease those to fit, then decrease the containing window, etc.
 */
static int
decrease_size(SCREEN *sp, int ToLines, int ToCols, int stolen EXTRA_DCLS)
{
    bool found;
    int depth = 0;
    WINDOWLIST *wp;

    T((T_CALLED("decrease_size(%p, %d, %d)"), sp, ToLines, ToCols));

    do {
	found = FALSE;
	TR(TRACE_UPDATE, ("decreasing size of windows to %dx%d, depth=%d",
			  ToLines, ToCols, depth));
	for (each_window(sp, wp)) {
	    WINDOW *win = &(wp->win);

	    if (!(win->_flags & _ISPAD)) {
		if (child_depth(win) == depth) {
		    found = TRUE;
		    if (adjust_window(win, ToLines, ToCols,
				      stolen EXTRA_ARGS) != OK)
			returnCode(ERR);
		}
	    }
	}
	++depth;
    } while (found);
    returnCode(OK);
}

/*
 * If we're increasing size, recursively search for windows that have no
 * parent, increase those to fit, then increase the contained window, etc.
 */
static int
increase_size(SCREEN *sp, int ToLines, int ToCols, int stolen EXTRA_DCLS)
{
    bool found;
    int depth = 0;
    WINDOWLIST *wp;

    T((T_CALLED("increase_size(%p, %d, %d)"), sp, ToLines, ToCols));

    do {
	found = FALSE;
	TR(TRACE_UPDATE, ("increasing size of windows to %dx%d, depth=%d",
			  ToLines, ToCols, depth));
	for (each_window(sp, wp)) {
	    WINDOW *win = &(wp->win);

	    if (!(win->_flags & _ISPAD)) {
		if (parent_depth(win) == depth) {
		    found = TRUE;
		    if (adjust_window(win, ToLines, ToCols,
				      stolen EXTRA_ARGS) != OK)
			returnCode(ERR);
		}
	    }
	}
	++depth;
    } while (found);
    returnCode(OK);
}

/*
 * This function reallocates NCURSES window structures, with no side-effects
 * such as ungetch().
 */
NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_resize_term) (SCREEN *sp, int ToLines, int ToCols)
{
    int result = OK EXTRA_ARGS;
    int was_stolen;

    T((T_CALLED("_nc_resize_term(%p,%d,%d) old(%d,%d)"),
       sp, ToLines, ToCols,
       sp == 0 ? -1 : screen_lines(sp), sp == 0 ? -1 : screen_columns(sp)));

    if (sp == 0) {
	returnCode(ERR);
    }

    was_stolen = (screen_lines(sp) - sp->_lines_avail);
    if (NCURSES_SP_NAME(_nc_is_term_resized) (sp, ToLines, ToCols)) {
	int myLines = CurLines = screen_lines(sp);
	int myCols = CurCols = screen_columns(sp);

#ifdef TRACE
	if (USE_TRACEF(TRACE_UPDATE)) {
	    show_window_sizes("before");
	    _nc_unlock_global(tracef);
	}
#endif
	if (ToLines > screen_lines(sp)) {
	    increase_size(sp, myLines = ToLines, myCols, was_stolen EXTRA_ARGS);
	    CurLines = myLines;
	    CurCols = myCols;
	}

	if (ToCols > screen_columns(sp)) {
	    increase_size(sp, myLines, myCols = ToCols, was_stolen EXTRA_ARGS);
	    CurLines = myLines;
	    CurCols = myCols;
	}

	if (ToLines < myLines ||
	    ToCols < myCols) {
	    decrease_size(sp, ToLines, ToCols, was_stolen EXTRA_ARGS);
	}

	screen_lines(sp) = ToLines;
	screen_columns(sp) = ToCols;

	CallDriver_2(sp, setsize, ToLines, ToCols);

	sp->_lines_avail = ToLines - was_stolen;

	if (sp->oldhash) {
	    FreeAndNull(sp->oldhash);
	}
	if (sp->newhash) {
	    FreeAndNull(sp->newhash);
	}
#ifdef TRACE
	if (USE_TRACEF(TRACE_UPDATE)) {
	    SET_LINES(ToLines - was_stolen);
	    SET_COLS(ToCols);
	    show_window_sizes("after");
	    _nc_unlock_global(tracef);
	}
#endif
    }

    /*
     * Always update LINES, to allow for call from lib_doupdate.c which
     * needs to have the count adjusted by the stolen (ripped off) lines.
     */
    SET_LINES(ToLines - was_stolen);
    SET_COLS(ToCols);

    returnCode(result);
}

NCURSES_EXPORT(int)
resize_term(int ToLines, int ToCols)
{
    int res = ERR;
    _nc_lock_global(curses);
    res = NCURSES_SP_NAME(_nc_resize_term) (CURRENT_SCREEN, ToLines, ToCols);
    _nc_unlock_global(curses);
    return (res);
}

/*
 * This function reallocates NCURSES window structures.  It is invoked in
 * response to a SIGWINCH interrupt.  Other user-defined windows may also need
 * to be reallocated.
 *
 * Because this performs memory allocation, it should not (in general) be
 * invoked directly from the signal handler.
 */
NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_resizeterm) (SCREEN *sp, int ToLines, int ToCols)
{
    int result = ERR;

    T((T_CALLED("_nc_resizeterm(%p, %d,%d) old(%d,%d)"),
       sp, ToLines, ToCols,
       sp == 0 ? -1 : screen_lines(sp), screen_columns(sp)));

    if (sp != 0) {
	result = OK;
	sp->_sig_winch = FALSE;

	if (NCURSES_SP_NAME(_nc_is_term_resized) (sp, ToLines, ToCols)) {
#if USE_SIGWINCH
	    ripoff_t *rop;
	    bool slk_visible = (sp != 0
				&& sp->_slk != 0
				&& !(sp->_slk->hidden));

	    if (slk_visible) {
		slk_clear();
	    }
#endif
	    result = NCURSES_SP_NAME(_nc_resize_term) (sp, ToLines, ToCols);

#if USE_SIGWINCH
	    NCURSES_SP_NAME(ungetch) (sp, KEY_RESIZE);	/* so application can know this */
	    clearok(sp->_curscr, TRUE);		/* screen contents are unknown */

	    /* ripped-off lines are a special case: if we did not lengthen
	     * them, we haven't moved them either.  repaint them, too.
	     *
	     * for the rest - stdscr and other windows - the client has to
	     * decide which to repaint, since without panels, ncurses does
	     * not know which are really on top.
	     */
	    for (each_ripoff(rop)) {
		if (rop->win != sp->_stdscr
		    && rop->win != 0
		    && rop->line < 0) {

		    if (rop->hook != _nc_slk_initialize) {
			touchwin(rop->win);
			wnoutrefresh(rop->win);
		    }
		}
	    }

	    /* soft-keys are a special case: we _know_ how to repaint them */
	    if (slk_visible) {
		NCURSES_SP_NAME(slk_restore) (sp);
		NCURSES_SP_NAME(slk_touch) (sp);
		NCURSES_SP_NAME(slk_refresh) (sp);
	    }
#endif
	}
    }

    returnCode(result);
}

NCURSES_EXPORT(int)
resizeterm(int ToLines, int ToCols)
{
    return NCURSES_SP_NAME(_nc_resizeterm) (CURRENT_SCREEN, ToLines, ToCols);
}
