
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
 *		delay_output()
 *		_nc_outch()
 *		tputs()
 *
 */

#include <curses.priv.h>
#include <ctype.h>
#include <term.h>	/* padding_baud_rate, xon_xoff */
#include <tic.h>

MODULE_ID("$Id: lib_tputs.c,v 1.27 1997/11/16 01:33:18 tom Exp $")

#define OUTPUT ((SP != 0) ? SP->_ofp : stdout)

int _nc_nulls_sent;	/* used by 'tack' program */

static int (*my_outch)(int c) = _nc_outch;

int delay_output(int ms)
{
	T((T_CALLED("delay_output(%d)"), ms));

	if (cur_term == 0 || cur_term->_baudrate <= 0) {
		(void) fflush(OUTPUT);
		_nc_timed_wait(0, ms, (int *)0);
	}
#ifdef no_pad_char
	else if (no_pad_char)
		napms(ms);
#endif /* no_pad_char */
	else {
		register int	nullcount;
		char	null = '\0';

#ifdef pad_char
		if (pad_char)
			null = pad_char[0];
#endif /* pad_char */

		nullcount = ms * cur_term->_baudrate / 10000;
		for (_nc_nulls_sent += nullcount; nullcount > 0; nullcount--)
			my_outch(null);
		if (my_outch == _nc_outch)
			(void) fflush(OUTPUT);
	}

	returnCode(OK);
}

int _nc_outch(int ch)
{
#ifdef TRACE
    	_nc_outchars++;
#endif /* TRACE */

	putc(ch, OUTPUT);
	return OK;
}

int putp(const char *string)
{
	return tputs(string, 1, _nc_outch);
}

int tputs(const char *string, int affcnt, int (*outc)(int))
{
bool	always_delay = (string == bell) || (string == flash_screen);
bool	normal_delay =
	 !xon_xoff
#ifdef padding_baud_rate
	 && padding_baud_rate
	 && (!cur_term || cur_term->_baudrate >= padding_baud_rate)
#endif
	 ;
int	number;
#ifdef BSD_TPUTS
int	trailpad;
#endif /* BSD_TPUTS */

#ifdef TRACE
char	addrbuf[17];

	if (_nc_tracing & TRACE_TPUTS)
	{
		if (outc == _nc_outch)
			(void) strcpy(addrbuf, "_nc_outch");
		else
			(void) sprintf(addrbuf, "%p", outc);
		if (_nc_tputs_trace)
			TR(TRACE_MAXIMUM, ("tputs(%s = %s, %d, %s) called", _nc_tputs_trace, _nc_visbuf(string), affcnt, addrbuf));
		else
			TR(TRACE_MAXIMUM, ("tputs(%s, %d, %s) called", _nc_visbuf(string), affcnt, addrbuf));
		_nc_tputs_trace = (char *)NULL;
	}
#endif /* TRACE */

	if (string == ABSENT_STRING || string == CANCELLED_STRING)
		return ERR;

#ifdef BSD_TPUTS
	/*
	 * This ugly kluge deals with the fact that some ancient BSD programs
	 * (like nethack) actually do the likes of tputs("50") to get delays.
	 */
	trailpad = 0;
	while (isdigit(*string)) {
		trailpad = trailpad * 10 + (*string - '0');
		string++;
	}
	trailpad *= 10;
	if (*string == '.') {
		string++;
		if (isdigit(*string)) {
			trailpad += (*string - '0');
			string++;
		}
		while (isdigit(*string))
			string++;
	}

	if (*string == '*') {
		trailpad *= affcnt;
		string++;
	}
#endif /* BSD_TPUTS */

	my_outch = outc;	/* redirect delay_output() */
	while (*string) {
		if (*string != '$')
			(*outc)(*string);
		else {
			string++;
			if (*string != '<') {
				(*outc)('$');
				if (*string)
				    (*outc)(*string);
			} else {
				bool mandatory;

				string++;
				if ((!isdigit(*string) && *string != '.') || !strchr(string, '>')) {
					(*outc)('$');
					(*outc)('<');
					continue;
				}

				number = 0;
				while (isdigit(*string)) {
					number = number * 10 + (*string - '0');
					string++;
				}
				number *= 10;
				if (*string == '.') {
					string++;
					if (isdigit(*string)) {
						number += (*string - '0');
						string++;
					}
					while (isdigit(*string))
						string++;
				}

				mandatory = FALSE;
				while (*string == '*' || *string == '/')
				{
					if (*string == '*') {
						number *= affcnt;
						string++;
					}
					else /* if (*string == '/') */ {
						mandatory = TRUE;
						string++;
					}
				}

				if (number > 0
				 && (always_delay
				  || normal_delay
				  || mandatory))
					delay_output(number/10);

			} /* endelse (*string == '<') */
		} /* endelse (*string == '$') */

		if (*string == '\0')
			break;

		string++;
	}

#ifdef BSD_TPUTS
	/*
	 * Emit any BSD-style prefix padding that we've accumulated now.
	 */
	if (trailpad > 0
	 && (always_delay || normal_delay))
		delay_output(trailpad/10);
#endif /* BSD_TPUTS */

	my_outch = _nc_outch;
	return OK;
}
