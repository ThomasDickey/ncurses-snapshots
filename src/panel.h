/* panel.h -- interface file for panels library */

/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                     panels is copyright (C) 1995                         *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*	      All praise to the original author, Warren Tucker.            *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute panels   *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of panels in any    *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        panels comes AS IS with no warranty, implied or expressed.        *
*                                                                          *
***************************************************************************/

#ifndef _PANEL_H
#define _PANEL_H

#include <curses.h>

typedef struct panel
{
	WINDOW *win;
	int wstarty;
	int wendy;
	int wstartx;
	int wendx;
	struct panel *below;
	struct panel *above;
	char *user;
	struct panelcons *obscure;
}
PANEL;

extern  WINDOW *panel_window(PANEL *pan);
extern  void update_panels(void );
extern  int hide_panel(PANEL *pan);
extern  int show_panel(PANEL *pan);
extern  int del_panel(PANEL *pan);
extern  int top_panel(PANEL *pan);
extern  int bottom_panel(PANEL *pan);
extern  PANEL *new_panel(WINDOW *win);
extern  PANEL *panel_above(PANEL *pan);
extern  PANEL *panel_below(PANEL *pan);
extern  int set_panel_userptr(PANEL *pan,char *uptr);
extern  char *panel_userptr(PANEL *pan);
extern  int move_panel(PANEL *pan,int starty,int startx);
extern  int replace_panel(PANEL *pan,WINDOW *win);

#endif /* _PANEL_H */

/* end of panel.h */
