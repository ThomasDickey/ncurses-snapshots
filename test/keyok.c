/*
 * $Id: keyok.c,v 1.1 2002/11/23 23:52:31 tom Exp $
 *
 * Demonstrate the keyok() function.
 * Thomas Dickey - 2002/11/23
 */

#include <test.priv.h>

#if defined(NCURSES_VERSION)
int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    int lastch = ERR;
    int ch;
    WINDOW *win;

    initscr();
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    printw("Typing any function key will disable it, but typing it twice in\n");
    printw("a row will turn it back on (just for a demo).");
    refresh();

    win = newwin(LINES - 2, COLS, 2, 0);
    scrollok(win, TRUE);
    keypad(win, TRUE);
    wmove(win, 0, 0);

    while ((ch = wgetch(win)) != ERR) {
	char *name = keyname(ch);
	wprintw(win, "Keycode %d, name %s\n",
		ch,
		name != 0 ? name : "<null>");
	wclrtoeol(win);
	wrefresh(win);
	if (ch >= KEY_MIN) {
	    keyok(ch, FALSE);
	    lastch = ch;
	} else if (lastch >= KEY_MIN) {
	    keyok(lastch, TRUE);
	}
    }
    endwin();
    return EXIT_SUCCESS;
}
#else
int
main(void)
{
    printf("This program requires the ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif
