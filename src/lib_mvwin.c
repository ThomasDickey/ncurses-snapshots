

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
**	lib_mvwin.c
**
**	The routine mvwin().
**
*/

#include "curses.priv.h"

int mvwin(WINDOW *win, int by, int bx)
{
	T(("mvwin(%x,%d,%d) called", win, by, bx));

	if (win->_flags & _SUBWIN)
	    return(ERR);

	if (by + win->_maxy > screen_lines - 1  ||  bx + win->_maxx > screen_columns - 1)
	    return(ERR);

	win->_begy = by;
	win->_begx = bx;

	touchwin(win);

	return(OK);
}
