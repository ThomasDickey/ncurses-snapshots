
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
 *	infocmp.c -- decompile an entry, or compare two entries
 *		written by Eric S. Raymond
 *
 */

#include <stdlib.h>
#include <sys/param.h>		/* for MAXPATHLEN */
#include <string.h>
#include <ctype.h>
#include "terminfo.h"
#include "tic.h"
#include "dump_entry.h"

#define VERSION		"infocmp 1.0"

#define MAXTERMS	32	/* max # terminal arguments we can handle */

char *progname = "infocmp";

typedef char	path[MAXPATHLEN];

/***************************************************************************
 *
 * The following control variables, together with the contents of the
 * terminfo entries, completely determine the actions of the program.
 *
 ***************************************************************************/

static char *tname[MAXTERMS];	/* terminal type names */
static TERMTYPE term[MAXTERMS];	/* terminfo entries */
static int termcount;		/* count of terminal entries */

static int outform;		/* output format */
static int sortmode;		/* sort_mode */
static int trace;		/* trace flag for debugging */
static int mwidth = 60;

/* main comparison mode */
static int compare;
#define C_DEFAULT	0	/* don't force comparison mode */
#define C_DIFFERENCE	1	/* list differences between two terminals */
#define C_COMMON	2	/* list common capabilities */
#define C_NAND		3	/* list capabilities in neither terminal */
#define C_USEALL	4	/* generate relative use-form entry */
static bool ignorepads;		/* ignore pad prefixes when diffing */

/***************************************************************************
 *
 * Predicates for dump function
 *
 ***************************************************************************/

static int use_predicate(int type, int index)
/* predicate function to use for use decompilation */
{
TERMTYPE *tp;

	switch(type) {
	case BOOLEAN: {
	int is_set = FALSE;

		/*
		 * This assumes that multiple use entries are supposed
		 * to contribute the logical or of their boolean capabilities.
		 * This is true if we take the semantics of multiple uses to
		 * be 'each capability gets the first non-default value found
		 * in the sequence of use entries'.
		 */
		for (tp = &term[1]; tp < term + termcount; tp++)
			if (tp->Booleans[index]) {
				is_set = TRUE;
				break;
			}
			if (is_set != term->Booleans[index])
				return(!is_set);
			else
				return(FAIL);
		}

	case NUMBER: {
	int	value = -1;

		/*
		 * We take the semantics of multiple uses to be 'each
		 * capability gets the first non-default value found in the
		 * sequence of use entries.
		 */
		for (tp = &term[1]; tp < term + termcount; tp++)
			if (tp->Numbers[index] >= 0) {
				value = tp->Numbers[index];
				break;
			}

		if (value != term->Numbers[index])
			return(value != -1);
		else
			return(FAIL);
		}

		break;

	case STRING:
	{
	TERMTYPE *tp;
	char *termstr, *usestr = (char *)NULL;

	termstr = term->Strings[index];

	/*
	 * We take the semantics of multiple uses to be 'each
	 * capability gets the first non-default value found in the
	 * sequence of use entries'.
	 */
	for (tp = &term[1]; tp < term + termcount; tp++)
		if (tp->Strings[index])
		{
		usestr = tp->Strings[index];
		break;
		}

	if (usestr == (char *)NULL && termstr == (char *)NULL)
		return(FAIL);
	else if (!usestr || !termstr || strcmp(usestr, termstr) != 0)
		return(TRUE);
	else
		return(FAIL);
	break;
	}
	}
	
	return(FALSE);	/* pacify compiler */
}

static int capcmp(const char *s, const char *t)
/* compare two string capabilities */
{
    for (; *s == '\0' || *t == '\0'; s++, t++)
    {
	if (s[0] == '$' && s[1] == '<')
	{
	    for (s += 2; ; s++)
		if (isdigit(*s) || *s=='.' || *s=='*' || *s=='/'  || *s=='>')
		    continue;
	    --s;
	}

	if (t[0] == '$' && t[1] == '<')
	{
	    for (t += 2; ; t++)
		if (isdigit(*t) || *t=='.' || *t=='*' || *t=='/'  || *t=='>')
		    continue;
	    --t;
	}

	if (*s != *t)
	    return(*t - *s);
    }

    return(0);
}

