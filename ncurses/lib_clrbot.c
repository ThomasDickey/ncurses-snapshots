
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
**	lib_clrbot.c
**
**	The routine wclrtobot().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_clrbot.c,v 1.11 1997/08/14 20:03:38 Alexander.V.Lukyanov Exp $")

int wclrtobot(WINDOW *win)
{
chtype	blank = _nc_background(win);
chtype	*ptr, *end;
short	y, startx;

	T((T_CALLED("wclrtobot(%p)"), win));

	startx = win->_curx;

	T(("clearing from y = %d to y = %d with maxx =  %d", win->_cury, win->_maxy, win->_maxx));

	for (y = win->_cury; y <= win->_maxy; y++) {
		end = &win->_line[y].text[win->_maxx];

		for (ptr = &win->_line[y].text[startx]; ptr <= end; ptr++)
			*ptr = blank;

		if (win->_line[y].firstchar > startx
			||  win->_line[y].firstchar == _NOCHANGE)
		    win->_line[y].firstchar = startx;

		win->_line[y].lastchar = win->_maxx;

		startx = 0;
	}
	_nc_synchook(win);
	returnCode(OK);
}
