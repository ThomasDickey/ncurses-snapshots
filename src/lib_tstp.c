
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
**	lib_tstp.c
**
**	The routine tstp().
**
*/

#include "curses.priv.h"
#include "terminfo.h"
#ifdef SVR4_ACTION
#define _POSIX_SOURCE
#endif
#include <signal.h>

void tstp(int dummy)
{
sigaction_t act, oact;
sigset_t mask;

	T(("tstp() called"));

	endwin();

	sigemptyset(&mask);
	sigaddset(&mask, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	act.sa_handler = SIG_DFL;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGTSTP, &act, &oact);
	kill(getpid(), SIGTSTP);

	T(("SIGCONT received"));
	sigaction(SIGTSTP, &oact, NULL);
	reset_prog_mode();
	flushinp();
	if (enter_ca_mode)
		putp(enter_ca_mode);
	doupdate();
}

