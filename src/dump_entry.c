#include <stdlib.h>
#include <sys/param.h>		/* for MAXPATHLEN */
#include <string.h>
#include <ctype.h>
#include "tic.h"
#include "terminfo.h"
#include "dump_entry.h"
#include "termsort.c"		/* this C file is generated */

#define INDENT	8

static int outform;		/* output format to use */
static int sortmode;		/* sort mode to use */
static int width = 60;		/* max line width for listings */
static int tracelevel;		/* level of debug output */

/* indirection pointers for implementing sort and display modes */
static int *bool_indirect, *num_indirect, *str_indirect;
static char **bool_names, **num_names, **str_names;

static char *separator, *trailer;

void dump_init(int mode, int sort, int twidth, int trace)
/* set up for entry display */
{
    width = twidth;
    tracelevel = trace;

    /* implement display modes */
    switch (outform = mode)
    {
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

    case F_TCONVERT:
    case F_TERMCAP:
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

char *expand(unsigned char *str)
{
char		buffer[1024];
int		bufp;
unsigned char	*ptr;

    	bufp = 0;
    	ptr = str;
    	while (*str) {
		if (*str == '\033') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'E';
		} else if ((*str < '\177') && (*str >= ' '))
		    	buffer[bufp++] = *str;
		else {
		    	sprintf(&buffer[bufp], "\\%03o", *str);
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
		return (cur_type->Booleans[index] == TRUE) ? TRUE : FAIL;

    	case NUMBER:
		return(cur_type->Numbers[index] != -1) ? TRUE : FAIL;

    	case STRING:
		return(cur_type->Strings[index] != (char *)NULL) ? TRUE : FAIL;
    	}

    	return(FALSE);	/* pacify compiler */
}

void dump_entry(TERMTYPE *tterm, int (*pred)(int type, int index))
/* dump a single entry */
{
int	i, j;
int	column;
char	buffer[MAXPATHLEN];
int	predval;

    	if (pred == NULL) {
		cur_type = tterm;
		pred = dump_predicate;
    	}

    	printf("%s,", tterm->term_names);
    	(void) fputs(trailer, stdout);
    	column = INDENT;

    	for (j=0; j < BOOLCOUNT; j++) {
		if (sortmode == S_NOSORT)
		    	i = j;
		else
	    		i = bool_indirect[j];

		if (outform == F_TERMCAP && !bool_from_termcap[i])
	    		continue;
		else if ((outform == F_TERMINFO || outform == F_VARIABLE)
		 	&& bool_names[i][0] == 'O' && bool_names[i][1] == 'T')
	    		continue;

		predval = pred(BOOLEAN, i);
		if (predval != FAIL) {
	    		(void) strcpy(buffer, bool_names[i]);
	    		if (!predval)
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
	else if ((outform == F_TERMINFO || outform == F_VARIABLE)
		 && num_names[i][0] == 'O' && num_names[i][1] == 'T')
	    continue;

	predval = pred(NUMBER, i);
	if (predval != FAIL) {
	    if (tterm->Numbers[i] == -1)
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
	else if ((outform == F_TERMINFO || outform == F_VARIABLE)
		 && str_names[i][0] == 'O' && str_names[i][1] == 'T')
	    continue;

	predval = pred(STRING, i);
	if (predval != FAIL) {
	    if (tterm->Strings[i] == (char *)NULL)
		sprintf(buffer, "%s@", str_names[i]);
	    else
		sprintf(buffer, "%s=%s", str_names[i], expand(tterm->Strings[i]));
	    (void) strcat(buffer, separator);
	    expand((unsigned char *)buffer);
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
	else if ((outform == F_TERMINFO || outform == F_VARIABLE)
		 && bool_names[i][0] == 'O' && bool_names[i][1] == 'T')
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
	else if ((outform == F_TERMINFO || outform == F_VARIABLE)
		 && num_names[i][0] == 'O' && num_names[i][1] == 'T')
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
	else if ((outform == F_TERMINFO || outform == F_VARIABLE)
		 && str_names[i][0] == 'O' && str_names[i][1] == 'T')
	    continue;

	(*hook)(STRING, i, str_names[i]);
    }
}

