
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
 * tset.c - terminal initialization utility
 *
 * This code was mostly swiped from 4.4BSD tset, with some obsolescent
 * cruft removed and substantial portions rewritten.  A Regents of the
 * University of California copyright applies to some portions of the
 * code, and is reproduced below:
 */
/*-
 * Copyright (c) 1980, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <termcap.h>
#include <fcntl.h>

#include <curses.h>	/* for bool typedef */
#include <term.h>
#include <dump_entry.h>

#ifndef CTRL
#define CTRL(x)	((x) & 0x1f)
#endif /* CTRL */

char *_nc_progname = "tset";

static struct termios mode, oldmode;

static int	terasechar;		/* new erase character */
static int	intrchar;		/* new interrupt character */
static int	isreset;		/* invoked as reset */
static int	tkillchar;		/* new kill character */
static int	tlines, tcolumns;		/* window size */

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

static void
#if __STDC__
err(const char *fmt, ...)
#else
err(fmt, va_alist)
	char *fmt;
        va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "tset: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(1);
	/* NOTREACHED */
}

static void
cat(char *file)
{
	register int fd, nr, nw;
	char buf[1024];

	if ((fd = open(file, O_RDONLY, 0)) < 0)
		err("%s: %s", file, strerror(errno));

	while ((nr = read(fd, buf, sizeof(buf))) > 0)
		if ((nw = write(STDERR_FILENO, buf, nr)) == -1)
			err("write to stderr: %s", strerror(errno));
	if (nr != 0)
		err("%s: %s", file, strerror(errno));
	(void)close(fd);
}

static int
outc(int c)
{
	return putc(c, stderr);
}

/* Prompt the user for a terminal type. */
static char *
askuser(char *dflt)
{
	static char answer[256];
	char *p;

	/* We can get recalled; if so, don't continue uselessly. */
	if (feof(stdin) || ferror(stdin)) {
		(void)fprintf(stderr, "\n");
		exit(1);
	}
	for (;;) {
		if (dflt)
			(void)fprintf(stderr, "Terminal type? [%s] ", dflt);
		else
			(void)fprintf(stderr, "Terminal type? ");
		(void)fflush(stderr);

		if (fgets(answer, sizeof(answer), stdin) == NULL) {
			if (dflt == NULL) {
				(void)fprintf(stderr, "\n");
				exit(1);
			}
			return (dflt);
		}

		if ((p = index(answer, '\n')) != 0)
			*p = '\0';
		if (answer[0])
			return (answer);
		if (dflt != NULL)
			return (dflt);
	}
}

/**************************************************************************
 *
 * Mapping logic begins here
 *
 **************************************************************************/

/* Baud rate conditionals for mapping. */
#define	GT		0x01
#define	EQ		0x02
#define	LT		0x04
#define	NOT		0x08
#define	GE		(GT | EQ)
#define	LE		(LT | EQ)

typedef struct map {
	struct map *next;	/* Linked list of maps. */
	char *porttype;		/* Port type, or "" for any. */
	char *type;		/* Terminal type to select. */
	int conditional;	/* Baud rate conditionals bitmask. */
	int speed;		/* Baud rate to compare against. */
} MAP;

static MAP *cur, *maplist;

typedef struct speeds {
	char	*string;
	int	speed;
} SPEEDS;

static SPEEDS speeds[] = {
	{ "0",		B0 },
	{ "50",		B50 },
	{ "75",		B75 },
	{ "110",	B110 },
	{ "134",	B134 },
	{ "134.5",	B134 },
	{ "150",	B150 },
	{ "200",	B200 },
	{ "300",	B300 },
	{ "600",	B600 },
	{ "1200",	B1200 },
	{ "1800",	B1800 },
	{ "2400",	B2400 },
	{ "4800",	B4800 },
	{ "9600",	B9600 },
	{ "19200",	B19200 },
	{ "38400",	B38400 },
	{ "19200",	B19200 },
	{ "38400",	B38400 },
	{ (char *)0,    0 }
};

