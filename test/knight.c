/*
 * Knight's Tour - a brain game
 *
 * The original of this game was anonymous.  It had an unbelievably bogus
 * interface, you actually had to enter square coordinates!  Redesign by
 * Eric S. Raymond <esr@snark.thyrsus.com> July 22 1995.
 */

#include <curses.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/* board size */
#define BDEPTH	8
#define BWIDTH	8

/* where to start the instructions */
#define INSTRY	2
#define INSTRX	36

/* corner of board */
#define BOARDY	2
#define BOARDX	0

/* notification line */
#define NOTIFYY	21

#define cellmove(y, x)	move (BOARDY+1 + 2*(y),       BOARDX+2 + 4 * (x));

static short	board[BDEPTH * BWIDTH];	/* the squares */
static lastrow, lastcol;		/* location of last move */
static int	rw,col;			/* current row and column */
static int	movecount;		/* count of moves so far */
static WINDOW	*helpwin;		/* the help window */

static void init(void);
static bool play(void);
static void dosquares(void);
static int  getrc(void);
static void putstars(int);
static bool evalmove(int);
static bool chkmoves(void);
static bool endgame(void);
static int  iabs(int);			/* <stdlib.h> declares 'abs()' */
static bool chksqr(int, int);

int main(int argc, char *argv[])
{
    init ();
    for (;;)  
	if (!play ())
	{
	    endwin ();
	    exit (0);
	}
}

static void init (void)
{
    srand ((unsigned)getpid());
    initscr ();
    cbreak ();			/* immediate char return */
    noecho ();			/* no immediate echo */
    keypad(stdscr, TRUE);
    helpwin = subwin(stdscr, 0, 0, INSTRY, INSTRX);
}

static void help1(void)
/* game explanation -- initial help screen */
{
    (void)waddstr(helpwin, "Knight's move is a solitaire puzzle.  Your\n");
    (void)waddstr(helpwin, "objective is to visit each square of the  \n");
    (void)waddstr(helpwin, "chessboard exactly once by making knight's\n");
    (void)waddstr(helpwin, "moves (one square right or left followed  \n");
    (void)waddstr(helpwin, "by two squares up or down, or two squares \n");
    (void)waddstr(helpwin, "right or left followed by one square up or\n");
    (void)waddstr(helpwin, "down).  You may start anywhere.\n\n");

    (void)waddstr(helpwin, "Use arrow keys to move the cursor around.\n");
    (void)waddstr(helpwin, "When you want to move your knight to the \n");
    (void)waddstr(helpwin, "cursor location, press <space> or Enter.\n");
    (void)waddstr(helpwin, "Illegal moves will be rejected with an  \n");
    (void)waddstr(helpwin, "audible beep.\n\n");
    (void)waddstr(helpwin, "The program will detect if you solve the\n");
    (void)waddstr(helpwin, "puzzle; also inform you when you run out\n");
    (void)waddstr(helpwin, "of legal moves.\n\n");

    (void)mvwaddstr(helpwin, NOTIFYY-INSTRY, 0,
		    "Press `?' to go to keystroke help."); 
}

static void help2(void)
/* keystroke help screen */
{
    (void)waddstr(helpwin, "Possible moves are shown with `-'.\n\n");

    (void)waddstr(helpwin, "You can move around with the arrow keys,:\n");
    (void)waddstr(helpwin, "or with the rogue/hack movement keys, or:\n");
    (void)waddstr(helpwin, "with your keypad digit keys, as follows:\n\n");

    (void)waddstr(helpwin, "             y k u    7 8 9\n");
    (void)waddstr(helpwin, "              \\|/      \\|/ \n");
    (void)waddstr(helpwin, "             h-+-l    4-+-6\n");
    (void)waddstr(helpwin, "              /|\\      /|\\ \n");
    (void)waddstr(helpwin, "             b j n    1 2 3\n");

    (void)waddstr(helpwin,"\nYou can place your knight on the selected\n");
    (void)waddstr(helpwin, "square with spacebar, Enter, or the keypad\n");
    (void)waddstr(helpwin, "center key.\n\n");

    (void)waddstr(helpwin, "You can quit with `x' or `q'.\n");

    (void)mvwaddstr(helpwin, NOTIFYY-INSTRY, 0,
		    "Press `?' to go to game explanation"); 
}

