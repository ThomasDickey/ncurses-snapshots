
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


#include <stdlib.h>
#include <sys/param.h>		/* for MAXPATHLEN */
#include <string.h>
#include <ctype.h>
#include "tic.h"
#include "term.h"
#include "dump_entry.h"
#include "termsort.c"		/* this C file is generated */
#include "parametrized.h"	/* so is this */

#define INDENT	8

static int outform;		/* output format to use */
static int sortmode;		/* sort mode to use */
static int width = 60;		/* max line width for listings */
static int tracelevel;		/* level of debug output */

/* indirection pointers for implementing sort and display modes */
static int *bool_indirect, *num_indirect, *str_indirect;
static char **bool_names, **num_names, **str_names;

static unsigned char *separator, *trailer;

#define OBSOLETE(n) (n[0] == 'O' && n[1] == 'T')

void dump_init(int mode, int sort, int twidth, int trace)
/* set up for entry display */
{
    width = twidth;
    tracelevel = trace;

    /* implement display modes */
    switch (outform = mode)
    {
    case F_LITERAL:
    case F_TERMINFO:
	bool_names = boolnames;
	num_names = numnames;
	str_names = strnames;
	separator = ", ";
	trailer = "\n\t";
	break;

    case F_VARIABLE:
	bool_names = boolfnames;
	num_names = numfnames;
	str_names = strfnames;
	separator = ", ";
	trailer = "\n\t";
	break;

    case F_TERMCAP:
    case F_TCONVERT:
    case F_TCONVERR:
	bool_names = boolcodes;
	num_names = numcodes;
	str_names = strcodes;
	separator = ":";
	trailer = "\\\n\t:";
	break;
    }

    /* implement sort modes */
    switch(sortmode = sort)
    { 
    case S_NOSORT:
	if (trace)
	    (void) fprintf(stderr,
			   "%s: sorting by term structure order\n", progname);
	break;

    case S_TERMINFO:
	if (trace)
	    (void) fprintf(stderr,
			   "%s: sorting by terminfo name order\n", progname);
	bool_indirect = bool_terminfo_sort;
	num_indirect = num_terminfo_sort;
	str_indirect = str_terminfo_sort;
	break;

    case S_VARIABLE:
	if (trace)
	    (void) fprintf(stderr,
			   "%s: sorting by C variable order\n", progname);
	bool_indirect = bool_variable_sort;
	num_indirect = num_variable_sort;
	str_indirect = str_variable_sort;
	break;

    case S_TERMCAP:
	if (trace)
	    (void) fprintf(stderr,
			   "%s: sorting by termcap name order\n", progname);
	bool_indirect = bool_termcap_sort;
	num_indirect = num_termcap_sort;
	str_indirect = str_termcap_sort;
	break;
    }

    if (trace)
	(void) fprintf(stderr,
		       "%s: width = %d, outform = %d\n",
		       progname, width, outform);
}

char *expand(unsigned char *srcp)
{
static char	buffer[1024];
int		bufp;
unsigned char	*ptr, *str = srcp;
bool		islong = (strlen(srcp) > 3);

    	bufp = 0;
    	ptr = str;
    	while (*str) {
		if (*str == '\033') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'E';

		}
		else if (*str == '\\' && (str == srcp || str[-1] != '^')) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = '\\';
		}
		else if (*str == '^' && (str == srcp || str[-1] != '%')) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = '^';
		}
		else if (isprint(*str) && (*str != ',' && *str != ':'))
		    	buffer[bufp++] = *str;
		else if (*str == '\r' && (islong || (strlen(srcp) > 2 && str[1] == '\0'))) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'r';
		}
		else if (*str == '\n' && islong) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'n';
		}
		else if (*str < 0x80 && iscntrl(*str) && *str != '\\' && (!islong || isdigit(str[1])))
		{
			(void) sprintf(&buffer[bufp], "^%c", *str + '@');
			bufp += 2;
		}
		else
		{
			(void) sprintf(&buffer[bufp], "\\%03o", *str);
			bufp += 4;
		}

		str++;
    	}

    	buffer[bufp] = '\0';
    	return(buffer);
}

static TERMTYPE	*cur_type;

static int dump_predicate(int type, int index)
/* predicate function to use for ordinary decompilation */
{
    	switch(type) {
    	case BOOLEAN:
		return (cur_type->Booleans[index] == FALSE)
		    ? FAIL : cur_type->Booleans[index];

    	case NUMBER:
		return (cur_type->Numbers[index] == ABSENT_NUMERIC)
		    ? FAIL : cur_type->Numbers[index];

    	case STRING:
		return (cur_type->Strings[index] != ABSENT_STRING)
		    ? TRUE : FAIL;
    	}

    	return(FALSE);	/* pacify compiler */
}

static void set_obsolete_termcaps(TERMTYPE *tp);

