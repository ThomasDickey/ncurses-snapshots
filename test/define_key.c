/*
 * $Id: define_key.c,v 1.1 2002/11/24 00:49:38 tom Exp $
 *
 * Demonstrate the define_key() function.
 * Thomas Dickey - 2002/11/23
 */

#include <test.priv.h>

#if defined(NCURSES_VERSION)

#include <term.h>

static void
duplicate(char *name, int code)
{
    char *value = tigetstr(name);

    if (value != 0) {
	char *prefix = 0;
	char temp[BUFSIZ];

	if (!strncmp(value, "\033[", 2)) {
	    prefix = "\033O";
	} else if (!strncmp(value, "\033O", 2)) {
	    prefix = "\033[";
	}
	if (prefix != 0) {
	    sprintf(temp, "%s%s", prefix, value + 2);
	    define_key(temp, code);
	}
    }
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    char *fkeys[12];
    int n;
    int ch;
    WINDOW *win;

    initscr();
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    printw("This demo is best on xterm: it reverses the definitions for f1-f12,\n");
    printw("adds duplicate definitions for cursor application and normal modes,");
    printw("and removes any definitions for the mini keypad.  Type any of those:");
    refresh();

    win = newwin(LINES - 2, COLS, 2, 0);
    scrollok(win, TRUE);
    keypad(win, TRUE);
    wmove(win, 0, 0);

    /* we do the define_key() calls after keypad(), since the first call to
     * keypad() initializes the corresponding data.
     */
    for (n = 0; n < 12; ++n) {
	char name[10];
	sprintf(name, "kf%d", n + 1);
	fkeys[n] = tigetstr(name);
	define_key(0, KEY_F(n + 1));
    }
    for (n = 0; n < 12; ++n) {
	define_key(fkeys[11 - n], KEY_F(n + 1));
    }

    duplicate("kcub1", KEY_LEFT);
    duplicate("kcuu1", KEY_UP);
    duplicate("kcud1", KEY_DOWN);
    duplicate("kcuf1", KEY_RIGHT);

    define_key(0, KEY_A1);
    define_key(0, KEY_A3);
    define_key(0, KEY_B2);
    define_key(0, KEY_C1);
    define_key(0, KEY_C3);

    while ((ch = wgetch(win)) != ERR) {
	char *name = keyname(ch);
	wprintw(win, "Keycode %d, name %s\n",
		ch,
		name != 0 ? name : "<null>");
	wclrtoeol(win);
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
