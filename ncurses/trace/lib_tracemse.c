/****************************************************************************
 * Copyright 2020-2024,2026 Thomas E. Dickey                                *
 * Copyright 1998-2012,2014 Free Software Foundation, Inc.                  *
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
 *	lib_tracemse.c - Tracing/Debugging routines (mouse events)
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_tracemse.c,v 1.27 2026/10/10 18:07:15 tom Exp $")

#ifdef TRACE

#define my_buffer sp->tracemse_buf

NCURSES_EXPORT(char *)
_nc_trace_mmask_t(SCREEN *sp, mmask_t code)
{
    int button;

#define BUTTON_MASK(name) NCURSES_MOUSE_MASK(button, name)

#define SHOW_BUTTON(name, show) \
    if ((code & BUTTON_MASK(name)) == BUTTON_MASK(name)) { \
	char my_button[10]; \
	size_t n = strlen(my_buffer); \
	sprintf(my_button, "-%d", button); \
	if (n && (my_buffer[n-1] != L_BRACE)) \
	_nc_STRCAT(my_buffer, ", ", sizeof(my_buffer)); \
	_nc_STRCAT(my_buffer, show, sizeof(my_buffer)); \
	_nc_STRCAT(my_buffer, my_button, sizeof(my_buffer)); \
    }

#define SHOW(name, show) \
    if ((code & name) == name) { \
	size_t n = strlen(my_buffer); \
	if (n && (my_buffer[n-1] != L_BRACE)) \
	_nc_STRCAT(my_buffer, ", ", sizeof(my_buffer)); \
	_nc_STRCAT(my_buffer, show, sizeof(my_buffer)); \
    }

    *my_buffer = '\0';
    for (button = 1; button <= MAX_BUTTON; ++button) {
	SHOW_BUTTON(NCURSES_BUTTON_PRESSED, "press");
	SHOW_BUTTON(NCURSES_BUTTON_RELEASED, "release");
	SHOW_BUTTON(NCURSES_BUTTON_CLICKED, "clicked");
	SHOW_BUTTON(NCURSES_DOUBLE_CLICKED, "doubleclick");
	SHOW_BUTTON(NCURSES_TRIPLE_CLICKED, "tripleclick");
#if NCURSES_MOUSE_VERSION == 1
	SHOW_BUTTON(NCURSES_RESERVED_EVENT, "reserved");
#endif
    }

    SHOW(BUTTON_CTRL, "ctrl");
    SHOW(BUTTON_SHIFT, "shift");
    SHOW(BUTTON_ALT, "alt");
    SHOW(ALL_MOUSE_EVENTS, "all-events");
    SHOW(REPORT_MOUSE_POSITION, "position");

#undef SHOW

    if (my_buffer[strlen(my_buffer) - 1] == ' ')
	my_buffer[strlen(my_buffer) - 2] = '\0';

    return (my_buffer);
}

NCURSES_EXPORT(char *)
_nc_tracemouse(SCREEN *sp, MEVENT const *ep)
{
    char *result = NULL;

    if (sp != NULL) {
	_nc_SPRINTF(my_buffer, _nc_SLIMIT(sizeof(my_buffer))
		    TRACEMSE_FMT,
		    ep->id,
		    ep->x,
		    ep->y,
		    ep->z,
		    (unsigned long) ep->bstate);

	(void) _nc_trace_mmask_t(sp, ep->bstate);
	_nc_STRCAT(my_buffer, "}", sizeof(my_buffer));
	result = (my_buffer);
    }
    return result;
}

NCURSES_EXPORT(mmask_t)
_nc_retrace_mmask_t(SCREEN *sp, mmask_t code)
{
    if (sp != NULL) {
	*my_buffer = '\0';
	T((T_RETURN("{%s}"), _nc_trace_mmask_t(sp, code)));
    } else {
	T((T_RETURN("{?}")));
    }
    return code;
}

NCURSES_EXPORT(char *)
_tracemouse(MEVENT const *ep)
{
    return _nc_tracemouse(CURRENT_SCREEN, ep);
}

#else /* !TRACE */
EMPTY_MODULE(_nc_lib_tracemouse)
#endif
