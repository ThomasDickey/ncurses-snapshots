
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
#include "object.h"

#define OFFSET_BUFSIZE	100

#define min(a, b)	((a) > (b)  ?  (b)  :  (a))

/*
 *	int
 *	read_file_entry(filename, ptr)
 *
 *	Read the compiled terminfo entry in the given file into the
 *	structure pointed to by ptr, allocating space for the string
 *	table and placing its address in ptr->str_table.
 *
 */

#define swap(x)		(((x >> 8) & 0377) + 256 * (x & 0377))

TERMINAL *cur_term;

int read_file_entry(char *filename, TERMTYPE *ptr)
{
int		fd;
int		numread;
int		num_strings;
int		cur_string;
int		i;
struct header	header;
unsigned char	bytebuf[2];
char		ch;
union {
    short            number;
    unsigned char    byte[2];
} offset_buf[OFFSET_BUFSIZE];
char namebuf[128];

	fd = open(filename, 0);

	if (fd < 0) {
	    return(-1);
	}

	read(fd, &header, sizeof(header));

	if (must_swap()) {
	    	header.magic = swap(header.magic);
	    	header.name_size = swap(header.name_size);
	    	header.bool_count = swap(header.bool_count);
	    	header.num_count = swap(header.num_count);
	    	header.str_count = swap(header.str_count);
	    	header.str_size = swap(header.str_size);
	}

	if (header.magic != MAGIC)
	{
	    	close(fd);
	    	return(-1);
	}

	read(fd, namebuf, min(127, header.name_size));
	namebuf[127] = '\0';
	ptr->term_names = calloc(sizeof(char), strlen(namebuf) + 1);
	(void) strcpy(ptr->term_names, namebuf);
	if (header.name_size > 127)
	    	lseek(fd, (off_t) (header.name_size - 127), 1);

	read(fd, ptr->Booleans, min(BOOLCOUNT, header.bool_count));
	if (header.bool_count > BOOLCOUNT)
	    	lseek(fd, (off_t) (header.bool_count - BOOLCOUNT), 1);
	else
	    	for (i=header.bool_count; i < BOOLCOUNT; i++)
			ptr->Booleans[i] = 0;

	if ((header.name_size + header.bool_count) % 2 != 0)
	    	read(fd, &ch, 1);

	if (!must_swap()) {
	    	read(fd, ptr->Numbers, min(NUMCOUNT * 2, header.num_count * 2));
	} else {
	   	for (i=0; i < min(header.num_count, NUMCOUNT); i++) {
			read(fd, bytebuf, 2);
			if (bytebuf[0] == 0xff  &&  bytebuf[1] == 0xff)
		    	ptr->Numbers[i] = -1;
			else
		    	ptr->Numbers[i] = bytebuf[0] + 256 * bytebuf[1];
	   	}
	}

	if (header.num_count > NUMCOUNT)
	   	lseek(fd, (off_t) (2 * (header.num_count - NUMCOUNT)), 1);
	else
	   	for (i=header.num_count; i < NUMCOUNT; i++)
			ptr->Numbers[i] = -1;

   	ptr->str_table = malloc(header.str_size);
   	if (ptr->str_table == NULL) {
		close(fd);
		return (-1);
   	}

	num_strings = min(STRCOUNT, header.str_count);
	cur_string = 0;

	while (num_strings > 0) {
	    	numread = read(fd, offset_buf, 2*min(num_strings, OFFSET_BUFSIZE));
	    	if (numread <= 0) {
			close(fd);
			return(-1);
	    	}

	    	if (must_swap()) {
			for (i = 0; i < numread / 2; i++) {
		    		ptr->Strings[i + cur_string] =
				(offset_buf[i].byte[0] == 0377
			    &&  offset_buf[i].byte[1] == 0377) ? 0
			: ((offset_buf[i].byte[0] + 256*offset_buf[i].byte[1])
							      + ptr->str_table);
			}
	    	} else {
			for (i = 0; i < numread / 2; i++) {
		    		ptr->Strings[i + cur_string] =
				(offset_buf[i].number == -1) ?  0
				: offset_buf[i].number + ptr->str_table;
			}
	    	}

	    	cur_string += numread / 2;
	    	num_strings -= numread / 2;
	}

	if (header.str_count > STRCOUNT)
	    	lseek(fd, (off_t) (2 * (header.str_count - STRCOUNT)), 1);
	else
	    	for (i=header.str_count; i < STRCOUNT; i++)
			ptr->Strings[i] = 0;

	numread = read(fd, ptr->str_table, header.str_size);
	close(fd);
	if (numread != header.str_size)
	    	return(-1);

	return(0);
}

/*
 *	read_entry(char *tn, TERMTYPE *tp)
 *
 *	Find and read the compiled entry for a given terminal type,
 *	if it exists.
 */

int read_entry(const char *tn, TERMTYPE *tp)
{
char		filename[1024];
char		*directory = TERMINFO;
char		*terminfo;

	if ((terminfo = getenv("TERMINFO")) != NULL)
	    	directory = terminfo;

	/* try a local directory */
	(void) sprintf(filename, "%s/%c/%s", directory, tn[0], tn);
	if (read_file_entry(filename, tp) == OK)
		return(OK);

	/* try the system directory */
	(void) sprintf(filename, "%s/%c/%s", TERMINFO, tn[0], tn);
	if (read_file_entry(filename, tp) == OK)
		return(OK);

	return(ERR);
}

/*
 *	int
 *	must_swap(void)
 *
 *	Test whether this machine will need byte-swapping
 *
 */

int
must_swap(void)
{
union {
    short num;
    unsigned char  byte[2];
}test;

	test.num = 1;
	return(test.byte[1]);
}

/*
 *	bool name_match(namelist, name, delim)
 *
 *	Is the given name matched in namelist?
 */

int name_match(char *namelst, const char *name, const char *delim)
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

