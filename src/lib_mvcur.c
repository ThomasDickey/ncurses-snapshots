/*
**	lib_mvcur.c
**
**	The routines for moving the physical cursor and scrolling:
**
**		int mvcur(int old_y, int old_x, int new_y, int new_x)
**
**		int scrolln(int n, int top, int bot, int maxy)
**
** Some of this code was originally part of the legacy 4.4BSD curses
** (cr_put.c and refresh.c version 8.3).  I (esr) translated it to speak
** terminfo and cleaned out a lot of cruft having to do with obsolete 
** capabilities.  Then I threw away plod() and wrote a smarter local movement 
** optimizer.  By the time I was done, only a few pieces of the 4.4BSD logic
** were left (in mvcur(), which mostly used to be fgoto()).  Keith Bostic's
** scrolln() routine survived almost intact, though.
**
** Comparisons with older curses packages:  
**    SVr3 curses mvcur() can't use cursor_to_ll or auto_left_margin.
**    4.4BSD curses can't use cuu/cud/cuf/cub/hpa/vpa/tab/cbt for local
** motions.  It doesn't use tactics based on auto_left_margin.  Weirdly
** enough, it doesn't use its own hardware-scrolling routine to scroll up
** destination lines for out-of-bounds addresses!
**    old ncurses optimizer: less accurate cost computations (in fact,
** it was broken and had to be commented out!).
**
** Compile with -DMAIN to build an interactive tester/timer for the movement
** optimizer.  You can use it to investigate the optimizer's behavior.
** You can also use it for tuning the formulas used to determine whether
** or not full optimization is attempted.
**
** This code has a nasty tendency to find bugs in terminfo entries, because it
** exercises the non-cup movement capabilities heavily.  If you think you've
** found a bug, try deleting subsets of the following capabilities (arranged
** in decreasing order of suspiciousness): it, tab, cbt, hpa, vpa, cuu, cud,
** cuf, cub, cuu1, cud1, cuf1, cub1.  It may be that one or more are wrong.
**
** This software is Copyright (C) 1994 by Eric S. Raymond, all rights reserved.
** It is issued with ncurses under the same terms and conditions as the ncurses
** library sources.
**/

/****************************************************************************
 *
 * Constants and macros for optimizer tuning.
 *
 ****************************************************************************/

/*
 * The average overhead of a full optimization computation in character
 * transmission times.  If it's too high, the algorithm will be a bit
 * over-biased toward using cup rather than local motions; if it's too
 * low, the algorithm may spend more time than is strictly optimal
 * looking for non-cup motions.  Profile the optimizer using the `t'
 * command of the exerciser (see below), and round to the nearest integer.
 * Assuming your machine runs compiled C about as efficiently as a 486, the
 * following mapping should be about right:
 *
 *		Kbps:	28.8	19.2	14.4	9.60	2.40
 *	       		-----	----	----	----	----
 *	25MHz	->	17	11	8	6	1
 *	50MHz	->	8	6	4	3	1
 *	66MHz	->	6	4	3	2	0
 *	100MHz	->	4	3	2	1	0
 *	120MHz	->	3	2	1	0	0
 *
 * Cross your average line speed with your processor speed to get
 * your overhead value.
 *
 * Yes, I thought about computing expected overhead dynamically, say
 * by derivation from a running average of optimizer times.  But the
 * whole point of this optimization is to *decrease* the frequency of
 * system calls. :-)
 */
#define COMPUTE_OVERHEAD	3	/* I use a 50MHz box @ 9.6Kbs */

/*
 * LONG_DIST is the distance we consider to be just as costly to move over as a
 * cup sequence is to emit.  In other words, it's the length of a cup sequence
 * adjusted for computation overhead.  The magic number is the length of
 * "\033[yy;xxH", the typical cup sequence these days.
 */
#define LONG_DIST		(8 - COMPUTE_OVERHEAD)

/*
 * Tell whether a motion is optimizable by local motions.  Needs to be cheap to
 * compute. In general, all the fast moves go to either the right or left edge
 * of the screen.  So any motion to a location that is (a) further away than
 * LONG_DIST and (b) further inward from the right or left edge than LONG_DIST,
 * we'll consider nonlocal.
 */
