
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/

/***************************************************************************
* Module menu.priv.h                                                       *
* Top level private header file for all libnmenu modules                   *
***************************************************************************/

#include <stdlib.h>
#include <assert.h>

#include "menu.h"

/* Backspace code */
#define BS (8)

/* Maximum regular 8-bit character code */
#define MAX_REGULAR_CHARACTER (0xff)

extern ITEM _nc_Default_Item;
extern MENU _nc_Default_Menu;

#define RETURN(code) return( errno=(code) )

/* Normalize item to default if none was given */
#define CDEFITEM( item ) if (!(item)) item = &_nc_Default_Item

/* Normalize menu to default if none was given */
#define CDEFMENU( menu ) if (!(menu)) menu = &_nc_Default_Menu

/* Normalize menu window */
#define CDEFWIN(  menu ) \
   ( menu->usersub  ? menu->usersub  : (\
     menu->userwin  ? menu->userwin  : stdscr ))

/* Call menu handler */
#define CALL_HANDLER( menu, handler ) \
   if ( (menu) && ((menu)->handler) )\
   {\
	(menu)->status |= _IN_DRIVER;\
	(menu)->handler(menu);\
	(menu)->status &= ~_IN_DRIVER;\
   }

/* This macro should be called to assert that one is not in the
   middle of a hook
*/
#define ASSERT_NOT_IN_HOOK( menu ) \
   if ( (menu)->status & _IN_DRIVER )\
      RETURN(E_BAD_STATE)

/* This macro asserts that the menu is not already posted */
#define ASSERT_NOT_POSTED( menu ) \
   if ( (menu)->status & _POSTED )\
      RETURN(E_POSTED)

/* This macro asserts, that the menu is posted */
#define ASSERT_POSTED( menu ) \
   if ( !( (menu)->status & _POSTED ) )\
      RETURN(E_NOT_POSTED)

/* Move to the window position of an item and draw it */
#define MOVE_AND_POSTITEM(menu,item) \
   wmove((menu)->win,(item)->y,((menu)->itemlen+1)*(item)->x);\
   _nc_Post_Item((menu),(item))

#define MOVE_TO_CURRENT_ITEM(menu,item) \
   if ( (item) != (menu)->curitem)\
   {\
      MOVE_AND_POSTITEM(menu,item);\
      MOVE_AND_POSTITEM(menu,(menu)->curitem);\
   }

/* This macro ensures, that the item becomes visible, if possible with the
   specified row as the top row of the window. If this is not possible,
   the top row will be adjusted and the value is stored in the row argument. 
*/
#define ADJUST_CURRENT_ITEM(menu,row,item) \
   if ((item)->y < row) \
      row = (item)->y;\
   if ( (item)->y >= (row + (menu)->height) )\
       row = ( (item)->y < ((menu)->rows - row) ) ? \
	                    (item)->y : (menu)->rows - (menu)->height;\
   _nc_New_TopRow_and_CurrentItem(menu,row,item)

#define RESET_PATTERN(menu) \
   (menu)->pindex = 0; \
   (menu)->pattern[0] = '\0'

#define REMOVE_CHARACTER_FROM_PATTERN(menu) \
   (menu)->pattern[--((menu)->pindex)] = '\0'

#define ADD_CHARACTER_TO_PATTERN(menu,ch) \
    (menu)->pattern[((menu)->pindex)++] = (ch);\
    (menu)->pattern[(menu)->pindex] = '\0'

/* Internal functions. */						
extern void _nc_Draw_Menu(const MENU *);
extern void _nc_Show_Menu(const MENU *);
extern void _nc_Calculate_Item_Length_and_Width(MENU *);
extern void _nc_Post_Item(const MENU *, const ITEM *);
extern bool _nc_Connect_Items(MENU *, ITEM **);
extern void _nc_Disconnect_Items(MENU *);
extern void _nc_New_TopRow_and_CurrentItem(MENU *,int, ITEM *);
extern void _nc_Link_Items(MENU *);
