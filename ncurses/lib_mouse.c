
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

/*
 * This module is intended to encapsulate ncurses's interface to pointing
 * devices. 
 *
 * The first method used is xterm's internal mouse-tracking facility.
 * The second (not yet implemented) will be Alessandro Rubini's GPM server.
 *
 * Notes for implementors of new mouse-interface methods:
 *
 * The code is logically split into a lower level that accepts event reports
 * in a device-dependent format and an upper level that parses mouse gestures
 * and filters events.  The mediating data structure is a circular queue of
 * MEVENT structures.
 *
 * Functionally, the lower level's job is to pick up primitive events and
 * put them on the circular queue.  This can happen in one of two ways:
 * either (a) _nc_mouse_event() detects a series of incoming mouse reports
 * and queues them, or (b) code in lib_getch.c detects the kmous prefix in
 * the keyboard input stream and calls _nc_mouse_inline to queue up a series
 * of adjacent mouse reports.
 *
 * In either case, _nc_mouse_parse() should be called after the series is
 * accepted to parse the digested mouse reports (low-level MEVENTs) into
 * a gesture (a high-level or composite MEVENT).
 *
 * Don't be too shy about adding new event types or modifiers, if you can find
 * room for them in the 32-bit mask.  The API is written so that users get
 * feedback on which theoretical event types they won't see when they call
 * mousemask. There's one bit per button (the RESERVED_EVENT bit) not being
 * used yet, and a couple of bits open at the high end.
 */

#include <curses.h>
#include <string.h>
#include "term.h"
#include "curses.priv.h"

#define INVALID_EVENT	-1

int _nc_max_click_interval = 166;	/* max press/release separation */

static int		mousetype;
#define M_XTERM		-1	/* use xterm's mouse tracking? */
#define M_NONE		0	/* no mouse device */
#define M_GPM		1	/* use GPM (not yet implemented) */

static mmask_t	eventmask;		/* current event mask */

/* maintain a circular list of mouse events */
#define EV_MAX		8		/* size of circular event queue */
static MEVENT	events[EV_MAX];		/* hold the last mouse event seen */
static MEVENT	*eventp = events;	/* next free slot in event queue */
#define NEXT(ep)	((ep == events + EV_MAX - 1) ? events : ep + 1)
#define PREV(ep)	((ep == events) ? events + EV_MAX - 1 : ep - 1)

void _nc_mouse_init(SCREEN *sp)
/* initialize the mouse -- called at screen-setup time */
{
    int i;

    T(("_nc_mouse_init() called"));

    for (i = 0; i < EV_MAX; i++)
	events[i].id = INVALID_EVENT;

    /* we know how to recognize mouse events under xterm */
    if (!strncmp(cur_term->type.term_names, "xterm", 5) && key_mouse)
	mousetype = M_XTERM;

    /* GPM: initialize connection to gpm server */
}

bool _nc_mouse_event(SCREEN *sp)
/* query to see if there is a pending mouse event */
{
    /* xterm: never have to query, mouse events are  in the keyboard stream */
    /* GPM: query server for event, return TRUE if we find one */
    return(FALSE);	/* no event waiting */
}