#define NOT_LOCAL(fy, fx, ty, tx)	((tx > LONG_DIST) && (tx < screen_lines - 1 - LONG_DIST) && (abs(ty-fy) + abs(tx-fx) > LONG_DIST))

/****************************************************************************
 *
 * External interfaces
 *
 ****************************************************************************/

#include "curses.h"

#include "terminfo.h"
#include "curses.priv.h"
#define NLMAPPING	SP->_nl			/* nl() on? */
#define RAW		SP->_raw		/* raw() on? */
#define CURRENT_ATTR	SP->_current_attr	/* current phys attribute */
#define CURRENT_ROW	SP->_cursrow		/* phys cursor row */
#define CURRENT_COLUMN	SP->_curscol		/* phys cursor column */
#define REAL_ATTR	SP->_current_attr	/* phys current attribute */
#define REAL_CHAR(y, x)	SP->_curscr->_line[y].text[x]	/* phys screen data */

/****************************************************************************
 *
 * Optimized cursor movement
 *
 ****************************************************************************/

#include <string.h>

#ifdef TRACE
bool no_optimize;	/* suppress optimization */
#endif /* TRACE */

#ifdef MAIN
#include <sys/time.h>

static bool profiling = FALSE;
static float diff;
#endif /* MAIN */

static int	onscreen_mvcur(int, int, int, int, bool);
#ifndef NO_OPTIMIZE
static int	relative_move(char *, int, int, int, int, bool);
#endif /* !NO_OPTIMIZE */

int mvcur(int yold, int xold, int ynew, int xnew)
/* optimized cursor move from (yold, xold) to (ynew, xnew) */
{
    TR(TRACE_MOVE, ("mvcur(%d,%d,%d,%d) called", yold, xold, ynew, xnew));

    if (yold == ynew && xold == xnew)
	return(OK);

    /*
     * Most work here is rounding for terminal boundaries getting the
     * column position implied by wraparound or the lack thereof and
     * rolling up the screen to get ynew on the screen.
     */

    if (xnew >= screen_columns)
    {
	ynew += xnew / screen_columns;
	xnew %= screen_columns;
    }
    if (xold >= screen_columns)
    {
	int	l;

	l = (xold + 1) / screen_columns;
	yold += l;
	xold %= screen_columns;
	if (!auto_right_margin)
	{
	    while (l > 0) {
		if (newline)
		    tputs(newline, 0, _outch);
		else
		    putchar('\n');
		l--;
	    }
	    xold = 0;
	}
	if (yold > screen_lines - 1)
	{
	    ynew -= yold - (screen_lines - 1);
	    yold = screen_lines - 1;
	}
    }

#ifdef CURSES_OVERRUN	/* not used, it takes us out of sync with curscr */
    /*
     * The destination line is offscreen. Try to scroll the screen to
     * bring it onscreen.  Note: this is not a documented feature of the
     * API.  It's here for compatibility with archaic curses code, a
     * feature no one seems to have actually used in a long time.
     */
    if (ynew >= screen_lines)
    {
	if (scrolln((ynew - (screen_lines - 1)), 0, screen_lines - 1, screen_lines - 1) == OK)
	    ynew = screen_lines - 1;
	else
	    return(ERR);
    }
#endif /* CURSES_OVERRUN */

    /* destination location is on screen now */
    return(onscreen_mvcur(yold, xold, ynew, xnew, TRUE));
}

/*
 * With the machinery set up above, it's conceivable that
 * onscreen_mvcur could be modified into a recursive function that does
 * an alpha-beta search of motion space, as though it were a chess
 * move tree, with the weight function being boolean and the search
 * depth equated to length of string.  However, this would jack up the
 * computation cost a lot, especially on terminals without a cup
 * capability constraining the search tree depth.  So we settle for
 * the simpler method below.
 */

/*
 * This must be a character that never occurs as a control sequence leader.
 * Any printable character ought to do.
 */
#define OUT_OF_BAND	'x'

