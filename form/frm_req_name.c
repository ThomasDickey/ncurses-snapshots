
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
* Module form_request_name                                                 *
* Routines to handle external names of menu requests                       *
***************************************************************************/

#include "form.priv.h"

static const char *request_names[ MAX_FORM_COMMAND - MIN_FORM_COMMAND + 1 ] = {
  "NEXT_PAGE"	 ,
  "PREV_PAGE"	 ,
  "FIRST_PAGE"	 ,
  "LAST_PAGE"	 ,

  "NEXT_FIELD"	 ,
  "PREV_FIELD"	 ,
  "FIRST_FIELD"	 ,
  "LAST_FIELD"	 ,
  "SNEXT_FIELD"	 ,
  "SPREV_FIELD"	 ,
  "SFIRST_FIELD" ,
  "SLAST_FIELD"	 ,
  "LEFT_FIELD"	 ,
  "RIGHT_FIELD"	 ,
  "UP_FIELD"	 ,
  "DOWN_FIELD"	 ,

  "NEXT_CHAR"	 ,
  "PREV_CHAR"	 ,
  "NEXT_LINE"	 ,
  "PREV_LINE"	 ,
  "NEXT_WORD"	 ,
  "PREV_WORD"	 ,
  "BEG_FIELD"	 ,
  "END_FIELD"	 ,
  "BEG_LINE"	 ,
  "END_LINE"	 ,
  "LEFT_CHAR"	 ,
  "RIGHT_CHAR"	 ,
  "UP_CHAR"	 ,
  "DOWN_CHAR"	 ,

  "NEW_LINE"	 ,
  "INS_CHAR"	 ,
  "INS_LINE"	 ,
  "DEL_CHAR"	 ,
  "DEL_PREV"	 ,
  "DEL_LINE"	 ,
  "DEL_WORD"	 ,
  "CLR_EOL"	 ,
  "CLR_EOF"	 ,
  "CLR_FIELD"	 ,
  "OVL_MODE"	 ,
  "INS_MODE"	 ,
  "SCR_FLINE"	 ,
  "SCR_BLINE"	 ,
  "SCR_FPAGE"	 ,
  "SCR_BPAGE"	 ,
  "SCR_FHPAGE"   ,
  "SCR_BHPAGE"   ,
  "SCR_FCHAR"    ,
  "SCR_BCHAR"    ,
  "SCR_HFLINE"   ,
  "SCR_HBLINE"   ,
  "SCR_HFHALF"   ,
  "SCR_HBHALF"   ,

  "VALIDATION"	 ,
  "NEXT_CHOICE"	 ,
  "PREV_CHOICE"	 
};
#define A_SIZE (sizeof(request_names)/sizeof(request_names[0]))

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  const char * form_request_name (int request);
|   
|   Description   :  Get the external name of a form request.
|
|   Return Values :  Pointer to name      - on success
|                    NULL                 - on invalid request code
+--------------------------------------------------------------------------*/
const char *form_request_name( int request )
{
  if ( (request < MIN_FORM_COMMAND) || (request > MAX_FORM_COMMAND) )
    {
      SET_ERROR (E_BAD_ARGUMENT);
      return (const char *)0;
    }
  else
    return request_names[ request - MIN_FORM_COMMAND ];
}


/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int form_request_by_name (const char *str);
|   
|   Description   :  Search for a request with this name.
|
|   Return Values :  Request Id       - on success
|                    E_NO_MATCH       - request not found
+--------------------------------------------------------------------------*/
int form_request_by_name( const char *str )
{ 
  /* because the table is so small, it doesn't really hurt
     to run sequentially through it.
  */
  unsigned int i = 0;
  char buf[16];
  
  if (str)
    {
      strncpy(buf,str,sizeof(buf));
      while( (i<sizeof(buf)) && (buf[i] != '\0') )
	{
	  buf[i] = toupper(buf[i]);
	  i++;
	}
      
      for (i=0; i < A_SIZE; i++)
	{
	  if (strncmp(request_names[i],buf,sizeof(buf))==0)
	    return MIN_FORM_COMMAND + i;
	} 
    }
  RETURN(E_NO_MATCH);
}

/* frm_req_name.c ends here */
