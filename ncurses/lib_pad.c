
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/


/*
 * lib_pad.c
 * newpad	-- create a new pad
 * pnoutrefresh -- refresh a pad, no update
 * pechochar	-- add a char to a pad and refresh
 */

#include "curses.priv.h"

#include <stdlib.h>
#include <errno.h>

#if !HAVE_EXTERN_ERRNO
extern int errno;
#endif

WINDOW *newpad(int l, int c)
{
WINDOW *win;
chtype *ptr;
int i, j;

	T(("newpad(%d, %d) called", l, c));

	if (l <= 0 || c <= 0)
		return NULL;

	if ((win = _nc_makenew(l,c,0,0)) == NULL)
		return NULL;

	win->_flags |= _ISPAD;

	for (i = 0; i < l; i++) {
	    if ((win->_line[i].text = (chtype *) calloc((size_t)c, sizeof(chtype))) == NULL) {
			for (j = 0; j < i; j++)
			    free(win->_line[j].text);

			free(win->_line);
			free(win);

			errno = ENOMEM;
			return NULL;
	    }
	    else
		for (ptr = win->_line[i].text; ptr < win->_line[i].text + c; )
		    *ptr++ = ' ';
	}

	T(("newpad: returned window is %p", win));

	return(win);
}

WINDOW *subpad(WINDOW *orig, int l, int c, int begy, int begx)
{
WINDOW	*win;

	T(("subpad(%d, %d) called", l, c));

	if ((win = derwin(orig, l, c, begy, begx)) == NULL)
	    return NULL;

	win->_flags |= _ISPAD;

	T(("subpad: returned window is %p", win));

	return(win);
}

int prefresh(WINDOW *win, int pminrow, int pmincol,
	int sminrow, int smincol, int smaxrow, int smaxcol)
{
	T(("prefresh() called"));
	if (pnoutrefresh(win, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol) != ERR)
		return (doupdate());
	else
		return ERR;	

}

int pnoutrefresh(WINDOW *win, int pminrow, int pmincol,
	int sminrow, int smincol, int smaxrow, int smaxcol)
{
int	i, j;
int	m, n;
int	pmaxrow;
int	pmaxcol;

	T(("pnoutrefresh(%p, %d, %d, %d, %d, %d, %d) called", 
		win, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol));

	if (win == 0)
		return ERR;

	if (!(win->_flags & _ISPAD))
		return ERR;

	/* negative values are interpreted as zero */
	if (pminrow < 0) pminrow = 0;
	if (pmincol < 0) pmincol = 0;
	if (sminrow < 0) sminrow = 0;
	if (smincol < 0) smincol = 0;

	if (smaxrow >= screen_lines
	 || smaxcol >= screen_columns
	 || sminrow > smaxrow
	 || smincol > smaxcol)
		return ERR;

	pmaxrow = pminrow + smaxrow - sminrow;
	pmaxcol = pmincol + smaxcol - smincol;

	T((" pminrow + smaxrow - sminrow %d, win->_maxy %d", pmaxrow, win->_maxy));
	T((" pmincol + smaxcol - smincol %d, win->_maxx %d", pmaxcol, win->_maxx));
	if ((pmaxrow > win->_maxy)
	 || (pmaxcol > win->_maxx))
		return ERR;

	T(("pad being refreshed"));

	for (i = pminrow, m = sminrow; i <= pmaxrow; i++, m++) {
		for (j = pmincol, n = smincol; j <= pmaxcol; j++, n++) {
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
		win->_line[i].firstchar = win->_line[i].lastchar = _NOCHANGE;
	}

	win->_begx = smincol;
	win->_begy = sminrow;

	if (win->_clear) {
	    win->_clear = FALSE;
	    newscr->_clear = TRUE;
	}

	/*
	 * Use the pad's current position, if it will be visible.
	 * If not, don't do anything; it's not an error.
	 */
	if (win->_leaveok == FALSE
	 && win->_cury  >= pminrow
	 && win->_curx  >= pmincol
	 && win->_cury  <= pmaxrow
	 && win->_curx  <= pmaxcol) {
		newscr->_cury = win->_cury - pminrow + win->_begy;
		newscr->_curx = win->_curx - pmincol + win->_begx;
	}
	return OK;
}

int pechochar(WINDOW *pad, chtype ch)
{
int x, y;

	T(("echochar(%p, %lx)", pad, ch));

	if (pad->_flags & _ISPAD)
		return ERR;

	x = pad->_begx + pad->_curx;
	y = pad->_begy + pad->_cury;

	waddch(curscr, ch);
	doupdate();
	return OK;
}

