/*
 * Test lower-right-hand corner access
 *
 * by Eric S. Raymond <esr@thyrsus.com>
 *
 * This can't be part of ncurses, because ncurses rips off the bottom line
 * to do labels.
 */
#include <curses.h>

int
main(int argc, char *argv[])
{
    initscr();

    move(LINES/2-1, 4);
    addstr("This is a test of access to the lower right corner.\n");
    move(LINES/2, 4);
    addstr("If the top of the box is missing, the test failed.\n");
    move(LINES/2+1, 4);
    addstr("Please report this (along with a copy of your terminfo entry).\n");
    move(LINES/2+2, 4);
    addstr("to the ncurses maintainers, at ncurses-list@netcom.com.\n");

    box(stdscr, 0, 0);
    move(LINES-1, COLS-1);

    refresh();

    getch();
    endwin();
    return 0;
}

/* lrtest.c ends here */