void dump_entry(TERMTYPE *tterm, int (*pred)(int type, int index))
/* dump a single entry */
{
int	i, j;
int	column;
char	buffer[1024];
int	predval;

    if (pred == NULL) {
	cur_type = tterm;
	pred = dump_predicate;
    }

    if (outform==F_TERMCAP || outform==F_TCONVERT || outform==F_TCONVERR)
	set_obsolete_termcaps(tterm);

    (void) fputs(tterm->term_names, stdout);
    (void) fputs(separator, stdout);
    (void) fputs(trailer, stdout);
    column = INDENT;

    for (j=0; j < BOOLCOUNT; j++) {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = bool_indirect[j];

	if (outform == F_TERMCAP && !bool_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(bool_names[i]) && outform != F_LITERAL))
	    continue;

	predval = pred(BOOLEAN, i);
	if (predval != FAIL) {
	    (void) strcpy(buffer, bool_names[i]);
	    if (predval <= 0)
		(void) strcat(buffer, "@");
	    (void) strcat(buffer, separator);
	    if (column > INDENT &&  column + strlen(buffer) > width) {
		(void) fputs(trailer, stdout);
		column = INDENT;
	    }
	    (void) fputs(buffer, stdout);	
	    column += strlen(buffer);
	}
    }

    if (column != INDENT)
    {
	(void) fputs(trailer, stdout);
	column = INDENT;
    }

    for (j=0; j < NUMCOUNT; j++) {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = num_indirect[j];

	if (outform == F_TERMCAP && !num_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(num_names[i]) && outform != F_LITERAL))
	    continue;

	predval = pred(NUMBER, i);
	if (predval != FAIL) {
	    if (tterm->Numbers[i] < 0)
		sprintf(buffer, "%s@", num_names[i]);
	    else
		sprintf(buffer, "%s#%d", num_names[i], tterm->Numbers[i]);
	    (void) strcat(buffer, separator);
	    if (column > INDENT &&  column + strlen(buffer) > width)
	    {
		(void) fputs(trailer, stdout);
		column = INDENT;
	    }
	    (void) fputs(buffer, stdout);
	    column += strlen(buffer);
	}
    }
    if (column != INDENT)
    {
	(void) fputs(trailer, stdout);
	column = INDENT;
    }

    for (j=0; j < STRCOUNT; j++) {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = str_indirect[j];

	if (outform == F_TERMCAP && !str_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(str_names[i]) && outform != F_LITERAL))
	    continue;

	predval = pred(STRING, i);
	if (predval != FAIL) {
	    if (tterm->Strings[i] == (char *)NULL || tterm->Strings[i] == CANCELLED_STRING)
		sprintf(buffer, "%s@", str_names[i]);
	    else if ((outform == F_TCONVERT || outform == F_TCONVERR))
	    {
		char *srccap = expand(tterm->Strings[i]);
		char *cv = infotocap(str_names[i], srccap, parametrized[i]);

		if (cv == (char *)NULL)
		{
		    if (outform == F_TCONVERR)
			sprintf(buffer, "%s=!!! %s WILL NOT CONVERT !!!", str_names[i], srccap);
		    else
			sprintf(buffer, "..%s=%s", str_names[i], srccap);
		}
		else
		    sprintf(buffer, "%s=%s", str_names[i], cv);
	    }
	    else
		sprintf(buffer, "%s=%s", str_names[i], expand(tterm->Strings[i]));
	    (void) strcat(buffer, separator);
	    if (column > INDENT  &&  column + strlen(buffer) > width)
	    {
		(void) fputs(trailer, stdout);
		column = INDENT;
	    }
	    (void) fputs(buffer, stdout);
	    column += strlen(buffer);
	}
    }
}

void compare_entry(void (*hook)(int t, int i, char *name))
/* compare two entries */
{
    int	i, j;

    (void) fputs("    comparing booleans.\n", stdout);
    for (j=0; j < BOOLCOUNT; j++)
    {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = bool_indirect[j];

	if (outform == F_TERMCAP && !bool_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(bool_names[i]) && outform != F_LITERAL))
	    continue;

	(*hook)(BOOLEAN, i, bool_names[i]);
    }

    (void) fputs("    comparing numbers.\n", stdout);
    for (j=0; j < NUMCOUNT; j++)
    {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = num_indirect[j];

	if (outform == F_TERMCAP && !num_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(num_names[i]) && outform != F_LITERAL))
	    continue;

	(*hook)(NUMBER, i, num_names[i]);
    }

    (void) fputs("    comparing strings.\n", stdout);
    for (j=0; j < STRCOUNT; j++)
    {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = str_indirect[j];

	if (outform == F_TERMCAP && !str_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(str_names[i]) && outform != F_LITERAL))
	    continue;

	(*hook)(STRING, i, str_names[i]);
    }
}

#define NOTSET(s)	((s) == (char *)NULL)

/*
 * This bit of legerdemain turns all the terminfo variable names into
 * references to locations in the arrays Booleans, Numbers, and Strings ---
 * precisely what's needed.
 */
#undef CUR
#define CUR tp->

static void set_obsolete_termcaps(TERMTYPE *tp)
{
#include "capdefaults.c"
}
