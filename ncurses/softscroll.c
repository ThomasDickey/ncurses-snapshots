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
#include <curses.priv.h>

MODULE_ID("$Id: softscroll.c,v 1.1 1997/08/09 14:40:36 tom Exp $")

/*
 * Compute indices for the given WINDOW, preparing it for scrolling.
 *
 * TODO (this implementation is only for proof-of-concept)
 *	(a) ensure that curscr's oldindex values are cached properly so we
 *	    don't have to recompute them on each pass.
 *	(b) investigate if there are gains to be made by iterating newscr's
 *	    row indices outward from the current position, rather than by
 *	    all rows.
 */
static void compute_curscr(void)
{
	int y, x, z;
	for (y = 0; y < LINES; y++) {
		struct ldat *nline = &curscr->_line[y];
		int found = y;
		for (z = 0; z < y; z++) {
			int same = TRUE;
			struct ldat *oline = &curscr->_line[z];
			for (x = 0; x < COLS; x++) {
				if (nline->text[x] != oline->text[x]) {
					same = FALSE;
					break;
				}
			}
			if (same) {
				found = z;
				break;
			}
		}
		nline->oldindex = found;
	}
}

static void compute_newscr(void)
{
	int y, x, z;
	for (y = 0; y < LINES; y++) {
		struct ldat *nline = &newscr->_line[y];
		int found = _NEWINDEX;
		for (z = 0; z < LINES; z++) {
			int same = TRUE;
			struct ldat *oline = &curscr->_line[z];
			for (x = 0; x < COLS; x++) {
				if (nline->text[x] != oline->text[x]) {
					same = FALSE;
					break;
				}
			}
			if (same) {
				found = z;
				break;
			}
		}
		nline->oldindex = found;
	}
}

void
_nc_setup_scroll(void)
{
#ifdef TRACE
	if (_nc_tracing & TRACE_UPDATE) {
		_tracef("_nc_setup_scroll");
		_nc_linedump();
	}
#endif /* TRACE */
	compute_curscr();
	compute_newscr();
#ifdef TRACE
	if (_nc_tracing & TRACE_UPDATE) {
		_tracef("..._nc_setup_scroll");
		_nc_linedump();
	}
#endif
}
