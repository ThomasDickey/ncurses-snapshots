
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

#if HAVE_REGEX_H	/* We assume POSIX regex */
#include <regex.h>
#endif

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Make_RegularExpression_Type(va_list * ap)
|   
|   Description   :  Allocate structure for regex type argument.
|
|   Return Values :  Pointer to argument structure or NULL on error
+--------------------------------------------------------------------------*/
static void *Make_RegularExpression_Type(va_list * ap)
{
#if HAVE_REGEX_H
  char *rx = va_arg(*ap,char *);
  regex_t *preg;

  preg = (regex_t*)malloc(sizeof(regex_t));
  if (preg)
    {
      if (regcomp(preg,rx,0))
	{
	  free(preg);
	  preg = (regex_t*)0;
	}
    }
  return((void *)preg);
#else
  return 0;
#endif
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void *Copy_RegularExpression_Type(
|                                      const void * argp)
|   
|   Description   :  Copy structure for regex type argument.  
|
|   Return Values :  Pointer to argument structure or NULL on error.
+--------------------------------------------------------------------------*/
static void *Copy_RegularExpression_Type(const void * argp)
{
#if HAVE_REGEX_H
  char *ap  = (char *)argp;
  char *new = (char *)0; 
  
  if (argp)
    {
      /* FIXME: this cannot possibly work! */
      new = (char *)malloc(1+strlen(ap));
      if (new)
	strcpy(new,ap);
    }
  return (void *)new;
#else
  return 0;
#endif
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void Free_RegularExpression_Type(void * argp)
|   
|   Description   :  Free structure for regex type argument.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
static void Free_RegularExpression_Type(void * argp)
{
  if (argp) 
    free(argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static bool Check_RegularExpression_Field(
|                                      FIELD * field,
|                                      const void  * argp)
|   
|   Description   :  Validate buffer content to be a valid regular expression
|
|   Return Values :  TRUE  - field is valid
|                    FALSE - field is invalid
+--------------------------------------------------------------------------*/
static bool Check_RegularExpression_Field(FIELD * field, const void  * argp)
{
#if HAVE_REGEX_H
  regex_t *preg = (regex_t*)argp;

  return ((regexec(preg,field_buffer(field,0),0,NULL,0)==0) ? TRUE:FALSE);
#else
  return FALSE;
#endif
}

static FIELDTYPE typeREGEXP = {
  _HAS_ARGS | _RESIDENT,
  1,
  (FIELDTYPE *)0,
  (FIELDTYPE *)0,
  Make_RegularExpression_Type,
  Copy_RegularExpression_Type,
  Free_RegularExpression_Type,
  Check_RegularExpression_Field,
  NULL,
  NULL,
  NULL
};

FIELDTYPE* TYPE_REGEXP = &typeREGEXP;

/* fty_regex.c ends here */
