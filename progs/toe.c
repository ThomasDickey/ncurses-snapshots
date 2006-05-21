/****************************************************************************
 * Copyright (c) 1998-2005,2006 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	toe.c --- table of entries report generator
 */

#include <progs.priv.h>

#include <sys/stat.h>

#include <dump_entry.h>

MODULE_ID("$Id: toe.c,v 1.35 2006/05/20 17:03:10 tom Exp $")

#define isDotname(name) (!strcmp(name, ".") || !strcmp(name, ".."))

const char *_nc_progname;

#if NO_LEAKS
#undef ExitProgram
static void ExitProgram(int code) GCC_NORETURN;
static void
ExitProgram(int code)
{
    _nc_free_entries(_nc_head);
    _nc_leaks_dump_entry();
    _nc_free_and_exit(code);
}
#endif

static bool
is_a_file(char *path)
{
    struct stat sb;
    return (stat(path, &sb) == 0
	    && (sb.st_mode & S_IFMT) == S_IFREG);
}

static bool
is_a_directory(char *path)
{
    struct stat sb;
    return (stat(path, &sb) == 0
	    && (sb.st_mode & S_IFMT) == S_IFDIR);
}

static char *
get_directory(char *path)
{
    if (path != 0) {
	if (!is_a_directory(path)
	    || access(path, R_OK | X_OK) != 0)
	    path = 0;
    }
    return path;
}

static void
deschook(const char *cn, TERMTYPE *tp)
/* display a description for the type */
{
    const char *desc;

    if ((desc = strrchr(tp->term_names, '|')) == 0)
	desc = "(No description)";
    else
	++desc;

    (void) printf("%-10s\t%s\n", cn, desc);
}

static int
typelist(int eargc, char *eargv[],
	 bool verbosity,
	 void (*hook) (const char *, TERMTYPE *tp))
/* apply a function to each entry in given terminfo directories */
{
    int i;

    for (i = 0; i < eargc; i++) {
	DIR *termdir;
	DIRENT *subdir;

	if ((termdir = opendir(eargv[i])) == 0) {
	    (void) fflush(stdout);
	    (void) fprintf(stderr,
			   "%s: can't open terminfo directory %s\n",
			   _nc_progname, eargv[i]);
	    return (EXIT_FAILURE);
	} else if (verbosity)
	    (void) printf("#\n#%s:\n#\n", eargv[i]);

	while ((subdir = readdir(termdir)) != 0) {
	    size_t len = NAMLEN(subdir);
	    char buf[PATH_MAX];
	    char name_1[PATH_MAX];
	    DIR *entrydir;
	    DIRENT *entry;

	    strncpy(name_1, subdir->d_name, len)[len] = '\0';
	    if (isDotname(name_1))
		continue;

	    (void) sprintf(buf, "%s/%s/", eargv[i], name_1);
	    if (chdir(buf) != 0)
		continue;

	    entrydir = opendir(".");
	    while ((entry = readdir(entrydir)) != 0) {
		char name_2[PATH_MAX];
		TERMTYPE lterm;
		char *cn;
		int status;

		len = NAMLEN(entry);
		strncpy(name_2, entry->d_name, len)[len] = '\0';
		if (isDotname(name_2) || !is_a_file(name_2))
		    continue;

		status = _nc_read_file_entry(name_2, &lterm);
		if (status <= 0) {
		    (void) fflush(stdout);
		    (void) fprintf(stderr,
				   "toe: couldn't open terminfo file %s.\n",
				   name_2);
		    return (EXIT_FAILURE);
		}

		/* only visit things once, by primary name */
		cn = _nc_first_name(lterm.term_names);
		if (!strcmp(cn, name_2)) {
		    /* apply the selected hook function */
		    (*hook) (cn, &lterm);
		}
		_nc_free_termtype(&lterm);
	    }
	    closedir(entrydir);
	}
	closedir(termdir);
    }

    return (EXIT_SUCCESS);
}

