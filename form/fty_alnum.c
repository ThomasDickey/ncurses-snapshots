
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

#include "form.priv.h"

typedef struct {
  int width;
} alnumARG;

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Make_AlphaNumeric_Type(va_list *ap)
|   
|   Description   :  Allocate structure for alphanumeric type argument.
|
|   Return Values :  Pointer to argument structure or NULL on error
+--------------------------------------------------------------------------*/
static void *Make_AlphaNumeric_Type(va_list * ap)
{
  alnumARG *argp = (alnumARG *)malloc(sizeof(alnumARG));

  if (argp)
    argp->width = va_arg(*ap,int);

  return ((void *)argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Copy_AlphaNumericType(const void *argp)
|   
|   Description   :  Copy structure for alphanumeric type argument.  
|
|   Return Values :  Pointer to argument structure or NULL on error.
+--------------------------------------------------------------------------*/
static void *Copy_AlphaNumeric_Type(const void *argp)
{
  alnumARG *ap  = (alnumARG *)argp;
  alnumARG *new = (alnumARG *)malloc(sizeof(alnumARG));

  if (new)
    *new = *ap;

  return ((void *)new);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void Free_AlphaNumeric_Type(void *argp)
|   
|   Description   :  Free structure for alphanumeric type argument.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
static void Free_AlphaNumeric_Type(void * argp)
{
  if (argp) 
    free(argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_AlphaNumeric_Field(
|                                      FIELD *field,
|                                      const void *argp)
|   
|   Description   :  Validate buffer content to be a valid alphanumeric value
|
|   Return Values :  TRUE  - field is valid
|                    FALSE - field is invalid
+--------------------------------------------------------------------------*/
static bool Check_AlphaNumeric_Field(FIELD * field, const void * argp)
{
  int width = ((alnumARG *)argp)->width;
  unsigned char *bp  = (unsigned char *)field_buffer(field,0);
  int  l = -1;
  unsigned char *s;

  while(*bp && *bp==' ') 
    bp++;
  if (*bp)
    {
      s = bp;
      while(*bp && isalnum(*bp)) 
	bp++;
      l = (int)(bp-s);
      while(*bp && *bp==' ') 
	bp++;
    }
  return ((*bp || (l < width)) ? FALSE : TRUE);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_AlphaNumeric_Character(
|                                      int c, 
|                                      const void *argp )
|   
|   Description   :  Check a character for the alphanumeric type.
|
|   Return Values :  TRUE  - character is valid
|                    FALSE - character is invalid
+--------------------------------------------------------------------------*/
static bool Check_AlphaNumeric_Character(int c, const void * argp)
{
  return (isalnum(c) ? TRUE : FALSE);
}

static FIELDTYPE const typeALNUM = {
  _HAS_ARGS | _RESIDENT,
  1,
  (FIELDTYPE *)0,
  (FIELDTYPE *)0,
  Make_AlphaNumeric_Type,
  Copy_AlphaNumeric_Type,
  Free_AlphaNumeric_Type,
  Check_AlphaNumeric_Field,
  Check_AlphaNumeric_Character,
  NULL,
  NULL
};

FIELDTYPE const * TYPE_ALNUM = &typeALNUM;

/* fty_alnum.c ends here */
