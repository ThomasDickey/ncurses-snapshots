
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


/* This file exist to handle non-POSIX systems which don't
   have <unistd.h>, and usually no sigaction() nor
   <termios.h>
*/

#ifndef _SIGACTION_H
#define _SIGACTION_H

#include <libc.h>

typedef struct sigvec sigaction_t;

#define sa_mask sv_mask
#define sa_handler sv_handler
#define sa_flags sv_flags

extern void sigaction (int sig, sigaction_t * sigact, sigaction_t *  osigact);
extern void sigprocmask (int mode, int *mask, int *omask);
extern void sigemptyset (int *mask);
extern int sigsuspend (int *mask);
extern void sigdelset (int *mask, int sig);
extern void sigaddset (int *mask, int sig);

#endif

