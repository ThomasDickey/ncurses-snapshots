
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

#include <curses.priv.h>
#include <term.h>	/* cur_term */

#include <string.h>

MODULE_ID("$Id: lib_raw.c,v 1.9 1996/07/28 00:36:48 tom Exp $")

#ifdef __QNX__		/* Allows compilation under the QNX 4.2 OS */
#define ONLCR 0
#endif

#ifdef SVR4_TERMIO
#define _POSIX_SOURCE
#endif

#if HAVE_SYS_TERMIO_H
#include <sys/termio.h>	/* needed for ISC */
#endif

/* may be undefined if we're using termio.h */
#ifndef TOSTOP
#define TOSTOP 0
#endif
#ifndef IEXTEN
#define IEXTEN 0
#endif

/*
 * COOKED_INPUT defines the collection of input mode bits to be
 * cleared when entering raw mode, then re-set by noraw().
 *
 * We used to clear ISTRIP and INPCK when going to raw mode.  Keith
 * Bostic says that's wrong, because those are hardware bits that the
 * user has to get right in his/her initial environment -- he says
 * curses can't do any good by clearing these, and may do harm.  In
 * 1995's world of 8N1 connections over error-correcting modems, all
 * the parity-check stuff is pretty nearly irrelevant anyway.
 *
 * What's supposed to happen when noraw() executes has never been very
 * well-defined.  Yes, it should reset ISIG/ICANON/OPOST (historical
 * practice is for it to attempt to take the driver back to cooked
 * mode, rather going to some half-baked cbreak-like intermediate
 * level).
 *
 * We make a design choice here to turn off CR/LF translation a la BSD
 * when raw() is enabled, on the theory that a programmer requesting
 * raw() ideally wants an 8-bit data stream that's been messed with as
 * little as possible.  The man pages document this.
 *
 * We originally opted for the simplest way to handle noraw(); just set all
 * the flags we cleared.  Unfortunately, having noraw() set IGNCR
 * turned out to be too painful.  So raw() now clears the COOKED_INPUT
 * flags, but also clears (ICRNL|INLCR|IGNCR) which noraw() doesn't
 * restore.
 *
 * Unfortunately, this means noraw() may still force some COOKED_INPUT
 * flags on that the user had initially cleared via stty.  It'll all
 * come out in the wash when endwin() restores the user's original
 * input bits (we hope...)
 *
 */
#define COOKED_INPUT	(IXON|IGNBRK|BRKINT|PARMRK)