static int onscreen_mvcur(int yold, int xold, int ynew, int xnew, bool ovw)
/* onscreen move from (yold, xold) to (ynew, xnew) */
{
    char	use[512], *sp;
#ifndef NO_OPTIMIZE
    char	move[512];
#endif /* !NO_OPTIMIZE */

#ifdef MAIN
    struct timeval before, after;

    gettimeofday(&before, NULL);
#endif /* MAIN */

    use[0] = OUT_OF_BAND;

    /* tactic #0: use direct cursor addressing */
    sp = tgoto(cursor_address, xnew, ynew);
    if (sp)
    {
	(void) strcpy(use, sp);

#ifdef TRACE
	if (no_optimize)
	    xold = yold = -1;
#endif /* TRACE */

	/*
	 * We may be able to tell in advance that the full optimization
	 * will probably not be worth its overhead.  Also, don't try to
	 * use local movement if the current attribute is anything but
	 * A_NORMAL...there are just too many ways this can screw up
	 * (like, say, local-movement \n getting mapped to some obscure
	 * character because A_ALTCHARSET is on).
	 */
	if (yold == -1 || xold == -1  || 
	    REAL_ATTR != A_NORMAL || NOT_LOCAL(yold, xold, ynew, xnew))
	{
#ifdef MAIN
	    if (!profiling)
	    {
		(void) fputs("nonlocal\n", stderr);
		goto nonlocal;	/* always run the optimizer if profiling */
	    }
#else
	    goto nonlocal;
#endif /* MAIN */
	}
    }

#ifndef NO_OPTIMIZE
    /* tactic #1: use local movement */
    if (yold != -1 && xold != -1
		&& (relative_move(move, yold, xold, ynew, xnew, ovw)==OK)
		&& (use[0] == OUT_OF_BAND || strlen(move) < strlen(use)))
	(void) strcpy(use, move);

    /* tactic #2: use carriage-return + local movement */
    if (yold < screen_lines - 1 && xold < screen_columns - 1)
    {
	if (carriage_return
		&& (relative_move(move, yold,0,ynew,xnew, ovw) == OK)
		&& (use[0] == OUT_OF_BAND || (strlen(carriage_return) + strlen(move) < strlen(use))))
	{
	    (void) strcpy(use, carriage_return);
	    (void) strcat(use, move);
	}
    }

    /* tactic #3: use home-cursor + local movement */
    if (cursor_home
	&& (relative_move(move, 0, 0, ynew, xnew, ovw) == OK)
	&& (use[0] == OUT_OF_BAND || (strlen(cursor_home) + strlen(move) < strlen(use))))
    {
	(void) strcpy(use, cursor_home);
	(void) strcat(use, move);
    }

    /* tactic #4: use home-down + local movement */
    if (cursor_to_ll
    	&& (relative_move(move, screen_lines-1, 0, ynew, xnew, ovw) == OK)
	&& (use[0] == OUT_OF_BAND || (strlen(cursor_to_ll) + strlen(move) < strlen(use))))
    {
	(void) strcpy(use, cursor_to_ll);
	(void) strcat(use, move);
    }

    /* tactic #5: use left margin for wrap to right-hand side */
    if (auto_left_margin && yold > 0 && yold < screen_lines - 1 && cursor_left
	&& (relative_move(move, yold-1, screen_columns-1, ynew, xnew, ovw) == OK)
	&& (use[0] == OUT_OF_BAND || (strlen(carriage_return) + strlen(cursor_left) + strlen(move) < strlen(use))))
    {
	use[0] = '\0';
	if (xold > 0)
	    (void) strcat(use, carriage_return);
	(void) strcat(use, cursor_left);
	(void) strcat(use, move);
    }
#endif /* !NO_OPTIMIZE */

#ifdef MAIN
    gettimeofday(&after, NULL);
    diff = after.tv_usec - before.tv_usec
	+ (after.tv_sec - before.tv_sec) * 1000000;
    if (!profiling)
	(void) fprintf(stderr, "onscreen: %d msec, %f 28.8Kbps char-equivalents\n",
		       (int)diff, diff/288);
#endif /* MAIN */

 nonlocal:
    if (use[0] != OUT_OF_BAND)
    {
	tputs(use, 1, _outch);
	return(OK);
    }
    else
	return(ERR);
}

