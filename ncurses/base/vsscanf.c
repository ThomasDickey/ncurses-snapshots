/****************************************************************************
 * Copyright (c) 1998,2000,2001 Free Software Foundation, Inc.              *
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

/*
 * This function is needed to support vwscanw
 */

#include <curses.priv.h>

#if !HAVE_VSSCANF

MODULE_ID("$Id: vsscanf.c,v 1.14 2001/12/08 20:12:35 tom Exp $")

/*VARARGS2*/
NCURSES_EXPORT(int)
vsscanf(const char *str, const char *format, va_list ap)
{
#if HAVE_VFSCANF || HAVE__DOSCAN
    /*
     * This code should work on anything descended from AT&T SVr1.
     */
    FILE strbuf;

    strbuf._flag = _IOREAD;
    strbuf._ptr = strbuf._base = (unsigned char *) str;
    strbuf._cnt = strlen(str);
    strbuf._file = _NFILE;

#if HAVE_VFSCANF
    return (vfscanf(&strbuf, format, ap));
#else
    return (_doscan(&strbuf, format, ap));
#endif
#else
    /*
     * You don't have a native vsscanf(3), and you don't have System-V
     * compatible stdio internals.  You're probably using a BSD
     * older than 4.4 or a really old Linux.  You lose.  Upgrade
     * to a current C library to win.
     */
    return -1;			/* not implemented */
#endif
}
#else
extern
NCURSES_EXPORT(void)
_nc_vsscanf(void);		/* quiet's gcc warning */
NCURSES_EXPORT(void)
_nc_vsscanf(void)
{
}				/* nonempty for strict ANSI compilers */
#endif /* !HAVE_VSSCANF */
