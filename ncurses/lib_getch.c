
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
**	lib_getch.c
**
**	The routine getch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_getch.c,v 1.32 1997/09/09 17:37:14 Alexander.V.Lukyanov Exp $")

#define head	SP->_fifohead
#define tail	SP->_fifotail
/* peek points to next uninterpreted character */
#define peek	SP->_fifopeek

#define h_inc() { head == FIFO_SIZE-1 ? head = 0 : head++; if (head == tail) head = -1, tail = 0;}
#define h_dec() { head == 0 ?  head = FIFO_SIZE-1 : head--; if (head == tail) tail = -1;}
#define t_inc() { tail == FIFO_SIZE-1 ? tail = 0 : tail++; if (tail == head) tail = -1;}
#define t_dec() { tail == 0 ?  tail = FIFO_SIZE-1 : tail--; if (head == tail) fifo_clear();}
#define p_inc() { peek == FIFO_SIZE-1 ? peek = 0 : peek++;}

int ESCDELAY = 1000;	/* max interval betw. chars in funkeys, in millisecs */

static inline int fifo_peek(void)
{
	int ch = SP->_fifo[peek];
	T(("peeking at %d", peek));
	
	p_inc();
	return ch;
}

#define cooked_key_in_fifo()	(head!=-1 && peek!=head)
#define raw_key_in_fifo()	(head!=-1 && peek!=tail)

#ifdef TRACE
static inline void fifo_dump(void)
{
int i;
	T(("head = %d, tail = %d, peek = %d", head, tail, peek));
	for (i = 0; i < 10; i++)
		T(("char %d = %s", i, _trace_key(SP->_fifo[i])));
}
#endif /* TRACE */

static inline int fifo_pull(void)
{
int ch;
	ch = SP->_fifo[head];
	T(("pulling %d from %d", ch, head));

	if (peek == head)
	{
	    h_inc();
	    peek = head;
	}
	else
	    h_inc();
	    
#ifdef TRACE
	if (_nc_tracing & TRACE_IEVENT) fifo_dump();
#endif
	return ch;
}

int ungetch(int ch)
{
	if (tail == -1)
		return ERR;
	if (head == -1) {
		head = 0;
		t_inc()
		peek = tail; /* no raw keys */
	} else
		h_dec();

	SP->_fifo[head] = ch;
	T(("ungetch %#x ok", ch));
#ifdef TRACE
	if (_nc_tracing & TRACE_IEVENT) fifo_dump();
#endif
	return OK;
}

#undef HIDE_EINTR

static inline int fifo_push(void)
{
int n;
unsigned int ch;

	if (tail == -1) return ERR;

#ifdef HIDE_EINTR
again:
	errno = 0;
#endif

#if USE_GPM_SUPPORT	
	if ((_nc_mouse_fd() >= 0) 
	 && (_nc_timed_wait(3, -1, (int *)0) & 2))
	{
		_nc_mouse_event(SP);
		ch = KEY_MOUSE;
		n = 1;
	} else
#endif
	{
		unsigned char c2=0;
		n = read(SP->_ifd, &c2, 1);
		ch = c2;
	}

#ifdef HIDE_EINTR
	/*
	 * Under System V curses with non-restarting signals, getch() returns
	 * with value ERR when a handled signal keeps it from completing.  
	 * If signals restart system calls, OTOH, the signal is invisible
	 * except to its handler.
	 * 
	 * We don't want this difference to show.  This piece of code
	 * tries to make it look like we always have restarting signals.
	 */
	if (n <= 0 && errno == EINTR)
		goto again;
#endif

	if ((n == -1) || (n == 0))
	{
	    T(("read(%d,&ch,1)=%d, errno=%d", SP->_ifd, n, errno));
	    return ERR;
	}
	T(("read %d characters", n));

	SP->_fifo[tail] = ch;
	SP->_fifohold = 0;
	if (head == -1)
	    head = peek = tail;
	t_inc();
	T(("pushed %#x at %d", ch, tail));
#ifdef TRACE
	if (_nc_tracing & TRACE_IEVENT) fifo_dump();
#endif
	return ch;
}

static inline void fifo_clear(void)
{
int i;
	for (i = 0; i < FIFO_SIZE; i++)
		SP->_fifo[i] = 0;
	head = -1; tail = peek = 0;
}

static int kgetch(WINDOW *);

#define wgetch_should_refresh(win) (\
	(is_wintouched(win) || (win->_flags & _HASMOVED)) \
	&& !(win->_flags & _ISPAD))

