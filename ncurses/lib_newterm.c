
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

#include "curses.priv.h"
#include <stdlib.h>
#include "term.h"	/* clear_screen, cup & friends, cur_term */

/* This should moved to TERMINAL */
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
	if (setupterm(term, fileno(ofp), &errret) != 1)
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

	/* optional optimization hack */
#if HAVE_SETVBUF
	{
	    /* 
	     * If the output file descriptor is connected to a tty
	     * (the typical case) it will probably be line-buffered.
	     * Keith Bostic pointed out that we don't want this; it
	     * hoses people running over networks by forcing out a
	     * bunch of small packets instead of one big one, so
	     * screen updates on ptys look jerky.  Restore block
	     * buffering to prevent this minor lossage.
	     *
	     * The buffer size is a compromise.  Ideally we'd like a
	     * buffer that can hold the maximum possible update size
	     * (the whole screen plus cup commands to change lines as
	     * it's painted).  On a modern 66-line xterm this can
	     * become excessive.  So we min it with the amount of data
	     * we think we can get through one Ethernet packet
	     * (maximum packet size - 100 for TCP/IP overhead), This
	     * is the largest update we're likely to be able to send
	     * atomically (it also protects against the rare case of
	     * no cursor addressing).
	     */
	    int bufsiz = min(LINES * (COLS + SP->_cup_cost), 1400);

	    (void) setvbuf(SP->_ofp, malloc(bufsiz), _IOFBF, bufsiz);
	}
#endif /* HAVE_SETVBUF */

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

