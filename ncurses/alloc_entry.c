
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
 * alloc_entry.c -- allocation functions for terminfo entries
 *
 *	_nc_init_entry()
 *	_nc_save_str()
 *	_nc_merge_entry();
 *	_nc_wrap_entry();
 *	_nc_free_entry();
 *
 */

#include <stdlib.h>
#include <string.h>
#include "tic.h"
#include "term.h"
#include "term_entry.h"

#define MAX_STRTAB	4096	/* documented maximum entry size */

static char	stringbuf[MAX_STRTAB];	/* buffer for string capabilities */
static size_t	next_free;		/* next free character in stringbuf */

void _nc_init_entry(TERMTYPE *tp)
/* initialize a terminal type data block */
{
int	i;

	for (i=0; i < BOOLCOUNT; i++)
		    tp->Booleans[i] = FALSE;
	
	for (i=0; i < NUMCOUNT; i++)
		    tp->Numbers[i] = -1;

	for (i=0; i < STRCOUNT; i++)
		    tp->Strings[i] = (char *)NULL;

	next_free = 0;
}

char *_nc_save_str(char *string)
/* save a copy of string in the string buffer */
{
size_t	old_next_free = next_free;
size_t	len = strlen(string) + 1;

	if (next_free + len < MAX_STRTAB)
	{
		strcpy(&stringbuf[next_free], string);
		DEBUG(7, ("Saved string '%s' ", _nc_visbuf(string)));
		DEBUG(7, ("at location %d", (int) next_free));
		next_free += len;
	}
	return(stringbuf + old_next_free);
}

void _nc_wrap_entry(ENTRY *ep)
/* copy the string parts to allocated storage, preserving pointers to it */
{
int	offsets[STRCOUNT], useoffsets[MAX_USES];
int	i, n;

	n = ep->tterm.term_names - stringbuf;
	for (i=0; i < STRCOUNT; i++)
		if (ep->tterm.Strings[i] == (char *)NULL)
			offsets[i] = -1;
		else if (ep->tterm.Strings[i] == CANCELLED_STRING)
			offsets[i] = -2;
		else
			offsets[i] = ep->tterm.Strings[i] - stringbuf;

	for (i=0; i < ep->nuses; i++)
		if (ep->uses[i] == (void *)NULL)
			useoffsets[i] = -1;
		else
			useoffsets[i] = (char *)(ep->uses[i]) - stringbuf;

	if ((ep->tterm.str_table = (char *)malloc(next_free)) == (char *)NULL)
		_nc_err_abort("Out of memory");
	(void) memcpy(ep->tterm.str_table, stringbuf, next_free);

	ep->tterm.term_names = ep->tterm.str_table + n;
	for (i=0; i < STRCOUNT; i++)
		if (offsets[i] == -1)
			ep->tterm.Strings[i] = (char *)NULL;
		else if (offsets[i] == -2)
			ep->tterm.Strings[i] = CANCELLED_STRING;
		else
			ep->tterm.Strings[i] = ep->tterm.str_table + offsets[i];

	for (i=0; i < ep->nuses; i++)
		if (useoffsets[i] == -1)
			ep->uses[i] = (void *)NULL;
		else
			ep->uses[i] = (char *)(ep->tterm.str_table + useoffsets[i]);
}

void _nc_merge_entry(TERMTYPE *to, TERMTYPE *from)
/* merge capabilities from `from' entry to un-occupied slots in `to' entry */
{
int	i;

	/*
	 * Note: the copies of strings this makes don't have their own
	 * storage.  This is OK right now, but will be a problem if we
	 * we ever want to deallocate entries.
	 */

    	for (i=0; i < BOOLCOUNT; i++) {
		if (to->Booleans[i] == (char)FALSE && from->Booleans[i] == (char)TRUE)
		    to->Booleans[i] = TRUE;
    	}

    	for (i=0; i < NUMCOUNT; i++) {
		if (to->Numbers[i] == -1 && from->Numbers[i] != -1)
		    to->Numbers[i] = from->Numbers[i];
    	}

    	for (i=0; i < STRCOUNT; i++) {
		if (to->Strings[i] == (char *)NULL && from->Strings[i] != (char *)NULL)
		    to->Strings[i] = from->Strings[i];
    	}
}

