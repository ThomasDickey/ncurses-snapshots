

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
**	lib_printw.c
**
**	The routines printw(), wprintw() and friends.
**
*/

#include "curses.priv.h"

int printw(char *fmt, ...)
{
va_list argp;
char buf[BUFSIZ];

	T(("printw(\"%s\",...) called", fmt));

	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	return(waddstr(stdscr, buf));
}



int wprintw(WINDOW *win, char *fmt, ...)
{
va_list argp;
char buf[BUFSIZ];

	T(("wprintw(%p,\"%s\",...) called", win, fmt));

	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	return(waddstr(win, buf));
}



int mvprintw(int y, int x, char *fmt, ...)
{
va_list argp;
char buf[BUFSIZ];

	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	return(move(y, x) == OK ? waddstr(stdscr, buf) : ERR);
}



int mvwprintw(WINDOW *win, int y, int x, char *fmt, ...)
{
va_list argp;
char buf[BUFSIZ];

	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	return(wmove(win, y, x) == OK ? waddstr(win, buf) : ERR);
}

int vwprintw(WINDOW *win, char *fmt, va_list argp)
{
char buf[BUFSIZ];

	vsprintf(buf, fmt, argp);
	return(waddstr(win, buf));
}
