
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
 *	comp_error.c -- Error message routines
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#include "tic.h"

static const char *sourcename;

void set_source(const char *name)
{
    sourcename = name;
}

void warning(const char *fmt, ...)
{
    va_list argp;

    va_start(argp,fmt);
    fprintf (stderr, "compile: Warning: near line %d: ", curr_line);
    fprintf (stderr, "terminal '%s', ", sourcename);
    vfprintf (stderr, fmt, argp);
    fprintf (stderr, "\n");
    va_end(argp);
}


void err_abort(const char *fmt, ...)
{
    va_list argp;

    va_start(argp,fmt);
    fprintf (stderr, "compile: Line %d: ", curr_line);
    fprintf (stderr, "terminal '%s', ", sourcename);
    vfprintf (stderr, fmt, argp);
    fprintf (stderr, "\n");
    va_end(argp);
    exit(1);
}


void syserr_abort(const char *fmt, ...)
{
    va_list argp;

    va_start(argp,fmt);
    fprintf (stderr, "PROGRAM ERROR: Line %d: ", curr_line);
    fprintf (stderr, "terminal '%s', ", sourcename);
    vfprintf (stderr, fmt, argp);
    fprintf (stderr, "\n");
    va_end(argp);
    abort();
}
