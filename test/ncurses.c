/****************************************************************************

NAME
   ncurses.c --- ncurses library exerciser

SYNOPSIS
   ncurses

DESCRIPTION
   An interactive test module for the ncurses library.

AUTHOR
   This software is Copyright (C) 1993 by Eric S. Raymond, all rights reserved.
It is issued with ncurses under the same terms and conditions as the ncurses
library source.

$Id: ncurses.c,v 1.52 1996/09/08 00:36:15 tom Exp $

***************************************************************************/
/*LINTLIBRARY */

#include "test.priv.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#if HAVE_GETTIMEOFDAY
#if HAVE_SYS_TIME_H && ! SYSTEM_LOOKS_LIKE_SCO
#include <sys/time.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#endif

#if HAVE_PANEL_H
#include <panel.h>
#endif

#if HAVE_MENU_H && HAVE_LIBMENU
#include <menu.h>
#endif

#if HAVE_FORM_H
#include <form.h>
#endif

#ifdef NCURSES_VERSION

#ifdef TRACE
static int save_trace = TRACE_ORDINARY|TRACE_CALLS;
extern int _nc_tracing;
#endif

#ifndef HAVE_NAPMS
#define HAVE_NAPMS 1
#endif

#else

#define mmask_t chtype		/* not specified in XSI */
#define attr_t chtype		/* not specified in XSI */
#define ACS_S3          (acs_map['p'])  /* scan line 3 */
#define ACS_S7          (acs_map['r'])  /* scan line 7 */
#define ACS_LEQUAL      (acs_map['y'])  /* less/equal */
#define ACS_GEQUAL      (acs_map['z'])  /* greater/equal */
#define ACS_PI          (acs_map['{'])  /* Pi */
#define ACS_NEQUAL      (acs_map['|'])  /* not equal */
#define ACS_STERLING    (acs_map['}'])  /* UK pound sign */

#endif

#define P(string)	printw("%s\n", string)
#ifndef CTRL
#define CTRL(x)		((x) & 0x1f)
#endif

#define SIZEOF(table)	(sizeof(table)/sizeof(table[0]))
#define QUIT		CTRL('Q')
#define ESCAPE		CTRL('[')
#define BLANK		' '		/* this is the background character */

/* The behavior of mvhline, mvvline for negative/zero length is unspecified,
 * though we can rely on negative x/y values to stop the macro.
 */
static void do_h_line(int y, int x, chtype c, int to)
{
	if ((to) > (x))
		mvhline(y, x, c, (to) - (x));
}

static void do_v_line(int y, int x, chtype c, int to)
{
	if ((to) > (y))
		mvvline(y, x, c, (to) - (y));
}

/* Common function to allow ^T to toggle trace-mode in the middle of a test
 * so that trace-files can be made smaller.
 */
static int wGetchar(WINDOW *win)
{
	int c;
#ifdef TRACE
	while ((c = wgetch(win)) == CTRL('T')) {
		if (_nc_tracing) {
			save_trace = _nc_tracing;
			_tracef("TOGGLE-TRACING OFF");
			_nc_tracing = 0;
		} else {
			_nc_tracing = save_trace;
		}
		trace(_nc_tracing);
		if (_nc_tracing)
			_tracef("TOGGLE-TRACING ON");
	}
#else
	c = wgetch(win);
#endif
	return c;
}
#define Getchar() wGetchar(stdscr)

static void Pause(void)
{
	move(LINES - 1, 0);
	addstr("Press any key to continue... ");
	(void) Getchar();
}

static void Cannot(char *what)
{
	printw("\nThis %s terminal %s\n\n", getenv("TERM"), what);
	Pause();
}

/****************************************************************************
 *
 * Character input test
 *
 ****************************************************************************/

static void getch_test(void)
/* test the keypad feature */
{
char buf[BUFSIZ];
int c;
int incount = 0, firsttime = 0;
bool blocking = TRUE;
int y, x;

    refresh();

#ifdef NCURSES_VERSION
    mousemask(ALL_MOUSE_EVENTS, (mmask_t *)NULL);
#endif

    (void) printw("Delay in 10ths of a second (<CR> for blocking input)? ");
    echo();
    getstr(buf);
    noecho();
    nonl();

    if (isdigit(buf[0]))
    {
	timeout(atoi(buf) * 100);
	blocking = FALSE;
    }

    c = '?';
    raw();
    for (;;)
    {
	if (firsttime++)
	{
	    printw("Key pressed: %04o ", c);
#ifdef NCURSES_VERSION
	    if (c == KEY_MOUSE)
	    {
		MEVENT	event;

		getmouse(&event);
		printw("KEY_MOUSE, %s\n", _tracemouse(&event));
	    }
	    else
#endif	/* NCURSES_VERSION */
	     if (c >= KEY_MIN)
	    {
		(void) addstr(keyname(c));
		addch('\n');
	    }
	    else if (c > 0x80)
	    {
		int c2 = (c & 0x7f);
		if (isprint(c2))
		    (void) printw("M-%c", c2);
		else
		    (void) printw("M-%s", unctrl(c2));
		addstr(" (high-half character)\n");
	    }
	    else
	    {
		if (isprint(c))
		    (void) printw("%c (ASCII printable character)\n", c);
		else
		    (void) printw("%s (ASCII control character)\n", unctrl(c));
	    }
	    getyx(stdscr, y, x);
	    if (y >= LINES-1)
		move(0,0);
	    clrtoeol();
	}

	if (c == 'g')
	{
	    addstr("getstr test: ");
	    echo(); getstr(buf); noecho();
	    printw("I saw `%s'.\n", buf);
	}
	if (c == 's')
	{
	    addstr("Shelling out...");
	    def_prog_mode();
	    endwin();
	    system("sh");
	    addstr("returned from shellout.\n");
	    refresh();
	}
	if (c == 'x' || c == 'q')
	    break;
	if (c == '?')
	{
	    addstr("Type any key to see its keypad value.  Also:\n");
	    addstr("g -- triggers a getstr test\n");
	    addstr("s -- shell out\n");
	    addstr("q -- quit\n");
	    addstr("? -- repeats this help message\n");
	}

	while ((c = Getchar()) == ERR)
	    if (!blocking)
		(void) printw("%05d: input timed out\n", incount++);
    }

#ifdef NCURSES_VERSION
    mousemask(0, (mmask_t *)NULL);
#endif
    timeout(-1);
    erase();
    noraw();
    nl();
    endwin();
}

static void attr_test(void)
/* test text attributes */
{
    refresh();

    mvaddstr(0, 20, "Character attribute test display");

    mvaddstr(2,8,"This is STANDOUT mode: ");
    attron(A_STANDOUT);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_STANDOUT);

    mvaddstr(4,8,"This is REVERSE mode: ");
    attron(A_REVERSE);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_REVERSE);

    mvaddstr(6,8,"This is BOLD mode: ");
    attron(A_BOLD);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_BOLD);

    mvaddstr(8,8,"This is UNDERLINE mode: ");
    attron(A_UNDERLINE);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_UNDERLINE);

    mvaddstr(10,8,"This is DIM mode: ");
    attron(A_DIM);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_DIM);

    mvaddstr(12,8,"This is BLINK mode: ");
    attron(A_BLINK);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_BLINK);

    mvaddstr(14,8,"This is PROTECT mode: ");
    attron(A_PROTECT);
    addstr("abcde fghij klmno pqrst uvwxy z");
    attroff(A_PROTECT);

    attrset(A_INVIS);
    mvaddstr(16,8,"This is INVISIBLE mode: ");
    addstr("abcde fghij klmno pqrst uvwxy z");

    attrset(A_NORMAL);
    mvaddstr(18,8,"This is NORMAL mode: ");
    addstr("abcde fghij klmno pqrst uvwxy z");

    refresh();

    Pause();

    erase();
    endwin();
}

/****************************************************************************
 *
 * Color support tests
 *
 ****************************************************************************/

static char	*colors[] =
{
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "magenta",
    "cyan",
    "white"
};

static void color_test(void)
/* generate a color test pattern */
{
    int i;

    refresh();
    (void) printw("There are %d color pairs\n", COLOR_PAIRS);

    (void) mvprintw(1, 0,
	 "%dx%d matrix of foreground/background colors, bright *off*\n",
	 COLORS, COLORS);
    for (i = 0; i < COLORS; i++)
	mvaddstr(2, (i+1) * 8, colors[i]);
    for (i = 0; i < COLORS; i++)
	mvaddstr(3 + i, 0, colors[i]);
    for (i = 1; i < COLOR_PAIRS; i++)
    {
	init_pair(i, i % COLORS, i / COLORS);
	attron((attr_t)COLOR_PAIR(i));
	mvaddstr(3 + (i / COLORS), (i % COLORS + 1) * 8, "Hello");
	attrset(A_NORMAL);
    }

    (void) mvprintw(COLORS + 4, 0,
	   "%dx%d matrix of foreground/background colors, bright *on*\n",
	   COLORS, COLORS);
    for (i = 0; i < COLORS; i++)
	mvaddstr(5 + COLORS, (i+1) * 8, colors[i]);
    for (i = 0; i < COLORS; i++)
	mvaddstr(6 + COLORS + i, 0, colors[i]);
    for (i = 1; i < COLOR_PAIRS; i++)
    {
	init_pair(i, i % COLORS, i / COLORS);
	attron((attr_t)(COLOR_PAIR(i) | A_BOLD));
	mvaddstr(6 + COLORS + (i / COLORS), (i % COLORS + 1) * 8, "Hello");
	attrset(A_NORMAL);
    }

    Pause();

    erase();
    endwin();
}

