/****************************************************************************
 * Copyright 2019-2023,2024 Thomas E. Dickey                                *
 * Copyright 2016,2017 Free Software Foundation, Inc.                       *
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

#include <reset_cmd.h>
#include <tty_settings.h>

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

#if HAVE_SIZECHANGE
# if !defined(sun) || !TERMIOS
#  if HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#  endif
# endif
#endif

#if NEED_PTEM_H
/* they neglected to define struct winsize in termios.h -- it is only
   in termio.h	*/
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

MODULE_ID("$Id: reset_cmd.c,v 1.38 2024/12/07 22:12:53 tom Exp $")

/*
 * SCO defines TIOCGSIZE and the corresponding struct.  Other systems (SunOS,
 * Solaris, IRIX) define TIOCGWINSZ and struct winsize.
 */
#ifdef TIOCGSIZE
# define IOCTL_GET_WINSIZE TIOCGSIZE
# define IOCTL_SET_WINSIZE TIOCSSIZE
# define STRUCT_WINSIZE struct ttysize
# define WINSIZE_ROWS(n) n.ts_lines
# define WINSIZE_COLS(n) n.ts_cols
#else
# ifdef TIOCGWINSZ
#  define IOCTL_GET_WINSIZE TIOCGWINSZ
#  define IOCTL_SET_WINSIZE TIOCSWINSZ
#  define STRUCT_WINSIZE struct winsize
#  define WINSIZE_ROWS(n) n.ws_row
#  define WINSIZE_COLS(n) n.ws_col
# endif
#endif

#define set_flags(target, mask)    target |= mask
#define clear_flags(target, mask)  target &= ~((unsigned)(mask))

static FILE *my_file;

static bool use_reset = FALSE;	/* invoked as reset */
static bool use_init = FALSE;	/* invoked as init */

static GCC_NORETURN void
failed(const char *msg)
{
    int code = errno;

    (void) fprintf(stderr, "%s: %s: %s\n", _nc_progname, msg, strerror(code));
    restore_tty_settings();
    (void) fprintf(my_file, "\n");
    fflush(my_file);
    ExitProgram(ErrSystem(code));
    /* NOTREACHED */
}

static bool
cat_file(char *file)
{
    FILE *fp;
    size_t nr;
    char buf[BUFSIZ];
    bool sent = FALSE;

    if (file != NULL) {
	if ((fp = safe_fopen(file, "r")) == NULL)
	    failed(file);

	while ((nr = fread(buf, sizeof(char), sizeof(buf), fp)) != 0) {
	    if (fwrite(buf, sizeof(char), nr, my_file) != nr) {
		failed(file);
	    }
	    sent = TRUE;
	}
	fclose(fp);
    }
    return sent;
}

static int
out_char(int c)
{
    return putc(c, my_file);
}

/**************************************************************************
 * Mode-setting logic
 **************************************************************************/

/* some BSD systems have these built in, some systems are missing
 * one or more definitions. The safest solution is to override unless the
 * commonly-altered ones are defined.
 */
#if !(defined(CERASE) && defined(CINTR) && defined(CKILL) && defined(CQUIT))
#undef CEOF
#undef CERASE
#undef CINTR
#undef CKILL
#undef CLNEXT
#undef CRPRNT
#undef CQUIT
#undef CSTART
#undef CSTOP
#undef CSUSP
#endif

/* control-character defaults */
#ifndef CEOF
#define CEOF	CTRL('D')
#endif
#ifndef CERASE
#define CERASE	CTRL('H')
#endif
#ifndef CINTR
#define CINTR	127		/* ^? */
#endif
#ifndef CKILL
#define CKILL	CTRL('U')
#endif
#ifndef CLNEXT
#define CLNEXT  CTRL('v')
#endif
#ifndef CRPRNT
#define CRPRNT  CTRL('r')
#endif
#ifndef CQUIT
#define CQUIT	CTRL('\\')
#endif
#ifndef CSTART
#define CSTART	CTRL('Q')
#endif
#ifndef CSTOP
#define CSTOP	CTRL('S')
#endif
#ifndef CSUSP
#define CSUSP	CTRL('Z')
#endif

