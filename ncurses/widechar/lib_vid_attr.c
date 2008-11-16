/****************************************************************************
 * Copyright (c) 2002-2006,2007 Free Software Foundation, Inc.              *
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
#define CUR TerminalOf(sp)->type.

MODULE_ID("$Id: lib_vid_attr.c,v 1.5.1.1 2008/11/16 00:19:59 juergen Exp $")

#define doPut(mode) TPUTS_TRACE(#mode); NC_SNAME(_nc_tputs)(sp, mode, 1, outc)

#define TurnOn(mask,mode) \
	if ((turn_on & mask) && mode) { doPut(mode); }

#define TurnOff(mask,mode) \
	if ((turn_off & mask) && mode) { doPut(mode); turn_off &= ~mask; }

	/* if there is no current screen, assume we *can* do color */
#define SetColorsIf(why, old_attr, old_pair) \
	if (can_color && (why)) { \
		TR(TRACE_ATTRS, ("old pair = %d -- new pair = %d", old_pair, pair)); \
		if ((pair != old_pair) \
		 || (fix_pair0 && (pair == 0)) \
		 || (reverse ^ ((old_attr & A_REVERSE) != 0))) { \
		  NC_SNAME(_nc_do_color)(sp,old_pair, pair, reverse, outc); \
		} \
	}

#define set_color(mode, pair) mode &= ALL_BUT_COLOR; mode |= COLOR_PAIR(pair)

NCURSES_EXPORT(int)
NC_SNAME(_nc_vid_puts)(SCREEN *sp,
		       attr_t newmode,
		       short pair,
		       void *opts GCC_UNUSED,
		       int (*outc) (SCREEN *, int))
{
#if NCURSES_EXT_COLORS
    static attr_t previous_attr = A_NORMAL;
    static int previous_pair = 0;

    attr_t turn_on, turn_off;
    bool reverse = FALSE;
    bool can_color = (sp == 0 || sp->_coloron);
#if NCURSES_EXT_FUNCS
    bool fix_pair0 = (sp != 0 && sp->_coloron && !sp->_default_color);
#else
#define fix_pair0 FALSE
#endif

    newmode &= A_ATTRIBUTES;
    T((T_CALLED("vid_puts(%s,%d)"), _traceattr(newmode), pair));

    /* this allows us to go on whether or not newterm() has been called */
    if (sp) {
	previous_attr = AttrOf(SCREEN_ATTRS(sp));
	previous_pair = GetPair(SCREEN_ATTRS(sp));
    }

    TR(TRACE_ATTRS, ("previous attribute was %s, %d",
		     _traceattr(previous_attr), previous_pair));

#if !USE_XMC_SUPPORT
    if ((sp != 0)
	&& (magic_cookie_glitch > 0))
	newmode &= ~(sp->_xmc_suppress);
#endif

    /*
     * If we have a terminal that cannot combine color with video
     * attributes, use the colors in preference.
     */
    if ((pair != 0
	 || fix_pair0)
	&& (no_color_video > 0)) {
	/*
	 * If we had chosen the A_xxx definitions to correspond to the
	 * no_color_video mask, we could simply shift it up and mask off the
	 * attributes.  But we did not (actually copied Solaris' definitions).
	 * However, this is still simpler/faster than a lookup table.
	 *
	 * The 63 corresponds to A_STANDOUT, A_UNDERLINE, A_REVERSE, A_BLINK,
	 * A_DIM, A_BOLD which are 1:1 with no_color_video.  The bits that
	 * correspond to A_INVIS, A_PROTECT (192) must be shifted up 1 and
	 * A_ALTCHARSET (256) down 2 to line up.  We use the NCURSES_BITS
	 * macro so this will work properly for the wide-character layout.
	 */
	unsigned value = no_color_video;
	attr_t mask = NCURSES_BITS((value & 63)
				   | ((value & 192) << 1)
				   | ((value & 256) >> 2), 8);

	if ((mask & A_REVERSE) != 0
	    && (newmode & A_REVERSE) != 0) {
	    reverse = TRUE;
	    mask &= ~A_REVERSE;
	}
	newmode &= ~mask;
    }

    if (newmode == previous_attr
	&& pair == previous_pair)
	returnCode(OK);

    if (reverse) {
	newmode &= ~A_REVERSE;
    }

    turn_off = (~newmode & previous_attr) & ALL_BUT_COLOR;
    turn_on = (newmode & ~previous_attr) & ALL_BUT_COLOR;

    SetColorsIf(((pair == 0) && !fix_pair0), previous_attr, previous_pair);

    if (newmode == A_NORMAL) {
	if ((previous_attr & A_ALTCHARSET) && exit_alt_charset_mode) {
	    doPut(exit_alt_charset_mode);
	    previous_attr &= ~A_ALTCHARSET;
	}
	if (previous_attr) {
	    if (exit_attribute_mode) {
		doPut(exit_attribute_mode);
	    } else {
		if (!sp || sp->_use_rmul) {
		    TurnOff(A_UNDERLINE, exit_underline_mode);
		}
		if (!sp || sp->_use_rmso) {
		    TurnOff(A_STANDOUT, exit_standout_mode);
		}
	    }
	    previous_attr &= ALL_BUT_COLOR;
	    previous_pair = 0;
	}

	SetColorsIf((pair != 0) || fix_pair0, previous_attr, previous_pair);
    } else if (set_attributes) {
	if (turn_on || turn_off) {
	    TPUTS_TRACE("set_attributes");
	    NC_SNAME(_nc_tputs)(sp,
				TPARM_9(set_attributes,
					(newmode & A_STANDOUT) != 0,
					(newmode & A_UNDERLINE) != 0,
					(newmode & A_REVERSE) != 0,
					(newmode & A_BLINK) != 0,
					(newmode & A_DIM) != 0,
					(newmode & A_BOLD) != 0,
					(newmode & A_INVIS) != 0,
					(newmode & A_PROTECT) != 0,
					(newmode & A_ALTCHARSET) != 0), 1, outc);
	    previous_attr &= ALL_BUT_COLOR;
	    previous_pair = 0;
	}
	SetColorsIf((pair != 0) || fix_pair0, previous_attr, previous_pair);
    } else {

	TR(TRACE_ATTRS, ("turning %s off", _traceattr(turn_off)));

	TurnOff(A_ALTCHARSET, exit_alt_charset_mode);

	if (!sp || sp->_use_rmul) {
	    TurnOff(A_UNDERLINE, exit_underline_mode);
	}

	if (!sp || sp->_use_rmso) {
	    TurnOff(A_STANDOUT, exit_standout_mode);
	}

	if (turn_off && exit_attribute_mode) {
	    doPut(exit_attribute_mode);
	    turn_on |= (newmode & ALL_BUT_COLOR);
	    previous_attr &= ALL_BUT_COLOR;
	    previous_pair = 0;
	}
	SetColorsIf((pair != 0) || fix_pair0, previous_attr, previous_pair);

	TR(TRACE_ATTRS, ("turning %s on", _traceattr(turn_on)));
	/* *INDENT-OFF* */
	TurnOn(A_ALTCHARSET,	enter_alt_charset_mode);
	TurnOn(A_BLINK,		enter_blink_mode);
	TurnOn(A_BOLD,		enter_bold_mode);
	TurnOn(A_DIM,		enter_dim_mode);
	TurnOn(A_REVERSE,	enter_reverse_mode);
	TurnOn(A_STANDOUT,	enter_standout_mode);
	TurnOn(A_PROTECT,	enter_protected_mode);
	TurnOn(A_INVIS,		enter_secure_mode);
	TurnOn(A_UNDERLINE,	enter_underline_mode);
#if USE_WIDEC_SUPPORT
	TurnOn(A_HORIZONTAL,	enter_horizontal_hl_mode);
	TurnOn(A_LEFT,		enter_left_hl_mode);
	TurnOn(A_LOW,		enter_low_hl_mode);
	TurnOn(A_RIGHT,		enter_right_hl_mode);
	TurnOn(A_TOP,		enter_top_hl_mode);
	TurnOn(A_VERTICAL,	enter_vertical_hl_mode);
#endif
	/* *INDENT-ON* */

    }

    if (reverse)
	newmode |= A_REVERSE;

    if (sp) {
	SetAttr(SCREEN_ATTRS(sp), newmode);
	SetPair(SCREEN_ATTRS(sp), pair);
    } else {
	previous_attr = newmode;
	previous_pair = pair;
    }

    returnCode(OK);
#else
    T((T_CALLED("vid_puts(%s,%d)"), _traceattr(newmode), pair));
    set_color(newmode, pair);
    returnCode(NC_SNAME(_nc_vidputs)(sp, newmode, outc));
#endif
}

