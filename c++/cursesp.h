// * This makes emacs happy -*-Mode: C++;-*-
#ifndef _CURSESP_H
#define _CURSESP_H

#if 0
#pragma interface
#endif

#include "cursesw.h"
extern "C" {
#include <panel.h>
}

class NCursesPanel : public NCursesWindow {
    protected:
	PANEL *p;

    public:
	NCursesPanel();		// make a full window size panel
	NCursesPanel(int lines, int cols, int begin_y, int begin_x);
	inline ~NCursesPanel();

	// basic manipulation
	int hide() { return hide_panel(p); }
	int show() { return show_panel(p); }
	int top() { return top_panel(p); }
	int bottom() { return bottom_panel(p); }
	int mvpan(int y, int x) { return move_panel(p, y, x); }
	int mvwin(int y, int x) { return move_panel(p, y, x); }

	static void redraw();	// redraw all panels
	static void refresh();	// update screen

	// decorations
	int frame(char *title =0);
	int boldframe(char *title =0);
};

inline NCursesPanel::NCursesPanel()
	: NCursesWindow(0, 0, 0, 0)
{
    p = new_panel(w);
}

inline NCursesPanel::NCursesPanel(int lines, int cols,
				int begin_y, int begin_x)
	: NCursesWindow(lines, cols, begin_y, begin_x)
{
    p = new_panel(w);
}

inline NCursesPanel::~NCursesPanel()
{
    del_panel(p);
    update_panels();
    doupdate();
}

#endif // _CURSESP_H