#if defined(_POSIX_VDISABLE)
#define DISABLED(val)   (((_POSIX_VDISABLE != -1) \
		       && ((val) == _POSIX_VDISABLE)) \
		      || ((val) <= 0))
#else
#define DISABLED(val)   ((int)(val) <= 0)
#endif

#define CHK(val, dft)   (unsigned char) (DISABLED(val) ? dft : val)

#define reset_char(item, value) \
    tty_settings->c_cc[item] = CHK(tty_settings->c_cc[item], value)

/*
 * Simplify ifdefs
 */
#ifndef BSDLY
#define BSDLY 0
#endif
#ifndef CRDLY
#define CRDLY 0
#endif
#ifndef ECHOCTL
#define ECHOCTL 0
#endif
#ifndef ECHOKE
#define ECHOKE 0
#endif
#ifndef ECHOPRT
#define ECHOPRT 0
#endif
#ifndef FFDLY
#define FFDLY 0
#endif
#ifndef IMAXBEL
#define IMAXBEL 0
#endif
#ifndef IUCLC
#define IUCLC 0
#endif
#ifndef IXANY
#define IXANY 0
#endif
#ifndef NLDLY
#define NLDLY 0
#endif
#ifndef OCRNL
#define OCRNL 0
#endif
#ifndef OFDEL
#define OFDEL 0
#endif
#ifndef OFILL
#define OFILL 0
#endif
#ifndef OLCUC
#define OLCUC 0
#endif
#ifndef ONLCR
#define ONLCR 0
#endif
#ifndef ONLRET
#define ONLRET 0
#endif
#ifndef ONOCR
#define ONOCR 0
#endif
#ifndef OXTABS
#define OXTABS 0
#endif
#ifndef TAB3
#define TAB3 0
#endif
#ifndef TABDLY
#define TABDLY 0
#endif
#ifndef TOSTOP
#define TOSTOP 0
#endif
#ifndef VTDLY
#define VTDLY 0
#endif
#ifndef XCASE
#define XCASE 0
#endif

/*
 * Reset the terminal mode bits to a sensible state.  Very useful after
 * a child program dies in raw mode.
 */
