
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


#include <stdlib.h>
#include <string.h>
#include "curses.priv.h"
#include "term.h"

int tigetflag(char *str)
{
int i;

	T(("tigetflag(%s)", str));

	for (i = 0; i < BOOLCOUNT; i++)
		if (!strcmp(str, boolnames[i]))
			return cur_term->type.Booleans[i];

	return -1;
}

int tigetnum(char *str)
{
int i;

	T(("tigetnum(%s)", str));

	for (i = 0; i < NUMCOUNT; i++)
		if (!strcmp(str, numnames[i]))
			return cur_term->type.Numbers[i];

	return -2;
}

char *tigetstr(char *str)
{
int i;

	T(("tigetstr(%s)", str));

	for (i = 0; i < STRCOUNT; i++)
		if (!strcmp(str, strnames[i]))
			return cur_term->type.Strings[i];

	return (char *)-1;
}
