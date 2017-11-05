/****************************************************************************
 * Copyright (c) 2014,2017 Free Software Foundation, Inc.                   *
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

/*
 * Author: Thomas E. Dickey
 *
 * $Id: dots_curses.c,v 1.11 2017/10/30 22:22:22 tom Exp $
 *
 * A simple demo of the curses interface used for comparison with termcap.
 */
#include <test.priv.h>

#if !defined(__MINGW32__)
#include <sys/time.h>
#endif

#include <time.h>

static bool interrupted = FALSE;
static long total_chars = 0;
static time_t started;

static void
cleanup(void)
{
    endwin();

    printf("\n\n%ld total cells, rate %.2f/sec\n",
	   total_chars,
	   ((double) (total_chars) / (double) (time((time_t *) 0) - started)));
}

static void
onsig(int n GCC_UNUSED)
{
    interrupted = TRUE;
}

static double
ranf(void)
{
    long r = (rand() & 077777);
    return ((double) r / 32768.);
}

static int
mypair(int fg, int bg)
{
    int pair = (fg * COLORS) + bg;
    return (pair >= COLOR_PAIRS) ? -1 : pair;
}

static void
set_colors(int fg, int bg)
{
    int pair = mypair(fg, bg);
    if (pair > 0) {
	attron(COLOR_PAIR(mypair(fg, bg)));
    }
}

static void
usage(void)
{
    static const char *msg[] =
    {
	"Usage: dots_curses [options]"
	,""
	,"Options:"
	," -T TERM  override $TERM"
#if HAVE_USE_DEFAULT_COLORS
	," -d       invoke use_default_colors()"
#endif
	," -e       allow environment $LINES / $COLUMNS"
	," -m SIZE  set margin (default: 2)"
	," -s MSECS delay 1% of the time (default: 1 msecs)"
    };
    size_t n;

    for (n = 0; n < SIZEOF(msg); n++)
	fprintf(stderr, "%s\n", msg[n]);

    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int x, y, z, p;
    int fg, bg;
    double r;
    double c;
#if HAVE_USE_DEFAULT_COLORS
    bool d_option = FALSE;
#endif
    int m_option = 2;
    int s_option = 1;

    while ((x = getopt(argc, argv, "T:dem:s:")) != -1) {
	switch (x) {
	case 'T':
	    putenv(strcat(strcpy(malloc(6 + strlen(optarg)), "TERM="), optarg));
	    break;
#if HAVE_USE_DEFAULT_COLORS
	case 'd':
	    d_option = TRUE;
	    break;
#endif
#if HAVE_USE_ENV
	case 'e':
	    use_env(TRUE);
	    break;
#endif
	case 'm':
	    m_option = atoi(optarg);
	    break;
	case 's':
	    s_option = atoi(optarg);
	    break;
	default:
	    usage();
	    break;
	}
    }

    srand((unsigned) time(0));

    InitAndCatch(initscr(), onsig);
    if (has_colors()) {
	start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (d_option)
	    use_default_colors();
#endif
	for (fg = 0; fg < COLORS; fg++) {
	    for (bg = 0; bg < COLORS; bg++) {
		int pair;
		if (interrupted) {
		    cleanup();
		    ExitProgram(EXIT_FAILURE);
		}
		pair = mypair(fg, bg);
		if (pair > 0)
		    init_pair((short) pair, (short) fg, (short) bg);
	    }
	}
    }

    r = (double) (LINES - (m_option * 2));
    c = (double) (COLS - (m_option * 2));
    started = time((time_t *) 0);

    fg = COLOR_WHITE;
    bg = COLOR_BLACK;
    while (!interrupted) {
	x = (int) (c * ranf()) + m_option;
	y = (int) (r * ranf()) + m_option;
	p = (ranf() > 0.9) ? '*' : ' ';

	move(y, x);
	if (has_colors()) {
	    z = (int) (ranf() * COLORS);
	    if (ranf() > 0.01) {
		set_colors(fg = z, bg);
		attron(COLOR_PAIR(mypair(fg, bg)));
	    } else {
		set_colors(fg, bg = z);
		napms(s_option);
	    }
	} else {
	    if (ranf() <= 0.01) {
		if (ranf() > 0.6) {
		    attron(A_REVERSE);
		} else {
		    attroff(A_REVERSE);
		}
		napms(s_option);
	    }
	}
	AddCh(p);
	refresh();
	++total_chars;
    }
    cleanup();
    ExitProgram(EXIT_SUCCESS);
}
