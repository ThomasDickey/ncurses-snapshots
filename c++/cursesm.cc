// * this is for making emacs happy: -*-Mode: C++;-*-
/*----------------------------------------------------------------------------+
|            The ncurses menu C++ binding is Copyright (C) 1997               |
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

#include "internal.h"

#ifdef __GNUG__
#  pragma implementation
#endif

MODULE_ID("$Id: cursesm.cc,v 1.6 1997/07/13 20:33:05 juergen Exp $")
  
#include "cursesm.h"

const int CMD_QUIT   = MAX_COMMAND + 1;
const int CMD_ACTION = MAX_COMMAND + 2;

NCursesMenuItem::~NCursesMenuItem() {
}

bool 
NCursesMenuItem::action() {
  return FALSE;
};

NCursesMenuCallbackItem::~NCursesMenuCallbackItem() {
}

bool 
NCursesMenuCallbackItem::action() {
  if (p_fct)
    return p_fct (*this);
  else
    return FALSE;
}

unsigned long NCursesMenu::total_count = 0;

/* Internal hook functions. They will route the hook
 * calls to virtual methods of the NCursesMenu class,
 * so in C++ providing a hook is done simply by 
 * implementing a virtual method in a derived class
 */
void
NCursesMenu::mnu_init(MENU *m) {
  getHook(m)->On_Menu_Init();
}

void
NCursesMenu::mnu_term(MENU *m) {
  getHook(m)->On_Menu_Termination();
}

void
NCursesMenu::itm_init(MENU *m) {
  NCursesMenu* M = getHook(m);
  M->On_Item_Init (M->current_item ());
}

void
NCursesMenu::itm_term(MENU *m) {
  NCursesMenu* M = getHook(m);
  M->On_Item_Termination (M->current_item ());
}

/* Construct an ITEM* array from an array of NCursesMenuItem
 * objects.
 */
ITEM**
NCursesMenu::mapItems(NCursesMenuItem* nitems[]) {
  int itemCount = 0,lcv;
  
  for (lcv=0; nitems[lcv]->item; ++lcv)
    ++itemCount;
  
  ITEM** items = new ITEM*[itemCount + 1];
  
  for (lcv=0;nitems[lcv]->item;++lcv) {
    items[lcv] = nitems[lcv]->item;
  }
  items[lcv] = NULL;
  
  my_items = nitems;
  
  if (menu)
    delete[] ::menu_items(menu);  
  return items;
}

void
NCursesMenu::InitMenu(NCursesMenuItem* nitems[],
		      bool with_frame) {
  int mrows, mcols;
  
  if (total_count++==0) {
    raw();
    keypad(TRUE);
  }
  
  b_framed = with_frame;

  menu = (MENU*)0;
  menu = ::new_menu(mapItems(nitems));
  if (!menu)
    OnError (E_SYSTEM_ERROR);
  
  UserHook* hook = new UserHook;
  hook->m_user   = NULL;
  hook->m_back   = this;
  hook->m_owner  = menu;
  ::set_menu_userptr(menu,(void*)hook);
  
  ::set_menu_init (menu, NCursesMenu::mnu_init);
  ::set_menu_term (menu, NCursesMenu::mnu_term);
  ::set_item_init (menu, NCursesMenu::itm_init);
  ::set_item_term (menu, NCursesMenu::itm_term);
  
  scale(mrows, mcols);
  ::set_menu_win(menu, w);
  
  if (with_frame) {
    if ((mrows > height()-2) || (mcols > width()-2))
      OnError(E_NO_ROOM);  
    sub = new NCursesWindow(*this,mrows,mcols,1,1,'r');
    ::set_menu_sub(menu, sub->w);
    b_sub_owner = TRUE;
  }
  else {
    sub = (NCursesWindow*)0;
    b_sub_owner = FALSE;
  }
  setDefaultAttributes();
}

void
NCursesMenu::setDefaultAttributes() {
  if (NumberOfColors() > 1) {
    setcolor(1);
    setpalette(COLOR_YELLOW,COLOR_BLUE);
    setcolor(2);
    setpalette(COLOR_CYAN,COLOR_BLUE);
    setcolor(3);
    setpalette(COLOR_WHITE,COLOR_CYAN);
    ::set_menu_fore(menu, COLOR_PAIR(1));
    ::set_menu_back(menu, COLOR_PAIR(2));
    ::set_menu_grey(menu, COLOR_PAIR(3));
  }
  else {
    ::set_menu_fore(menu, A_BOLD);
    ::set_menu_back(menu, A_NORMAL);
    ::set_menu_grey(menu, A_DIM);
  }
}

