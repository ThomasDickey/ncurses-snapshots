/****************************************************************************
 * Copyright (c) 1998-2005,2009 Free Software Foundation, Inc.              *
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
 *  Author: Thomas E. Dickey          1998-on                               *
 *          Juergen Pfeifer           2009                                  *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: lib_dft_fgbg.c,v 1.19.1.1 2009/02/21 16:22:22 tom Exp $")

/*
 * Modify the behavior of color-pair 0 so that the library doesn't assume that
 * it is white on black.  This is an extension to XSI curses.
 */
NCURSES_EXPORT(int)
NCURSES_SP_NAME(use_default_colors) (NCURSES_SP_DCL0)
{
    T((T_CALLED("use_default_colors(%p)"), SP_PARM));
    returnCode(NCURSES_SP_NAME(assume_default_colors) (NCURSES_SP_ARGx - 1, -1));
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
use_default_colors(void)
{
    return NCURSES_SP_NAME(use_default_colors) (CURRENT_SCREEN);
}
#endif

/*
 * Modify the behavior of color-pair 0 so that the library assumes that it
 * is something specific, possibly not white on black.
 */
NCURSES_EXPORT(int)
NCURSES_SP_NAME(assume_default_colors) (NCURSES_SP_DCLx int fg, int bg)
{
    int code = ERR;
    T((T_CALLED("assume_default_colors(%p,%d,%d)"), sp, fg, bg));
    if (sp != 0)
	code = CallDriver_2(sp, defaultcolors, fg, bg);
    returnCode(code);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
assume_default_colors(int fg, int bg)
{
    return NCURSES_SP_NAME(assume_default_colors) (CURRENT_SCREEN, fg, bg);
}
#endif
