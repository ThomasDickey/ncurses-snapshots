

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
**	lib_erase.c
**
**	The routine werase().
**
*/

#include "curses.priv.h"

int  werase(WINDOW	*win)
{
int	y;
chtype	*sp, *end, *start, *maxx = NULL;
int	minx;

	T(("werase(%x) called", win));

	for (y = 0; y <= win->_maxy; y++) {
	    	minx = _NOCHANGE;
	    	start = win->_line[y].text;
	    	end = &start[win->_maxx];
	
	    	maxx = start;
	    	for (sp = start; sp <= end; sp++) {
		    	maxx = sp;
		    	if (minx == _NOCHANGE)
					minx = sp - start;
		    	*sp = BLANK;
	    	}

	    	if (minx != _NOCHANGE) {
			if (win->_line[y].firstchar > minx ||
		    	    win->_line[y].firstchar == _NOCHANGE)
		    		win->_line[y].firstchar = minx;

			if (win->_line[y].lastchar < maxx - win->_line[y].text)
		    	win->_line[y].lastchar = maxx - win->_line[y].text;
	    	}
	}
	win->_curx = win->_cury = 0;
	wchangesync(win);
	return OK;
}