static int
tbaudrate(char *rate)
{
	SPEEDS *sp;
	int found = FALSE;

	/* The baudrate number can be preceded by a 'B', which is ignored. */
	if (*rate == 'B')
		++rate;

	for (sp = speeds; sp->string; ++sp) {
		if (!strcasecmp(rate, sp->string)) {
			found = TRUE;
			break;
		}
	}
	if (!found)
		err("unknown baud rate %s", rate);
	return (sp->speed);
}

/*
 * Syntax for -m:
 * [port-type][test baudrate]:terminal-type
 * The baud rate tests are: >, <, @, =, !
 */
static void
add_mapping(char *port, char *arg)
{
	MAP *mapp;
	char *copy, *p, *termp;

	copy = strdup(arg);
	mapp = malloc((u_int)sizeof(MAP));
	if (copy == NULL || mapp == NULL)
		err("%s", strerror(errno));
	mapp->next = NULL;
	if (maplist == NULL)
		cur = maplist = mapp;
	else {
		cur->next = mapp;
		cur =  mapp;
	}

	mapp->porttype = arg;
	mapp->conditional = 0;

	arg = strpbrk(arg, "><@=!:");

	if (arg == NULL) {			/* [?]term */
		mapp->type = mapp->porttype;
		mapp->porttype = NULL;
		goto done;
	}

	if (arg == mapp->porttype)		/* [><@=! baud]:term */
		termp = mapp->porttype = NULL;
	else
		termp = arg;

	for (;; ++arg)				/* Optional conditionals. */
		switch(*arg) {
		case '<':
			if (mapp->conditional & GT)
				goto badmopt;
			mapp->conditional |= LT;
			break;
		case '>':
			if (mapp->conditional & LT)
				goto badmopt;
			mapp->conditional |= GT;
			break;
		case '@':
		case '=':			/* Not documented. */
			mapp->conditional |= EQ;
			break;
		case '!':
			mapp->conditional |= NOT;
			break;
		default:
			goto next;
		}

next:	if (*arg == ':') {
		if (mapp->conditional)
			goto badmopt;
		++arg;
	} else {				/* Optional baudrate. */
		arg = index(p = arg, ':');
		if (arg == NULL)
			goto badmopt;
		*arg++ = '\0';
		mapp->speed = tbaudrate(p);
	}

	if (arg == (char *)NULL)		/* Non-optional type. */
		goto badmopt;

	mapp->type = arg;

	/* Terminate porttype, if specified. */
	if (termp != NULL)
		*termp = '\0';

	/* If a NOT conditional, reverse the test. */
	if (mapp->conditional & NOT)
		mapp->conditional = ~mapp->conditional & (EQ | GT | LT);

	/* If user specified a port with an option flag, set it. */
done:	if (port) {
		if (mapp->porttype)
badmopt:		err("illegal -m option format: %s", copy);
		mapp->porttype = port;
	}

#ifdef MAPDEBUG
	(void)printf("port: %s\n", mapp->porttype ? mapp->porttype : "ANY");
	(void)printf("type: %s\n", mapp->type);
	(void)printf("conditional: ");
	p = "";
	if (mapp->conditional & GT) {
		(void)printf("GT");
		p = "/";
	}
	if (mapp->conditional & EQ) {
		(void)printf("%sEQ", p);
		p = "/";
	}
	if (mapp->conditional & LT)
		(void)printf("%sLT", p);
	(void)printf("\nspeed: %d\n", mapp->speed);
#endif
}

/*
 * Return the type of terminal to use for a port of type 'type', as specified
 * by the first applicable mapping in 'map'.  If no mappings apply, return
 * 'type'.
 */
