
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
 *	nl.c
 *
 *	Routines:
 *		nl()
 *		nonl()
 *
 */

#include <curses.priv.h>
#include <term.h>	/* cur_term */

MODULE_ID("$Id: lib_nl.c,v 1.1 1998/02/02 06:21:16 J.T.Conklin Exp $")

#ifdef __EMX__
#include <io.h>
#include <fcntl.h>
#endif

int nl(void)
{
	T((T_CALLED("nl()")));

	SP->_nl = TRUE;

#ifdef __EMX__
	fflush(SP->_ofp);
	_fsetmode(SP->_ofp, "t");
#endif

	returnCode(OK);
}

int nonl(void)
{
	T((T_CALLED("nonl()")));

	SP->_nl = FALSE;

#ifdef __EMX__
	fflush(SP->_ofp);
	_fsetmode(SP->_ofp, "b");
#endif

	returnCode(OK);
}
