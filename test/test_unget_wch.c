/****************************************************************************
 * Copyright 2022 Thomas E. Dickey                                          *
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
 * $Id: test_unget_wch.c,v 1.3 2022/04/16 17:04:04 tom Exp $
 *
 * Demonstrate the unget_wch and unget functions.
 */

#include <test.priv.h>

#if USE_WIDEC_SUPPORT && HAVE_UNGET_WCH
int
main(void)
{
    int step = 0;

    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);

    for (;;) {
	wint_t widechar;
	int rc = get_wch(&widechar);
	if (rc == KEY_CODE_YES) {
	    printw("KEY[%d] %s\n", ++step, keyname((int) widechar));
	    ungetch((int) widechar);
	    printw("...[%d] %s\n", step, keyname(getch()));
	} else if (widechar == QUIT || widechar == ESCAPE) {
	    break;
	} else {
	    printw("CHR[%d] %s\n", ++step, key_name((wchar_t) widechar));
	    unget_wch((wchar_t) widechar);
	    rc = get_wch(&widechar);
	    printw("%s[%d] %s\n",
		   ((rc == KEY_CODE_YES)
		    ? "???"
		    : "..."),
		   step, key_name((wchar_t) widechar));
	}
    }

    endwin();

    ExitProgram(EXIT_SUCCESS);
}

#else
int
main(void)
{
    printf("This program requires the wide-ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif
