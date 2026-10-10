/****************************************************************************
 * Copyright 2022-2025,2026 Thomas E. Dickey                                *
 * Copyright 2022 Leonid S. Usov <leonid.s.usov at gmail.com>               *
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
 * $Id: test_mouse.c,v 1.51 2026/10/10 21:47:43 tom Exp $
 *
 * Author: Leonid S Usov
 *
 * Observe mouse events in the raw terminal or parsed ncurses modes
 */

#include <test.priv.h>

#if defined(NCURSES_MOUSE_VERSION) && !defined(_NC_WINDOWS_NATIVE)

#if NCURSES_VERSION_PATCH > 20260926
#define USE_PER_BUTTON 1
#else
#define USE_PER_BUTTON 0
#endif

static int logoffset = 0;

static void
cleanup(void)
{
    stop_curses();
}

static void
onsig(int sig GCC_UNUSED)
{
    cleanup();
    ExitProgram(EXIT_FAILURE);
}

static void
raw_loop(void)
{
    const char *xtermcap;

    printf("Entering raw mode. Ctrl-c to quit.\n");

    newterm(NULL, stdout, stdin);
    raw();
    xtermcap = tigetstr(UserCap(XM));
    if (!VALID_STRING(xtermcap)) {
	fprintf(stderr, "couldn't get XM terminfo");
	return;
    }

    putp(tgoto(xtermcap, 1, 1));
    fflush(stdout);

    while (1) {
	int c = getc(stdin);
	const char *pretty;

	if (c == -1 || c == '\003') {
	    break;
	} else if (c == '\033') {
	    printf("\r\n\\E");
	} else if ((pretty = unctrl((chtype) c)) != NULL) {
	    printf("%s", pretty);
	} else if (isprint(c)) {
	    printf("%c", c);
	} else {
	    printf("{%x}", UChar(c));
	}
    }

    putp(tgoto(xtermcap, 0, 0));
    fflush(stdout);
    noraw();
}

static void logw(const char *fmt, ...) GCC_PRINTFLIKE(1, 2);

static void
logw(const char *fmt, ...)
{
    int row = getcury(stdscr);
    va_list args;

    va_start(args, fmt);
    wmove(stdscr, row++, 0);
    vw_printw(stdscr, fmt, args);
    va_end(args);

    clrtoeol();

    row %= (getmaxy(stdscr) - logoffset);
    if (row < logoffset) {
	row = logoffset;
    }

    wmove(stdscr, row, 0);
    wprintw(stdscr, ">");
    clrtoeol();
}