static void color_edit(void)
/* display the color test pattern, without trying to edit colors */
{
    int	i, c, value = 0, current = 0, field = 0, usebase = 0;

    refresh();

    for (i = 0; i < COLORS; i++)
	init_pair(i, COLOR_WHITE, i);

    do {
	short	red, green, blue;

	attron(A_BOLD);
	mvaddstr(0, 20, "Color RGB Value Editing");
	attroff(A_BOLD);

	for (i = 0; i < COLORS; i++)
        {
	    mvprintw(2 + i, 0, "%c %-8s:",
		     (i == current ? '>' : ' '),
		     (i < (int) SIZEOF(colors)
			? colors[i] : ""));
	    attrset(COLOR_PAIR(i));
	    addstr("        ");
	    attrset(A_NORMAL);

	    /*
	     * Note: this refresh should *not* be necessary!  It works around
	     * a bug in attribute handling that apparently causes the A_NORMAL
	     * attribute sets to interfere with the actual emission of the
	     * color setting somehow.  This needs to be fixed.
	     */
	    refresh();

	    color_content(i, &red, &green, &blue);
	    addstr("   R = ");
	    if (current == i && field == 0) attron(A_STANDOUT);
	    printw("%04d", red);
	    if (current == i && field == 0) attrset(A_NORMAL);
	    addstr(", G = ");
	    if (current == i && field == 1) attron(A_STANDOUT);
	    printw("%04d", green);
	    if (current == i && field == 1) attrset(A_NORMAL);
	    addstr(", B = ");
	    if (current == i && field == 2) attron(A_STANDOUT);
	    printw("%04d", blue);
	    if (current == i && field == 2) attrset(A_NORMAL);
	    attrset(A_NORMAL);
	    addstr(")");
	}

	mvaddstr(COLORS + 3, 0,
	    "Use up/down to select a color, left/right to change fields.");
	mvaddstr(COLORS + 4, 0,
	    "Modify field by typing nnn=, nnn-, or nnn+.  ? for help.");

	move(2 + current, 0);

	switch (c = Getchar())
	{
	case KEY_UP:
	    current = (current == 0 ? (COLORS - 1) : current - 1);
	    value = 0;
	    break;

	case KEY_DOWN:
	    current = (current == (COLORS - 1) ? 0 : current + 1);
	    value = 0;
	    break;

	case KEY_RIGHT:
	    field = (field == 2 ? 0 : field + 1);
	    value = 0;
	    break;

	case KEY_LEFT:
	    field = (field == 0 ? 2 : field - 1);
	    value = 0;
	    break;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    do {
		value = value * 10 + (c - '0');
		c = Getchar();
	    } while
		(isdigit(c));
	    if (c != '+' && c != '-' && c != '=')
		beep();
	    else
		ungetch(c);
	    break;

	case '+':
	    usebase = 1;
	    goto changeit;

	case '-':
	    value = -value;
	    usebase = 1;
	    goto changeit;

	case '=':
	    usebase = 0;
	changeit:
	    color_content(current, &red, &green, &blue);
	    if (field == 0)
		red = red * usebase + value;
	    else if (field == 1)
		green = green * usebase + value;
	    else if (field == 2)
		blue = blue * usebase + value;
	    init_color(current, red, green, blue);
	    break;

	case '?':
	    erase();
    P("                      RGB Value Editing Help");
    P("");
    P("You are in the RGB value editor.  Use the arrow keys to select one of");
    P("the fields in one of the RGB triples of the current colors; the one");
    P("currently selected will be reverse-video highlighted.");
    P("");
    P("To change a field, enter the digits of the new value; they won't be");
    P("echoed.  Finish by typing `='; the change will take effect instantly.");
    P("To increment or decrement a value, use the same procedure, but finish");
    P("with a `+' or `-'.");
    P("");
    P("To quit, do `x' or 'q'");

	    Pause();
	    erase();
	    break;

	case 'x':
	case 'q':
	    break;

	default:
	    beep();
	    break;
	}
    } while
	(c != 'x' && c != 'q');

    erase();
    endwin();
}

/****************************************************************************
 *
 * Soft-key label test
 *
 ****************************************************************************/

static void slk_test(void)
/* exercise the soft keys */
{
    int	c, fmt = 1;
    char buf[9];

    c = CTRL('l');
    do {
	move(0, 0); 
	switch(c)
	{
	case CTRL('l'):
	    erase();
	    attron(A_BOLD);
	    mvaddstr(0, 20, "Soft Key Exerciser");
	    attroff(A_BOLD);

	    move(2, 0);
	    P("Available commands are:");
	    P("");
	    P("^L         -- refresh screen");
	    P("a          -- activate or restore soft keys");
	    P("d          -- disable soft keys");
	    P("c          -- set centered format for labels");
	    P("l          -- set left-justified format for labels");
	    P("r          -- set right-justified format for labels");
	    P("[12345678] -- set label; labels are numbered 1 through 8");
	    P("e          -- erase stdscr (should not erase labels)");
	    P("s          -- test scrolling of shortened screen");
	    P("x, q       -- return to main menu");
	    P("");
	    P("Note: if activating the soft keys causes your terminal to");
	    P("scroll up one line, your terminal auto-scrolls when anything");
	    P("is written to the last screen position.  The ncurses code");
	    P("does not yet handle this gracefully.");
	    refresh();
	    /* fall through */

	case 'a':
	    slk_restore();
	    break;

	case 'e':
	    wclear(stdscr);
	    break;

	case 's':
	    move(20, 0);
	    while ((c = Getchar()) != 'Q' && (c != ERR))
		addch((chtype)c);
	    break;

	case 'd':
	    slk_clear();
	    break;

	case 'l':
	    fmt = 0;
	    break;

	case 'c':
	    fmt = 1;
	    break;

	case 'r':
	    fmt = 2;
	    break;

	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8':
	    (void) mvaddstr(20, 0, "Please enter the label value: ");
	    echo();
	    wgetnstr(stdscr, buf, 8);
	    noecho();
	    slk_set((c - '0'), buf, fmt);
	    slk_refresh();
	    move(20, 0); clrtoeol(); 
	    break;

	case 'x':
	case 'q':
	    goto done;

	default:
	    beep();
	}
    } while
	((c = Getchar()) != EOF);

 done:
    erase();
    endwin();
}

/****************************************************************************
 *
 * Alternate character-set stuff
 *
 ****************************************************************************/

/* ISO 6429:  codes 0x80 to 0x9f may be control characters that cause the
 * terminal to perform functions.  The remaining codes can be graphic.
 */
static void show_upper_chars(int first)
{
	bool C1 = (first == 128);
	int code;
	int last = first + 31;
	int reply;

	erase();
	attron(A_BOLD);
	mvprintw(0, 20, "Display of %s Character Codes %d to %d",
		C1 ? "C1" : "GR", first, last);
	attroff(A_BOLD);
	refresh();

	for (code = first; code <= last; code++) {
		int row = 4 + ((code - first) % 16);
		int col = ((code - first) / 16) * COLS / 2;
		char tmp[80];
		sprintf(tmp, "%3d (0x%x)", code, code);
		mvprintw(row, col, "%*s: ", COLS/4, tmp);
		if (C1)
			nodelay(stdscr, TRUE);
		echochar(code);
		if (C1) {
			/* (yes, this _is_ crude) */
			while ((reply = Getchar()) != ERR) {
				addch(reply);
				napms(10);
			}
			nodelay(stdscr, TRUE);
		}
	}
}

static int show_1_acs(int n, char *name, chtype code)
{
	const int height = 16;
	int row = 4 + (n % height);
	int col = (n / height) * COLS / 2;
	mvprintw(row, col, "%*s : ", COLS/4, name);
	addch(code);
	return n + 1;
}

static void show_acs_chars(void)
/* display the ACS character set */
{
	int n;

#define BOTH(name) #name, name

	erase();
	attron(A_BOLD);
	mvaddstr(0, 20, "Display of the ACS Character Set");
	attroff(A_BOLD);
	refresh();

	n = show_1_acs(0, BOTH(ACS_ULCORNER));
	n = show_1_acs(n, BOTH(ACS_LLCORNER));
	n = show_1_acs(n, BOTH(ACS_URCORNER));
	n = show_1_acs(n, BOTH(ACS_LRCORNER));
	n = show_1_acs(n, BOTH(ACS_RTEE));
	n = show_1_acs(n, BOTH(ACS_LTEE));
	n = show_1_acs(n, BOTH(ACS_BTEE));
	n = show_1_acs(n, BOTH(ACS_TTEE));
	n = show_1_acs(n, BOTH(ACS_HLINE));
	n = show_1_acs(n, BOTH(ACS_VLINE));
	n = show_1_acs(n, BOTH(ACS_PLUS));
	n = show_1_acs(n, BOTH(ACS_S1));
	n = show_1_acs(n, BOTH(ACS_S9));
	n = show_1_acs(n, BOTH(ACS_DIAMOND));
	n = show_1_acs(n, BOTH(ACS_CKBOARD));
	n = show_1_acs(n, BOTH(ACS_DEGREE));
	n = show_1_acs(n, BOTH(ACS_PLMINUS));
	n = show_1_acs(n, BOTH(ACS_BULLET));
	n = show_1_acs(n, BOTH(ACS_LARROW));
	n = show_1_acs(n, BOTH(ACS_RARROW));
	n = show_1_acs(n, BOTH(ACS_DARROW));
	n = show_1_acs(n, BOTH(ACS_UARROW));
	n = show_1_acs(n, BOTH(ACS_BOARD));
	n = show_1_acs(n, BOTH(ACS_LANTERN));
	n = show_1_acs(n, BOTH(ACS_BLOCK));
	n = show_1_acs(n, BOTH(ACS_S3));
	n = show_1_acs(n, BOTH(ACS_S7));
	n = show_1_acs(n, BOTH(ACS_LEQUAL));
	n = show_1_acs(n, BOTH(ACS_GEQUAL));
	n = show_1_acs(n, BOTH(ACS_PI));
	n = show_1_acs(n, BOTH(ACS_NEQUAL));
	n = show_1_acs(n, BOTH(ACS_STERLING));
}

