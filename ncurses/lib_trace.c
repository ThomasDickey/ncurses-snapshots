
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

MODULE_ID("$Id: lib_trace.c,v 1.16 1997/02/09 00:45:37 tom Exp $")

#include <ctype.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

unsigned _nc_tracing = 0;
const char *_nc_tputs_trace = "";
long _nc_outchars;
int _nc_optimize_enable = OPTIMIZE_ALL;

static FILE *	tracefp;	/* default to writing to stderr */

void trace(const unsigned int tracelevel)
{
static bool	been_here = FALSE;

	if (! been_here && tracelevel) {
		been_here = TRUE;

		if ((tracefp = fopen("trace", "w")) == 0) {
			perror("curses: Can't open 'trace' file: ");
			exit(EXIT_FAILURE);
		}
		/* Try to set line-buffered mode, or (failing that) unbuffered,
		 * so that the trace-output gets flushed automatically at the
		 * end of each line.  This is useful in case the program dies. 
		 */
#if HAVE_SETVBUF	/* ANSI */
		(void) setvbuf(tracefp, (char *)0, _IOLBF, 0);
#elif HAVE_SETBUF	/* POSIX */
		(void) setbuffer(tracefp, (char *)0);
#endif
		_tracef("TRACING NCURSES version %s", NCURSES_VERSION);
	}

   	_nc_tracing = tracelevel;
}

const char *_nc_visbuf(const char *buf)
/* visibilize a given string */
{
static size_t have;
static char *vbuf;
size_t need;
char *tp = vbuf;

	if (buf == 0)
	    return("(null)");

	if ((need = ((strlen(buf) * 4) + 5)) > have) {
		free(vbuf);
		tp = vbuf = malloc(have = need);
	}

	*tp++ = '"';
    	while (*buf) {
		if (*buf == '"') {
			*tp++ = '\\';
			*tp++ = '"';
		} else if (is7bits(*buf) && (isgraph(*buf) || *buf == ' '))
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
	*tp++ = '"';
	*tp++ = '\0';
	return(vbuf);
}

void
_tracef(const char *fmt, ...)
{
static const char Called[] = T_CALLED("");
static const char Return[] = T_RETURN("");
static int level;
va_list ap;
bool	before = FALSE;
bool	after = FALSE;
int	doit = _nc_tracing;

	if (strlen(fmt) > sizeof(Called)) {
		if (!strncmp(fmt, Called, sizeof(Called)-1)) {
			before = TRUE;
			level++;
		} else if (!strncmp(fmt, Return, sizeof(Return)-1)) {
			after = TRUE;
		}
		if (before || after) {
			if ((level <= 1)
			 || (doit & TRACE_ICALLS) != 0)
				doit &= TRACE_CALLS;
			else
				doit = 0;
		}
	}

	if (doit != 0) {
		if (tracefp == 0)
			tracefp = stderr;
		if (before || after) {
			int n;
			for (n = 1; n < level; n++)
				fputs("+ ", tracefp);
		}
		va_start(ap, fmt);
		vfprintf(tracefp, fmt, ap);
		fputc('\n', tracefp);
		va_end(ap);
		fflush(tracefp);
	}

	if (after && level)
		level--;
}

/* Trace 'int' return-values */
int _nc_retrace_int(int code)
{
	T((T_RETURN("%d"), code));
	return code;
}

/* Trace 'WINDOW *' return-values */
WINDOW *_nc_retrace_win(WINDOW *code)
{
	T((T_RETURN("%p"), code));
	return code;
}
