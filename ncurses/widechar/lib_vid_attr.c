/****************************************************************************
 * Copyright (c) 2002,2004 Free Software Foundation, Inc.                   *
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
#include <term.h>

MODULE_ID("$Id: lib_vid_attr.c,v 1.2 2004/10/23 20:43:55 tom Exp $")

#define set_color(mode, pair) mode &= ~A_COLOR; mode |= COLOR_PAIR(pair)

NCURSES_EXPORT(int)
vid_puts(attr_t newmode, short pair, void *opts GCC_UNUSED, int (*outc) (int))
{
    T((T_CALLED("vid_puts(%s,%d)"), _traceattr(newmode), pair));
    set_color(newmode, pair);
    returnCode(vidputs(newmode, outc));
}

#undef vid_attr
NCURSES_EXPORT(int)
vid_attr(attr_t newmode, short pair, void *opts GCC_UNUSED)
{
    T((T_CALLED("vid_attr(%s,%d)"), _traceattr(newmode), pair));
    set_color(newmode, pair);
    returnCode(vidputs(newmode, _nc_outch));
}

/*
 * This implementation uses the same mask values for A_xxx and WA_xxx, so
 * we can use termattrs() for part of the logic.
 */
NCURSES_EXPORT(attr_t)
term_attrs(void)
{
    attr_t attrs;

    T((T_CALLED("term_attrs()")));
    attrs = termattrs();

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