static char *
mapped(char *type)
{
	MAP *mapp;
	int match;

	for (mapp = maplist; mapp; mapp = mapp->next)
		if (mapp->porttype == NULL || !strcmp(mapp->porttype, type)) {
			switch (mapp->conditional) {
			case 0:			/* No test specified. */
				match = TRUE;
				break;
			case EQ:
				match = (ospeed == mapp->speed);
				break;
			case GE:
				match = (ospeed >= mapp->speed);
				break;
			case GT:
				match = (ospeed > mapp->speed);
				break;
			case LE:
				match = (ospeed <= mapp->speed);
				break;
			case LT:
				match = (ospeed < mapp->speed);
				break;
			default:
				match = FALSE;
			}
			if (match)
				return (mapp->type);
		}
	/* No match found; return given type. */
	return (type);
}

/**************************************************************************
 *
 * Entry fetching
 *
 **************************************************************************/

/*
 * Figure out what kind of terminal we're dealing with, and then read in
 * its termcap entry.
 */
static char *
get_termcap_entry(char *userarg)
{
	int rval, errret;
	char *p, *ttype;
#ifdef HAVE_GETTTYNAM
	struct ttyent *t;
	char *ttypath;
#endif

	if (userarg) {
		ttype = userarg;
		goto found;
	}

	/* Try the environment. */
	if ((ttype = getenv("TERM")) != 0)
		goto map;

#ifdef HAVE_GETTTYNAM
	/*
	 * We have the 4.3BSD library call getttynam(3); that means
	 * there's an /etc/ttys to look up device-to-type mappings in.
	 * Try ttyname(3); check for dialup or other mapping.
	 */
	if (ttypath = ttyname(STDERR_FILENO)) {
		if (p = rindex(ttypath, '/'))
			++p;
		else
			p = ttypath;
		if ((t = getttynam(p))) {
			ttype = t->ty_type;
			goto map;
		}
	}
#endif /* BSD */

	/* If still undefined, use "unknown". */
	ttype = "unknown";

map:	ttype = mapped(ttype);

	/*
	 * If not a path, remove TERMCAP from the environment so we get a
	 * real entry from /etc/termcap.  This prevents us from being fooled
	 * by out of date stuff in the environment.
	 */
found:	if ((p = getenv("TERMCAP")) != NULL && *p != '/')
		unsetenv("TERMCAP");

	/*
	 * ttype now contains a pointer to the type of the terminal.
	 * If the first character is '?', ask the user.
	 */
	if (ttype[0] == '?')
		if (ttype[1] != '\0')
			ttype = askuser(ttype + 1);
		else
			ttype = askuser(NULL);

	/* Find the terminfo entry.  If it doesn't exist, ask the user. */
	while ((rval = setupterm(ttype, STDOUT_FILENO, &errret)) == ERR) {
		(void)fprintf(stderr,
		    "tset: can't initialize terminal type %s (error %d)\n",
			      ttype, errret);
		ttype = askuser(NULL);
	}
	return (ttype);
}

/**************************************************************************
 *
 * Mode-setting logic
 *
 **************************************************************************/

/* control-character defaults */
#define CEOF	CTRL('D')
#define CERASE	CTRL('H')
#define CINTR	127		/* ^? */
#define CKILL	CTRL('U')
#define CQUIT	CTRL('\\')
#define CSTART	CTRL('Q')
#define CSTOP	CTRL('S')
#define CSUSP	CTRL('Z')

#define	CHK(val, dft)	(val <= 0 ? dft : val)

static int	set_tabs __P((void));

/*
 * Reset the terminal mode bits to a sensible state.  Very useful after
 * a child program dies in raw mode.
 */
