
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
**	lib_addstr.c
*
**	The routines waddnstr(), waddchnstr().
**
*/

#include "curses.priv.h"

int
waddnstr(WINDOW *win, char *str, int n)
{
	T(("waddnstr(%x,\"%s\",%d) called", win, visbuf(str), n));

	if (str == NULL)
		return ERR;

	T(("str is not null"));
	if (n < 0) {
		while (*str != '\0') {
		    	if (waddch(win, (chtype)(unsigned char)*str++) == ERR)
				return(ERR);
		}
		T(("waddnstr() done."));
		return OK;
	}

	while(n-- > 0) {
		T(("*str = %x", *str));
		if (*str == '\0')
			break;
		if (waddch(win, (chtype)(unsigned char)*str++) == ERR)
			return ERR;
	}
	T(("waddnstr() done."));
	return OK;
}

int
waddchnstr(WINDOW *win, chtype *str, int n)
{
	T(("waddchnstr(%x,%x,%d) called", win, str, n));

	if (n < 0) {
		while (*str) {
		    if (waddch(win, (chtype)(unsigned char)*str++) == ERR)
			return(ERR);
		}
		return OK;
	}

	while(n-- > 0) {
		if (waddch(win, (chtype)(unsigned char)*str++) == ERR)
			return ERR;
	}
	return OK;
}
