
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
 * Termcap compatibility support
 *
 * If your OS integrator didn't install a terminfo database, you can
 * call _nc_read_termcap_entry() to support reading and translating
 * capabilities from the system termcap file.  This is a kluge; it
 * will bulk up and slow down every program that uses ncurses, and
 * translated termcap entries cannot use full terminfo capabilities.
 * Don't use it unless you absolutely have to; instead, get your
 * system people to run tic(1) from root on the terminfo master
 * included with ncurses to translate it into a terminfo database.
 *
 * If USE_GETCAP is enabled, we use what is effectively a copy of the
 * 4.4BSD getcap code to fetch entries.  There are disadvantages to this;
 * mainly that getcap(3) does its own resolution, meaning that entries
 * read in in this way can't reference the terminfo tree. The only thing
 * it buys is faster startup time, getcap(3) is much faster than our tic 
 * parser.
 */

#include "curses.priv.h"
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <errno.h>

#ifdef USE_GETCAP
/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)termcap.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

#define	PBUFSIZ		512	/* max length of filename path */
#define	PVECSIZ		32	/* max number of names in path */

static char *_nc_cgetcap(char *, char *, int);
static int _nc_cgetent(char **, char **, char *);
static int _nc_cgetmatch(char *, char *);
static int _nc_cgetset(char *);
static int _nc_getent (char **, unsigned int *, char **, int, char *, int, char *);
static int _nc_nfcmp(char *, char *);
static int _nc_tgetent(char *, char *);

/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * BUG:		Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 * Note:	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 */

static	char *tbuf;	/* termcap buffer */

/*
 * Get an entry for terminal name in buffer bp from the termcap file.
 */
static int
_nc_tgetent(char *bp, char *name)
{
	register char *p;
	register char *cp;
	char  *dummy;
	char **fname;
	char  *home;
	int    i;
	char   pathbuf[PBUFSIZ];	/* holds raw path of filenames */
	char  *pathvec[PVECSIZ];	/* to point to names in pathbuf */
	char **pvec;			/* holds usable tail of path vector */
	char  *termpath;

	fname = pathvec;
	pvec = pathvec;
	tbuf = bp;
	p = pathbuf;
	cp = getenv("TERMCAP");
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.  If TERMCAP
	 * does not hold a file name then a path of names is searched
	 * instead.  The path is found in the TERMPATH variable, or
	 * becomes "$HOME/.termcap /etc/termcap" if no TERMPATH exists.
	 */
	if (!cp || *cp != '/') {	/* no TERMCAP or it holds an entry */
		if (termpath = getenv("TERMPATH"))
			strncpy(pathbuf, termpath, PBUFSIZ);
		else {
			if (home = getenv("HOME")) {	/* set up default */
				p += strlen(home);	/* path, looking in */
				strcpy(pathbuf, home);	/* $HOME first */
				*p++ = '/';
			}	/* if no $HOME look in current directory */
#define	_PATH_DEF	".termcap /usr/share/misc/termcap"
			strncpy(p, _PATH_DEF, PBUFSIZ - (p - pathbuf));
		}
	}
	else				/* user-defined name in TERMCAP */
		strncpy(pathbuf, cp, PBUFSIZ);	/* still can be tokenized */

	*fname++ = pathbuf;	/* tokenize path into vector of names */
	while (*++p)
		if (*p == ' ' || *p == ':') {
			*p = '\0';
			while (*++p)
				if (*p != ' ' && *p != ':')
					break;
			if (*p == '\0')
				break;
			*fname++ = p;
			if (fname >= pathvec + PVECSIZ) {
				fname--;
				break;
			}
		}
	*fname = (char *) 0;			/* mark end of vector */
	if (cp && *cp && *cp != '/')
		if (_nc_cgetset(cp) < 0)
			return(-2);

	i = _nc_cgetent(&dummy, pathvec, name);      
	
	if (i == 0)
		strcpy(bp, dummy);
	
	if (dummy)
		free(dummy);
	/* no tc reference loop return code in libterm XXX */
	if (i == -3)
		return(-1);
	return(i + 1);
}

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Casey Leedom of Lawrence Livermore National Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getcap.c	8.3 (Berkeley) 3/25/94";
#endif /* LIBC_SCCS and not lint */

