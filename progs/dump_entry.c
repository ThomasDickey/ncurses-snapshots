
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
#define __INTERNAL_CAPS_VISIBLE
#include "term.h"
#include "dump_entry.h"
#include "termsort.c"		/* this C file is generated */
#include "parametrized.h"	/* so is this */

#define INDENT			8

static int outform;		/* output format to use */
static int sortmode;		/* sort mode to use */
static int width = 60;		/* max line width for listings */
static int tracelevel;		/* level of debug output */

/* indirection pointers for implementing sort and display modes */
static const int *bool_indirect, *num_indirect, *str_indirect;
static char * const *bool_names, * const *num_names, * const *str_names;

static char *separator, *trailer;

#define OBSOLETE(n) (n[0] == 'O' && n[1] == 'T')

void dump_init(int mode, int sort, int twidth, int traceval)
/* set up for entry display */
{
    width = twidth;
    tracelevel = traceval;

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
	if (traceval)
	    (void) fprintf(stderr,
			   "%s: sorting by term structure order\n", _nc_progname);
	break;

    case S_TERMINFO:
	if (traceval)
	    (void) fprintf(stderr,
			   "%s: sorting by terminfo name order\n", _nc_progname);
	bool_indirect = bool_terminfo_sort;
	num_indirect = num_terminfo_sort;
	str_indirect = str_terminfo_sort;
	break;

    case S_VARIABLE:
	if (traceval)
	    (void) fprintf(stderr,
			   "%s: sorting by C variable order\n", _nc_progname);
	bool_indirect = bool_variable_sort;
	num_indirect = num_variable_sort;
	str_indirect = str_variable_sort;
	break;

    case S_TERMCAP:
	if (traceval)
	    (void) fprintf(stderr,
			   "%s: sorting by termcap name order\n", _nc_progname);
	bool_indirect = bool_termcap_sort;
	num_indirect = num_termcap_sort;
	str_indirect = str_termcap_sort;
	break;
    }

    if (traceval)
	(void) fprintf(stderr,
		       "%s: width = %d, outform = %d\n",
		       _nc_progname, width, outform);
}

/* this deals with differences over whether 0x7f and 0x80..0x9f are controls */
#define CHAR_OF(s) (*(unsigned char *)s)
#define REALCTL(s) (CHAR_OF(s) < 127 && iscntrl(CHAR_OF(s)))

char *expand(char *srcp)
{
static char	buffer[1024];
int		bufp;
char		*ptr, *str = srcp;
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
		else if (REALCTL(str) && *str != '\\' && (!islong || isdigit(str[1])))
		{
			(void) sprintf(&buffer[bufp], "^%c", (0xff & *str) + '@');
			bufp += 2;
		}
		else
		{
			(void) sprintf(&buffer[bufp], "\\%03o", 0xff & *str);
			bufp += 4;
		}

		str++;
    	}

    	buffer[bufp] = '\0';
    	return(buffer);
}

char *canonical_name(char *ptr, char *buf)
/* extract the terminal type's primary name */
{
    static char	mycopy[NAMESIZE];
    char	*bp;

    if (buf == (char *)NULL)
	buf = mycopy;

    (void) strcpy(buf, ptr);
    if ((bp = strchr(buf, '|')) != (char *)NULL)
	*bp = '\0';

    return(buf);
}

static TERMTYPE	*cur_type;

static int dump_predicate(int type, int idx)
/* predicate function to use for ordinary decompilation */
{
    	switch(type) {
    	case BOOLEAN:
		return (cur_type->Booleans[idx] == FALSE)
		    ? FAIL : cur_type->Booleans[idx];

    	case NUMBER:
		return (cur_type->Numbers[idx] == ABSENT_NUMERIC)
		    ? FAIL : cur_type->Numbers[idx];

    	case STRING:
		return (cur_type->Strings[idx] != ABSENT_STRING)
		    ? (int)TRUE : FAIL;
    	}

    	return(FALSE);	/* pacify compiler */
}

static void set_obsolete_termcaps(TERMTYPE *tp);

static bool termcap_form(int f)
/* return true if F indicates a termcap output format */
{
    return (f == F_TERMCAP || f == F_TCONVERT || f == F_TCONVERR);
}

