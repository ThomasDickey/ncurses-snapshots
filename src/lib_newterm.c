

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
**	lib_newterm.c
**
** 	The newterm() function.
**
*/

#include <stdlib.h>
#include "curses.priv.h"
#include "terminfo.h"	/* enter_ca_mode */

SCREEN * newterm(char *term, FILE *ofp, FILE *ifp)
{
int	errret;

#ifdef TRACE
	_init_trace();
	T(("newterm(\"%s\",%p,%p) called", term, ofp, ifp));
#endif

	/* this loads the capability entry, then sets LINES and COLS */
	if (setupterm(term, fileno(ofp), &errret) != 1)
	    	return NULL;

	/* this actually allocates the screen structure */
	if (setupscreen(LINES, COLS) == ERR)
	    	return NULL;

	SP->_ifd        = fileno(ifp);
	SP->_checkfd	= fileno(ifp);
	typeahead(fileno(ifp));
	SP->_ofp        = ofp;
#ifdef TERMIOS
	SP->_use_meta   = ((cur_term->Ottyb.c_cflag & CSIZE) == CS8 &&
			    !(cur_term->Ottyb.c_iflag & ISTRIP));
#else
	SP->_use_meta   = FALSE;
#endif
	SP->_endwin	= FALSE;

#ifdef enter_ca_mode
	if (enter_ca_mode)
	    	putp(enter_ca_mode);
#endif /* enter_ca_mode */

	baudrate();	/* sets a field in the SP structure */

	curses_signal_handler(TRUE);

	T(("newterm returns %p", SP));

	return(SP);
}