#define	BFRAG		1024
#define	BSIZE		1024
#define	ESC		('[' & 037)	/* ASCII ESC */
#define	MAX_RECURSION	32		/* maximum getent recursion */
#define	SFRAG		100		/* cgetstr mallocs in SFRAG chunks */

#define RECOK	(char)0
#define TCERR	(char)1
#define	SHADOW	(char)2

static size_t	 topreclen;	/* toprec length */
static char	*toprec;	/* Additional record specified by cgetset() */
static int	 gottoprec;	/* Flag indicating retrieval of toprecord */

/*
 * Cgetset() allows the addition of a user specified buffer to be added
 * to the database array, in effect "pushing" the buffer on top of the
 * virtual database. 0 is returned on success, -1 on failure.
 */
static int
_nc_cgetset(char *ent)
{
	if (ent == NULL) {
		if (toprec)
			free(toprec);
                toprec = NULL;
                topreclen = 0;
                return (0);
        }
        topreclen = strlen(ent);
        if ((toprec = malloc (topreclen + 1)) == NULL) {
		errno = ENOMEM;
                return (-1);
	}
	gottoprec = 0;
        (void)strcpy(toprec, ent);
        return (0);
}

/*
 * Cgetcap searches the capability record buf for the capability cap with
 * type `type'.  A pointer to the value of cap is returned on success, NULL
 * if the requested capability couldn't be found.
 *
 * Specifying a type of ':' means that nothing should follow cap (:cap:).
 * In this case a pointer to the terminating ':' or NUL will be returned if
 * cap is found.
 *
 * If (cap, '@') or (cap, terminator, '@') is found before (cap, terminator)
 * return NULL.
 */
static char *
_nc_cgetcap(char *buf, char *cap, int type)
{
	register char *bp, *cp;

	bp = buf;
	for (;;) {
		/*
		 * Skip past the current capability field - it's either the
		 * name field if this is the first time through the loop, or
		 * the remainder of a field whose name failed to match cap.
		 */
		for (;;)
			if (*bp == '\0')
				return (NULL);
			else
				if (*bp++ == ':')
					break;

		/*
		 * Try to match (cap, type) in buf.
		 */
		for (cp = cap; *cp == *bp && *bp != '\0'; cp++, bp++)
			continue;
		if (*cp != '\0')
			continue;
		if (*bp == '@')
			return (NULL);
		if (type == ':') {
			if (*bp != '\0' && *bp != ':')
				continue;
			return(bp);
		}
		if (*bp != type)
			continue;
		bp++;
		return (*bp == '@' ? NULL : bp);
	}
	/* NOTREACHED */
}

/*
 * Cgetent extracts the capability record name from the NULL terminated file
 * array db_array and returns a pointer to a malloc'd copy of it in buf.
 * Buf must be retained through all subsequent calls to cgetcap, cgetnum,
 * cgetflag, and cgetstr, but may then be free'd.  0 is returned on success,
 * -1 if the requested record couldn't be found, -2 if a system error was
 * encountered (couldn't open/read a file, etc.), and -3 if a potential
 * reference loop is detected.
 */
static int
_nc_cgetent(char **buf, char **db_array, char *name)
{
	unsigned int dummy;

	return (_nc_getent(buf, &dummy, db_array, -1, name, 0, NULL));
}

