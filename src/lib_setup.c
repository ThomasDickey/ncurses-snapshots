
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

#include <stdlib.h>
#include <string.h>
#include "curses.priv.h"
#include "term.h"	/* lines, columns, cur_term */

/****************************************************************************
 *
 * Terminal size computation
 *
 ****************************************************************************/

#ifndef SUNIOCTL
#include <sys/ioctl.h>
#endif

static int _use_env = TRUE;

void use_env(bool f)
{
	_use_env = f;
}

int LINES, COLS;

static int resize(int fd)
{
#ifndef BROKEN_TIOCGWINSZ
struct winsize size;

	if (ioctl(fd, TIOCGWINSZ, &size) < 0) {
		perror("TIOCGWINSZ");
		return 1;
	}
	LINES = size.ws_row;
	COLS = size.ws_col;
	if (LINES == 0 || COLS == 0)
		return 1;
	return 0;
#else
	return 1;
#endif
}

void get_screensize(void)
/* set LINES and COLS from the environment and/or terminfo entry */
{
char 		*rows, *cols;

	/* figure out the size of the screen */
	T(("screen size: terminfo lines = %d columns = %d", lines, columns));
	
	/* get value of LINES and COLUMNS environment variables */
	LINES = COLS = 0;
	rows = getenv("LINES");
	if (rows != (char *)NULL)
		LINES = atoi(rows);
	cols = getenv("COLUMNS");
	if (cols != (char *)NULL)
		COLS = atoi(cols);
	T(("screen size: environment LINES = %d COLUMNS = %d", LINES, COLS));

	/* if _use_env is false then override the environment */
	if (_use_env == FALSE) 
		if (lines > 0 && columns > 0) {
			LINES = lines;
			COLS  = columns; 
		} 

	/* If _use_env is true but environment is undefined:
	   If we can get window size, use it,
	   else try lines/columns,
	   else give up.
	*/

	if (LINES <= 0 || COLS <= 0) {
		if (!isatty(cur_term->Filedes) || resize(cur_term->Filedes)) {
			/* no window size, our last hope is terminfo */
			if (lines > 0 && columns > 0) {
				LINES = lines;
				COLS  = columns; 
			} 
		}
	}

	T(("screen size is %dx%d", LINES, COLS));
}

/****************************************************************************
 *
 * Mode sets
 *
 ****************************************************************************/

#ifndef BSDTABS
#define tabs TAB3
#else
#define tabs OXTABS
#endif
 
int def_shell_mode()
{
    /*
     *	Turn off the XTABS bit in the tty structure if it was on
     *	If XTABS was on, remove the tab and backtab capabilities.
     */

	T(("def_shell_mode() called"));

#ifdef TERMIOS
 	if((tcgetattr(cur_term->Filedes, &cur_term->Ottyb)) == -1) {
 		perror("def_shell_mode() tcgetattr() failed:");
 		exit(1);
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

int def_prog_mode()
{
	T(("def_prog_mode() called"));

#ifdef TERMIOS
 	if((tcgetattr(cur_term->Filedes, &cur_term->Nttyb)) == -1) {
 		perror("def_prog_mode() tcgetattr() failed:");
 		exit(1);
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

static int grab_entry(char *tn, TERMTYPE *tp)
{
	if (read_entry(tn, tp) == OK)
	    return(OK);

#ifdef TERMCAP_FILE
	/* try falling back on the termcap file */
	if (read_termcap_entry(tn, tp) == OK)
		return(OK);
#endif /* TERMCAP_FILE */

	return(ERR);
}

char ttytype[NAMESIZE];

/*
 *	setupterm(termname, Filedes, errret)
 *
 *	Find and read the appropriate object file for the terminal
 *	Make cur_term point to the structure.
 *
 */

int setupterm(char *termname, int Filedes, int *errret)
{
struct term	*term_ptr;
static void do_prototype(void);

#ifdef TRACE
	T(("setupterm(%s,%d,%p) called", termname, Filedes, errret));
#endif

	if (termname == NULL) {
    		termname = getenv("TERM");
    		if (termname == NULL)
			ret_error0(-1, "TERM environment variable not set.\n");
	}

	T(("your terminal name is %s", termname));

	if (name_match(ttytype, termname) == FALSE || isendwin()) {
	
		if (isendwin()) {
			extern int del_curterm(TERMINAL *term);

			T(("deleting cur_term"));
			T(("must be resizing"));
			del_curterm(cur_term);
		}

       		term_ptr = (struct term *) malloc(sizeof(struct term));

		if (term_ptr == NULL)
	    		ret_error0(-1, "Not enough memory to create terminal structure.\n") ;

		if (grab_entry(termname, &term_ptr->type) < 0)
		    	ret_error(-1,
				  "'%s': Unknown terminal type.\n",
				  termname);

		cur_term = term_ptr;
		if (generic_type)
		    	ret_error(-1,
				  "'%s': I need something more specific.\n",
				  termname);
		if (hard_copy)
		    	ret_error(-1,
				  "'%s': I can't handle hardcopy terminals.\n",
				  termname);

		if (command_character  &&  getenv("CC"))
		    	do_prototype();
	
		strncpy(ttytype, cur_term->type.term_names, NAMESIZE - 1);
		ttytype[NAMESIZE - 1] = '\0';
		cur_term->Filedes = Filedes;

		get_screensize();
	}

	if (errret)
		*errret = 1;
	return(1);

}

int restartterm(char *term, int filenum, int *errret)
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
