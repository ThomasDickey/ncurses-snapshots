
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
waddnstr(WINDOW *win, char *const astr, int n)
{
char *str = astr;
int code = ERR;

	T(("waddnstr(%p,\"%s\",%d) called", win, visbuf(str), n));

	if (str != NULL) {

		TR(TRACE_VIRTPUT, ("str is not null"));
		code = OK;
		if (n < 0)
			n = strlen(str);

		while((n-- > 0) && (*str != '\0')) {
			TR(TRACE_VIRTPUT, ("*str = %x", *str));
			if (waddch(win, (chtype)(unsigned char)*str++) == ERR) {
				code = ERR;
				break;
			}
		}
	}
	TR(TRACE_VIRTPUT, ("waddnstr returns %d", code));
	return code;
}

int
waddchnstr(WINDOW *win, chtype *const astr, int n)
{
chtype *str = astr;

	T(("waddchnstr(%p,%p,%d) called", win, str, n));

	if (n < 0) {
		while (*str) {
		    if (waddch(win, *str++) == ERR)
			return(ERR);
		}
		return OK;
	}

	while(n-- > 0) {
		if (waddch(win, *str++) == ERR)
			return ERR;
	}
	return OK;
}