static void acs_display(void)
{
	int	c = 'a';

	do {
		switch (c) {
		case 'a':
			show_acs_chars();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
			show_upper_chars((c - '0') * 32 + 128);
			break;
		}
		mvprintw(LINES-3,0, "Note: ANSI terminals may not display C1 characters.");
		mvprintw(LINES-2,0, "Select: a=ACS, 0=C1, 1,2,3=GR characters, q=quit");
		refresh();
	} while ((c = Getchar()) != 'x' && c != 'q');

	Pause();
	erase();
	endwin();
}

/*
 * Graphic-rendition test (adapted from vttest)
 */
static void
test_sgr_attributes(void)
{
    int pass;

    for (pass = 0; pass < 2; pass++) {
	int normal = ((pass == 0 ? A_NORMAL : A_REVERSE)) | BLANK;

	/* Use non-default colors if possible to exercise bce a little */
	if (has_colors()) {
	    init_pair(1, COLOR_WHITE, COLOR_BLUE);
	    normal |= COLOR_PAIR(1);
	}
	bkgdset(normal);
	erase();
	mvprintw( 1,20, "Graphic rendition test pattern:");

	mvprintw( 4, 1, "vanilla");

#define set_sgr(mask) bkgdset((normal^(mask)));
	set_sgr(A_BOLD);
	mvprintw( 4,40, "bold");

	set_sgr(A_UNDERLINE);
	mvprintw( 6, 6, "underline");

	set_sgr(A_BOLD|A_UNDERLINE);
	mvprintw( 6,45, "bold underline");

	set_sgr(A_BLINK);
	mvprintw( 8, 1, "blink");

	set_sgr(A_BLINK|A_BOLD);
	mvprintw( 8,40, "bold blink");

	set_sgr(A_UNDERLINE|A_BLINK);
	mvprintw(10, 6, "underline blink");

	set_sgr(A_BOLD|A_UNDERLINE|A_BLINK);
	mvprintw(10,45, "bold underline blink");

	set_sgr(A_REVERSE);
	mvprintw(12, 1, "negative");

	set_sgr(A_BOLD|A_REVERSE);
	mvprintw(12,40, "bold negative");

	set_sgr(A_UNDERLINE|A_REVERSE);
	mvprintw(14, 6, "underline negative");

	set_sgr(A_BOLD|A_UNDERLINE|A_REVERSE);
	mvprintw(14,45, "bold underline negative");

	set_sgr(A_BLINK|A_REVERSE);
	mvprintw(16, 1, "blink negative");

	set_sgr(A_BOLD|A_BLINK|A_REVERSE);
	mvprintw(16,40, "bold blink negative");

	set_sgr(A_UNDERLINE|A_BLINK|A_REVERSE);
	mvprintw(18, 6, "underline blink negative");

	set_sgr(A_BOLD|A_UNDERLINE|A_BLINK|A_REVERSE);
	mvprintw(18,45, "bold underline blink negative");

	bkgdset(normal);
	mvprintw(LINES-2,1, "%s background. ", pass == 0 ? "Dark" : "Light");
	clrtoeol();
	Pause();
    }

    bkgdset(A_NORMAL | BLANK);
    erase();
    endwin();
}

/****************************************************************************
 *
 * Windows and scrolling tester.
 *
 ****************************************************************************/

#define BOTLINES	4	/* number of line stolen from screen bottom */

typedef struct
{
    int y, x;
}
pair;

struct frame
{
	struct frame	*next, *last;
	bool		flag;
	WINDOW		*wind;
};

static void transient(struct frame *curp, char *msg)
{
    if (msg)
    {
	mvaddstr(LINES - 1, 0, msg);
	refresh();
	sleep(1);
    }

    move(LINES-1, 0);
    printw("All other characters are echoed, window should %sscroll.",
	((curp != 0) && curp->flag) ? "" : "not " );
    clrtoeol();
    refresh();
}

static void newwin_report(struct frame *curp)
/* report on the cursor's current position, then restore it */
{
	WINDOW *win = (curp != 0) ? curp->wind : stdscr;
	int y, x;

	if (win != stdscr)
		transient(curp, (char *)0);
	getyx(win, y, x);
	move(LINES - 1, COLS - 17);
	printw("Y = %2d X = %2d", y, x);
	if (win != stdscr)
		refresh();
	else
		wmove(win, y, x);
}

static pair *selectcell(int uli, int ulj, int lri, int lrj)
/* arrows keys move cursor, return location at current on non-arrow key */
{
    static pair	res;			/* result cell */
    int		si = lri - uli + 1;	/* depth of the select area */
    int		sj = lrj - ulj + 1;	/* width of the select area */
    int		i = 0, j = 0;		/* offsets into the select area */

    res.y = uli;
    res.x = ulj;
    for (;;)
    {
	move(uli + i, ulj + j);
	newwin_report((struct frame *)0);

	switch(Getchar())
	{
	case KEY_UP:	i += si - 1; break;
	case KEY_DOWN:	i++; break;
	case KEY_LEFT:	j += sj - 1; break;
	case KEY_RIGHT:	j++; break;
	case QUIT:
	case ESCAPE:	return((pair *)NULL);
	default:	res.y = uli + i; res.x = ulj + j; return(&res);
	}
	i %= si;
	j %= sj;
    }
}

static void outerbox(pair ul, pair lr, bool onoff)
/* draw or erase a box *outside* the given pair of corners */
{
    mvaddch(ul.y-1, lr.x-1, onoff ? ACS_ULCORNER : ' ');
    mvaddch(ul.y-1, lr.x+1, onoff ? ACS_URCORNER : ' ');
    mvaddch(lr.y+1, lr.x+1, onoff ? ACS_LRCORNER : ' ');
    mvaddch(lr.y+1, ul.x-1, onoff ? ACS_LLCORNER : ' ');
    move(ul.y-1, ul.x);   hline(onoff ? ACS_HLINE : ' ', lr.x - ul.x + 1);
    move(ul.y,   ul.x-1); vline(onoff ? ACS_VLINE : ' ', lr.y - ul.y + 1);
    move(lr.y+1, ul.x);   hline(onoff ? ACS_HLINE : ' ', lr.x - ul.x + 1);
    move(ul.y,   lr.x+1); vline(onoff ? ACS_VLINE : ' ', lr.y - ul.y + 1);
}

static WINDOW *getwindow(void)
/* Ask user for a window definition */
{
    WINDOW	*rwindow;
    pair	ul, lr, *tmp;

    move(0, 0); clrtoeol();
    addstr("Use arrows to move cursor, anything else to mark corner 1");
    refresh();
    if ((tmp = selectcell(2, 1, LINES-BOTLINES-2, COLS-2)) == (pair *)NULL)
	return((WINDOW *)NULL);
    memcpy(&ul, tmp, sizeof(pair));
    mvaddch(ul.y-1, ul.x-1, ACS_ULCORNER);
    move(0, 0); clrtoeol();
    addstr("Use arrows to move cursor, anything else to mark corner 2");
    refresh();
    if ((tmp = selectcell(ul.y, ul.x, LINES-BOTLINES-2, COLS-2)) == (pair *)NULL)
	return((WINDOW *)NULL);
    memcpy(&lr, tmp, sizeof(pair));

    rwindow = subwin(stdscr, lr.y - ul.y + 1, lr.x - ul.x + 1, ul.y, ul.x);

    outerbox(ul, lr, TRUE);
    refresh();

    wrefresh(rwindow);

    move(0, 0); clrtoeol();
    return(rwindow);
}

static void newwin_legend(void)
{
	static const char *const legend[] = {
	"^C = make new window, ^N = next window, ^P = previous window,",
	"^F = scroll forward, ^B = scroll backward, ^S toggle scrollok",
	"^W = save window to file, ^R = restore window, ^X = resize, ^Q/ESC = exit"
	};
	size_t n;
	for (n = 0; n < SIZEOF(legend); n++) {
		int line = LINES - SIZEOF(legend) - 1 + n;
		mvprintw(line, 0, legend[n]);
		clrtoeol();
	}
}

static void newwin_move(struct frame *curp, int dy, int dx)
{
	WINDOW *win = (curp != 0) ? curp->wind : stdscr;
	int cur_y, cur_x;
	int max_y, max_x;

	getyx(win, cur_y, cur_x);
	getmaxyx(win, max_y, max_x);
	if ((cur_x += dx) < 0)
		cur_x = 0;
	else if (cur_x >= max_x)
		cur_x = max_x - 1;
	if ((cur_y += dy) < 0)
		cur_y = 0;
	else if (cur_y >= max_y)
		cur_y = max_y - 1;
	wmove(win, cur_y, cur_x);
}

