/******************************************************************************
 * Copyright 1996 by Thomas E. Dickey <dickey@clark.net>                      *
 * All Rights Reserved.                                                       *
 *                                                                            *
 * Permission to use, copy, modify, and distribute this software and its      *
 * documentation for any purpose and without fee is hereby granted, provided  *
 * that the above copyright notice appear in all copies and that both that    *
 * copyright notice and this permission notice appear in supporting           *
 * documentation, and that the name of the above listed copyright holder(s)   *
 * not be used in advertising or publicity pertaining to distribution of the  *
 * software without specific, written prior permission. THE ABOVE LISTED      *
 * COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,  *
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO     *
 * EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY         *
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER       *
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF       *
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN        *
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                   *
 ******************************************************************************/
/* $Id: test.priv.h,v 1.6 1996/07/07 18:12:00 tom Exp $ */
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

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
