
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992, 1993, 1994                 *
*                          by Zeyd M. Ben-Halim                            *
*                          zmbenhal@netcom.com                             *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, not removed   *
*        from header files, and is reproduced in any documentation         *
*        accompanying it or the applications linked with it.               *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/

/*
**	lib_initscr.c
**
**	The routines initscr(), and termname().
**
*/

#include <stdlib.h>
#include <string.h>
#include "curses.priv.h"

WINDOW *initscr()
{
  	if (newterm(getenv("TERM"), stdout, stdin) == NULL) {
  		fprintf(stderr, "Error opening terminal: %s.\n", getenv("TERM"));
  		exit(1);
	} else {
		def_shell_mode();
		def_prog_mode();
		return(stdscr);
	}
}

char *
termname(void)
{
	char	*term = getenv("TERM");
	static char	ret[15];

	T(("termname() called"));

	if (term == (char *)NULL)
		return(char *)NULL;
	else
	{
		(void) strncpy(ret, term, sizeof(ret) - 1);
		return(ret);
	}
}
