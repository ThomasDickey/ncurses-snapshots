
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
 * Terminfo-only terminal setup routines:
 *
 *		int restartterm(const char *, int, int *)
 *		TERMINAL *set_curterm(TERMINAL *)
 *		int del_curterm(TERMINAL *)
 */

#include <curses.priv.h>

#include <string.h>

#ifdef SVR4_TERMIO
#define _POSIX_SOURCE
#endif

#include "term.h"	/* lines, columns, cur_term */

#undef tabs

#ifdef TAB3
# define tabs TAB3
#else
# ifdef XTABS
#  define tabs XTABS
# else
#  ifdef OXTABS
#   define tabs OXTABS
#  else
#   define tabs 0
#  endif
# endif
#endif
 
int def_shell_mode(void)
{
    /*
     *	Turn off the XTABS bit in the tty structure if it was on
     *	If XTABS was on, remove the tab and backtab capabilities.
     */

	T(("def_shell_mode() called"));

#ifdef TERMIOS
 	if((tcgetattr(cur_term->Filedes, &cur_term->Ottyb)) == -1) {
		return ERR;
 	}
 	if (cur_term->Ottyb.c_oflag & tabs)
		tab = back_tab = NULL;
	
#else
	gtty(cur_term->Filedes, &cur_term->Ottyb);
	if (cur_term->Ottyb.sg_flags & XTABS)
	    	tab = back_tab = NULL;
#endif
	return OK;
}

int def_prog_mode(void)
{
	T(("def_prog_mode() called"));

#ifdef TERMIOS
 	if((tcgetattr(cur_term->Filedes, &cur_term->Nttyb)) == -1) {
		return ERR;
 	}
 	cur_term->Nttyb.c_oflag &= ~tabs;
#else
	gtty(cur_term->Filedes, &cur_term->Nttyb);
	
	cur_term->Nttyb.sg_flags &= ~XTABS;
#endif
	return OK;
}

int restartterm(const char *term, int filenum, int *errret)
{
int saveecho = SP->_echo;
int savecbreak = SP->_cbreak;
int saveraw = SP->_raw;
int savenl = SP->_nl;

	setupterm(term, filenum, errret);

	if (saveecho)
		echo();
	else
		noecho();

	if (savecbreak) {
		cbreak();
		noraw();
	} else if (saveraw) {
		nocbreak();
		raw();
	} else {
		nocbreak();
		noraw();
	}
	if (savenl)
		nl();
	else
		nonl();

	reset_prog_mode();

	_nc_get_screensize();

	return(OK);
}

TERMINAL *set_curterm(TERMINAL *term)
{
	TERMINAL	*oldterm = cur_term;

	cur_term = term;
	return oldterm;
}

int del_curterm(TERMINAL *term)
{

	if (term != NULL) {
		if (term->type.str_table != NULL)
			free(term->type.str_table);
		free(term);
		return OK;
	}
	return ERR;
}

