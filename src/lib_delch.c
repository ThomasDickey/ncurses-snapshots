

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
**	lib_delch.c
**
**	The routine wdelch().
**
*/

#include "curses.priv.h"

int wdelch(WINDOW *win)
{
chtype	*temp1, *temp2;
chtype	*end;

	T(("wdelch(%x) called", win));

	end = &win->_line[win->_cury].text[win->_maxx];
	temp2 = &win->_line[win->_cury].text[win->_curx + 1];
	temp1 = temp2 - 1;

	while (temp1 < end)
	    *temp1++ = *temp2++;

	*temp1 = ' ' | win->_attrs;

	win->_line[win->_cury].lastchar = win->_maxx;

	if (win->_line[win->_cury].firstchar == _NOCHANGE
				   || win->_line[win->_cury].firstchar > win->_curx)
	    win->_line[win->_cury].firstchar = win->_curx;

	wchangesync(win);
	return OK;
}
