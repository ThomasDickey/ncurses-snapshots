
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


#include "curses.priv.h"
#include "terminfo.h"

/* 
   some of the code in here was contributed by:
   Magnus Bengtsson, d6mbeng@dtek.chalmers.se
*/

char PC = '\0';		/* pad character */
char *UP = NULL;	/* up one line capability */
char *BC = NULL;	/* backspace if not ^h */
speed_t ospeed = 0;	/* output speed */

/***************************************************************************
 *
 * tgetent(bufp, term)
 *
 * In termcap, this function reads in the entry for terminal `term' into the
 * buffer pointed to by bufp. It must be called before any of the functions
 * below are called.
 * In this terminfo emulation, tgetent() simply calls setupterm() (which
 * does a bit more than tgetent() in termcap does), and returns its return
 * value (1 if successful, 0 if no terminal with the given name could be
 * found, or -1 if no terminal descriptions have been installed on the
 * system). bufp is ignored.
 *
 ***************************************************************************/

int tgetent(char *bp, char *name)
{
int errcode;

	setupterm(name, 1, &errcode);
	if (errcode == 1) {
		if (pad_char != NULL)
			PC = pad_char[0];
		if (cursor_up != NULL)
			UP = cursor_up;
		if (backspace_if_not_bs != NULL)
			BC = backspace_if_not_bs;
	}
	return errcode;
}

/***************************************************************************
 *
 * tgetflag(str)
 *
 * Look up boolean termcap capability str and return its value (TRUE=1 if
 * present, FALSE=0 if not).
 *
 ***************************************************************************/

int tgetflag(char id[2])
{
int i;

	for (i = 0; i < BOOLCOUNT; i++)
		if (!strcmp(id, boolcodes[i])) 
			return cur_term->type.Booleans[i];
	return ERR;
}

/***************************************************************************
 *
 * tgetnum(str)
 *
 * Look up numeric termcap capability str and return its value, or -1 if
 * not given.
 *
 ***************************************************************************/

int tgetnum(char id[2])
{
int i;

	for (i = 0; i < NUMCOUNT; i++)
		if (!strcmp(id, numcodes[i])) 
			return cur_term->type.Numbers[i];
	return ERR;
}

/***************************************************************************
 *
 * tgetstr(str, area)
 *
 * Look up string termcap capability str and return a pointer to its value,
 * or NULL if not given. 
 *
 ***************************************************************************/

char *tgetstr(char id[2], char **area)
{
int i;

	for (i = 0; i < STRCOUNT; i++)
		if (!strcmp(id, strcodes[i])) 
			return cur_term->type.Strings[i];
	return NULL;
}

/*
 *	char *
 *	tgoto(string, x, y)
 *
 *	Retained solely for upward compatibility.  Note the intentional
 *	reversing of the last two arguments.
 *
 */

char *tgoto(char *string, int x, int y)
{
	return(tparm(string, y, x));
}
