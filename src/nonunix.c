/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992, 1993, 1994                 *
*                          by Zeyd M. Ben-Halim                            *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
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

#include "system.h"

/*
 * Prototype kernel file for non-UNIX ports.
 * The OS interface for non-UNIX ports should be modeled on this file.
 *
 * See also the non-UNIX sections in lib_acs.c, lib_beep.c, lib_color.c,
 * lib_endwin.c, lib_newterm.c, lib_options.c, lib_set_term.c.
 * Those sections are intended to capture functions which are trivial in any
 * non-UNIX port (i.e., mode tests that always return TRUE or FALSE);
 * the non-trivial BIOS interfacing should go here.
 *
 * What's implemented here is just curses; we don't bother with
 * tgetent()/tgetflag()/tgetnum()/tgetstr() or tigetent()/tigetflag()/
 * tigetnum()/tigetstr() because they can't be made to work; better
 * that developers should get a link failure when they try to use them.
 * We also don't implement erasechar()/killchar(), setupterm()/restartterm(),
 * set_curterm()/del_curterm().
 */

#include "curses.priv.h"

/* functions corresponding to UNIX lib_options.c */

int curs_set(int vis)
{
int cursor = SP->_cursor;

	T(("curs_set(%d)", vis));

	if (vis < 0 || vis > 2)
		return ERR;

	switch(vis) {
	case 2:
		/* replace this line with code to make cursor conspicuous */
		break;
	case 1:
		/* replace this line with code to make cursor normal */
		break;
	case 0:
		/* replace this line with code to make cursor invisible */
		break;
	}
	SP->_cursor = vis;
	return cursor;	
}

/*
 * functions corresponding to UNIX lib_vidattr.c
 *
 * we don't implement vidputs() or vidattr.
 */

attr_t termattrs(void)
{
    return (A_REVERSE|A_BLINK|A_DIM|A_BOLD|A_INVIS|A_PROTECT|A_ALTCHARSET);
}

/* functions corresponding to UNIX lib_longname.c */

char *longname(void)
{
    T(("longname() called"));

    return("memory-mapped console");	/* replace this with actual type */
}

/* functions corresponding to UNIX lib_mvcur.c */

void mvcur_init(SCREEN *SP)
{
    /* initialize screen mode */
}

int mvcur(int yold, int xold, int ynew, int xnew)
{
    TR(TRACE_MOVE, ("mvcur(%d,%d,%d,%d) called", yold, xold, ynew, xnew));

    if (yold == ynew && xold == xnew)
	return(OK);

    /* move the cursor to (ynew, xnew) */

    return(OK);
}

void mvcur_wrap(void)
{
    /* de-initialize screen mode */
}

/* functions corresponding to UNIX lib_doupdate.c */

int doupdate(void)
{
int	i, j;
	
	T(("doupdate() called"));

	/* actual update logic goes here */
}

int _outch(int ch)
{
	putc(ch, stdout);
	return OK;
}

void outstr(char *str)
{
    (void) fputs(str, stdout);
    (void) fflush(stdout);
}

/* functions corresponding to UNIX lib_twait.c */

void usleep(unsigned long usec)
/* sleep for usec milliseconds */
{
    /* replace this */
}

int timed_wait(int fd, int wait, int *timeleft)
{
    /* replace this */
}

/* functions corresponding to UNIX lib_raw.c */

int reset_prog_mode()
{
	T(("reset_prog_mode() called"));

	/* set curses mode */

	return OK; 
}

int reset_shell_mode()
{
	T(("reset_shell_mode() called"));

	fflush(SP->_ofp);

	/* set normal mode */

	return OK; 
}

int delay_output(float ms)
{
	T(("delay_output(%f) called", ms));

	/* waste ms milliseconds */

    	return OK;
}

int flushinp()
{
	T(("flushinp() called"));

	/* flush pending input */

	return OK;
}

int baudrate()
{
	T(("baudrate() called"));

	return(19200);	/* a guess at BIOS speed */
}

int savetty()
{
	T(("savetty() called"));

	/* a no-op */

	return OK;
}

int resetty()
{
	T(("resetty() called"));

	/* a no-op */

	return OK;
}

/* functions corresponding to UNIX lib_raw.c */

int raw()
{
	T(("raw() called"));

	SP->_raw = TRUE;
	SP->_cbreak = TRUE;

	/* set raw mode */

	return OK;
}

int cbreak()
{
	T(("cbreak() called"));

	SP->_cbreak = TRUE;

	/* set cbreak mode */

	return OK;
}

int echo()
{
	T(("echo() called"));

	SP->_echo = TRUE;
    
	/* set echo mode */

	return OK;
}

int nl()
{
	T(("nl() called"));

	SP->_nl = TRUE;

	/* replace this */

	return OK;
}

int qiflush()
{
	T(("qiflush() called"));

	/* replace this */

	return OK;
}

int noraw()
{
	T(("noraw() called"));

	SP->_raw = FALSE;
	SP->_cbreak = FALSE;

	/* replace this */

	return OK;
}

int nocbreak()
{
	T(("nocbreak() called"));

	SP->_cbreak = 0;
	
	/* replace this */

	return OK;
}

int noecho()
{
	T(("noecho() called"));

	SP->_echo = FALSE;
	
	/* replace this */

	return OK;
}

int nonl()
{
	T(("nonl() called"));

	SP->_nl = FALSE;
	
	/* replace this */

	return OK;
}

int noqiflush()
{
	T(("noqiflush() called"));

	/* replace this */

	return OK;
}

int intrflush(WINDOW *win, bool flag)
{
	T(("intrflush() called"));

	/* replace this */

	return OK;
}

/* functions corresponding to UNIX lib_raw.c */

void use_env(bool f)
{
	/* a no-op */
}

int def_shell_mode(void)
{
	T(("def_shell_mode() called"));

	/* a no-op */
}

int def_prog_mode(void)
{
	T(("def_prog_mode() called"));

	/* a no-op */
}

/* functions corresponding to UNIX lib_tstp.c */

void _nc_signal_handler(bool enable)
{
	/* replace this line */
}

/* the test main */

#ifdef MAIN
int main(int argc, char *argv[])
{
    /*
     * Right now, this is here only to flush out dangling references
     * in the link.  Soon we'll turn this into a real test driver.
     */
}
#endif /* MAIN */
