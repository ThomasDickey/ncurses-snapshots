
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
 *	comp_hash.c --- Routines to deal with the hashtable of capability
 *			names.
 *
 */

#include <string.h>
#include "tic.h"
#include "term.h"
#include "hashsize.h"

static  int hash_function(const char *);

/*
 *	_nc_make_hash_table()
 *
 *	Takes the entries in cap_table[] and hashes them into cap_hash_table[]
 *	by name.  There are Captabsize entries in cap_table[] and Hashtabsize
 *	slots in cap_hash_table[].
 *
 */

void _nc_make_hash_table(struct name_table_entry *table,
		     struct name_table_entry **hash_table)
{
int	i;
int	hashvalue;
int	collisions = 0;

	for (i = 0; i < CAPTABSIZE; i++) {
	    hashvalue = hash_function(table[i].nte_name);       

	    if (hash_table[hashvalue] != (struct name_table_entry *) 0)
		collisions++;

	    table[i].nte_link = hash_table[hashvalue];
	    hash_table[hashvalue] = &table[i];
	}

	DEBUG(4, ("Hash table complete: %d collisions out of %d entries", collisions, CAPTABSIZE));
}


/*
 *	int hash_function(string)
 *
 *	Computes the hashing function on the given string.
 *
 *	The current hash function is the sum of each consectutive pair
 *	of characters, taken as two-byte integers, mod Hashtabsize.
 *
 */

static
int
hash_function(const char *string)
{
long	sum = 0;

	DEBUG(9, ("hashing %s", string));
	while (*string) {
	    sum += *string + (*(string + 1) << 8);
	    string++;
	}

	DEBUG(9, ("sum is %ld", sum));
	return (sum % HASHTABSIZE);
}


/*
 *	struct name_table_entry *
 *	find_entry(string)
 *
 *	Finds the entry for the given string in the hash table if present.
 *	Returns a pointer to the entry in the table or 0 if not found.
 *
 */

struct name_table_entry *
_nc_find_entry(const char *string, struct name_table_entry **hash_table)
{
int	hashvalue;
struct name_table_entry	*ptr;

	hashvalue = hash_function(string);

	ptr = hash_table[hashvalue];

	while (ptr != (struct name_table_entry *) 0  && strcmp(ptr->nte_name, string) != 0)
	    	ptr = ptr->nte_link;

	return (ptr);
}

/*
 *	struct name_table_entry *
 *	find_type_entry(string, type, table)
 *
 *	Finds the first entry for the given name with the given type in the
 *	given table if present (as distinct from find_entry, which finds the
 *	the last entry regardless of type).  You can use this if you detect
 *	a name clash.  It's slower, though.  Returns a pointer to the entry
 *	in the table or 0 if not found.
 */

struct name_table_entry *
_nc_find_type_entry(const char *string,
		    int type,
		    struct name_table_entry *table)
{
struct name_table_entry	*ptr;

	for (ptr = table; ptr < table + CAPTABSIZE; ptr++) {
	    if (ptr->nte_type == type && strcmp(string, ptr->nte_name) == 0)
		return(ptr);
	}

	return ((struct name_table_entry *)NULL);
}