int fmt_entry(TERMTYPE *tterm,
			   int (*pred)(int type, int idx),
			   char *outbuf, bool suppress_untranslatable,
			   bool infodump)
{
int	i, j;
int	column;
char    buffer[MAX_TERMINFO_LENGTH];
int	predval, len = 0;

    if (pred == NULL) {
	cur_type = tterm;
	pred = dump_predicate;
    }

    (void) strcpy(outbuf, tterm->term_names);
    (void) strcat(outbuf, separator);
    (void) strcat(outbuf, trailer);
    column = INDENT;

    for (j=0; j < BOOLCOUNT; j++) {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = bool_indirect[j];

	if (termcap_form(outform) && !bool_from_termcap[i])
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
		(void) strcat(outbuf, trailer);
		column = INDENT;
	    }
	    (void) strcat(outbuf, buffer);	
	    column += strlen(buffer);
	}
    }

    if (column != INDENT)
    {
	(void) strcat(outbuf, trailer);
	column = INDENT;
    }

    for (j=0; j < NUMCOUNT; j++) {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = num_indirect[j];

	if (termcap_form(outform) && !num_from_termcap[i])
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
		(void) strcat(outbuf, trailer);
		column = INDENT;
	    }
	    (void) strcat(outbuf, buffer);
	    column += strlen(buffer);
	}
    }
    if (column != INDENT)
    {
	(void) strcat(outbuf, trailer);
	column = INDENT;
    }

    len = strlen(tterm->term_names) + 1;
    for (j=0; j < STRCOUNT; j++) {
	if (sortmode == S_NOSORT)
	    i = j;
	else
	    i = str_indirect[j];

	if (termcap_form(outform) && !str_from_termcap[i])
	    continue;
	else if ((outform == F_LITERAL || outform == F_TERMINFO || outform == F_VARIABLE)
		 && (OBSOLETE(str_names[i]) && outform != F_LITERAL))
	    continue;

	/*
	 * Some older versions of vi want rmir/smir to be defined
	 * for ich/ich1 to work.  If they're not defined, force
	 * them to be output as defined and empty.
	 */
	if (outform==F_TERMCAP || outform==F_TCONVERT || outform==F_TCONVERR)
#undef CUR
#define CUR tterm->
	    if (insert_character || parm_ich)
	    {
		if (&tterm->Strings[i] == &enter_insert_mode
		    && enter_insert_mode == ABSENT_STRING)
		{
		    (void) strcpy(buffer, "im=");
		    len++;
		    goto catenate;
		}

		if (&tterm->Strings[i] == &exit_insert_mode
		    && exit_insert_mode == ABSENT_STRING)
		{
		    (void) strcpy(buffer, "ei=");
		    len++;
		    goto catenate;
		}
	    }

	predval = pred(STRING, i);
	buffer[0] = '\0';
	if (predval != FAIL) {
	    if (tterm->Strings[i] == ABSENT_STRING || tterm->Strings[i] == CANCELLED_STRING)
		sprintf(buffer, "%s@", str_names[i]);
	    else if ((outform == F_TCONVERT || outform == F_TCONVERR))
	    {
		char *srccap = expand(tterm->Strings[i]);
		char *cv = _nc_infotocap(str_names[i], srccap,parametrized[i]);

		if (cv == (char *)NULL)
		{
		    if (outform == F_TCONVERR)
			sprintf(buffer, "%s=!!! %s WILL NOT CONVERT !!!", str_names[i], srccap);
		    else if (suppress_untranslatable)
			continue;
		    else
			sprintf(buffer, "..%s=%s", str_names[i], srccap);
		}
		else
		    sprintf(buffer, "%s=%s", str_names[i], cv);
		len += strlen(tterm->Strings[i]) + 1;
	    }
	    else
	    {
		sprintf(buffer,"%s=%s",str_names[i],expand(tterm->Strings[i]));
		len += strlen(tterm->Strings[i]) + 1;
	    }

	catenate:
	    (void) strcat(buffer, separator);
	    if (column > INDENT  &&  column + strlen(buffer) > width)
	    {
		(void) strcat(outbuf, trailer);
		column = INDENT;
	    }
	    (void) strcat(outbuf, buffer);
	    column += strlen(buffer);
	}
    }

    /*
     * kluge: trim off trailing \n\t to avoid an extra blank line
     * in infocmp -u output when there are no string differences
     */
    if ((j = strlen(outbuf)) >= 2)
    {
	if (outbuf[j-1] == '\t' && outbuf[j-2] == '\n')
	    outbuf[j-2] = '\0';
    }

#if 0
    fprintf(stderr, "term_names=%s, len=%d, strlen(outbuf)=%d, outbuf=%s\n",
	    tterm->term_names, len, strlen(outbuf), outbuf);
#endif
    /*
     * Here's where we use infodump to trigger a more stringent length check
     * for termcap-translation purposes.
     * strlen(outbuf) is the length of the raw entry, without tc= expansions,
     * and (incorrectly) counting backslash-newlines.
     * It gives an idea of which entries are deadly to even *scan past*,
     * as opposed to *use*.
     */
    return(infodump ? len : strlen(outbuf));
}

void dump_entry(TERMTYPE *tterm, int (*pred)(int type, int idx))
/* dump a single entry */
{
    int	len, critlen;
    char	*legend, outbuf[MAX_TERMINFO_LENGTH * 2];
    bool	infodump;

    if (outform==F_TERMCAP || outform==F_TCONVERT || outform==F_TCONVERR)
    {
	critlen = MAX_TERMCAP_LENGTH;
	legend = "older termcap";
	infodump = FALSE;
	set_obsolete_termcaps(tterm);
    }
    else
    {
	critlen = MAX_TERMINFO_LENGTH;
	legend = "terminfo";
	infodump = TRUE;
    }

    if ((len = fmt_entry(tterm, pred, outbuf, FALSE, infodump)) > critlen)
    {
	(void) printf("# (untranslatable capabilities removed to fit entry within %d bytes)\n",
		      critlen);
	if ((len = fmt_entry(tterm, pred, outbuf, TRUE, infodump)) > critlen)
	{
	    /*
	     * We pick on sgr because it's a nice long string capability that
	     * is really just an optimization hack.
	     */
	    char *oldsgr = set_attributes;
	    set_attributes = ABSENT_STRING; 
	    (void) printf("# (sgr removed to fit entry within %d bytes)\n",
			  critlen);
	    if ((len=fmt_entry(tterm, pred, outbuf, TRUE, infodump)) > critlen)
	    {
		(void) fprintf(stderr,
			       "warning: %s entry is %d bytes long\n",
			       canonical_name(tterm->term_names, (char *)NULL),
			       len);
		(void) printf(
			      "# WARNING: this entry, %d bytes long, may core-dump %s libraries!\n",
			      len, legend);
	    }
	    set_attributes = oldsgr;
	}
    }

    (void) fputs(outbuf, stdout);
}

void compare_entry(void (*hook)(int t, int i, const char *name))
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

	if (termcap_form(outform) && !bool_from_termcap[i])
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

	if (termcap_form(outform) && !num_from_termcap[i])
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

	if (termcap_form(outform) && !str_from_termcap[i])
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
