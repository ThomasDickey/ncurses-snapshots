/******************************************************************************
 * Copyright 1997 by Thomas E. Dickey <dickey@clark.net>                      *
 * All Rights Reserved.                                                       *
 *                                                                            *
 * Permission to use, copy, modify, and distribute this software and its      *
 * documentation for any purpose and without fee is hereby granted, provided  *
 * that the above copyright notice appear in all copies and that both that    *
 * copyright notice and this permission notice appear in supporting           *
 * documentation, and that the name of the above listed copyright holder(s)   *
 * not be used in advertising or publicity pertaining to distribution of the  *
 * software without specific, written prior permission.                       *
 *                                                                            *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD   *
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND  *
 * FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE  *
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES          *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN      *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR *
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                *
 ******************************************************************************/
/*
 * Module that "owns" the 'cur_term' variable:
 *
 *	TERMINAL *set_curterm(TERMINAL *)
 *	int del_curterm(TERMINAL *)
 */

#include <curses.priv.h>
#include <term.h>	/* TTY, cur_term */

MODULE_ID("$Id: lib_cur_term.c,v 1.1 1997/09/02 22:54:44 tom Exp $")

TERMINAL *cur_term;

int _nc_get_curterm(TTY *buf)
{
	if (cur_term == 0
	 || GET_TTY(cur_term->Filedes, buf) != 0)
		return(ERR);
	return (OK);
}

int _nc_set_curterm(TTY *buf)
{
	if (cur_term == 0
	 || SET_TTY(cur_term->Filedes, buf) != 0)
		return(ERR);
	return (OK);
}

TERMINAL *set_curterm(TERMINAL *term)
{
	TERMINAL *oldterm = cur_term;

	cur_term = term;
	return oldterm;
}

int del_curterm(TERMINAL *term)
{
	T((T_CALLED("del_curterm(%p)"), term));

	if (term != 0) {
		FreeIfNeeded(term->type.str_table);
		FreeIfNeeded(term->type.term_names);
		free(term);
		returnCode(OK);
	}
	returnCode(ERR);
}