#ifdef TRACE
char *_tracebits(void)
/* describe the state of the terminal control bits exactly */
{
static char	buf[BUFSIZ];
static const	struct {unsigned int val; char *name;}

#ifdef TERMIOS
iflags[] =
    {
	{BRKINT,	"BRKINT"},
	{IGNBRK,	"IGNBRK"},
	{IGNPAR,	"IGNPAR"},
	{PARMRK,	"PARMRK"},
	{INPCK, 	"INPCK"},
	{ISTRIP,	"ISTRIP"},
	{INLCR, 	"INLCR"},
	{IGNCR, 	"IGNC"},
	{ICRNL, 	"ICRNL"},
	{IXON,  	"IXON"},
	{IXOFF, 	"IXOFF"},
	{0,		NULL}
#define ALLIN	(BRKINT|IGNBRK|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF)
    },
oflags[] =
    {
	{OPOST, 	"OPOST"},
	{0,		NULL}
#define ALLOUT	(OPOST)
    },
cflags[] =
    {
	{CLOCAL,	"CLOCAL"},
	{CREAD, 	"CREAD"},
	{CSIZE, 	"CSIZE"},
	{CSTOPB,	"CSTOPB"},
	{HUPCL, 	"HUPCL"},
	{PARENB,	"PARENB"},
	{PARODD|PARENB,	"PARODD"},	/* concession to readability */
	{0,		NULL}
#define ALLCTRL	(CLOCAL|CREAD|CSIZE|CSTOPB|HUPCL|PARENB|PARODD)
    },
lflags[] =
    {
	{ECHO,  	"ECHO"},
	{ECHOE|ECHO, 	"ECHOE"},	/* concession to readability */
	{ECHOK|ECHO, 	"ECHOK"},	/* concession to readability */
	{ECHONL,	"ECHONL"},
	{ICANON,	"ICANON"},
	{ISIG,  	"ISIG"},
	{NOFLSH,	"NOFLSH"},
#if TOSTOP != 0
	{TOSTOP,	"TOSTOP"},
#endif
#if IEXTEN != 0
	{IEXTEN,	"IEXTEN"},
#endif
	{0,		NULL}
#define ALLLOCAL	(ECHO|ECHONL|ICANON|ISIG|NOFLSH|TOSTOP|IEXTEN)
    },
    *sp;

    if (cur_term->Nttyb.c_iflag & ALLIN)
    {
	(void) strcpy(buf, "iflags: {");
	for (sp = iflags; sp->val; sp++)
	    if ((cur_term->Nttyb.c_iflag & sp->val) == sp->val)
	    {
		(void) strcat(buf, sp->name);
		(void) strcat(buf, ", ");
	    }
	if (buf[strlen(buf) - 2] == ',')
	    buf[strlen(buf) - 2] = '\0';
	(void) strcat(buf,"} ");
    }

    if (cur_term->Nttyb.c_oflag & ALLOUT)
    {
	(void) strcat(buf, "oflags: {");
	for (sp = oflags; sp->val; sp++)
	    if ((cur_term->Nttyb.c_oflag & sp->val) == sp->val)
	    {
		(void) strcat(buf, sp->name);
		(void) strcat(buf, ", ");
	    }
	if (buf[strlen(buf) - 2] == ',')
	    buf[strlen(buf) - 2] = '\0';
	(void) strcat(buf,"} ");
    }

    if (cur_term->Nttyb.c_cflag & ALLCTRL)
    {
	(void) strcat(buf, "cflags: {");
	for (sp = cflags; sp->val; sp++)
	    if ((cur_term->Nttyb.c_cflag & sp->val) == sp->val)
	    {
		(void) strcat(buf, sp->name);
		(void) strcat(buf, ", ");
	    }
	if (buf[strlen(buf) - 2] == ',')
	    buf[strlen(buf) - 2] = '\0';
	(void) strcat(buf,"} ");
    }

    if (cur_term->Nttyb.c_lflag & ALLLOCAL)
    {
	(void) strcat(buf, "lflags: {");
	for (sp = lflags; sp->val; sp++)
	    if ((cur_term->Nttyb.c_lflag & sp->val) == sp->val)
	    {
		(void) strcat(buf, sp->name);
		(void) strcat(buf, ", ");
	    }
	if (buf[strlen(buf) - 2] == ',')
	    buf[strlen(buf) - 2] = '\0';
	(void) strcat(buf,"} ");
    }

#else
    /* reference: ttcompat(4M) on SunOS 4.1 */
#ifndef EVENP
#define EVENP 0
#endif
#ifndef LCASE
#define LCASE 0
#endif
#ifndef LLITOUT
#define LLITOUT 0
#endif
#ifndef ODDP
#define ODDP 0
#endif
#ifndef TANDEM
#define TANDEM 0
#endif

cflags[] =
    {
	{CBREAK,	"CBREAK"},
	{CRMOD,		"CRMOD"},
	{ECHO,		"ECHO"},
	{EVENP,		"EVENP"},
	{LCASE,		"LCASE"},
	{LLITOUT,	"LLITOUT"},
	{ODDP,		"ODDP"},
	{RAW,		"RAW"},
	{TANDEM,	"TANDEM"},
	{XTABS,		"XTABS"},
	{0,		NULL}
#define ALLCTRL	(CBREAK|CRMOD|ECHO|EVENP|LCASE|LLITOUT|ODDP|RAW|TANDEM|XTABS)
    },
    *sp;

    if (cur_term->Nttyb.sg_flags & ALLCTRL)
    {
	(void) strcat(buf, "cflags: {");
	for (sp = cflags; sp->val; sp++)
	    if ((cur_term->Nttyb.sg_flags & sp->val) == sp->val)
	    {
		(void) strcat(buf, sp->name);
		(void) strcat(buf, ", ");
	    }
	if (buf[strlen(buf) - 2] == ',')
	    buf[strlen(buf) - 2] = '\0';
	(void) strcat(buf,"} ");
    }

#endif
    return(buf);
}

#define BEFORE(N)	if (_nc_tracing&TRACE_BITS) _tracef("%s before bits: %s", N, _tracebits())
#define AFTER(N)	if (_nc_tracing&TRACE_BITS) _tracef("%s after bits: %s", N, _tracebits())
#else
#define BEFORE(s)
#define AFTER(s)
#endif /* TRACE */

