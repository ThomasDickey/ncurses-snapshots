/****************************************************************************
 * Copyright (c) 2001 Free Software Foundation, Inc.                        *
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
 *  Author: Thomas E. Dickey 1996-2001                                      *
 *     and: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/*
 *	visbuf.c - Tracing/Debugging support routines
 */

#include <curses.priv.h>

#include <tic.h>
#include <ctype.h>

MODULE_ID("$Id: visbuf.c,v 1.1 2001/10/20 19:23:45 tom Exp $")

static char *
_nc_vischar(char *tp, chtype c)
{
    if (c == '"' || c == '\\') {
	*tp++ = '\\';
	*tp++ = c;
    } else if (is7bits(c) && (isgraph(c) || c == ' ')) {
	*tp++ = c;
    } else if (c == '\n') {
	*tp++ = '\\';
	*tp++ = 'n';
    } else if (c == '\r') {
	*tp++ = '\\';
	*tp++ = 'r';
    } else if (c == '\b') {
	*tp++ = '\\';
	*tp++ = 'b';
    } else if (c == '\033') {
	*tp++ = '\\';
	*tp++ = 'e';
    } else if (is7bits(c) && iscntrl(UChar(c))) {
	*tp++ = '\\';
	*tp++ = '^';
	*tp++ = '@' + c;
    } else {
	sprintf(tp, "\\%03o", UChar(c));
	tp += strlen(tp);
    }
    return tp;
}

NCURSES_EXPORT(const char *)
_nc_visbuf2(int bufnum, const char *buf)
{
    char *vbuf;
    char *tp;
    int c;

    if (buf == 0)
	return ("(null)");
    if (buf == CANCELLED_STRING)
	return ("(cancelled)");

#ifdef TRACE
    tp = vbuf = _nc_trace_buf(bufnum, (strlen(buf) * 4) + 5);
#else
    {
	static char *mybuf[2];
	mybuf[bufnum] = _nc_doalloc(mybuf[bufnum], (strlen(buf) * 4) + 5);
	tp = vbuf = mybuf[bufnum];
    }
#endif
    *tp++ = '"';
    while ((c = *buf++) != '\0') {
	tp = _nc_vischar(tp, UChar(c));
    }
    *tp++ = '"';
    *tp++ = '\0';
    return (vbuf);
}

NCURSES_EXPORT(const char *)
_nc_visbuf(const char *buf)
{
    return _nc_visbuf2(0, buf);
}

#if USE_WIDEC_SUPPORT
NCURSES_EXPORT(const char *)
_nc_viswbuf2(int bufnum, const wchar_t * buf)
{
    char *vbuf;
    char *tp;
    int c;

    if (buf == 0)
	return ("(null)");

#ifdef TRACE
    tp = vbuf = _nc_trace_buf(bufnum, (wcslen(buf) * 4) + 5);
#else
    {
	static char *mybuf[2];
	mybuf[bufnum] = _nc_doalloc(mybuf[bufnum], (wcslen(buf) * 4) + 5);
	tp = vbuf = mybuf[bufnum];
    }
#endif
    *tp++ = '"';
    while ((c = *buf++) != '\0') {
	tp = _nc_vischar(tp, UChar(c));
    }
    *tp++ = '"';
    *tp++ = '\0';
    return (vbuf);
}

NCURSES_EXPORT(const char *)
_nc_viswbuf(const wchar_t * buf)
{
    return _nc_viswbuf2(0, buf);
}
#endif
