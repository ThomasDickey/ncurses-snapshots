
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
* Module frm_adabind.c                                                     *
* Helper routines to ease the implementation of an Ada95 binding to        *
* ncurses. For details and copyright of the binding see the ../Ada95       *
* subdirectory.                                                            *
***************************************************************************/
#include "form.priv.h"

/* Prototypes for the functions in this module */
void _nc_ada_normalize_field_opts (int *opt);
void _nc_ada_normalize_form_opts (int *opt);
void *_nc_ada_getvarg(va_list *);


void _nc_ada_normalize_field_opts (int *opt)
{
  *opt = ALL_FIELD_OPTS & (*opt);
}

void _nc_ada_normalize_form_opts (int *opt)
{
  *opt = ALL_FORM_OPTS & (*opt);
}


/*  This tiny stub helps us to get a void pointer from an argument list.
//  The mechanism for libform to handle arguments to field types uses
//  unfortunately functions with variable argument lists. In the Ada95
//  binding we replace this by a mechanism that only uses one argument
//  that is a pointer to a record describing all the specifics of an
//  user defined field type. So we need only this simple generic
//  procedure to get the pointer from the arglist.
*/
void *_nc_ada_getvarg(va_list *ap)
{
  return va_arg(*ap,void*);
}
