
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
* Module menu_request_name                                                 *
* Routines to handle external names of menu requests                       *
***************************************************************************/

#include "menu.priv.h"

static const char *request_names[ MAX_MENU_COMMAND - MIN_MENU_COMMAND + 1 ] = {
  "LEFT_ITEM"    ,
  "RIGHT_ITEM"   ,
  "UP_ITEM"      ,
  "DOWN_ITEM"    ,
  "SCR_ULINE"    ,
  "SCR_DLINE"    ,
  "SCR_DPAGE"    ,
  "SCR_UPAGE"    ,
  "FIRST_ITEM"   ,
  "LAST_ITEM"    ,
  "NEXT_ITEM"    ,
  "PREV_ITEM"    ,
  "TOGGLE_ITEM"  ,
  "CLEAR_PATTERN",
  "BACK_PATTERN" ,
  "NEXT_MATCH"   ,
  "PREV_MATCH"          
};
#define A_SIZE (sizeof(request_names)/sizeof(request_names[0]))

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  const char * menu_request_name (int request);
|   
|   Description   :  Get the external name of a menu request.
|
|   Return Values :  Pointer to name      - on success
|                    NULL                 - on invalid request code
+--------------------------------------------------------------------------*/
const char *menu_request_name( int request )
{
  if ( (request < MIN_MENU_COMMAND) || (request > MAX_MENU_COMMAND) )
    {
      SET_ERROR(E_BAD_ARGUMENT);
      return (const char *)0;
    }
  else
    return request_names[ request - MIN_MENU_COMMAND ];
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int menu_request_by_name (const char *str);
|   
|   Description   :  Search for a request with this name.
|
|   Return Values :  Request Id       - on success
|                    E_NO_MATCH       - request not found
+--------------------------------------------------------------------------*/
int menu_request_by_name( const char *str )
{ 
  /* because the table is so small, it doesn't really hurt
     to run sequentially through it.
  */
  unsigned int i = 0;
  char buf[16];
  
  if (str)
    {
      strncpy(buf,str,sizeof(buf));
      while( (i<sizeof(buf)) && (buf[i] != '\0') )
	{
	  buf[i] = toupper(buf[i]);
	  i++;
	}
      
      for (i=0; i < A_SIZE; i++)
	{
	  if (strncmp(request_names[i],buf,sizeof(buf))==0)
	    return MIN_MENU_COMMAND + i;
	} 
    }
  RETURN(E_NO_MATCH);
}

/* m_req_name.c ends here */
