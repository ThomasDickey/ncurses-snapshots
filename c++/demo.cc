/*
 *   Silly demo program for the NCursesPanel class.
 *
 *   written by Anatoly Ivasyuk (anatoly@nick.csh.rit.edu)
 */

#include "cursesw.h"
#include "cursesp.h"

#ifdef HAVE_LIBC_H
#include "libc.h"
#endif

main()
{
    //  We need to define a full screen panel for the main screen so
    //  that when redraws happen, the main screen actually gets redrawn.
    //  I think there may be a bug in the panels code which won't redraw
    //  the main screen otherwise.  Maybe someone out there can find it...

    NCursesPanel *std = new NCursesPanel();

    //  Make a few small demo panels

    NCursesPanel *u = new NCursesPanel(10,20,12,4);
    NCursesPanel *v = new NCursesPanel(10,20,10,6);
    NCursesPanel *w = new NCursesPanel(10,20,8,8);
    NCursesPanel *x = new NCursesPanel(10,20,6,10);
    NCursesPanel *y = new NCursesPanel(10,20,4,12);
    NCursesPanel *z = new NCursesPanel(10,30,2,14);

    //  Draw something on the main screen, so we can see what happens
    //  when panels get moved or deleted.

    std->box();
    std->move(10,0);
    std->hline('a',79);
    std->move(0,40);
    std->vline(20);

    //  Draw frames with titles around panels so that we can see where
    //  the panels are located.

    u->boldframe("Win U");
    v->frame("Win V");
    w->boldframe("Win W");
    x->frame("Win X");
    y->boldframe("Win Y");
    z->frame("Win Z");

    //  A refresh to any valid panel updates all panels and refreshes
    //  the screen.  Using std is just convenient - We know it's always
    //  valid until the end of the program.

    std->refresh();

    //  Show that things actually come back correctly when the screen
    //  is cleared and the global NCursesPanel::redraw() is called.

    sleep(2);
    ::clear();			// call ncurses clear() directly
    ::wrefresh(stdscr);		// call ncurses refresh directly
    sleep(2);
    NCursesPanel::redraw();

    //  Show what happens when panels are deleted and moved.

    sleep(2);
    delete u;
    std->refresh();

    sleep(2);
    delete z;
    std->refresh();

    sleep(2);
    delete v;
    std->refresh();

    // show how it looks when a panel moves
    sleep(2);
    y->mvpan(5,30);
    std->refresh();

    sleep(2);
    delete y;
    std->refresh();

    // show how it looks when you raise a panel
    sleep(2);
    w->top();
    std->refresh();

    sleep(2);
    delete w;
    std->refresh();

    sleep(2);
    delete x;
    std->refresh();

    //  Don't forget to clean up the main screen.  Since this is the
    //  last thing using NCursesWindow, this has the effect of
    //  shutting down ncurses and restoring the terminal state.

    sleep(2);
    delete std;
}