#define ShowQ(btn,name) \
	(((this_event & NCURSES_MOUSE_MASK(btn, NCURSES_ ## name)) != 0) \
	 ? (" " #name) \
	 : "")
#define ShowM(name) \
	(((this_event & BUTTON_ ## name) != 0) \
	 ? (" " #name) \
	 : "")
#define ShowP() \
	 ((this_event & REPORT_MOUSE_POSITION) != 0 \
	  ? " position" \
	  : "")
#if NCURSES_MOUSE_VERSION == 3
#define HexFormat "[%016lX]"
#else
#define HexFormat "[%08lX]"
#endif

static void
cooked_loop(char *my_environ, int interval)
{
    MEVENT event;

    signal(SIGINT, onsig);
    initscr();
    noecho();
    cbreak();			/* Line buffering disabled; pass everything */
    nonl();
    keypad(stdscr, TRUE);

    /* Get all the mouse events */
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(interval);

    logw("Ctrl-c to quit");
    logw("--------------");
    if (my_environ)
	logw("%s", my_environ);
    logoffset = getcury(stdscr);

    while (1) {
	int c = getch();

	switch (c) {
	case KEY_MOUSE:
	    if (getmouse(&event) == OK) {
		unsigned btn;
#if NCURSES_MOUSE_VERSION > 2
		const unsigned max_btn = 11;
#elif NCURSES_MOUSE_VERSION > 1
		const unsigned max_btn = 5;
#else
		const unsigned max_btn = 4;
#endif
		const mmask_t btn_mask = (NCURSES_BUTTON_RELEASED |
					  NCURSES_BUTTON_PRESSED |
					  NCURSES_BUTTON_CLICKED |
					  NCURSES_DOUBLE_CLICKED |
					  NCURSES_TRIPLE_CLICKED);
		bool found = FALSE;
		mmask_t this_event = event.bstate;
		for (btn = 1; btn <= max_btn; btn++) {
		    mmask_t this_btn = (mmask_t) (this_event
						  & NCURSES_MOUSE_MASK(btn, btn_mask));
		    if (this_btn == 0)
			continue;
		    logw(HexFormat " button %2d%s%s%s%s%s%s @%3d,%3d%s%s%s",
			 (unsigned long) this_btn,
			 btn,
			 ShowQ(btn, BUTTON_RELEASED),
			 ShowQ(btn, BUTTON_PRESSED),
			 ShowQ(btn, BUTTON_CLICKED),
			 ShowQ(btn, DOUBLE_CLICKED),
			 ShowQ(btn, TRIPLE_CLICKED),
			 ShowP(),
			 event.y, event.x,
			 ShowM(SHIFT),
			 ShowM(CTRL),
			 ShowM(ALT));
		    found = TRUE;
		}
		/*
		 * A position report need not have a button associated with it.
		 * The modifiers probably are unused.
		 */
		if (!found && (this_event & REPORT_MOUSE_POSITION)) {
		    logw(HexFormat "%s @%3d,%3d%s%s%s",
			 (unsigned long) this_event,
			 ShowP(),
			 event.y, event.x,
			 ShowM(SHIFT),
			 ShowM(CTRL),
			 ShowM(ALT));
		}
	    }
	    break;
	case ERR:
	case '\003':
	    goto end;
	default:
	    logw("got another char: 0x%x", UChar(c));
	}
	refresh();
    }
  end:
    endwin();
}

#if USE_PER_BUTTON
static void
button_loop(char *my_environ, int interval)
{
    MEVENT event;
    int num_buttons = 0;
    int btn;

    signal(SIGINT, onsig);
    initscr();
    noecho();
    cbreak();			/* Line buffering disabled; pass everything */
    nonl();
    keypad(stdscr, TRUE);

    /* determine the number of buttons (internal may support more than mask) */
    for (btn = 0; btn < 12; ++btn) {
	mmask_t my_mask;
	(void) mouse_mask(btn, ALL_MOUSE_EVENTS, &my_mask);
	if (mouse_mask(btn, ALL_MOUSE_EVENTS, &my_mask) == 0) {
	    break;
	}
	num_buttons = btn;
    }

    mouseinterval(interval);

    logw("Ctrl-c to quit (%d buttons)", num_buttons);
    logw("--------------");
    if (my_environ)
	logw("%s", my_environ);
    logoffset = getcury(stdscr);

    while (1) {
	int c = getch();
	int rc;

	switch (c) {
	case KEY_MOUSE:
	    rc = ERR;
	    for (btn = num_buttons; btn >= 0; --btn) {
		if (mouse_test(btn, &event) == OK) {
		    rc = mouse_get(btn, &event);
		    break;
		}
	    }
	    if (rc == OK) {
		const mmask_t btn_mask = (NCURSES_BUTTON_RELEASED |
					  NCURSES_BUTTON_PRESSED |
					  NCURSES_BUTTON_CLICKED |
					  NCURSES_DOUBLE_CLICKED |
					  NCURSES_TRIPLE_CLICKED);
		mmask_t this_event = event.bstate;
		if (btn != 0) {
		    mmask_t this_btn = (mmask_t) (this_event
						  & NCURSES_MOUSE_MASK(1, btn_mask));
		    if (this_btn == 0)
			continue;
		    logw(HexFormat " button %2d%s%s%s%s%s%s @%3d,%3d%s%s%s",
			 (unsigned long) this_btn,
			 btn,
			 ShowQ(1, BUTTON_RELEASED),
			 ShowQ(1, BUTTON_PRESSED),
			 ShowQ(1, BUTTON_CLICKED),
			 ShowQ(1, DOUBLE_CLICKED),
			 ShowQ(1, TRIPLE_CLICKED),
			 ShowP(),
			 event.y, event.x,
			 ShowM(SHIFT),
			 ShowM(CTRL),
			 ShowM(ALT));
		} else {
		    /*
		     * A position report need not have a button associated with
		     * it.  The modifiers probably are unused.
		     */
		    logw(HexFormat "%s @%3d,%3d%s%s%s",
			 (unsigned long) this_event,
			 ShowP(),
			 event.y, event.x,
			 ShowM(SHIFT),
			 ShowM(CTRL),
			 ShowM(ALT));
		}
	    }
	    break;
	case ERR:
	case '\003':
	    goto end;
	default:
	    logw("got another char: 0x%x", UChar(c));
	}
	refresh();
    }
  end:
    endwin();
}
#endif /* USE_PER_BUTTON */

static void
usage(int ok)
{
    static const char *msg[] =
    {
	"Usage: test_mouse [options]"
	,""
	,"Test mouse events.  These examples for $TERM demonstrate xterm"
	,"features:"
	,"    xterm"
	,"    xterm-1002"
	,"    xterm-1003"
	,""
	,USAGE_COMMON
	,"Options:"
#if USE_PER_BUTTON
	," -b       use button-level API, e.g., mouse_get rather than getmouse"
#endif
	," -i NUM   set mouse interval to NUM; default is 0 (no double-clicks)"
	," -r       show raw input stream, injecting a new line before every ESC"
	," -T TERM  override $TERM"
    };
    unsigned n;
    for (n = 0; n < sizeof(msg) / sizeof(char *); ++n) {
	fprintf(stderr, "%s\n", msg[n]);
    }
    ExitProgram(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
/* *INDENT-OFF* */
VERSION_COMMON()
/* *INDENT-ON* */

int
main(int argc, char *argv[])
{
    bool rawmode = FALSE;
    int interval = -1;
    int ch;
    size_t my_len;
    char *my_environ = NULL;
    const char *term_format = "TERM=%s";
#if USE_PER_BUTTON
    bool per_button = FALSE;
#endif

    while ((ch = getopt(argc, argv, OPTS_COMMON "bi:rT:")) != -1) {
	switch (ch) {
#if USE_PER_BUTTON
	case 'b':
	    per_button = TRUE;
	    break;
#endif
	case 'i':
	    interval = atoi(optarg);
	    break;
	case 'r':
	    rawmode = TRUE;
	    break;
	case 'T':
	    my_len = strlen(term_format) + strlen(optarg) + 1;
	    my_environ = malloc(my_len);
	    if (my_environ != NULL) {
		_nc_SPRINTF(my_environ, _nc_SLIMIT(my_len) term_format, optarg);
		putenv(my_environ);
	    }
	    break;
	default:
	    CASE_COMMON;
	    /* NOTREACHED */
	}
    }
    if (optind < argc) {
	usage(FALSE);
	ExitProgram(EXIT_FAILURE);
    }

    if (rawmode)
	raw_loop();
#if USE_PER_BUTTON
    else if (per_button)
	button_loop(my_environ, interval);
#endif
    else
	cooked_loop(my_environ, interval);

    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif
