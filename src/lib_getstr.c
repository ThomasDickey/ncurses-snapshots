
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
**	lib_getstr.c
**
**	The routine wgetstr().
**
*/

#include "unctrl.h"
#include "curses.priv.h"

inline void backspace(WINDOW *win)
{
	mvwaddstr(curscr, win->_begy + win->_cury, win->_begx + win->_curx,
		 "\b \b");
	waddstr(win, "\b \b");

	/*
	 * This used to do the equivalent of outstr("\b \b"), which
	 * would fail on terminals with a non-backspace cursor_left
	 * character.
	 */
	mvcur(win->_begy + win->_cury, win->_begx + win->_curx,
	      win->_begy + win->_cury, win->_begx + win->_curx - 1);
	outstr(" ");
	mvcur(win->_begy + win->_cury, win->_begx + win->_curx,
	      win->_begy + win->_cury, win->_begx + win->_curx - 1);
	SP->_curscol--; 
}

int wgetnstr(WINDOW *win, char *str, int maxlen)
{
bool	oldnl, oldecho, oldraw, oldcbreak, oldkeypad;
char	erasec;
char	killc;
char	*oldstr;
int ch;
  
	T(("wgetnstr(%p,%p, %d) called", win, str, maxlen));

	oldnl = SP->_nl;
	oldecho = SP->_echo;
	oldraw = SP->_raw;
	oldcbreak = SP->_cbreak;
	oldkeypad = win->_use_keypad;
	nl();
	noecho();
	noraw();
	cbreak();
	keypad(win, TRUE);

	erasec = erasechar();
	killc = killchar();

	oldstr = str;

	vidattr(win->_attrs);
	if (is_wintouched(win) || (win->_flags & _HASMOVED))
		wrefresh(win);

	while ((ch = wgetch(win)) != ERR) {
		if (ch == '\n' || ch == '\r')
			break;
	   	if (ch == erasec || ch == KEY_LEFT || ch == KEY_BACKSPACE) {
			if (str > oldstr) {
		    		str--;
		    		if (oldecho == TRUE)
			    		backspace(win);
			}
	 	} else if (ch == killc) {
			while (str > oldstr) {
			    	str--;
		    		if (oldecho == TRUE)
		    			backspace(win);
			}
		} else if (maxlen >= 0 && str - oldstr >= maxlen) {
		    beep();
		} else {
			if (oldecho == TRUE) {
			        char	*glyph = unctrl(ch);

				mvwaddstr(curscr, win->_begy + win->_cury,
				  	win->_begx + win->_curx, glyph);
				waddstr(win, glyph);
				outstr(glyph);
				SP->_curscol += strlen(glyph);
			} else
				*str++ = ch;
	   	}
	}

    	win->_curx = 0;
    	if (win->_cury < win->_maxy)
       		win->_cury++;
	wrefresh(win);

	if (oldnl == FALSE)
	    nonl();

	if (oldecho == TRUE)
	    echo();

	if (oldraw == TRUE)
	    raw();

	if (oldcbreak == FALSE)
	    nocbreak();

	if (oldkeypad == FALSE)
		keypad(win, FALSE);

	if (ch == ERR) {
		*str = '\0';
		return ERR;
	}
	*str = '\0';

	T(("wgetnstr returns \"%s\"", visbuf(oldstr)));

	return(OK);
}
