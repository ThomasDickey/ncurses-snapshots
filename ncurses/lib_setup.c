
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
 * Terminal setup routines:
 *
 *		setup_sizes(void)
 *		use_env(bool)
 *		setupterm(char *, int, int *)
 *		TERMINAL *set_curterm(TERMINAL *)
 *		int del_curterm(TERMINAL *)
 */

#include "curses.priv.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if !HAVE_EXTERN_ERRNO
extern int errno;
#endif

#include "term.h"	/* lines, columns, cur_term */

/****************************************************************************
 *
 * Terminal size computation
 *
 ****************************************************************************/

#if !defined(sun) || !HAVE_TERMIOS_H
#include <sys/ioctl.h>
#endif

static int _use_env = TRUE;

static void do_prototype(void);

void use_env(bool f)
{
	_use_env = f;
}

int LINES, COLS, TABSIZE;

static void get_screensize(void)
/* set LINES and COLS from the environment and/or terminfo entry */
{
char 		*rows, *cols;

	/* figure out the size of the screen */
	T(("screen size: terminfo lines = %d columns = %d", lines, columns));

	if (!_use_env)
	{
	    LINES = (int)lines;
	    COLS  = (int)columns;
	}
	else	/* usually want to query LINES and COLUMNS from environment */
	{
	    LINES = COLS = 0;

	    /* first, look for environment variables */
	    rows = getenv("LINES");
	    if (rows != (char *)NULL)
		LINES = atoi(rows);
	    cols = getenv("COLUMNS");
	    if (cols != (char *)NULL)
		COLS = atoi(cols);
	    T(("screen size: environment LINES = %d COLUMNS = %d",LINES,COLS));

#if defined(TIOCGWINSZ) && !defined(BROKEN_TIOCGWINSZ)
	    /* if that didn't work, maybe we can try asking the OS */
	    if (LINES <= 0 || COLS <= 0)
	    {
		if (isatty(cur_term->Filedes))
		{
		    struct winsize size;

		    errno = 0;
		    do {
			if (ioctl(cur_term->Filedes, TIOCGWINSZ, &size) < 0
				&& errno != EINTR)
			    goto failure;
		    } while
			(errno == EINTR);

		    LINES = (int)size.ws_row;
		    COLS  = (int)size.ws_col;
		}
		/* FALLTHRU */
	    failure:;
	    }
#endif /* defined(TIOCGWINSZ) && !defined(BROKEN_TIOCGWINSZ) */

	    /* if we can't get dynamic info about the size, use static */
	    if (LINES <= 0 || COLS <= 0)
		if (lines > 0 && columns > 0)
		{
		    LINES = (int)lines;
		    COLS  = (int)columns;		
		}

	    /* the ultimate fallback, assume fixed 24x80 size */
	    if (LINES <= 0 || COLS <= 0)
	    {
		LINES = 24;
		COLS  = 80;
	    }

	    /*
	     * Put the derived values back in the screen-size caps, so
	     * tigetnum() and tgetnum() will do the right thing.
	     */
	    lines   = (short)LINES;
	    columns = (short)COLS;
	}

	T(("screen size is %dx%d", LINES, COLS));

#ifdef init_tabs
	if (init_tabs != -1)
		TABSIZE = (int)init_tabs;
	else
#endif /* init_tabs */
		TABSIZE = 8;
	T(("TABSIZE = %d", TABSIZE));

}

/****************************************************************************
 *
 * Mode sets
 *
 ****************************************************************************/

#undef tabs

#ifdef TAB3
# define tabs TAB3
#else
# ifdef XTABS
#  define tabs XTABS
# else
#  ifdef OXTABS
#   define tabs OXTABS
#  else
#   define tabs 0
#  endif
# endif
#endif
 
int def_shell_mode(void)
{
    /*
     *	Turn off the XTABS bit in the tty structure if it was on
     *	If XTABS was on, remove the tab and backtab capabilities.
     */

	T(("def_shell_mode() called"));

#ifdef TERMIOS
 	if((tcgetattr(cur_term->Filedes, &cur_term->Ottyb)) == -1) {
#if 0
 		perror("def_shell_mode() tcgetattr() failed:");
 		exit(1);
#else
		return ERR;
#endif
 	}
 	if (cur_term->Ottyb.c_oflag & tabs)
		tab = back_tab = NULL;
	
#else
	gtty(cur_term->Filedes, &cur_term->Ottyb);
	if (cur_term->Ottyb.sg_flags & XTABS)
	    	tab = back_tab = NULL;
#endif
	return OK;
}

int def_prog_mode(void)
{
	T(("def_prog_mode() called"));

#ifdef TERMIOS
 	if((tcgetattr(cur_term->Filedes, &cur_term->Nttyb)) == -1) {
#if 0
 		perror("def_prog_mode() tcgetattr() failed:");
 		exit(1);
#else
		return ERR;
#endif
 	}
 	cur_term->Nttyb.c_oflag &= ~tabs;
#else
	gtty(cur_term->Filedes, &cur_term->Nttyb);
	
	cur_term->Nttyb.sg_flags &= ~XTABS;
#endif
	return OK;
}

