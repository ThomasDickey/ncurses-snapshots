#include <curses.h>

main()
{
int i = 0;

	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	while(getch() != 'q') {
		mvprintw(LINES-1, 0, "printing line # %d", i++);
		scroll(stdscr); 
		refresh();
	}
	endwin();

}
