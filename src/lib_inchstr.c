

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
**	lib_inchstr.c
**
**	The routine winchnstr().
**
*/

#include "curses.priv.h"

int winchnstr(WINDOW *win, chtype *str, int n)
{
	int	i;

	T(("winchnstr(%x,'%x',%d) called", win, str, n));

	for (i = 0; (n < 0 || (i < n)) && (win->_curx + i <= win->_maxx); i++)
	    str[i] = win->_line[win->_cury].text[win->_curx + i];
	str[i] = (chtype)0;

	return(i);
}