bool _nc_mouse_inline(SCREEN *sp)
/* mouse report received in the keyboard stream -- parse its info */
{
    T(("_nc_mouse_inline() called"));

    if (mousetype == M_XTERM)
    {
	char	kbuf[4];
	MEVENT	*prev;

	/* This code requires that your xterm entry contain the kmous
	 * capability and that it be set to the \E[M documented in the
	 * Xterm Control Sequences reference.  This is how we
	 * arrange for mouse events to be reported via a KEY_MOUSE
	 * return value from wgetch().  After this value is received,
	 * _nc_mouse_inline() gets called and is immediately
	 * responsible for parsing the mouse status information
	 * following the prefix.
	 *
	 * The following quotes from the ctrlseqs.ms document in the
	 * X distribution, describing the X mouse tracking feature:
	 *
	 * Parameters for all mouse tracking escape sequences
	 * generated by xterm encode numeric parameters in a single
	 * character as value+040.  For example, !  is 1.
	 *
	 * On button press or release, xterm sends ESC [ M CbCxCy.
	 * The low two bits of Cb encode button information: 0=MB1
	 * pressed, 1=MB2 pressed, 2=MB3 pressed, 3=release.  The
	 * upper bits encode what modifiers were down when the
	 * button was pressed and are added together.  4=Shift,
	 * 8=Meta, 16=Control.  Cx and Cy are the x and y coordinates
	 * of the mouse event.  The upper left corner is (1,1).
	 *
	 * (End quote)  By the time we get here, we've eaten the
	 * key prefix.
	 */
	(void) read(sp->_ifd, &kbuf, 3);
	kbuf[3] = '\0';

	T(("_nc_mouse_inline sees the following xterm data: '%s'", kbuf));

	eventp->id = 0;		/* there's only one mouse... */

	/* processing code goes here */
	eventp->bstate = 0;
	switch (kbuf[0] & 0x3)
	{
	case 0x0:
	    T(("...mouse button-1 pressed"));
	    eventp->bstate = BUTTON1_PRESSED;
	    break;

	case 0x1:
	    T(("...mouse button-2 pressed"));
	    eventp->bstate = BUTTON2_PRESSED;
	    break;

	case 0x2:
	    T(("...mouse button-3 pressed"));
	    eventp->bstate = BUTTON3_PRESSED;
	    break;

	case 0x3:
	    T(("...mouse button released"));
	    /*
	     * Release events aren't reported for individual buttons,
	     * just for the button set as a whole...
	     */
	    eventp->bstate = 
		(BUTTON1_RELEASED |
		 BUTTON2_RELEASED |
		 BUTTON3_RELEASED);
	    /*
	     * ...however, because there are no kinds of mouse events under
	     * xterm that can intervene between press and release, we can
	     * deduce which buttons were actually released by looking at the
	     * previous event.
	     */
	    prev = PREV(eventp);
	    if (prev->id != INVALID_EVENT)		/* this should always be true */
	    {
		if (!(prev->bstate & BUTTON1_PRESSED))
		    eventp->bstate &=~ BUTTON1_RELEASED;
		if (!(prev->bstate & BUTTON2_PRESSED))
		    eventp->bstate &=~ BUTTON2_RELEASED;
		if (!(prev->bstate & BUTTON3_PRESSED))
		    eventp->bstate &=~ BUTTON3_RELEASED;
	    }
	    break;
	}
	if (kbuf[0] & 4) {
	    T(("mouse event: SHIFT"));
	    eventp->bstate |= BUTTON_SHIFT;
	}
	if (kbuf[0] & 8) {
	    T(("mouse event: ALT"));
	    eventp->bstate |= BUTTON_ALT;
	}
	if (kbuf[0] & 16) {
	    T(("mouse event: CTRL"));
	    eventp->bstate |= BUTTON_CTRL;
	}

	eventp->x = (kbuf[1] - ' ') - 1; 
	eventp->y = (kbuf[2] - ' ') - 1; 
	T(("mouse-event at (%d,%d)", eventp->y, eventp->x));

	/* bump the next-free pointer into the circular list */
	eventp = NEXT(eventp);
    }

    return(FALSE);
}

static void mouse_activate(bool on)
{
    if (mousetype == M_XTERM)
    {
	if (on)
	{
	    TPUTS_TRACE("xterm mouse initialization");
	    putp("\033[?1000h");
	}
	else
	{
	    TPUTS_TRACE("xterm mouse deinitialization");
	    putp("\033[?1000l");
	}
	(void) fflush(SP->_ofp);
    }
}

/**************************************************************************
 *
 * Device-independent code
 *
 **************************************************************************/

