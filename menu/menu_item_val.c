
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
* Module menu_item_val                                                     *
* Set and get menus item values                                            *
***************************************************************************/

#include "menu.priv.h"

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_value(ITEM *item, int value)
|   
|   Description   :  Programmatically set the items selection value. This is
|                    only allowed if the item is selectable at all and if
|                    it is not connected to a single-valued menu.
|                    If the item is connected to a posted menu, the menu
|                    will be redisplayed.  
|
|   Return Values :  E_OK              - on success
|                    E_REQUEST_DENIED  - not selectable or single valued menu
+--------------------------------------------------------------------------*/
int set_item_value(ITEM *item, int value)
{
    MENU *menu;
  
    /* we always normalize the value */
    if (value)    
	value = 1;

    if (item)
    {
	menu = item->imenu;

	if ((!(item->opt & O_SELECTABLE)) ||
		(menu && (menu->opt & O_ONEVALUE))) 
	    RETURN(E_REQUEST_DENIED);
      
	if (item->value != value)
	{
	    item->value = value;
	    if (menu)
	    {
		if (menu->status & _POSTED)
		{
		    MOVE_AND_POSTITEM(menu,item);
		    _nc_Show_Menu(menu);
		}
	    }
	}
    }
    else
	_nc_Default_Item.value = value;
  
    RETURN(E_OK);
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_value(const ITEM *item)
|   
|   Description   :  Return the selection value of the item
|
|   Return Values :  The items selection value
|
+--------------------------------------------------------------------------*/
int item_value(const ITEM *item)
{
    CDEFITEM(item);
    return (item->value ? 1 : 0);
}

/* menu_item_val.c ends here */
