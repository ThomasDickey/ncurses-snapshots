

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


/*
 *	beep.c
 *
 *	Routines beep() and flash()
 *
 */

#include "curses.priv.h"
#include "terminfo.h"

/*
 *	beep()
 *
 *	Sound the current terminal's audible bell if it has one.   If not,
 *	flash the screen if possible.
 *
 */

int beep()
{
	T(("beep() called"));

	/* should make sure that we are not in altchar mode */
	if (bell)
		return(putp(bell));
	else if (flash_screen)
		return(putp(flash_screen));
	else
		return(ERR);
}

/*
 *	flash()
 *
 *	Flash the current terminal's screen if possible.   If not,
 *	sound the audible bell if one exists.
 *
 */

int flash()
{
	T(("flash() called"));

	/* should make sure that we are not in altchar mode */
	if (flash_screen)
		return(putp(flash_screen));
	else if (bell)
		return(putp(bell));
	else
		return(ERR);
}
