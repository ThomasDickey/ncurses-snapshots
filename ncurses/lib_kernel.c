
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
 *	lib_kernel.c
 *
 *	Misc. low-level routines:
 *		napms()
 *		reset_prog_mode()
 *		reset_shell_mode()
 *		erasechar()
 *		killchar()
 *		flushinp()
 *		savetty()
 *		resetty()
 *
 * The baudrate() and delay_output() functions could logically live here,
 * but are in other modules to reduce the static-link size of programs
 * that use only these facilities.
 */

#include <curses.priv.h>
#include <term.h>	/* cur_term */

MODULE_ID("$Id: lib_kernel.c,v 1.12 1996/09/07 17:08:05 tom Exp $")

int napms(int ms)
{
	T(("napms(%d) called", ms));

	usleep(1000*(unsigned)ms);
	return OK;
}

int reset_prog_mode(void)
{
	T(("reset_prog_mode() called"));

	SET_TTY(cur_term->Filedes, &cur_term->Nttyb);
	if (SP && stdscr && stdscr->_use_keypad)
		_nc_keypad(TRUE);

	return OK;
}


int reset_shell_mode(void)
{
	T(("reset_shell_mode() called"));

	if (SP)
	{
		fflush(SP->_ofp);
		_nc_keypad(FALSE);
	}

	SET_TTY(cur_term->Filedes, &cur_term->Ottyb);
	return OK;
}

/*
 *	erasechar()
 *
 *	Return erase character as given in cur_term->Ottyb.
 *
 */

char
erasechar(void)
{
	T(("erasechar() called"));

#ifdef TERMIOS
	return(cur_term->Ottyb.c_cc[VERASE]);
#else
	return(cur_term->Ottyb.sg_erase);
#endif

}



/*
 *	killchar()
 *
 *	Return kill character as given in cur_term->Ottyb.
 *
 */

char
killchar(void)
{
	T(("killchar() called"));

#ifdef TERMIOS
	return(cur_term->Ottyb.c_cc[VKILL]);
#else
	return(cur_term->Ottyb.sg_kill);
#endif
}



/*
 *	flushinp()
 *
 *	Flush any input on cur_term->Filedes
 *
 */

int flushinp(void)
{
	T(("flushinp() called"));

#ifdef TERMIOS
	tcflush(cur_term->Filedes, TCIFLUSH);
#else
	errno = 0;
	do {
	    ioctl(cur_term->Filedes, TIOCFLUSH, 0);
	} while
	    (errno == EINTR);
#endif
	if (SP) {
		SP->_fifohead = -1;
		SP->_fifotail = 0;
		SP->_fifopeek = 0;
	}
	return OK;

}

/*
**	savetty()  and  resetty()
**
*/

static TTY   buf;

int savetty(void)
{
	T(("savetty() called"));

	GET_TTY(cur_term->Filedes, &buf);
	return OK;
}

int resetty(void)
{
	T(("resetty() called"));

	SET_TTY(cur_term->Filedes, &buf);
	return OK;
}