static void acs_and_scroll(void)
/* Demonstrate windows */
{
    int	c, i;
    FILE *fp;
    struct frame *current = (struct frame *)0, *neww;

#define DUMPFILE	"screendump"

    newwin_legend();
    transient((struct frame *)0, (char *)0);

    c = CTRL('C');
    raw();
    do {
	switch(c)
	{
	case CTRL('C'):
	    neww = (struct frame *) malloc(sizeof(struct frame));
	    if ((neww->wind = getwindow()) == (WINDOW *)NULL)
		goto breakout;

	    if (current == NULL)	/* First element,  */
	    {
		neww->next = neww; /*   so point it at itself */
		neww->last = neww;
	    }
	    else
	    {
		neww->next = current->next;
		neww->last = current;
		neww->last->next = neww;
		neww->next->last = neww;
	    }
	    current = neww;
	    keypad(neww->wind, TRUE);
	    scrollok(current->wind, current->flag = TRUE);
	    transient(current, (char *)0);
	    break;

	case CTRL('N'):		/* go to next window */
	    if (current)
		current = current->next;
	    break;

	case CTRL('P'):		/* go to previous window */
	    if (current)
		current = current->last;
	    break;

	case CTRL('F'):		/* scroll current window forward */
	    if (current)
		wscrl(current->wind, 1);
	    break;

	case CTRL('B'):		/* scroll current window backwards */
	    if (current)
		wscrl(current->wind, -1);
	    break;

	case CTRL('S'):
	    if (current) {
		current->flag = !current->flag;
		scrollok(current->wind, current->flag);
	    }
	    transient(current, (char *)0);
	    break;

	case CTRL('W'):		/* save and delete window */
	    if (current == current->next)
		break;
	    if ((fp = fopen(DUMPFILE, "w")) == (FILE *)NULL)
		transient(current, "Can't open screen dump file");
	    else
	    {
		(void) putwin(current->wind, fp);
		(void) fclose(fp);

		current->last->next = current->next;
		current->next->last = current->last;

		werase(current->wind);
		wrefresh(current->wind);
		delwin(current->wind);

		neww = current->next;
		free(current);
		current = neww;
	    }
	    break;

	case CTRL('R'):		/* restore window */
	    if ((fp = fopen(DUMPFILE, "r")) == (FILE *)NULL)
		transient(current, "Can't open screen dump file");
	    else
	    {
		neww = (struct frame *) malloc(sizeof(struct frame));

		neww->next = current->next;
		neww->last = current;
		neww->last->next = neww;
		neww->next->last = neww;

		neww->wind = getwin(fp);
		(void) fclose(fp);

		wrefresh(neww->wind);
	    }
	    break;

#ifdef NCURSES_VERSION
	case CTRL('X'):		/* resize window */
	    if (current)
	    {
		pair *tmp, ul, lr;

		move(0, 0); clrtoeol();
		addstr("Use arrows to move cursor, anything else to mark new corner");
		refresh();

		getbegyx(current->wind, ul.y, ul.x);

		tmp = selectcell(ul.y, ul.x, LINES-BOTLINES-2, COLS-2);
		if (tmp == (pair *)NULL)
		    break;

		getmaxyx(current->wind, lr.y, lr.x);
		lr.y += (ul.y - 1);
		lr.x += (ul.x - 1);
		outerbox(ul, lr, FALSE);
		wnoutrefresh(stdscr);

		/* strictly cosmetic hack for the test */
		if (current->wind->_maxy > tmp->y - ul.y)
		{
		  getyx(current->wind, lr.y, lr.x);
		  wmove(current->wind, tmp->y - ul.y + 1, 0);
		  wclrtobot(current->wind);
		  wmove(current->wind, lr.y, lr.x);
		}
		if (current->wind->_maxx > tmp->x - ul.x)
		  for (i = 0; i < current->wind->_maxy; i++)
		  {
		    wmove(current->wind, i, tmp->x - ul.x + 1);
		    wclrtoeol(current->wind);
		  }
		wnoutrefresh(current->wind);

		memcpy(&lr, tmp, sizeof(pair));
		(void) wresize(current->wind, lr.y-ul.y+1, lr.x-ul.x+1);

		getbegyx(current->wind, ul.y, ul.x);
		getmaxyx(current->wind, lr.y, lr.x);
		lr.y += (ul.y - 1);
		lr.x += (ul.x - 1);
		outerbox(ul, lr, TRUE);
		wnoutrefresh(stdscr);

		wnoutrefresh(current->wind);
		doupdate();
	    }
	    break;
#endif	/* NCURSES_VERSION */

	case KEY_F(10):	/* undocumented --- use this to test area clears */
	    selectcell(0, 0, LINES - 1, COLS - 1);
	    clrtobot();
	    refresh();
	    break;

	case KEY_UP:
	    newwin_move(current, -1,  0);
	    break;
	case KEY_DOWN:
	    newwin_move(current,  1,  0);
	    break;
	case KEY_LEFT:
	    newwin_move(current,  0, -1);
	    break;
	case KEY_RIGHT:
	    newwin_move(current,  0,  1);
	    break;

	case '\r':
	    c = '\n';
	    /* FALLTHROUGH */

	default:
	    if (current)
		waddch(current->wind, (chtype)c);
	    else
		beep();
	    break;
	}
	newwin_report(current);
	wrefresh(current ? current->wind : stdscr);
    } while
	((c = wGetchar(current ? current->wind : stdscr)) != QUIT
	 && (c != ESCAPE)
	 && (c != ERR));

 breakout:
    scrollok(stdscr, TRUE);	/* reset to driver's default */
    erase();
    endwin();
}

/****************************************************************************
 *
 * Panels tester
 *
 ****************************************************************************/

#if HAVE_PANEL_H
static PANEL *p1;
static PANEL *p2;
static PANEL *p3;
static PANEL *p4;
static PANEL *p5;
static WINDOW *w1;
static WINDOW *w2;
static WINDOW *w3;
static WINDOW *w4;
static WINDOW *w5;

static unsigned long nap_msec = 1;

char *mod[] =
{
	"test ",
	"TEST ",
	"(**) ",
	"*()* ",
	"<--> ",
	"LAST "
};

/*+-------------------------------------------------------------------------
	wait_a_while(msec)
--------------------------------------------------------------------------*/
static void
wait_a_while(unsigned long msec GCC_UNUSED)
{
#if HAVE_NAPMS
	if(nap_msec == 1)
		getchar();
	else
		napms(nap_msec);
#else
	if(nap_msec == 1)
		getchar();
	else if(msec > 1000L)
		sleep((int)msec/1000L);
	else
		sleep(1);
#endif
}	/* end of wait_a_while */

/*+-------------------------------------------------------------------------
	saywhat(text)
--------------------------------------------------------------------------*/
static void
saywhat(char *text)
{
	wmove(stdscr,LINES - 1,0);
	wclrtoeol(stdscr);
	waddstr(stdscr, text);
}	/* end of saywhat */

/*+-------------------------------------------------------------------------
	mkpanel(rows,cols,tly,tlx) - alloc a win and panel and associate them
--------------------------------------------------------------------------*/
static PANEL *
mkpanel(int rows, int cols, int tly, int tlx)
{
WINDOW *win = newwin(rows,cols,tly,tlx);
PANEL *pan;

	if(!win)
		return((PANEL *)0);
	if((pan = new_panel(win)))
		return(pan);
	delwin(win);
	return((PANEL *)0);
}	/* end of mkpanel */

/*+-------------------------------------------------------------------------
	rmpanel(pan)
--------------------------------------------------------------------------*/
static void
rmpanel(PANEL *pan)
{
WINDOW *win = panel_window(pan);
	del_panel(pan);
	delwin(win);
}	/* end of rmpanel */

/*+-------------------------------------------------------------------------
	pflush()
--------------------------------------------------------------------------*/
static void
pflush(void)
{
	update_panels();
	doupdate();
}	/* end of pflush */

/*+-------------------------------------------------------------------------
	fill_panel(win)
--------------------------------------------------------------------------*/
static void
fill_panel(PANEL *pan)
{
WINDOW *win = panel_window(pan);
int num = panel_userptr(pan)[1];
int y,x;

	box(win, 0, 0);
	wmove(win,1,1);
	wprintw(win,"-pan%c-", num);
	for(y = 2; y < getmaxy(win) - 1; y++)
	{
		for(x = 1; x < getmaxx(win) - 1; x++)
		{
			wmove(win,y,x);
			waddch(win,num);
		}
	}
}	/* end of fill_panel */

