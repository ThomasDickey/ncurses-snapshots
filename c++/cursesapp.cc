// * this is for making emacs happy: -*-Mode: C++;-*-
/*----------------------------------------------------------------------------+
|         The ncurses Application C++ binding is Copyright (C) 1997           |
|             by Juergen Pfeifer <Juergen.Pfeifer@T-Online.de>                |
|                          All Rights Reserved.                               |
|                                                                             |
| Permission to use, copy, modify, and distribute this software and its       |
| documentation for any purpose and without fee is hereby granted, provided   |
| that the above copyright notice appear in all copies and that both that     |
| copyright notice and this permission notice appear in supporting            |
| documentation, and that the name of the above listed copyright holder(s)    |
| not be used in advertising or publicity pertaining to distribution of the   |
| software without specific, written prior permission.                        |
|                                                                             |
| THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO |
| THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT- |
| NESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR  |
| ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER    |
| RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CON-   |
| TRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION |
| WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                               |
+----------------------------------------------------------------------------*/

#include "cursesapp.h"
#include "internal.h"

MODULE_ID("$Id: cursesapp.cc,v 1.1 1997/09/07 22:22:56 juergen Exp $")

void
NCursesApplication::init(bool bColors) {
  if (bColors)
    NCursesWindow::useColors();
  
  if (Root_Window->colors() > 1) {    
    b_Colors = TRUE;
    Root_Window->setcolor(1);
    Root_Window->setpalette(COLOR_YELLOW,COLOR_BLUE);
    Root_Window->setcolor(2);
    Root_Window->setpalette(COLOR_CYAN,COLOR_BLUE);
    Root_Window->setcolor(3);
    Root_Window->setpalette(COLOR_BLACK,COLOR_BLUE);
    Root_Window->setcolor(4);
    Root_Window->setpalette(COLOR_BLACK,COLOR_CYAN);
    Root_Window->setcolor(5);
    Root_Window->setpalette(COLOR_BLUE,COLOR_YELLOW);
    Root_Window->setcolor(6);
    Root_Window->setpalette(COLOR_BLACK,COLOR_GREEN);
  }
  else
    b_Colors = FALSE;

  Root_Window->bkgd(' '|window_backgrounds());
}

NCursesApplication* NCursesApplication::theApp = 0;
NCursesWindow* NCursesApplication::titleWindow = 0;
NCursesApplication::SLK_Link* NCursesApplication::slk_stack = 0;

NCursesApplication::~NCursesApplication() {
  Soft_Label_Key_Set* S;

  delete titleWindow;
  while( (S=top()) ) {
    pop();
    delete S;
  }
  delete Root_Window;
  ::endwin();
}

int NCursesApplication::rinit(NCursesWindow& w) {
  titleWindow = &w;
  return OK;
}

void NCursesApplication::push(Soft_Label_Key_Set& S) {
  SLK_Link* L = new SLK_Link;
  assert(L);
  L->prev = slk_stack;
  L->SLKs = &S;
  slk_stack = L;
  if (Root_Window)
    S.show();
}

bool NCursesApplication::pop() {
  if (slk_stack) {
    SLK_Link* L = slk_stack;
    slk_stack = slk_stack->prev;
    delete L;
    if (Root_Window && top())
      top()->show();
  }
  return (slk_stack ? FALSE : TRUE);
}

Soft_Label_Key_Set* NCursesApplication::top() const {
  if (slk_stack)
    return slk_stack->SLKs;
  else
    return (Soft_Label_Key_Set*)0;
}

int NCursesApplication::operator()(void) {
  bool bColors = b_Colors;
  Soft_Label_Key_Set* S;

  int ts = titlesize();
  if (ts>0)
    NCursesWindow::ripoffline(ts,rinit);
  Soft_Label_Key_Set::Label_Layout fmt = useSLKs();
  if (fmt!=Soft_Label_Key_Set::None) {    
    S = new Soft_Label_Key_Set(fmt);
    assert(S);
    init_labels(*S);
  }

  Root_Window = new NCursesWindow(::stdscr);
  init(bColors);

  if (ts>0)
    title();
  if (fmt!=Soft_Label_Key_Set::None) {
    push(*S);
  }

  return run();
}

NCursesApplication::NCursesApplication(bool bColors) {
  b_Colors = bColors;
  if (theApp)
    THROW(new NCursesException("Application object already created."));
  else
    theApp = this;
}
