
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
**	lib_set_term.c
**
**	The routine set_term().
**
*/

#include <curses.priv.h>

#include <term.h>	/* cur_term */

MODULE_ID("$Id: lib_set_term.c,v 1.10 1996/07/30 22:49:24 tom Exp $")

SCREEN * set_term(SCREEN *screen)
{
SCREEN	*oldSP;

	T(("set_term(%p) called", screen));

	oldSP = SP;
	_nc_set_screen(screen);

	cur_term = SP->_term;
	curscr   = SP->_curscr;
	newscr   = SP->_newscr;
	stdscr   = SP->_stdscr;

	return(oldSP);
}

void delscreen(SCREEN *sp)
{
	free(sp);
}

ripoff_t rippedoff[5], *rsp = rippedoff;
#define N_RIPS (int)(sizeof(rippedoff)/sizeof(rippedoff[0]))

int _nc_setupscreen(short slines, short const scolumns, FILE *output)
/* OS-independent screen initializations */
{
int	bottom_stolen = 0, i;

	if (!_nc_alloc_screen())
		return ERR;

	SP->_term        = cur_term;
	SP->_lines       = slines;
	SP->_lines_avail = slines;
	SP->_columns     = scolumns;
	SP->_cursrow     = -1;
	SP->_curscol     = -1;
	SP->_keytry      = UNINITIALISED;
	SP->_nl          = TRUE;
	SP->_raw         = FALSE;
	SP->_cbreak      = FALSE;
	SP->_echo        = TRUE;
	SP->_fifohead    = -1;
	SP->_fifotail    = 0;
	SP->_fifopeek    = 0;
	SP->_endwin      = TRUE;
	SP->_ofp         = output;      /* (may be overridden later) */
	SP->_coloron     = 0;
	SP->_stdscr      = (WINDOW*)NULL;
	SP->_topstolen   = 0;

	init_acs();

	T(("creating newscr"));
	if ((newscr = newwin(slines, scolumns, 0, 0)) == (WINDOW *)NULL)
		return ERR;

	T(("creating curscr"));
	if ((curscr = newwin(slines, scolumns, 0, 0)) == (WINDOW *)NULL)
		return ERR;

	SP->_newscr = newscr;
	SP->_curscr = curscr;

	newscr->_clear = TRUE;
	curscr->_clear = FALSE;

	for (i=0, rsp = rippedoff; rsp->line && (i < N_RIPS); rsp++, i++) {
	  if (rsp->hook) {
	      WINDOW *w;
	      int count = (rsp->line < 0) ? -rsp->line : rsp->line;

	      if (rsp->line < 0) {
		  w = newwin(count,scolumns,SP->_lines_avail - count,0);
		  if (w) {
		      rsp->w = w;
		      rsp->hook(w, scolumns);
		      bottom_stolen += count;
		  }
		  else
		    return ERR;
	      } else {
		  w = newwin(count,scolumns, 0, 0);
		  if (w) {
		      rsp->w = w;
		      rsp->hook(w, scolumns);
		      SP->_topstolen += count;
		  }
		  else
		    return ERR;
	      }
	      SP->_lines_avail -= count;
	  }
	}

	T(("creating stdscr"));
	assert ((SP->_lines_avail + SP->_topstolen + bottom_stolen) == slines);
	if ((stdscr = newwin(LINES = SP->_lines_avail, scolumns, 0, 0)) == NULL)
		return ERR;
	SP->_stdscr = stdscr;

	def_shell_mode();
	def_prog_mode();

	return OK;
}

/* The internal implementation interprets line as the number of
   lines to rip off from the top or bottom.
   */
int
_nc_ripoffline(int line, int (*init)(WINDOW *,int))
{
    if (line == 0)
	return(OK);

    if (rsp >= rippedoff + N_RIPS)
	return(ERR);

    rsp->line = line;
    rsp->hook = init;
    rsp->w    = (WINDOW *)NULL;
    rsp++;

    return(OK);
}

int
ripoffline(int line, int (*init)(WINDOW *, int))
{
    if (line == 0)
	return(OK);

    return _nc_ripoffline ((line<0) ? -1 : 1, init);
}
