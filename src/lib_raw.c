

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
 *	raw.c
 *
 *	Routines:
 *		raw()
 *		echo()
 *		nl()
 *		cbreak()
 *		noraw()
 *		noecho()
 *		nonl()
 *		nocbreak()
 *		qiflush()
 *		noqiflush()
 *		intrflush()
 *
 */

#include "curses.priv.h"
#include "terminfo.h"

int raw()
{
	T(("raw() called"));

	SP->_raw = TRUE;
	SP->_cbreak = TRUE;

#ifdef TERMIOS
	cur_term->Nttyb.c_lflag &= ~(ICANON|ISIG);
	cur_term->Nttyb.c_iflag &= ~(INPCK|ISTRIP|IXON);
	cur_term->Nttyb.c_oflag &= ~(OPOST);
	cur_term->Nttyb.c_cc[VMIN] = 1;
	cur_term->Nttyb.c_cc[VTIME] = 0;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags |= RAW;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}

int cbreak()
{
	T(("cbreak() called"));

	SP->_cbreak = TRUE;

#ifdef TERMIOS
	cur_term->Nttyb.c_lflag &= ~ICANON; 
	cur_term->Nttyb.c_lflag |= ISIG;
	cur_term->Nttyb.c_cc[VMIN] = 1;
	cur_term->Nttyb.c_cc[VTIME] = 0;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags |= CBREAK;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}

int echo()
{
	T(("echo() called"));

	SP->_echo = TRUE;
    
#ifdef TERMIOS
	cur_term->Nttyb.c_lflag |= ECHO;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags |= ECHO;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}


int nl()
{
	T(("nl() called"));

	SP->_nl = TRUE;

#ifdef TERMIOS
	cur_term->Nttyb.c_iflag |= IXON|ICRNL|IXOFF;
	cur_term->Nttyb.c_oflag |= OPOST|ONLCR;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags |= CRMOD;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}


int qiflush()
{
	T(("qiflush() called"));

	/*
	 * Note: this implementation may be wrong.  See the comment under
	 * intrflush().
	 */

#ifdef TERMIOS
	cur_term->Nttyb.c_lflag &= ~(NOFLSH);
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	return ERR;
#endif
}


int noraw()
{
	T(("noraw() called"));

	SP->_raw = FALSE;
	SP->_cbreak = FALSE;

#ifdef TERMIOS
	cur_term->Nttyb.c_lflag |= ISIG|ICANON;
	cur_term->Nttyb.c_iflag |= IXON;
	cur_term->Nttyb.c_oflag |= OPOST;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags &= ~(RAW|CBREAK);
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif

}


int nocbreak()
{
	T(("nocbreak() called"));

	SP->_cbreak = 0;
	
#ifdef TERMIOS
	cur_term->Nttyb.c_lflag |= ICANON;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else 
	cur_term->Nttyb.sg_flags &= ~CBREAK;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}

int noecho()
{
	T(("noecho() called"));

	SP->_echo = FALSE;
	
#ifdef TERMIOS
	cur_term->Nttyb.c_lflag &= ~(ECHO);
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags &= ~ECHO;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}


int nonl()
{
	T(("nonl() called"));

	SP->_nl = FALSE;
	
#ifdef TERMIOS
	cur_term->Nttyb.c_iflag &= ~ICRNL;
	cur_term->Nttyb.c_oflag &= ~ONLCR;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	cur_term->Nttyb.sg_flags &= ~CRMOD;
	stty(cur_term->Filedes, &cur_term->Nttyb);
	return OK;
#endif
}

int noqiflush()
{
	T(("noqiflush() called"));

	/*
	 * Note: this implementation may be wrong.  See the comment under
	 * intrflush().
	 */

#ifdef TERMIOS
	cur_term->Nttyb.c_lflag |= NOFLSH;
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	return ERR;
#endif
}

int intrflush(WINDOW *win, bool flag)
{
	T(("intrflush() called"));

	/*
	 * This call does the same thing as the qiflush()/noqiflush()
	 * pair.  We know for certain that SVr3 intrflush() tweaks the
	 * NOFLSH bit; on the other hand, the match (in the SVr4 man
	 * pages) between the language describing NOFLSH in termio(7)
	 * and the language describing qiflush()/noqiflush() in
	 * curs_inopts(3x) is too exact to be coincidence.
	 */

#ifdef TERMIOS
	if (flag)
		cur_term->Nttyb.c_lflag &= ~(NOFLSH);
	else
		cur_term->Nttyb.c_lflag |= (NOFLSH);
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	return ERR;
#endif
}

