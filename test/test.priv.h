/* $Id: test.priv.h,v 1.4 1996/07/04 13:37:47 tom Exp $ */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <sys/types.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <curses.h>

#if HAVE_GETOPT_H
#include <getopt.h>
#else
/* 'getopt()' may be prototyped in <stdlib.h>, but declaring its variables
 * doesn't hurt.
 */
extern char *optarg;
extern int optind;
#endif /* HAVE_GETOPT_H */

#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
