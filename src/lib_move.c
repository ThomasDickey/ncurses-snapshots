

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
**	lib_move.c
**
**	The routine wmove().
**
*/

#include "curses.priv.h"

int
wmove(WINDOW *win, int y, int x)
{
	T(("wmove(%p,%d,%d) called", win, y, x));

	if (x >= 0  &&  x <= win->_maxx  &&
		y >= 0  &&  y <= win->_maxy)
	{
		win->_curx = x;
		win->_cury = y;

		win->_flags |= _HASMOVED;
		return(OK);
	} else
		return(ERR);
}
