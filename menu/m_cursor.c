
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
* Module menu_cursor                                                       *
* Correctly position a menus cursor                                        *
***************************************************************************/

#include "menu.priv.h"

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  pos_menu_cursor  
|   
|   Description   :  Position logical cursor to current item in menu
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid menu
|                    E_NOT_POSTED    - Menu is not posted
+--------------------------------------------------------------------------*/
int pos_menu_cursor(const MENU * menu)
{
    if (!menu)
	RETURN(E_BAD_ARGUMENT);
    else
    {
	ITEM *item;
	int x, y;
	WINDOW *w, *s;
	  
	ASSERT_POSTED(menu);
      
	item = menu->curitem;
	assert(item);

	x = item->x * (1 + menu->itemlen);
	y = item->y - menu->toprow;
	w = menu->userwin ? menu->userwin : stdscr;
	s = menu->usersub ? menu->usersub : w;
	assert(w && s);
	  
	if ((menu->opt & O_SHOWMATCH) && (menu->pindex > 0))
	    x += ( menu->pindex + menu->marklen - 1);
      
	wmove(s,y,x);
      
	if ( w != s )
	{
	    wcursyncup(s);
	    wsyncup(s);
	    untouchwin(s);
	} 
    }
    RETURN(E_OK);
}

/* menu_cursor.c ends here */
