
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
 *	tic.c --- Main program for terminfo compiler
 *			by Eric S. Raymond
 *
 */

#include <progs.priv.h>

#include <string.h>

#include "term.h"
#include "tic.h"
#include "dump_entry.h"
#include "term_entry.h"

char	*_nc_progname;

static bool immedhook(ENTRY *ep)
/* write out entries with no use capabilities immediately to save storage */
{
#ifndef HAVE_BIG_CORE
    /*
     * This is strictly a core-economy kluge.  The really clean way to handle
     * compilation is to slurp the whole file into core and then do all the
     * name-collision checks and entry writes in one swell foop.  But the
     * terminfo master file is large enough that some core-poor systems swap
     * like crazy when you compile it this way...there have been reports of
     * this process taking *three hours*, rather than the twenty seconds or
     * less typical on my development box.  
     *
     * So.  This hook *immediately* writes out the referenced entry if it
     * has no use capabilities.  The compiler main loop refrains from
     * adding the entry to the in-core list when this hook fires.  If some
     * other entry later needs to reference an entry that got written
     * immediately, that's OK; the resolution code will fetch it off disk
     * when it can't find it in core.
     *
     * Name collisions will still be detected, just not as cleanly.  The
     * write_entry() code complains before overwriting an entry that
     * postdates the time of tic's first call to write_entry(),  Thus
     * it will complain about overwriting entries newly made during the
     * tic run, but not about overwriting ones that predate it.
     *
     * The reason this is a hook, and not in line with the rest of the
     * compiler code, is that the support for termcap fallback cannot assume
     * it has anywhere to spool out these entries!  
     *
     * The _nc_set_type() call here requires a compensating one in 
     * _nc_parse_entry().
     *
     * If you define HAVE_BIG_CORE, you'll disable this kluge.  This will
     * make tic a bit faster (because the resolution code won't have to do
     * disk I/O nearly as often).
     */
    if (ep->nuses == 0)
    {
	int	oldline = _nc_curr_line;

	_nc_set_type(_nc_first_name(ep->tterm.term_names));
	_nc_curr_line = ep->startline;
	_nc_write_entry(&ep->tterm);
	_nc_curr_line = oldline;
	free(ep->tterm.str_table);
	return(TRUE);
    }
    else
#endif /* HAVE_BIG_CORE */
	return(FALSE);
}

static void put_translate(int c)
/* emit a comment char, translating terminfo names to termcap names */
{
    static bool in_name = FALSE;
    static char namebuf[132], suffix[132], *sp;

    if (!in_name)
    {
	if (c == '<')
	{
	    in_name = TRUE;
	    sp = namebuf;
	}
	else
	    putchar(c);
    }
    else if (c == '\n' || c == '@')
    {
	*sp++ = '\0';
	(void) putchar('<');
	(void) fputs(namebuf, stdout);
	putchar(c);
	in_name = FALSE;
    }
    else if (c != '>')
	*sp++ = c;
    else		/* ah! candidate name! */
    {
	char	*up, *tp;

	*sp++ = '\0';
	in_name = FALSE;

	suffix[0] = '\0';
	if ((up = strchr(namebuf, '#'))
		|| (up = strchr(namebuf, '=')) 
		|| ((up = strchr(namebuf, '@')) && up[1] == '>'))
	{
	    (void) strcpy(suffix, up);
	    *up = '\0';
	}

	if ((tp = nametrans(namebuf)) != (char *)NULL)
	{
	    (void) putchar(':');
	    (void) fputs(tp, stdout);
	    (void) fputs(suffix, stdout);
	    (void) putchar(':');
	}
	else
	{
	    /* couldn't find a translation, just dump the name */
	    (void) putchar('<');
	    (void) fputs(namebuf, stdout);
	    (void) fputs(suffix, stdout);
	    (void) putchar('>');
	}

    }
}

int main (int argc, char *argv[])
{
static	const	char usage_string[] = "[-c] [-v[n]] [-ICNRrw1] source-file\n";

int	i, debug_level = 0;
int	argflag = FALSE, smart_defaults = TRUE;
char    *termcap;
ENTRY	*qp;

int	width = 60;
bool	infodump = FALSE;	/* running as captoinfo? */
bool	capdump = FALSE;	/* running as infotocap? */
bool	forceresolve = FALSE;	/* force resolution */
char	*tversion = (char *)NULL;
char	*source_file = "terminfo";
bool	check_only = FALSE;

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
				check_only = TRUE;
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
			case 'R':
				tversion = &argv[i][2];
				break;
			case 'r':
				forceresolve = TRUE;
				break;
	    	    	case 'w':
				width = argv[i][2]  ?  atoi(&argv[i][2])  :  1;
				break;
		    	case 'V':
				(void) fputs(NCURSES_VERSION, stdout);
				putchar('\n');
				exit(0);
		    	case '1':
		    		width = 0;
		    		break;
		    	default:
				fprintf (stderr, 
					"%s: Unknown option. Usage is:\n\t%s",
					_nc_progname,
				        usage_string);
				exit(1);
			}
	    	} else if (argflag) {
			fprintf (stderr, 
				"%s: Too many file names.  Usage is:\n\t%s %s",
				_nc_progname,
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
				"%s: File name needed.  Usage is:\n\t%s %s",
				_nc_progname,
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
		dump_init(tversion,
			  smart_defaults ? F_TERMINFO : F_LITERAL,
			  S_TERMINFO, width, debug_level);
	else if (capdump)
		dump_init(tversion,
			  F_TERMCAP, S_TERMCAP, width, debug_level);

	/* parse entries out of the source file */
	_nc_set_source(source_file);
	_nc_read_entry_source(stdin, (char *)NULL,
			      !smart_defaults, FALSE,
			      (check_only || infodump || capdump) ? NULLHOOK : immedhook);

	/* do use resolution */
	if (check_only || (!infodump && !capdump) || forceresolve)
	    if (!_nc_resolve_uses() && !check_only)
		exit(1);

	/* length check */
	if (check_only && (capdump || infodump))
	    for_entry_list(qp)
	    {
		char	outbuf[MAX_TERMINFO_LENGTH * 2];
		int	len = fmt_entry(&qp->tterm, NULL,outbuf,TRUE,infodump);

		if (len > (infodump?MAX_TERMINFO_LENGTH:MAX_TERMCAP_LENGTH))
		    	    (void) fprintf(stderr,
			   "warning: resolved %s entry is %d bytes long\n",
			   _nc_first_name(qp->tterm.term_names),
			   len);
	    }

	/* write or dump all entries */
	if (!check_only)
	    if (!infodump && !capdump)
	    {
		for_entry_list(qp)
		{
		    _nc_set_type(_nc_first_name(qp->tterm.term_names));
		    _nc_curr_line = qp->startline;
		    _nc_write_entry(&qp->tterm);
		}
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
			if (infodump)
			    (void) putchar(getchar());
			else
			    put_translate(getchar());

		    dump_entry(&qp->tterm, NULL);
		    for (j = 0; j < qp->nuses; j++)
			dump_uses((char *)(qp->uses[j]), infodump);
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

