
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
 *	setupterm(termname, Filedes, errret)
 *
 *	Find and read the appropriate object file for the terminal
 *	Make cur_term point to the structure.
 *	Turn off the XTABS bit in the tty structure if it was on
 *	If XTABS was on, remove the tab and backtab capabilities.
 *
 */

#include <stdlib.h>
#include <string.h>
#ifndef SUNIOCTL
#include <sys/ioctl.h>
#endif
#include "curses.priv.h"
#include "terminfo.h"

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


static void do_prototype(void);

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

#ifndef BSDTABS
#define tabs TAB3
#else
#define tabs OXTABS
#endif
 
int def_shell_mode()
{
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

static bool name_match(char *namelst, const char *name)
{
char *cp, namecopy[NAMESIZE];

	T(("matching %s with %s", name, namelst));
	if (namelst == NULL)
		return FALSE;
    	strcpy(namecopy, namelst);
    	if ((cp = strtok(namelst, "|")) != NULL)
    		do {
			if (strcmp(cp, name) == 0)
			    return(TRUE);
    		} while
		((cp = strtok((char *)NULL, "|")) != NULL);

    	return FALSE;
}

char ttytype[NAMESIZE];

int setupterm(char *termname, int Filedes, int *errret)
{
char		filename1[1024];
char		filename2[1024];
char		*directory = SRCDIR;
char		*terminfo;
struct term	*term_ptr;
char 		*rows, *cols;

#ifdef TRACE
	_init_trace();
	T(("setupterm(%s,%d,%x) called", termname, Filedes, errret));
#endif

	if (termname == NULL) {
    		termname = getenv("TERM");
    		if (termname == NULL)
			ret_error0(-1, "TERM environment variable not set.\n");
	}

	T(("your terminal name is %s", termname));

	if (name_match(ttytype, termname) == FALSE || isendwin()) {
	
		if (isendwin()) {
			extern int del_curterm();

			T(("deleting cur_term"));
			T(("must be resizing"));
			del_curterm(cur_term);
		}

       		term_ptr = (struct term *) malloc(sizeof(struct term));

		if (term_ptr == NULL)
	    		ret_error0(-1, "Not enough memory to create terminal structure.\n") ;

		if ((terminfo = getenv("TERMINFO")) != NULL)
		    	directory = terminfo;

		sprintf(filename1, "%s/%c/%s", directory, termname[0], termname);
		sprintf(filename2, "%s/%c/%s", SRCDIR, termname[0], termname);

		if (read_entry(filename1, &term_ptr->type) < 0 &&  read_entry(filename2, &term_ptr->type) < 0)
		    	ret_error(-1, "'%s': Unknown terminal type.\n", termname);

		cur_term = term_ptr;
		if (command_character  &&  getenv("CC"))
		    	do_prototype();
	
		strncpy(ttytype, cur_term->type.term_names, NAMESIZE - 1);
		ttytype[NAMESIZE - 1] = '\0';
		cur_term->Filedes = Filedes;

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
			if (!isatty(Filedes) || (resize(Filedes) == 1)) {
				/* no window size, our last hope is terminfo */
				if (lines > 0 && columns > 0) {
					LINES = lines;
					COLS  = columns; 
				} 
			}
		}
		/* should have found out the dimensions by now */
		lines = LINES;
		columns = COLS;
			
		T(("screen size is %dx%d and %dx%d", LINES, COLS, lines, columns));
	}
	if (errret)
		*errret = 1;
	return(1);
}

int set_curterm(TERMINAL *term)
{

	cur_term = term;
	return OK;
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
do_prototype()
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
