
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
**	lib_insdel.c
**
**	The routine winsdel(win, n).
**  positive n insert n lines above current line
**  negative n delete n lines starting from current line 
**
*/

#include <stdlib.h>
#include "curses.priv.h"
#include "terminfo.h"

void insdel_window(WINDOW *win, int n)
{
int	line, i;
chtype	*ptr, *temp;
chtype  **saved;
chtype	blank = ' ';

	saved = (chtype **)malloc(sizeof(chtype *) * abs(n));

	if (n < 0) {
		T(("deleting %d line starting at %d", -n, win->_cury));

		/* save overwritten lines */
		
		for (i = 0; i < -n; i++) {
			saved[i] = win->_line[win->_cury+i];
			T(("saved line %d", win->_cury+i));
		}

		/* shift n lines */
		
		for (line = win->_cury; line <= win->_regbottom+n; line++) {
			win->_line[line] = win->_line[line-n];
			T(("line %d = line %d", line, line-n));
		}

		/* restore saved lines and blank them */

		for (i = 0, line = win->_regbottom; i < -n; line++, i++) {
			T(("blanking line %d with saved %d", line, i));
			win->_line[line] = saved[i]; 
			temp = win->_line[line];
			for (ptr = temp; ptr - temp <= win->_maxx; ptr++)
				*ptr = blank;
		}
 	}

 	if (n > 0) {
 		T(("inserting %d lines starting at line %d", n, win->_cury));

		/* save overwritten lines */
		
		for (i = 0; i < n; i++) {
		 	saved[i] = win->_line[win->_cury+i];
			T(("saved line %d", win->_cury+i));
		}
		/* shift n lines */
		
		for (line = win->_regbottom; line > win->_cury; line--) {
		 	win->_line[line] = win->_line[line-n];
			T(("line %d = line %d", line, line-n));
		}

		/* restore saved lines and blank them */

		for (i = 0, line = win->_cury; i < n; line++, i++) {
		 	win->_line[line] = saved[i];
		 	temp = win->_line[line];
		 	for (ptr = temp; ptr - temp <= win->_maxx; ptr++)
				*ptr = blank;
			T(("blanked line %d", line));
		}
	}
	
	free(saved);
}

int
winsdelln(WINDOW *win, int n)
{
int	physical = FALSE;
int	ox = win->_curx, oy = win->_cury;

	T(("winsdel(%x,%d) called", win, n));

	if (n == 0)
		return OK;
	if (n < 0 && win->_cury - n >= win->_maxy)
		/* request to delete too many lines */
		/* should we truncate to an appropriate number? */
		return ERR;

	/* as an optimization, if the window's width is that of the screen
	   then use hardware insert/delete line if available */

    	if (win->_maxx == columns-1) {
    		if ((n < 0 && delete_line != NULL) || (n < 0 && parm_delete_line != NULL))
    			physical = TRUE;
    		if ((n > 0 && insert_line != NULL) || (n > 0 && parm_insert_line != NULL))
    			physical = TRUE;
    	}

	if (physical == TRUE) {
		wrefresh(win);
		T(("doing physical screen"));
		insdel_window(curscr, n);
		insdel_window(newscr, n);
	}
	insdel_window(win, n);
	T(("done with window"));

	if (physical == TRUE) {

		if (n > 0) {
			mvcur(-1, -1, win->_cury, 0);
			if (parm_insert_line) {
				putp(tparm(parm_insert_line, n));
			} else if (insert_line) {
				while (n--)
					putp(insert_line);
			}
		}

		if (n < 0) {
			mvcur(-1, -1, win->_cury, 0);
			n = abs(n);
			if (parm_delete_line) {
				putp(tparm(parm_delete_line, n));
		    	} else if (delete_line) {
		    		while (n--)
					putp(delete_line);
			}
		}
		fflush(SP->_ofp);
		mvcur(-1, -1, oy, ox);
	} else 
	    	touchline(win, win->_cury, win->_regbottom - win->_cury + 1);

    	return OK;
}


