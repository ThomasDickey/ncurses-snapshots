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
 *	comp_parse.c -- parser driver loop and use handling.
 *
 *	read_entry_source(FILE *, literal)
 *	resolve_uses(void)
 *	free_entries(void)
 *
 *	Use this code by calling read_entry_source() on as many source
 *	files as you like (either terminfo or termcap syntax).  If you
 *	want use-resolution, call resolve_uses().  To free the list storage,
 *	do free_entries().
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tic.h"
#include "term.h"
#include "term_entry.h"

/****************************************************************************
 *
 * Debugging support
 *
 ****************************************************************************/

static char *first_name(TERMTYPE *tp)
/* get the first name of the given terminal */
{
    static char	buf[NAMESIZE], *cp;

    (void) strcpy(buf, tp->term_names);

    cp = strchr(buf, '|');
    if (cp)
	*cp = '\0';

    return(buf);
}

/****************************************************************************
 *
 * Entry queue handling
 *
 ****************************************************************************/
/*
 *  The entry list is a doubly linked list with NULLs terminating the lists:
 *
 *	  ---------   ---------   ---------
 *	  |       |   |       |   |       |   offset
 *        |-------|   |-------|   |-------|
 *	  |   ----+-->|   ----+-->|  NULL |   next
 *	  |-------|   |-------|   |-------|
 *	  |  NULL |<--+----   |<--+----   |   last
 *	  ---------   ---------   ---------
 *	      ^                       ^
 *	      |                       |
 *	      |                       |
 *	     head                    tail
 */

ENTRY *head, *tail;

static void enqueue(ENTRY *ep)
/* add an entry to the in-core list */
{
	ENTRY	*newp = (ENTRY *)malloc(sizeof(ENTRY));

	if (newp == NULL)
	    err_abort("Out of memory");

	(void) memcpy(newp, ep, sizeof(ENTRY));

	newp->last = tail;
	tail = newp;

	newp->next = (ENTRY *)NULL;
	if (newp->last)
	    newp->last->next = newp;
}


static void check_name(char *name)
/* abort on illegal name */
{
    if (!isalnum(name[0])) {
	fprintf(stderr, "compile: Line %d: Illegal terminal name - '%s'\n",
		curr_line, name);
	fprintf(stderr,	"Terminal names must start with letter or digit\n");
	exit(1);
    }
}

void read_entry_source(FILE *fp, int literal)
/* slurp all entries in the given file into core */
{
    ENTRY	thisentry;

    for (reset_input(fp); parse_entry(&thisentry, literal) != ERR; )
    {
	check_name(thisentry.tterm.term_names);
	enqueue(&thisentry);
    }

    /* set up the head pointer */
    for (head = tail; head->last; head = head->last)
	continue;

    DEBUG(1, ("head = %s", head->tterm.term_names));
    DEBUG(1, ("tail = %s", tail->tterm.term_names));
}

bool entry_match(char *n1, char *n2)
/* do any of the aliases in a pair of terminal names match? */
{
    char	*pstart, *qstart, *pend, *qend;
    char	nc1[NAMESIZE+1], nc2[NAMESIZE+1];

    if (strchr(n1, '|') == NULL)
    {
	(void) strcpy(nc1, n1);
	(void) strcat(nc1, "|");
	n1 = nc1;
    }

    if (strchr(n2, '|') == NULL)
    {
	(void) strcpy(nc2, n2);
	(void) strcat(nc2, "|");
	n2 = nc2;
    }

    for (pstart = n1; (pend = strchr(pstart, '|')); pstart = pend + 1)
	for (qstart = n2; (qend = strchr(qstart, '|')); qstart = qend + 1)
	    if ((pend-pstart == qend-qstart)
	     && memcmp(pstart, qstart, (size_t)(pend-pstart)) == 0)
		return(TRUE);

    	return(FALSE);
}

int resolve_uses(void)
/* try to resolve all use capabilities */
{
    ENTRY	*qp, *rp;
    bool	keepgoing;
    int		unresolved, multiples;

    do {
	keepgoing = FALSE;

	for_entry_list(qp)
	    if (qp->nuses > 0)
	    {
		char	*lookfor = qp->uses[qp->nuses - 1];

		DEBUG(2, ("%s: attempting resolution",first_name(&qp->tterm)));

		/* first, try to resolve from in-core records */
		for_entry_list(rp)
		    if (rp != qp
			&& name_match(rp->tterm.term_names, lookfor)
			&& rp->nuses == 0)
		    {
			keepgoing = TRUE;
			DEBUG(1, ("%s: resolving use=%s (in core)",
				  first_name(&qp->tterm), lookfor));
			merge_entry(&qp->tterm, &rp->tterm);
			qp->nuses--;
		    }

		/* if that didn't work, try to merge in a compiled entry */
		if (!keepgoing)
		{
		    TERMTYPE	thisterm;

		    if (read_entry(lookfor, &thisterm) == OK)
		    {
			keepgoing = TRUE;
			merge_entry(&qp->tterm, &thisterm);
			DEBUG(1, ("%s: resolving use=%s (compiled)",
				  first_name(&qp->tterm), lookfor));
			qp->nuses--;
		    }
		}
	    }

    } while
	(keepgoing);

    unresolved = 0;
    for_entry_list(qp)
    {
	int	i;

	for (i = 0; i < BOOLCOUNT; i++) {
	    if (qp->tterm.Booleans[i] == CANCELLED_BOOLEAN)
		qp->tterm.Booleans[i] = FALSE;
	}

	for (i = 0; i < NUMCOUNT; i++) {
	    if (qp->tterm.Numbers[i] == CANCELLED_NUMERIC)
		qp->tterm.Numbers[i] = -1;
	}

	for (i = 0; i < STRCOUNT; i++) {
	    if (qp->tterm.Strings[i] == CANCELLED_STRING)
		qp->tterm.Strings[i] = (char *)NULL;
	}


	if (qp->nuses)
	    unresolved++;
    }

    multiples = 0;
    for_entry_list(qp)
    {
	int matchcount = 0;

	for_entry_list(rp)
	    if (entry_match(qp->tterm.term_names, rp->tterm.term_names))
	    {
		if (matchcount == 2)
		{
		    (void) fprintf(stderr, "Name collision:\n\t%s\n",
			   qp->tterm.term_names);
		    multiples++;
		}
		if (matchcount >= 2)
		    (void) fprintf(stderr, "\t%s\n", rp->tterm.term_names);
		matchcount++;
	    }
    }

    return(unresolved == 0 && multiples == 0);
}

void free_entries(void)
/* free the allocated storage consumed by list entries */
{
    ENTRY	*ep, *next;

    for (ep = head; ep; ep = next)
    {
	/*
	 * This conditional lets us disconnect storage from the list.
	 * To do this, copy an entry out of the list, then null out
	 * the string-table member in the original.
	 */
	if (ep->tterm.str_table)
	    free(ep->tterm.str_table);

	next = ep->next;

	free(ep);
    }
}