static void demo_panels(void)
{
int itmp;
register y,x;

	refresh();

	for(y = 0; y < LINES - 1; y++)
	{
		for(x = 0; x < COLS; x++)
			wprintw(stdscr,"%d",(y + x) % 10);
	}
	for(y = 0; y < 5; y++)
	{
		p1 = mkpanel(10,10,0,0);
		w1 = panel_window(p1);
		set_panel_userptr(p1,"p1");

		p2 = mkpanel(14,14,5,5);
		w2 = panel_window(p2);
		set_panel_userptr(p2,"p2");

		p3 = mkpanel(6,8,12,12);
		w3 = panel_window(p3);
		set_panel_userptr(p3,"p3");

		p4 = mkpanel(10,10,10,30);
		w4 = panel_window(p4);
		set_panel_userptr(p4,"p4");

		p5 = mkpanel(10,10,13,37);
		w5 = panel_window(p5);
		set_panel_userptr(p5,"p5");

		fill_panel(p1);
		fill_panel(p2);
		fill_panel(p3);
		fill_panel(p4);
		fill_panel(p5);
		hide_panel(p4);
		hide_panel(p5);
		pflush();
		saywhat("press any key to continue");
		wait_a_while(nap_msec);

		saywhat("h3 s1 s2 s4 s5; press any key to continue");
		move_panel(p1,0,0);
		hide_panel(p3);
		show_panel(p1);
		show_panel(p2);
		show_panel(p4);
		show_panel(p5);
		pflush();
		wait_a_while(nap_msec);

		saywhat("s1; press any key to continue");
		show_panel(p1);
		pflush();
		wait_a_while(nap_msec);

		saywhat("s2; press any key to continue");
		show_panel(p2);
		pflush();
		wait_a_while(nap_msec);

		saywhat("m2; press any key to continue");
		move_panel(p2,10,10);
		pflush();
		wait_a_while(nap_msec);

		saywhat("s3;");
		show_panel(p3);
		pflush();
		wait_a_while(nap_msec);

		saywhat("m3; press any key to continue");
		move_panel(p3,5,5);
		pflush();
		wait_a_while(nap_msec);

		saywhat("b3; press any key to continue");
		bottom_panel(p3);
		pflush();
		wait_a_while(nap_msec);

		saywhat("s4; press any key to continue");
		show_panel(p4);
		pflush();
		wait_a_while(nap_msec);

		saywhat("s5; press any key to continue");
		show_panel(p5);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t3; press any key to continue");
		top_panel(p3);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t1; press any key to continue");
		top_panel(p1);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t2; press any key to continue");
		top_panel(p2);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t3; press any key to continue");
		top_panel(p3);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t4; press any key to continue");
		top_panel(p4);
		pflush();
		wait_a_while(nap_msec);

		for(itmp = 0; itmp < 6; itmp++)
		{
			saywhat("m4; press any key to continue");
			wmove(w4,3,1);
			waddstr(w4,mod[itmp]);
			move_panel(p4,4,itmp*10);
			wmove(w5,4,1);
			waddstr(w5,mod[itmp]);
			pflush();
			wait_a_while(nap_msec);
			saywhat("m5; press any key to continue");
			wmove(w4,4,1);
			waddstr(w4,mod[itmp]);
			move_panel(p5,7,(itmp*10) + 6);
			wmove(w5,3,1);
			waddstr(w5,mod[itmp]);
			pflush();
			wait_a_while(nap_msec);
		}

		saywhat("m4; press any key to continue");
		move_panel(p4,4,itmp*10);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t5; press any key to continue");
		top_panel(p5);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t2; press any key to continue");
		top_panel(p2);
		pflush();
		wait_a_while(nap_msec);

		saywhat("t1; press any key to continue");
		top_panel(p1);
		pflush();
		wait_a_while(nap_msec);

		saywhat("d2; press any key to continue");
		rmpanel(p2);
		pflush();
		wait_a_while(nap_msec);

		saywhat("h3; press any key to continue");
		hide_panel(p3);
		pflush();
		wait_a_while(nap_msec);

		saywhat("d1; press any key to continue");
		rmpanel(p1);
		pflush();
		wait_a_while(nap_msec);

		saywhat("d4; press any key to continue");
		rmpanel(p4);
		pflush();
		wait_a_while(nap_msec);

		saywhat("d5; press any key to continue");
		rmpanel(p5);
		pflush();
		wait_a_while(nap_msec);
		if(nap_msec == 1)
			break;
		nap_msec = 100L;
	}

    erase();
    endwin();
}

/****************************************************************************
 *
 * Pad tester
 *
 ****************************************************************************/

#define GRIDSIZE	3

static bool show_panner_legend = TRUE;

static int panner_legend(int line)
{
	static const char *const legend[] = {
		"Use arrow keys (or U,D,L,R) to pan, q to quit (?,t,s flags)",
		"Use +,- (or j,k) to grow/shrink the panner vertically.",
		"Use <,> (or h,l) to grow/shrink the panner horizontally."
	};
	int n = (SIZEOF(legend) - (LINES - line));
	if (line < LINES && (n >= 0)) {
		move(line, 0);
		if (show_panner_legend)
			printw("%s", legend[n]);
		clrtoeol();
		return show_panner_legend;
	}
	return FALSE;
}

static void panner_h_cleanup(int from_y, int from_x, int to_x)
{
	if (!panner_legend(from_y))
		do_h_line(from_y, from_x, ' ', to_x);
}

static void panner_v_cleanup(int from_y, int from_x, int to_y)
{
	if (!panner_legend(from_y))
		do_v_line(from_y, from_x, ' ', to_y);
}

static void panner(WINDOW *pad,
		   int hx, int hy, int iy, int ix,
		   int (*pgetc)(WINDOW *))
{
#if HAVE_GETTIMEOFDAY
    struct timeval before, after;
    bool timing = TRUE;
#endif
    bool scrollers = TRUE;
    int basex = 0;
    int basey = 0;
    int portx = ix;
    int porty = iy;
    int top_x = hx;
    int top_y = hy;
    int pxmax, pymax, lowend, highend, c;

    getmaxyx(pad, pymax, pxmax);
    scrollok(stdscr, FALSE);	/* we don't want stdscr to scroll! */

    c = KEY_REFRESH;
    do {
	switch(c)
	{
	case '?':
	    show_panner_legend = !show_panner_legend;
	    panner_legend(LINES - 3);
	    panner_legend(LINES - 2);
	    panner_legend(LINES - 1);
	    break;
#if HAVE_GETTIMEOFDAY
	case 't':
	    timing = !timing;
	    if (!timing)
		panner_legend(LINES-1);
	    break;
#endif
	case 's':
	    scrollers = !scrollers;
	    break;

	case KEY_REFRESH:
	    /* do nothing */
	    break;

	    /* Move the top-left corner of the pad, keeping the bottom-right
	     * corner fixed.
	     */
	case 'h':	/* increase-columns: move left edge to left */
	    if (top_x <= 0)
		beep();
	    else
	    {
		panner_v_cleanup(top_y, top_x, porty);
		top_x--;
	    }
	    break;

	case 'j':	/* decrease-lines: move top-edge down */
	    if (top_y >= porty)
		beep();
	    else
	    {
		panner_h_cleanup(top_y - 1, top_x - (top_x > 0), portx);
		top_y++;
	    }
	    break;

	case 'k':	/* increase-lines: move top-edge up */
	    if (top_y <= 0)
		beep();
	    else
	    {
		top_y--;
		panner_h_cleanup(top_y, top_x, portx);
	    }
	    break;

	case 'l':	/* decrease-columns: move left-edge to right */
	    if (top_x >= portx)
		beep();
	    else
	    {
		panner_v_cleanup(top_y - (top_y > 0), top_x - 1, porty);
		top_x++;
	    }
	    break;

	    /* Move the bottom-right corner of the pad, keeping the top-left
	     * corner fixed.
	     */
	case KEY_IC:	/* increase-columns: move right-edge to right */
	    if (portx >= pxmax || portx >= COLS)
		beep();
	    else
	    {
		panner_v_cleanup(top_y - (top_y > 0), portx - 1, porty);
		++portx;
	    }
	    break;

	case KEY_IL:	/* increase-lines: move bottom-edge down */
	    if (porty >= pymax || porty >= LINES)
		beep();
	    else
	    {
		panner_h_cleanup(porty - 1, top_x - (top_x > 0), portx);
		++porty;
	    }
	    break;

	case KEY_DC:	/* decrease-columns: move bottom edge up */
	    if (portx <= top_x)
		beep();
	    else
	    {
		portx--;
		panner_v_cleanup(top_y - (top_y > 0), portx, porty);
	    }
	    break;

	case KEY_DL:	/* decrease-lines */
	    if (porty <= top_y)
		beep();
	    else
	    {
		porty--;
		panner_h_cleanup(porty, top_x - (top_x > 0), portx);
	    }
	    break;

	case KEY_LEFT:	/* pan leftwards */
	    if (basex > 0)
		basex--;
	    else
		beep();
	    break;

	case KEY_RIGHT:	/* pan rightwards */
	    if (basex + portx - (pymax > porty) < pxmax)
		basex++;
	    else
		beep();
	    break;

	case KEY_UP:	/* pan upwards */
	    if (basey > 0)
		basey--;
	    else
		beep();
	    break;

	case KEY_DOWN:	/* pan downwards */
	    if (basey + porty - (pxmax > portx) < pymax)
		basey++;
	    else
		beep();
	    break;

	default:
	    beep();
	    break;
	}

	mvaddch(top_y - 1, top_x - 1, ACS_ULCORNER);
	do_v_line(top_y, top_x - 1, ACS_VLINE, porty);
	do_h_line(top_y - 1, top_x, ACS_HLINE, portx);

	if (scrollers && (pxmax > portx - 1)) {
	    int length  = (portx - top_x - 1);
	    float ratio = ((float) length) / ((float) pxmax);

	    lowend  = top_x + (basex * ratio);
	    highend = top_x + ((basex + length) * ratio);

	    do_h_line(porty - 1, top_x, ACS_HLINE, lowend);
	    if (highend < portx) {
		attron(A_REVERSE);
		do_h_line(porty - 1, lowend, ' ', highend + 1);
		attroff(A_REVERSE);
		do_h_line(porty - 1, highend + 1, ACS_HLINE, portx);
	    }
        } else
	    do_h_line(porty - 1, top_x, ACS_HLINE, portx);

	if (scrollers && (pymax > porty - 1)) {
	    int length  = (porty - top_y - 1);
	    float ratio = ((float) length) / ((float) pymax);

	    lowend  = top_y + (basey * ratio);
	    highend = top_y + ((basey + length) * ratio);

	    do_v_line(top_y, portx - 1, ACS_VLINE, lowend);
		if (highend < porty) {
		attron(A_REVERSE);
		do_v_line(lowend, portx - 1, ' ', highend + 1);
		attroff(A_REVERSE);
		do_v_line(highend + 1, portx - 1, ACS_VLINE, porty);
	    }
        } else
	    do_v_line(top_y, portx - 1, ACS_VLINE, porty);

	mvaddch(top_y - 1, portx - 1, ACS_URCORNER);
	mvaddch(porty - 1, top_x - 1, ACS_LLCORNER);
	mvaddch(porty - 1, portx - 1, ACS_LRCORNER);

#if defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&before, NULL);
#endif
	wnoutrefresh(stdscr);

	pnoutrefresh(pad,
		 basey, basex,
		 top_y, top_x,
		 porty - (pxmax > portx) - 1,
		 portx - (pymax > porty) - 1);

	doupdate();
#if defined(HAVE_GETTIMEOFDAY)
	if (timing) {
		double elapsed;
		gettimeofday(&after, NULL);
		elapsed = (after.tv_sec  + after.tv_usec  / 1.0e6)
			- (before.tv_sec + before.tv_usec / 1.0e6);
		move(LINES-1, COLS-20);
		printw("Secs: %2.03f", elapsed);
		refresh();
	}
#endif

    } while
	((c = pgetc(pad)) != KEY_EXIT);

    scrollok(stdscr, TRUE);	/* reset to driver's default */
}