#ifndef NO_OPTIMIZE
#define NEXTTAB(fr)	(fr + init_tabs - (fr % init_tabs))
#define LASTTAB(fr)	(fr - init_tabs + (fr % init_tabs))

static int relative_move(char *move, int from_y,int from_x,int to_y,int to_x, bool ovw)
/* move via local motions (cuu/cuu1/cud/cud1/cub1/cub/cuf1/cuf/vpa/hpa) */
{
    char	*ep, *sp;
    int	n;

    move[0] = '\0';
    ep = move;

    if (to_y != from_y)
    {
	ep[0] = OUT_OF_BAND;

	if (row_address)
	    (void) strcpy(ep, tparm(row_address, to_y));

	if (to_y > from_y)
	{
	    n = (to_y - from_y);

	    if (parm_down_cursor)
	    {
		sp = tparm(parm_down_cursor, n);
		if (ep[0] == OUT_OF_BAND && strlen(sp) < strlen(ep))
		    (void) strcpy(ep, sp);
	    }

	    if (cursor_down
		&& (ep[0] == OUT_OF_BAND || n * strlen(cursor_down) < strlen(ep)))
	    {
		ep[0] = '\0';
		while (n--)
		    (void) strcat(ep, cursor_down);
	    }
	}
	else /* (to_y < from_y) */
	{
	    n = (from_y - to_y);

	    if (parm_up_cursor)
	    {
		sp = tparm(parm_up_cursor, n);
		if (ep[0] == OUT_OF_BAND && strlen(sp) < strlen(ep))
		    (void) strcpy(ep, sp);
	    }

	    if (cursor_up
		&& (ep[0] == OUT_OF_BAND || n * strlen(cursor_up) < strlen(ep)))
	    {
		ep[0] = '\0';
		while (n--)
		    (void) strcat(ep, cursor_up);
	    }
	}

	if (ep[0] == OUT_OF_BAND)
	    return(ERR);
    }

    /*
     * It may be that we're using a cud1 capability of \n with the side-effect
     * of taking the cursor to column 0.  Deal with this.
     */
    if (ep[0] == '\n' && NLMAPPING && !RAW)
	from_x = 0;
	
    ep = move + strlen(move);

    if (to_x != from_x)
    {
#if defined(REAL_ATTR) && defined(REAL_CHAR)
	int	i;
#endif /* defined(REAL_ATTR) && defined(REAL_CHAR) */
	char	try[512];

	ep[0] = OUT_OF_BAND;

	if (column_address)
	    (void) strcpy(ep, tparm(column_address, to_x));

	if (to_x > from_x)
	{
	    n = to_x - from_x;

	    if (parm_right_cursor)
	    {
	        sp = tparm(parm_right_cursor, n);
		if (ep[0] == OUT_OF_BAND && strlen(sp) < strlen(ep))
		    (void) strcpy(ep, sp);
	    }

	    if (cursor_right)
	    {
		try[0] = '\0';

		/* use hard tabs, if we have them, to do as much as possible */
		if (init_tabs > 0 && tab)
		{
		    int	nxt, fr;

		    for (fr = from_x; (nxt = NEXTTAB(fr)) <= to_x; fr = nxt)
			(void) strcat(try, tab);

		    n = to_x - fr;
		    from_x = fr;
		}

#if defined(REAL_ATTR) && defined(REAL_CHAR)
		/*
		 * If we have no attribute changes, overwrite is cheaper.
		 * Note: must suppress this by passing in ovw = FALSE whenever
		 * REAL_CHAR would return invalid data.  In particular, this 
		 * is true between the time a hardware scroll has been done
		 * and the time the structure REAL_CHAR would access has been
		 * updated.
		 */
		if (ovw)
		{
		    char	*sp;

		    for (i = 0; i < n; i++)
			if ((REAL_CHAR(to_y, from_x + i) & A_ATTRIBUTES) != CURRENT_ATTR)
			{
			    ovw = FALSE;
			    break;
			}

		    sp = try + strlen(try);

		    for (i = 0; i < n; i++)
			*sp++ = REAL_CHAR(to_y, from_x + i);
		    *sp = '\0';
	        }
		else
#endif /* defined(REAL_ATTR) && defined(REAL_CHAR) */
		    while (n--)
			(void) strcat(try, cursor_right);

		if (ep[0] == OUT_OF_BAND || strlen(try) < strlen(ep))
		    (void) strcpy(ep, try);
	    }
	}
	else /* (to_x < from_x) */
	{
	    n = from_x - to_x;

	    if (parm_left_cursor)
	    {
		sp = tparm(parm_left_cursor, n);
		if (ep[0] == OUT_OF_BAND && strlen(sp) < strlen(ep))
		    (void) strcpy(ep, sp);
	    }

	    if (cursor_left)
	    {
		try[0] = '\0';

		if (init_tabs > 0 && back_tab)
		{
		    int	nxt, fr;

		    for (fr = from_x; (nxt = LASTTAB(fr)) >= to_x; fr = nxt)
			(void) strcat(try, back_tab);

		    n = to_x - fr;
		}
		while (n--)
		    (void) strcat(try, cursor_left);

		if (ep[0] == OUT_OF_BAND || strlen(try) < strlen(ep))
		    (void) strcpy(ep, try);
	    }
	}

	if (ep[0] == OUT_OF_BAND)
	    return(ERR);
    }

    return(OK);
}
#endif /* !NO_OPTIMIZE */