/*
 * Getent implements the functions of cgetent.  If fd is non-negative,
 * *db_array has already been opened and fd is the open file descriptor.  We
 * do this to save time and avoid using up file descriptors for tc=
 * recursions.
 *
 * Getent returns the same success/failure codes as cgetent.  On success, a
 * pointer to a malloc'ed capability record with all tc= capabilities fully
 * expanded and its length (not including trailing ASCII NUL) are left in
 * *cap and *len.
 *
 * Basic algorithm:
 *	+ Allocate memory incrementally as needed in chunks of size BFRAG
 *	  for capability buffer.
 *	+ Recurse for each tc=name and interpolate result.  Stop when all
 *	  names interpolated, a name can't be found, or depth exceeds
 *	  MAX_RECURSION.
 */
static int
_nc_getent(
	char **cap,
	unsigned int *len,
	char **db_array,
	int fd,
	char *name,
	int depth,
	char *nfield)
{
	register char *r_end, *rp, **db_p;
	int myfd, eof, foundit, retval, clen;
	char *record, *cbuf;
	int tc_not_resolved;
	char pbuf[1024];
	
	/*
	 * Return with ``loop detected'' error if we've recursed more than
	 * MAX_RECURSION times.
	 */
	if (depth > MAX_RECURSION)
		return (-3);

	/*
	 * Check if we have a top record from cgetset().
         */
	if (depth == 0 && toprec != NULL && _nc_cgetmatch(toprec, name) == 0) {
		if ((record = malloc (topreclen + BFRAG)) == NULL) {
			errno = ENOMEM;
			return (-2);
		}
		(void)strcpy(record, toprec);
		myfd = 0;
		db_p = db_array;
		rp = record + topreclen + 1;
		r_end = rp + BFRAG;
		goto tc_exp;
	}
	/*
	 * Allocate first chunk of memory.
	 */
	if ((record = malloc(BFRAG)) == NULL) {
		errno = ENOMEM;
		return (-2);
	}
	r_end = record + BFRAG;
	foundit = 0;
	/*
	 * Loop through database array until finding the record.
	 */

	for (db_p = db_array; *db_p != NULL; db_p++) {
		eof = 0;

		/*
		 * Open database if not already open.
		 */

		if (fd >= 0) {
			(void)lseek(fd, (off_t)0, SEEK_SET);
			myfd = 0;
		} else {
			fd = open(*db_p, O_RDONLY, 0);
			if (fd < 0) {
				/* No error on unfound file. */
				if (errno == ENOENT)
					continue;
				free(record);
				return (-2);
			}
			myfd = 1;
		}
		/*
		 * Find the requested capability record ...
		 */
		{
		char buf[2048];
		register char *b_end, *bp;
		register int c;

		/*
		 * Loop invariants:
		 *	There is always room for one more character in record.
		 *	R_end always points just past end of record.
		 *	Rp always points just past last character in record.
		 *	B_end always points just past last character in buf.
		 *	Bp always points at next character in buf.
		 */
		b_end = buf;
		bp = buf;
		for (;;) {

			/*
			 * Read in a line implementing (\, newline)
			 * line continuation.
			 */
			rp = record;
			for (;;) {
				if (bp >= b_end) {
					int n;
		
					n = read(fd, buf, sizeof(buf));
					if (n <= 0) {
						if (myfd)
							(void)close(fd);
						if (n < 0) {
							free(record);
							return (-2);
						} else {
							fd = -1;
							eof = 1;
							break;
						}
					}
					b_end = buf+n;
					bp = buf;
				}
	
				c = *bp++;
				if (c == '\n') {
					if (rp > record && *(rp-1) == '\\') {
						rp--;
						continue;
					} else
						break;
				}
				*rp++ = c;

				/*
				 * Enforce loop invariant: if no room 
				 * left in record buffer, try to get
				 * some more.
				 */
				if (rp >= r_end) {
					unsigned int pos;
					size_t newsize;

					pos = rp - record;
					newsize = r_end - record + BFRAG;
					record = realloc(record, newsize);
					if (record == NULL) {
						errno = ENOMEM;
						if (myfd)
							(void)close(fd);
						return (-2);
					}
					r_end = record + newsize;
					rp = record + pos;
				}
			}
				/* loop invariant let's us do this */
			*rp++ = '\0';

			/*
			 * If encountered eof check next file.
			 */
			if (eof)
				break;
				
			/*
			 * Toss blank lines and comments.
			 */
			if (*record == '\0' || *record == '#')
				continue;
	
			/*
			 * See if this is the record we want ...
			 */
			if (_nc_cgetmatch(record, name) == 0) {
				if (nfield == NULL || !_nc_nfcmp(nfield, record)) {
					foundit = 1;
					break;	/* found it! */
				}
			}
		}
	}
		if (foundit)
			break;
	}

	if (!foundit)
		return (-1);

	/*
	 * Got the capability record, but now we have to expand all tc=name
	 * references in it ...
	 */
tc_exp:	{
		register char *newicap, *s;
		register int newilen;
		unsigned int ilen;
		int diff, iret, tclen;
		char *icap, *scan, *tc, *tcstart, *tcend;

		/*
		 * Loop invariants:
		 *	There is room for one more character in record.
		 *	R_end points just past end of record.
		 *	Rp points just past last character in record.
		 *	Scan points at remainder of record that needs to be
		 *	scanned for tc=name constructs.
		 */
		scan = record;
		tc_not_resolved = 0;
		for (;;) {
			if ((tc = _nc_cgetcap(scan, "tc", '=')) == NULL)
				break;

			/*
			 * Find end of tc=name and stomp on the trailing `:'
			 * (if present) so we can use it to call ourselves.
			 */
			s = tc;
			for (;;)
				if (*s == '\0')
					break;
				else
					if (*s++ == ':') {
						*(s - 1) = '\0';
						break;
					}
			tcstart = tc - 3;
			tclen = s - tcstart;
			tcend = s;

			iret = _nc_getent(&icap, &ilen, db_p, fd, tc, depth+1, 
				      NULL);
			newicap = icap;		/* Put into a register. */
			newilen = ilen;
			if (iret != 0) {
				/* an error */
				if (iret < -1) {
					if (myfd)
						(void)close(fd);
					free(record);
					return (iret);
				}
				if (iret == 1)
					tc_not_resolved = 1;
				/* couldn't resolve tc */
				if (iret == -1) {
					*(s - 1) = ':';			
					scan = s - 1;
					tc_not_resolved = 1;
					continue;
					
				}
			}
			/* not interested in name field of tc'ed record */
			s = newicap;
			for (;;)
				if (*s == '\0')
					break;
				else
					if (*s++ == ':')
						break;
			newilen -= s - newicap;
			newicap = s;

			/* make sure interpolated record is `:'-terminated */
			s += newilen;
			if (*(s-1) != ':') {
				*s = ':';	/* overwrite NUL with : */
				newilen++;
			}

			/*
			 * Make sure there's enough room to insert the
			 * new record.
			 */
			diff = newilen - tclen;
			if (diff >= r_end - rp) {
				unsigned int pos, tcpos, tcposend;
				size_t newsize;

				pos = rp - record;
				newsize = r_end - record + diff + BFRAG;
				tcpos = tcstart - record;
				tcposend = tcend - record;
				record = realloc(record, newsize);
				if (record == NULL) {
					errno = ENOMEM;
					if (myfd)
						(void)close(fd);
					free(icap);
					return (-2);
				}
				r_end = record + newsize;
				rp = record + pos;
				tcstart = record + tcpos;
				tcend = record + tcposend;
			}

			/*
			 * Insert tc'ed record into our record.
			 */
			s = tcstart + newilen;
			memmove(s, tcend, rp - tcend);
			memmove(tcstart, newicap, newilen);
			rp += diff;
			free(icap);

			/*
			 * Start scan on `:' so next cgetcap works properly
			 * (cgetcap always skips first field).
			 */
			scan = s-1;
		}
	
	}
	/*
	 * Close file (if we opened it), give back any extra memory, and
	 * return capability, length and success.
	 */
	if (myfd)
		(void)close(fd);
	*len = rp - record - 1;	/* don't count NUL */
	if (r_end > rp)
		if ((record = 
		     realloc(record, (size_t)(rp - record))) == NULL) {
			errno = ENOMEM;
			return (-2);
		}
		
	*cap = record;
	if (tc_not_resolved)
		return (1);
	return (0);
}	

