/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

#include <stdlib.h>
#include <string.h>
#include "curses.h"
#include "curses.priv.h"
#include "terminfo.h"

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
