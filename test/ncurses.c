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

***************************************************************************/
/*LINTLIBRARY */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <curses.h>
#include <panel.h>
#include <menu.h>

#define P(string)	printw("%s\n", string)
#ifndef CTRL
#define CTRL(x)		((x) & 0x1f)
#endif

static void Pause(void)
{
	move(LINES - 1, 0);
	addstr("Press any key to continue... ");
	(void) getch();
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
  
     (void) printw("Delay in 10ths of a second (<CR> for blocking input)? ");
     echo();
     getstr(buf);
     noecho();
 
     if (isdigit(buf[0]))
     {
 	timeout(atoi(buf) * 100);
 	blocking = FALSE;
     }
 
     c = '?';
     for (;;)
     {
	if (firsttime++)
	{
	    printw("Key pressed: %04o ", c);
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

	while ((c = getch()) == ERR)
	    if (!blocking)
		(void) printw("%05d: input timed out\n", incount++);
    }

    timeout(-1);
    erase();
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
		     (i < sizeof(colors)/sizeof(colors[0]) ? colors[i] : ""));
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

	switch (c = getch())
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
		c = getch();
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
	    while ((c = getch()) != 'Q' && (c != ERR))
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
	    break;

	case 'x':
	case 'q':
	    goto done;

	default:
	    beep();
	}
    } while
	((c = getch()) != EOF);

 done:
    erase();
    endwin();
}

/****************************************************************************
 *
 * Alternate character-set stuff
 *
 ****************************************************************************/

static void acs_display(void)
/* display the ACS character set */
{
    int	i, j;

    erase();
    attron(A_BOLD);
    mvaddstr(0, 20, "Display of the ACS Character Set");
    attroff(A_BOLD);
    refresh();

#define ACSY	2
    mvaddstr(ACSY + 0, 0, "ACS_ULCORNER: "); addch(ACS_ULCORNER);
    mvaddstr(ACSY + 1, 0, "ACS_LLCORNER: "); addch(ACS_LLCORNER);
    mvaddstr(ACSY + 2, 0, "ACS_URCORNER: "); addch(ACS_URCORNER);
    mvaddstr(ACSY + 3, 0, "ACS_LRCORNER: "); addch(ACS_LRCORNER);
    mvaddstr(ACSY + 4, 0, "ACS_RTEE: "); addch(ACS_RTEE);
    mvaddstr(ACSY + 5, 0, "ACS_LTEE: "); addch(ACS_LTEE);
    mvaddstr(ACSY + 6, 0, "ACS_BTEE: "); addch(ACS_BTEE);
    mvaddstr(ACSY + 7, 0, "ACS_TTEE: "); addch(ACS_TTEE);
    mvaddstr(ACSY + 8, 0, "ACS_HLINE: "); addch(ACS_HLINE);
    mvaddstr(ACSY + 9, 0, "ACS_VLINE: "); addch(ACS_VLINE);
    mvaddstr(ACSY + 10,0, "ACS_PLUS: "); addch(ACS_PLUS);
    mvaddstr(ACSY + 11,0, "ACS_S1: "); addch(ACS_S1);
    mvaddstr(ACSY + 12,0, "ACS_S9: "); addch(ACS_S9);

    mvaddstr(ACSY + 0, 40, "ACS_DIAMOND: "); addch(ACS_DIAMOND);
    mvaddstr(ACSY + 1, 40, "ACS_CKBOARD: "); addch(ACS_CKBOARD);
    mvaddstr(ACSY + 2, 40, "ACS_DEGREE: "); addch(ACS_DEGREE);
    mvaddstr(ACSY + 3, 40, "ACS_PLMINUS: "); addch(ACS_PLMINUS);
    mvaddstr(ACSY + 4, 40, "ACS_BULLET: "); addch(ACS_BULLET);
    mvaddstr(ACSY + 5, 40, "ACS_LARROW: "); addch(ACS_LARROW);
    mvaddstr(ACSY + 6, 40, "ACS_RARROW: "); addch(ACS_RARROW);
    mvaddstr(ACSY + 7, 40, "ACS_DARROW: "); addch(ACS_DARROW);
    mvaddstr(ACSY + 8, 40, "ACS_UARROW: "); addch(ACS_UARROW);
    mvaddstr(ACSY + 9, 40, "ACS_BOARD: "); addch(ACS_BOARD);
    mvaddstr(ACSY + 10,40, "ACS_LANTERN: "); addch(ACS_LANTERN);
    mvaddstr(ACSY + 11,40, "ACS_BLOCK: "); addch(ACS_BLOCK);

#define HYBASE 	(ACSY + 13)    
    mvprintw(HYBASE + 1, 0, "High-half characters via echochar:\n");
    for (i = 0; i < 4; i++)
    {
	move(HYBASE + i + 3, 24);
	for (j = 0; j < 32; j++)
	    echochar((chtype)(128 + 32 * i + j));
    }

    Pause();

    erase();
    endwin();
}