/****************************************************************************
 *
 * Physical-scrolling support
 *
 ****************************************************************************/

int scrolln(int n, int top, int bot, int maxy)
/* scroll region from top to bot by n lines */
{
    int i, oy, ox;

    oy = CURRENT_ROW;
    ox = CURRENT_COLUMN;

    TR(TRACE_MOVE, ("scrolln(%d, %d, %d, %d)", n, top, bot, maxy)); 
    /*
     * This code was adapted from Keith Bostic's hardware scrolling
     * support for 4BSD curses.  I translated it to use terminfo
     * capabilities, narrowed the call interface slightly, and cleaned
     * up some convoluted tests.
     *
     * For this code to work, we must have either
     * change_scroll_region and scroll forward/reverse commands, or
     * insert and delete line capabilities.
     * When the scrolling region has been set, the cursor has to
     * be at the last line of the region to make the scroll
     * happen.
     *
     * This code makes one aesthetic decision in the opposite way from
     * BSD curses.  BSD curses preferred pairs of il/dl operations
     * over scrolls, allegedly because il/dl looked faster.  We, on
     * the other hand, prefer scrolls because (a) they're just as fast
     * on modern terminals and (b) using them avoids bouncing an
     * unchanged bottom section of the screen up and down, which is
     * visually nasty.
     */
    if (n > 0)
    {
	if (change_scroll_region && (scroll_forward || parm_index))
	{
	    tputs(tparm(change_scroll_region, top, bot), 0, _outch);
	    onscreen_mvcur(-1, -1, bot, 0, TRUE);
	    if (parm_index != NULL)
		tputs(tparm(parm_index, n, 0), n, _outch);
	    else
		for (i = 0; i < n; i++)
		    tputs(scroll_forward, 0, _outch);
	    tputs(tparm(change_scroll_region, 0, maxy), 0, _outch);
	    onscreen_mvcur(-1, -1, oy, ox, FALSE);
	    return(OK);
	}

	/* Scroll up the block. */
	if (parm_index && top == 0)
	{
	    onscreen_mvcur(oy, ox, bot, 0, TRUE);
	    tputs(tparm(parm_index, n, 0), n, _outch);
	}
	else if (parm_delete_line)
	{
	    onscreen_mvcur(oy, ox, top, 0, TRUE);
	    tputs(tparm(parm_delete_line, n, 0), n, _outch);
	}
	else if (delete_line)
	{
	    onscreen_mvcur(oy, ox, top, 0, TRUE);
	    for (i = 0; i < n; i++)
		tputs(delete_line, 0, _outch);
	}
	else if (scroll_forward && top == 0)
	{
	    onscreen_mvcur(oy, ox, bot, 0, TRUE);
	    for (i = 0; i < n; i++)
		tputs(scroll_forward, 0, _outch);
	}
	else
	    return(ERR);

	/* Push down the bottom region. */
	onscreen_mvcur(top, 0, bot - n + 1, 0, FALSE);
	if (parm_insert_line)
	    tputs(tparm(parm_insert_line, n, 0), n, _outch);
	else if (insert_line)
	    for (i = 0; i < n; i++)
		tputs(insert_line, 0, _outch);
	else
	    return(ERR);
	onscreen_mvcur(bot - n + 1, 0, oy, ox, FALSE);
    }
    else /* (n < 0) */
    {
	if (change_scroll_region && (scroll_reverse || parm_rindex))
	{
	    tputs(tparm(change_scroll_region, top, bot), 0, _outch);
	    onscreen_mvcur(-1, -1, top, 0, TRUE);
	    if (parm_rindex)
		tputs(tparm(parm_rindex, -n, 0), -n, _outch);
	    else
		for (i = n; i < 0; i++)
		    tputs(scroll_reverse, 0, _outch);
	    tputs(tparm(change_scroll_region, 0, maxy), 0, _outch);
	    onscreen_mvcur(-1, -1, oy, ox, FALSE);
	    return(OK);
	}

	/* Preserve the bottom lines. */
	onscreen_mvcur(oy, ox, bot + n + 1, 0, TRUE);
	if (parm_rindex && bot == maxy)
	    tputs(tparm(parm_rindex, -n, 0), -n, _outch);
	else if (parm_delete_line)
	    tputs(tparm(parm_delete_line, -n, 0), -n, _outch);
	else if (delete_line)
	    for (i = n; i < 0; i++)
		tputs(delete_line, 0, _outch);
	else if (scroll_reverse && bot == maxy)
	    for (i = n; i < 0; i++)
		tputs(scroll_reverse, 0, _outch);
	else
	    return(ERR);

	/* Scroll the block down. */
	onscreen_mvcur(bot + n + 1, 0, top, 0, FALSE);
	if (parm_insert_line)
	    tputs(tparm(parm_insert_line, -n, 0), -n, _outch);
	else if (insert_line)
	    for (i = n; i < 0; i++)
		tputs(insert_line, 0, _outch);
	else
	    return(ERR);
	onscreen_mvcur(top, 0, oy, ox, FALSE);
    }

    return(OK);
}

