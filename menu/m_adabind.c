/*-----------------------------------------------------------------------------+
|           The ncurses menu library is  Copyright (C) 1995-1997               |
|             by Juergen Pfeifer <Juergen.Pfeifer@T-Online.de>                 |
|                          All Rights Reserved.                                |
|                                                                              |
| Permission to use, copy, modify, and distribute this software and its        |
| documentation for any purpose and without fee is hereby granted, provided    |
| that the above copyright notice appear in all copies and that both that      |
| copyright notice and this permission notice appear in supporting             |
| documentation, and that the name of the above listed copyright holder(s) not |
| be used in advertising or publicity pertaining to distribution of the        |
| software without specific, written prior permission.                         | 
|                                                                              |
| THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO  |
| THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-  |
| NESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR   |
| ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RE- |
| SULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, |
| NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH    |
| THE USE OR PERFORMANCE OF THIS SOFTWARE.                                     |
+-----------------------------------------------------------------------------*/

/***************************************************************************
* Module m_adabind.c                                                       *
* Helper routines to ease the implementation of an Ada95 binding to        *
* ncurses. For details and copyright of the binding see the ../Ada95       *
* subdirectory.                                                            *
***************************************************************************/
#include "menu.priv.h"

MODULE_ID("$Id: m_adabind.c,v 1.5 1997/09/05 23:01:32 juergen Exp $")

/* Prototypes for the functions in this module */
void  _nc_ada_normalize_menu_opts (int *opt);
void  _nc_ada_normalize_item_opts (int *opt);
ITEM* _nc_get_item(const MENU*, int);

void _nc_ada_normalize_menu_opts (int *opt)
{
  *opt = ALL_MENU_OPTS & (*opt);
}

void _nc_ada_normalize_item_opts (int *opt)
{
  *opt = ALL_ITEM_OPTS & (*opt);
}

ITEM* _nc_get_item(const MENU* menu, int idx) {
  if (menu && menu->items && idx>=0 && (idx<menu->nitems))
    {
      return menu->items[idx];
    }
  else
    return (ITEM*)0;
}
