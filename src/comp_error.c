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
 *	comp_error.c -- Error message routines
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "tic.h"

#define NAMESIZE 64

static const char *sourcename;
static char termtype[NAMESIZE];

void set_source(const char *name)
{
	sourcename = name;
}

void set_type(const char *name)
{
	if (name)
		strncpy( termtype, name, NAMESIZE );
	else
		termtype[0] = '\0';
}

void warning(const char *fmt, ...)
{
va_list argp;

	va_start(argp,fmt);
	fprintf (stderr, "\"%s\", line %d: ", sourcename, curr_line);
	if (termtype[0])
		fprintf (stderr, "terminal '%s', ", termtype);
	vfprintf (stderr, fmt, argp);
	fprintf (stderr, "\n");
	va_end(argp);
}


void err_abort(const char *fmt, ...)
{
va_list argp;

	va_start(argp,fmt);
	fprintf (stderr, "\"%s\", line %d: ", sourcename, curr_line);
	if (termtype[0])
		fprintf (stderr, "terminal '%s', ", termtype);
	vfprintf (stderr, fmt, argp);
	fprintf (stderr, "\n");
	va_end(argp);
	exit(1);
}


void syserr_abort(const char *fmt, ...)
{
va_list argp;

	va_start(argp,fmt);
	fprintf (stderr, "\"%s\", line %d: ", sourcename, curr_line);
	if (termtype[0])
		fprintf (stderr, "terminal '%s', ", termtype);
	vfprintf (stderr, fmt, argp);
	fprintf (stderr, "\n");
	va_end(argp);
	abort();
}
