

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
**	lib_scrreg.c
**
**	The routine wsetscrreg().
**
*/

#include "curses.priv.h"

int wsetscrreg(WINDOW *win, int top, int bottom)
{
	T(("wsetscrreg(%p,%d,%d) called", win, top, bottom));

    	if (top >= 0  && top <= win->_maxy &&
		bottom >= 0  &&  bottom <= win->_maxy &&
		bottom > top)
	{
	    	win->_regtop = top;
	    	win->_regbottom = bottom;

	    	return(OK);
	} else
	    	return(ERR);
}