static void compare_predicate(int type, int index, char *name)
/* predicate function to use for ordinary decompilation */
{
	register TERMTYPE *t1 = &term[0];
	register TERMTYPE *t2 = &term[1];
	char *s1, *s2;

	switch(type)
	{
	case BOOLEAN:
	switch(compare)
	{
	case C_DIFFERENCE:
		if (t1->Booleans[index] != t2->Booleans[index])
		(void) printf("\t%s: %c:%c.\n", 
				  name,
				  t1->Booleans[index] ? 'T' : 'F',
				  t2->Booleans[index] ? 'T' : 'F');
		break;

	case C_COMMON:
		if (t1->Booleans[index] && t2->Booleans[index])
		(void) printf("\t%s= T.\n", name);
		break;

	case C_NAND:
		if (!t1->Booleans[index] && !t2->Booleans[index])
		(void) printf("\t!%s.\n", name);
		break;
	}
	break;

	case NUMBER:
	switch(compare)
	{
	case C_DIFFERENCE:
		if (t1->Numbers[index] != t2->Numbers[index])
		(void) printf("\t%s: %d:%d.\n", 
				  name, t1->Numbers[index], t2->Numbers[index]);
		break;

	case C_COMMON:
		if (t1->Numbers[index] != -1 && t2->Numbers[index] != -1
			&& t1->Numbers[index] == t2->Numbers[index])
		(void) printf("\t%s= %d.\n", name, t1->Numbers[index]);
		break;

	case C_NAND:
		if (t1->Numbers[index] == -1 && t2->Numbers[index] == -1)
		(void) printf("\t!%s.\n", name);
		break;
	}
	break;

	case STRING:
	s1 = t1->Strings[index];
	s2 = t2->Strings[index];
	switch(compare)
	{
	case C_DIFFERENCE:
		if ((s1 || s2) && (!s1 || !s2 || capcmp(s1, s2)))
		{
		char	buf1[BUFSIZ], buf2[BUFSIZ];

		if (s1 == (char *)NULL)
			(void) strcpy(buf1, "NULL");
		else
			(void) strcpy(buf1, expand(s1));

		if (s2 == (char *)NULL)
			(void) strcpy(buf2, "NULL");
		else
			(void) strcpy(buf2, expand(s2));

		(void) printf("\t%s: '%s', '%s'.\n", name, buf1, buf2);
		}
		break;

	case C_COMMON:
		if (s1 && s2 && !capcmp(s1, s2))
			(void) printf("\t%s= '%s'.\n", name, expand(s1));
		break;

	case C_NAND:
		if (!s1 && !s2)
			(void) printf("\t!%s.\n", name);
		break;
	}
	break;
	}

}

/***************************************************************************
 *
 * Main sequence
 *
 ***************************************************************************/

extern char *optarg;
extern int optind;