/****************************************************************************
 *
 * Windows and scrolling tester.
 *
 ****************************************************************************/

#define BOTLINES	3	/* number of line stolen from screen bottom */

typedef struct
{
    int y, x;
}
pair;

static void report(void)
/* report on the cursor's current position, then restore it */
{
    int y, x;

    getyx(stdscr, y, x);
    move(LINES - 1, COLS - 17);
    printw("Y = %2d X = %2d", y, x);
    move(y, x);
}

static pair *selectcell(int uli, int ulj, int lri, int lrj)
/* arrows keys move cursor, return location at current on non-arrow key */
{
    static pair	res;			/* result cell */
    int		si = lri - uli + 1;	/* depth of the select area */
    int		sj = lrj - ulj + 1;	/* width of the select area */
    int		i = 0, j = 0;		/* offsets into the select area */

    for (;;)
    {
	move(LINES - 1, COLS - 17);
	printw("Y = %2d X = %2d", uli + i, ulj + j);
	move(uli + i, ulj + j);

	switch(getch())
	{
	case KEY_UP:	i += si - 1; break;
	case KEY_DOWN:	i++; break;
	case KEY_LEFT:	j += sj - 1; break;
	case KEY_RIGHT:	j++; break;
	case '\004':	return((pair *)NULL);
	default:	res.y = uli + i; res.x = ulj + j; return(&res);
	}
	i %= si;
	j %= sj;
    }
}

static WINDOW *getwindow(void)
/* Ask user for a window definition */
{
    WINDOW	*rwindow;
    pair	ul, lr, *tmp;

    move(0, 0); clrtoeol();
    addstr("Use arrows to move cursor, anything else to mark corner 1");
    refresh();
    if ((tmp = selectcell(1,    0,    LINES-BOTLINES, COLS-1)) == (pair *)NULL)
	return((WINDOW *)NULL);
    memcpy(&ul, tmp, sizeof(pair));
    mvaddch(ul.y-1, ul.x-1, ACS_ULCORNER);
    move(0, 0); clrtoeol();
    addstr("Use arrows to move cursor, anything else to mark corner 2");
    refresh();
    if ((tmp = selectcell(ul.y, ul.x, LINES-BOTLINES, COLS-1)) == (pair *)NULL)
	return((WINDOW *)NULL);
    memcpy(&lr, tmp, sizeof(pair));

    rwindow = subwin(stdscr, lr.y - ul.y + 1, lr.x - ul.x + 1, ul.y, ul.x);

    mvaddch(ul.y-1, lr.x+1, ACS_URCORNER);
    mvaddch(lr.y+1, lr.x+1, ACS_LRCORNER);
    mvaddch(lr.y+1, ul.x-1, ACS_LLCORNER);
    move(ul.y-1, ul.x);   hline(ACS_HLINE, lr.x - ul.x + 1);
    move(ul.y,   ul.x-1); vline(ACS_VLINE, lr.y - ul.y + 1);
    move(lr.y+1, ul.x);   hline(ACS_HLINE, lr.x - ul.x + 1);
    move(ul.y,   lr.x+1); vline(ACS_VLINE, lr.y - ul.y + 1);
    refresh();

    scrollok(rwindow, TRUE);
/*    immedok(rwindow);	*/
    wrefresh(rwindow);

    return(rwindow);
}

static void transient(char *msg)
{
    if (msg)
    {
	mvaddstr(LINES - 1, 0, msg);
	refresh();
	sleep(1);
    }

    mvaddstr(LINES - 1, 0,
	     "All other characters are echoed, windows should scroll.");
    refresh();
}