static
int padgetch(WINDOW *win)
{
    int	c;

    switch(c = wGetchar(win))
    {
    case 'U': return(KEY_UP);
    case 'D': return(KEY_DOWN);
    case 'R': return(KEY_RIGHT);
    case 'L': return(KEY_LEFT);
    case '+': return(KEY_IL);
    case '-': return(KEY_DL);
    case '>': return(KEY_IC);
    case '<': return(KEY_DC);
    case 'q': return(KEY_EXIT);
    default: return(c);
    }
}

static void demo_pad(void)
/* Demonstrate pads. */
{
    int i, j;
    unsigned gridcount = 0;
    WINDOW *panpad = newpad(200, 200);

    for (i = 0; i < 200; i++)
    {
	for (j = 0; j < 200; j++)
	    if (i % GRIDSIZE == 0 && j % GRIDSIZE == 0)
	    {
		if (i == 0 || j == 0)
		    waddch(panpad, '+');
		else
		    waddch(panpad, (chtype)('A' + (gridcount++ % 26)));
	    }
	    else if (i % GRIDSIZE == 0)
		waddch(panpad, '-');
	    else if (j % GRIDSIZE == 0)
		waddch(panpad, '|');
	    else
		waddch(panpad, ' ');
    }
    panner_legend(LINES - 3);
    panner_legend(LINES - 2);
    panner_legend(LINES - 1);

    keypad(panpad, TRUE);

    /* Make the pad (initially) narrow enough that a trace file won't wrap.
     * We'll still be able to widen it during a test, since that's required
     * for testing boundaries.
     */
    panner(panpad, 2, 2, LINES - 4, COLS-15, padgetch);

    delwin(panpad);
    endwin();
    erase();
}
#endif /* HAVE_PANEL_H */

/****************************************************************************
 *
 * Tests from John Burnell's PDCurses tester
 *
 ****************************************************************************/

static void Continue (WINDOW *win)
{
    noecho();
    wmove(win, 10, 1);
    mvwaddstr(win, 10, 1, " Press any key to continue");
    wrefresh(win);
    wGetchar(win);
}

static void flushinp_test(WINDOW *win)
/* Input test, adapted from John Burnell's PDCurses tester */
{
    int w, h, bx, by, sw, sh, i;

    WINDOW *subWin;
    wclear (win);

    w  = win->_maxx;
    h  = win->_maxy;
    bx = win->_begx;
    by = win->_begy;
    sw = w / 3;
    sh = h / 3;
    if((subWin = subwin(win, sh, sw, by + h - sh - 2, bx + w - sw - 2)) == NULL)
        return;

#ifdef A_COLOR
    if (has_colors())
    {
	init_pair(2,COLOR_CYAN,COLOR_BLUE);
	wattrset(subWin, COLOR_PAIR(2) | A_BOLD);
    }
    else
	wattrset(subWin, A_BOLD);
#else
    wattrset(subWin, A_BOLD);
#endif
    box(subWin, ACS_VLINE, ACS_HLINE);
    mvwaddstr(subWin, 2, 1, "This is a subwindow");
    wrefresh(win);

    nocbreak();
    mvwaddstr(win, 0, 1, "This is a test of the flushinp() call.");

    mvwaddstr(win, 2, 1, "Type random keys for 5 seconds.");
    mvwaddstr(win, 3, 1,
      "These should be discarded (not echoed) after the subwindow goes away.");
    wrefresh(win);

    for (i = 0; i < 5; i++)
    {
	mvwprintw (subWin, 1, 1, "Time = %d", i);
	wrefresh(subWin);
	sleep(1);
	flushinp();
    }

    delwin (subWin);
    werase(win);
    flash();
    wrefresh(win);
    sleep(1);

    mvwaddstr(win, 2, 1,
	      "If you were still typing when the window timer expired,");
    mvwaddstr(win, 3, 1,
	      "or else you typed nothing at all while it was running,");
    mvwaddstr(win, 4, 1,
	      "test was invalid.  You'll see garbage or nothing at all. ");
    mvwaddstr(win, 6, 1, "Press a key");
    wmove(win, 9, 10);
    wrefresh(win);
    echo();
    wGetchar(win);
    flushinp();
    mvwaddstr(win, 12, 0,
	      "If you see any key other than what you typed, flushinp() is broken.");
    Continue(win);

    wmove(win, 9, 10);
    wdelch(win);
    wrefresh(win);
    wmove(win, 12, 0);
    clrtoeol();
    waddstr(win,
	    "What you typed should now have been deleted; if not, wdelch() failed.");
    Continue(win);

    cbreak();
}

/****************************************************************************
 *
 * Menu test
 *
 ****************************************************************************/

#if HAVE_MENU_H

#define MENU_Y	8
#define MENU_X	8

static int menu_virtualize(int c)
{
    if (c == '\n' || c == KEY_EXIT)
	return(MAX_COMMAND + 1);
    else if (c == 'n' || c == KEY_DOWN)
	return(REQ_NEXT_ITEM);
    else if (c == 'p' || c == KEY_UP)
	return(REQ_PREV_ITEM);
    else if (c == ' ')
	return(REQ_TOGGLE_ITEM);
    else
	return(c);
}

static char *animals[] =
{
    "Lions", "Tigers", "Bears", "(Oh my!)", "Newts", "Platypi", "Lemurs",
    (char *)NULL
};

static void menu_test(void)
{
    MENU	*m;
    ITEM	*items[SIZEOF(animals)];
    ITEM	**ip = items;
    char	**ap;
    int		mrows, mcols;
    WINDOW	*menuwin;

    mvaddstr(0, 0, "This is the menu test:");
    mvaddstr(2, 0, "  Use up and down arrow to move the select bar.");
    mvaddstr(3, 0, "  Press return to exit.");
    refresh();

    for (ap = animals; *ap; ap++)
	*ip++ = new_item(*ap, "");
    *ip = (ITEM *)NULL;

    m = new_menu(items);

    scale_menu(m, &mrows, &mcols);

    menuwin = newwin(mrows + 2, mcols +  2, MENU_Y, MENU_X);
    set_menu_win(m, menuwin);
    keypad(menuwin, TRUE);
    box(menuwin, 0, 0);

    set_menu_sub(m, derwin(menuwin, mrows, mcols, 1, 1));

    post_menu(m);

    while (menu_driver(m, menu_virtualize(wGetchar(menuwin))) != E_UNKNOWN_COMMAND)
	continue;

    (void) mvprintw(LINES - 2, 0,
		     "You chose: %s\n", item_name(current_item(m)));
    (void) addstr("Press any key to continue...");
    wGetchar(stdscr);

    unpost_menu(m);
    delwin(menuwin);

    for (ip = items; *ip; ip++)
	free_item(*ip);
    free_menu(m);
}

#ifdef TRACE
static char *levels[] =
{
    "TRACE_DISABLE",
    "TRACE_TPUTS",
    "TRACE_UPDATE",
    "TRACE_MOVE",
    "TRACE_CHARPUT",
    "TRACE_ORDINARY",
    "TRACE_CALLS",
    "TRACE_VIRTPUT",
    "TRACE_IEVENT",
    "TRACE_BITS",
    "TRACE_MAXIMUM",
    (char *)NULL
};