bool _nc_mouse_parse(int runcount)
/* parse a run of atomic mouse events into a gesture */
{
    MEVENT	*runp;
    int		n;
    bool	merge;

    /*
     * When we enter this routine, the event list next-free pointer
     * points just past a run of mouse events that we know were separated
     * in time by less than the critical click interval. The job of this
     * routine is to collaps this run into a single higher-level event
     * or gesture.
     *
     * We accomplish this in two passes.  The first pass merges press/release
     * pairs into click events.  The second merges runs of click events into
     * double or triple-click events.
     *
     * It's possible that the run may not resolve to a single event (for 
     * example, if the user quadruple-clicks).  If so, leading events
     * in the run are ignored.
     *
     * Note that this routine is independent of the format of the specific
     * format of the pointing-device's reports.  We can use it to parse
     * gestures on anything that reports press/release events on a per-
     * button basis, as long as the device-dependent mouse code puts stuff
     * on the queue in MEVENT format.
     */
    if (runcount == 1)
	return(PREV(eventp)->bstate & eventmask);

    /* find the start of the run */
    runp = eventp;
    for (n = runcount; n > 0; n--)
	runp = PREV(runp);

    /* first pass; merge press/release pairs */
    do {
	MEVENT	*ep, *next;

	merge = FALSE;
	for (ep = runp; next = NEXT(ep), next != eventp; ep = next)
	{
	    if (ep->x == next->x && ep->y == next->y
		&& (ep->bstate & (BUTTON1_PRESSED|BUTTON2_PRESSED|BUTTON3_PRESSED))
		&& (!(ep->bstate & BUTTON1_PRESSED)
		    == !(next->bstate & BUTTON1_RELEASED))
		&& (!(ep->bstate & BUTTON2_PRESSED)
		    == !(next->bstate & BUTTON2_RELEASED))
		&& (!(ep->bstate & BUTTON3_PRESSED)
		    == !(next->bstate & BUTTON3_RELEASED))
		)
	    {
		if ((eventmask & BUTTON1_CLICKED)
			&& (ep->bstate & BUTTON1_PRESSED))
		{
		    ep->bstate &=~ BUTTON1_PRESSED;
		    ep->bstate |= BUTTON1_CLICKED;
		    merge = TRUE;
		}
		if ((eventmask & BUTTON2_CLICKED)
			&& (ep->bstate & BUTTON2_PRESSED))
		{
		    ep->bstate &=~ BUTTON2_PRESSED;
		    ep->bstate |= BUTTON2_CLICKED;
		    merge = TRUE;
		}
		if ((eventmask & BUTTON3_CLICKED)
			&& (ep->bstate & BUTTON3_PRESSED))
		{
		    ep->bstate &=~ BUTTON3_PRESSED;
		    ep->bstate |= BUTTON3_CLICKED;
		    merge = TRUE;
		}
		if (merge)
		    next->id = INVALID_EVENT;
	    }
	}
    } while
	(merge);
    
    /* 
     * Second pass; merge click runs.  At this point, click events are
     * each followed by one invalid event. We merge click events
     * forward in the queue.
     */
    do {
	MEVENT	*ep, *next, *follower;

	merge = FALSE;
	for (ep = runp; next = NEXT(ep), next != eventp; ep = next)
	    if (ep->id != INVALID_EVENT)
	    {
		if (next->id != INVALID_EVENT)
		    continue;
		follower = NEXT(next);
		if (follower->id == INVALID_EVENT)
		    continue;

		/* merge click events forward */
		if ((ep->bstate &
			(BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED))
		    && (follower->bstate &
		    	(BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED)))
		{
		    if ((eventmask & BUTTON1_DOUBLE_CLICKED)
			&& (follower->bstate & BUTTON1_CLICKED))
		    {
			follower->bstate &=~ BUTTON1_CLICKED;
			follower->bstate |= BUTTON1_DOUBLE_CLICKED;
			merge = TRUE;
		    }
		    if ((eventmask & BUTTON2_DOUBLE_CLICKED)
			&& (follower->bstate & BUTTON2_CLICKED))
		    {
			follower->bstate &=~ BUTTON2_CLICKED;
			follower->bstate |= BUTTON2_DOUBLE_CLICKED;
			merge = TRUE;
		    }
		    if ((eventmask & BUTTON3_DOUBLE_CLICKED)
			&& (follower->bstate & BUTTON3_CLICKED))
		    {
			follower->bstate &=~ BUTTON3_CLICKED;
			follower->bstate |= BUTTON3_DOUBLE_CLICKED;
			merge = TRUE;
		    }
		    if (merge)
			ep->id = INVALID_EVENT;
		}

		/* merge double-click events forward */
		if ((ep->bstate &
			(BUTTON1_DOUBLE_CLICKED 
			 | BUTTON2_DOUBLE_CLICKED 
			 | BUTTON3_DOUBLE_CLICKED))
		    && (follower->bstate &
		    	(BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED)))
		{
		    if ((eventmask & BUTTON1_TRIPLE_CLICKED)
			&& (follower->bstate & BUTTON1_CLICKED))
		    {
			follower->bstate &=~ BUTTON1_CLICKED;
			follower->bstate |= BUTTON1_TRIPLE_CLICKED;
			merge = TRUE;
		    }
		    if ((eventmask & BUTTON2_TRIPLE_CLICKED)
			&& (follower->bstate & BUTTON2_CLICKED))
		    {
			follower->bstate &=~ BUTTON2_CLICKED;
			follower->bstate |= BUTTON2_TRIPLE_CLICKED;
			merge = TRUE;
		    }
		    if ((eventmask & BUTTON3_TRIPLE_CLICKED)
			&& (follower->bstate & BUTTON3_CLICKED))
		    {
			follower->bstate &=~ BUTTON3_CLICKED;
			follower->bstate |= BUTTON3_TRIPLE_CLICKED;
			merge = TRUE;
		    }
		    if (merge)
			ep->id = INVALID_EVENT;
		}
	    }
    } while
	(merge);

    /*
     * Now try to throw away trailing events flagged invalid, or that
     * don't match the current event mask.
     */
    while (runcount--)
    {
	MEVENT	*prev = PREV(eventp);

	if (prev->id == INVALID_EVENT || !(prev->bstate & eventmask))
	    eventp = prev;
    }

    /* after all this, do we have a valid event? */
    return(PREV(eventp)->id != INVALID_EVENT);
}