static void
reset_mode(void)
{
	tcgetattr(STDERR_FILENO, &mode);

#if defined(VDISCARD) && defined(CDISCARD)
	mode.c_cc[VDISCARD] = CHK(mode.c_cc[VDISCARD], CDISCARD);
#endif
	mode.c_cc[VEOF] = CHK(mode.c_cc[VEOF], CEOF);
	mode.c_cc[VERASE] = CHK(mode.c_cc[VERASE], CERASE);
#if defined(VFLUSH) && defined(CFLUSH)
	mode.c_cc[VFLUSH] = CHK(mode.c_cc[VFLUSH], CFLUSH);
#endif
	mode.c_cc[VINTR] = CHK(mode.c_cc[VINTR], CINTR);
	mode.c_cc[VKILL] = CHK(mode.c_cc[VKILL], CKILL);
#if defined(VLNEXT) && defined(CLNEXT)
	mode.c_cc[VLNEXT] = CHK(mode.c_cc[VLNEXT], CLNEXT);
#endif
	mode.c_cc[VQUIT] = CHK(mode.c_cc[VQUIT], CQUIT);
#if defined(VREPRINT) && defined(CRPRNT)
	mode.c_cc[VREPRINT] = CHK(mode.c_cc[VREPRINT], CRPRNT);
#endif
	mode.c_cc[VSTART] = CHK(mode.c_cc[VSTART], CSTART);
	mode.c_cc[VSTOP] = CHK(mode.c_cc[VSTOP], CSTOP);
	mode.c_cc[VSUSP] = CHK(mode.c_cc[VSUSP], CSUSP);
#if defined(VWERASE) && defined(CWERASE)
	mode.c_cc[VWERASE] = CHK(mode.c_cc[VWERASE], CWERASE);
#endif

	mode.c_iflag &= ~(IGNBRK | PARMRK | INPCK | ISTRIP | INLCR | IGNCR
#ifdef IUCLC
			  | IUCLC
#endif
#ifdef IXANY
			  | IXANY
#endif
			  | IXOFF);

	mode.c_iflag |= (BRKINT | IGNPAR | ICRNL | IXON
#ifdef IMAXBEL
			 | IMAXBEL
#endif
			 );

	mode.c_oflag &= ~(0
#ifdef OLCUC
			  | OLCUC
#endif
#ifdef OCRNL
			  | OCRNL
#endif
#ifdef ONOCR
			  | ONOCR
#endif
#ifdef ONLRET
			  | ONLRET
#endif
#ifdef OFILL
			  | OFILL
#endif
#ifdef OFDEL
			  | OFDEL
#endif
#ifdef NLDLY
			  | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY
#endif
			  );

	mode.c_oflag |= (OPOST
#ifdef ONLCR
			 | ONLCR
#endif
			 );

	mode.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD | CLOCAL);
	mode.c_cflag |= (CS8 | CREAD);
	mode.c_lflag &= ~(ECHONL | NOFLSH | TOSTOP
#ifdef ECHOPTR
			  | ECHOPRT
#endif
#ifdef XCASE
			  | XCASE
#endif
			  );

	mode.c_lflag |= (ISIG | ICANON | ECHO | ECHOE | ECHOK
#ifdef ECHOCTL
			 | ECHOCTL
#endif
#ifdef ECHOKE
			 | ECHOKE
#endif
 			 );

	tcsetattr(STDERR_FILENO, TCSADRAIN, &mode);
}

/*
 * Determine the erase, interrupt, and kill characters from the termcap
 * entry and command line and update their values in 'mode'.
 */
