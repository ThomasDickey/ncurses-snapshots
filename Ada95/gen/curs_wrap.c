/*----------------------------------------------------------------------------
//                                                                          --
//                           GNAT ncurses Binding                           --
//                                                                          --
//                               curs_wrap.c                                --
//                                                                          --
//                                 B O D Y                                  --
//                                                                          --
//  Version 00.91                                                           --
//                                                                          --
//  The ncurses Ada95 binding is copyrighted 1996 by                        --
//  Juergen Pfeifer, Email: Juergen.Pfeifer@T-Online.de                     --
//                                                                          --
//  Permission is hereby granted to reproduce and distribute this           --
//  binding by any means and for any fee, whether alone or as part          --
//  of a larger distribution, in source or in binary form, PROVIDED         --
//  this notice is included with any such distribution, and is not          --
//  removed from any of its header files. Mention of ncurses and the        --
//  author of this binding in any applications linked with it is            --
//  highly appreciated.                                                     --
//                                                                          --
//  This binding comes AS IS with no warranty, implied or expressed.        --
//----------------------------------------------------------------------------
    Version Control
    $Revision: 1.2 $
  --------------------------------------------------------------------------*/
#include <curses.h>

/*  In (n)curses are a few functionalities that can't be expressed as 
//  functions, because for historic reasons they use as macro argument
//  variable names that are "out" parameters. For those macros we provide
//  small wrappers.
*/

/* Prototypes for the functions in this module */
int _nc_ada_getmaxyx (WINDOW *win, int *y, int *x);
int _nc_ada_getbegyx (WINDOW *win, int *y, int *x);
int _nc_ada_getyx (WINDOW *win, int *y, int *x);
int _nc_ada_getparyx (WINDOW *win, int *y, int *x);
int _nc_ada_isscroll (WINDOW *win);


int _nc_ada_getmaxyx (WINDOW *win, int *y, int *x)
{
  if (win && y && x)
    {
      getmaxyx(win,*y,*x);
      return OK;
    }
  else
    return ERR;
}

int _nc_ada_getbegyx (WINDOW *win, int *y, int *x)
{
  if (win && y && x)
    {
      getbegyx(win,*y,*x);
      return OK;
    }
  else
    return ERR;
}

int _nc_ada_getyx (WINDOW *win, int *y, int *x)
{
  if (win && y && x)
    {
      getyx(win,*y,*x);
      return OK;
    }
  else
    return ERR;
}

int _nc_ada_getparyx (WINDOW *win, int *y, int *x)
{
  if (win && y && x)
    {
      getparyx(win,*y,*x);
      return OK;
    }
  else
    return ERR;
}

int _nc_ada_isscroll (WINDOW *win)
{
  return win ? (win->_scroll ? TRUE : FALSE) : ERR;
}

int _nc_ada_coord_transform( WINDOW *win, int *Y, int *X, int dir);
int _nc_ada_coord_transform( WINDOW *win, int *Y, int *X, int dir)
{
  if (win && Y && X)
    {
      int y = *Y; int x = *X;
      if (dir)
	{ /* to screen coordinates */
	  y += win->_yoffset;
	  y += win->_begy;
	  x += win->_begx;
	  if (!wenclose(win,y,x))
	    return FALSE;
	}
      else
	{ /* from screen coordinates */
	  if (!wenclose(win,y,x))
	    return FALSE;
	  y -= win->_yoffset;
	  y -= win->_begy;
	  x -= win->_begx;
	}
      *X = x;
      *Y = y;
      return TRUE;
    }
  return FALSE;
}
