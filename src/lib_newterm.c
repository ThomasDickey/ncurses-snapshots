
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
**	lib_newterm.c
**
** 	The newterm() function.
**
*/

#include <stdlib.h>
#include "curses.priv.h"
#include "term.h"	/* clear_screen, cup & friends, cur_term */

/* This should move to TERMINAL */
static filter_mode = FALSE;

void filter(void)
{
    filter_mode = TRUE;
}

SCREEN * newterm(const char *term, FILE *ofp, FILE *ifp)
{
int	errret;

#ifdef TRACE
	T(("newterm(\"%s\",%p,%p) called", term, ofp, ifp));
#endif

	/* this loads the capability entry, then sets LINES and COLS */
	if (setupterm((char *)term, fileno(ifp), &errret) != 1)
	    	return NULL;

	/* implement filter mode */
	if (filter_mode) {
	    LINES = 1;

#ifdef clear_screen
	    clear_screen = (char *)NULL;
	    cursor_down = parm_down_cursor = (char *)NULL;
	    cursor_address = (char *)NULL;
	    cursor_up = parm_up_cursor = (char *)NULL;
	    row_address = (char *)NULL;

	    cursor_home = carriage_return;
#endif /* clear_screen */
	}

	/* if we must simulate soft labels, grab off the line to be used */
#ifdef num_labels
	if (num_labels <= 0)
#endif /* num_labels */
	    if (_slk_init)
		ripoffline(-1, slk_initialize);

	/* this actually allocates the screen structure */
	if (_nc_setupscreen(LINES, COLS) == ERR)
	    	return NULL;

#ifdef num_labels
	/* if the terminal type has real soft labels, set those up */
	if (_slk_init && num_labels > 0)
	    slk_initialize(stdscr, COLS);
#endif /* num_labels */

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

	baudrate();	/* sets a field in the SP structure */

	/* compute movement costs so we can do better move optimization */
	mvcur_init(SP);
#if 0
	/* initialize soft labels */
	if (_slk_init)
	    if (num_labels <= 0)
		ripoffline(-1, slk_initialize);
	    else
		slk_initialize(stdscr, COLS);
#endif
	_nc_signal_handler(TRUE);

	T(("newterm returns %p", SP));

	return(SP);
}

