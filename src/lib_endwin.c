

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
**	lib_endwin.c
**
**	The routine endwin().
**
*/

#include "terminfo.h"	/* exit_ca_mode, orig_pair, change_scroll_region */
#include "curses.priv.h"

int isendwin()
{
	if (SP == NULL)
		return FALSE;
	return SP->_endwin;
}

int
endwin()
{
	T(("endwin() called"));

	SP->_endwin = TRUE;

#ifdef change_scroll_region
	if (change_scroll_region)
		putp(tparm(change_scroll_region, 0, lines - 1));
#endif /* change_scroll_region */

	mvcur(-1, -1, screen_lines - 1, 0);

	curs_set(1);	/* set cursor to normal mode */

#ifdef exit_ca_mode
	if (exit_ca_mode)
	    putp(exit_ca_mode);
#endif /* exit_ca_mode */

#ifdef orig_pair
	if (SP->_coloron == TRUE)
		putp(orig_pair);
#endif /* orig_pair */

	if (curscr  &&  (curscr->_attrs != A_NORMAL)) 
	    vidattr(curscr->_attrs = A_NORMAL);

	return(reset_shell_mode());
}
