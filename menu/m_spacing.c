
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
* Module menu_spacing                                                      *
* Routines to handle spacing between entries                               *
***************************************************************************/

#include "menu.priv.h"

#define MAX_SPC_DESC ((TABSIZE) ? (TABSIZE) : 8)
#define MAX_SPC_COLS ((TABSIZE) ? (TABSIZE) : 8)
#define MAX_SPC_ROWS (3)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_spacing(MENU *menu,int desc, int r, int c);
|   
|   Description   :  Set the spacing between entried
|
|   Return Values :  E_OK                 - on success
+--------------------------------------------------------------------------*/
int set_menu_spacing(MENU *menu, int s_desc, int s_row, int s_col )
{
  MENU *m = Normalize_Menu(menu);

  assert(m);
  if (m->status & _POSTED)
    RETURN(E_POSTED);

  if (((s_desc < 0) || (s_desc > MAX_SPC_DESC)) ||
      ((s_row  < 0) || (s_row  > MAX_SPC_ROWS)) ||
      ((s_col  < 0) || (s_col  > MAX_SPC_COLS)))
    RETURN(E_BAD_ARGUMENT);

  m->spc_desc = s_desc ? s_desc : 1;
  m->spc_rows = s_row  ? s_row  : 1;
  m->spc_cols = s_col  ? s_col  : 1;
  _nc_Calculate_Item_Length_and_Width(m);

  RETURN(E_OK);
}


/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int menu_spacing (const MENU *,int *,int *,int *);
|   
|   Description   :  Retrieve info about spacing between the entries
|
|   Return Values :  E_OK             - on success
+--------------------------------------------------------------------------*/
int menu_spacing( const MENU *menu, int* s_desc, int* s_row, int* s_col) 
{ 
  const MENU *m = Normalize_Menu(menu);
  
  assert(m);
  if (s_desc) *s_desc = m->spc_desc;
  if (s_row)  *s_row  = m->spc_rows;
  if (s_col)  *s_col  = m->spc_cols;

  RETURN(E_OK);
}

/* m_spacing.c ends here */