static void
set_control_chars(void)
{
#ifdef __OBSOLETE__
	/*
	 * 4.4BSD logic for setting erasechar, left here in case there is some
	 * necessary subtletly missed in the production code below that really
	 * needs to be added someday (in particular, I don't understand what
	 * the second if-statement involving the os flag is doing, and it makes
	 * my head hurt when I try and follow out all the combinations).
	 */
	char *bp, *p, bs_char, buf[1024];

	bp = buf;
	p = tgetstr("kb", &bp);
	if (p == NULL || p[1] != '\0')
		p = tgetstr("bc", &bp);
	if (p != NULL && p[1] == '\0')
		bs_char = p[0];
	else if (tgetflag("bs"))
		bs_char = CTRL('h');
	else
		bs_char = 0;

	if (terasechar==0 && !tgetflag("os") && mode.c_cc[VERASE] != CERASE) {
		if (tgetflag("bs") || bs_char != 0)
			terasechar = -1;
	}
	if (terasechar < 0)
		terasechar = (bs_char != 0) ? bs_char : CTRL('h');
#else
	if (key_backspace != (char *)NULL)
		terasechar = key_backspace[0];
#endif /* __OBSOLETE__ */

	if (mode.c_cc[VERASE] == 0 || terasechar != 0)
		mode.c_cc[VERASE] = terasechar ? terasechar : CERASE;

	if (mode.c_cc[VINTR] == 0 || intrchar != 0)
		 mode.c_cc[VINTR] = intrchar ? intrchar : CINTR;

	if (mode.c_cc[VKILL] == 0 || tkillchar != 0)
		mode.c_cc[VKILL] = tkillchar ? tkillchar : CKILL;
}

/*
 * Set up various conversions in 'mode', including parity, tabs, returns,
 * echo, and case, according to the termcap entry.  If the program we're
 * running was named with a leading upper-case character, map external
 * uppercase to internal lowercase.
 */
static void
set_conversions(void)
{
#ifdef __OBSOLETE__
	/*
	 * Conversion logic for some *really* ancient terminal glitches,
	 * not supported in terminfo.  Left here for succeeding generations
	 * to marvel at.
	 */
	if (tgetflag("UC")) {
#ifdef IUCLC
		mode.c_iflag |= IUCLC;
		mode.c_oflag |= OLCUC;
#endif
	} else if (tgetflag("LC")) {
#ifdef IUCLC
		mode.c_iflag &= ~IUCLC;
		mode.c_oflag &= ~OLCUC;
#endif
	}
	mode.c_iflag &= ~(PARMRK | INPCK);
	mode.c_lflag |= ICANON;
	if (tgetflag("EP")) {
		mode.c_cflag |= PARENB;
		mode.c_cflag &= ~PARODD;
	}
	if (tgetflag("OP")) {
		mode.c_cflag |= PARENB;
		mode.c_cflag |= PARODD;
	}
#endif /* __OBSOLETE__ */

#ifdef ONLCR
	mode.c_oflag |= ONLCR;
#endif
	mode.c_iflag |= ICRNL;
	mode.c_lflag |= ECHO;
#ifdef OXTABS
	mode.c_oflag |= OXTABS;
#endif /* OXTABS */

	/* test used to be tgetflag("NL") */
	if (newline != (char *)NULL && newline[0] == '\n' && !newline[1]) {
		/* Newline, not linefeed. */
#ifdef ONLCR
		mode.c_oflag &= ~ONLCR;
#endif
		mode.c_iflag &= ~ICRNL;
	}
#ifdef __OBSOLETE__
	if (tgetflag("HD"))			/* Half duplex. */
		mode.c_lflag &= ~ECHO;
#endif /* __OBSOLETE__ */
#ifdef OXTABS
	/* test used to be tgetflag("pt") */
	if (has_hardware_tabs)			/* Print tabs. */
		mode.c_oflag &= ~OXTABS;
#endif /* OXTABS */
	mode.c_lflag |= (ECHOE | ECHOK);
}

/* Output startup string. */
static void
set_init(void)
{
	char	*p;
	int settle;

#ifdef __OBSOLETE__
	if (pad_char != (char *)NULL)		/* Get/set pad character. */
		PC = pad_char[0];
#endif /* OBSOLETE */

#ifdef TAB3
	if (oldmode.c_oflag & (TAB3 | ONLCR | OCRNL | ONLRET)) {
		oldmode.c_oflag &= (TAB3 | ONLCR | OCRNL | ONLRET);
		tcsetattr(STDERR_FILENO, TCSADRAIN, &oldmode);
	}
#endif
	settle = set_tabs();

	if (isreset) {
		if ((p = reset_1string) || (p = reset_2string)) {
			tputs(p, 0, outc);
			settle = 1;
		}
		if ((p = reset_file) || (p = init_file)) {
			cat(p);
			settle = 1;
		}
	}

	if (settle) {
		(void)putc('\r', stderr);
		(void)fflush(stderr);
		(void)sleep(1);			/* Settle the terminal. */
	}
}

