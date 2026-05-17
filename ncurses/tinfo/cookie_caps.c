/****************************************************************************
 * Copyright 2026 Thomas E. Dickey                                          *
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

#include <curses.priv.h>
#include <tic.h>

#ifndef CUR
#define CUR SP_TERMTYPE
#endif


MODULE_ID("$Id: cookie_caps.c,v 1.4 2026/05/16 23:42:24 tom Exp $")

NCURSES_EXPORT(void)
_nc_cookie_init(SCREEN *sp)
{
    bool support_cookies = USE_XMC_SUPPORT;
    if (sp == NULL || !IsTermInfo(sp))
	return;
    if (sp->cookie_initialized)
	return;

#if USE_XMC_SUPPORT
    /*
     * If we have no magic-cookie support compiled-in, or if it is suppressed
     * in the environment, reset the support-flag.
     */
    if (magic_cookie_glitch >= 0) {
	if (getenv("NCURSES_NO_MAGIC_COOKIE") != NULL) {
	    support_cookies = FALSE;
	}
    }
#endif

    if (!support_cookies && magic_cookie_glitch >= 0) {
	T(("will disable attributes to work w/o magic cookies"));
    }

    if (magic_cookie_glitch > 0) {	/* tvi, wyse */

	sp->_xmc_triggers = sp->_ok_attributes & XMC_CONFLICT;
#if 0
	/*
	 * We "should" treat colors as an attribute.  The wyse350 (and its
	 * clones) appear to be the only ones that have both colors and magic
	 * cookies.
	 */
	if (has_colors()) {
	    sp->_xmc_triggers |= A_COLOR;
	}
#endif
	sp->_xmc_suppress = sp->_xmc_triggers & (chtype) ~(A_BOLD);

	T(("magic cookie attributes %s", _traceattr(sp->_xmc_suppress)));
	/*
	 * Supporting line-drawing may be possible.  But make the regular
	 * video attributes work first.
	 */
	acs_chars = ABSENT_STRING;
	ena_acs = ABSENT_STRING;
	enter_alt_charset_mode = ABSENT_STRING;
	exit_alt_charset_mode = ABSENT_STRING;
#if USE_XMC_SUPPORT
	/*
	 * To keep the cookie support simple, suppress all of the optimization
	 * hooks except for clear_screen and the cursor addressing.
	 */
	if (support_cookies) {
	    clr_eol = ABSENT_STRING;
	    clr_eos = ABSENT_STRING;
	    set_attributes = ABSENT_STRING;
	}
#endif
    } else if (magic_cookie_glitch == 0) {	/* hpterm */
    }

    /*
     * If magic cookies are not supported, cancel the strings that set
     * video attributes.
     */
    if (!support_cookies && magic_cookie_glitch >= 0) {
	/* *INDENT-EQLS* */
	magic_cookie_glitch  = ABSENT_NUMERIC;
	set_attributes       = ABSENT_STRING;
	enter_blink_mode     = ABSENT_STRING;
	enter_bold_mode      = ABSENT_STRING;
	enter_dim_mode       = ABSENT_STRING;
	enter_reverse_mode   = ABSENT_STRING;
	enter_standout_mode  = ABSENT_STRING;
	enter_underline_mode = ABSENT_STRING;
    }

    /* initialize normal acs before wide, since we use mapping in the latter */
#if !USE_WIDEC_SUPPORT
    if (_nc_unicode_locale() && _nc_locale_breaks_acs(sp->_term)) {
	/* *INDENT-EQLS* */
	acs_chars              = NULL;
	ena_acs                = NULL;
	enter_alt_charset_mode = NULL;
	exit_alt_charset_mode  = NULL;
	set_attributes         = NULL;
    }
#endif
    sp->cookie_initialized = TRUE;
}

/*
 * Check if the current terminal entry says to expect magic cookies, and if so,
 * whether there are any valid capabilities by which the cookies would be made.
 */
NCURSES_EXPORT(bool)
_nc_cookie_allowed(const SCREEN *sp)
{
    bool result = FALSE;
    if (sp != NULL && IsTermInfo(sp) &&
	magic_cookie_glitch >= 0 &&
	(VALID_STRING(set_attributes) ||
	 VALID_STRING(enter_blink_mode) ||
	 VALID_STRING(enter_bold_mode) ||
	 VALID_STRING(enter_dim_mode) ||
	 VALID_STRING(enter_reverse_mode) ||
	 VALID_STRING(enter_standout_mode) ||
	 VALID_STRING(enter_underline_mode))) {
	result = TRUE;
    }
    return result;
}
