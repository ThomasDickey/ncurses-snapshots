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

#include "system.h"

/*
**	lib_addch.c
**
**	The routine waddch().
**
*/

#include <ctype.h>
#include "unctrl.h"
#include "curses.priv.h"

#define ALL_BUT_COLOR ((chtype)~(A_COLOR))

int TABSIZE = 8; /* default size of a TAB */

int wattron(WINDOW *win, const attr_t at)
{
	T(("wattron(%p,%s) current = %s", win, _traceattr(at), _traceattr(win->_attrs)));
	if (PAIR_NUMBER(at) > 0x00) {
		win->_attrs = (win->_attrs & ALL_BUT_COLOR) | at ;
		T(("new attribute is %s", _traceattr(win->_attrs)));
	} else {
		win->_attrs |= at;
		T(("new attribute is %s", _traceattr(win->_attrs)));
	}
	return OK;
}

int wattroff(WINDOW *win, const attr_t at)
{
#define IGNORE_COLOR_OFF FALSE

	T(("wattroff(%p,%s) current = %s", win, _traceattr(at), _traceattr(win->_attrs)));
	if (IGNORE_COLOR_OFF == TRUE) {
		if (PAIR_NUMBER(at) == 0xff) /* turn off color */
			win->_attrs &= ~at;
		else /* leave color alone */
			win->_attrs &= ~(at|ALL_BUT_COLOR);
	} else {
		if (PAIR_NUMBER(at) > 0x00) /* turn off color */
			win->_attrs &= ~at;
		else /* leave color alone */
			win->_attrs &= ~(at|ALL_BUT_COLOR);
	}
	T(("new attribute is %s", _traceattr(win->_attrs)));
	return OK;
}

inline chtype _nc_render(WINDOW *win, chtype oldch, chtype newch, bool erase)
/* compute a rendition of the given char correct for the current context */
{
	if ((oldch & A_CHARTEXT) == ' ')
		newch |= win->_bkgd;
	else if (!(newch & A_ATTRIBUTES))
		newch |= (win->_bkgd & A_ATTRIBUTES);
	TR(TRACE_VIRTPUT, ("bkg = %lx -> ch = %lx", win->_bkgd, newch));

	if (!erase) {
		TR(TRACE_VIRTPUT, ("win attr = %s", _traceattr(win->_attrs)));
		newch |= win->_attrs;
	}

	return(newch);
}

static inline int
wladdch(WINDOW *win, const chtype c, const bool literal)
{
int	x, y;
int	newx;
chtype	ch = c;

	x = win->_curx;
	y = win->_cury;

	if (y > win->_maxy  ||  x > win->_maxx  ||  y < 0  ||  x < 0)
	    	return(ERR);

#ifdef A_PCCHARSET
	if (ch & A_PCCHARSET)
		goto noctrl;
#endif /* A_PCCHARSET */

	if (ch & A_ALTCHARSET)
		goto noctrl;

	/* ugly, but necessary --- and, bizarrely enough, even portable! */
	if (literal)
	    	goto noctrl;

	switch (ch&A_CHARTEXT) {
    	case '\t':
		for (newx = x + (8 - (x & 07)); x < newx; x++)
	    		if (waddch(win, ' ') == ERR)
				return(ERR);
		return(OK);
    	case '\n':
		wclrtoeol(win);
		x = 0;
		goto do_newline;
    	case '\r':
		x = 0;
		break;
    	case '\b':
		if (--x < 0)
		    	x = 0;
		break;
    	default:
		if (isascii(ch & A_CHARTEXT) && iscntrl(ch & A_CHARTEXT))
		    	return(waddstr(win, unctrl(ch)));

		/* FALL THROUGH */
        noctrl:
		ch = _nc_render(win, win->_line[y].text[x], ch, FALSE);

		if (win->_line[y].text[x] != ch) {
		    	if (win->_line[y].firstchar == _NOCHANGE)
				win->_line[y].firstchar = win->_line[y].lastchar = x;
		    	else if (x < win->_line[y].firstchar)
				win->_line[y].firstchar = x;
		    	else if (x > win->_line[y].lastchar)
				win->_line[y].lastchar = x;

		}

		win->_line[y].text[x++] = ch;
		TR(TRACE_VIRTPUT, ("(%d, %d) = %s | %s", 
				   y, x,
				   _tracechar(ch & A_CHARTEXT),
				   _traceattr((ch & (chtype)A_ATTRIBUTES))));
		if (x > win->_maxx) {
		    	x = 0;
do_newline:
		    	y++;
		    	if (y > win->_regbottom) {
				y--;
				if (win->_scroll)
				    	scroll(win);
		    	}
		}
		break;
	}

	win->_curx = x;
	win->_cury = y;

	TR(TRACE_VIRTPUT, ("waddch() is done"));

	_nc_synchook(win);
	return(OK);
}

int waddch(WINDOW *win, const chtype ch)
{
	TR(TRACE_VIRTPUT, ("waddch(%p, %s | %s) called", win,
			  _tracechar(ch & A_CHARTEXT),
			  _traceattr((ch & (chtype)A_ATTRIBUTES))));
	return wladdch(win, ch, FALSE);
}

int wechochar(WINDOW *win, chtype ch)
{
	TR(TRACE_VIRTPUT, ("wechochar(%p,%s (%s)) called", win,
			  _tracechar(ch & A_CHARTEXT),
			  _traceattr((ch & (chtype)A_ATTRIBUTES))));

	return wladdch(win, ch, TRUE);
}
