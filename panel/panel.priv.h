/* $Id: panel.priv.h,v 1.2 1996/12/21 15:31:02 tom Exp $ */

#ifndef _PANEL_PRIV_H
#define _PANEL_PRIV_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>

#if HAVE_LIBDMALLOC
#include <dmalloc.h>    /* Gray Watson's library */
#endif

#if HAVE_LIBDBMALLOC
#include <dbmalloc.h>   /* Conor Cahill's library */
#endif

#include "panel.h"


#if ( CC_HAS_INLINE_FUNCS && !defined(TRACE) )
#  define INLINE inline
#else
#  define INLINE
#endif


typedef struct panelcons
{
  struct panelcons *above;
  struct panel *pan;
} PANELCONS;

#ifdef TRACE
#  define dBug(x) _tracef x
#else
#  define dBug(x)
#endif

#define MODULE_ID(s)

#define P_TOUCH  (0)
#define P_UPDATE (1)

#endif