static int masks[] =	/* must parallel the array above */
{
    TRACE_DISABLE,
    TRACE_TPUTS,
    TRACE_UPDATE,
    TRACE_MOVE,
    TRACE_CHARPUT,
    TRACE_ORDINARY,
    TRACE_CALLS,
    TRACE_VIRTPUT,
    TRACE_IEVENT,
    TRACE_BITS,
    TRACE_MAXIMUM,
};

static char *tracetrace(int tlevel)
{
    static char	*buf;
    char	**sp;

    if (buf == 0) {
	size_t need = 12;
	int n;
	for (n = 0; levels[n] != 0; n++)
	    need += strlen(levels[n]) + 2;
	buf = malloc(need);
    }
    sprintf(buf, "0x%02x = {", tlevel);
    if (tlevel == 0) {
	sprintf(buf, "%s, ", levels[0]);
    } else {
	for (sp = levels + 1; *sp; sp++)
	    if ((tlevel & masks[sp - levels]) == masks[sp - levels])
	    {
		strcat(buf, *sp);
		strcat(buf, ", ");
	    }
    }
    if (buf[strlen(buf) - 2] == ',')
	buf[strlen(buf) - 2] = '\0';
    return(strcat(buf,"}"));
}

/* fake a dynamically reconfigurable menu using the 0th entry to deselect
 * the others
 */
static int run_trace_menu(MENU *m)
{
    ITEM **items;
    ITEM *i, **p;

    for (;;) {
	bool changed = FALSE;
	switch (menu_driver(m, menu_virtualize(wGetchar(menu_win(m))))) {
	case E_UNKNOWN_COMMAND:
	    return FALSE;
	default:
	    items = menu_items(m);
	    i = current_item(m);
	    if (i == items[0]) {
		if (item_value(i)) {
		    for (p = items+1; *p != 0; p++)
			if (item_value(*p)) {
			    set_item_value(*p, FALSE);
			    changed = TRUE;
			}
		}
	    } else {
		for (p = items+1; *p != 0; p++)
		    if (item_value(*p)) {
			set_item_value(items[0], FALSE);
			changed = TRUE;
			break;
		    }
	    }
	    if (!changed)
		return TRUE;
	}
    }
}

static void trace_set(void)
/* interactively set the trace level */
{
    MENU	*m;
    ITEM	*items[SIZEOF(levels)];
    ITEM	**ip = items;
    char	**ap;
    int		mrows, mcols, newtrace;
    WINDOW	*menuwin;

    mvaddstr(0, 0, "Interactively set trace level:");
    mvaddstr(2, 0, "  Press space bar to toggle a selection.");
    mvaddstr(3, 0, "  Use up and down arrow to move the select bar.");
    mvaddstr(4, 0, "  Press return to set the trace level.");
    mvprintw(6, 0, "(Current trace level is %s)", tracetrace(_nc_tracing));

    refresh();

    for (ap = levels; *ap; ap++)
	*ip++ = new_item(*ap, "");
    *ip = (ITEM *)NULL;

    m = new_menu(items);

    scale_menu(m, &mrows, &mcols);

    menu_opts_off(m, O_ONEVALUE);
    menuwin = newwin(mrows + 2, mcols +  2, MENU_Y, MENU_X);
    set_menu_win(m, menuwin);
    keypad(menuwin, TRUE);
    box(menuwin, 0, 0);

    set_menu_sub(m, derwin(menuwin, mrows, mcols, 1, 1));

    post_menu(m);

    for (ip = menu_items(m); *ip; ip++)
	if (masks[item_index(*ip)] == 0)
	    set_item_value(*ip, _nc_tracing == 0);
	else if ((masks[item_index(*ip)] & _nc_tracing) == masks[item_index(*ip)])
	    set_item_value(*ip, TRUE);

    while (run_trace_menu(m))
	continue;

    newtrace = 0;
    for (ip = menu_items(m); *ip; ip++)
	if (item_value(*ip))
	    newtrace |= masks[item_index(*ip)];
    trace(newtrace);
    _tracef("trace level interactively set to %s", tracetrace(_nc_tracing));

    (void) mvprintw(LINES - 2, 0,
		     "Trace level is %s\n", tracetrace(_nc_tracing));
    (void) addstr("Press any key to continue...");
    wGetchar(stdscr);

    unpost_menu(m);
    delwin(menuwin);

    for (ip = items; *ip; ip++)
	free_item(*ip);
    free_menu(m);
}
#endif /* TRACE */
#endif /* HAVE_MENU_H */

/****************************************************************************
 *
 * Forms test
 *
 ****************************************************************************/
#if HAVE_FORM_H
static FIELD *make_label(int frow, int fcol, char *label)
{
    FIELD	*f = new_field(1, strlen(label), frow, fcol, 0, 0);

    if (f)
    {
	set_field_buffer(f, 0, label);
	set_field_opts(f, field_opts(f) & ~O_ACTIVE);
    }
    return(f);
}

static FIELD *make_field(int frow, int fcol, int rows, int cols)
{
    FIELD	*f = new_field(rows, cols, frow, fcol, 0, 0);

    if (f)
	set_field_back(f, A_UNDERLINE);
    return(f);
}

static void display_form(FORM *f)
{
    WINDOW	*w;
    int rows, cols;

    scale_form(f, &rows, &cols);

    if ((w =newwin(rows+2, cols+4, 0, 0)) != (WINDOW *)NULL)
    {
	set_form_win(f, w);
	set_form_sub(f, derwin(w, rows, cols, 1, 2));
	box(w, 0, 0);
	keypad(w, TRUE);
    }

    if (post_form(f) != E_OK)
	wrefresh(w);
}

static void erase_form(FORM *f)
{
    WINDOW	*w = form_win(f);
    WINDOW	*s = form_sub(f);

    unpost_form(f);
    werase(w);
    wrefresh(w);
    delwin(s);
    delwin(w);
}

static int form_virtualize(WINDOW *w)
{
    static int	mode = REQ_INS_MODE;
    int		c = wGetchar(w);

    switch(c)
    {
    case QUIT:
    case ESCAPE:
	return(MAX_FORM_COMMAND + 1);

    /* demo doesn't use these three, leave them in anyway as sample code */
    case KEY_NPAGE:
    case CTRL('F'):
	return(REQ_NEXT_PAGE);
    case KEY_PPAGE:
	return(REQ_PREV_PAGE);

    case KEY_NEXT:
    case CTRL('N'):
	return(REQ_NEXT_FIELD);
    case KEY_PREVIOUS:
    case CTRL('P'):
	return(REQ_PREV_FIELD);

    case KEY_HOME:
	return(REQ_FIRST_FIELD);
    case KEY_END:
    case KEY_LL:
	return(REQ_LAST_FIELD);

    case CTRL('L'):
	return(REQ_LEFT_FIELD);
    case CTRL('R'):
	return(REQ_RIGHT_FIELD);
    case CTRL('U'):
	return(REQ_UP_FIELD);
    case CTRL('D'):
	return(REQ_DOWN_FIELD);

    case CTRL('W'):
	return(REQ_NEXT_WORD);
    case CTRL('B'):
	return(REQ_PREV_WORD);
    case CTRL('S'):
	return(REQ_BEG_FIELD);
    case CTRL('E'):
	return(REQ_END_FIELD);

    case KEY_LEFT:
	return(REQ_LEFT_CHAR);
    case KEY_RIGHT:
	return(REQ_RIGHT_CHAR);
    case KEY_UP:
	return(REQ_UP_CHAR);
    case KEY_DOWN:
	return(REQ_DOWN_CHAR);

    case CTRL('M'):
	return(REQ_NEW_LINE);
    case CTRL('I'):
	return(REQ_INS_CHAR);
    case CTRL('O'):
	return(REQ_INS_LINE);
    case CTRL('V'):
	return(REQ_DEL_CHAR);

    case CTRL('H'):
    case KEY_BACKSPACE:
	return(REQ_DEL_PREV);
    case CTRL('Y'):
	return(REQ_DEL_LINE);
    case CTRL('G'):
	return(REQ_DEL_WORD);

    case CTRL('C'):
	return(REQ_CLR_EOL);
    case CTRL('K'):
	return(REQ_CLR_EOF);
    case CTRL('X'):
	return(REQ_CLR_FIELD);
    case CTRL('A'):
	return(REQ_NEXT_CHOICE);
    case CTRL('Z'):
	return(REQ_PREV_CHOICE);

    case CTRL(']'):
	if (mode == REQ_INS_MODE)
	    return(mode = REQ_OVL_MODE);
	else
	    return(mode = REQ_INS_MODE);

    default:
	return(c);
    }
}

static int my_form_driver(FORM *form, int c)
{
    if (c == (MAX_FORM_COMMAND + 1)
		&& form_driver(form, REQ_VALIDATION) == E_OK)
	return(TRUE);
    else
    {
	beep();
	return(FALSE);
    }
}