/****************************************************************************
 *
 * Terminal setup
 *
 ****************************************************************************/

#define ret_error(code, fmt, arg)	if (errret) {\
					    *errret = code;\
					    return(code);\
					} else {\
					    fprintf(stderr, fmt, arg);\
					    exit(1);\
					}

#define ret_error0(code, msg)		if (errret) {\
					    *errret = code;\
					    return(code);\
					} else {\
					    fprintf(stderr, msg);\
					    exit(1);\
					}

static int grab_entry(const char *const tn, TERMTYPE *const tp)
/* return 1 if entry found, 0 if not found, -1 if database not accessible */
{
	char	filename[PATH_MAX];
	int	status;

	if ((status = _nc_read_entry(tn, filename, tp)) == 1)
	    return(1);

#ifndef PURE_TERMINFO
	/*
	 * Try falling back on the termcap file.  Note: allowing this call
	 * links the entire terminfo/termcap compiler into the startup code.
	 * It's preferable to build a real terminfo database and use that.
	 */
	status = _nc_read_termcap_entry(tn, tp);
#endif /* PURE_TERMINFO */

	return(status);
}

char ttytype[NAMESIZE];

/*
 *	setupterm(termname, Filedes, errret)
 *
 *	Find and read the appropriate object file for the terminal
 *	Make cur_term point to the structure.
 *
 */

int setupterm(const char *tname, int Filedes, int *errret)
{
struct term	*term_ptr;

	T(("setupterm(%s,%d,%p) called", tname, Filedes, errret));

	if (tname == NULL) {
    		tname = getenv("TERM");
    		if (tname == NULL)
			ret_error0(-1, "TERM environment variable not set.\n");
	}

	T(("your terminal name is %s", tname));

	if (_nc_name_match(ttytype, tname, "|") == FALSE || isendwin()) {
		int status;

		if (isendwin()) {
			T(("deleting cur_term"));
			T(("must be resizing"));
			del_curterm(cur_term);
		}

       		term_ptr = (struct term *) calloc(1, sizeof(struct term));

		if (term_ptr == NULL)
	    		ret_error0(-1, "Not enough memory to create terminal structure.\n") ;
		status = grab_entry(tname, &term_ptr->type);

		/* try fallback list if entry on disk */
		if (status != 1)
		{
		    const TERMTYPE	*fallback = _nc_fallback(tname);

		    if (fallback)
		    {
			memcpy(&term_ptr->type, fallback, sizeof(TERMTYPE));
			status = 1;
		    }
		}

		if (status == -1)
		{
			ret_error0(-1, "terminals database is inaccessible\n");
		}
		else if (status == 0)
		{
		    	ret_error(0, "'%s': unknown terminal type.\n", tname);
		}

		cur_term = term_ptr;
		if (generic_type)
		    	ret_error(0, "'%s': I need something more specific.\n", tname);
		if (hard_copy)
		    	ret_error(1, "'%s': I can't handle hardcopy terminals.\n", tname);

		if (command_character  &&  getenv("CC"))
		    	do_prototype();
	
		strncpy(ttytype, cur_term->type.term_names, NAMESIZE - 1);
		ttytype[NAMESIZE - 1] = '\0';

		/*
		 * Allow output redirection.  This is what SVr3 does.
		 * If stdout is directed to a file, screen updates go
		 * to standard error.
		 */
		if (Filedes == STDOUT_FILENO && !isatty(Filedes))
		    Filedes = STDERR_FILENO;
		cur_term->Filedes = Filedes;

		get_screensize();
	}

	if (errret)
		*errret = 1;
	return(1);

}

int restartterm(const char *term, int filenum, int *errret)
{
int saveecho = SP->_echo;
int savecbreak = SP->_cbreak;
int saveraw = SP->_raw;
int savenl = SP->_nl;

	setupterm(term, filenum, errret);

	if (saveecho)
		echo();
	else
		noecho();

	if (savecbreak) {
		cbreak();
		noraw();
	} else if (saveraw) {
		nocbreak();
		raw();
	} else {
		nocbreak();
		noraw();
	}
	if (savenl)
		nl();
	else
		nonl();

	reset_prog_mode();

	get_screensize();

	return(OK);
}

TERMINAL *set_curterm(TERMINAL *term)
{
	TERMINAL	*oldterm = cur_term;

	cur_term = term;
	return oldterm;
}

int del_curterm(TERMINAL *term)
{

	if (term != NULL) {
		if (term->type.str_table != NULL)
			free(term->type.str_table);
		free(term);
		return OK;
	}
	return ERR;
}


/*
**	do_prototype()
**
**	Take the real command character out of the CC environment variable
**	and substitute it in for the prototype given in 'command_character'.
**
*/

static void
do_prototype(void)
{
int	i, j;
char	CC;
char	proto;
char    *tmp;

	tmp = getenv("CC");
	CC = *tmp;
	proto = *command_character;

	for (i=0; i < STRCOUNT; i++) {
    		j = 0;
    		while (cur_term->type.Strings[i][j]) {
			if (cur_term->type.Strings[i][j] == proto)
	    			cur_term->type.Strings[i][j] = CC;
			j++;
    		}
	}
}