static void
usage(void)
{
    (void) fprintf(stderr, "usage: toe [-ahuUV] [-v n] [file...]\n");
    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    bool all_dirs = FALSE;
    bool direct_dependencies = FALSE;
    bool invert_dependencies = FALSE;
    bool header = FALSE;
    int i;
    int code;
    int this_opt, last_opt = '?';
    int v_opt = 0;

    _nc_progname = _nc_rootname(argv[0]);

    while ((this_opt = getopt(argc, argv, "0123456789ahuvUV")) != EOF) {
	/* handle optional parameter */
	if (isdigit(this_opt)) {
	    switch (last_opt) {
	    case 'v':
		v_opt = (this_opt - '0');
		break;
	    default:
		if (isdigit(last_opt))
		    v_opt *= 10;
		else
		    v_opt = 0;
		v_opt += (this_opt - '0');
		last_opt = this_opt;
	    }
	    continue;
	}
	switch (this_opt) {
	case 'a':
	    all_dirs = TRUE;
	    break;
	case 'h':
	    header = TRUE;
	    break;
	case 'u':
	    direct_dependencies = TRUE;
	    break;
	case 'v':
	    v_opt = 1;
	    break;
	case 'U':
	    invert_dependencies = TRUE;
	    break;
	case 'V':
	    puts(curses_version());
	    ExitProgram(EXIT_SUCCESS);
	default:
	    usage();
	}
    }
    set_trace_level(v_opt);

    if (direct_dependencies || invert_dependencies) {
	if (freopen(argv[optind], "r", stdin) == 0) {
	    (void) fflush(stdout);
	    fprintf(stderr, "%s: can't open %s\n", _nc_progname, argv[optind]);
	    ExitProgram(EXIT_FAILURE);
	}

	/* parse entries out of the source file */
	_nc_set_source(argv[optind]);
	_nc_read_entry_source(stdin, 0, FALSE, FALSE, NULLHOOK);
    }

    /* maybe we want a direct-dependency listing? */
    if (direct_dependencies) {
	ENTRY *qp;

	for_entry_list(qp) {
	    if (qp->nuses) {
		int j;

		(void) printf("%s:", _nc_first_name(qp->tterm.term_names));
		for (j = 0; j < qp->nuses; j++)
		    (void) printf(" %s", qp->uses[j].name);
		putchar('\n');
	    }
	}

	ExitProgram(EXIT_SUCCESS);
    }

    /* maybe we want a reverse-dependency listing? */
    if (invert_dependencies) {
	ENTRY *qp, *rp;
	int matchcount;

	for_entry_list(qp) {
	    matchcount = 0;
	    for_entry_list(rp) {
		if (rp->nuses == 0)
		    continue;

		for (i = 0; i < rp->nuses; i++)
		    if (_nc_name_match(qp->tterm.term_names,
				       rp->uses[i].name, "|")) {
			if (matchcount++ == 0)
			    (void) printf("%s:",
					  _nc_first_name(qp->tterm.term_names));
			(void) printf(" %s",
				      _nc_first_name(rp->tterm.term_names));
		    }
	    }
	    if (matchcount)
		putchar('\n');
	}

	ExitProgram(EXIT_SUCCESS);
    }

    /*
     * If we get this far, user wants a simple terminal type listing.
     */
    if (optind < argc) {
	code = typelist(argc - optind, argv + optind, header, deschook);
    } else if (all_dirs) {
	DBDIRS state;
	int offset;
	int pass;
	const char *path;
	char **eargv = 0;

	code = EXIT_FAILURE;
	for (pass = 0; pass < 2; ++pass) {
	    int count = 0;

	    _nc_first_db(&state, &offset);
	    while ((path = _nc_next_db(&state, &offset)) != 0) {
		if (eargv != 0) {
		    int n, found = FALSE;

		    /* eliminate duplicates */
		    for (n = 0; n < count; ++n) {
			if (!strcmp(path, eargv[n])) {
			    found = TRUE;
			    break;
			}
		    }
		    if (!found) {
			eargv[count] = strdup(path);
			++count;
		    }
		} else {
		    ++count;
		}
	    }
	    if (!pass) {
		eargv = typeCalloc(char *, count + 1);
	    } else {
		code = typelist(count, eargv, header, deschook);
	    }
	}
    } else {
	static char system_db[] = TERMINFO;
	char *eargv[3];
	int j;

	j = 0;
	if ((eargv[j] = get_directory(getenv("TERMINFO"))) != 0) {
	    j++;
	} else {
	    if ((eargv[j] = get_directory(_nc_home_terminfo())) != 0)
		j++;
	    if ((eargv[j] = get_directory(system_db)) != 0)
		j++;
	}
	eargv[j] = 0;

	code = typelist(j, eargv, header, deschook);
    }

    ExitProgram(code);
}
