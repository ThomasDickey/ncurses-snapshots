
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
 *	lib_baudrate.c
 *
 */

#include "curses.priv.h"
#include "term.h"	/* cur_term, pad_char */

/*
 *	int
 *	baudrate()
 *
 *	Returns the current terminal's baud rate.
 *
 */

struct speed {
	speed_t s;
	int sp;
};

static struct speed const speeds[] = {
	{B0, 0},
	{B50, 50},
	{B75, 75},
	{B110, 110},
	{B134, 134},
	{B150, 150},
	{B200, 200},
	{B300, 300},
	{B600, 600},
	{B1200, 1200},
	{B1800, 1800},
	{B2400, 2400},
	{B4800, 4800},
	{B9600, 9600},
#define MAX_BAUD	B9600
#ifdef B19200
#undef MAX_BAUD
#define MAX_BAUD	B19200
	{B19200, 19200},
#else 
#ifdef EXTA
#define MAX_BAUD	EXTA
	{EXTA, 19200},
#endif
#endif
#ifdef B38400
#undef MAX_BAUD
#define MAX_BAUD	B38400
	{B38400, 38400},
#else 
#ifdef EXTB
#define MAX_BAUD	EXTB
	{EXTB, 38400},
#endif
#endif
#ifdef B57600
#undef MAX_BAUD
#define MAX_BAUD        B57600
	{B57600, 57600},
#endif
#ifdef B115200
#undef MAX_BAUD
#define MAX_BAUD        B115200
	{B115200, 115200},
#endif
};

int
baudrate(void)
{
size_t i;
int ret;

	T(("baudrate() called"));

#ifdef TERMIOS
	ret = cfgetospeed(&cur_term->Nttyb);
#else
	ret = cur_term->Nttyb.sg_ospeed;
#endif
	if(ret < 0 || ret > MAX_BAUD)
		return ERR;
	SP->_baudrate = ERR;
	for (i = 0; i < (sizeof(speeds) / sizeof(struct speed)); i++)
		if (speeds[i].s == (speed_t)ret)
		{
			SP->_baudrate = speeds[i].sp;
			break;
		}
	return(SP->_baudrate);
}

  
