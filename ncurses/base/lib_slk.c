/****************************************************************************
 * Copyright (c) 1998-2005,2008 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Authors:                                                                *
 *          Gerhard Fuernkranz                      1993 (original)         *
 *          Zeyd M. Ben-Halim                       1992,1995 (sic)         *
 *          Eric S. Raymond                                                 *
 *          Juergen Pfeifer                         1996-on                 *
 *          Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 *	lib_slk.c
 *	Soft key routines.
 */

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_slk.c,v 1.35.1.1 2008/11/16 00:19:59 juergen Exp $")

/*
 * Free any memory related to soft labels, return an error.
 */
static int
slk_failed(SCREEN *sp)
{
  if ((0!=sp) && sp->_slk) {
	FreeIfNeeded(sp->_slk->ent);
	free(sp->_slk);
	sp->_slk = (SLK *) 0;
    }
    return ERR;
}

NCURSES_EXPORT(int)
_nc_format_slks(SCREEN *sp, int cols)
{
    int gap, i, x;
    unsigned max_length = sp->_slk->maxlen;

    if (!sp)
      return ERR;

    if (sp->slk_format >= 3) {	/* PC style */
	gap = (cols - 3 * (3 + 4 * max_length)) / 2;

	if (gap < 1)
	    gap = 1;

	for (i = x = 0; i < sp->_slk->maxlab; i++) {
	    sp->_slk->ent[i].ent_x = x;
	    x += max_length;
	    x += (i == 3 || i == 7) ? gap : 1;
	}
    } else {
	if (sp->slk_format == 2) {	/* 4-4 */
	    gap = cols - (sp->_slk->maxlab * max_length) - 6;

	    if (gap < 1)
		gap = 1;
	    for (i = x = 0; i < sp->_slk->maxlab; i++) {
		sp->_slk->ent[i].ent_x = x;
		x += max_length;
		x += (i == 3) ? gap : 1;
	    }
	} else {
	    if (sp->slk_format == 1) {	/* 1 -> 3-2-3 */
		gap = (cols - (sp->_slk->maxlab * max_length) - 5)
		/ 2;

		if (gap < 1)
		    gap = 1;
		for (i = x = 0; i < sp->_slk->maxlab; i++) {
		    sp->_slk->ent[i].ent_x = x;
		    x += max_length;
		    x += (i == 2 || i == 4) ? gap : 1;
		}
	    } else
		returnCode(slk_failed(sp));
	}
    }
    sp->_slk->dirty = TRUE;

    return OK;
}

/*
 * Initialize soft labels.
 * Called from newterm()
 */
NCURSES_EXPORT(int)
_nc_slk_initialize(WINDOW *stwin, int cols)
{
    int i;
    int res = OK;
    unsigned max_length;
    SCREEN *sp;
    TERMINAL* term;
    int numlab;

    T((T_CALLED("_nc_slk_initialize()")));

    assert(stwin);
    
    sp = _nc_screen_of(stwin);
    if (0 == sp)
	returnCode(ERR);

    term = TerminalOf(sp);
    assert(term);

    numlab = InfoOf(sp)->numlabels;

    if (sp->_slk) {		/* we did this already, so simply return */
	returnCode(OK);
    } else if ((sp->_slk = typeCalloc(SLK, 1)) == 0)
	returnCode(ERR);

    sp->_slk->hidden = TRUE;
    sp->_slk->ent = NULL;
    if (!sp->slk_format)
      sp->slk_format = _nc_globals.slk_format;

    /*
     * If we use colors, vidputs() will suppress video attributes that conflict
     * with colors.  In that case, we're still guaranteed that "reverse" would
     * work.
     */
    if ((InfoOf(sp)->nocolorvideo & 1) == 0)
	SetAttr(sp->_slk->attr, A_STANDOUT);
    else
	SetAttr(sp->_slk->attr, A_REVERSE);

    sp->_slk->maxlab = ((numlab > 0)
			? numlab
			: MAX_SKEY(sp->slk_format));
    sp->_slk->maxlen = ((numlab > 0)
			? InfoOf(sp)->labelwidth * InfoOf(sp)->labelheight
			: MAX_SKEY_LEN(sp->slk_format));
    sp->_slk->labcnt = ((sp->_slk->maxlab < MAX_SKEY(sp->slk_format))
			? MAX_SKEY(sp->slk_format)
			: sp->_slk->maxlab);

    if (sp->_slk->maxlen <= 0
	|| sp->_slk->labcnt <= 0
	|| (sp->_slk->ent = typeCalloc(slk_ent,
				       (unsigned) sp->_slk->labcnt)) == NULL)
	returnCode(slk_failed(sp));

    max_length = sp->_slk->maxlen;
    for (i = 0; i < sp->_slk->labcnt; i++) {
	size_t used = max_length + 1;

	if ((sp->_slk->ent[i].ent_text = (char *) _nc_doalloc(0, used)) == 0)
	    returnCode(slk_failed(sp));
	memset(sp->_slk->ent[i].ent_text, 0, used);

	if ((sp->_slk->ent[i].form_text = (char *) _nc_doalloc(0, used)) == 0)
	    returnCode(slk_failed(sp));
	memset(sp->_slk->ent[i].form_text, 0, used);

	memset(sp->_slk->ent[i].form_text, ' ', max_length);
	sp->_slk->ent[i].visible = (char) (i < sp->_slk->maxlab);
    }

    res = _nc_format_slks(sp,cols);

    if ((sp->_slk->win = stwin) == NULL) {
	returnCode(slk_failed(sp));
    }

    /* We now reset the format so that the next newterm has again
     * per default no SLK keys and may call slk_init again to
     * define a new layout. (juergen 03-Mar-1999)
     */
    _nc_globals.slk_format = 0;
    returnCode(res);
}

/*
 * Restore the soft labels on the screen.
 */
NCURSES_EXPORT(int)
NC_SNAME(slk_restore)(SCREEN* sp)
{
  T((T_CALLED("slk_restore(%p)"), sp));

    if (0 == sp)
        returnCode(ERR);
    if (sp->_slk == NULL)
	returnCode(ERR);
    sp->_slk->hidden = FALSE;
    sp->_slk->dirty  = TRUE;

    returnCode(NC_SNAME(slk_refresh)(sp));
}
 
NCURSES_EXPORT(int)
slk_restore (void)
{
    return NC_SNAME(slk_restore)(CURRENT_SCREEN);
}
