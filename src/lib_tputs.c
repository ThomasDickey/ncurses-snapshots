

/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992, 1993, 1994                 *
*                          by Zeyd M. Ben-Halim                            *
*                          zmbenhal@netcom.com                             *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, not removed   *
*        from header files, and is reproduced in any documentation         *
*        accompanying it or the applications linked with it.               *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/


/*
 *	tputs.c
 *
 */

#include <string.h>
#include <ctype.h>
#include "curses.priv.h"
#include "terminfo.h"	/* padding_baud_rate, xon_xoff */

int putp(char *string)
{
	return tputs(string, 1, _outch);
}

int tputs(char *string, int affcnt, int (*outc)(char))
{
float	number;

	TR(TRACE_MAXIMUM, ("tputs(\"%s\", %d, %o) called", visbuf(string), affcnt, outc));

	if (string == NULL)
		return ERR;

	while (*string) {
	    	if (*string != '$')
			(*outc)(*string);
	    	else {
			string++;
			if (*string != '<') {
			    	(*outc)('$');
			    	(*outc)(*string);
			} else {

			    	number = 0;
			    	string++;

			    	if ((!isdigit(*string) && *string != '.') || !strchr(string, '>')) {
					(*outc)('$');
					(*outc)('<');
					continue;
			    	}
			    	while (isdigit(*string)) {
					number = number * 10 + *string - '0';
					string++;
			    	}

			    	if (*string == '.') {
					string++;
					if (isdigit(*string)) {
					    	number += (float) (*string - '0') / 10.;
					    	string++;
					}
			    	}

			    	if (*string == '*') {
					number *= affcnt;
					string++;
			    	}

#ifdef padding_baud_rate
			    	if (padding_baud_rate && SP->_baudrate >= padding_baud_rate && !xon_xoff)
					delay_output(number);
#endif /* padding_baud_rate */

			} /* endelse (*string == '<') */
		} /* endelse (*string == '$') */

		if (*string == '\0')
			break;

		string++;
	}
	return OK;
}

