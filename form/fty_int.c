
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
  int precision;
  int low;
  int high;
} integerARG;

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Make_Integer_Type( va_list * ap )
|   
|   Description   :  Allocate structure for integer type argument.
|
|   Return Values :  Pointer to argument structure or NULL on error
+--------------------------------------------------------------------------*/
static void *Make_Integer_Type(va_list * ap)
{
  integerARG *argp = (integerARG *)malloc(sizeof(integerARG));

  if (argp)
    {
      argp->precision = va_arg(*ap,int);
      argp->low       = va_arg(*ap,int);
      argp->high      = va_arg(*ap,int);
    }
  return (void *)argp;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Copy_Integer_Type(const void * argp)
|   
|   Description   :  Copy structure for integer type argument.  
|
|   Return Values :  Pointer to argument structure or NULL on error.
+--------------------------------------------------------------------------*/
static void *Copy_Integer_Type(const void * argp)
{
  integerARG *ap  = (integerARG *)argp;
  integerARG *new = (integerARG *)0;

  if (argp)
    {
      new = (integerARG *)malloc(sizeof(integerARG));
      if (new)
	*new = *ap;
    }
  return (void *)new;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void Free_Integer_Type(void * argp)
|   
|   Description   :  Free structure for integer type argument.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
static void Free_Integer_Type(void * argp)
{
  if (argp) 
    free(argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_Integer_Field(
|                                                    FIELD * field,
|                                                    const void * argp)
|   
|   Description   :  Validate buffer content to be a valid integer value
|
|   Return Values :  TRUE  - field is valid
|                    FALSE - field is invalid
+--------------------------------------------------------------------------*/
static bool Check_Integer_Field(FIELD * field, const void * argp)
{
  integerARG *argi  = (integerARG *)argp;
  int low           = argi->low;
  int high          = argi->high;
  int prec          = argi->precision;
  unsigned char *bp = (unsigned char *)field_buffer(field,0);
  char *s           = (char *)bp;
  long val;
  char buf[100];

  while( *bp && *bp==' ') bp++;
  if (*bp)
    {
      if (*bp=='-') bp++;
      while (*bp)
	{
	  if (!isdigit(*bp)) break;
	  bp++;
	}
      while(*bp && *bp==' ') bp++;
      if (*bp=='\0')
	{
	  val = atol(s);
	  if (low<high)
	    {
	      if (val<low || val>high) return FALSE;
	    }
	  sprintf(buf,"%.*ld",prec,val);
	  set_field_buffer(field,0,buf);
	  return TRUE;
	}
    }
  return FALSE;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_Integer_Character(
|                                      int c,
|                                      const void * argp)
|   
|   Description   :  Check a character for the integer type.
|
|   Return Values :  TRUE  - character is valid
|                    FALSE - character is invalid
+--------------------------------------------------------------------------*/
static bool Check_Integer_Character(int c, const void * argp)
{
  return ((isdigit(c) || (c=='-')) ? TRUE : FALSE);
}

static FIELDTYPE typeINTEGER = {
  _HAS_ARGS | _RESIDENT,
  1,
  (FIELDTYPE *)0,
  (FIELDTYPE *)0,
  Make_Integer_Type,
  Copy_Integer_Type,
  Free_Integer_Type,
  Check_Integer_Field,
  Check_Integer_Character,
  NULL,
  NULL
};

FIELDTYPE* TYPE_INTEGER = &typeINTEGER;

/* fty_int.c ends here */
