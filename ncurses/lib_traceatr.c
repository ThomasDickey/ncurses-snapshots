
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

#include <curses.priv.h>
#include <term.h>	/* acs_chars */

#include <string.h>

MODULE_ID("$Id: lib_traceatr.c,v 1.8 1996/07/31 00:19:49 tom Exp $")

char *_traceattr(attr_t newmode)
{
static char	buf[BUFSIZ];
static const	struct {unsigned int val; char *name;}
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

char *_tracechtype(chtype ch)
{
    static char	buf[BUFSIZ];

    if (ch & A_ALTCHARSET)
    {
	bool	found = FALSE;
	char	*cp;
	static const	struct {unsigned int val; char *name;}
	names[] =
	{
	    {'l', "ACS_ULCORNER"},	/* upper left corner */
	    {'m', "ACS_LLCORNER"},	/* lower left corner */
	    {'k', "ACS_URCORNER"},	/* upper right corner */
	    {'j', "ACS_LRCORNER"},	/* lower right corner */
	    {'t', "ACS_LTEE"},	/* tee pointing right */
	    {'u', "ACS_RTEE"},	/* tee pointing left */
	    {'v', "ACS_BTEE"},	/* tee pointing up */
	    {'w', "ACS_TTEE"},	/* tee pointing down */
	    {'q', "ACS_HLINE"},	/* horizontal line */
	    {'x', "ACS_VLINE"},	/* vertical line */
	    {'n', "ACS_PLUS"},	/* large plus or crossover */
	    {'o', "ACS_S1"},	/* scan line 1 */
	    {'s', "ACS_S9"},	/* scan line 9 */
	    {'`', "ACS_DIAMOND"},	/* diamond */
	    {'a', "ACS_CKBOARD"},	/* checker board (stipple) */
	    {'f', "ACS_DEGREE"},	/* degree symbol */
	    {'g', "ACS_PLMINUS"},	/* plus/minus */
	    {'~', "ACS_BULLET"},	/* bullet */
	    {',', "ACS_LARROW"},	/* arrow pointing left */
	    {'+', "ACS_RARROW"},	/* arrow pointing right */
	    {'.', "ACS_DARROW"},	/* arrow pointing down */
	    {'-', "ACS_UARROW"},	/* arrow pointing up */
	    {'h', "ACS_BOARD"},	/* board of squares */
	    {'I', "ACS_LANTERN"},	/* lantern symbol */
	    {'0', "ACS_BLOCK"},	/* solid square block */
	    {'p', "ACS_S3"},	/* scan line 3 */
	    {'r', "ACS_S7"},	/* scan line 7 */
	    {'y', "ACS_LEQUAL"},	/* less/equal */
	    {'z', "ACS_GEQUAL"},	/* greater/equal */
	    {'{', "ACS_PI"},	/* Pi */
	    {'|', "ACS_NEQUAL"},	/* not equal */
	    {'}', "ACS_STERLING"},	/* UK pound sign */
	    {'\0',(char *)0}
	},
	*sp;

	for (cp = acs_chars; *cp; cp++)
	{
	    if ((chtype)cp[1] == (ch & A_CHARTEXT))
	    {
		ch = cp[0];
		found = TRUE;
		break;
	    }
	}

	if (found)
	{
	    for (sp = names; sp->val; sp++)
		if (sp->val == ch)
		{
		    (void) sprintf(buf, "%s | %s",
			    sp->name,
			    _traceattr(AttrOf(ch) & (chtype)(~A_ALTCHARSET)));
		    return(buf);
		}
	}
    }

    (void) sprintf(buf, "%s | %s",
		   _tracechar((unsigned char)(ch & A_CHARTEXT)),
		   _traceattr(AttrOf(ch)));
    return(buf);
}
