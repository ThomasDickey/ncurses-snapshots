
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

#include "curses.priv.h"

/* This file provides sigaction() emulation using sigvec() */
/* Use only if this is non POSIX system */

#if !HAVE_SIGACTION
#include <signal.h>

int
sigaction (int sig, sigaction_t * sigact, sigaction_t * osigact)
{
  return sigvec(sig, sigact, osigact);
}

int
sigemptyset (int * mask)
{
  *mask = 0;
  return 0;
}

int
sigprocmask (int mode, int * mask, int * omask)
{
  *omask = sigsetmask (0);
  return 0;
}

int
sigsuspend (int * mask)
{
  return sigpause (*mask);
}

int
sigdelset (int * mask, int sig)
{
  *mask &= ~sigmask (sig);
  return 0;
}

int
sigaddset (int * mask, int sig)
{
  *mask |= sigmask (sig);
  return 0;
}
#else
void _nc_sigaction(void) { } /* nonempty for strict ANSI compilers */
#endif
