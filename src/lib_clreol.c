

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
**	lib_clreol.c
**
**	The routine wclrtoeol().
**
*/

#include "curses.priv.h"

int  wclrtoeol(WINDOW *win)
{
chtype	*maxx, *ptr, *end;
int	y, x, minx;

	T(("wclrtoeol(%x) called", win));

	y = win->_cury;
	x = win->_curx;

	end = &win->_line[y].text[win->_maxx];
	minx = _NOCHANGE;
	maxx = &win->_line[y].text[x];

	for (ptr = maxx; ptr <= end; ptr++) {
	    if (*ptr != BLANK) {
			maxx = ptr;
			if (minx == _NOCHANGE)
			    minx = ptr - win->_line[y].text;
			*ptr = BLANK;
	    }
	}

	if (minx != _NOCHANGE) {
	    if (win->_line[y].firstchar > minx || win->_line[y].firstchar == _NOCHANGE)
			win->_line[y].firstchar = minx;

	    if (win->_line[y].lastchar < maxx - win->_line[y].text)
			win->_line[y].lastchar = maxx - win->_line[y].text;
	}
	wchangesync(win);
	return(OK);
}
