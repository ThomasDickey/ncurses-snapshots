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

#include "system.h"

/*
 * lib_pad.c
 * newpad	-- create a new pad
 * pnoutrefresh -- refresh a pad, no update
 * pechochar	-- add a char to a pad and refresh
 */

#include <stdlib.h>
#include <errno.h>

#if !HAVE_EXTERN_ERRNO
extern int errno;
#endif

#include "curses.priv.h"

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

	T(("pnoutrefresh(%p, %d, %d, %d, %d, %d, %d) called", 
		win, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol));

	if (!(win->_flags & _ISPAD))
		return ERR;

	if (pminrow < 0) pminrow = 0;
	if (pmincol < 0) pmincol = 0;
	if (sminrow < 0) sminrow = 0;
	if (smincol < 0) smincol = 0;

	if (smaxrow >= screen_lines || smaxcol >= screen_columns)
		return ERR;

	T((" pminrow + smaxrow - sminrow %d, win->_maxy %d", pminrow + smaxrow - sminrow ,  win->_maxy));
	T((" pmincol + smaxcol - smincol %d, win->_maxx %d", pmincol + smaxcol - smincol ,  win->_maxx));
	if ((pminrow + smaxrow - sminrow > win->_maxy) ||
	    (pmincol + smaxcol - smincol > win->_maxx) ||
	    (smaxrow < 0) ||
	    (smaxcol < 0))
		return ERR;

	T(("pad being refreshed"));

	for (i = pminrow, m = sminrow; i <= pminrow + smaxrow-sminrow; i++, m++) {
		for (j = pmincol, n = smincol; j <= pmincol + smaxcol-smincol;
		     j++, n++) {
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

#if 0
	/*
	 * This code causes problems if the pad is larger than the
	 * terminal screen -- doupdate may try taking the newscr
	 * cursor to a non-existent location.
	 */
	if (! win->_leave) {
	    newscr->_cury = win->_cury + win->_begy;
	    newscr->_curx = win->_curx + win->_begx;
	}
#endif /* 0 */
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

