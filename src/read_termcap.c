
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
 * If your OS integrator didn't install a terminfo database,
 * use -DTERMCAP_FILE to compile this code giving runtime support
 * for reading and translating capabilities from the termcap
 * database.  This is a kluge; it will bulk up and slow down
 * every program that uses ncurses, and translated termcap
 * entries cannot use full terminfo capabilities.  Don't use
 * it unless you absolutely have to; instead, get your system
 * people to run tic(1) from root on the existing termcap file
 * to translate it into a terminfo database.
 *
 */

#include <stdlib.h>
#include <string.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "curses.priv.h"
#include "term.h"
#include "tic.h"
#include "term_entry.h"

int read_termcap_entry(char *tn, TERMTYPE *tp)
{
#ifdef TERMCAP_FILE
    /*
     * Here is what the BSD termcap(3) page prescribes:
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
     */
    FILE	*fp;
    ENTRY	*ep;
#define MAXPATHS	32
    char	*tc, *termpaths[MAXPATHS], pathbuf[BUFSIZ];
    int    	i, filecount = 0;
    bool	delete = FALSE;

    if ((tc = getenv("TERMCAP")) != (char *)NULL)
    {
	if (tc[0] == '/')	/* interpret as a filename */
	{
	    termpaths[0] = tc;
	    termpaths[filecount = 1] = (char *)NULL;
	}
	else if (name_match(tc, tn))    /* treat it as a capability file */
	{
	    termpaths[0] = mktemp("/tmp/tcXXXXXX");
	    termpaths[filecount = 1] = (char *)NULL;

	    if ((fp = fopen(termpaths[0], "w")) != (FILE *)NULL)
	    {
		(void) fwrite(tc, strlen(tc), sizeof(char), fp);
		(void) fclose(fp);
		delete = TRUE;
	    }
	}
	else if ((tc = getenv("TERMPATHS")) != (char *)NULL)
	{
	    char    *cp;

	    for (cp = tc; *cp; cp++)
	    {
		if (*cp == ':')
		    *cp = '\0';
		else if (cp == tc || cp[-1] == '\0')
		{
		    if (filecount >= MAXPATHS - 1)
			return(ERR);

		    termpaths[filecount++] = cp;
		}
	    }

	    termpaths[filecount] = (char *)NULL;
	}
    }
    else	/* normal case */
    {
	(void) sprintf(pathbuf, "%s/.termcap", getenv("HOME"));
	termpaths[0] = pathbuf;
	termpaths[1] = TERMCAP_FILE;
	termpaths[filecount = 2] = (char *)NULL;
    }

    /* get the data from all designated files */
    make_hash_table(info_table, info_hash_table);
    make_hash_table(cap_table, cap_hash_table);
    for (i = 0; i < filecount; i++)
    {
	T(("Looking for %s in %s", tn, termpaths[i]));
	if ((fp = fopen(termpaths[i], "r")) != (FILE *)NULL)
	{
	    set_source(termpaths[i]);
	    read_entry_source(fp, FALSE);
	    (void) fclose(fp);
	}
    }

    if (delete)
	(void) remove(termpaths[0]);

    if (head == (ENTRY *)NULL)
	return(ERR);

    resolve_uses();

    /* find a terminal matching tn, if we can */
    for_entry_list(ep)
	if (name_match(ep->tterm.term_names, tn))
	{
	    memcpy(tp, &ep->tterm, sizeof(TERMTYPE));
	    ep->tterm.str_table = (char *)NULL;
	    return(OK);
	}

    /*
     * Now that we have a local copy of the terminal capabilities.
     * free all entry storage except the string table for the loaded
     * type (which we disconnected from the list by NULLing out
     * ep->tterm.str_table above).
     */
    free_entries();
#endif /* TERMCAP_FILE */

    return(ERR);
}
