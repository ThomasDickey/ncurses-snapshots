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

/****************************************************************************
 *  Author: Thomas E. Dickey                    1996-on                     *
 ****************************************************************************/

#include <curses.priv.h>
#include <term_entry.h>
#include <tic.h>

#if HAVE_NC_FREEALL

#if HAVE_LIBDBMALLOC
extern int malloc_errfd;	/* FIXME */
#endif

MODULE_ID("$Id: lib_freeall.c,v 1.54.1.1 2008/11/16 00:19:59 juergen Exp $")

/*
 * Free all ncurses data.  This is used for testing only (there's no practical
 * use for it as an extension).
 */
NCURSES_EXPORT(void)
NC_SNAME(_nc_freeall)(SCREEN *sp)
{
    WINDOWLIST *p, *q;
    static va_list empty_va;

    T((T_CALLED("_nc_freeall()")));
#if NO_LEAKS
    if (sp != 0) {
	if (sp->_oldnum_list != 0) {
	    FreeAndNull(sp->_oldnum_list);
	}
	if (sp->_panelHook.destroy != 0) {
	    sp->_panelHook.destroy(sp->_panelHook.stdscr_pseudo_panel);
	}
    }
#endif
    if (sp != 0) {
	_nc_lock_global(curses);

	while (sp->_windowlist != 0) {
	    bool deleted = FALSE;

	    /* Delete only windows that're not a parent */
	    for (safe_each_window(sp, p)) {
		bool found = FALSE;

		for (safe_each_window(sp, q)) {
		    if ((p != q)
			&& (q->win._flags & _SUBWIN)
			&& (&(p->win) == q->win._parent)) {
			found = TRUE;
			break;
		    }
		}

		if (!found) {
		    if (delwin(&(p->win)) != ERR)
			deleted = TRUE;
		    break;
		}
	    }

	    /*
	     * Don't continue to loop if the list is trashed.
	     */
	    if (!deleted)
		break;
	}
	delscreen(sp);
	_nc_unlock_global(curses);
    }
    if (HasTerminal(sp))
	del_curterm(sp->_term);

    (void) _nc_printf_string(0, empty_va);
#ifdef TRACE
    (void) _nc_trace_buf(-1, 0);
#endif
#if USE_WIDEC_SUPPORT
    FreeIfNeeded(_nc_wacs);
#endif
    _nc_leaks_tinfo();

#if HAVE_LIBDBMALLOC
    malloc_dump(malloc_errfd);
#elif HAVE_LIBDMALLOC
#elif HAVE_LIBMPATROL
    __mp_summary();
#elif HAVE_PURIFY
    purify_all_inuse();
#endif
    returnVoid;
}

NCURSES_EXPORT(void)
_nc_freeall(void)
{
    NC_SNAME(_nc_freeall)(CURRENT_SCREEN);
}

NCURSES_EXPORT(void)
NC_SNAME(_nc_free_and_exit)(SCREEN *sp, int code)
{
    char *last_setbuf = (sp != 0) ? sp->_setbuf : 0;

    NC_SNAME(_nc_freeall)(sp);
#ifdef TRACE
    trace(0);			/* close trace file, freeing its setbuf */
    {
	static va_list fake;
	free(_nc_varargs("?", fake));
    }
#endif
    fclose(stdout);
    FreeIfNeeded(last_setbuf);
    exit(code);
}

#else
NCURSES_EXPORT(void)
_nc_freeall (void)
{
}

NCURSES_EXPORT(void)
NC_SNAME(_nc_free_and_exit)(SCREEN *sp, int code)
{
    if (sp) {
	delscreen(sp);
	if (sp->_term)
	    NC_SNAME(_nc_del_curterm)(sp, sp->_term);
    }
    exit(code);
}
#endif

NCURSES_EXPORT(void)
_nc_free_and_exit (int code)
{
    NC_SNAME(_nc_free_and_exit)(CURRENT_SCREEN, code);
}
