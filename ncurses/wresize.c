/******************************************************************************
 * Copyright 1996 by Thomas E. Dickey <dickey@clark.net>                      *
 * All Rights Reserved.                                                       *
 *                                                                            *
 * Permission to use, copy, modify, and distribute this software and its      *
 * documentation for any purpose and without fee is hereby granted, provided  *
 * that the above copyright notice appear in all copies and that both that    *
 * copyright notice and this permission notice appear in supporting           *
 * documentation, and that the name of the above listed copyright holder(s)   *
 * not be used in advertising or publicity pertaining to distribution of the  *
 * software without specific, written prior permission. THE ABOVE LISTED      *
 * COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,  *
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO     *
 * EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY         *
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER       *
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF       *
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN        *
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                   *
 ******************************************************************************/

#include <curses.priv.h>
#include <term.h>

MODULE_ID("$Id: wresize.c,v 1.1 1996/09/07 14:59:00 tom Exp $")

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
		chtype	blank	= _nc_background(win);

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
