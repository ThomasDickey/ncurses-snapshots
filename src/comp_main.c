
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992, 1993, 1994                 *
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
 *	comp_main.c --- Main program for terminfo compiler
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tic.h"
#include "version.h"
#include "terminfo.h"
#include "dump_entry.h"

char	*source_file = "terminfo";
char	*destination = SRCDIR;
char	*usage_string = "tic [-v[n]] source-file\n";
char	check_only = 0;
int	width = 60;
int	dodump = 0;		/* running as captoinfo? */
char	*progname;

static void check_writeable(void);

int main (int argc, char *argv[])
{
int	i;
int	argflag = FALSE;
char    *termcap;

	progname = argv[0];
	dodump = (strcmp(progname, "captoinfo") == 0);

	debug_level = 0;

	for (i = 1; i < argc; i++) {
	    	if (argv[i][0] == '-') {
			switch (argv[i][1]) {
		    	case 'c':
				check_only = 1;
				break;
		    	case 'v':
				debug_level = argv[i][2]  ?  atoi(&argv[i][2])  :  1;
				break;
		    	case 'I':
				dodump = 1;
				break;
		    	case '1':
		    		width = 0;
		    		break;
	    	    	case 'w':
				width = argv[i][2]  ?  atoi(&argv[i][2])  :  1;
				break;
		    	case 'V':
				fputs(NCURSES_VERSION, stdout);
				exit(0);
		    	default:
				fprintf(stderr, "%s: Unknown option. Usage is:\n\t%s\n",
				        argv[0], usage_string);
				exit(1);
			}
	    	} else if (argflag) {
			fprintf(stderr, "%s: Too many file names.  Usage is:\n\t%s\n",
				argv[0], usage_string);
			exit(1);
		} else {
			argflag = TRUE;
			source_file = argv[i];
	    	}
	}

	if (argflag == FALSE && dodump == 1) {
		if ((termcap = getenv("TERMCAP")) != NULL) {
			if (access(termcap, F_OK) == 0) {
				/* file exists */
				source_file = termcap;
			}
		}
	}

	if (freopen(source_file, "r", stdin) == NULL) {
	    	fprintf(stderr, "%s: Can't open %s\n", progname, source_file);
	    	exit(1);
	}

	make_hash_table();
	if (dodump)
		dump_init(F_TERMINFO, S_TERMINFO, width, debug_level);
	else
		check_writeable();
	compile();

	return(0);
}


/*
 *	check_writeable(progname)
 *
 *	Miscellaneous initialisations
 *
 *	Check for access rights to destination directories
 *	Create any directories which don't exist.
 *
 */

static void check_writeable(void)
{
struct stat	statbuf;
char		*dirnames = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
char		dir[2];

	if (getenv("TERMINFO") != NULL)
	    destination = getenv("TERMINFO");

	if (access(destination, 7) < 0) {
	    fprintf(stderr, "%s: %s non-existant or permission denied\n",
			    progname, destination);
	    exit(1);
	}

	if (chdir(destination) < 0) {
	    fprintf(stderr, "%s: %s is not a directory\n",
			    progname, destination);
	    exit(1);
	}
	
	dir[1] = '\0';
	for (dir[0] = *dirnames; *dirnames != '\0'; dir[0] = *(++dirnames)) {
	    	if (stat(dir, &statbuf) < 0) {
			mkdir(dir, 0755);
	    	} else if (access(dir, 7) < 0) {
			fprintf(stderr, "%s: %s/%s: Permission denied\n",
				        progname, destination, dir);
			exit(1);
	    	}
#ifdef _POSIX_SOURCE
	    	else if (!(S_ISDIR(statbuf.st_mode)))
#else
	    	else if ((statbuf.st_mode & S_IFMT) != S_IFDIR)
#endif	    
	    	{
			fprintf(stderr, "%s: %s/%s: Not a directory\n",
						    progname, destination, dir);
			exit(1);
	    	}
	}
}