static void acs_and_scroll(void)
/* Demonstrate windows */
{
    int	c;
    FILE *fp;
    struct frame
    {
        struct frame	*next, *last;
        WINDOW		*wind;
    }
    *current = (struct frame *)NULL, *neww;

#define DUMPFILE	"screendump"

    refresh();
    mvaddstr(LINES - 4, 0,
	     "F1 = make new window, F2 = next window, F3 = previous window, ");
    mvaddstr(LINES - 3, 0,
	     "F4 = scroll current window forward, F5 = scroll current window backward");
    mvaddstr(LINES - 2, 0,
	     "F6 = save window to file, F7 = restore window, Ctrl-D = exit");
    transient((char *)NULL);

    c = KEY_F(1);
    do {
	report();
	if (current)
	    wrefresh(current->wind);

	switch(c)
	{
	case KEY_F(1):
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
	    break;

	case KEY_F(2):		/* go to next window */
	    current = current->next;
	    break;

	case KEY_F(3):		/* go to previous window */
	    current = current->last;
	    break;

	case KEY_F(4):		/* scroll current window forward */
	    if (current)
		wscrl(current->wind, 1);
	    break;

	case KEY_F(5):		/* scroll current window backwards */
	    if (current)
		wscrl(current->wind, -1);
	    break;

	case KEY_F(6):		/* save window */
	    if ((fp = fopen(DUMPFILE, "w")) == (FILE *)NULL)
		transient("Can't open screen dump file");
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

	case KEY_F(7):		/* restore window */
	    if ((fp = fopen(DUMPFILE, "r")) == (FILE *)NULL)
		transient("Can't open screen dump file");
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

	case KEY_F(10):	/* undocumented --- use this to test area clears */
	    selectcell(0, 0, LINES - 1, COLS - 1);
	    clrtobot();
	    refresh();
	    break;

	case '\r':
	    c = '\n';
	    /* FALLTHROUGH */

	default:
	    waddch(current->wind, (chtype)c);
	    break;
	}
	report();
	wrefresh(current->wind);
    } while
	((c = wgetch(current->wind)) != '\004'
	 && (c != ERR));

 breakout:
    erase();
    endwin();
}

/****************************************************************************
 *
 * Panels tester
 *
 ****************************************************************************/

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

#define	nap(x)		usleep(1000*x)

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
wait_a_while(unsigned long msec)
{
#ifdef NONAP
	if(nap_msec == 1)
		getchar();
	else if(msec > 1000L)
		sleep((int)msec/1000L);
	else
		sleep(1);
#else
	if(nap_msec == 1)
		getchar();
	else
		nap(nap_msec);
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
WINDOW *win = pan->win;
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
WINDOW *win = pan->win;
chtype num = *(pan->user + 1);
int y,x;

	box(win, 0, 0);  
	wmove(win,1,1);
	wprintw(win,"-pan%c-",num);
	for(y = 2; y < getmaxy(win) - 1; y++)
	{
		for(x = 1; x < getmaxx(win) - 1; x++)
		{
			wmove(win,y,x);
			waddch(win,num);
		}
	}
}	/* end of fill_panel */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/

static void demo_panels(void)
{
int itmp;
register y,x;

#ifdef FOO
	if((argc > 1) && atol(argv[1]))
		nap_msec = atol(argv[1]);
#endif /* FOO */

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

#define GRIDSIZE	5

static void panner(WINDOW *pad, int iy, int ix, int (*pgetc)(WINDOW *))
{
    static int porty, portx, basex = 0, basey = 0;
    int pxmax, pymax, lowend, highend, i, j, c;

    porty = iy; portx = ix;

    getmaxyx(pad, pymax, pxmax);

    c = KEY_REFRESH;
    do {
	switch(c)
	{
	case KEY_REFRESH:
	    /* do nothing */
	    break;

	case KEY_IC:
	    if (portx >= pxmax || portx >= ix)
		beep();
	    else
		++portx;
	    break;

	case KEY_IL:
	    if (porty >= pymax || porty >= iy)
		beep();
	    else
		++porty;
	    break;

	case KEY_DC:
	    if (portx <= 0)
		beep();
	    else
	    {
		--portx;
		for (i = 0; i < porty; i++)
		    mvaddch(i, portx, ' ');
	    }
	    break;

	case KEY_DL:
	    if (porty <= 0)
		beep();
	    else
	    {
		--porty;
		for (j = 0; j < portx; j++)
		    mvaddch(porty, j, ' ');
	    }
	    break;

	case KEY_LEFT:
	    if (basex > 0)
		basex--;
	    else
		beep();
	    break;

	case KEY_RIGHT:
	    if (basex + portx - (pymax > porty) < pxmax)
		basex++;
	    else
		beep();
	    break;

	case KEY_UP:
	    if (basey > 0)
		basey--;
	    else
		beep();
	    break;

	case KEY_DOWN:
	    if (basey + porty - (pxmax > portx) < pymax)
		basey++;
	    else
		beep();
	    break;
	}

	if (pxmax > portx - 1) {

	    lowend = basex * ((float)(portx - 1) / (float)pxmax);
	    highend = (basex + portx - 1) * ((float)(portx - 1) / (float)pxmax);

	    for (j = 0; j < lowend; j++)
		mvaddch(porty - 1, j, ACS_HLINE);
	    attron(A_REVERSE);
	    for (j = lowend; j <= highend; j++)
		mvaddch(porty - 1, j, ' ');
	    attroff(A_REVERSE);
	    for (j = highend + 1; j < portx; j++)
		mvaddch(porty - 1, j, ACS_HLINE);
        }
	if (pymax > porty - 1) {

	    lowend = basey * ((float)(porty - 1) / (float)pymax);
	    highend = (basey + porty - 1) * ((float)(porty - 1) / (float)pymax);

	    for (i = 0; i < lowend; i++)
		mvaddch(i, portx - 1, ACS_VLINE);
	    attron(A_REVERSE);
	    for (i = lowend; i <= highend; i++)
		mvaddch(i, portx - 1, ' ');
	    attroff(A_REVERSE);
	    for (i = highend + 1; i < porty; i++)
		mvaddch(i, portx - 1, ACS_VLINE);
        }

	mvaddch(porty - 1, portx - 1, ACS_LRCORNER);
	wnoutrefresh(stdscr);

	prefresh(pad,
		 basey, basex,
		 0, 0,
		 porty - (pxmax > portx) - 1,
		 portx - (pymax > porty) - 1); 

	doupdate();

    } while
	((c = pgetc(pad)) != KEY_EXIT);
}

static
int padgetch(WINDOW *win)
{
    int	c;

    switch(c = wgetch(win))
    {
    case 'u': return(KEY_UP);
    case 'd': return(KEY_DOWN);
    case 'r': return(KEY_RIGHT);
    case 'l': return(KEY_LEFT);
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
    mvprintw(LINES - 3, 0, "Use arrow keys to pan over the test pattern");
    mvprintw(LINES - 2, 0, "Use +,- to grow/shrink the panner vertically.");
    mvprintw(LINES - 1, 0, "Use <,> to grow/shrink the panner horizontally.");

    keypad(panpad, TRUE);
    panner(panpad, LINES - 4, COLS, padgetch);

    endwin();
    erase();
}

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
    wgetch(win);
}

static void input_test(WINDOW *win)
/* Input test, adapted from John Burnell's PDCurses tester */
{
    int w, h, bx, by, sw, sh, i;

    WINDOW *subWin;
    wclear (win);

#ifdef FOO
    char buffer [80];
    int num;
#endif /* FOO */

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
#ifdef FOO
    mvwaddstr(subWin, 2, 1, "This is a subwindow");
#endif /* FOO */
    wrefresh(win);

    nocbreak();
    mvwaddstr(win, 1, 1, "Type random keys for 5 seconds.");
    mvwaddstr(win, 2, 1,
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

    mvwaddstr(win, 2, 1, "Press a key");
    wmove(win, 9, 10);
    wrefresh(win);
    echo();
    wgetch(win);
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

#ifdef FOO
    /*
     * This test won't be portable until vsscanf() is
     */
    mvwaddstr(win, 6, 2, "Enter a number then a string separated by space");
    echo();
    mvwscanw(win, 7, 6, "%d %s", &num,buffer);
    mvwprintw(win, 8, 6, "String: %s Number: %d", buffer,num);
#endif /* FOO */

    Continue(win);
}

/****************************************************************************
 *
 * Menu test
 *
 ****************************************************************************/

#define MENU_Y	4
#define MENU_X	4

static int input_virtualize(int c)
{
    if (c == '\n' || c == KEY_EXIT)
	return(MAX_COMMAND + 1);
    else if (c == 'n' || c == KEY_DOWN)
	return(REQ_NEXT_ITEM);
    else if (c == 'p' || c == KEY_UP)
	return(REQ_PREV_ITEM);
    else
	return(c);
}

static char *animals[] =
{
    "Lions", "Tigers", "Bears", "(Oh my!)", "Newts", "Platypi", "Lemurs",
    (char *)NULL
};
static ITEM *items[sizeof(animals)/sizeof(char *)];

static void menu_test(void)
{
    MENU	*m;
    ITEM	**ip = items;
    char	**ap;
    int		mrows, mcols;
    WINDOW	*menuwin;

    mvaddstr(LINES - 3, 0, "This is the menu test.");

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

    while (menu_driver(m, input_virtualize(wgetch(menuwin))) != E_UNKNOWN_COMMAND)
	continue;

    (void) mvprintw(LINES - 2, 0,
		     "You chose: %s\n", item_name(current_item(m)));
    (void) addstr("Press any key to continue...");
    wgetch(stdscr);

    unpost_menu(m);
    delwin(menuwin);

    for (ip = items; *ip; ip++)
	free_item(*ip);
    free_menu(m);
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
	return(TRUE);

    case 'b':
	attr_test();
	return(TRUE);

    case 'c':
	if (!has_colors())
	    Cannot("does not support color.");
	else
	    color_test();
	return(TRUE);

    case 'd':
	if (!has_colors())
	    Cannot("does not support color.");
	else if (!can_change_color())
	    Cannot("has hardwired color values.");
	else
	    color_edit();
	return(TRUE);

    case 'e':
	slk_test();
	return(TRUE);

    case 'f':
	acs_display();
	return(TRUE);

    case 'o':
	demo_panels();
	return(TRUE);

    case 'g':
	acs_and_scroll();
	return(TRUE);

    case 'm':
	menu_test();
	return(TRUE);

    case 'p':
	demo_pad();
	return(TRUE);

    case 'i':
	input_test(stdscr);
	return(TRUE);

    case '?':
	(void) puts("This is the ncurses capability tester.");
	(void) puts("You may select a test from the main menu by typing the");
	(void) puts("key letter of the choice (the letter to left of the =)");
	(void) puts("at the > prompt.  The commands `x' or `q' will exit.");
	return(TRUE);
    }

    return(FALSE);
}

int main(const int argc, const char *argv[])
{
    char	buf[BUFSIZ];

    /* enable debugging */
    trace(TRACE_ORDINARY);

    /* tell it we're going to play with soft keys */
    slk_init(1);

    /* we must initialize the curses data structure only once */
    initscr();

    /* tests, in general, will want these modes */
    start_color();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);

    /*
     * Return to terminal mode, so we're guaranteed of being able to
     * select terminal commands even if the capabilities are wrong.
     */
    endwin();

    (void) puts("Welcome to ncurses.  Press ? for help.");

    do {
	(void) puts("This is the ncurses main menu");
	(void) puts("a = character input test");
	(void) puts("b = character attribute test");
	(void) puts("c = color test pattern");
	(void) puts("d = edit RGB color values");
	(void) puts("e = exercise soft keys");
	(void) puts("f = display ACS characters");
	(void) puts("g = display windows and scrolling");
	(void) puts("m = menu code test");
	(void) puts("o = exercise panels library");
	(void) puts("p = exercise pad features");
	(void) puts("i = subwindow input test");
	(void) puts("? = repeat this command summary");

	(void) fputs("> ", stdout);
	(void) fflush(stdout);		/* necessary under SVr4 curses */
	(void) fgets(buf, BUFSIZ, stdin);

	if (do_single_test(buf[0])) {
		clear();
		refresh();
		endwin();
	    continue;
	}
    } while
	(buf[0] != 'q' && buf[0] != 'x');

    exit(0);
}

/* ncurses.c ends here */
