/*-----------------------------------------------------------------------------+
|           The ncurses form library is  Copyright (C) 1995-1997               |
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

#include "form.priv.h"

MODULE_ID("$Id: fld_type.c,v 1.6 1997/10/21 13:24:19 juergen Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_type(FIELD *field, FIELDTYPE *type,...)
|   
|   Description   :  Associate the specified fieldtype with the field.
|                    Certain field types take additional arguments. Look
|                    at the spec of the field types !
|
|   Return Values :  E_OK           - success
|                    E_SYSTEM_ERROR - system error
+--------------------------------------------------------------------------*/
int set_field_type(FIELD *field,FIELDTYPE *type, ...)
{
  va_list ap;
  int res = E_SYSTEM_ERROR;
  int err = 0;

  va_start(ap,type);

  Normalize_Field(field);
  _nc_Free_Type(field);

  field->type = type;
  field->arg  = (void *)_nc_Make_Argument(field->type,&ap,&err);

  if (err)
    {
      _nc_Free_Argument(field->type,(TypeArgument *)(field->arg));
      field->type = (FIELDTYPE *)0;
      field->arg  = (void *)0;
    }
  else
    {
      res = E_OK;
      if (field->type) 
	field->type->ref++;
    }

  va_end(ap);
  RETURN(res);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  FIELDTYPE *field_type(const FIELD *field)
|   
|   Description   :  Retrieve the associated fieldtype for this field.
|
|   Return Values :  Pointer to fieldtype of NULL if none is defined.
+--------------------------------------------------------------------------*/
FIELDTYPE *field_type(const FIELD * field)
{
  return Normalize_Field(field)->type;
}

/* fld_type.c ends here */
