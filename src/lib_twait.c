
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
**	lib_twait.c
**
**	The routine _nc_timed_wait().
**
*/

#include "curses.priv.h"

#include <sys/types.h>		/* some systems can't live without this */
#include <string.h>
#include <sys/time.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if !HAVE_USLEEP
void usleep(unsigned int usec)
{
struct timeval tval;

	tval.tv_sec = usec / 1000000;
	tval.tv_usec = usec % 1000000;
	select(0, NULL, NULL, NULL, &tval);

}
#endif

int _nc_timed_wait(int fd, int wait, int *timeleft)
{
int result;
struct timeval timeout;
static fd_set set;
#ifndef GOOD_SELECT
struct timeval starttime, returntime;

	 gettimeofday(&starttime, NULL);
#endif

	 FD_ZERO(&set);
	 FD_SET(fd, &set);

	 /* the units of wait are milliseconds */
	 timeout.tv_sec = wait / 1000;
	 timeout.tv_usec = (wait % 1000) * 1000;

	 T(("start twait: sec = %ld, usec = %ld", timeout.tv_sec, timeout.tv_usec));

	 result = select(fd+1, &set, NULL, NULL, &timeout);

#ifndef GOOD_SELECT
	 gettimeofday(&returntime, NULL);
	 timeout.tv_sec -= (returntime.tv_sec - starttime.tv_sec);
	 timeout.tv_usec -= (returntime.tv_usec - starttime.tv_usec);
	 if (timeout.tv_usec < 0 && timeout.tv_sec > 0) {
		timeout.tv_sec--;
		timeout.tv_usec += 1000000;
	 }
	 if (timeout.tv_sec < 0)
		timeout.tv_sec = timeout.tv_usec = 0;
#endif

	 /* return approximate time left on the timeout, in milliseconds */
	 if (timeleft)
		*timeleft = (timeout.tv_sec * 1000) + (timeout.tv_usec / 1000);

	 T(("end twait: returned %d, sec = %ld, usec = %ld (%d msec)",
		 result, timeout.tv_sec, timeout.tv_usec, 
	 	timeleft ? *timeleft : -1));

	 return(result);
}
