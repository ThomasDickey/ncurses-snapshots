
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
**	lib_endwin.c
**
**	The routine endwin().
**
*/

#include <curses.priv.h>
#include <term.h>

MODULE_ID("$Id: lib_endwin.c,v 1.11 1997/08/18 13:32:29 Alexander.V.Lukyanov Exp $")

int
endwin(void)
{
	T((T_CALLED("endwin()")));

	SP->_endwin = TRUE;

	_nc_mouse_wrap(SP);
	_nc_screen_wrap();
	_nc_mvcur_wrap();	/* wrap up cursor addressing */

	returnCode(reset_shell_mode());
}
