
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992, 1993, 1994                 *
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
**	lib_insdel.c
**
**	The routine winsdelln(win, n).
**  positive n insert n lines above current line
**  negative n delete n lines starting from current line 
**
*/

#include <stdlib.h>
#include "curses.priv.h"

int
winsdelln(WINDOW *win, int n)
{
	T(("winsdel(%p,%d) called", win, n));

	if (n == 0)
		return OK;
	if (n < 0 && win->_cury - n >= win->_maxy)
		/* request to delete too many lines */
		/* should we truncate to an appropriate number? */
		return ERR;

	scroll_window(win, -n, win->_cury, win->_maxy);
	touchline(win, win->_cury, win->_maxy - win->_cury + 1);

	wchangesync(win);
    	return OK;
}


