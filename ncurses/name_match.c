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

#include <curses.priv.h>
#include <term.h>
#include <tic.h>

MODULE_ID("$Id: name_match.c,v 1.1 1997/06/01 00:18:48 tom Exp $")

/*
 *	_nc_first_name(char *names)
 *
 *	Extract the primary name from a compiled entry.
 */

char *_nc_first_name(const char *const sp)
/* get the first name from the given name list */
{
    static char	buf[MAX_NAME_SIZE];
    register char *cp;

    (void) strcpy(buf, sp);

    cp = strchr(buf, '|');
    if (cp)
	*cp = '\0';

    return(buf);
}

/*
 *	bool _nc_name_match(namelist, name, delim)
 *
 *	Is the given name matched in namelist?
 */

int _nc_name_match(const char *const namelst, const char *const name, const char *const delim)
/* microtune this, it occurs in several critical loops */
{
char namecopy[MAX_ENTRY_SIZE];	/* this may get called on a TERMCAP value */
register char *cp;

	if (namelst == 0)
		return(FALSE);
	(void) strcpy(namecopy, namelst);
	if ((cp = strtok(namecopy, delim)) != 0) {
		do {
			/* avoid strcmp() function-call cost if possible */
			if (cp[0] == name[0] && strcmp(cp, name) == 0)
			    return(TRUE);
		} while
		    ((cp = strtok((char *)0, delim)) != 0);
	}
	return(FALSE);
}
