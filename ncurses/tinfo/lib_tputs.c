/****************************************************************************
 * Copyright (c) 1998-2008,2009 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	tputs.c
 *		delay_output()
 *		_nc_outch()
 *		tputs()
 *
 */

#include <curses.priv.h>
#define CUR TerminalOf(sp)->type.

#include <ctype.h>
#include <termcap.h>		/* ospeed */
#include <tic.h>

MODULE_ID("$Id: lib_tputs.c,v 1.67.1.1 2009/02/21 19:00:26 tom Exp $")

NCURSES_EXPORT_VAR(char) PC = 0;              /* used by termcap library */
NCURSES_EXPORT_VAR(NCURSES_OSPEED) ospeed = 0;        /* used by termcap library */

NCURSES_EXPORT_VAR(int) _nc_nulls_sent = 0;   /* used by 'tack' program */

#if NCURSES_NO_PADDING
NCURSES_EXPORT(void)
_nc_set_no_padding(SCREEN *sp)
{
    bool no_padding = (getenv("NCURSES_NO_PADDING") != 0);

    if (sp)
	sp->_no_padding = no_padding;
    else
	_nc_prescreen._no_padding = no_padding;

    TR(TRACE_CHARPUT | TRACE_MOVE, ("padding will%s be used",
				    GetNoPadding(sp) ? " not" : ""));
}
#endif

NCURSES_EXPORT(int)
NCURSES_SP_NAME(delay_output) (NCURSES_SP_DCLx int ms)
{
    T((T_CALLED("delay_output(%p,%d)"), SP_PARM, ms));

    if (!HasTInfoTerminal(SP_PARM))
	returnCode(ERR);

    if (no_pad_char) {
	NCURSES_SP_NAME(_nc_flush) (SP_PARM);
	napms(ms);
    } else {
	register int nullcount;

	nullcount = (ms * _nc_baudrate(ospeed)) / (BAUDBYTE * 1000);
	for (_nc_nulls_sent += nullcount; nullcount > 0; nullcount--)
	    SP_PARM->_outch(SP_PARM, PC);
	if (SP_PARM->_outch == NCURSES_SP_NAME(_nc_outch))
	    NCURSES_SP_NAME(_nc_flush) (SP_PARM);
    }

    returnCode(OK);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
delay_output(int ms)
{
    return NCURSES_SP_NAME(delay_output) (CURRENT_SCREEN, ms);
}
#endif

NCURSES_EXPORT(void)
NCURSES_SP_NAME(_nc_flush) (SCREEN *sp)
{
    (void) fflush(NC_OUTPUT(sp));
}

NCURSES_EXPORT(void)
_nc_flush(void)
{
    NCURSES_SP_NAME(_nc_flush) (CURRENT_SCREEN);
}

NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_outch) (SCREEN *sp, int ch)
{
    COUNT_OUTCHARS(1);

    if (HasTInfoTerminal(sp)
	&& sp->_cleanup) {
	char tmp = ch;
	/*
	 * POSIX says write() is safe in a signal handler, but the
	 * buffered I/O is not.
	 */
	write(fileno(NC_OUTPUT(sp)), &tmp, 1);
    } else {
	putc(ch, NC_OUTPUT(sp));
    }
    return OK;
}

NCURSES_EXPORT(int)
_nc_outch(int ch)
{
    return NCURSES_SP_NAME(_nc_outch) (CURRENT_SCREEN, ch);
}

NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_putp) (SCREEN *sp, const char *string)
{
    return NCURSES_SP_NAME(_nc_tputs) (sp, string, 1, NCURSES_SP_NAME(_nc_outch));
}

NCURSES_EXPORT(int)
putp(const char *string)
{
    return NCURSES_SP_NAME(_nc_putp) (CURRENT_SCREEN, string);
}

