
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

#include <curses.priv.h>

#include <string.h>

#if HAVE_SYS_TIME_H && ! SYSTEM_LOOKS_LIKE_SCO
#include <sys/time.h>
#endif

#if USE_FUNC_POLL
#include <stropts.h>
#include <poll.h>
#elif defined(HAVE_SELECT)
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#endif

MODULE_ID("$Id: lib_twait.c,v 1.12 1996/11/09 21:48:48 tom Exp $")

/*
 * We want to define GOOD_SELECT if the last argument of select(2) is
 * modified to indicate time left.  The code will deal gracefully with
 * the other case, this is just an optimization to reduce the number
 * of system calls per input event.
 *
 * In general, expect System-V-like UNIXes to have this behavior and BSD-like
 * ones to not have it.  Check your manual page.  If it doesn't explicitly
 * say the last argument is modified, assume it's not.
 *
 * (We'd really like configure to autodetect this, but writing a proper test
 * turns out to be hard.)
 */

#if HAVE_GETTIMEOFDAY
#if (defined(TRACE) && !defined(HAVE_USLEEP)) || !defined(GOOD_SELECT)
static void _nc_gettime(struct timeval *tp)
{
	gettimeofday(tp, (struct timezone *)0);
	T(("time: %ld.%06ld", tp->tv_sec, tp->tv_usec));
}
#endif
#endif

#if !HAVE_USLEEP
int _nc_usleep(unsigned int usec)
{
int code;
struct timeval tval;

#if defined(TRACE) && HAVE_GETTIMEOFDAY
	_nc_gettime(&tval);
#endif
#if USE_FUNC_POLL
	{
	struct pollfd fds[1];
	code = poll(fds, 0, usec / 1000);
	}
#elif defined(HAVE_SELECT)
	tval.tv_sec = usec / 1000000;
	tval.tv_usec = usec % 1000000;
	code = select(0, NULL, NULL, NULL, &tval);
#endif

#if defined(TRACE) && HAVE_GETTIMEOFDAY
	_nc_gettime(&tval);
#endif
	return code;
}
#endif /* !HAVE_USLEEP */

int _nc_timed_wait(int fd, int milliseconds, int *timeleft)
{
long	whole_secs = milliseconds / 1000;
long	micro_secs = (milliseconds % 1000) * 1000;

int result = 0;
struct timeval ntimeout;

#if defined(HAVE_SELECT)
static fd_set set;
#endif

#if !defined(GOOD_SELECT) && HAVE_GETTIMEOFDAY
struct timeval starttime, returntime;
long delta;

	_nc_gettime(&starttime);
#endif

	ntimeout.tv_sec  = whole_secs;
	ntimeout.tv_usec = micro_secs;

	T(("start twait: %ld.%06ld secs", ntimeout.tv_sec, ntimeout.tv_usec));

	/*
	 * The do loop tries to make it look like we have restarting signals,
	 * even if we don't.
	 */
	do {
#if USE_FUNC_POLL
		struct pollfd fds[1];
		fds[0].fd = fd;
		fds[0].events = POLLIN;
		result = poll(fds, 1, milliseconds);
#elif defined(HAVE_SELECT)
		/*
		 * Some systems modify the fd_set arguments; do this in the
		 * loop.
		 */
		FD_ZERO(&set);
		FD_SET(fd, &set);

		errno = 0;
		result = select(fd+1, &set, NULL, NULL, &ntimeout);
#endif

#if !defined(GOOD_SELECT) && HAVE_GETTIMEOFDAY
		_nc_gettime(&returntime);

		/* The contents of ntimeout aren't guaranteed after return from
		 * 'select()', so we disregard its contents.  Also, note that
		 * on some systems, tv_sec and tv_usec are unsigned.
		 */
		ntimeout.tv_sec  = whole_secs;
		ntimeout.tv_usec = micro_secs;

#define DELTA(f) (long)ntimeout.f - (long)returntime.f + (long)starttime.f

		delta = DELTA(tv_sec);
		if (delta < 0)
			delta = 0;
		ntimeout.tv_sec = delta;

		delta = DELTA(tv_usec);
		while (delta < 0 && ntimeout.tv_sec != 0) {
			ntimeout.tv_sec--;
			delta += 1000000;
		}
		ntimeout.tv_usec = delta;
		if (delta < 0)
			ntimeout.tv_sec = ntimeout.tv_usec = 0;

		/*
		 * If the timeout hasn't expired, and we've gotten no data,
		 * this is probably a system where 'select()' needs to be left
		 * alone so that it can complete.  Make this process sleep,
		 * then come back for more.
		 */
		if (result == 0
		 && (ntimeout.tv_sec != 0 || ntimeout.tv_usec > 100000)) {
			napms(100);
			continue;
		}
#endif
	 } while (result == -1 && errno == EINTR);

	 /* return approximate time left on the ntimeout, in milliseconds */
	 if (timeleft)
		*timeleft = (ntimeout.tv_sec * 1000) + (ntimeout.tv_usec / 1000);

	 T(("end twait: returned %d, remaining time %ld.%06ld secs (%d msec)",
		 result, ntimeout.tv_sec, ntimeout.tv_usec,
		timeleft ? *timeleft : -1));

	 return(result);
}
