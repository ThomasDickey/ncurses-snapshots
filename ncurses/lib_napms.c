
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
 *	lib_napms.c
 *
 *	The routine napms.
 *
 */

#include <curses.priv.h>

#if USE_FUNC_POLL
#include <stropts.h>
#include <poll.h>
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#elif HAVE_SELECT
#if HAVE_SYS_TIME_H && HAVE_SYS_TIME_SELECT
#include <sys/time.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#endif  

MODULE_ID("$Id: lib_napms.c,v 1.2 1998/02/02 07:29:05 J.T.Conklin Exp $")

int napms(int ms)
{
	T((T_CALLED("napms(%d)"), ms));

#if HAVE_NANOSLEEP
	{
		struct timespec ts;
		ts.tv_sec = ms / 1000000;
		ts.tv_nsec = (ms % 1000000) * 1000000;
		nanosleep(&ts, NULL);
	}
#elif HAVE_USLEEP
	usleep(1000*(unsigned)ms);
#elif USE_FUNC_POLL
	{
		struct pollfd fds[1];
		poll(fds, 0, ms);
	}
#elif HAVE_SELECT
	{
		struct timeval tval;
		tval.tv_sec = ms / 1000;
		tval.tv_usec = (ms % 1000) * 1000;
		select(0, NULL, NULL, NULL, &tval);
	}
#endif
	returnCode(OK);
}
