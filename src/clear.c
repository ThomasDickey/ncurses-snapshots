
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
 * clear.c --  clears the terminal's screen
 */

#include <curses.h>
#include <stdlib.h>
#include "config.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <term.h>

static int putch(int c)
{
	return putchar(c);
}

int main(int argc, char *argv[])
{
	setupterm((char *) 0, STDOUT_FILENO, (int *) 0); 
	if (clear_screen == (char *) 0)
		exit(1);
	tputs(clear_screen, lines > 0 ? lines : 1, putch);
	return 0;
}

