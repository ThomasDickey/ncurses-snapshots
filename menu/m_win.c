
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
* Module menu_win                                                          *
* Menus window and subwindow association routines                          *
***************************************************************************/

#include "menu.priv.h"

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_win(MENU *menu, WINDOW *w)
|   
|   Description   :  Sets the window of the menu.
|
|   Return Values :  E_OK               - success
|                    E_POSTED           - menu is already posted
|
+--------------------------------------------------------------------------*/
int set_menu_win(MENU *menu, WINDOW *w)
{
    if (menu)
    {
	ASSERT_NOT_POSTED( menu );
	menu->userwin = w;
	_nc_Calculate_Item_Length_and_Width(menu);
    }
    else
	_nc_Default_Menu.userwin = w;
  
    RETURN(E_OK);
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  WINDOW *menu_win(const MENU *)
|   
|   Description   :  Returns pointer to the window of the menu
|
|   Return Values :  NULL on error, otherwise pointer to window
|
+--------------------------------------------------------------------------*/
WINDOW *menu_win(const MENU *menu)
{
    return menu ? menu->userwin : _nc_Default_Menu.userwin;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_sub(MENU *menu, WINDOW *w)
|   
|   Description   :  Sets the subwindow of the menu.
|
|   Return Values :  E_OK           - success
|                    E_POSTED       - menu is already posted
|
+--------------------------------------------------------------------------*/
int set_menu_sub(MENU *menu, WINDOW *w)
{
    if (menu)
    {
	ASSERT_NOT_POSTED(menu);
	menu->usersub = w;
	_nc_Calculate_Item_Length_and_Width(menu);
    }
    else
	_nc_Default_Menu.usersub = w;
  
    RETURN(E_OK);
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  WINDOW *menu_sub(const MENU *menu)
|   
|   Description   :  Returns a pointer to the subwindow of the menu
|
|   Return Values :  NULL on error, otherwise a pointer to the window
|
+--------------------------------------------------------------------------*/
WINDOW *menu_sub(const MENU * menu)
{
    return menu ? menu->usersub : _nc_Default_Menu.usersub;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int scale_menu(const MENU *menu)
|   
|   Description   :  Returns the minimum window size necessary for the
|                    subwindow of menu.  
|
|   Return Values :  E_OK                  - success
|                    E_BAD_ARGUMENT        - invalid menu pointer
|                    E_NOT_CONNECTED       - no items are connected to menu
|
+--------------------------------------------------------------------------*/
int scale_menu(const MENU *menu, int *rows, int *cols)
{
    if (!menu) 
	RETURN( E_BAD_ARGUMENT );

    if (menu->items && *(menu->items))
    {
	if (rows)
	    *rows = menu->height;
	if (cols)
	    *cols = menu->width;
	RETURN(E_OK);
    }
    else
	RETURN( E_NOT_CONNECTED );
}

/* menu_win.c ends here */
