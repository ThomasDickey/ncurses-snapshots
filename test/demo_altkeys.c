/*
 * $Id: demo_altkeys.c,v 1.2 2005/10/22 18:59:42 tom Exp $
 *
 * Demonstrate the define_key() function.
 * Thomas Dickey - 2005/10/22
 */

#include <test.priv.h>

#if defined(NCURSES_VERSION) && NCURSES_EXT_FUNCS

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <term.h>

#define MY_LOGFILE "demo_altkeys.log"
#define MY_KEYS (KEY_MAX + 1)

/*
 * Log the most recently-written line to our logfile
 */
static void
log_last_line(WINDOW *win)
{
    FILE *fp;
    int y, x, n;
    char temp[256];

    if ((fp = fopen(MY_LOGFILE, "a")) != 0) {
	int need = sizeof(temp) - 1;
	if (need > COLS)
	    need = COLS;
	getyx(win, y, x);
	wmove(win, y - 1, 0);
	n = winnstr(win, temp, need);
	while (n-- > 0) {
	    if (isspace(UChar(temp[n])))
		temp[n] = '\0';
	    else
		break;
	}
	wmove(win, y, x);
	fprintf(fp, "%s\n", temp);
	fclose(fp);
    }
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    int n;
    int ch;
    SCREEN *sp;
#if HAVE_GETTIMEOFDAY
    int secs, msecs;
    struct timeval current, previous;
#endif

    unlink(MY_LOGFILE);

    sp = newterm(0, stdout, stdin);
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    move(0, 0);

    /* we do the define_key() calls after keypad(), since the first call to
     * keypad() initializes the corresponding data.
     */
    for (n = 0; n < 255; ++n) {
	char temp[10];
	sprintf(temp, "\033%c", n);
	define_key(temp, n + MY_KEYS);
    }
    for (n = KEY_MIN; n < KEY_MAX; ++n) {
	char *value;
	if ((value = keybound(n, 0)) != 0) {
	    char *temp = malloc(strlen(value) + 2);
	    sprintf(temp, "\033%s", value);
	    define_key(temp, n + MY_KEYS);
	    free(temp);
	    free(value);
	}
    }

#if HAVE_GETTIMEOFDAY
    gettimeofday(&previous, 0);
#endif

    while ((ch = getch()) != ERR) {
	bool escaped = (ch >= MY_KEYS);
	const char *name = keyname(escaped ? (ch - MY_KEYS) : ch);

#if HAVE_GETTIMEOFDAY
	gettimeofday(&current, 0);
	secs = current.tv_sec - previous.tv_sec;
	msecs = (current.tv_usec - previous.tv_usec) / 1000;
	if (msecs < 0) {
	    msecs += 1000;
	    --secs;
	}
	if (msecs >= 1000) {
	    secs += msecs / 1000;
	    msecs %= 1000;
	}
	printw("%6d.%03d ", secs, msecs);
	previous = current;
#endif
	printw("Keycode %d, name %s%s\n",
	       ch,
	       escaped ? "ESC-" : "",
	       name != 0 ? name : "<null>");
	log_last_line(stdscr);
	clrtoeol();
	if (ch == 'q')
	    break;
    }
    endwin();
#if NO_LEAKS
    delscreen(sp);
#endif
    ExitProgram(EXIT_FAILURE);
}
#else
int
main(void)
{
    printf("This program requires the ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif
