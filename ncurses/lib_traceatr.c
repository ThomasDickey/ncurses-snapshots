
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
 *	lib_traceatr.c - Tracing/Debugging routines (attributes)
 */

#ifndef TRACE
#define TRACE			/* turn on internal defs for this module */
#endif

#include "curses.priv.h"

#include <string.h>

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
	    short pairnum = PAIR_NUMBER(newmode);
	    short fg, bg;

	    if (pair_content(pairnum, &fg, &bg) == OK)
		(void) sprintf(buf + strlen(buf),
			       "COLOR_PAIR(%d) = (%s, %s), ",
			       pairnum,
			       colors[fg].name,
			       colors[bg].name
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
