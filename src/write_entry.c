
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
 *	write_entry.c -- write a terminfo structure onto the file system
 */

#include "config.h"

#include <stdlib.h>

#include <errno.h>
#if !HAVE_EXTERN_ERRNO
extern int errno;
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#else
# if HAVE_LIBC_H
# include <libc.h>
# endif
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "tic.h"
#include "term.h"
#include "term_entry.h"
#include "object.h"

static int write_object(FILE *, TERMTYPE *);

static time_t	start_time;		/* time at start of writes */
static char	*destination = TERMINFO;

/*
 *	check_writeable(void)
 *
 *	Miscellaneous initialisations
 *
 *	Check for access rights to destination directories
 *	Create any directories which don't exist.
 *
 */

static void check_writeable(void)
{
struct stat	statbuf;
char		*dirnames = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
char		dir[2];

	if (getenv("TERMINFO") != NULL)
	    destination = getenv("TERMINFO");

	if (access(destination, W_OK) < 0) {
	    err_abort("`%s' non-existant or permission denied (errno %d)", destination, errno);
	}

	/*
	 * Note: because of this code, this function should be called
	 * *once only* per run.
	 */
	if (chdir(destination) < 0) {
	    err_abort("%s is not a directory", destination);
	}
	
	dir[1] = '\0';
	for (dir[0] = *dirnames; *dirnames != '\0'; dir[0] = *(++dirnames)) {
	    	if (stat(dir, &statbuf) < 0) {
			mkdir(dir, 0755);
	    	} else if (access(dir, 7) < 0) {
			err_abort("%s/%s: Permission denied",destination, dir);
	    	}
#ifdef _POSIX_SOURCE
	    	else if (!(S_ISDIR(statbuf.st_mode)))
#else
	    	else if ((statbuf.st_mode & S_IFMT) != S_IFDIR)
#endif	    
			err_abort("%s/%s: Not a directory\n", destination,dir);
	}
}

/*
 *	write_entry()
 *
 *	Save the compiled version of a description in the filesystem.
 *
 *	make a copy of the name-list
 *	break it up into first-name and all-but-last-name
 *	creat(first-name)
 *	write object information to first-name
 *	close(first-name)
 *      for each name in all-but-last-name
 *	    link to first-name
 *
 *	Using 'time()' to obtain a reference for file timestamps is unreliable,
 *	e.g., with NFS, because the filesystem may have a different time
 *	reference.  We check for pre-existence of links by latching the first
 *	timestamp from a file that we create.
 */

void write_entry(TERMTYPE *tp)
{
struct stat	statbuf;
FILE		*fp;
char		name_list[1024];
char		*first_name, *other_names;
char		*ptr;
char		filename[100];
char		linkname[100];
static int	call_count;

	if (call_count++ == 0) {
		check_writeable();
		start_time = 0;
	}

	strcpy(name_list, tp->term_names);
	DEBUG(7, ("Name list = '%s'", name_list));

	first_name = name_list;

	ptr = &name_list[strlen(name_list) - 1];
	other_names = ptr + 1;

	while (ptr > name_list  &&  *ptr != '|')
	    	ptr--;

	if (ptr != name_list) {
	    	*ptr = '\0';

	    	for (ptr = name_list; *ptr != '\0'  &&  *ptr != '|'; ptr++)
			;
	    
	    	if (*ptr == '\0')
			other_names = ptr;
	    	else {
			*ptr = '\0';
			other_names = ptr + 1;
	    	}
	}

	DEBUG(7, ("First name = '%s'", first_name));
	DEBUG(7, ("Other names = '%s'", other_names));

	if (strlen(first_name) > sizeof(filename)-3)
	    	warning("'%s': terminal name too long.", first_name);

	sprintf(filename, "%c/%s", first_name[0], first_name);

	fp = fopen(filename, "w");
	if (fp == NULL) {
	    	perror(filename);
	    	syserr_abort("Can't open %s/%s", destination, filename);
	}
	DEBUG(1, ("Created %s", filename));

	if (write_object(fp, tp) == ERR) {
	    	syserr_abort("Error in writing %s/%s", destination, filename);
	}
	fclose(fp);

	if (start_time == 0) {
	    	if (stat(filename, &statbuf) < 0
		 || (start_time = statbuf.st_mtime) == 0) {
	    		syserr_abort("Error in obtaining time from %s/%s",
				destination, filename);
		}
	}
	while (*other_names != '\0') {
	    	ptr = other_names++;
	    	while (*other_names != '|'  &&  *other_names != '\0')
			other_names++;

	    	if (*other_names != '\0')
			*(other_names++) = '\0';

	    	if (strlen(ptr) > sizeof(linkname)-3) {
			warning("'%s': terminal name too long.", ptr);
			continue;
	    	}

	    	sprintf(linkname, "%c/%s", ptr[0], ptr);

	    	if (strcmp(filename, linkname) == 0) {
			warning("Terminal name '%s' synonym for itself", first_name);
	    	}
	    	else if (stat(linkname, &statbuf) >= 0  &&
						statbuf.st_mtime < start_time)
	    	{
			fprintf(stderr,
				"'%s' defined in more than one entry; using '%s'.\n",
				ptr, tp->term_names);
		}
		else
	    	{
			unlink(linkname);
			if (link(filename, linkname) < 0)
			    syserr_abort("Can't link %s to %s", filename, linkname);
			DEBUG(1, ("Linked %s", linkname));
	    	}
	}
}