static bool play (void)
/* play the game */
{
    int i, j;

    /* clear screen and draw board */
    erase ();
    dosquares ();
    refresh ();
    help1();

    for (j = 0; j < (BWIDTH * BDEPTH); j++)
	board[j] = 0;
    for (i = 0; i < BDEPTH; i++)
	for (j = 0; j < BWIDTH; j++)
	{
	    cellmove(i, j);
	    addch('-');
	}
    rw = col = 0;
    for (movecount = 0;;)
    {
	j = getrc();
	if (j == KEY_EXIT)
	    return(FALSE);
	if (evalmove(j))
	{
	    putstars (j);
	    if (!chkmoves()) 
		return (endgame ());
	}
	else beep();
    }
}

static void dosquares (void)
{
    int i, j;

    mvaddstr(0, 20, "KNIGHT'S MOVE -- a logical solitaire");

    move(BOARDY,BOARDX);
    addch(ACS_ULCORNER);
    for (j = 0; j < 7; j++)
    {
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_TTEE);
    }
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_URCORNER);

    for (i = 1; i < BDEPTH; i++)
    {
	move(BOARDY + i * 2 - 1, BOARDX);
	addch(ACS_VLINE); 
	for (j = 0; j < BWIDTH; j++)
	{
	    addch(' ');
	    addch(' ');
	    addch(' ');
	    addch(ACS_VLINE);
	}
	move(BOARDY + i * 2, BOARDX);
	addch(ACS_LTEE); 
	for (j = 0; j < BWIDTH - 1; j++)
	{
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    addch(ACS_PLUS);
	}
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_RTEE);
    }

    move(BOARDY + i * 2 - 1, BOARDX);
    addch(ACS_VLINE);
    for (j = 0; j < BWIDTH; j++)
    {
	addch(' ');
	addch(' ');
	addch(' ');
	addch(ACS_VLINE);
    }

    move(BOARDY + i * 2, BOARDX);
    addch(ACS_LLCORNER);
    for (j = 0; j < BWIDTH - 1; j++)
    {
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_BTEE);
    }
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_LRCORNER);
}

static void mark_possibles(int prow, int pcol, chtype mark)
{
    if (chksqr(prow+2,pcol+1))  {cellmove(prow+2, pcol+1); addch(mark);};
    if (chksqr(prow+2,pcol-1))  {cellmove(prow+2, pcol-1); addch(mark);};
    if (chksqr(prow-2,pcol+1))  {cellmove(prow-2, pcol+1); addch(mark);};
    if (chksqr(prow-2,pcol-1))  {cellmove(prow-2, pcol-1); addch(mark);};
    if (chksqr(prow+1,pcol+2))  {cellmove(prow+1, pcol+2); addch(mark);};
    if (chksqr(prow+1,pcol-2))  {cellmove(prow+1, pcol-2); addch(mark);};
    if (chksqr(prow-1,pcol+2))  {cellmove(prow-1, pcol+2); addch(mark);};
    if (chksqr(prow-1,pcol-2))  {cellmove(prow-1, pcol-2); addch(mark);};
}

