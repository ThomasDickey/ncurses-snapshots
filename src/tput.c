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

#include "system.h"

/*
 * tput.c -- shellscript access to terminal capabilities
 *
 * by Eric S. Raymond <esr@snark.thyrsus.com>, portions based on code from
 * Ross Ridge's mytinfo package.
 */

#include <ctype.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <curses.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#if !HAVE_EXTERN_ERRNO
extern int errno;
#endif

#include "term.h"

#define PUTS(s)		fputs(s, stdout)
#define PUTCHAR(c)	putchar(c)
#define FLUSH		fflush(stdout)

static void (*cleanup)();

static char *prg_name;

static void quit(int status, const char *fmt, ...)
{
va_list argp;

	va_start(argp,fmt);
	vfprintf (stderr, fmt, argp);
	fprintf(stderr, "\n");
	va_end(argp);
	exit(status);
}

static void clean(int e)
{
	return;
}

static void usage(int e)
{
	fprintf(stderr, "usage: %s [-T term] capname\n", prg_name);
	return;
}

static int tput(int argc, char *argv[])
{
char *s;
int i, j, c;
int reset, status;
FILE *f;

	reset = 0;
	if (strcmp(argv[0], "reset") == 0) {
		reset = 1;
	}
	if (reset || strcmp(argv[0], "init") == 0) {
		if (init_prog != NULL) {
			system(init_prog);
	}
	FLUSH;

	if (reset && reset_1string != NULL) {
		PUTS(reset_1string);
	} else if (init_1string != NULL) {
		PUTS(init_1string);
	}
	FLUSH;

	if (reset && reset_2string != NULL) {
		PUTS(reset_2string);
	} else if (init_2string != NULL) {
		PUTS(init_2string);
	}
	FLUSH;

	if (set_lr_margin != NULL) {
		PUTS(tparm(set_lr_margin, 0, columns - 1));
	} else if (set_left_margin_parm != NULL
		   && set_right_margin_parm != NULL) {
		PUTS(tparm(set_left_margin_parm, 0));
		PUTS(tparm(set_right_margin_parm, columns - 1));
	} else if (clear_margins != NULL && set_left_margin != NULL
		   && set_right_margin != NULL) {
		PUTS(clear_margins);
		if (carriage_return != NULL) {
			PUTS(carriage_return);
		} else {
			PUTCHAR('\r');
		}
		PUTS(set_left_margin);
		if (parm_right_cursor) {
			PUTS(tparm(parm_right_cursor, columns - 1));
		} else {
			for(i = 0; i < columns - 1; i++) {
				PUTCHAR(' ');
			}
		}
		PUTS(set_right_margin);
		if (carriage_return != NULL) {
			PUTS(carriage_return);
		} else {
			PUTCHAR('\r');
		}
	}
	FLUSH;

	if (init_tabs != 8) {
		if (clear_all_tabs != NULL && set_tab != NULL) {
			for(i = 0; i < columns - 1; i += 8) {
				if (parm_right_cursor) {
					PUTS(tparm(parm_right_cursor, 8));
				} else {
					for(j = 0; j < 8; j++) 
						PUTCHAR(' ');
				}
				PUTS(set_tab);
			}
			FLUSH;
		}
	}

	if (reset && reset_file != NULL) {
		f = fopen(reset_file, "r");
		if (f == NULL) {
			quit(errno, "Can't open reset_file: '%s'", reset_file);
		}
		while((c = fgetc(f)) != EOF) {
			PUTCHAR(c);
		}
		fclose(f);
	} else if (init_file != NULL) {
		f = fopen(init_file, "r");
		if (f == NULL) {
			quit(errno, "Can't open init_file: '%s'", init_file);
		}
		while((c = fgetc(f)) != EOF) {
			PUTCHAR(c);
		}
		fclose(f);
	}
	FLUSH;

	if (reset && reset_3string != NULL) {
		PUTS(reset_3string);
	} else if (init_2string != NULL) {
		PUTS(init_2string);
	}
	FLUSH;
	return 0;
	}

	if (strcmp(argv[0], "longname") == 0) {
		PUTS(longname());
		return 0;
	}

	if ((status = tigetflag(argv[0])) != -1)
		return(status != 0);
	else if ((status = tigetnum(argv[0])) != -2) {
		(void) printf("%d", status);
		return(0);
	}
	else if ((s = tigetstr(argv[0])) == (char *)-1)
		quit(4, "%s: unknown terminfo capability '%s'", prg_name, argv[0]);
	else if (s != (char *)NULL) {
		if (argc > 1) {
		int k;

			/* Nasty hack time. The tparm function needs to see numeric
			 * parameters as numbers, not as pointers to their string
			 * representations
			 */

			 for (k = 1; k < argc; k++)
			 	if (isdigit(argv[k][0]))
			 		argv[k] = (char *)atoi(argv[k]);

				s = tparm(s,argv[1],argv[2],argv[3],argv[4],
					    argv[5],argv[6],argv[7],argv[8],
					    argv[9]);
		}
		fputs(s, stdout);
		return(0);
	}
	return(0);
}

int main(argc, argv)
int argc;
char **argv;
{
char *s, *term;
int errret, cmdline = 1;

	prg_name = argv[0];
	s = strrchr(prg_name, '/');
	if (s != NULL && *++s != '\0')
	prg_name = s;

	term = getenv("TERM");

	cleanup = usage;

	if (argc > 2 && argv[1][0] == '-' && argv[1][1] == 'S') {
	cmdline = 0;
	argc--;
	argv++;
	}

	if (argc > 2 && argv[1][0] == '-' && argv[1][1] == 'T') {
	if (argv[1][2] == '\0' && argc > 3) {
		term = argv[2];
		argc--;
		argv++;
	} else {
		term = argv[1] + 2;
	}
	argc--;
	argv++;
	}

	if (argc < 2) {
	quit(2, "usage error");
	}

	cleanup = clean;

	setupterm(term, STDOUT_FILENO, &errret);
	if (errret == ERR)
	quit(3, "unknown terminal \"%s\"", term);

	if (cmdline)
	return(tput(argc - 1, argv + 1));
	else {
	char	buf[BUFSIZ];
	int errors = 0;

	while (fgets(buf, sizeof(buf), stdin) != (char *)NULL) {
		int	 argnum = 0;
		char    *cp;

		/* crack the argument list into a dope vector */
		for (cp = buf; *cp; cp++) {
		if (isspace(*cp))
			*cp = '\0';
		else if (cp == buf || cp[-1] == 0)
			argv[argnum++] = cp;
		}
		argv[argnum] = (char *)NULL;

		if (tput(argnum, argv) != 0)
		errors++;
	}

	return(errors > 0);
	}
}