void _nc_mouse_wrap(SCREEN *sp)
/* release mouse -- called by endwin() before shellout/exit */
{
    T(("_nc_mouse_wrap() called"));

    /* xterm: turn off reporting */
    if (mousetype == M_XTERM && eventmask)
    {
	mouse_activate(FALSE);
	eventmask = 0;
    }

    /* GPM: pass all mouse events to next client */
}

void _nc_mouse_resume(SCREEN *sp)
/* re-connect to mouse -- called by doupdate() after shellout */
{
    T(("_nc_mouse_resume() called"));

    /* xterm: re-enable reporting */
    if (mousetype == M_XTERM && eventmask)
	mouse_activate(TRUE);

    /* GPM: reclaim our event set */
}

/**************************************************************************
 *
 * Mouse interface entry points for the API
 *
 **************************************************************************/

int getmouse(MEVENT *aevent)
/* grab a copy of the current mouse event */
{
    if (mousetype == M_XTERM)
    {
	/* compute the current-event pointer */
	MEVENT	*prev = PREV(eventp);

	/* copy the event we find there */
	*aevent = *prev;

	return(OK);
    }
    return(ERR);
}

int ungetmouse(MEVENT *aevent)
/* enqueue a synthesized mouse event to be seen by the next wgetch() */
{
    /* stick the given event in the next-free slot */
    *eventp = *aevent;

    /* bump the next-free pointer into the circular list */
    eventp = NEXT(eventp);

    /* push back the notification event on the keyboard queue */
    return ungetch(KEY_MOUSE);
}

mmask_t mousemask(mmask_t newmask, mmask_t *oldmask)
/* set the mouse event mask */
{
    if (oldmask)
	*oldmask = eventmask;

    if (mousetype == M_XTERM)
    {
	eventmask = newmask &
	    (BUTTON_ALT | BUTTON_CTRL | BUTTON_SHIFT
	     | BUTTON1_PRESSED | BUTTON1_RELEASED | BUTTON1_CLICKED
	     | BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED 
	     | BUTTON2_PRESSED | BUTTON2_RELEASED | BUTTON2_CLICKED
	     | BUTTON2_DOUBLE_CLICKED | BUTTON2_TRIPLE_CLICKED 
	     | BUTTON3_PRESSED | BUTTON3_RELEASED | BUTTON3_CLICKED
	     | BUTTON3_DOUBLE_CLICKED | BUTTON3_TRIPLE_CLICKED);

	mouse_activate(eventmask != 0);

	return(eventmask);
    }

    return(0);
}

bool wenclose(WINDOW *win, int y, int x)
/* check to see if given window encloses given screen location */
{
    return
	(win->_begy <= y && win->_begx <= x
	 && win->_maxx >= x && win->_maxy >= y);
}

int mouseinterval(int maxclick)
/* set the maximum mouse interval within which to recognize a click */
{
    int	oldval = _nc_max_click_interval;

    _nc_max_click_interval = maxclick;
    return(oldval);
}

/* lib_mouse.c ends here */