int raw(void)
{
	T(("raw() called"));

	SP->_raw = TRUE;
	SP->_cbreak = TRUE;

#ifdef TERMIOS
	BEFORE("raw");
	cur_term->Nttyb.c_lflag &= ~(ICANON|ISIG|IEXTEN);
	cur_term->Nttyb.c_iflag &= ~(COOKED_INPUT|ICRNL|INLCR|IGNCR);
	cur_term->Nttyb.c_oflag &= ~(OPOST);
	cur_term->Nttyb.c_cc[VMIN] = 1;
	cur_term->Nttyb.c_cc[VTIME] = 0;
	AFTER("raw");
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

int cbreak(void)
{
	T(("cbreak() called"));

	SP->_cbreak = TRUE;

#ifdef TERMIOS
	BEFORE("cbreak");
	cur_term->Nttyb.c_lflag &= ~ICANON;
	cur_term->Nttyb.c_lflag |= ISIG;
	cur_term->Nttyb.c_iflag &= ~ICRNL;
	cur_term->Nttyb.c_cc[VMIN] = 1;
	cur_term->Nttyb.c_cc[VTIME] = 0;
	AFTER("cbreak");
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

int echo(void)
{
	T(("echo() called"));

	SP->_echo = TRUE;

	return OK;
}


int nl(void)
{
	T(("nl() called"));

	SP->_nl = TRUE;

#ifdef TERMIOS
	BEFORE("nl");
	/* the code used to set IXON|IXOFF here, Ghod knows why... */
	cur_term->Nttyb.c_iflag |= ICRNL;
	cur_term->Nttyb.c_oflag |= OPOST|ONLCR;
	AFTER("nl");
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


int qiflush(void)
{
	T(("qiflush() called"));

	/*
	 * Note: this implementation may be wrong.  See the comment under
	 * intrflush().
	 */

#ifdef TERMIOS
	BEFORE("qiflush");
	cur_term->Nttyb.c_lflag &= ~(NOFLSH);
	AFTER("qiflush");
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	return ERR;
#endif
}


int noraw(void)
{
	T(("noraw() called"));

	SP->_raw = FALSE;
	SP->_cbreak = FALSE;

#ifdef TERMIOS
	BEFORE("noraw");
	cur_term->Nttyb.c_lflag |= ISIG|ICANON|IEXTEN;
	cur_term->Nttyb.c_iflag |= COOKED_INPUT;
	cur_term->Nttyb.c_oflag |= OPOST;
	AFTER("noraw");
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


int nocbreak(void)
{
	T(("nocbreak() called"));

	SP->_cbreak = 0;

#ifdef TERMIOS
	BEFORE("nocbreak");
	cur_term->Nttyb.c_lflag |= ICANON;
	cur_term->Nttyb.c_iflag |= ICRNL;
	AFTER("nocbreak");
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

int noecho(void)
{
	T(("noecho() called"));
	SP->_echo = FALSE;
	return OK;
}


int nonl(void)
{
	T(("nonl() called"));

	SP->_nl = FALSE;

#ifdef TERMIOS
	BEFORE("nonl");
	cur_term->Nttyb.c_iflag &= ~ICRNL;
	cur_term->Nttyb.c_oflag &= ~ONLCR;
	AFTER("nonl");
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

int noqiflush(void)
{
	T(("noqiflush() called"));

	/*
	 * Note: this implementation may be wrong.  See the comment under
	 * intrflush().
	 */

#ifdef TERMIOS
	BEFORE("noqiflush");
	cur_term->Nttyb.c_lflag |= NOFLSH;
	AFTER("noqiflush");
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
	T(("intrflush(%d) called", flag));

	/*
	 * This call does the same thing as the qiflush()/noqiflush()
	 * pair.  We know for certain that SVr3 intrflush() tweaks the
	 * NOFLSH bit; on the other hand, the match (in the SVr4 man
	 * pages) between the language describing NOFLSH in termio(7)
	 * and the language describing qiflush()/noqiflush() in
	 * curs_inopts(3x) is too exact to be coincidence.
	 */

#ifdef TERMIOS
	BEFORE("intrflush");
	if (flag)
		cur_term->Nttyb.c_lflag &= ~(NOFLSH);
	else
		cur_term->Nttyb.c_lflag |= (NOFLSH);
	AFTER("intrflush");
	if((tcsetattr(cur_term->Filedes, TCSANOW, &cur_term->Nttyb)) == -1)
		return ERR;
	else
		return OK;
#else
	return ERR;
#endif
}
