

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
**	lib_insch.c
**
**	The routine winsch().
**
*/

#include "curses.priv.h"

int  winsch(WINDOW *win, chtype c)
{
chtype	*temp1, *temp2;
chtype	*end;

	T(("winsch(%p,'%lx') called", win, c));

	end = &win->_line[win->_cury].text[win->_curx];
	temp1 = &win->_line[win->_cury].text[win->_maxx];
	temp2 = temp1 - 1;

	while (temp1 > end)
	    *temp1-- = *temp2--;

	*temp1 = c | win->_attrs;

	win->_line[win->_cury].lastchar = win->_maxx;
	if (win->_line[win->_cury].firstchar == _NOCHANGE
	    			||  win->_line[win->_cury].firstchar > win->_curx)
	    win->_line[win->_cury].firstchar = win->_curx;
	return OK;
}
