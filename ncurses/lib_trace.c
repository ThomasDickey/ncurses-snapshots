
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
 *	lib_trace.c - Tracing/Debugging routines
 */

#ifndef TRACE
#define TRACE			/* turn on internal defs for this module */
#endif

#include <curses.priv.h>

MODULE_ID("$Id: lib_trace.c,v 1.12 1996/12/21 15:11:11 tom Exp $")

#include <ctype.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

unsigned _nc_tracing = 0;
char *_nc_tputs_trace;
long _nc_outchars;
int _nc_optimize_enable = OPTIMIZE_ALL;

static int	tracefd = 2;	/* default to writing to stderr */

void trace(const unsigned int tracelevel)
{
static bool	been_here = FALSE;

	if (! been_here && tracelevel) {
		been_here = TRUE;

		if ((tracefd = creat("trace", 0644)) < 0) {
			perror("curses: Can't open 'trace' file: ");
			exit(EXIT_FAILURE);
		}
		_tracef("TRACING NCURSES version %s", NCURSES_VERSION);
	}

   	_nc_tracing = tracelevel;
}

char *_nc_visbuf(const char *buf)
/* visibilize a given string */
{
static char vbuf[BUFSIZ];
char *tp = vbuf;

	if (buf == (char *)NULL)
	    return("(null)");

    	while (*buf) {
		if (is7bits(*buf) && (isgraph(*buf) || *buf == ' '))
			*tp++ = *buf++;
		else if (*buf == '\n') {
			*tp++ = '\\'; *tp++ = 'n';
			buf++;
		}
		else if (*buf == '\r') {
			*tp++ = '\\'; *tp++ = 'r';
			buf++;
		} else if (*buf == '\b') {
			*tp++ = '\\'; *tp++ = 'b';
			buf++;
		} else if (*buf == '\033') {
			*tp++ = '\\'; *tp++ = 'e';
			buf++;
		} else if (is7bits(*buf) && iscntrl(*buf)) {
			*tp++ = '\\'; *tp++ = '^'; *tp++ = '@' + *buf;
			buf++;
		} else {
			sprintf(tp, "\\0x%02x", *buf++);
			tp += strlen(tp);
		}
    }
    *tp++ = '\0';
    return(vbuf);
}

void
_tracef(const char *fmt, ...)
{
va_list ap;
char buffer[BUFSIZ];

	if (_nc_tracing) {
		va_start(ap, fmt);
		vsprintf(buffer, fmt, ap);
		write(tracefd, buffer, strlen(buffer));
		write(tracefd, "\n", 1);
	}
}
