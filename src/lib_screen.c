

/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
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


#include <stdio.h>
#include "curses.priv.h"

WINDOW *getwin(FILE *filep)
{
	WINDOW	try, *nwin;
	int	n;

	(void) fread(&try, sizeof(WINDOW), 1, filep);
	if (ferror(filep))
		return (WINDOW *)NULL;

	if ((nwin = newwin(try._maxy+1, try._maxx+1, 0, 0)) == (WINDOW *)NULL)
		return (WINDOW *)NULL;

	/*
	 * We deliberately do not restore the _parx, _pary, or _parent
	 * fields, because the window hierarchy within which they
	 * made sense is probably gone.
	 */
	nwin->_curx       = try._curx;
	nwin->_cury       = try._cury;
	nwin->_maxy       = try._maxy;
	nwin->_maxx       = try._maxx;       
	nwin->_begy       = try._begy;
	nwin->_begx       = try._begx;
	nwin->_flags      = try._flags;

	nwin->_attrs      = try._attrs;
	nwin->_bkgd	  = try._bkgd; 

	nwin->_clear      = try._clear;
	nwin->_scroll     = try._scroll;
	nwin->_leave      = try._leave;
	nwin->_use_keypad = try._use_keypad;
	nwin->_delay   	  = try._delay;
	nwin->_immed	  = try._immed;
	nwin->_sync	  = try._sync;

	nwin->_regtop     = try._regtop;
	nwin->_regbottom  = try._regbottom;

	for (n = 0; n < nwin->_maxy + 1; n++)
	{
		(void) fread(nwin->_line[n].text,
			      sizeof(chtype), nwin->_maxx + 1, filep);
		if (ferror(filep))
		{
			delwin(nwin);
			return((WINDOW *)NULL);
		}
	}
	touchwin(nwin);

	return nwin;
}

int putwin(WINDOW *win, FILE *filep)
{
	int	n;

	(void) fwrite(win, sizeof(WINDOW), 1, filep);
	if (ferror(filep))
		return ERR;

	for (n = 0; n < win->_maxy + 1; n++)
	{
		(void) fwrite(win->_line[n].text,
			      sizeof(chtype), win->_maxx + 1, filep);
		if (ferror(filep))
			return(ERR);
	}

	return(OK);
}

int scr_restore(char *file)
{
	FILE	*fp;

	if ((fp = fopen(file, "r")) == (FILE *)NULL)
	    return ERR;
	else
	{
	    delwin(newscr);
	    newscr = getwin(fp);
	    (void) fclose(fp);
	    return OK;
	}
	return ERR;
}

int scr_dump(char *file)
{
	FILE	*fp;

	if ((fp = fopen(file, "w")) == (FILE *)NULL)
	    return ERR;
	else
	{
	    (void) putwin(newscr, fp);
	    (void) fclose(fp);
	    return OK;
	}
}

int scr_init(char *file)
{
	FILE	*fp;

	if ((fp = fopen(file, "r")) == (FILE *)NULL)
	    return ERR;
	else
	{
	    delwin(curscr);
	    curscr = getwin(fp);
	    (void) fclose(fp);
	    return OK;
	}

	return ERR;
}

int scr_set(char *file)
{
    if (scr_init(file) == ERR)
	return(ERR);
    else
    {
	delwin(newscr);
	newscr = dupwin(curscr);
	return(OK);
    }
}


