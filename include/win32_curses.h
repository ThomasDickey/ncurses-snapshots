/****************************************************************************
 * Copyright 2018-2024,2025 Thomas E. Dickey                                *
 * Copyright 2008-2014,2017 Free Software Foundation, Inc.                  *
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
 * Author: Juergen Pfeifer, 2008-on                                         *
 ****************************************************************************/

/* $Id: win32_curses.h,v 1.8 2025/10/04 18:54:44 tom Exp $ */

/*
 * This is the interface we use on Windows to mimic the control of the settings
 * of what acts like the classic TTY - the Windows Console.
 */

#ifndef _NC_WIN32_CURSES_H
#define _NC_WIN32_CURSES_H 1

#include <nc_win32.h>

#if (defined(_WIN32) || defined(_WIN64))
/*
  A few definitions of Unix functions we need to emulate
*/
#undef  ttyname
#define ttyname(fd) NULL

#undef sleep
#define sleep(n) Sleep((n) * 1000)

#endif /* _WIN32||_WIN64 */

#endif /* _NC_WIN32_CURSES_H */