void
reset_tty_settings(int fd, TTY * tty_settings, int noset)
{
    unsigned mask;
#ifdef TIOCMGET
    int modem_bits;
#endif

    GET_TTY(fd, tty_settings);

#ifdef TERMIOS
#if defined(VDISCARD) && defined(CDISCARD)
    reset_char(VDISCARD, CDISCARD);
#endif
    reset_char(VEOF, CEOF);
    reset_char(VERASE, CERASE);
#if defined(VERASE2) && defined(CERASE2)
    reset_char(VERASE2, CERASE2);
#endif
#if defined(VFLUSH) && defined(CFLUSH)
    reset_char(VFLUSH, CFLUSH);
#endif
    reset_char(VINTR, CINTR);
    reset_char(VKILL, CKILL);
#if defined(VLNEXT) && defined(CLNEXT)
    reset_char(VLNEXT, CLNEXT);
#endif
    reset_char(VQUIT, CQUIT);
#if defined(VREPRINT) && defined(CRPRNT)
    reset_char(VREPRINT, CRPRNT);
#endif
#if defined(VSTART) && defined(CSTART)
    reset_char(VSTART, CSTART);
#endif
#if defined(VSTOP) && defined(CSTOP)
    reset_char(VSTOP, CSTOP);
#endif
#if defined(VSUSP) && defined(CSUSP)
    reset_char(VSUSP, CSUSP);
#endif
#if defined(VWERASE) && defined(CWERASE)
    reset_char(VWERASE, CWERASE);
#endif

    clear_flags(tty_settings->c_iflag, (IGNBRK
					| PARMRK
					| INPCK
					| ISTRIP
					| INLCR
					| IGNCR
					| IUCLC
					| IXANY
					| IXOFF));

    set_flags(tty_settings->c_iflag, (BRKINT
				      | IGNPAR
				      | ICRNL
				      | IXON
				      | IMAXBEL));

    clear_flags(tty_settings->c_oflag, (0
					| OLCUC
					| OCRNL
					| ONOCR
					| ONLRET
					| OFILL
					| OFDEL
					| NLDLY
					| CRDLY
					| TABDLY
					| BSDLY
					| VTDLY
					| FFDLY));

    set_flags(tty_settings->c_oflag, (OPOST
				      | ONLCR));

    mask = (CSIZE | CSTOPB | PARENB | PARODD);
#ifdef TIOCMGET
    /* leave clocal alone if this appears to use a modem */
    if (ioctl(fd, TIOCMGET, &modem_bits) == -1)
	mask |= CLOCAL;
#else
    /* cannot check - use the behavior from tset */
    mask |= CLOCAL;
#endif
    clear_flags(tty_settings->c_cflag, mask);

    set_flags(tty_settings->c_cflag, (CS8 | CREAD));
    clear_flags(tty_settings->c_lflag, (ECHONL
					| NOFLSH
					| TOSTOP
					| ECHOPRT
					| XCASE));

    set_flags(tty_settings->c_lflag, (ISIG
				      | ICANON
				      | ECHO
				      | ECHOE
				      | ECHOK
				      | ECHOCTL
				      | ECHOKE));
#endif /* TERMIOS */

    if (!noset) {
	SET_TTY(fd, tty_settings);
    }
}

/*
 * Returns a "good" value for the erase character.  This is loosely based on
 * the BSD4.4 logic.
 */
static int
default_erase(void)
{
    int result;

    if (over_strike
	&& VALID_STRING(key_backspace)
	&& strlen(key_backspace) == 1) {
	result = key_backspace[0];
    } else {
	result = CERASE;
    }

    return result;
}

/*
 * Update the values of the erase, interrupt, and kill characters in the TTY
 * parameter.
 *
 * SVr4 tset (e.g., Solaris 2.5) only modifies the intr, quit or erase
 * characters if they're unset, or if we specify them as options.  This differs
 * from BSD 4.4 tset, which always sets erase.
 */
void
set_control_chars(TTY * tty_settings, int my_erase, int my_intr, int my_kill)
{
#if defined(EXP_WIN32_DRIVER)
    /* noop */
    (void) tty_settings;
    (void) my_erase;
    (void) my_intr;
    (void) my_kill;
#else
    if (DISABLED(tty_settings->c_cc[VERASE]) || my_erase >= 0) {
	tty_settings->c_cc[VERASE] = UChar((my_erase >= 0)
					   ? my_erase
					   : default_erase());
    }

    if (DISABLED(tty_settings->c_cc[VINTR]) || my_intr >= 0) {
	tty_settings->c_cc[VINTR] = UChar((my_intr >= 0)
					  ? my_intr
					  : CINTR);
    }

    if (DISABLED(tty_settings->c_cc[VKILL]) || my_kill >= 0) {
	tty_settings->c_cc[VKILL] = UChar((my_kill >= 0)
					  ? my_kill
					  : CKILL);
    }
#endif
}

/*
 * Set up various conversions in the TTY parameter, including parity, tabs,
 * returns, echo, and case, according to the termcap entry.
 */
