
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
* Module m_adabind.c                                                       *
* Helper routines to ease the implementation of an Ada95 binding to        *
* ncurses. For details and copyright of the binding see the ../Ada95       *
* subdirectory.                                                            *
***************************************************************************/
#include "menu.priv.h"

/* Prototypes for the functions in this module */
void _nc_ada_normalize_menu_opts (int *opt);
void _nc_ada_normalize_item_opts (int *opt);


void _nc_ada_normalize_menu_opts (int *opt)
{
  *opt = ALL_MENU_OPTS & (*opt);
}

void _nc_ada_normalize_item_opts (int *opt)
{
  *opt = ALL_ITEM_OPTS & (*opt);
}
