/*
 * clear.c --  clears the terminal's screen
 */

#include <curses.h>
#include <terminfo.h>

static int putch(char c)
{
	return putchar(c);
}

int main(int argc, char *argv[])
{
	setupterm((char *) 0, 1, (int *) 0); 
	if (clear_screen == (char *) 0)
		exit(1);
	tputs(clear_screen, lines > 0 ? lines : 1, putch);
	return 0;
}