static int getrc (void)
/* interactively select a square to move the knight to */
{
    static int		curow,curcol;   /* current row and column integers */
    static bool		keyhelp;	/* TRUE if keystroke help is up */
    static chtype	oldch = '-';
    int	c, ny = 0, nx = 0;

    for (;;)
    {
	if (rw != curow || col != curcol)
	{
	    cellmove(curow, curcol);
	    if (board[curow*BWIDTH + curcol])
		addch('#');
	    else
		addch(oldch);

	    cellmove(rw, col);
	    oldch = inch();

	    curow = rw;
	    curcol= col;
	}
	cellmove(rw, col);
	addch('+');
	cellmove(rw, col);

	switch (c = getch())
	{
	case 'k': case '8':
	case KEY_UP:
	    ny = rw+BDEPTH-1; nx = col;
	    break;
	case 'j': case '2':
	case KEY_DOWN:
	    ny = rw+1;        nx = col;
	    break;
	case 'h': case '4':
	case KEY_LEFT:
	    ny = rw;          nx = col+BWIDTH-1;
	    break;
	case 'l': case '6':
	case KEY_RIGHT:
	    ny = rw;          nx = col+1;
	    break;
	case 'y': case '7':
	case KEY_A1:
	    ny = rw+BDEPTH-1; nx = col+BWIDTH-1;
	    break;
	case 'b': case '1':
	case KEY_C1:
	    ny = rw+1;        nx = col+BWIDTH-1;
	    break;
	case 'u': case '9':
	case KEY_A3:
	    ny = rw+BDEPTH-1; nx = col+1;
	    break;
	case 'n': case '3':
	case KEY_C3:
	    ny = rw+1;        nx = col+1;
	    break;

	case KEY_B2:
	case '\n':
	case ' ':
	    return((BWIDTH * rw) + col);

	case 'q':
	case 'x':
	    return(KEY_EXIT);

	case '?':
	    werase(helpwin);
	    if (keyhelp)
	    {
		help1();
		keyhelp = FALSE;
	    }
	    else
	    {
		help2();
		keyhelp = TRUE;
	    }
	    wrefresh(helpwin);
	    break;

	default:
	    beep();
	    break;
	}

	col = nx % BWIDTH;
	rw = ny % BDEPTH;
    }
}

static void putstars (int loc)
/* place the stars, update board & currents */
{
    if (movecount == 0)
    {
	int i, j;

	for (i = 0; i < BDEPTH; i++)
	    for (j = 0; j < BWIDTH; j++)
	    {
		cellmove(i, j);
		if (inch() == '-')
		    addch(' ');
	    }
    }
    else
    {
	cellmove(lastrow, lastcol);
	addch('\b'); addstr("###");
	mark_possibles(lastrow, lastcol, ' ');
    }    

    cellmove(rw, col);
    addch('\b'); addstr("###");
    mark_possibles(rw, col, '-');
    board[loc] = 1;
    mvprintw(NOTIFYY,0, "Move %d", movecount+1);

    lastrow = rw;
    lastcol = col;

    movecount++;
}

static bool evalmove(int j)
/* evaluate move */
{
    int	rdif, cdif;		/* difference between input and current */

    if (movecount == 0)
	return(TRUE);

    rdif = iabs(rw  - lastrow);
    cdif = iabs(col - lastcol);
    if (((rdif == 1) && (cdif == 2)) || ((rdif == 2) && (cdif == 1)))
	if (board [j] == 0)
	    return(TRUE);
    return(FALSE);
}

static bool chkmoves (void)
/* check to see if valid moves are available */
{
    if (chksqr(rw+2,col+1)) return(TRUE);
    if (chksqr(rw+2,col-1)) return(TRUE);
    if (chksqr(rw-2,col+1)) return(TRUE);
    if (chksqr(rw-2,col-1)) return(TRUE);
    if (chksqr(rw+1,col+2)) return(TRUE);
    if (chksqr(rw+1,col-2)) return(TRUE);
    if (chksqr(rw-1,col+2)) return(TRUE);
    if (chksqr(rw-1,col-2)) return(TRUE);
    return (FALSE);
}

static bool endgame (void)
/* check for filled board or not */
{
    int j;

    rw = 0;
    for (j = 0; j < (BWIDTH * BDEPTH); j++)
	if (board[j] != 0)
	    rw += 1;
    if (rw == (BWIDTH * BDEPTH))
	mvaddstr (NOTIFYY, 0, "Congratulations!! You got 'em all.");
    else
	mvprintw (NOTIFYY, 0, "%2d squares are filled", rw);
    mvaddstr (NOTIFYY+1, 0, "Play again? (y/n) ");
    if (tolower(getch()) == 'y')
	return(TRUE);
    else
	return (FALSE);
}

static int iabs(int num)
{
	if (num < 0) return (-num);
		else return (num);
}

static bool chksqr (int r1, int c1)
{
    if ((r1 < 0) || (r1 > BDEPTH - 1))
	return(FALSE);
    if ((c1 < 0) || (c1 > BWIDTH - 1))
	return(FALSE);
    return (!board[r1*BWIDTH+c1]);
}

/* knight.c ends here */

