
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
#include <locale.h>

typedef struct {
  int    precision;
  double low;
  double high;
  struct lconv* L;
} numericARG;

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Make_Numeric_Type(va_list * ap)
|   
|   Description   :  Allocate structure for numeric type argument.
|
|   Return Values :  Pointer to argument structure or NULL on error
+--------------------------------------------------------------------------*/
static void *Make_Numeric_Type(va_list * ap)
{
  numericARG *argn = (numericARG *)malloc(sizeof(numericARG));

  if (argn)
    {
      argn->precision = va_arg(*ap,int);
      argn->low       = va_arg(*ap,double);
      argn->high      = va_arg(*ap,double);
      argn->L         = localeconv();
    }
  return (void *)argn;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Copy_Numeric_Type(const void * argp)
|   
|   Description   :  Copy structure for numeric type argument.  
|
|   Return Values :  Pointer to argument structure or NULL on error.
+--------------------------------------------------------------------------*/
static void *Copy_Numeric_Type(const void * argp)
{
  numericARG *ap  = (numericARG *)argp;
  numericARG *new = (numericARG *)0;

  if (argp)
    {
      new = (numericARG *)malloc(sizeof(numericARG));
      if (new)
	*new  = *ap;
    }
  return (void *)new;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void Free_Numeric_Type(void * argp)
|   
|   Description   :  Free structure for numeric type argument.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
static void Free_Numeric_Type(void * argp)
{
  if (argp) 
    free(argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_Numeric_Field(FIELD * field,
|                                                    const void * argp)
|   
|   Description   :  Validate buffer content to be a valid numeric value
|
|   Return Values :  TRUE  - field is valid
|                    FALSE - field is invalid
+--------------------------------------------------------------------------*/
static bool Check_Numeric_Field(FIELD * field, const void * argp)
{
  numericARG *argn    = (numericARG *)argp;
  double low          = argn->low;
  double high         = argn->high;
  int prec            = argn->precision;
  unsigned char *bp   = (unsigned char *)field_buffer(field,0);
  char *s             = (char *)bp;
  double val          = 0.0;
  struct lconv* L     = argn->L;
  char buf[64];

  while(*bp && *bp==' ') bp++;
  if (*bp)
    {
      if (*bp=='-' || *bp=='+')
	bp++;
      while(*bp)
	{
	  if (!isdigit(*bp)) break;
	  bp++;
	}
      if (*bp==((L && L->decimal_point) ? *(L->decimal_point) : '.'))
	{
	  bp++;
	  while(*bp)
	    {
	      if (!isdigit(*bp)) break;
	      bp++;
	    }
	}
      while(*bp && *bp==' ') bp++;
      if (*bp=='\0')
	{
	  val = atof(s);
	  if (low<high)
	    {
	      if (val<low || val>high) return FALSE;
	    }
	  sprintf(buf,"%.*f",prec,val);
	  set_field_buffer(field,0,buf);
	  return TRUE;
	}
    }
  return FALSE;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_Numeric_Character(
|                                      int c,
|                                      const void * argp)
|   
|   Description   :  Check a character for the numeric type.
|
|   Return Values :  TRUE  - character is valid
|                    FALSE - character is invalid
+--------------------------------------------------------------------------*/
static bool Check_Numeric_Character(int c, const void * argp)
{
  numericARG *argn = (numericARG *)argp;
  struct lconv* L  = argn->L;  

  return (isdigit(c)  || 
	  c == '+'    || 
	  c == '-'    || 
	  c == ((L && L->decimal_point) ? *(L->decimal_point) : '.')
	 ) ? TRUE : FALSE;
}

static FIELDTYPE const typeNUMERIC = {
  _HAS_ARGS | _RESIDENT,
  1,
  (FIELDTYPE *)0,
  (FIELDTYPE *)0,
  Make_Numeric_Type,
  Copy_Numeric_Type,
  Free_Numeric_Type,
  Check_Numeric_Field,
  Check_Numeric_Character,
  NULL,
  NULL
};

FIELDTYPE const * TYPE_NUMERIC = &typeNUMERIC;

/* fty_num.c ends here */