/*
 *	int
 *	write_object(fp, tp)
 *
 *	Write out the compiled entry to the given file.
 *	Return 0 if OK or -1 if not.
 *
 */

#define swap(x)		(((x >> 8) & 0377) + 256 * (x & 0377))

static int write_object(FILE *fp, TERMTYPE *tp)
{
struct header	header;
char		*namelist;
short		namelen;
char		zero = '\0';
int		i, nextfree;
short		offsets[STRCOUNT];

	namelist = tp->term_names;
	namelen = strlen(namelist) + 1;

	nextfree = 0;
	for (i = 0; i < STRWRITE; i++)
	    if (tp->Strings[i] == (char *)NULL)
		offsets[i] = -1;
	    else
	    {
		offsets[i] = nextfree;
		nextfree += strlen(tp->Strings[i]) + 1;
	    }

	if (must_swap()) {
	    	header.magic = swap(MAGIC);
	    	header.name_size = swap(namelen);
	    	header.bool_count = swap(BOOLWRITE);
	    	header.num_count = swap(NUMWRITE);
	    	header.str_count = swap(STRWRITE);
	    	header.str_size = swap(nextfree);
	} else {
	    	header.magic = MAGIC;
	    	header.name_size = namelen;
	    	header.bool_count = BOOLWRITE;
	    	header.num_count = NUMWRITE;
	    	header.str_count = STRWRITE;
	    	header.str_size = nextfree;
	}

	if (fwrite(&header, sizeof(header), 1, fp) != 1
		||  fwrite(namelist, sizeof(char), (size_t)namelen, fp) != namelen
		||  fwrite(tp->Booleans, sizeof(char), BOOLWRITE, fp) != BOOLWRITE)
	    	return(ERR);
	
	if ((namelen+BOOLWRITE) % 2 != 0  &&  fwrite(&zero, sizeof(char), 1, fp) != 1)
	    	return(ERR);

	if (must_swap()) {
	    	for (i = 0; i < NUMWRITE; i++)
			tp->Numbers[i] = swap(tp->Numbers[i]);
	    	for (i = 0; i < STRWRITE; i++)
			offsets[i] = swap(offsets[i]);
	}

	if (fwrite(tp->Numbers, sizeof(short), NUMWRITE, fp) != NUMWRITE
			|| fwrite(offsets, sizeof(short), STRWRITE, fp) != STRWRITE)
		return(ERR);

	for (i = 0; i < STRWRITE; i++)
	    if (tp->Strings[i] != (char *)NULL)
		if (fwrite(tp->Strings[i], sizeof(char), strlen(tp->Strings[i]) + 1, fp) != strlen(tp->Strings[i]) + 1)
		    return(ERR);

        return(OK);
}