#ifdef MAIN
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tic.h"
#include "dump_entry.h"

char *progname = "mvcur";

int tputs(char *string, int affcnt, int (*outc)(char))
/* stub tputs() that dumps sequences in a visible form */
{
    if (!profiling)
	(void) fputs(visbuf(string), stdout);
    return(OK);
}

int putp(char *string)
{
    return(tputs(string, 1, _outch));
}

static char	tname[BUFSIZ];

static void load_term(void)
{
    int	errret;

    (void) setupterm(tname, 1, &errret);
    if (errret == 1)
	(void) printf("Loaded \"%s\" entry.\n", tname);
    else
	(void) printf("Load failed, error %d.\n", errret);
}

int roll(int n)
{
    int i, j;

    i = (RAND_MAX / n) * n;
    while ((j = rand()) >= i)
	continue;
    return (j % n);
}

int main(int argc, char *argv[])
{
    (void) strcpy(tname, getenv("TERM"));
    load_term();
    setupscreen(lines, columns);
    make_hash_table(info_table, info_hash_table);

    (void) puts("The mvcur tester.  Type ? for help");
    for (;;)
    {
	int	fy, fx, ty, tx, n, i;
	char	buf[BUFSIZ], capname[BUFSIZ];

	(void) fputs("> ", stdout);
	(void) fgets(buf, sizeof(buf), stdin);

	if (buf[0] == '?')
	{
(void) puts("?                -- display this help message");
(void) puts("fy fx ty tx      -- (4 numbers) display (fy,fx)->(ty,tx) move");
(void) puts("s[croll] n t b m -- display scrolling sequence");
(void) printf("r[eload]         -- reload terminal info for %s\n",
	      getenv("TERM"));
(void) puts("l[oad] <term>    -- load terminal info for type <term>");
(void) puts("nl               -- assume NL -> CR/LF when computing (default)");
(void) puts("nonl             -- don't assume NL -> CR/LF when computing");
(void) puts("d[elete] <cap>   -- delete named capability");
(void) puts("i[nspect]        -- display terminal capabilities");
(void) puts("t[orture] <num>  -- torture-test with <num> random moves");
(void) puts("q[uit]           -- quit the program");
	}
	else if (sscanf(buf, "%d %d %d %d", &fy, &fx, &ty, &tx) == 4)
	{
	    struct timeval before, after;

	    putchar('"');

	    gettimeofday(&before, NULL);
	    mvcur(fy, fx, ty, tx);
	    gettimeofday(&after, NULL);

	    printf("\" (%ld msec)\n",
		after.tv_usec - before.tv_usec + (after.tv_sec - before.tv_sec) * 1000000);
	}
	else if (sscanf(buf, "s %d %d %d %d", &fy, &fx, &ty, &tx) == 4)
	{
	    struct timeval before, after;

	    putchar('"');

	    gettimeofday(&before, NULL);
	    scrolln(fy, fx, ty, tx);
	    gettimeofday(&after, NULL);

	    printf("\" (%ld msec)\n",
		after.tv_usec - before.tv_usec + (after.tv_sec - before.tv_sec) * 1000000);
	}
	else if (buf[0] == 'r')
	{
	    (void) strcpy(tname, getenv("TERM"));
	    load_term();
	}
	else if (sscanf(buf, "l %s", tname) == 1)
	{
	    load_term();
	}
	else if (strncmp(buf, "nl", 2) == 0)
	{
	    NLMAPPING = TRUE;
	    (void) puts("NL -> CR/LF will be assumed.");
	}
	else if (strncmp(buf, "nonl", 4) == 0)
	{
	    NLMAPPING = FALSE;
	    (void) puts("NL -> CR/LF will not be assumed.");
	}
	else if (sscanf(buf, "d %s", capname) == 1)
	{
	    struct name_table_entry	*np = find_entry(capname,
							 info_hash_table);

	    if (np == NULL)
		(void) printf("No such capability as \"%s\"\n", capname);
	    else
	    {
		switch(np->nte_type)
		{
		case BOOLEAN:
		    cur_term->type.Booleans[np->nte_index] = FALSE;
		    (void) printf("Boolean capability `%s' (%d) turned off.\n",
				  np->nte_name, np->nte_index);
		    break;

		case NUMBER:
		    cur_term->type.Numbers[np->nte_index] = -1;
		    (void) printf("Number capability `%s' (%d) set to -1.\n",
				  np->nte_name, np->nte_index);
		    break;

		case STRING:
		    cur_term->type.Strings[np->nte_index] = (char *)NULL;
		    (void) printf("String capability `%s' (%d) deleted.\n",
				  np->nte_name, np->nte_index);
		    break;
		}
	    }
	}
	else if (buf[0] == 'i')
	{
	     dump_init(F_TERMINFO, S_TERMINFO, 70, 0);
	     dump_entry(&cur_term->type, NULL);
	     putchar('\n');
	}
	else if (sscanf(buf, "t %d", &n) == 1)
	{
	    float cumtime = 0;

	    srand(getpid() + time((time_t *)0));
	    profiling = TRUE;
	    for (i = 0; i < n; i++)
	    {
		mvcur(roll(lines), roll(columns), roll(lines), roll(columns));
		if (diff)
		    cumtime += diff;
	    }
	    profiling = FALSE;
	    (void) printf("%d moves in %d secs; %f 28.8Kbps char-xmits.\n",
		n, (int)cumtime, cumtime/(n * 288));
	}
	else if (buf[0] == 'x' || buf[0] == 'q')
	    break;
	else
	    (void) puts("Invalid command.");
    }

    return(0);
}

#endif /* MAIN */

/* lib_mvcur.c ends here */
