
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

#include <config.h>

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
#include <tic.h>
#include "term.h"
#include "term_entry.h"

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
	    _nc_err_abort("`%s' non-existant or permission denied (errno %d)", destination, errno);
	}

	/*
	 * Note: because of this code, this function should be called
	 * *once only* per run.
	 */
	if (chdir(destination) < 0) {
	    _nc_err_abort("%s is not a directory", destination);
	}
	
	dir[1] = '\0';
	for (dir[0] = *dirnames; *dirnames != '\0'; dir[0] = *(++dirnames)) {
	    	if (stat(dir, &statbuf) < 0) {
			mkdir(dir, 0755);
	    	} else if (access(dir, 7) < 0) {
			_nc_err_abort("%s/%s: Permission denied",destination, dir);
	    	}
#ifdef _POSIX_SOURCE
	    	else if (!(S_ISDIR(statbuf.st_mode)))
#else
	    	else if ((statbuf.st_mode & S_IFMT) != S_IFDIR)
#endif	    
			_nc_err_abort("%s/%s: Not a directory\n", destination,dir);
	}
}

/*
 *	_nc_write_entry()
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

void _nc_write_entry(TERMTYPE *tp)
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

	(void) strcpy(name_list, tp->term_names);
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
	    	_nc_warning("'%s': terminal name too long.", first_name);

	sprintf(filename, "%c/%s", first_name[0], first_name);

	fp = fopen(filename, "w");
	if (fp == NULL) {
	    	perror(filename);
	    	_nc_syserr_abort("Can't open %s/%s", destination, filename);
	}
	DEBUG(1, ("Created %s", filename));

	if (write_object(fp, tp) == ERR) {
	    	_nc_syserr_abort("Error in writing %s/%s", destination, filename);
	}
	fclose(fp);

	if (start_time == 0) {
	    	if (stat(filename, &statbuf) < 0
		 || (start_time = statbuf.st_mtime) == 0) {
	    		_nc_syserr_abort("Error in obtaining time from %s/%s",
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
			_nc_warning("'%s': terminal name too long.", ptr);
			continue;
	    	}

	    	sprintf(linkname, "%c/%s", ptr[0], ptr);

	    	if (strcmp(filename, linkname) == 0) {
			_nc_warning("Terminal name '%s' synonym for itself", first_name);
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
			    _nc_syserr_abort("Can't link %s to %s", filename, linkname);
			DEBUG(1, ("Linked %s", linkname));
	    	}
	}
}

/*
 *	int
 *	_nc_must_swap(void)
 *
 *	Test whether this machine will need byte-swapping
 *
 */

static inline int
must_swap(void)
{
union {
    short num;
    unsigned char  byte[2];
}test;

	test.num = 1;
	return(test.byte[1]);
}

#define swap(x)		(((x >> 8) & 0377) + 256 * (x & 0377))

#define HI(x)			((x) / 256)
#define LO(x)			((x) % 256)
#define LITTLE_ENDIAN(p, x)	(p)[0] = LO(x), (p)[1] = HI(x)
#define min(a, b)		((a) > (b)  ?  (b)  :  (a))

#define SHOWOFFSET

static int write_object(FILE *fp, TERMTYPE *tp)
{
char		*namelist;
short		namelen, boolmax, nummax, strmax;
char		zero = '\0';
int		i, nextfree;
short		offsets[STRCOUNT];
unsigned char	buf[MAX_ENTRY_SIZE];

	namelist = tp->term_names;
	namelen = strlen(namelist) + 1;

	boolmax = 0;
	for (i = 0; i < BOOLWRITE; i++)
		if (tp->Booleans[i])
			boolmax = i+1;

	nummax = 0;
	for (i = 0; i < NUMWRITE; i++)
		if (tp->Numbers[i] != ABSENT_NUMERIC)
			nummax = i+1;

	strmax = 0;
	for (i = 0; i < STRWRITE; i++)
		if (tp->Strings[i] != ABSENT_STRING)
			strmax = i+1;

	nextfree = 0;
	for (i = 0; i < strmax; i++)
	    if (tp->Strings[i] == ABSENT_STRING)
		offsets[i] = -1;
	    else
	    {
		offsets[i] = nextfree;
		nextfree += strlen(tp->Strings[i]) + 1;
	    }

	/* fill in the header */
	LITTLE_ENDIAN(buf,    MAGIC);
	LITTLE_ENDIAN(buf+2,  min(namelen, MAX_NAME_SIZE + 1));
	LITTLE_ENDIAN(buf+4,  boolmax);
	LITTLE_ENDIAN(buf+6,  nummax);
	LITTLE_ENDIAN(buf+8,  strmax);
	LITTLE_ENDIAN(buf+10, nextfree);

	/* write out the header */
	if (fwrite(buf, 12, 1, fp) != 1
		||  fwrite(namelist, sizeof(char), (size_t)namelen, fp) != namelen
		||  fwrite(tp->Booleans, sizeof(char), boolmax, fp) != boolmax)
	    	return(ERR);

	/* the even-boundary padding byte */
	if ((namelen+boolmax) % 2 != 0  &&  fwrite(&zero, sizeof(char), 1, fp) != 1)
	    	return(ERR);

#ifdef SHOWOFFSET
	(void) fprintf(stderr, "Numerics begin at %04lx\n", ftell(fp));
#endif /* SHOWOFFSET */

	/* the numerics */
	for (i = 0; i < nummax; i++)
	{
		if (tp->Numbers[i] == -1)	/* HI/LO won't work */
			buf[2*i] = buf[2*i + 1] = 0377;
		else
			LITTLE_ENDIAN(buf + 2*i, tp->Numbers[i]);
	}
	if (fwrite(buf, 2, nummax, fp) != nummax)
		return(ERR);
 
#ifdef SHOWOFFSET
	(void) fprintf(stderr, "String offets begin at %04lx\n", ftell(fp));
#endif /* SHOWOFFSET */

	/* the string offsets */
	for (i = 0; i < strmax; i++)
		if (offsets[i] == -1)	/* HI/LO won't work */
			buf[2*i] = buf[2*i + 1] = 0377;
		else
			LITTLE_ENDIAN(buf + 2*i, offsets[i]);
	if (fwrite(buf, 2, strmax, fp) != strmax)
		return(ERR);

#ifdef SHOWOFFSET
	(void) fprintf(stderr, "String table begins at %04lx\n", ftell(fp));
#endif /* SHOWOFFSET */

	/* the strings */
	for (i = 0; i < strmax; i++)
	    if (tp->Strings[i] != (char *)NULL)
		if (fwrite(tp->Strings[i], sizeof(char), strlen(tp->Strings[i]) + 1, fp) != strlen(tp->Strings[i]) + 1)
		    return(ERR);

        return(OK);
}



