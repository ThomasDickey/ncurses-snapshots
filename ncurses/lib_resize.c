/******************************************************************************
 * Copyright 1995 by Thomas E. Dickey.  All Rights Reserved.                  *
 *                                                                            *
 * You may freely copy or redistribute this software, so long as there is no  *
 * profit made from its use, sale trade or reproduction. You may not change   *
 * this copyright notice, and it must be included in any copy made.           *
 ******************************************************************************/

/*
 * This is an extension to the curses library.  It provides callers with a hook
 * into the NCURSES data to resize windows, primarily for use by programs
 * running in an X Window terminal (e.g., xterm).  I abstracted this module
 * from my application library for NCURSES because it must be compiled with
 * the private data structures -- T.Dickey 1995/7/4.
 */

#include "curses.priv.h"
#include <term.h>
#include <stdlib.h>

/*
 * This function reallocates NCURSES window structures.  It is invoked in
 * response to a SIGWINCH interrupt.  Other user-defined windows may also need
 * to be reallocated.
 *
 * Because this performs memory allocation, it should not (in general) be
 * invoked directly from the signal handler.
 */
int
resizeterm(int ToLines, int ToCols)
{
#ifdef TRACE
	T(("resizeterm called (%d,%d) to (%d,%d)", LINES, COLS, ToLines, ToCols));
#endif
	if (ToLines != LINES
	 || ToCols  != COLS) {

		if (wresize(stdscr, ToLines, ToCols) != OK
		 || wresize(curscr, ToLines, ToCols) != OK
		 || wresize(newscr, ToLines, ToCols) != OK)
			return ERR;

		LINES = screen_lines   = lines    = ToLines;
		COLS  = screen_columns = columns  = ToCols;
	}
	return OK;
}

/*
 * Reallocate a curses WINDOW struct to either shrink or grow to the specified
 * new lines/columns.  If it grows, the new character cells are filled with
 * blanks.  The application is responsible for repainting the blank area.
 */

static void *doalloc(void *p, size_t n)
{
	if (p == 0)
		p = malloc(n);
	else
		p = realloc(p, n);
	return p;
}

#define DOALLOC(p,t,n)  (t *)doalloc(p, sizeof(t)*(n))
#define	ld_ALLOC(p,n)	DOALLOC(p,struct ldat,n)
#define	c_ALLOC(p,n)	DOALLOC(p,chtype,n)

int
wresize(WINDOW *win, int ToLines, int ToCols)
{
	register int	row;
	int	size_x, size_y;

#ifdef TRACE
	T(("wresize(win=%p, lines=%d, cols=%d) called", win, ToLines, ToCols));
	TR(TRACE_UPDATE, ("...beg (%d, %d), max(%d,%d), reg(%d,%d)",
		win->_begy, win->_begx,
		win->_maxy, win->_maxx,
		win->_regtop, win->_regbottom));
	_tracedump("...before", win);
#endif

	if (ToLines <= 0 || ToCols <= 0)
		return ERR;

	size_x = (win->_maxx - win->_begx);
	size_y = (win->_maxy - win->_begy);

	/*
	 * If the number of lines has changed, adjust the size of the overall
	 * vector:
	 */
	if (ToLines != size_y) {
		for (row = ToLines+1; row <= size_y; row++)
			free((char *)(win->_line[row].text));

		win->_line = ld_ALLOC(win->_line, ToLines+1);
		if (win->_line == 0)
			return ERR;

		for (row = size_y+1; row <= ToLines; row++) {
			win->_line[row].text      = 0;
			win->_line[row].firstchar = 0;
			win->_line[row].lastchar  = ToCols;
		}
	}

	/*
	 * Adjust the width of the columns:
	 */
	for (row = 0; row <= ToLines; row++) {
		chtype	*s	= win->_line[row].text;
		int	begin	= (s == 0) ? 0 : size_x;
		int	end	= ToCols;
		chtype	blank	= _nc_render(win, ' ', BLANK);

		win->_line[row].oldindex = row;

		if (ToCols != begin) {
			win->_line[row].text = s = c_ALLOC(s, ToCols+1);
			if (win->_line[row].text == 0)
				return ERR;

			if (end > begin) {	/* growing */
				if (win->_line[row].firstchar < begin)
					win->_line[row].firstchar = begin;
				win->_line[row].lastchar = ToCols;
				do {
					s[end] = blank;
				} while (--end >= begin);
			} else {		/* shrinking */
				win->_line[row].firstchar = 0;
				win->_line[row].lastchar  = ToCols;
			}
		}
	}

	/*
	 * Finally, adjust the parameters showing screen size and cursor
	 * position:
	 */
	if (win->_regtop > ToLines)	win->_regtop    = ToLines;
	if (win->_regbottom > ToLines)	win->_regbottom = ToLines;

	if (win->_curx > ToCols)	win->_curx      = ToCols;
	if (win->_cury > ToLines)	win->_cury      = ToLines;

	if (win->_begx == 0
	 && win->_maxx == COLS-1)
	 	win->_maxx = ToCols - 1;
	else if (size_x >= ToCols)
		win->_maxx = ToCols - 1 + win->_begx;

	if (win->_begy == 0
	 && win->_maxy == LINES-1)
	 	win->_maxy = ToLines - 1;
	else if (size_y >= ToLines)
		win->_maxy = ToLines - 1 + win->_begy;

#ifdef TRACE
	TR(TRACE_UPDATE, ("...beg (%d, %d), max(%d,%d), reg(%d,%d)",
		win->_begy, win->_begx,
		win->_maxy, win->_maxx,
		win->_regtop, win->_regbottom));
	_tracedump("...after:", win);
#endif
	return OK;
}
