

/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          by Zeyd M. Ben-Halim                            *
*                          zmbenhal@netcom.com                             *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, not removed   *
*        from header files, and is reproduced in any documentation         *
*        accompanying it or the applications linked with it.               *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/


/*
 *	lib_refresh.c
 *
 *	The routines wredrawln(), wrefresh() and wnoutrefresh().
 *
 */

#include "curses.priv.h"

int wredrawln(WINDOW *win, int beg, int num)
{
	T(("wredrawln(%p,%d,%d) called", win, beg, num));
	touchline(win, beg, num);
	wrefresh(win);
	return OK;
}

int wrefresh(WINDOW *win)
{
	T(("wrefresh(%p) called", win));

	if (win == curscr)
	    	curscr->_clear = TRUE;
	else
	    	wnoutrefresh(win);
	return(doupdate());
}

int wnoutrefresh(WINDOW *win)
{
int	i, j;
int	begx = win->_begx;
int	begy = win->_begy;
int	m, n;
bool	wide;

	T(("wnoutrefresh(%p) called", win));

	/*
	 * If 'newscr' has a different background than the window that we're trying
	 * to refresh, we'll have to copy the whole thing.
	 */
	if (win->_bkgd != newscr->_bkgd) {
		touchwin(win);
		newscr->_bkgd = win->_bkgd;
	}

	/* merge in change information from all subwindows of this window */
	wsyncdown(win);

	/*
	 * For pure efficiency, we'd want to transfer scrolling information
	 * from the window to newscr whenever the window is wide enough that
	 * its update will dominate the cost of the update for the horizontal
	 * band of newscr that it occupies.  Unfortunately, this threshold
	 * tends to be complex to estimate, and in any case scrolling the
	 * whole band and rewriting the parts outside win's image would look
	 * really ugly.  So.  What we do is consider the window "wide" if it
	 * either (a) occupies the whole width of newscr, or (b) occupies
	 * all but at most one column on either vertical edge of the screen
	 * (this caters to fussy people who put boxes around full-screen
	 * windows).  Note that changing this formula will not break any code,
	 * merely change the costs of various update cases.
	 */
	wide = (begx <= 1 && win->_maxx >= (newscr->_maxx - 1));

	win->_flags &= ~_HASMOVED;
	for (i = 0, m = begy; i <= win->_maxy && m <= newscr->_maxy; i++, m++) {
		if (win->_line[i].firstchar != _NOCHANGE) {
			j = win->_line[i].firstchar;
			n = j + begx;
			for (; j <= win->_line[i].lastchar; j++, n++) {
		    		if (win->_line[i].text[j] != newscr->_line[m].text[n]) {
					newscr->_line[m].text[n] = win->_line[i].text[j];

					if (newscr->_line[m].firstchar == _NOCHANGE)
			   			newscr->_line[m].firstchar = newscr->_line[m].lastchar = n;
					else if (n < newscr->_line[m].firstchar)
			   			newscr->_line[m].firstchar = n;
					else if (n > newscr->_line[m].lastchar)
			   			newscr->_line[m].lastchar = n;
		    		}
			}
		}

		if (wide) {
		    int	oind = win->_line[i].oldindex;

		    newscr->_line[m].oldindex 
			= (oind == NEWINDEX) ? NEWINDEX : begy + oind;
		}

		win->_line[i].firstchar = win->_line[i].lastchar = _NOCHANGE;
		win->_line[i].oldindex = i;
	}

	if (win->_clear) {
	   	win->_clear = FALSE;
	   	newscr->_clear = TRUE;
	}

	if (! win->_leave) {
	   	newscr->_cury = win->_cury + win->_begy;
	   	newscr->_curx = win->_curx + win->_begx;
	}
	return(OK);
}
