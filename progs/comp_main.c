
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
#define NCURSES_VERSION "1.9.3"

#include <string.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
# if HAVE_LIBC_H
#include <libc.h>
# endif
#endif

#include "tic.h"
#include "term.h"
#include "dump_entry.h"
#include "term_entry.h"

char	*progname;

static int	width = 60;
static int	infodump = 0;		/* running as captoinfo? */
static int	capdump = 0;		/* running as infotocap? */
static char	*source_file = "terminfo";
static char	*usage_string = "tic [-v[n]] source-file\n";
static char	check_only = 0;

int main (int argc, char *argv[])
{
int	i, debug_level = 0;
int	argflag = FALSE, smart_defaults = TRUE;
char    *termcap;
ENTRY	*qp;

	progname = argv[0];
	infodump = (strcmp(progname, "captoinfo") == 0);
	capdump = (strcmp(progname, "infotocap") == 0);

	for (i = 1; i < argc; i++) {
	    	if (argv[i][0] == '-') {
			switch (argv[i][1]) {
		    	case 'c':
				check_only = 1;
				break;
		    	case 'v':
				debug_level = argv[i][2] ? atoi(&argv[i][2]):1;
				_tracing = (1 << debug_level) - 1;
				break;
		    	case 'I':
				infodump = 1;
				break;
		    	case 'C':
				capdump = 1;
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
				err_abort("Unknown option. Usage is:\n\t%s",
				        usage_string);
			}
	    	} else if (argflag) {
			err_abort("Too many file names.  Usage is:\n\t%s\n",
				usage_string);
		} else {
			argflag = TRUE;
			source_file = argv[i];
	    	}
	}

	/* captoinfo's no-argument case */
	if (argflag == FALSE && infodump == 1) {
		termcap = "/etc/termcap";
		if ((termcap = getenv("TERMCAP")) != NULL) {
			if (access(termcap, F_OK) == 0) {
				/* file exists */
				source_file = termcap;
			}
		}
	}

	if (freopen(source_file, "r", stdin) == NULL) {
	    	err_abort("Can't open %s", source_file);
	}

	make_hash_table(info_table, info_hash_table);
	make_hash_table(cap_table, cap_hash_table);
	if (infodump)
		dump_init(smart_defaults ? F_TERMINFO : F_LITERAL,
			  S_TERMINFO, width, debug_level);
	else if (capdump)
		dump_init(F_TCONVERT, S_TERMCAP, width, debug_level);

	/* parse entries out of the source file */
	set_source(source_file);
	read_entry_source(stdin, (char *)NULL, !smart_defaults);

	/* do use resolution */
	if (check_only || (!infodump && !capdump))
	{
	    if (!resolve_uses())
	    {
		(void) fprintf(stderr, "There are unresolved use entries:\n");
		for_entry_list(qp)
		    if (qp->nuses)
		    {
			(void) fputs(qp->tterm.term_names, stderr);
			(void) fputc('\n', stderr);
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
		int	len = fmt_entry(&qp->tterm, NULL, outbuf, TRUE);

		if (len > (infodump?MAX_TERMINFO_LENGTH:MAX_TERMCAP_LENGTH))
		    	    (void) fprintf(stderr,
			   "warning: resolved %s entry string table is %d bytes long\n",
			   canonical_name(qp->tterm.term_names, (char *)NULL),
			   len);
	    }

	/* write or dump all entries */
	if (!check_only)
	    if (!infodump && !capdump)
	    {
		set_type((char *)NULL);
		for_entry_list(qp)
		    write_entry(&qp->tterm);
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
			    (void) printf("use=%s,", qp->uses[j]);
			else
			    (void) printf("tc=%s:", qp->uses[j]);
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

