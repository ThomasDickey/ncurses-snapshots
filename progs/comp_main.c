
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
 *	comp_main.c --- Main program for terminfo compiler
 *
 */

#include <config.h>

#include "tic.h"

#include <string.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
# if HAVE_LIBC_H
#include <libc.h>
# endif
#endif

#include "term.h"
#include "dump_entry.h"
#include "term_entry.h"

char	*_nc_progname;

static int	width = 60;
static bool	infodump = FALSE;	/* running as captoinfo? */
static bool	capdump = FALSE;	/* running as infotocap? */
static char	*source_file = "terminfo";
static char	*const usage_string = "tic [-v[n]] source-file\n";
static char	check_only = 0;

int main (int argc, char *argv[])
{
int	i, debug_level = 0;
int	argflag = FALSE, smart_defaults = TRUE;
char    *termcap;
ENTRY	*qp;

	if ((_nc_progname = strrchr(argv[0], '/')) == NULL)
		_nc_progname = argv[0];
	else
		_nc_progname++;

	infodump = (strcmp(_nc_progname, "captoinfo") == 0);
	capdump = (strcmp(_nc_progname, "infotocap") == 0);

	for (i = 1; i < argc; i++) {
	    	if (argv[i][0] == '-') {
			switch (argv[i][1]) {
		    	case 'c':
				check_only = 1;
				break;
		    	case 'v':
				debug_level = argv[i][2] ? atoi(&argv[i][2]):1;
				_nc_tracing = (1 << debug_level) - 1;
				break;
		    	case 'I':
				infodump = TRUE;
				break;
		    	case 'C':
				capdump = TRUE;
				break;
			case 'N':
				smart_defaults = FALSE;
				break;
		    	case '1':
		    		width = 0;
		    		break;
	    	    	case 'w':
				width = argv[i][2]  ?  atoi(&argv[i][2])  :  1;
				break;
		    	case 'V':
				(void) fputs(NCURSES_VERSION, stdout);
				exit(0);
		    	default:
				fprintf (stderr, 
					"%s: Unknown option. Usage is:\n\t%s",
					_nc_progname,
				        usage_string);
				exit(1);
			}
	    	} else if (argflag) {
			fprintf (stderr, 
				"%s: Too many file names.  Usage is:\n\t%s",
				_nc_progname,
				usage_string);
			exit(1);
		} else {
			argflag = TRUE;
			source_file = argv[i];
	    	}
	}

	if (argflag == FALSE) {
		if (infodump == TRUE) {
			/* captoinfo's no-argument case */
			termcap = "/etc/termcap";
			if ((termcap = getenv("TERMCAP")) != NULL) {
				if (access(termcap, F_OK) == 0) {
					/* file exists */
					source_file = termcap;
				}
			}
		} else {
		/* tic */
			fprintf (stderr, 
				"%s: File name needed.  Usage is:\n\t%s",
				_nc_progname,
				usage_string);
			exit(1);
		}
	}

	if (freopen(source_file, "r", stdin) == NULL) {
		fprintf (stderr, "%s: Can't open %s\n", _nc_progname, source_file);
		exit (1);
	}

	if (infodump)
		dump_init(smart_defaults ? F_TERMINFO : F_LITERAL,
			  S_TERMINFO, width, debug_level);
	else if (capdump)
		dump_init(F_TCONVERT, S_TERMCAP, width, debug_level);

	/* parse entries out of the source file */
	_nc_set_source(source_file);
	_nc_read_entry_source(stdin, (char *)NULL, !smart_defaults, FALSE);

	/* do use resolution */
	if (check_only || (!infodump && !capdump))
	{
	    if (!_nc_resolve_uses())
	    {
		(void) fprintf(stderr, "There are unresolved use entries:\n");
		for_entry_list(qp)
		    if (qp->nuses)
		    {
			for (i = 0; i < qp->nuses; i++)
			{
			    (void) fputs(canonical_name((char *)qp->uses[i], (char *)NULL),
					 stderr);
			    (void) fputc(' ', stderr);
			}    
			(void) fprintf(stderr,
				       "in %s\n",
				      canonical_name(qp->tterm.term_names, (char *)NULL));
		   }
		if (!check_only)
		    exit(1);
	    }
	}

	/* length check */
	if (check_only && (capdump || infodump))
	    for_entry_list(qp)
	    {
		char	outbuf[MAX_TERMINFO_LENGTH * 2];
		int	len = fmt_entry(&qp->tterm, NULL, outbuf, TRUE, infodump);

		if (len > (infodump?MAX_TERMINFO_LENGTH:MAX_TERMCAP_LENGTH))
		    	    (void) fprintf(stderr,
			   "warning: resolved %s entry is %d bytes long\n",
			   canonical_name(qp->tterm.term_names, (char *)NULL),
			   len);
	    }

	/* write or dump all entries */
	if (!check_only)
	    if (!infodump && !capdump)
	    {
		_nc_set_type((char *)NULL);
		for_entry_list(qp)
		    _nc_write_entry(&qp->tterm);
	    }
	    else
	    {
		bool	trailing_comment = FALSE;
		int	c, oldc = '\0';

		for_entry_list(qp)
		{
		    int	j = qp->cend - qp->cstart;

		    (void) fseek(stdin, qp->cstart, SEEK_SET); 
		    while (j-- )
			(void) putchar(getchar());

		    dump_entry(&qp->tterm, NULL);
		    for (j = 0; j < qp->nuses; j++)
			if (infodump)
			    (void) printf("use=%s,", (char *)(qp->uses[j]));
			else
			    (void) printf("tc=%s:", (char *)(qp->uses[j]));
		    (void) putchar('\n');
		}
		(void) fseek(stdin, _nc_tail->cend, SEEK_SET);
		while ((c = getchar()) != EOF)
		{
		    if (oldc == '\n' && c == '#')
			trailing_comment = TRUE;
		    if (trailing_comment)
			putchar(c);
		    oldc = c;
		}
	    }

	return(0);
}

