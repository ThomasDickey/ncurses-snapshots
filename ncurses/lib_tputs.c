
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


/*
 *	tputs.c
 *
 */

#include "curses.priv.h"
#include <string.h>
#include <ctype.h>
#include "term.h"	/* padding_baud_rate, xon_xoff */

int putp(char *string)
{
	return tputs(string, 1, _nc_outch);
}

int tputs(const char *string, int affcnt, int (*outc)(int))
{
float	number;

#ifdef TRACE
	if (_nc_tputs_trace)
		TR(TRACE_MAXIMUM, ("tputs(%s = \"%s\", %d, %p) called", _nc_tputs_trace, _nc_visbuf(string), affcnt, outc));
	else

		TR(TRACE_MAXIMUM, ("tputs(\"%s\", %d, %p) called", _nc_visbuf(string), affcnt, outc));
	_nc_tputs_trace = (char *)NULL;
#endif /* TRACE */

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
			    	if (!xon_xoff && padding_baud_rate && (!SP || SP->_baudrate >= padding_baud_rate))
					delay_output((float)number);
#endif /* padding_baud_rate */

			} /* endelse (*string == '<') */
		} /* endelse (*string == '$') */

		if (*string == '\0')
			break;

		string++;
	}
	return OK;
}