void
set_conversions(TTY * tty_settings)
{
#if defined(EXP_WIN32_DRIVER)
    /* FIXME */
#else
    set_flags(tty_settings->c_oflag, ONLCR);
    set_flags(tty_settings->c_iflag, ICRNL);
    set_flags(tty_settings->c_lflag, ECHO);
    set_flags(tty_settings->c_oflag, OXTABS);

    /* test used to be tgetflag("NL") */
    if (VALID_STRING(newline) && newline[0] == '\n' && !newline[1]) {
	/* Newline, not linefeed. */
	clear_flags(tty_settings->c_oflag, ONLCR);
	clear_flags(tty_settings->c_iflag, ICRNL);
    }
#if OXTABS
    /* test used to be tgetflag("pt") */
    if (VALID_STRING(set_tab) && VALID_STRING(clear_all_tabs))
	clear_flags(tty_settings->c_oflag, OXTABS);
#endif /* OXTABS */
    set_flags(tty_settings->c_lflag, (ECHOE | ECHOK));
#endif
}

static bool
sent_string(const char *s)
{
    bool sent = FALSE;
    if (VALID_STRING(s)) {
	tputs(s, 0, out_char);
	sent = TRUE;
    }
    return sent;
}

static bool
to_left_margin(void)
{
    if (VALID_STRING(carriage_return)) {
	sent_string(carriage_return);
    } else {
	out_char('\r');
    }
    return TRUE;
}

/*
 * Set the hardware tabs on the terminal, using the 'ct' (clear all tabs),
 * 'st' (set one tab) and 'ch' (horizontal cursor addressing) capabilities.
 * This is done before 'if' and 'is', so they can recover in case of error.
 *
 * Return TRUE if we set any tab stops, FALSE if not.
 */
static bool
reset_tabstops(int wide)
{
    if ((init_tabs != 8)
	&& VALID_NUMERIC(init_tabs)
	&& VALID_STRING(set_tab)
	&& VALID_STRING(clear_all_tabs)) {
	int c;

	to_left_margin();
	tputs(clear_all_tabs, 0, out_char);
	if (init_tabs > 1) {
	    if (init_tabs > wide)
		init_tabs = (short) wide;
	    for (c = init_tabs; c < wide; c += init_tabs) {
		fprintf(my_file, "%*s", init_tabs, " ");
		tputs(set_tab, 0, out_char);
	    }
	    to_left_margin();
	}
	return (TRUE);
    }
    return (FALSE);
}

/* Output startup string. */
bool
send_init_strings(int fd GCC_UNUSED, TTY * old_settings)
{
    int i;
    bool need_flush = FALSE;

    (void) old_settings;
#if TAB3
    if (old_settings != NULL &&
	old_settings->c_oflag & (TAB3 | ONLCR | OCRNL | ONLRET)) {
	old_settings->c_oflag &= (TAB3 | ONLCR | OCRNL | ONLRET);
	SET_TTY(fd, old_settings);
    }
#endif
    if (use_reset || use_init) {
	if (VALID_STRING(init_prog)) {
	    IGNORE_RC(system(init_prog));
	}

	need_flush |= sent_string((use_reset && (reset_1string != NULL))
				  ? reset_1string
				  : init_1string);

	need_flush |= sent_string((use_reset && (reset_2string != NULL))
				  ? reset_2string
				  : init_2string);

	if (VALID_STRING(clear_margins)) {
	    need_flush |= sent_string(clear_margins);
	}
#if defined(set_lr_margin)
	else if (VALID_STRING(set_lr_margin)) {
	    need_flush |= sent_string(TIPARM_2(set_lr_margin, 0, columns - 1));
	}
#endif
#if defined(set_left_margin_parm) && defined(set_right_margin_parm)
	else if (VALID_STRING(set_left_margin_parm)
		 && VALID_STRING(set_right_margin_parm)) {
	    need_flush |= sent_string(TIPARM_1(set_left_margin_parm, 0));
	    need_flush |= sent_string(TIPARM_1(set_right_margin_parm,
					       columns - 1));
	}
#endif
	else if (VALID_STRING(set_left_margin)
		 && VALID_STRING(set_right_margin)) {
	    need_flush |= to_left_margin();
	    need_flush |= sent_string(set_left_margin);
	    if (VALID_STRING(parm_right_cursor)) {
		need_flush |= sent_string(TIPARM_1(parm_right_cursor,
						   columns - 1));
	    } else {
		for (i = 0; i < columns - 1; i++) {
		    out_char(' ');
		    need_flush = TRUE;
		}
	    }
	    need_flush |= sent_string(set_right_margin);
	    need_flush |= to_left_margin();
	}

	need_flush |= reset_tabstops(columns);

	need_flush |= cat_file((use_reset && reset_file) ? reset_file : init_file);

	need_flush |= sent_string((use_reset && (reset_3string != NULL))
				  ? reset_3string
				  : init_3string);
    }

    return need_flush;
}

