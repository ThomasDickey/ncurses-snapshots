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

#include "system.h"

/*
 *	lib_trace.c - Tracing/Debugging routines
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#ifndef TRACE
#define TRACE			/* turn on internal defs for this module */
#endif

#include "curses.priv.h"

#if !HAVE_EXTERN_ERRNO
extern int errno;
#endif

#if !HAVE_STRERROR
extern char *strerror(int);
#endif

int _tracing = 0;  
char *_tputs_trace;

static int	tracefd = 2;	/* default to writing to stderr */

void trace(const unsigned int tracelevel)
{
static int	been_here = 0;

	if (! been_here) {
	   	been_here = 1;

	   	if ((tracefd = creat("trace", 0644)) < 0) {
			write(2, "curses: Can't open 'trace' file: ", 33);
			write(2, strerror(errno), strlen(strerror(errno)));
			write(2, "\n", 1);
			exit(1);
	   	}
	   	_tracef("TRACING NCURSES version %s", NCURSES_VERSION);
	}

   	_tracing = tracelevel;
}


char *_traceattr(attr_t newmode)
{
static char	buf[BUFSIZ];
struct {unsigned int val; char *name;}
names[] =
    {
	{A_STANDOUT,	"A_STANDOUT, ",},
	{A_UNDERLINE,	"A_UNDERLINE, ",},	
	{A_REVERSE,	"A_REVERSE, ",},
	{A_BLINK,	"A_BLINK, ",},
	{A_DIM,		"A_DIM, ",},
	{A_BOLD,	"A_BOLD, ",},	
	{A_ALTCHARSET,	"A_ALTCHARSET, ",},	
#ifdef A_PCCHARSET
	{A_PCCHARSET,	"A_PCCHARSET, ",},
#endif /* A_PCCHARSET */
	{A_INVIS,	"A_INVIS, ",},
	{A_PROTECT,	"A_PROTECT, ",},
	{A_CHARTEXT,	"A_CHARTEXT, ",},	
	{A_NORMAL,	"A_NORMAL, ",},
    },
colors[] =
    {
	{COLOR_BLACK,	"COLOR_BLACK",},
	{COLOR_RED,	"COLOR_RED",},
	{COLOR_GREEN,	"COLOR_GREEN",},
	{COLOR_YELLOW,	"COLOR_YELLOW",},
	{COLOR_BLUE,	"COLOR_BLUE",},
	{COLOR_MAGENTA,	"COLOR_MAGENTA",},
	{COLOR_CYAN,	"COLOR_CYAN",},
	{COLOR_WHITE,	"COLOR_WHITE",},
    },
    *sp;

	strcpy(buf, "{");
	for (sp = names; sp->val; sp++)
	    if (newmode & sp->val)
		strcat(buf, sp->name);
	if (newmode & A_COLOR)
	{
	    int pairnum = PAIR_NUMBER(newmode);

	    if (color_pairs)
		(void) sprintf(buf + strlen(buf),
			       "COLOR_PAIR(%d) = (%s, %s), ",
			       pairnum,
			       colors[FG(color_pairs[pairnum])].name,
			       colors[BG(color_pairs[pairnum])].name
			       );
	    else
		(void) sprintf(buf + strlen(buf), "COLOR_PAIR(%d) ", pairnum);
	}
	if ((newmode & A_ATTRIBUTES) == 0)
	    strcat(buf,"A_NORMAL, ");
	if (buf[strlen(buf) - 2] == ',')
	    buf[strlen(buf) - 2] = '\0';
	return(strcat(buf,"}"));
}

char *visbuf(const char *buf)
/* visibilize a given string */
{
static char vbuf[BUFSIZ];
char *tp = vbuf;

	if (buf == (char *)NULL)
	    return("(null)");

    	while (*buf) {
		if (isascii(*buf) && isgraph(*buf))
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
		} else if (isascii(*buf) && iscntrl(*buf)) {
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

char *_tracechar(const unsigned char ch)
{
    static char crep[20];
    /* 
     * We can show the actual character if it's either an ordinary printable
     * or one of the high-half characters.
     */
    if (isprint(ch) || (ch & 0x80))
    {
	crep[0] = '\'';
	crep[1] = ch;	/* necessary; printf tries too hard on metachars */
	(void) sprintf(crep + 2, "' = 0x%02x", ch);
    }
    else
	(void) sprintf(crep, "0x%02x", ch);
    return(crep);
}

void _tracedump(char *name, WINDOW *win)
{
    int	i, n;

    for (n = 0; n <= win->_maxy; n++)
    {
	char	buf[BUFSIZ], *ep;
	int j, haveattrs, havecolors;

	/* dump A_CHARTEXT part */
	(void) sprintf(buf, "%s[%2d]='", name, n);
	ep = buf + strlen(buf);
	for (j = 0; j <= win->_maxx; j++)
	    ep[j] = win->_line[n].text[j] & A_CHARTEXT;
	ep[j] = '\'';
	ep[j+1] = '\0';
	_tracef(buf);

	/* dump A_COLOR part, will screw up if there are more than 96 */
	havecolors = 0;
	for (j = 0; j <= win->_maxx; j++)
	    if (win->_line[n].text[j] & A_COLOR)
	    {
		havecolors = 1;
		break;
	    }
	if (havecolors)
	{
	    (void) sprintf(buf, "%*s[%2d]='", strlen(name), "colors", n);
	    ep = buf + strlen(buf);
	    for (j = 0; j <= win->_maxx; j++)
		ep[j] = ((win->_line[n].text[j] >> 8) & 0xff) + ' ';
	    ep[j] = '\'';
	    ep[j+1] = '\0';
	    _tracef(buf);
	}

	for (i = 0; i < 4; i++)
	{
	    const char	*hex = " 123456789ABCDEF";
	    chtype	mask = (0xf << ((i + 4) * 4));

	    haveattrs = 0;
	    for (j = 0; j <= win->_maxx; j++)
		if (win->_line[n].text[j] & mask)
		{
		    haveattrs = 1;
		    break;
		}
	    if (haveattrs)
	    {
		(void) sprintf(buf, "%*s%d[%2d]='", strlen(name)-1, "attrs", i, n);
		ep = buf + strlen(buf);
		for (j = 0; j <= win->_maxx; j++)
		    ep[j] = hex[(win->_line[n].text[j] & mask) >> ((i + 4) * 4)];
		ep[j] = '\'';
		ep[j+1] = '\0';
		_tracef(buf);
	    }
	}
    }
}

void
_tracef(char *fmt, ...)
{
va_list ap;
char buffer[BUFSIZ];

	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	write(tracefd, buffer, strlen(buffer));
	write(tracefd, "\n", 1);
}

