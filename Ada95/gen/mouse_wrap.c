/*----------------------------------------------------------------------------
//                                                                          --
//                           GNAT ncurses Binding                           --
//                                                                          --
//                               mouse_wrap.c                               --
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

#define BUTTON1_EVENTS (BUTTON1_RELEASED        |\
                        BUTTON1_PRESSED         |\
                        BUTTON1_CLICKED         |\
                        BUTTON1_DOUBLE_CLICKED  |\
                        BUTTON1_TRIPLE_CLICKED  |\
                        BUTTON1_RESERVED_EVENT  )

#define BUTTON2_EVENTS (BUTTON2_RELEASED        |\
                        BUTTON2_PRESSED         |\
                        BUTTON2_CLICKED         |\
                        BUTTON2_DOUBLE_CLICKED  |\
                        BUTTON2_TRIPLE_CLICKED  |\
                        BUTTON2_RESERVED_EVENT  )

#define BUTTON3_EVENTS (BUTTON3_RELEASED        |\
                        BUTTON3_PRESSED         |\
                        BUTTON3_CLICKED         |\
                        BUTTON3_DOUBLE_CLICKED  |\
                        BUTTON3_TRIPLE_CLICKED  |\
                        BUTTON3_RESERVED_EVENT  )

#define BUTTON4_EVENTS (BUTTON4_RELEASED        |\
                        BUTTON4_PRESSED         |\
                        BUTTON4_CLICKED         |\
                        BUTTON4_DOUBLE_CLICKED  |\
                        BUTTON4_TRIPLE_CLICKED  |\
                        BUTTON4_RESERVED_EVENT  )

void _nc_ada_mouse_event( mmask_t m, int *b, int *s );
void _nc_ada_mouse_event( mmask_t m, int *b, int *s )
{
  int k = 0;

  if ( m & BUTTON1_EVENTS)
    {
      k = 1;
    }
  else if ( m & BUTTON2_EVENTS)
    {
      k = 2;
    }
  else if ( m & BUTTON3_EVENTS)
    {
      k = 3;
    }
  else if ( m & BUTTON4_EVENTS)
    {
      k = 4;
    }

  if (k)
    {
      *b = k-1;
      if (BUTTON_RELEASE(m,k)) *s = 0;
      else if (BUTTON_PRESS(m,k)) *s = 1;
      else if (BUTTON_CLICK(m,k)) *s = 2;
      else if (BUTTON_DOUBLE_CLICK(m,k)) *s = 3;
      else if (BUTTON_TRIPLE_CLICK(m,k)) *s = 4;
      else if (BUTTON_RESERVED_EVENT(m,k)) *s = 5;
      else
	{
	  *s = -1;
	}
    }
  else
    {
      *s = 1;
      if (m & BUTTON_CTRL) *b = 4;
      else if (m & BUTTON_SHIFT) *b = 5;
      else if (m & BUTTON_ALT) *b = 6;
      else
	{
	  *b = -1;
	}
    }
}

int _nc_ada_mouse_mask ( int button, int state, mmask_t *mask );
int _nc_ada_mouse_mask ( int button, int state, mmask_t *mask )
{
  mmask_t b = (button<4) ? ((1<<button) << (6 * state)) :
    (BUTTON_CTRL << (button-4));

  if (button>=4 && state!=1)
    return ERR;

  *mask |= b;
  return OK;
}

extern SCREEN *SP;
extern void _nc_mouse_wrap(SCREEN *);
void _nc_ada_unregister_mouse (void);
void _nc_ada_unregister_mouse (void)
{
  _nc_mouse_wrap(SP);
}

