
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
**	lib_clearok.c
**
**	The routine clearok.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_clearok.c,v 1.1 1998/02/02 06:21:13 J.T.Conklin Exp $")

int clearok(WINDOW *win, bool flag)
{
	T((T_CALLED("clearok(%p,%d)"), win, flag));

	if (win) {
	  win->_clear = flag;
	  returnCode(OK);
	}
	else
	  returnCode(ERR);
}
