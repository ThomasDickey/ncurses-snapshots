
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
* Module menu_attribs                                                      *
* Control menus display attributes                                         *
***************************************************************************/

#include "menu.priv.h"
#include <ctype.h>

/* Macro to redraw menu if it is posted and changed */
#define REFRESH_MENU(menu) \
   if ( (menu) && ((menu)->status & _POSTED) )\
   {\
      _nc_Draw_Menu( menu );\
      _nc_Show_Menu( menu );\
   }

/* "Template" macro to generate a function to set a menus attribute */
#define GEN_MENU_ATTR_SET_FCT( name ) \
int set_menu_ ## name (\
   MENU * menu,\
   chtype attr\
)\
{\
   if (!(attr==A_NORMAL || (attr & A_ATTRIBUTES)!=0))\
      RETURN(E_BAD_ARGUMENT);\
   CDEFMENU( menu );\
   (menu -> name) = attr & A_ATTRIBUTES;\
   if (menu != &_nc_Default_Menu)\
     {\
	REFRESH_MENU( menu );\
     }\
   RETURN(E_OK);\
}

/* "Template" macro to generate a function to get a menus attribute */
#define GEN_MENU_ATTR_GET_FCT( name ) \
chtype menu_ ## name (\
   const MENU * menu\
)\
{\
   CDEFMENU( menu );\
   return ( (menu -> name) );\
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_fore(MENU *menu, chtype attr)
|   
|   Description   :  Set the attribute for selectable items. In single-
|                    valued menus thiis is used to highlight the current
|                    item ((i.e. where the cursor is), in multi-valued
|                    menus this is used to highlight the selected items.
|
|   Return Values :  E_OK              - on success
|                    E_BAD_ARGUMENT    - an invalid value has been passed   
|
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_SET_FCT( fore )

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  chtype menu_fore(const MENU* menu)
|   
|   Description   :  Return the attribute used for selectable items that
|                    are current (single-valued menu) or selected (multi-
|                    valued menu).   
|
|   Return Values :  Attribute value
|
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_GET_FCT( fore )

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_back(MENU *menu, chtype attr)
|   
|   Description   :  Set the attribute for selectable but not yet selected
|                    items.
|
|   Return Values :  E_OK             - on success  
|                    E_BAD_ARGUMENT   - an invalid value has been passed
|
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_SET_FCT( back )


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  chtype menu_back(const MENU *menu)
|   
|   Description   :  Return the attribute used for selectable but not yet
|                    selected items. 
|
|   Return Values :  Attribute value
|
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_GET_FCT( back )

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_grey(MENU *menu, chtype attr)
|   
|   Description   :  Set the attribute for unselectable items.
|
|   Return Values :  E_OK             - on success
|                    E_BAD_ARGUMENT   - an invalid value has been passed    
|
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_SET_FCT( grey )

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  chtype menu_grey(const MENU *menu)
|   
|   Description   :  Return the attribute used for non-selectable items
|
|   Return Values :  Attribute value
|
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_GET_FCT( grey )

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_pad(MENU *menu, int pad)
|   
|   Description   :  Set the character to be used to separate the item name
|                    from its description. This must be a printable 
|                    character.
|
|   Return Values :  E_OK              - on success
|                    E_BAD_ARGUMENT    - an invalid value has been passed
|
+--------------------------------------------------------------------------*/
int set_menu_pad(MENU *menu, int pad)
{
    unsigned char padch = (unsigned char)pad;

    if (!isprint(padch))
	RETURN(E_BAD_ARGUMENT);
  
    CDEFMENU( menu );
    menu->pad = pad;
  
    if (menu != &_nc_Default_Menu)
    {
	REFRESH_MENU( menu );
    }
    RETURN(E_OK);
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int menu_pad(const MENU *menu)
|   
|   Description   :  Return the value of the padding character
|
|   Return Values :  The pad character
|
+--------------------------------------------------------------------------*/
int menu_pad(const MENU * menu)
{
    CDEFMENU( menu );
    return (menu->pad);
}

/* menu_attribs.c ends here */
