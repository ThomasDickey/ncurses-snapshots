/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1995                    *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/* panel.c -- implementation of panels library, some core routines */
#include "panel.priv.h"

MODULE_ID("$Id: panel.c,v 1.17 1999/09/18 11:03:56 juergen Exp $")

#ifdef TRACE
#ifndef TRACE_TXT
const char *_nc_my_visbuf(const void *ptr)
{
	char temp[32];
	if (ptr != 0)
		sprintf(temp, "ptr:%p", ptr);
	else
		strcpy(temp, "<null>");
	return _nc_visbuf(temp);
}
#endif
#endif


/*+-------------------------------------------------------------------------
	dPanel(text,pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
void
_nc_dPanel(const char *text, const PANEL *pan)
{
	_tracef("%s id=%s b=%s a=%s y=%d x=%d",
		text, USER_PTR(pan->user),
		(pan->below) ?  USER_PTR(pan->below->user) : "--",
		(pan->above) ?  USER_PTR(pan->above->user) : "--",
		pan->wstarty, pan->wstartx);
}
#endif

/*+-------------------------------------------------------------------------
	dStack(fmt,num,pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
void
_nc_dStack(const char *fmt, int num, const PANEL *pan)
{
  char s80[80];

  sprintf(s80,fmt,num,pan);
  _tracef("%s b=%s t=%s",s80,
	  (_nc_bottom_panel) ?  USER_PTR(_nc_bottom_panel->user) : "--",
	  (_nc_top_panel)    ?  USER_PTR(_nc_top_panel->user)    : "--");
  if(pan)
    _tracef("pan id=%s", USER_PTR(pan->user));
  pan = _nc_bottom_panel;
  while(pan)
    {
      dPanel("stk",pan);
      pan = pan->above;
    }
}
#endif

/*+-------------------------------------------------------------------------
	Wnoutrefresh(pan) - debugging hook for wnoutrefresh
--------------------------------------------------------------------------*/
#ifdef TRACE
void
_nc_Wnoutrefresh(const PANEL *pan)
{
  dPanel("wnoutrefresh",pan);
  wnoutrefresh(pan->win);
}
#endif

/*+-------------------------------------------------------------------------
	Touchpan(pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
void
_nc_Touchpan(const PANEL *pan)
{
  dPanel("Touchpan",pan);
  touchwin(pan->win);
}
#endif

/*+-------------------------------------------------------------------------
	Touchline(pan,start,count)
--------------------------------------------------------------------------*/
#ifdef TRACE
void
_nc_Touchline(const PANEL *pan, int start, int count)
{
  char s80[80];
  sprintf(s80,"Touchline s=%d c=%d",start,count);
  dPanel(s80,pan);
  touchline(pan->win,start,count);
}
#endif

/*+-------------------------------------------------------------------------
	__panels_overlapped(pan1,pan2) - check panel overlapped
--------------------------------------------------------------------------*/
static INLINE bool
__panels_overlapped(register const PANEL *pan1, register const PANEL *pan2)
{
  if(!pan1 || !pan2)
    return(FALSE);

  dBug(("__panels_overlapped %s %s", USER_PTR(pan1->user), USER_PTR(pan2->user)));
  /* pan1 intersects with pan2 ? */
  if( (((pan1->wstarty >= pan2->wstarty) && (pan1->wstarty < pan2->wendy)) ||
       ((pan2->wstarty >= pan1->wstarty) && (pan2->wstarty < pan1->wendy))) &&
      (((pan1->wstartx >= pan2->wstartx) && (pan1->wstartx < pan2->wendx)) ||
       ((pan2->wstartx >= pan1->wstartx) && (pan2->wstartx < pan1->wendx)))
      ) return(TRUE);
  else {
    dBug(("  no"));
    return(FALSE);
  }
}

/*+-------------------------------------------------------------------------
	_nc_free_obscure(pan)
--------------------------------------------------------------------------*/
void
_nc_free_obscure(PANEL *pan)
{
  PANELCONS *tobs = pan->obscure;			/* "this" one */
  PANELCONS *nobs;					/* "next" one */

  while(tobs)
    {
      nobs = tobs->above;
      free((char *)tobs);
      tobs = nobs;
    }
  pan->obscure = (PANELCONS *)0;
}

/*+-------------------------------------------------------------------------
	__override(pan,show)
--------------------------------------------------------------------------*/
void
_nc_override(const PANEL *pan, int show)
{
  int y;
  PANEL *pan2;
  PANELCONS *tobs = pan->obscure;			   /* "this" one */

  dBug(("_nc_override %s,%d", USER_PTR(pan->user),show));

  switch (show)
    {
    case P_TOUCH:
      Touchpan(pan);
      /* The following while loop will now mark all panel window lines
       * obscured by use or obscuring us as touched, so they will be
       * updated.
       */
      break;
    case P_UPDATE:
      while(tobs && (tobs->pan != pan))
	tobs = tobs->above;
      /* The next loop will now only go through the panels obscuring pan;
       * it updates all the lines in the obscuring panels in sync. with
       * the lines touched in pan itself. This is called in update_panels()
       * in a loop from the bottom_panel to the top_panel, resulting in
       * the desired update effect.
       */
      break;
    default:
      return;
    }

  while(tobs)
    {
      if((pan2 = tobs->pan) != pan) {
	dBug(("test obs pan=%s pan2=%s", USER_PTR(pan->user), USER_PTR(pan2->user)));
	for(y = pan->wstarty; y < pan->wendy; y++) {
	  if( (y >= pan2->wstarty) && (y < pan2->wendy) &&
	      ((is_linetouched(pan->win,y - pan->wstarty) == TRUE)) )
	    Touchline(pan2,y - pan2->wstarty,1);
	}
      }
      tobs = tobs->above;
    }
}

/*+-------------------------------------------------------------------------
	__calculate_obscure()
--------------------------------------------------------------------------*/
void
_nc_calculate_obscure(void)
{
  PANEL *pan;
  PANEL *pan2;
  PANELCONS *tobs;			/* "this" one */
  PANELCONS *lobs = (PANELCONS *)0;	/* last one */

  pan = _nc_bottom_panel;
  while(pan)
    {
      if(pan->obscure)
	_nc_free_obscure(pan);
      dBug(("--> __calculate_obscure %s", USER_PTR(pan->user)));
      lobs = (PANELCONS *)0;		/* last one */
      pan2 = _nc_bottom_panel;
      /* This loop builds a list of panels obsured by pan or obscuring
	 pan; pan itself is in the list; all panels before pan are
	 obscured by pan, all panels after pan are obscuring pan. */
      while(pan2)
	{
	  if(__panels_overlapped(pan,pan2))
	    {
	      if(!(tobs = (PANELCONS *)malloc(sizeof(PANELCONS))))
		return;
	      tobs->pan = pan2;
	      dPanel("obscured",pan2);
	      tobs->above = (PANELCONS *)0;
	      if(lobs)
		lobs->above = tobs;
	      else
		pan->obscure = tobs;
	      lobs  = tobs;
	    }
	  pan2 = pan2->above;
	}
      _nc_override(pan,P_TOUCH);
      pan = pan->above;
    }
}