static void demo_forms(void)
{
    WINDOW	*w;
    FORM	*form;
    FIELD	*f[10];
    int		finished = 0, c;

    mvaddstr(10, 57, "Forms Entry Test");

    move(18, 0);
    addstr("Defined form-traversal keys:   ^Q/ESC- exit form\n");
    addstr("^N   -- go to next field       ^P  -- go to previous field\n");
    addstr("Home -- go to first field      End -- go to last field\n");
    addstr("^L   -- go to field to left    ^R  -- go to field to right\n");
    addstr("^U   -- move upward to field   ^D  -- move downward to field\n");
    addstr("^W   -- go to next word        ^B  -- go to previous word\n");
    addstr("^S   -- go to start of field   ^E  -- go to end of field\n");
    addstr("^H   -- delete previous char   ^Y  -- delete line\n");
    addstr("^G   -- delete current word    ^C  -- clear to end of line\n");
    addstr("^K   -- clear to end of field  ^X  -- clear field\n");
    addstr("Arrow keys move within a field as you would expect.");
    refresh();

    /* describe the form */
    f[0] = make_label(0, 15, "Sample Form");
    f[1] = make_label(2, 0, "Last Name");
    f[2] = make_field(3, 0, 1, 18);
    f[3] = make_label(2, 20, "First Name");
    f[4] = make_field(3, 20, 1, 12);
    f[5] = make_label(2, 34, "Middle Name");
    f[6] = make_field(3, 34, 1, 12);
    f[7] = make_label(5, 0, "Comments");
    f[8] = make_field(6, 0, 4, 46);
    f[9] = (FIELD *)NULL;

    form = new_form(f);

    display_form(form);

    w = form_win(form);
    raw();
    while (!finished)
    {
	switch(form_driver(form, c = form_virtualize(w)))
	{
	case E_OK:
	    break;
	case E_UNKNOWN_COMMAND:
	    finished = my_form_driver(form, c);
	    break;
	default:
	    beep();
	    break;
	}
    }

    erase_form(form);

    free_form(form);
    noraw();
}
#endif	/* HAVE_FORM_H */

/****************************************************************************
 *
 * Overlap test
 *
 ****************************************************************************/

static void fillwin(WINDOW *win, char ch)
{
    int y, x;

    for (y = 0; y <= win->_maxy; y++)
    {
	wmove(win, y, 0);
	for (x = 0; x <= win->_maxx; x++)
	    waddch(win, ch);
    }
}

static void crosswin(WINDOW *win, char ch)
{
    int y, x;

    for (y = 0; y <= win->_maxy; y++)
    {
	for (x = 0; x <= win->_maxx; x++)
	    if (((x > win->_maxx / 3) && (x <= 2 * win->_maxx / 3))
			|| (((y > win->_maxy / 3) && (y <= 2 * win->_maxy / 3))))
	    {
		wmove(win, y, x);
		waddch(win, ch);
	    }
    }
}

static void overlap_test(void)
/* test effects of overlapping windows */
{
    int	ch;

    WINDOW *win1 = newwin(9, 20, 3, 3);
    WINDOW *win2 = newwin(9, 20, 9, 16);

    refresh();
    move(0, 0);
    printw("This test shows the behavior of wnoutrefresh() with respect to\n");
    printw("the shared region of two overlapping windows.  The cross pattern\n");
    printw("in each window does not overlap the other.\n");


    move(18, 0);
    printw("1 = refresh window A, then window B, then doupdaute.\n");
    printw("2 = refresh window B, then window A, then doupdaute.\n");
    printw("3 = fill window A with letter A.  4 = fill window B with letter B.\n");
    printw("5 = cross pattern in window A.    6 = cross pattern in window B.\n");
    printw("7 = clear window A.               8 = clear window B.\n");
    printw("^Q/ESC = terminate test.");

    while ((ch = Getchar()) != QUIT && ch != ESCAPE)
	switch (ch)
	{
	case '1':		/* refresh window A first, then B */
	    wnoutrefresh(win1);
	    wnoutrefresh(win2);
	    doupdate();
	    break;

	case '2':		/* refresh window B first, then A */
	    wnoutrefresh(win2);
	    wnoutrefresh(win1);
	    doupdate();
	    break;

	case '3':		/* fill window A so it's visible */
	    fillwin(win1, 'A');
	    break;

	case '4':		/* fill window B so it's visible */
	    fillwin(win2, 'B');
	    break;

	case '5':		/* cross test pattern in window A */
	    crosswin(win1, 'A');
	    break;

	case '6':		/* cross test pattern in window A */
	    crosswin(win2, 'B');
	    break;

	case '7':		/* clear window A */
	    wclear(win1);
	    wmove(win1, 0, 0);
	    break;

	case '8':		/* clear window B */
	    wclear(win2);
	    wmove(win2, 0, 0);
	    break;
	}

    erase();
    endwin();
}

/****************************************************************************
 *
 * Main sequence
 *
 ****************************************************************************/

static bool
do_single_test(const char c)
/* perform a single specified test */
{
    switch (c)
    {
    case 'a':
	getch_test();
	break;

    case 'b':
	attr_test();
	break;

    case 'c':
	if (!has_colors())
	    Cannot("does not support color.");
	else
	    color_test();
	break;

    case 'd':
	if (!has_colors())
	    Cannot("does not support color.");
	else if (!can_change_color())
	    Cannot("has hardwired color values.");
	else
	    color_edit();
	break;

    case 'e':
	slk_test();
	break;

    case 'f':
	acs_display();
	break;

#if HAVE_PANEL_H
    case 'o':
	demo_panels();
	break;
#endif

    case 'g':
	acs_and_scroll();
	break;

    case 'i':
	flushinp_test(stdscr);
	break;

    case 'k':
	test_sgr_attributes();
	break;

#if HAVE_MENU_H
    case 'm':
	menu_test();
	break;
#endif

#if HAVE_PANEL_H
    case 'p':
	demo_pad();
	break;
#endif

#if HAVE_FORM_H
    case 'r':
	demo_forms();
	break;
#endif

    case 's':
        overlap_test();
	break;

#if defined(HAVE_MENU_H) && defined(TRACE)
    case 't':
        trace_set();
	break;
#endif

    case '?':
	(void) puts("This is the ncurses capability tester.");
	(void) puts("You may select a test from the main menu by typing the");
	(void) puts("key letter of the choice (the letter to left of the =)");
	(void) puts("at the > prompt.  The commands `x' or `q' will exit.");
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

static void
usage(void)
{
    static const char *const tbl[] = {
	 "Usage: ncurses [options]"
	,""
	,"Options:"
	,"  -s msec  specify nominal time for panel-demo (default: 1, to hold)"
#ifdef TRACE
	,"  -t mask  specify default trace-level (may toggle with ^T)"
#endif
    };
    size_t n;
    for (n = 0; n < sizeof(tbl)/sizeof(tbl[0]); n++)
	fprintf(stderr, "%s\n", tbl[n]);
    exit(EXIT_FAILURE);
}

static void
set_terminal_modes(void)
{
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    idlok(stdscr, TRUE);
    keypad(stdscr, TRUE);
}

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    char	buf[BUFSIZ];
    int		c;

    while ((c = getopt(argc, argv, "s:t:")) != EOF) {
	switch (c) {
	case 's':
	    nap_msec = atol(optarg);
	    break;
#ifdef TRACE
	case 't':
	    save_trace = atoi(optarg);
	    break;
#endif
	default:
	    usage();
	}
    }

    /*
     * If there's no menus (unlikely for ncurses!), then we'll have to set
     * tracing on initially, just in case the user wants to test something that
     * doesn't involve wGetchar.
     */
#ifdef TRACE
    /* enable debugging */
#ifndef HAVE_MENU_H
    trace(save_trace);
#endif /* HAVE_MENU_H */
#endif /* TRACE */

    /* tell it we're going to play with soft keys */
    slk_init(1);

    /* we must initialize the curses data structure only once */
    initscr();
    bkgdset(BLANK);

    /* tests, in general, will want these modes */
    start_color();
    set_terminal_modes();
    def_prog_mode();

    /*
     * Return to terminal mode, so we're guaranteed of being able to
     * select terminal commands even if the capabilities are wrong.
     */
    endwin();

    (void) puts("Welcome to ncurses.  Press ? for help.");

    do {
	(void) puts("This is the ncurses main menu");
	(void) puts("a = keyboard and mouse input test");
	(void) puts("b = character attribute test");
	(void) puts("c = color test pattern");
	(void) puts("d = edit RGB color values");
	(void) puts("e = exercise soft keys");
	(void) puts("f = display ACS characters");
	(void) puts("g = display windows and scrolling");
	(void) puts("i = test of flushinp()");
	(void) puts("k = display character attributes");
#if HAVE_MENU_H
	(void) puts("m = menu code test");
#endif
#if HAVE_PANEL_H
	(void) puts("o = exercise panels library");
	(void) puts("p = exercise pad features");
	(void) puts("q = quit");
#endif
#if HAVE_FORM_H
	(void) puts("r = exercise forms code");
#endif
	(void) puts("s = overlapping-refresh test");
#if defined(HAVE_MENU_H) && defined(TRACE)
	(void) puts("t = set trace level");
#endif
	(void) puts("? = repeat this command summary");

	(void) fputs("> ", stdout);
	(void) fflush(stdout);		/* necessary under SVr4 curses */
	(void) fgets(buf, sizeof(buf), stdin);

	if (do_single_test(buf[0])) {
		/*
		 * This may be overkill; it's intended to reset everything back
		 * to the initial terminal modes so that tests don't get in
		 * each other's way.
		 */
		flushinp();
		set_terminal_modes();
    		reset_prog_mode();
		clear();
		refresh();
		endwin();
		continue;
	}
    } while
	(buf[0] != 'q');

    exit(0);
}

/* ncurses.c ends here */