NCURSES_EXPORT(int)
vid_puts (attr_t newmode,
	  short pair,
	  void *opts GCC_UNUSED,
	  int (*outc) (int))
{
    SetSafeOutcWrapper(outc);
    return NC_SNAME(_nc_vid_puts)(CURRENT_SCREEN, newmode, pair, opts, _nc_outc_wrapper);
}

#undef vid_attr
NCURSES_EXPORT(int)
NC_SNAME(_nc_vid_attr)(SCREEN *sp, attr_t newmode, short pair, void *opts)
{
    T((T_CALLED("vid_attr(%s,%d)"), _traceattr(newmode), pair));
    returnCode(NC_SNAME(_nc_vid_puts)(sp, newmode, pair, opts, NC_SNAME(_nc_outch)));
}

NCURSES_EXPORT(int)
vid_attr (attr_t newmode, short pair, void *opts)
{
    return NC_SNAME(_nc_vid_attr)(CURRENT_SCREEN, newmode, pair, opts);
}

/*
 * This implementation uses the same mask values for A_xxx and WA_xxx, so
 * we can use termattrs() for part of the logic.
 */
NCURSES_EXPORT(attr_t)
NC_SNAME(term_attrs)(SCREEN *sp)
{
    attr_t attrs;

    T((T_CALLED("term_attrs()")));
    attrs = sp ? NC_SNAME(termattrs)(sp) : 0;

    /* these are only supported for wide-character mode */
    if (enter_horizontal_hl_mode)
	attrs |= WA_HORIZONTAL;
    if (enter_left_hl_mode)
	attrs |= WA_LEFT;
    if (enter_low_hl_mode)
	attrs |= WA_LOW;
    if (enter_right_hl_mode)
	attrs |= WA_RIGHT;
    if (enter_top_hl_mode)
	attrs |= WA_TOP;
    if (enter_vertical_hl_mode)
	attrs |= WA_VERTICAL;

    returnAttr(attrs);
}

NCURSES_EXPORT(attr_t)
term_attrs (void)
{
    return NC_SNAME(term_attrs)(CURRENT_SCREEN);
}
