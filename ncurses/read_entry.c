
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
 *	read_entry.c -- Routine for reading in a compiled terminfo file
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#else
# if HAVE_LIBC_H
# include <libc.h> 
# endif
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <curses.h>
#include "term.h"
#include "tic.h"

TERMINAL *cur_term;

#define min(a, b)	((a) > (b)  ?  (b)  :  (a))
#define max(a, b)	((a) < (b)  ?  (b)  :  (a))

/*
 *	int
 *	_nc_read_file_entry(filename, ptr)
 *
 *	Read the compiled terminfo entry in the given file into the
 *	structure pointed to by ptr, allocating space for the string
 *	table.
 */

#define IS_NEG1(p)	(((p)[0] == 0377) && ((p)[1] == 0377))
#define LOW_MSB(p)	((p)[0] + 256*(p)[1])

int _nc_read_file_entry(char *filename, TERMTYPE *ptr)
{
    int			name_size, bool_count, num_count, str_count, str_size;
    int			i, fd, numread;
    unsigned char 	buf[MAX_ENTRY_SIZE];

    if ((fd = open(filename, 0)) < 0)
	return(-1);

    /* grab the header */
    (void) read(fd, buf, 12);
    if (LOW_MSB(buf) != MAGIC)
    {
	close(fd);
	return(-1);
    }
    name_size  = LOW_MSB(buf + 2);
    bool_count = LOW_MSB(buf + 4);
    num_count  = LOW_MSB(buf + 6);
    str_count  = LOW_MSB(buf + 8);
    str_size   = LOW_MSB(buf + 10);

    /* try to allocate space for the string table */
    ptr->str_table = malloc((unsigned)str_size);
    if (ptr->str_table == NULL)
    {
	close(fd);
	return (-1);
    }

    /* grab the name */
    read(fd, buf, min(MAX_NAME_SIZE, (unsigned)name_size));
    buf[MAX_NAME_SIZE] = '\0';
    ptr->term_names = calloc(sizeof(char), strlen(buf) + 1);
    (void) strcpy(ptr->term_names, buf);
    if (name_size > MAX_NAME_SIZE)
	lseek(fd, (off_t) (name_size - MAX_NAME_SIZE), 1);

    /* grab the booleans */
    read(fd, ptr->Booleans, min(BOOLCOUNT, (unsigned)bool_count));
    if (bool_count > BOOLCOUNT)
	lseek(fd, (off_t) (bool_count - BOOLCOUNT), 1);
    else
	for (i=bool_count; i < BOOLCOUNT; i++)
	    ptr->Booleans[i] = 0;

    /*
     * If booleans end on an odd byte, skip it.  The machine they 
     * originally wrote terminfo on must have been a 16-bit 
     * word-oriented machine that would trap out if you tried a
     * word access off a 2-byte boundary.
     */
    if ((name_size + bool_count) % 2 != 0)
	read(fd, buf, 1);

    /* grab the numbers */
    (void) read(fd, buf, min(NUMCOUNT*2, (unsigned)num_count*2));
    for (i = 0; i < min(num_count, NUMCOUNT); i++) 
    {
	if (IS_NEG1(buf + 2*i))
	    ptr->Numbers[i] = -1;
	else
	    ptr->Numbers[i] = LOW_MSB(buf + 2*i);
    }
    if (num_count > NUMCOUNT)
	lseek(fd, (off_t) (2 * (num_count - NUMCOUNT)), 1);
    else
	for (i=num_count; i < NUMCOUNT; i++)
	    ptr->Numbers[i] = -1;

    /* grab the string offsets */
    numread = read(fd, buf, (unsigned)(str_count*2));
    if (numread < str_count*2)
    {
	close(fd);
	return(-1);
    }
    for (i = 0; i < numread/2; i++)
    {
	ptr->Strings[i] =
	    IS_NEG1(buf + 2*i) ? 0 : (LOW_MSB(buf+2*i) + ptr->str_table);
    }
    if (str_count > STRCOUNT)
	lseek(fd, (off_t) (2 * (str_count - STRCOUNT)), 1);
    else
	for (i = str_count; i < STRCOUNT; i++)
	    ptr->Strings[i] = 0;

    /* finally, grab the string table itself */
    numread = read(fd, ptr->str_table, (unsigned)str_size);
    close(fd);
    if (numread != str_size)
	return(-1);

    return(0);
}

/*
 *	_nc_read_entry(char *tn, TERMTYPE *tp)
 *
 *	Find and read the compiled entry for a given terminal type,
 *	if it exists.
 */

int _nc_read_entry(const char *tn, TERMTYPE *tp)
{
char		filename[1024];
char		*directory = TERMINFO;
char		*terminfo;

	if ((terminfo = getenv("TERMINFO")) != NULL)
	    	directory = terminfo;

	/* try a local directory */
	(void) sprintf(filename, "%s/%c/%s", directory, tn[0], tn);
	if (_nc_read_file_entry(filename, tp) == OK)
		return(OK);

	/* try the system directory */
	(void) sprintf(filename, "%s/%c/%s", TERMINFO, tn[0], tn);
	if (_nc_read_file_entry(filename, tp) == OK)
		return(OK);

	return(ERR);
}

/*
 *	bool _nc_name_match(namelist, name, delim)
 *
 *	Is the given name matched in namelist?
 */

int _nc_name_match(char *namelst, const char *name, const char *delim)
{
char *cp, namecopy[2048];

	if (namelst == NULL)
		return(FALSE);
    	(void) strcpy(namecopy, namelst);
    	if ((cp = strtok(namecopy, delim)) != NULL)
    		do {
			if (strcmp(cp, name) == 0)
			    return(TRUE);
    		} while
		    ((cp = strtok((char *)NULL, delim)) != NULL);

    	return(FALSE);
}