int
wgetch(WINDOW *win)
{
int	ch;

	T((T_CALLED("wgetch(%p)"), win));

	if (cooked_key_in_fifo())
	{
		if (wgetch_should_refresh(win))
			wrefresh(win);

		ch = fifo_pull();
    		T(("wgetch returning (pre-cooked): %#x = %s", ch, _trace_key(ch));)
		returnCode(ch);
	}

	/*
	 * Handle cooked mode.  Grab a string from the screen,
	 * stuff its contents in the FIFO queue, and pop off
	 * the first character to return it.
	 */
	if (head == -1 && !SP->_raw && !SP->_cbreak)
	{
		char	buf[MAXCOLUMNS], *sp;

		T(("filling queue in cooked mode"));

		wgetnstr(win, buf, MAXCOLUMNS);

		/* ungetch in reverse order */
		ungetch('\n');
		for (sp = buf+strlen(buf); sp>buf; sp--)
			ungetch(sp[-1]);

		returnCode(fifo_pull());
	}

	if (wgetch_should_refresh(win))
		wrefresh(win);

	if (!win->_notimeout && (win->_delay >= 0 || SP->_cbreak > 1)) {
	int delay;

		T(("timed delay in wgetch()"));
		if (SP->_cbreak > 1)
		    delay = (SP->_cbreak-1) * 100;
		else
		    delay = win->_delay;

		T(("delay is %d microseconds", delay));

		if (head == -1)	/* fifo is empty */
			if (!_nc_timed_wait(3, delay, (int *)0))
				returnCode(ERR);
		/* else go on to read data available */
	}

	if (win->_use_keypad) {
		/*
		 * This is tricky.  We only want to get special-key
		 * events one at a time.  But we want to accumulate
		 * mouse events until either (a) the mouse logic tells
		 * us it's picked up a complete gesture, or (b)
		 * there's a detectable time lapse after one.
		 *
		 * Note: if the mouse code starts failing to compose
		 * press/release events into clicks, you should probably
		 * increase _nc_max_click_interval.
		 */
		int runcount = 0;

		do {
			ch = kgetch(win);
			if (ch == KEY_MOUSE)
			{
				++runcount;
				if (_nc_mouse_inline(SP))
				    break;
			}
		} while
		    (ch == KEY_MOUSE
		     && (_nc_timed_wait(3, _nc_max_click_interval, (int *)0)
			 || !_nc_mouse_parse(runcount)));
		if (runcount > 0 && ch != KEY_MOUSE)
		{
		    /* mouse event sequence ended by keystroke, push it */
		    ungetch(ch);
		    ch = KEY_MOUSE;
		}
	} else {
		if (head == -1)
			fifo_push();
		ch = fifo_pull();
	}

	if (ch == ERR)
	{
	    if(SP->_sig_winch)
	    {
		_nc_get_screensize();
		resizeterm(LINES, COLS);
		/* resizeterm can push KEY_RESIZE */
		if(cooked_key_in_fifo())
		{
		    ch = fifo_pull();
		    T(("wgetch returning (pre-cooked): %#x = %s", ch, _trace_key(ch));)
		    returnCode(ch);
		}
	    }
	    T(("wgetch returning ERR"));
	    returnCode(ERR);
	}

	/*
	 * Simulate ICRNL mode
	 */
	if ((ch == '\r') && SP->_nl)
		ch = '\n';

	/* Strip 8th-bit if so desired.  We do this only for characters that
	 * are in the range 128-255, to provide compatibility with terminals
	 * that display only 7-bit characters.  Note that 'ch' may be a
	 * function key at this point, so we mustn't strip _those_.
	 */
	if ((ch < KEY_MIN) && (ch & 0x80))
		if (!SP->_use_meta)
			ch &= 0x7f;

	if (SP->_echo && ch < KEY_MIN && !(win->_flags & _ISPAD))
		wechochar(win, (chtype)ch);

	T(("wgetch returning : %#x = %s", ch, _trace_key(ch));)

	returnCode(ch);
}


/*
**      int
**      kgetch()
**
**      Get an input character, but take care of keypad sequences, returning
**      an appropriate code when one matches the input.  After each character
**      is received, set an alarm call based on ESCDELAY.  If no more of the
**      sequence is received by the time the alarm goes off, pass through
**      the sequence gotten so far.
**
**	This function must be called when there is no cooked keys in queue.
**	(that is head==-1 || peek==head)
**
*/

static int
kgetch(WINDOW *win GCC_UNUSED)
{
struct tries  *ptr;
int ch = 0;
int timeleft = ESCDELAY;

	TR(TRACE_IEVENT, ("kgetch(%p) called", win));

	ptr = SP->_keytry;

	for(;;)
	{
		if (!raw_key_in_fifo())
		{
		    if(fifo_push() == ERR)
		    {
			peek = head;	/* the keys stay uninterpreted */
			return ERR;
		    }
		}
		ch = fifo_peek();
		if (ch >= KEY_MIN)
		{
		    peek = head;
		    /* assume the key is the last in fifo */
		    t_dec(); /* remove the key */
		    return ch;
		}

		TR(TRACE_IEVENT, ("ch: %s", _trace_key((unsigned char)ch)));
		while ((ptr != NULL) && (ptr->ch != (unsigned char)ch))
			ptr = ptr->sibling;
#ifdef TRACE
		if (ptr == NULL)
			{TR(TRACE_IEVENT, ("ptr is null"));}
		else
			TR(TRACE_IEVENT, ("ptr=%p, ch=%d, value=%d",
					ptr, ptr->ch, ptr->value));
#endif /* TRACE */

		if (ptr == NULL)
			break;

		if (ptr->value != 0) {	/* sequence terminated */
			TR(TRACE_IEVENT, ("end of sequence"));
			if (peek == tail)
			    fifo_clear();
			else
			    head = peek;
			return(ptr->value);
		}

		ptr = ptr->child;

		if (!raw_key_in_fifo())
		{
			TR(TRACE_IEVENT, ("waiting for rest of sequence"));
			if (!_nc_timed_wait(3, timeleft, &timeleft)) {
				TR(TRACE_IEVENT, ("ran out of time"));
				break;
			}
		}
	}
	ch = fifo_pull();
	peek = head;
	return ch;
}