/*----------------------------------------------------------------------------
//                                                                          --
//                           GNAT ncurses Binding                           --
//                                                                          --
//                               menu_wrap.c                                --
//                                                                          --
//                                 B O D Y                                  --
//                                                                          --
//  Version 00.91                                                           --
//                                                                          --
//  The ncurses Ada95 binding is copyrighted 1996 by                        --
//  Juergen Pfeifer, Email: Juergen.Pfeifer@T-Online.de                     --
//                                                                          --
//  Permission is hereby granted to reproduce and distribute this           --
//  binding by any means and for any fee, whether alone or as part          --
//  of a larger distribution, in source or in binary form, PROVIDED         --
//  this notice is included with any such distribution, and is not          --
//  removed from any of its header files. Mention of ncurses and the        --
//  author of this binding in any applications linked with it is            --
//  highly appreciated.                                                     --
//                                                                          --
//  This binding comes AS IS with no warranty, implied or expressed.        --
//----------------------------------------------------------------------------
    Version Control
    $Revision: 1.2 $
  --------------------------------------------------------------------------*/
#include <menu.h>

/* Prototypes for the functions in this module */
void _nc_normalize_menu_opts (int *opt);
void _nc_normalize_item_opts (int *opt);


void _nc_normalize_menu_opts (int *opt)
{
#define ALL_MENU_OPTS (                 \
		       O_ONEVALUE     | \
		       O_SHOWDESC     | \
		       O_ROWMAJOR     | \
		       O_IGNORECASE   | \
		       O_SHOWMATCH    | \
		       O_NONCYCLIC    )

  *opt = ALL_MENU_OPTS & (*opt);
}

void _nc_normalize_item_opts (int *opt)
{
#define ALL_ITEM_OPTS (O_SELECTABLE)

  *opt = ALL_ITEM_OPTS & (*opt);
}
