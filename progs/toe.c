
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
 *	toe.c --- table of entries report generator
 *
 */

#include <config.h>

#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <dirent.h>

#include "tic.h"
#include "term.h"
#include "dump_entry.h"
#include "term_entry.h"

#if HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif /* HAVE_GETOPT_H */

char	*_nc_progname;

static void typelist(int eargc, char *eargv[]);

int main (int argc, char *argv[])
{
    bool	direct_dependencies = FALSE;
    bool	invert_dependencies = FALSE;
    int		i, c, debug_level = 0;

    if ((_nc_progname = strrchr(argv[0], '/')) == NULL)
	_nc_progname = argv[0];
    else
	_nc_progname++;

    while ((c = getopt(argc, argv, "u:v:U:V")) != EOF)
	switch (c)
	{
	case 'u':
	    direct_dependencies = TRUE;
	    break;
	case 'v':
	    debug_level = atoi(optarg);
	    _nc_tracing = (1 << debug_level) - 1;
	    break;
	case 'U':
	    invert_dependencies = TRUE;
	    break;
	case 'V':
	    (void) fputs(NCURSES_VERSION, stdout);
	    exit(0);
	default:
	    (void) fprintf (stderr, "usage: toe [-uUvV] [file...]\n");
	    exit(1);
	}

    if (direct_dependencies || invert_dependencies)
    {
	if (freopen(argv[optind], "r", stdin) == NULL)
	{
	    fprintf(stderr, "%s: can't open %s\n", _nc_progname, argv[optind]);
	    exit(1);
	}

	/* parse entries out of the source file */
	_nc_set_source(argv[optind]);
	_nc_read_entry_source(stdin, (char *)NULL,
			      FALSE, FALSE,
			      NULLHOOK);
    }

    /* maybe we want a direct-dependency listing? */
    if (direct_dependencies)
    {
	ENTRY	*qp;

	for_entry_list(qp)
	    if (qp->nuses)
	    {
		int		j;

		(void) printf("%s:", _nc_first_name(&qp->tterm));
		for (j = 0; j < qp->nuses; j++)
		    (void) printf(" %s", (char *)(qp->uses[j]));
		putchar('\n');
	    }

	exit(0);
    }

    /* maybe we want a reverse-dependency listing? */
    if (invert_dependencies)
    {
	ENTRY	*qp, *rp;
	int		matchcount;

	for_entry_list(qp)
	{
	    matchcount = 0;
	    for_entry_list(rp)
	    {
		if (rp->nuses == 0)
		    continue;

		for (i = 0; i < rp->nuses; i++)
		    if (_nc_name_match(qp->tterm.term_names,(char*)rp->uses[i], "|"))
		    {
			if (matchcount++ == 0)
			    (void) printf("%s:",
					  _nc_first_name(&qp->tterm));
			(void) printf(" %s", 
				      _nc_first_name(&rp->tterm));
		    }
	    }
	    if (matchcount)
		putchar('\n');
	}

	exit(0);
    }

    /*
     * If we get this far, user wants a simple terminal type listing.
     */
    if (optind < argc)
	typelist(argc-optind, argv+optind);	/* user supplied args */
    else
    {
	char	*eargv[2];
	extern char	*getenv(const char *);

	if ((eargv[0] = getenv("TERMINFO")) == (char *)NULL)
	    eargv[0] = TERMINFO;
	eargv[1] = (char *)NULL;

	typelist(1, eargv);
    }

    exit(0);
}

static void typelist(int eargc, char *eargv[])
/* summary listing of all given terminfo directories */
{
    int	i;

    for (i = 0; i < eargc; i++)
    {
	DIR	*termdir;
	struct dirent *subdir;

	if ((termdir = opendir(eargv[i])) == (DIR *)NULL)
	{
	    (void) fprintf(stderr,
			   "%s: can't open terminfo directory %s\n",
			   _nc_progname, eargv[i]);
	    exit(1);
	}

	while ((subdir = readdir(termdir)) != NULL)
	{
	    char	buf[256];
	    DIR	*entrydir;
	    struct dirent *entry;

	    if (!strcmp(subdir->d_name, ".")
		|| !strcmp(subdir->d_name, ".."))
		continue;

	    (void) strcpy(buf, eargv[i]);
	    (void) strcat(buf, "/");
	    (void) strcat(buf, subdir->d_name);
	    (void) strcat(buf, "/");
	    chdir(buf);
	    entrydir = opendir(".");
	    while ((entry = readdir(entrydir)) != NULL)
	    {
		TERMTYPE	lterm;
		char	*cn, *desc;
		int		status;

		if (!strcmp(entry->d_name, ".")
		    || !strcmp(entry->d_name, ".."))
		    continue;

		status = _nc_read_file_entry(entry->d_name, &lterm);
		if (status == -1)
		{
		    (void) fprintf(stderr,
				   "infocmp: couldn't open terminfo directory %s.\n",
				   eargv[i]);
		    return;
		}
		else if (status == 0)
		{
		    (void) fprintf(stderr,
				   "infocmp: couldn't open terminfo file %s.\n",
				   entry->d_name);
		    return;
		}

		/* only list things once, by primary name */
		cn = canonical_name(lterm.term_names, (char *)NULL);
		if (strcmp(cn, entry->d_name))
		    continue;

		/* get a description for the type */
		if ((desc = strrchr(lterm.term_names,'|')) == (char *)NULL)
		    desc = "(No description)";
		else
		    ++desc;

		(void) printf("%-10s\t%s\n", cn, desc);
	    }
	}

	exit(0);
    }
}
