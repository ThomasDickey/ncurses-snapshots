
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
* Module menu_item_opt                                                    *
* Menus item option routines                                               *
***************************************************************************/

#include "menu.priv.h"

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_opts(ITEM *item, Item_Options opts)  
|   
|   Description   :  Set the options of the item. If there are relevant
|                    changes, the item is connected and the menu is posted,
|                    the menu will be redisplayed.
|
|   Return Values :  E_OK      - on success
|
+--------------------------------------------------------------------------*/
int set_item_opts(ITEM *item, Item_Options opts)
{ 
    if (item)
    {
	if (item->opt != opts)
	{		
	    MENU *menu = item->imenu;
	  
	    item->opt = opts;

	    if ((!(opts & O_SELECTABLE)) && (item->value!=0))
		item->value = 0;
	  
	    if (menu && (menu->status & _POSTED))
	    {
		MOVE_AND_POSTITEM( menu, item );
		_nc_Show_Menu(menu);
	    }
	}
    }
    else
	_nc_Default_Item.opt = opts;
  
    RETURN(E_OK);
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_opts_off(ITEM *item, Item_Options opts)   
|   
|   Description   :  Switch of the options for this item.
|
|   Return Values :  E_OK     - on success
|
+--------------------------------------------------------------------------*/
int item_opts_off(ITEM *item, Item_Options  opts)
{
    ITEM *citem = item;
  
    CDEFITEM(citem);
    opts = citem->opt & ~opts;
    return set_item_opts( item, opts );
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_opts_on(ITEM *item, Item_Options opts)   
|   
|   Description   :  Switch on the options for this item.
|
|   Return Values :  E_OK     - on success
|
+--------------------------------------------------------------------------*/
int item_opts_on(ITEM *item, Item_Options opts)
{
    ITEM *citem = item;
  
    CDEFITEM(citem);
    opts = citem->opt | opts;
    return set_item_opts( item, opts );
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  Item_Options item_opts(const ITEM *item)   
|   
|   Description   :  Switch of the options for this item.
|
|   Return Values :  Items options
|
+--------------------------------------------------------------------------*/
Item_Options item_opts(const ITEM * item)
{
    CDEFITEM(item);
    return item->opt;
}

/* menu_item_opt.c ends here */