/*
 * Tell the user if a control key has been changed from the default value.
 */
static void
show_tty_change(TTY * old_settings,
		TTY * new_settings,
		const char *name,
		int which,
		unsigned def)
{
    unsigned older = 0, newer = 0;
    char *p;

#if defined(EXP_WIN32_DRIVER)
    /* noop */
    (void) old_settings;
    (void) new_settings;
    (void) name;
    (void) which;
    (void) def;
#else
    newer = new_settings->c_cc[which];
    older = old_settings->c_cc[which];

    if (older == newer && older == def)
	return;
#endif
    (void) fprintf(stderr, "%s %s ", name, older == newer ? "is" : "set to");

    if (DISABLED(newer)) {
	(void) fprintf(stderr, "undef.\n");
	/*
	 * Check 'delete' before 'backspace', since the key_backspace value
	 * is ambiguous.
	 */
    } else if (newer == 0177) {
	(void) fprintf(stderr, "delete.\n");
    } else if ((p = key_backspace) != NULL
	       && newer == (unsigned char) p[0]
	       && p[1] == '\0') {
	(void) fprintf(stderr, "backspace.\n");
    } else if (newer < 040) {
	newer ^= 0100;
	(void) fprintf(stderr, "control-%c (^%c).\n", UChar(newer), UChar(newer));
    } else
	(void) fprintf(stderr, "%c.\n", UChar(newer));
}

/**************************************************************************
 * Miscellaneous.
 **************************************************************************/

void
reset_start(FILE *fp, bool is_reset, bool is_init)
{
    my_file = fp;
    use_reset = is_reset;
    use_init = is_init;
}

void
reset_flush(void)
{
    if (my_file != NULL)
	fflush(my_file);
}

void
print_tty_chars(TTY * old_settings, TTY * new_settings)
{
#if defined(EXP_WIN32_DRIVER)
    /* noop */
#else
    show_tty_change(old_settings, new_settings, "Erase", VERASE, CERASE);
    show_tty_change(old_settings, new_settings, "Kill", VKILL, CKILL);
    show_tty_change(old_settings, new_settings, "Interrupt", VINTR, CINTR);
#endif
}

#if HAVE_SIZECHANGE
/*
 * Set window size if not set already, but update our copy of the values if the
 * size was set.
 */
void
set_window_size(int fd, NCURSES_INT2 *high, NCURSES_INT2 *wide)
{
    STRUCT_WINSIZE win;
    (void) ioctl(fd, IOCTL_GET_WINSIZE, &win);
    if (WINSIZE_ROWS(win) == 0 &&
	WINSIZE_COLS(win) == 0) {
	if (*high > 0 && *wide > 0) {
	    WINSIZE_ROWS(win) = (unsigned short) *high;
	    WINSIZE_COLS(win) = (unsigned short) *wide;
	    (void) ioctl(fd, IOCTL_SET_WINSIZE, &win);
	}
    } else if (WINSIZE_ROWS(win) > 0 &&
	       WINSIZE_COLS(win) > 0) {
	*high = (short) WINSIZE_ROWS(win);
	*wide = (short) WINSIZE_COLS(win);
    }
}
#endif