/*
 * Set the hardware tabs on the terminal, using the ct (clear all tabs),
 * st (set one tab) and ch (horizontal cursor addressing) capabilities.
 * This is done before if and is, so they can patch in case we blow this.
 * Return nonzero if we set any tab stops, zero if not.
 */
static int
set_tabs()
{
	int c;
	char *set_pos, *tg_out = 0;

	if (set_tab && clear_all_tabs) {
		(void)putc('\r', stderr);	/* Force to left margin. */
		tputs(clear_all_tabs, 0, outc);
	}

	set_pos = column_address ? NULL : cursor_address;

	if (set_tab) {
		for (c = 8; c < tcolumns; c += 8) {
			/*
			 * Get to the right column.
			 */
			if (column_address)
				tg_out = tgoto(column_address, 0, c);
			if (tg_out == (char *)NULL && set_pos)
				tg_out = tgoto(set_pos, c, tlines - 1);
			if (tg_out != (char *)NULL)
				tputs(tg_out, 1, outc);
			else
				(void)fprintf(stderr, "%s", "        ");
			/* Set the tab. */
			tputs(set_tab, 0, outc);
		}
		putc('\r', stderr);
		return (1);
	}
	return (0);
}

/**************************************************************************
 *
 * Main sequence
 *
 **************************************************************************/

/*
 * Tell the user if a control key has been changed from the default value.
 */
static void
report(char *name, int which, u_int def)
{
	u_int old, new;
	char *p;

	new = mode.c_cc[which];
	old = oldmode.c_cc[which];

	if (old == new && old == def)
		return;

	(void)fprintf(stderr, "%s %s ", name, old == new ? "is" : "set to");

	if ((p = key_backspace) && new == p[0] && p[1] == '\0')
		(void)fprintf(stderr, "backspace.\n");
	else if (new == 0177)
		(void)fprintf(stderr, "delete.\n");
	else if (new < 040) {
		new ^= 0100;
		(void)fprintf(stderr, "control-%c (^%c).\n", new, new);
	} else
		(void)fprintf(stderr, "%c.\n", new);
}

/*
 * Convert the obsolete argument form into something that getopt can handle.
 * This means that -e, -i and -k get default arguments supplied for them.
 */
static void
obsolete(char **argv)
{
	for (; *argv; ++argv) {
		char *parm = argv[0];
		if ((parm[0] != '-')
		 || (argv[1] && argv[1][0] != '-')
		 || (parm[1] != 'e' && parm[1] != 'i' && parm[1] != 'k')
		 || (parm[2] != '\0'))
			continue;
		switch(argv[0][1]) {
		case 'e':
			argv[0] = "-e^H";
			break;
		case 'i':
			argv[0] = "-i^C";
			break;
		case 'k':
			argv[0] = "-k^U";
			break;
		}
	}
}

static void
usage(void)
{
	(void)fprintf(stderr,
"usage: tset [-IQrs] [-] [-e ch] [-i ch] [-k ch] [-m mapping] [terminal]\n");
	exit(1);
}