int main(int argc, char *argv[])
{
	char *terminal, *firstdir, *restdir;
	path tfile[MAXTERMS];
	int saveoptind, c, i;

	if ((terminal = getenv("TERM")) == NULL)
	{
	fprintf(stderr, "infocmp: environment variable TERM not set\n");
	exit(1);
	}

	/* where is the terminfo database location going to default to? */
	if ((firstdir = getenv("TERMINFO")) == NULL)
	firstdir = SRCDIR;
	restdir = firstdir;

	while ((c = getopt(argc, argv, "dcCnlLprs:uvVw:A:B:1")) != EOF)
	switch (c)
	{
	case 'd':
		compare = C_DIFFERENCE;
		break;

	case 'c':
		compare = C_COMMON;
		break;

	case 'C':
		outform = F_TERMCAP;
		if (sortmode == S_DEFAULT)
		sortmode = S_TERMCAP;
		break;

	case 'l':
		outform = F_TERMINFO;
		break;

	case 'L':
		outform = F_VARIABLE;
		if (sortmode == S_DEFAULT)
		sortmode = S_VARIABLE;
		break;

	case 'n':
		compare = C_NAND;
		break;

	case 'p':
		ignorepads = TRUE;
		break;

	case 'r':
		outform = F_TCONVERR;
		break;

	case 's':
		if (*optarg == 'd')
		sortmode = S_NOSORT;
		else if (*optarg == 'i')
		sortmode = S_TERMINFO;
		else if (*optarg == 'l')
		sortmode = S_VARIABLE;
		else if (*optarg == 'c')
		sortmode = S_TERMCAP;
		else
		{
		fprintf(stderr, "infocmp: unknown sort mode\n");
		exit(1);
		}
		break;

	case 'u':
		compare = C_USEALL;
		break;

	case 'v':
		trace = 1;
		break;

	case 'V':
		(void) fputs(VERSION, stdout);
		exit(0);

	case 'w':
		mwidth = atoi(optarg);
		break;

	case 'A':
		firstdir = optarg;
		break;

	case 'B':
		restdir = optarg;
		break;

	case '1':
		mwidth = 0;
		break;
	}

	/* by default, sort by terminfo name */
	if (sortmode == S_DEFAULT)
	sortmode = S_TERMINFO;

	/* make sure we have at least one terminal name to work with */
	if (optind >= argc)
	argv[argc++] = terminal;

	/* if user is after a comparison, make sure we have two entries */
	if (compare != C_DEFAULT && optind >= argc - 1)
	argv[argc++] = terminal;

	/* grab the entries */
	termcount = 0;
	for (saveoptind = optind; optind < argc; optind++)
	if (termcount >= MAXTERMS)
	{
		fprintf(stderr, "infocmp: too many terminal types arguments\n");
		exit(1);
	}
	else
	{
		char	*directory = termcount ? restdir : firstdir;

		tname[termcount] = argv[optind];
		(void) sprintf(tfile[termcount], "%s/%c/%s",
			   directory,
			   *argv[optind], argv[optind]);
		if (trace)
		(void) fprintf(stderr,
				   "infocmp: reading entry %s from file %s\n",
				   argv[optind], tfile[termcount]);
		if (read_file_entry(tfile[termcount], &term[termcount]) == -1)
		{
		fprintf(stderr, "couldn't open terminfo file %s.\n",
			tfile[termcount]);
		fprintf(stderr, "The terminal you are using is not defined.\n");
		exit(1);
		}
		termcount++;
	}

	/* exactly two terminal names with no options means do -d */
	if (termcount == 2 && compare == C_DEFAULT)
	compare = C_DIFFERENCE;

	/* set up for display */
	dump_init(outform, sortmode, mwidth, trace);

	/*
	 * Here's where the real work gets done
	 */
	switch (compare)
	{
	case C_DEFAULT:
	if (trace)
		(void) fprintf(stderr,
			   "infocmp: about to dump %s\n",
			   argv[saveoptind]);
	(void) printf("#\tReconstructed via infocmp from file: %s\n",tfile[0]);
	dump_entry(&term[0], NULL);
	putchar('\n');
	break;

	case C_DIFFERENCE:
	if (trace)
		(void) fprintf(stderr, "infocmp: dumping differences\n");
	(void) printf("comparing %s to %s.\n", tname[0], tname[1]);
	compare_entry(compare_predicate);
	break;

	case C_COMMON:
	if (trace)
		(void) fprintf(stderr, "infocmp: dumping common capabilities\n");
	(void) printf("comparing %s to %s.\n", tname[0], tname[1]);
	compare_entry(compare_predicate);
	break;

	case C_NAND:
	if (trace)
		(void) fprintf(stderr, "infocmp: dumping differences\n");
	(void) printf("comparing %s to %s.\n", tname[0], tname[1]);
	compare_entry(compare_predicate);
	break;

	case C_USEALL:
	if (trace)
		(void) fprintf(stderr, "infocmp: dumping use entry\n");
	dump_entry(&term[0], use_predicate);
	putchar('\n');
	for (i = 1; i < termcount; i++)
		if (outform==F_TERMCAP || outform==F_TCONVERT || outform==F_TCONVERR)
			(void) printf("\ttc=%s,\n", tname[i]);
		else
			(void) printf("\tuse=%s,\n", tname[i]);
	break;
	}

	exit(0);
}

/* infocmp.c ends here */