/*
 * Cgetmatch will return 0 if name is one of the names of the capability
 * record buf, -1 if not.
 */
static int
_nc_cgetmatch(char *buf, char *name)
{
	register char *np, *bp;

	/*
	 * Start search at beginning of record.
	 */
	bp = buf;
	for (;;) {
		/*
		 * Try to match a record name.
		 */
		np = name;
		for (;;)
			if (*np == '\0')
				if (*bp == '|' || *bp == ':' || *bp == '\0')
					return (0);
				else
					break;
			else
				if (*bp++ != *np++)
					break;

		/*
		 * Match failed, skip to next name in record.
		 */
		bp--;	/* a '|' or ':' may have stopped the match */
		for (;;)
			if (*bp == '\0' || *bp == ':')
				return (-1);	/* match failed totally */
			else
				if (*bp++ == '|')
					break;	/* found next name */
	}
}

/*
 * Compare name field of record.
 */
static int
_nc_nfcmp(char *nf, char *rec)
{
	char *cp, tmp;
	int ret;
	
	for (cp = rec; *cp != ':'; cp++)
		;
	
	tmp = *(cp + 1);
	*(cp + 1) = '\0';
	ret = strcmp(nf, rec);
	*(cp + 1) = tmp;

	return (ret);
}
#endif /* USE_GETCAP */