int
main(int argc, char **argv)
{
#ifdef TIOCGWINSZ
	struct winsize win;
#endif
	int ch, noinit, noset, quiet, Sflag, sflag, showterm;
	char *p, *ttype;

	if (tcgetattr(STDERR_FILENO, &mode) < 0)
		err("standard error: %s", strerror(errno));

	oldmode = mode;
	ospeed = cfgetospeed(&mode);

	if ((p = strrchr(*argv, '/')) != 0)
		++p;
	else
		p = *argv;
	if (!strcasecmp(p, "reset")) {
		isreset = 1;
		reset_mode();
	}

	obsolete(argv);
	noinit = noset = quiet = Sflag = sflag = showterm = 0;
	while ((ch = getopt(argc, argv, "-a:d:e:Ii:k:m:np:QSrs")) != EOF) {
		switch (ch) {
		case '-':		/* display term only */
			noset = 1;
			break;
		case 'a':		/* OBSOLETE: map identifier to type */
			add_mapping("arpanet", optarg);
			break;
		case 'd':		/* OBSOLETE: map identifier to type */
			add_mapping("dialup", optarg);
			break;
		case 'e':		/* erase character */
			terasechar = optarg[0] == '^' && optarg[1] != '\0' ?
			    optarg[1] == '?' ? '\177' : CTRL(optarg[1]) :
			    optarg[0];
			break;
		case 'I':		/* no initialization strings */
			noinit = 1;
			break;
		case 'i':		/* interrupt character */
			intrchar = optarg[0] == '^' && optarg[1] != '\0' ?
			    optarg[1] == '?' ? '\177' : CTRL(optarg[1]) :
			    optarg[0];
			break;
		case 'k':		/* kill character */
			tkillchar = optarg[0] == '^' && optarg[1] != '\0' ?
			    optarg[1] == '?' ? '\177' : CTRL(optarg[1]) :
			    optarg[0];
			break;
		case 'm':		/* map identifier to type */
			add_mapping(NULL, optarg);
			break;
		case 'n':		/* OBSOLETE: set new tty driver */
			break;
		case 'p':		/* OBSOLETE: map identifier to type */
			add_mapping("plugboard", optarg);
			break;
		case 'Q':		/* don't output control key settings */
			quiet = 1;
			break;
		case 'S':		/* OBSOLETE: output TERM & TERMCAP */
			Sflag = 1;
			break;
		case 'r':		/* display term on stderr */
			showterm = 1;
			break;
		case 's':		/* output TERM set command */
			sflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 1)
		usage();

	ttype = get_termcap_entry(*argv);

	if (!noset) {
		tcolumns = columns;
		tlines = lines;

#ifdef TIOCGWINSZ
		/* Set window size */
		(void)ioctl(STDERR_FILENO, TIOCGWINSZ, &win);
		if (win.ws_row == 0 && win.ws_col == 0 &&
		    tlines > 0 && tcolumns > 0) {
			win.ws_row = tlines;
			win.ws_col = tcolumns;
			(void)ioctl(STDERR_FILENO, TIOCSWINSZ, &win);
		}
#endif
		set_control_chars();
		set_conversions();

		if (!noinit)
			set_init();

		/* Set the modes if they've changed. */
		if (memcmp(&mode, &oldmode, sizeof(mode)))
			tcsetattr(STDERR_FILENO, TCSADRAIN, &mode);
	}

	/* Get the terminal name from the entry. */
	ttype = canonical_name(cur_term->type.term_names, (char *)NULL);

	if (noset)
		(void)printf("%s\n", ttype);
	else {
		if (showterm)
			(void)fprintf(stderr, "Terminal type is %s.\n", ttype);
		/*
		 * If erase, kill and interrupt characters could have been
		 * modified and not -Q, display the changes.
		 */
		if (!quiet) {
			report("Erase", VERASE, CERASE);
			report("Kill", VKILL, CINTR);
			report("Interrupt", VINTR, CKILL);
		}
	}

	if (Sflag)
		err("The -S option is not supported under terminfo.");

	if (sflag) {
		/*
		 * Figure out what shell we're using.  A hack, we look for an
		 * environmental variable SHELL ending in "csh".
		 */
		if ((p = getenv("SHELL")) && !strcmp(p + strlen(p) - 3, "csh"))
			p = "set noglob;\nsetenv TERM %s;\nunset noglob;\n";
		else
			p = "TERM=%s;\n";
		(void) printf(p, ttype);
	}

	exit(0);
}

/* tset.c ends here */