NCURSES_EXPORT(int)
NCURSES_SP_NAME(_nc_tputs) (NCURSES_SP_DCLx
			    const char *string,
			    int affcnt,
			    int (*outc) (SCREEN *, int))
{
    bool always_delay;
    bool normal_delay;
    int number;
#if BSD_TPUTS
    int trailpad;
#endif /* BSD_TPUTS */

#ifdef TRACE
    char addrbuf[32];

    if (USE_TRACEF(TRACE_TPUTS)) {
	if (outc == NCURSES_SP_NAME(_nc_outch))
	    (void) strcpy(addrbuf, "_nc_outch");
	else
	    (void) sprintf(addrbuf, "%p", outc);
	if (_nc_tputs_trace) {
	    _tracef("tputs(%s = %s, %d, %s) called", _nc_tputs_trace,
		    _nc_visbuf(string), affcnt, addrbuf);
	} else {
	    _tracef("tputs(%s, %d, %s) called", _nc_visbuf(string), affcnt, addrbuf);
	}
	TPUTS_TRACE(NULL);
	_nc_unlock_global(tracef);
    }
#endif /* TRACE */

    if (SP_PARM != 0 && !HasTInfoTerminal(SP_PARM))
	return ERR;

    if (!VALID_STRING(string))
	return ERR;

    if (SP_PARM != 0 && SP_PARM->_term == 0) {
	always_delay = FALSE;
	normal_delay = TRUE;
    } else {
	always_delay = (string == bell) || (string == flash_screen);
	normal_delay =
	    !xon_xoff
	    && padding_baud_rate
#if NCURSES_NO_PADDING
	    && !GetNoPadding(SP_PARM)
#endif
	    && (_nc_baudrate(ospeed) >= padding_baud_rate);
    }

#if BSD_TPUTS
    /*
     * This ugly kluge deals with the fact that some ancient BSD programs
     * (like nethack) actually do the likes of tputs("50") to get delays.
     */
    trailpad = 0;
    if (isdigit(UChar(*string))) {
	while (isdigit(UChar(*string))) {
	    trailpad = trailpad * 10 + (*string - '0');
	    string++;
	}
	trailpad *= 10;
	if (*string == '.') {
	    string++;
	    if (isdigit(UChar(*string))) {
		trailpad += (*string - '0');
		string++;
	    }
	    while (isdigit(UChar(*string)))
		string++;
	}

	if (*string == '*') {
	    trailpad *= affcnt;
	    string++;
	}
    }
#endif /* BSD_TPUTS */

    SP_PARM->_outch = outc;	/* redirect delay_output() */
    while (*string) {
	if (*string != '$')
	    (*outc) (SP_PARM, *string);
	else {
	    string++;
	    if (*string != '<') {
		(*outc) (SP_PARM, '$');
		if (*string)
		    (*outc) (SP_PARM, *string);
	    } else {
		bool mandatory;

		string++;
		if ((!isdigit(UChar(*string)) && *string != '.')
		    || !strchr(string, '>')) {
		    (*outc) (SP_PARM, '$');
		    (*outc) (SP_PARM, '<');
		    continue;
		}

		number = 0;
		while (isdigit(UChar(*string))) {
		    number = number * 10 + (*string - '0');
		    string++;
		}
		number *= 10;
		if (*string == '.') {
		    string++;
		    if (isdigit(UChar(*string))) {
			number += (*string - '0');
			string++;
		    }
		    while (isdigit(UChar(*string)))
			string++;
		}

		mandatory = FALSE;
		while (*string == '*' || *string == '/') {
		    if (*string == '*') {
			number *= affcnt;
			string++;
		    } else {	/* if (*string == '/') */
			mandatory = TRUE;
			string++;
		    }
		}

		if (number > 0
		    && (always_delay
			|| normal_delay
			|| mandatory))
		    NCURSES_SP_NAME(delay_output) (SP_PARM, number / 10);

	    }			/* endelse (*string == '<') */
	}			/* endelse (*string == '$') */

	if (*string == '\0')
	    break;

	string++;
    }

#if BSD_TPUTS
    /*
     * Emit any BSD-style prefix padding that we've accumulated now.
     */
    if (trailpad > 0
	&& (always_delay || normal_delay))
	delay_output(trailpad / 10);
#endif /* BSD_TPUTS */

    SP_PARM->_outch = NCURSES_SP_NAME(_nc_outch);
    return OK;
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
_nc_outc_wrapper(SCREEN *sp, int c)
{
    if (0 == sp) {
	return (ERR);
    } else {
	return sp->jump(c);
    }
}
#endif

NCURSES_EXPORT(int)
tputs(const char *string, int affcnt, int (*outc) (int))
{
    SetSafeOutcWrapper(outc);
    return NCURSES_SP_NAME(_nc_tputs) (NCURSES_SP_ARGx string, affcnt, _nc_outc_wrapper);
}
