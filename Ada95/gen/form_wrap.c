/*----------------------------------------------------------------------------
//                                                                          --
//                           GNAT ncurses Binding                           --
//                                                                          --
//                               form_wrap.c                                --
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
#include <form.h>
#include <stdarg.h>

/* Prototypes for the functions in this module */
void _nc_normalize_field_opts (int *opt);
void _nc_normalize_form_opts (int *opt);


void _nc_normalize_field_opts (int *opt)
{
#define ALL_FIELD_OPTS (           \
			O_VISIBLE |\
			O_ACTIVE  |\
			O_PUBLIC  |\
			O_EDIT    |\
			O_WRAP    |\
			O_BLANK   |\
			O_AUTOSKIP|\
			O_NULLOK  |\
			O_PASSOK  |\
			O_STATIC   )

  *opt = ALL_FIELD_OPTS & (*opt);
}

void _nc_normalize_form_opts (int *opt)
{
#define ALL_FORM_OPTS  (                \
			O_NL_OVERLOAD  |\
			O_BS_OVERLOAD   )

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

/* Prototype */
void *_nc_ada_getvarg(va_list *);
 
/* Implementation */
void *_nc_ada_getvarg(va_list *ap)
{
  return va_arg(*ap,void*);
}
