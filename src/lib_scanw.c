

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
**	lib_scanw.c
**
**	The routines scanw(), wscanw() and friends.
**
*/

#include <stdio.h>
#include "curses.priv.h"
#include <termcap.h>

int vwscanw(WINDOW *win, char *fmt, va_list argp)
{
char buf[BUFSIZ];

	if (wgetstr(win, buf) == ERR)
	    return(ERR);
	
	return(vsscanf(buf, fmt, argp));
}

int scanw(char *fmt, ...)
{
va_list ap;

	T(("scanw(\"%s\",...) called", fmt));

	va_start(ap, fmt);
	return(vwscanw(stdscr, fmt, ap));
}

int wscanw(WINDOW *win, char *fmt, ...)
{
va_list ap;

	T(("wscanw(%p,\"%s\",...) called", win, fmt));

	va_start(ap, fmt);
	return(vwscanw(win, fmt, ap));
}



int mvscanw(int y, int x, char *fmt, ...)
{
va_list ap;

	va_start(ap, fmt);
	return(move(y, x) == OK ? vwscanw(stdscr, fmt, ap) : ERR);
}



int mvwscanw(WINDOW *win, int y, int x, char *fmt, ...)
{
va_list ap;

	va_start(ap, fmt);
	return(wmove(win, y, x) == OK ? vwscanw(win, fmt, ap) : ERR);
}