NCursesMenu::~NCursesMenu() {
  UserHook* hook = (UserHook*)::menu_userptr(menu);
  delete hook;
  if (b_sub_owner) {
    delete sub;
    ::set_menu_sub(menu,(WINDOW *)0);
  }
  if (menu) {
    ITEM** itms = ::menu_items(menu);
    ::free_menu(menu);
    // It's essential to do this after free_menu()
    delete[] itms;  
  }
  --total_count;
}

void
NCursesMenu::setSubWindow(NCursesWindow& nsub) {
  if (!isDescendant(nsub))
    OnError(E_SYSTEM_ERROR);
  else {
    if (b_sub_owner)
      delete sub;
    sub = &nsub;
    ::set_menu_sub(menu,sub->w);
  }
}

// call the menu driver and do basic error checking.
int 
NCursesMenu::driver (int c) {
  int res = ::menu_driver (menu, c);
  switch (res) {
  case E_OK:
  case E_REQUEST_DENIED:
  case E_NOT_SELECTABLE:
  case E_UNKNOWN_COMMAND:
  case E_NO_MATCH:
    break;
  default:
    OnError (res);
  }
  return (res);
}

bool
NCursesMenu::set_pattern (const char *pat) {
  int res = ::set_menu_pattern (menu, pat);
  switch(res) {
  case E_OK:
    break;
  case E_NO_MATCH:
    return FALSE;
  default:
    OnError (res);
  }
  return TRUE;
}

// Provide a default key virtualization. Translate the keyboard
// code c into a menu request code.
// The default implementation provides a hopefully straightforward
// mapping for the most common keystrokes and menu requests.
int 
NCursesMenu::virtualize(int c) {
  switch(c) {
  case CTRL('Q')     : return(CMD_QUIT);
  case KEY_DOWN      :
  case CTRL('N')     : return(REQ_NEXT_ITEM);
  case KEY_UP        :
  case CTRL('P')     : return(REQ_PREV_ITEM);
  case CTRL('U')     : return(REQ_SCR_ULINE);
  case CTRL('D')     : return(REQ_SCR_DLINE);
  case CTRL('F')     : return(REQ_SCR_DPAGE);
  case CTRL('B')     : return(REQ_SCR_UPAGE);
  case CTRL('X')     : return(REQ_CLEAR_PATTERN);
  case CTRL('H')     : return(REQ_BACK_PATTERN);
  case CTRL('A')     : return(REQ_NEXT_MATCH);
  case CTRL('Z')     : return(REQ_PREV_MATCH);
  case CTRL('T')     : return(REQ_TOGGLE_ITEM);
  case CTRL('J')     :
  case CTRL('M')     : return(CMD_ACTION);
  case KEY_HOME      : return(REQ_FIRST_ITEM);
  case KEY_LEFT      : return(REQ_LEFT_ITEM);
  case KEY_RIGHT     : return(REQ_RIGHT_ITEM);
  case KEY_END       : return(REQ_LAST_ITEM);
  case KEY_BACKSPACE : return(REQ_BACK_PATTERN);
  case KEY_NPAGE     : return(REQ_SCR_DPAGE);
  case KEY_PPAGE     : return(REQ_SCR_UPAGE);

  default:
    return(c);
  }
}

NCursesMenuItem&
NCursesMenu::operator()(void) {
  int drvCmnd;
  int err;
  int c;
  bool b_action = FALSE;

  post();
  show();
  refresh();
  
  while (!b_action && ((drvCmnd = virtualize((c=getch()))) != CMD_QUIT)) {
    switch((err=driver(drvCmnd))) {
    case E_REQUEST_DENIED:
      On_Request_Denied(c);
      break;
    case E_NOT_SELECTABLE:
      On_Not_Selectable(c);
      break;
    case E_UNKNOWN_COMMAND:
      if (drvCmnd == CMD_ACTION) {
	NCursesMenuItem& itm = current_item();
	b_action = itm.action();
      } else
	On_Unknown_Command(c);
      break;
    case E_NO_MATCH:
      On_No_Match(c);
      break;
    case E_OK:
      break;
    default:
      OnError(err);
    }
  }

  unpost();
  hide();
  refresh();
  return *(my_items[::item_index (::current_item (menu))]);
}

void
NCursesMenu::On_Menu_Init() {
}

void
NCursesMenu::On_Menu_Termination() {
}

void
NCursesMenu::On_Item_Init(NCursesMenuItem& item) {
}

void
NCursesMenu::On_Item_Termination(NCursesMenuItem& item) {
}

void
NCursesMenu::On_Request_Denied(int c) const {
  beep();
}

void
NCursesMenu::On_Not_Selectable(int c) const {
  beep();
}

void
NCursesMenu::On_No_Match(int c) const {
  beep();
}

void
NCursesMenu::On_Unknown_Command(int c) const {
  beep();
}