#include "term.h"
#include "tic.h"
#include "term_entry.h"

int _nc_read_termcap_entry(const char *const tn, TERMTYPE *const tp)
{
    ENTRY	*ep;
#ifndef USE_GETCAP
    /*
     * Here is what the 4.4BSD termcap(3) page prescribes:
     *
     * It will look in the environment for a TERMCAP variable.  If
     * found, and the value does not begin with a slash, and the
     * terminal type name is the same as the environment string
     * TERM, the TERMCAP string is used instead of reading a termcap
     * file.  If it does begin with a slash, the string is used as a
     * path name of the termcap file to search.  If TERMCAP does not
     * begin with a slash and name is different from TERM, tgetent()
     * searches the files $HOME/.termcap and /usr/share/misc/termcap,
     * in that order, unless the environment variable TERMPATH exists,
     * in which case it specifies a list of file pathnames (separated
     * by spaces or colons) to be searched instead.
     *
     * It goes on to state:
     *
     * Whenever multiple files are searched and a tc field occurs in the
     * requested entry, the entry it names must
     * be found in the same file or one of the succeeding files.
     *
     * However, this restriction is relaxed in ncurses; tc references to
     * previous files are permitted.
     *
     * This routine returns 1 if an entry is found, 0 if not found, and -1
     * if the database is not accessable.
     */
    FILE	*fp;
#define MAXPATHS	32
    char	*tc, *termpaths[MAXPATHS], pathbuf[2048];
    int    	filecount = 0;
    bool	use_buffer = FALSE;

    if ((tc = getenv("TERMCAP")) != (char *)NULL)
    {
	if (tc[0] == '/')	/* interpret as a filename */
	{
	    termpaths[0] = tc;
	    termpaths[filecount = 1] = (char *)NULL;
	}
	else if (_nc_name_match(tc, tn, "|:"))    /* treat as a capability file */
	{
 	    use_buffer = TRUE;
	    (void) strcpy(pathbuf, tc);
	    (void) strcat(pathbuf, "\n");
	}
	else if ((tc = getenv("TERMPATH")) != (char *)NULL)
	{
	    char    *cp;

	    for (cp = tc; *cp; cp++)
	    {
		if (*cp == ':')
		    *cp = '\0';
		else if (cp == tc || cp[-1] == '\0')
		{
		    if (filecount >= MAXPATHS - 1)
			return(-1);

		    termpaths[filecount++] = cp;
		}
	    }

	    termpaths[filecount] = (char *)NULL;
	}
    }
    else	/* normal case */
    {
	char	envhome[PATH_MAX];

	filecount = 0;

	/*
	 * Probably /etc/termcap is a symlink to /usr/share/misc/termcap.
	 * Avoid reading the same file twice.
	 */
	if (access("/etc/termcap", R_OK) == 0)
	    termpaths[filecount++] = "/etc/termcap";
	else if (access("/usr/share/misc/termcap", R_OK) == 0)
	    termpaths[filecount++] = "/usr/share/misc/termcap";

	/* user's .termcap, if any, should override it */
	(void) strncpy(envhome, getenv("HOME"), PATH_MAX - 10);
	envhome[PATH_MAX - 10] = '\0';
	(void) sprintf(pathbuf, "%s/.termcap", envhome);
	termpaths[filecount++] = pathbuf;

	termpaths[filecount] = (char *)NULL;
    }

    /* parse the sources */
    if (use_buffer)
    {
	_nc_set_source("TERMCAP");

	/*
	 * We don't suppress warning messages here.  The presumption is
	 * that since it's just a single entry, they won't be a pain.
	 */
	_nc_read_entry_source((FILE *)NULL, pathbuf, FALSE, FALSE, NULLHOOK);
    }
    else
    {
	int    	i;

	for (i = 0; i < filecount; i++)
	{
	    T(("Looking for %s in %s", tn, termpaths[i]));
	    if ((fp = fopen(termpaths[i], "r")) != (FILE *)NULL)
	    {
		_nc_set_source(termpaths[i]);

		/*
		 * Suppress warning messages.  Otherwise you get 400
		 * lines of crap from archaic termcap files as ncurses
		 * complains about all the obsolete capabilities.
		 */
		_nc_read_entry_source(fp, (char*)NULL, FALSE, TRUE, NULLHOOK);

		(void) fclose(fp);
	    }
	}
    }
#else
    char	tc[2048 * 2];

    /* we're using getcap(3) */
    if (_nc_tgetent(tc, (char *)tn) <= 0)
	    return (ERR);

    _nc_set_source("TERMCAP");
    _nc_read_entry_source((FILE *)NULL, tc, FALSE, FALSE, NULLHOOK);
#endif

    if (_nc_head == (ENTRY *)NULL)
	return(-1);

    /* resolve all use references */
    _nc_resolve_uses();

    /* find a terminal matching tn, if we can */
    for_entry_list(ep)
	if (_nc_name_match(ep->tterm.term_names, tn, "|:"))
	{
	    /*
	     * Make a local copy of the terminal capabilities.  free
	     * all entry storage except the string table for the
	     * loaded type (which we disconnected from the list by
	     * NULLing out ep->tterm.str_table above).
	     */
	    memcpy(tp, &ep->tterm, sizeof(TERMTYPE));
	    ep->tterm.str_table = (char *)NULL;
	    _nc_free_entries(_nc_head);

	    /*
	     * OK, now try to write the type to user's terminfo directory.
	     * Next time he loads this, it will come through terminfo.
	     *
	     * Advantage: Second and subsequent fetches of this entry
	     * will be very fast.
	     *
	     * Disadvantage: After the first time a termcap type is loaded
	     * by its user, editing it in the /etc/termcap file, or in TERMCAP,
	     * or in a local ~/.termcap, will be ineffective unless the
	     * terminfo entry is explicitly removed.
	     */
	    (void) _nc_write_entry(tp);

	    return(1);
	}

    _nc_free_entries(_nc_head);
    return(0);
}
