
/* This file provides sigaction() emulation using sigvec() */
/* Use only if this is non POSIX system */

#ifdef NOACTION
#include <signal.h>
#include "curses.priv.h"

void
sigaction (int sig, sigaction_t * sigact, sigaction_t * osigact)
{
  sigvec(sig, sigact, osigact);
}

void
sigemptyset (int * mask)
{
  *mask = 0;
}

void
sigprocmask (int mode, int * mask, int * omask)
{
  *omask = sigsetmask (0);
}

int
sigsuspend (int * mask)
{
  return sigpause (*mask);
}

void
sigdelset (int * mask, int sig)
{
  *mask &= ~sigmask (sig);
}

void
sigaddset (int * mask, int sig)
{
  *mask |= sigmask (sig);
}
#endif
