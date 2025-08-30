/****************************************************************************
 * Copyright 2018-2024,2025 Thomas E. Dickey                                *
 * Copyright 2008-2016,2017 Free Software Foundation, Inc.                  *
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
 *  Author: Juergen Pfeifer                                                 *
 *     and: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 * TODO - GetMousePos(POINT * result) from ntconio.c
 * TODO - implement nodelay
 * TODO - improve screen-repainting performance, using implied wraparound to reduce write's
 * TODO - make it optional whether screen is restored or not when non-buffered
 */

#define TTY int			/* FIXME: TTY originalMode */
#include <curses.priv.h>

#define CUR TerminalType(my_term).

MODULE_ID("$Id: win_driver.c,v 1.94 2025/08/30 20:50:55 tom Exp $")

#define WINMAGIC NCDRV_MAGIC(NCDRV_WINCONSOLE)
#define EXP_OPTIMIZE 0

extern NCURSES_EXPORT(WORD)   _nc_console_MapColor(bool fore, int color);

#define array_length(a) (sizeof(a)/sizeof(a[0]))

static bool InitConsole(void);

#define AssertTCB() assert(TCB != NULL && (TCB->magic == WINMAGIC))
#define validateConsoleHandle() (AssertTCB(), console_initialized || \
                                 InitConsole())
#define SetSP()     assert(TCB->csp != NULL); sp = TCB->csp; (void) sp

#define GenMap(vKey,key) MAKELONG(key, vKey)

#define AdjustY() (WINCONSOLE.buffered ? 0 : (int) WINCONSOLE.SBI.srWindow.Top)
/* *INDENT-OFF* */
static const LONG keylist[] =
{
    GenMap(VK_PRIOR,  KEY_PPAGE),
    GenMap(VK_NEXT,   KEY_NPAGE),
    GenMap(VK_END,    KEY_END),
    GenMap(VK_HOME,   KEY_HOME),
    GenMap(VK_LEFT,   KEY_LEFT),
    GenMap(VK_UP,     KEY_UP),
    GenMap(VK_RIGHT,  KEY_RIGHT),
    GenMap(VK_DOWN,   KEY_DOWN),
    GenMap(VK_DELETE, KEY_DC),
    GenMap(VK_INSERT, KEY_IC),
    GenMap(VK_TAB,    KEY_BTAB)
};
static const LONG ansi_keys[] =
{
    GenMap(VK_PRIOR,  'I'),
    GenMap(VK_NEXT,   'Q'),
    GenMap(VK_END,    'O'),
    GenMap(VK_HOME,   'H'),
    GenMap(VK_LEFT,   'K'),
    GenMap(VK_UP,     'H'),
    GenMap(VK_RIGHT,  'M'),
    GenMap(VK_DOWN,   'P'),
    GenMap(VK_DELETE, 'S'),
    GenMap(VK_INSERT, 'R'),
    GenMap(VK_TAB,    'Z')
};
/* *INDENT-ON* */
#define N_INI ((int)array_length(keylist))
#define FKEYS 24
#define MAPSIZE (FKEYS + N_INI)
#define CON_NUMPAIRS 64

/*   A process can only have a single console, so it is safe
     to maintain all the information about it in a single
     static structure.
 */
NCURSES_EXPORT_VAR(ConsoleInfo) _nc_CONSOLE;
static BOOL console_initialized = FALSE;

#define RevAttr(attr) (WORD) (((attr) & 0xff00) | \
		      ((((attr) & 0x07) << 4) | \
		       (((attr) & 0x70) >> 4)))

static WORD
MapAttr(WORD res, attr_t ch)
{
    if (ch & A_COLOR) {
	int p;

	p = PairNumber(ch);
	if (p > 0 && p < CON_NUMPAIRS) {
	    WORD a;
	    a = WINCONSOLE.pairs[p];
	    res = (WORD) ((res & 0xff00) | a);
	}
    }

    if (ch & A_REVERSE) {
	res = RevAttr(res);
    }

    if (ch & A_STANDOUT) {
	res = RevAttr(res) | BACKGROUND_INTENSITY;
    }

    if (ch & A_BOLD)
	res |= FOREGROUND_INTENSITY;

    if (ch & A_DIM)
	res |= BACKGROUND_INTENSITY;

    return res;
}

#if 0				/* def TRACE */
static void
dump_screen(const char *fn, int ln)
{
    int max_cells = (WINCONSOLE.SBI.dwSize.Y *
		     (1 + WINCONSOLE.SBI.dwSize.X)) + 1;
    char output[max_cells];
    CHAR_INFO save_screen[max_cells];
    COORD save_size;
    SMALL_RECT save_region;
    COORD bufferCoord;

    T(("dump_screen %s@%d", fn, ln));

    save_region.Top = WINCONSOLE.SBI.srWindow.Top;
    save_region.Left = WINCONSOLE.SBI.srWindow.Left;
    save_region.Bottom = WINCONSOLE.SBI.srWindow.Bottom;
    save_region.Right = WINCONSOLE.SBI.srWindow.Right;

    save_size.X = (SHORT) (save_region.Right - save_region.Left + 1);
    save_size.Y = (SHORT) (save_region.Bottom - save_region.Top + 1);

    bufferCoord.X = bufferCoord.Y = 0;

    if (read_screen(WINCONSOLE.hdl,
		    save_screen,
		    save_size,
		    bufferCoord,
		    &save_region)) {
	int i, j;
	int ij = 0;
	int k = 0;

	for (i = save_region.Top; i <= save_region.Bottom; ++i) {
	    for (j = save_region.Left; j <= save_region.Right; ++j) {
		output[k++] = save_screen[ij++].CharInfoChar;
	    }
	    output[k++] = '\n';
	}
	output[k] = 0;

	T(("DUMP: %d,%d - %d,%d",
	   save_region.Top,
	   save_region.Left,
	   save_region.Bottom,
	   save_region.Right));
	T(("%s", output));
    }
}

#else
#define dump_screen(fn,ln)	/* nothing */
#endif

#if USE_WIDEC_SUPPORT
/*
 * TODO: support surrogate pairs
 * TODO: support combining characters
 * TODO: support acsc
 * TODO: _nc_wacs should be part of sp.
 */
static BOOL
con_write16(TERMINAL_CONTROL_BLOCK * TCB,
	    int y, int x, cchar_t *str, int limit)
{
    int actual = 0;
    MakeArray(ci, CHAR_INFO, limit);
    COORD loc, siz;
    SMALL_RECT rec;
    int i;
    cchar_t ch;
    SCREEN *sp;

    AssertTCB();
    SetSP();

    for (i = actual = 0; i < limit; i++) {
	ch = str[i];
	if (isWidecExt(ch))
	    continue;
	ci[actual].CharInfoChar = CharOf(ch);
	ci[actual].Attributes = MapAttr(WINCONSOLE.SBI.wAttributes,
					AttrOf(ch));
	if (AttrOf(ch) & A_ALTCHARSET) {
	    if (_nc_wacs) {
		int which = CharOf(ch);
		if (which > 0
		    && which < ACS_LEN
		    && CharOf(_nc_wacs[which]) != 0) {
		    ci[actual].CharInfoChar = CharOf(_nc_wacs[which]);
		} else {
		    ci[actual].CharInfoChar = ' ';
		}
	    }
	}
	++actual;
    }

    loc.X = (SHORT) 0;
    loc.Y = (SHORT) 0;
    siz.X = (SHORT) actual;
    siz.Y = 1;

    rec.Left = (SHORT) x;
    rec.Top = (SHORT) (y + AdjustY());
    rec.Right = (SHORT) (x + limit - 1);
    rec.Bottom = rec.Top;

    return write_screen(WINCONSOLE.hdl, ci, siz, loc, &rec);
}
#define con_write(tcb, y, x, str, n) con_write16(tcb, y, x, str, n)
#else
static BOOL
con_write8(TERMINAL_CONTROL_BLOCK * TCB, int y, int x, chtype *str, int n)
{
    MakeArray(ci, CHAR_INFO, n);
    COORD loc, siz;
    SMALL_RECT rec;
    int i;
    chtype ch;
    SCREEN *sp;

    AssertTCB();
    SetSP();

    for (i = 0; i < n; i++) {
	ch = str[i];
	ci[i].CharInfoChar = ChCharOf(ch);
	ci[i].Attributes = MapAttr(WINCONSOLE.SBI.wAttributes,
				   ChAttrOf(ch));
	if (ChAttrOf(ch) & A_ALTCHARSET) {
	    if (sp->_acs_map)
		ci[i].CharInfoChar =
		    ChCharOf(NCURSES_SP_NAME(_nc_acs_char) (sp, ChCharOf(ch)));
	}
    }

    loc.X = (short) 0;
    loc.Y = (short) 0;
    siz.X = (short) n;
    siz.Y = 1;

    rec.Left = (short) x;
    rec.Top = (short) y;
    rec.Right = (short) (x + n - 1);
    rec.Bottom = rec.Top;

    return write_screen(WINCONSOLE.hdl, ci, siz, loc, &rec);
}
#define con_write(tcb, y, x, str, n) con_write8(tcb, y, x, str, n)
#endif

#if EXP_OPTIMIZE
/*
 * Comparing new/current screens, determine the last column-index for a change
 * beginning on the given row,col position.  Unlike a serial terminal, there is
 * no cost for "moving" the "cursor" on the line as we update it.
 */
static int
find_end_of_change(SCREEN *sp, int row, int col)
{
    int result = col;
    struct ldat *curdat = CurScreen(sp)->_line + row;
    struct ldat *newdat = NewScreen(sp)->_line + row;

    while (col <= newdat->lastchar) {
#if USE_WIDEC_SUPPORT
	if (isWidecExt(curdat->text[col]) ||
	    isWidecExt(newdat->text[col])) {
	    result = col;
	} else if (memcmp(&curdat->text[col],
			  &newdat->text[col],
			  sizeof(curdat->text[0]))) {
	    result = col;
	} else {
	    break;
	}
#else
	if (curdat->text[col] != newdat->text[col]) {
	    result = col;
	} else {
	    break;
	}
#endif
	++col;
    }
    return result;
}

/*
 * Given a row,col position at the end of a change-chunk, look for the
 * beginning of the next change-chunk.
 */
static int
find_next_change(SCREEN *sp, int row, int col)
{
    struct ldat *curdat = CurScreen(sp)->_line + row;
    struct ldat *newdat = NewScreen(sp)->_line + row;
    int result = newdat->lastchar + 1;

    while (++col <= newdat->lastchar) {
#if USE_WIDEC_SUPPORT
	if (isWidecExt(curdat->text[col]) !=
	    isWidecExt(newdat->text[col])) {
	    result = col;
	    break;
	} else if (memcmp(&curdat->text[col],
			  &newdat->text[col],
			  sizeof(curdat->text[0]))) {
	    result = col;
	    break;
	}
#else
	if (curdat->text[col] != newdat->text[col]) {
	    result = col;
	    break;
	}
#endif
    }
    return result;
}

#define EndChange(first) \
	find_end_of_change(sp, y, first)
#define NextChange(last)                        \
	find_next_change(sp, y, last)

#endif /* EXP_OPTIMIZE */

#define MARK_NOCHANGE(win,row)                 \
    win->_line[row].firstchar = _NOCHANGE;     \
    win->_line[row].lastchar  = _NOCHANGE

static void
_nc_console_selectActiveHandle(void)
{
    if (WINCONSOLE.lastOut != WINCONSOLE.hdl) {
	WINCONSOLE.lastOut = WINCONSOLE.hdl;
	SetConsoleActiveScreenBuffer(WINCONSOLE.lastOut);
    }
}

static bool
restore_original_screen(void)
{
    COORD bufferCoord;
    bool result = FALSE;
    SMALL_RECT save_region = WINCONSOLE.save_region;

    T(("... restoring %s", WINCONSOLE.window_only ?
       "window" : "entire buffer"));

    bufferCoord.X = (SHORT) (WINCONSOLE.window_only ?
			     WINCONSOLE.SBI.srWindow.Left : 0);
    bufferCoord.Y = (SHORT) (WINCONSOLE.window_only ?
			     WINCONSOLE.SBI.srWindow.Top : 0);

    if (write_screen(WINCONSOLE.hdl,
		     WINCONSOLE.save_screen,
		     WINCONSOLE.save_size,
		     bufferCoord,
		     &save_region)) {
	result = TRUE;
	mvcur(-1, -1, LINES - 2, 0);
	T(("... restore original screen contents ok %dx%d (%d,%d - %d,%d)",
	   WINCONSOLE.save_size.Y,
	   WINCONSOLE.save_size.X,
	   save_region.Top,
	   save_region.Left,
	   save_region.Bottom,
	   save_region.Right));
    } else {
	T(("... restore original screen contents err"));
    }
    return result;
}

static const char *
wcon_name(TERMINAL_CONTROL_BLOCK * TCB)
{
    (void) TCB;
    return "win32console";
}

static int
wcon_doupdate(TERMINAL_CONTROL_BLOCK * TCB)
{
    int result = ERR;
    int y, nonempty, n, x0, x1, Width, Height;
    SCREEN *sp;

    T((T_CALLED("win32con::wcon_doupdate(%p)"), TCB));
    if (validateConsoleHandle()) {
	SetSP();

	Width = screen_columns(sp);
	Height = screen_lines(sp);
	nonempty = Min(Height, NewScreen(sp)->_maxy + 1);

	T(("... %dx%d clear cur:%d new:%d",
	   Height, Width,
	   CurScreen(sp)->_clear,
	   NewScreen(sp)->_clear));

	if (SP_PARM->_endwin == ewSuspend) {

	    T(("coming back from shell mode"));
	    NCURSES_SP_NAME(reset_prog_mode) (NCURSES_SP_ARG);

	    NCURSES_SP_NAME(_nc_mvcur_resume) (NCURSES_SP_ARG);
	    NCURSES_SP_NAME(_nc_screen_resume) (NCURSES_SP_ARG);
	    SP_PARM->_mouse_resume(SP_PARM);

	    SP_PARM->_endwin = ewRunning;
	}

	if ((CurScreen(sp)->_clear || NewScreen(sp)->_clear)) {
	    int x;
#if USE_WIDEC_SUPPORT
	    MakeArray(empty, cchar_t, Width);
	    wchar_t blank[2] =
	    {
		L' ', L'\0'
	    };

	    for (x = 0; x < Width; x++)
		setcchar(&empty[x], blank, 0, 0, NULL);
#else
	    MakeArray(empty, chtype, Width);

	    for (x = 0; x < Width; x++)
		empty[x] = ' ';
#endif

	    for (y = 0; y < nonempty; y++) {
		con_write(TCB, y, 0, empty, Width);
		memcpy(empty,
		       CurScreen(sp)->_line[y].text,
		       (size_t) Width * sizeof(empty[0]));
	    }
	    CurScreen(sp)->_clear = FALSE;
	    NewScreen(sp)->_clear = FALSE;
	    touchwin(NewScreen(sp));
	    T(("... cleared %dx%d lines @%d of screen", nonempty, Width,
	       AdjustY()));
	}

	for (y = 0; y < nonempty; y++) {
	    x0 = NewScreen(sp)->_line[y].firstchar;
	    if (x0 != _NOCHANGE) {
#if EXP_OPTIMIZE
		int x2;
		int limit = NewScreen(sp)->_line[y].lastchar;
		while ((x1 = EndChange(x0)) <= limit) {
		    while ((x2 = NextChange(x1)) <=
			   limit && x2 <= (x1 + 2)) {
			x1 = x2;
		    }
		    n = x1 - x0 + 1;
		    memcpy(&CurScreen(sp)->_line[y].text[x0],
			   &NewScreen(sp)->_line[y].text[x0],
			   n * sizeof(CurScreen(sp)->_line[y].text[x0]));
		    con_write(TCB,
			      y,
			      x0,
			      &CurScreen(sp)->_line[y].text[x0], n);
		    x0 = NextChange(x1);
		}

		/* mark line changed successfully */
		if (y <= NewScreen(sp)->_maxy) {
		    MARK_NOCHANGE(NewScreen(sp), y);
		}
		if (y <= CurScreen(sp)->_maxy) {
		    MARK_NOCHANGE(CurScreen(sp), y);
		}
#else
		x1 = NewScreen(sp)->_line[y].lastchar;
		n = x1 - x0 + 1;
		if (n > 0) {
		    memcpy(&CurScreen(sp)->_line[y].text[x0],
			   &NewScreen(sp)->_line[y].text[x0],
			   (size_t) n *
			   sizeof(CurScreen(sp)->_line[y].text[x0]));
		    con_write(TCB,
			      y,
			      x0,
			      &CurScreen(sp)->_line[y].text[x0], n);

		    /* mark line changed successfully */
		    if (y <= NewScreen(sp)->_maxy) {
			MARK_NOCHANGE(NewScreen(sp), y);
		    }
		    if (y <= CurScreen(sp)->_maxy) {
			MARK_NOCHANGE(CurScreen(sp), y);
		    }
		}
#endif
	    }
	}

	/* put everything back in sync */
	for (y = nonempty; y <= NewScreen(sp)->_maxy; y++) {
	    MARK_NOCHANGE(NewScreen(sp), y);
	}
	for (y = nonempty; y <= CurScreen(sp)->_maxy; y++) {
	    MARK_NOCHANGE(CurScreen(sp), y);
	}

	if (!NewScreen(sp)->_leaveok) {
	    CurScreen(sp)->_curx = NewScreen(sp)->_curx;
	    CurScreen(sp)->_cury = NewScreen(sp)->_cury;

	    TCB->drv->td_hwcur(TCB,
			       0,
			       0,
			       CurScreen(sp)->_cury,
			       CurScreen(sp)->_curx);
	}
	_nc_console_selectActiveHandle();
	result = OK;
    }
    returnCode(result);
}

#ifdef __MINGW32__
#define SysISATTY(fd) _isatty(fd)
#else
#define SysISATTY(fd) isatty(fd)
#endif

static bool
wcon_CanHandle(TERMINAL_CONTROL_BLOCK * TCB,
	       const char *tname,
	       int *errret GCC_UNUSED)
{
    bool code = FALSE;

    T((T_CALLED("win32con::wcon_CanHandle(%p)"), TCB));

    assert((TCB != NULL) && (tname != NULL));

    TCB->magic = WINMAGIC;

    if (tname == NULL || *tname == 0)
	code = TRUE;
    else if (tname != NULL && *tname == '#') {
	/*
	 * Use "#" (a character which cannot begin a terminal's name) to
	 * select specific driver from the table.
	 *
	 * In principle, we could have more than one non-terminfo driver,
	 * e.g., "win32gui".
	 */
	size_t n = strlen(tname + 1);
	if (n != 0
	    && ((strncmp(tname + 1, "win32console", n) == 0)
		|| (strncmp(tname + 1, "win32con", n) == 0))) {
	    code = TRUE;
	}
    } else if (tname != NULL && stricmp(tname, "unknown") == 0) {
	code = TRUE;
    } else if (SysISATTY(TCB->term.Filedes)) {
	code = TRUE;
    }

    /*
     * This is intentional, to avoid unnecessary breakage of applications
     * using <term.h> symbols.
     */
    if (code && (TerminalType(&TCB->term).Booleans == 0)) {
	_nc_init_termtype(&TerminalType(&TCB->term));
#if NCURSES_EXT_NUMBERS
	_nc_export_termtype2(&TCB->term.type, &TerminalType(&TCB->term));
#endif
    }

    if (!code) {
	if (_nc_console_test(0)) {
	    T(("isTermInfoConsole=TRUE"));
	    WINCONSOLE.isTermInfoConsole = TRUE;
	}
    }
    returnBool(code);
}

static int
wcon_dobeepflash(TERMINAL_CONTROL_BLOCK * TCB,
		 int beepFlag)
{
    SCREEN *sp;
    int res = ERR;

    int high = (WINCONSOLE.SBI.srWindow.Bottom -
		WINCONSOLE.SBI.srWindow.Top + 1);
    int wide = (WINCONSOLE.SBI.srWindow.Right -
		WINCONSOLE.SBI.srWindow.Left + 1);
    int max_cells = (high * wide);
    int i;

    MakeArray(this_screen, CHAR_INFO, max_cells);
    MakeArray(that_screen, CHAR_INFO, max_cells);
    COORD this_size;
    SMALL_RECT this_region;
    COORD bufferCoord;

    if (validateConsoleHandle()) {
	SetSP();
	this_region.Top = WINCONSOLE.SBI.srWindow.Top;
	this_region.Left = WINCONSOLE.SBI.srWindow.Left;
	this_region.Bottom = WINCONSOLE.SBI.srWindow.Bottom;
	this_region.Right = WINCONSOLE.SBI.srWindow.Right;

	this_size.X = (SHORT) wide;
	this_size.Y = (SHORT) high;

	bufferCoord.X = this_region.Left;
	bufferCoord.Y = this_region.Top;

	if (!beepFlag &&
	    read_screen(WINCONSOLE.hdl,
			this_screen,
			this_size,
			bufferCoord,
			&this_region)) {

	    memcpy(that_screen,
		   this_screen,
		   sizeof(CHAR_INFO) * (size_t) max_cells);

	    for (i = 0; i < max_cells; i++) {
		that_screen[i].Attributes =
		    RevAttr(that_screen[i].Attributes);
	    }

	    write_screen(WINCONSOLE.hdl, that_screen, this_size,
			 bufferCoord, &this_region);
	    Sleep(200);
	    write_screen(WINCONSOLE.hdl, this_screen, this_size,
			 bufferCoord, &this_region);

	} else {
	    MessageBeep(MB_ICONWARNING);	/* MB_OK might be better */
	}
	res = OK;
    }
    return res;
}

static int
wcon_print(TERMINAL_CONTROL_BLOCK * TCB,
	   char *data GCC_UNUSED,
	   int len GCC_UNUSED)
{
    SCREEN *sp;

    AssertTCB();
    SetSP();

    return ERR;
}

static int
wcon_defaultcolors(TERMINAL_CONTROL_BLOCK * TCB,
		   int fg GCC_UNUSED,
		   int bg GCC_UNUSED)
{
    SCREEN *sp;
    int code = ERR;

    AssertTCB();
    SetSP();

    return (code);
}

static void
wcon_setcolor(TERMINAL_CONTROL_BLOCK * TCB,
	      int fore,
	      int color,
	      int (*outc) (SCREEN *, int) GCC_UNUSED)
{
    (void) TCB;
    if (validateConsoleHandle()) {
	WORD a = _nc_console_MapColor(fore, color);
	a |= (WORD) ((WINCONSOLE.SBI.wAttributes) & (fore ? 0xfff8 : 0xff8f));
	SetConsoleTextAttribute(WINCONSOLE.hdl, a);
	_nc_console_get_SBI();
    }
}

static bool
wcon_rescol(TERMINAL_CONTROL_BLOCK * TCB)
{
    bool res = FALSE;

    (void) TCB;
    if (validateConsoleHandle()) {
	WORD a = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
	SetConsoleTextAttribute(WINCONSOLE.hdl, a);
	_nc_console_get_SBI();
	res = TRUE;
    }
    return res;
}

static bool
wcon_rescolors(TERMINAL_CONTROL_BLOCK * TCB)
{
    int result = FALSE;
    SCREEN *sp;

    AssertTCB();
    SetSP();

    return result;
}

static int
wcon_size(TERMINAL_CONTROL_BLOCK * TCB, int *Lines, int *Cols)
{
    int result = ERR;

    T((T_CALLED("win32con::wcon_size(%p)"), TCB));

    if (validateConsoleHandle() &&
	(Lines != NULL) && (Cols != NULL)) {
	_nc_console_size(Lines, Cols);
	result = OK;
    }
    returnCode(result);
}

static int
wcon_setsize(TERMINAL_CONTROL_BLOCK * TCB GCC_UNUSED,
	     int l GCC_UNUSED,
	     int c GCC_UNUSED)
{
    AssertTCB();
    return ERR;
}

static int
wcon_sgmode(TERMINAL_CONTROL_BLOCK * TCB, int setFlag, TTY * buf)
{
    DWORD dwFlag = 0;
    tcflag_t iflag;
    tcflag_t lflag;
    int result = ERR;

    T((T_CALLED("win32con::wcon_sgmode(TCB=(%p),setFlag=%d,TTY=(%p)"),
       TCB, setFlag, buf));
    if (buf != NULL && validateConsoleHandle()) {

	if (setFlag) {
	    iflag = buf->c_iflag;
	    lflag = buf->c_lflag;

	    GetConsoleMode(WINCONSOLE.inp, &dwFlag);

	    if (lflag & ICANON)
		dwFlag |= ENABLE_LINE_INPUT;
	    else
		dwFlag &= (DWORD) (~ENABLE_LINE_INPUT);

	    if (lflag & ECHO)
		dwFlag |= ENABLE_ECHO_INPUT;
	    else
		dwFlag &= (DWORD) (~ENABLE_ECHO_INPUT);

	    if (iflag & BRKINT)
		dwFlag |= ENABLE_PROCESSED_INPUT;
	    else
		dwFlag &= (DWORD) (~ENABLE_PROCESSED_INPUT);

	    dwFlag |= ENABLE_MOUSE_INPUT;

	    buf->c_iflag = iflag;
	    buf->c_lflag = lflag;
	    SetConsoleMode(WINCONSOLE.inp, dwFlag);
	    TCB->term.Nttyb = *buf;
	} else {
	    iflag = TCB->term.Nttyb.c_iflag;
	    lflag = TCB->term.Nttyb.c_lflag;
	    GetConsoleMode(WINCONSOLE.inp, &dwFlag);

	    if (dwFlag & ENABLE_LINE_INPUT)
		lflag |= ICANON;
	    else
		lflag &= (tcflag_t) (~ICANON);

	    if (dwFlag & ENABLE_ECHO_INPUT)
		lflag |= ECHO;
	    else
		lflag &= (tcflag_t) (~ECHO);

	    if (dwFlag & ENABLE_PROCESSED_INPUT)
		iflag |= BRKINT;
	    else
		iflag &= (tcflag_t) (~BRKINT);

	    TCB->term.Nttyb.c_iflag = iflag;
	    TCB->term.Nttyb.c_lflag = lflag;

	    *buf = TCB->term.Nttyb;
	}
	result = OK;
    }
    returnCode(result);
}

#define MIN_WIDE 80
#define MIN_HIGH 24

/*
 * In "normal" mode, reset the buffer- and window-sizes back to their original values.
 */
static void
_nc_console_set_scrollback(bool normal, CONSOLE_SCREEN_BUFFER_INFO * info)
{
    SMALL_RECT rect;
    COORD coord;
    bool changed = FALSE;

    T((T_CALLED("win32con::_nc_console_set_scrollback(%s)"),
       (normal
	? "normal"
	: "application")));

    T(("... SBI.srWindow %d,%d .. %d,%d",
       info->srWindow.Top,
       info->srWindow.Left,
       info->srWindow.Bottom,
       info->srWindow.Right));
    T(("... SBI.dwSize %dx%d",
       info->dwSize.Y,
       info->dwSize.X));

    if (normal) {
	rect = info->srWindow;
	coord = info->dwSize;
	if (memcmp(info, &WINCONSOLE.SBI, sizeof(*info)) != 0) {
	    changed = TRUE;
	    WINCONSOLE.SBI = *info;
	}
    } else {
	int high = info->srWindow.Bottom - info->srWindow.Top + 1;
	int wide = info->srWindow.Right - info->srWindow.Left + 1;

	if (high < MIN_HIGH) {
	    T(("... height %d < %d", high, MIN_HIGH));
	    high = MIN_HIGH;
	    changed = TRUE;
	}
	if (wide < MIN_WIDE) {
	    T(("... width %d < %d", wide, MIN_WIDE));
	    wide = MIN_WIDE;
	    changed = TRUE;
	}

	rect.Left =
	    rect.Top = 0;
	rect.Right = (SHORT) (wide - 1);
	rect.Bottom = (SHORT) (high - 1);

	coord.X = (SHORT) wide;
	coord.Y = (SHORT) high;

	if (info->dwSize.Y != high ||
	    info->dwSize.X != wide ||
	    info->srWindow.Top != 0 ||
	    info->srWindow.Left != 0) {
	    changed = TRUE;
	}

    }

    if (changed) {
	T(("... coord %d,%d", coord.Y, coord.X));
	T(("... rect %d,%d - %d,%d",
	   rect.Top, rect.Left,
	   rect.Bottom, rect.Right));
	SetConsoleScreenBufferSize(WINCONSOLE.hdl, coord);	/* dwSize */
	SetConsoleWindowInfo(WINCONSOLE.hdl, TRUE, &rect);	/* srWindow */
	_nc_console_get_SBI();
    }
    returnVoid;
}

static int
wcon_mode(TERMINAL_CONTROL_BLOCK * TCB, int progFlag, int defFlag)
{
    SCREEN *sp;
    TERMINAL *_term = (TERMINAL *) TCB;
    int code = ERR;

    T((T_CALLED("win32con::wcon_mode(%p, progFlag=%d, defFlag=%d)"),
       TCB, progFlag, defFlag));

    if (validateConsoleHandle()) {
	sp = TCB->csp;

	WINCONSOLE.progMode = progFlag;
	WINCONSOLE.lastOut = progFlag ? WINCONSOLE.hdl : WINCONSOLE.out;
	SetConsoleActiveScreenBuffer(WINCONSOLE.lastOut);

	if (progFlag) /* prog mode */  {
	    if (defFlag) {
		if ((wcon_sgmode(TCB, FALSE, &(_term->Nttyb)) == OK)) {
		    _term->Nttyb.c_oflag &= (tcflag_t) (~OFLAGS_TABS);
		    code = OK;
		}
	    } else {
		/* reset_prog_mode */
		if (wcon_sgmode(TCB, TRUE, &(_term->Nttyb)) == OK) {
		    if (sp) {
			if (sp->_keypad_on)
			    _nc_keypad(sp, TRUE);
		    }
		    if (!WINCONSOLE.buffered) {
			_nc_console_set_scrollback(FALSE, &WINCONSOLE.SBI);
		    }
		    code = OK;
		}
	    }
	    T(("... buffered:%d, clear:%d",
	       WINCONSOLE.buffered, CurScreen(sp)->_clear));
	} else {		/* shell mode */
	    if (defFlag) {
		/* def_shell_mode */
		if (wcon_sgmode(TCB, FALSE, &(_term->Ottyb)) == OK) {
		    code = OK;
		}
	    } else {
		/* reset_shell_mode */
		if (sp) {
		    _nc_keypad(sp, FALSE);
		    NCURSES_SP_NAME(_nc_flush) (sp);
		}
		code = wcon_sgmode(TCB, TRUE, &(_term->Ottyb));
		if (!WINCONSOLE.buffered) {
		    _nc_console_set_scrollback(TRUE, &WINCONSOLE.save_SBI);
		    if (!restore_original_screen())
			code = ERR;
		}
		SetConsoleCursorInfo(WINCONSOLE.hdl, &WINCONSOLE.save_CI);
	    }
	}

    }
    returnCode(code);
}

static void
wcon_screen_init(SCREEN *sp GCC_UNUSED)
{
}

static void
wcon_wrap(SCREEN *sp GCC_UNUSED)
{
}

static int
rkeycompare(const void *el1, const void *el2)
{
    WORD key1 = (LOWORD((*((const LONG *) el1)))) & 0x7fff;
    WORD key2 = (LOWORD((*((const LONG *) el2)))) & 0x7fff;

    return ((key1 < key2) ? -1 : ((key1 == key2) ? 0 : 1));
}

static int
keycompare(const void *el1, const void *el2)
{
    WORD key1 = HIWORD((*((const LONG *) el1)));
    WORD key2 = HIWORD((*((const LONG *) el2)));

    return ((key1 < key2) ? -1 : ((key1 == key2) ? 0 : 1));
}

static int
MapKey(WORD vKey)
{
    WORD nKey = 0;
    void *res;
    LONG key = GenMap(vKey, 0);
    int code = -1;

    res = bsearch(&key,
		  WINCONSOLE.map,
		  (size_t) (N_INI + FKEYS),
		  sizeof(keylist[0]),
		  keycompare);
    if (res) {
	key = *((LONG *) res);
	nKey = LOWORD(key);
	code = (int) (nKey & 0x7fff);
	if (nKey & 0x8000)
	    code = -code;
    }
    return code;
}

static void
wcon_release(TERMINAL_CONTROL_BLOCK * TCB)
{
    T((T_CALLED("win32con::wcon_release(%p)"), TCB));

    AssertTCB();
    if (TCB->prop)
	free(TCB->prop);

    returnVoid;
}

static bool
read_screen_data(void)
{
    bool result = FALSE;
    COORD bufferCoord;
    size_t want;

    WINCONSOLE.save_size.X = (SHORT) (WINCONSOLE.save_region.Right
				      - WINCONSOLE.save_region.Left + 1);
    WINCONSOLE.save_size.Y = (SHORT) (WINCONSOLE.save_region.Bottom
				      - WINCONSOLE.save_region.Top + 1);

    want = (size_t) (WINCONSOLE.save_size.X * WINCONSOLE.save_size.Y);

    if ((WINCONSOLE.save_screen = malloc(want * sizeof(CHAR_INFO))) != NULL) {
	bufferCoord.X = (SHORT) (WINCONSOLE.window_only
				 ? WINCONSOLE.SBI.srWindow.Left
				 : 0);
	bufferCoord.Y = (SHORT) (WINCONSOLE.window_only
				 ? WINCONSOLE.SBI.srWindow.Top
				 : 0);

	T(("... reading console %s %dx%d into %d,%d - %d,%d at %d,%d",
	   WINCONSOLE.window_only ? "window" : "buffer",
	   WINCONSOLE.save_size.Y, WINCONSOLE.save_size.X,
	   WINCONSOLE.save_region.Top,
	   WINCONSOLE.save_region.Left,
	   WINCONSOLE.save_region.Bottom,
	   WINCONSOLE.save_region.Right,
	   bufferCoord.Y,
	   bufferCoord.X));

	if (read_screen(WINCONSOLE.hdl,
			WINCONSOLE.save_screen,
			WINCONSOLE.save_size,
			bufferCoord,
			&WINCONSOLE.save_region)) {
	    result = TRUE;
	} else {
	    T((" error %#lx", (unsigned long) GetLastError()));
	    FreeAndNull(WINCONSOLE.save_screen);
	}
    }

    return result;
}

/*
 * Attempt to save the screen contents.  PDCurses does this if
 * PDC_RESTORE_SCREEN is set, giving the same visual appearance on
 * restoration as if the library had allocated a console buffer.  MSDN
 * says that the data which can be read is limited to 64Kb (and may be
 * less).
 */
static bool
save_original_screen(void)
{
    bool result = FALSE;

    WINCONSOLE.save_region.Top = 0;
    WINCONSOLE.save_region.Left = 0;
    WINCONSOLE.save_region.Bottom = (SHORT) (WINCONSOLE.SBI.dwSize.Y - 1);
    WINCONSOLE.save_region.Right = (SHORT) (WINCONSOLE.SBI.dwSize.X - 1);

    if (read_screen_data()) {
	result = TRUE;
    } else {

	WINCONSOLE.save_region.Top = WINCONSOLE.SBI.srWindow.Top;
	WINCONSOLE.save_region.Left = WINCONSOLE.SBI.srWindow.Left;
	WINCONSOLE.save_region.Bottom = WINCONSOLE.SBI.srWindow.Bottom;
	WINCONSOLE.save_region.Right = WINCONSOLE.SBI.srWindow.Right;

	WINCONSOLE.window_only = TRUE;

	if (read_screen_data()) {
	    result = TRUE;
	}
    }

    T(("... save original screen contents %s", result ? "ok" : "err"));
    return result;
}

static void
wcon_init(TERMINAL_CONTROL_BLOCK * TCB)
{
    T((T_CALLED("win32con::wcon_init(%p)"), TCB));

    AssertTCB();

    if (TCB) {
	if (!InitConsole()) {
	    returnVoid;
	}

	TCB->info.initcolor = TRUE;
	TCB->info.canchange = FALSE;
	TCB->info.hascolor = TRUE;
	TCB->info.caninit = TRUE;

	TCB->info.maxpairs = CON_NUMPAIRS;
	TCB->info.maxcolors = 8;
	TCB->info.numlabels = 0;
	TCB->info.labelwidth = 0;
	TCB->info.labelheight = 0;
	TCB->info.nocolorvideo = 1;
	TCB->info.tabsize = 8;

	TCB->info.numbuttons = WINCONSOLE.numButtons;
	TCB->info.defaultPalette = _nc_cga_palette;

    }
    returnVoid;
}

static void
wcon_initpair(TERMINAL_CONTROL_BLOCK * TCB,
	      int pair,
	      int f,
	      int b)
{
    SCREEN *sp;

    if (validateConsoleHandle()) {
	SetSP();

	if ((pair > 0) && (pair < CON_NUMPAIRS) && (f >= 0) && (f < 8)
	    && (b >= 0) && (b < 8)) {
	    WINCONSOLE.pairs[pair] =
		_nc_console_MapColor(true, f) |
		_nc_console_MapColor(false, b);
	}
    }
}

static void
wcon_initcolor(TERMINAL_CONTROL_BLOCK * TCB,
	       int color GCC_UNUSED,
	       int r GCC_UNUSED,
	       int g GCC_UNUSED,
	       int b GCC_UNUSED)
{
    SCREEN *sp;

    AssertTCB();
    SetSP();
}

static void
wcon_do_color(TERMINAL_CONTROL_BLOCK * TCB,
	      int old_pair GCC_UNUSED,
	      int pair GCC_UNUSED,
	      int reverse GCC_UNUSED,
	      int (*outc) (SCREEN *, int) GCC_UNUSED
)
{
    SCREEN *sp;

    AssertTCB();
    SetSP();
}

static void
wcon_initmouse(TERMINAL_CONTROL_BLOCK * TCB)
{
    SCREEN *sp;

    T((T_CALLED("win32con::wcon_initmouse(%p)"), TCB));

    if (validateConsoleHandle()) {
	SetSP();

	sp->_mouse_type = M_TERM_DRIVER;
    }
    returnVoid;
}

static int
wcon_testmouse(TERMINAL_CONTROL_BLOCK * TCB,
	       int delay
	       EVENTLIST_2nd(_nc_eventlist * evl))
{
    int rc = 0;
    SCREEN *sp;

    T((T_CALLED("win32con::wcon_testmouse(%p)"), TCB));
    if (validateConsoleHandle()) {
	SetSP();

	if (sp->_drv_mouse_head < sp->_drv_mouse_tail) {
	    rc = TW_MOUSE;
	} else {
	    rc = TCBOf(sp)->drv->td_twait(TCBOf(sp),
					  TWAIT_MASK,
					  delay,
					  (int *) 0
					  EVENTLIST_2nd(evl));
	}
    }

    returnCode(rc);
}

static int
wcon_mvcur(TERMINAL_CONTROL_BLOCK * TCB,
	   int yold GCC_UNUSED, int xold GCC_UNUSED,
	   int y, int x)
{
    int ret = ERR;

    (void) TCB;
    if (validateConsoleHandle()) {
	COORD loc;
	loc.X = (short) x;
	loc.Y = (short) (y + AdjustY());
	SetConsoleCursorPosition(WINCONSOLE.hdl, loc);
	ret = OK;
    }
    return ret;
}

static void
wcon_hwlabel(TERMINAL_CONTROL_BLOCK * TCB,
	     int labnum GCC_UNUSED,
	     char *text GCC_UNUSED)
{
    SCREEN *sp;

    AssertTCB();
    SetSP();
}

static void
wcon_hwlabelOnOff(TERMINAL_CONTROL_BLOCK * TCB,
		  int OnFlag GCC_UNUSED)
{
    SCREEN *sp;

    AssertTCB();
    SetSP();
}

static chtype
wcon_conattr(TERMINAL_CONTROL_BLOCK * TCB GCC_UNUSED)
{
    chtype res = A_NORMAL;
    res |= (A_BOLD | A_DIM | A_REVERSE | A_STANDOUT | A_COLOR);
    return res;
}

static void
wcon_setfilter(TERMINAL_CONTROL_BLOCK * TCB)
{
    SCREEN *sp;

    AssertTCB();
    SetSP();
}

static void
wcon_initacs(TERMINAL_CONTROL_BLOCK * TCB,
	     chtype *real_map GCC_UNUSED,
	     chtype *fake_map GCC_UNUSED)
{
#define DATA(a,b) { a, b }
    static struct {
	int acs_code;
	int use_code;
    } table[] = {
	DATA('a', 0xb1),	/* ACS_CKBOARD  */
	    DATA('f', 0xf8),	/* ACS_DEGREE   */
	    DATA('g', 0xf1),	/* ACS_PLMINUS  */
	    DATA('j', 0xd9),	/* ACS_LRCORNER */
	    DATA('l', 0xda),	/* ACS_ULCORNER */
	    DATA('k', 0xbf),	/* ACS_URCORNER */
	    DATA('m', 0xc0),	/* ACS_LLCORNER */
	    DATA('n', 0xc5),	/* ACS_PLUS     */
	    DATA('q', 0xc4),	/* ACS_HLINE    */
	    DATA('t', 0xc3),	/* ACS_LTEE     */
	    DATA('u', 0xb4),	/* ACS_RTEE     */
	    DATA('v', 0xc1),	/* ACS_BTEE     */
	    DATA('w', 0xc2),	/* ACS_TTEE     */
	    DATA('x', 0xb3),	/* ACS_VLINE    */
	    DATA('y', 0xf3),	/* ACS_LEQUAL   */
	    DATA('z', 0xf2),	/* ACS_GEQUAL   */
	    DATA('0', 0xdb),	/* ACS_BLOCK    */
	    DATA('{', 0xe3),	/* ACS_PI       */
	    DATA('}', 0x9c),	/* ACS_STERLING */
	    DATA(',', 0xae),	/* ACS_LARROW   */
	    DATA('+', 0xaf),	/* ACS_RARROW   */
	    DATA('~', 0xf9),	/* ACS_BULLET   */
    };
#undef DATA
    unsigned n;

    SCREEN *sp;
    if (validateConsoleHandle()) {
	SetSP();

	for (n = 0; n < SIZEOF(table); ++n) {
	    real_map[table[n].acs_code] =
		(chtype) table[n].use_code | A_ALTCHARSET;
	    if (sp != NULL)
		sp->_screen_acs_map[table[n].acs_code] = TRUE;
	}
    }
}

static ULONGLONG
tdiff(FILETIME fstart, FILETIME fend)
{
    ULARGE_INTEGER ustart;
    ULARGE_INTEGER uend;
    ULONGLONG diff;

    ustart.LowPart = fstart.dwLowDateTime;
    ustart.HighPart = fstart.dwHighDateTime;
    uend.LowPart = fend.dwLowDateTime;
    uend.HighPart = fend.dwHighDateTime;

    diff = (uend.QuadPart - ustart.QuadPart) / 10000;
    return diff;
}

static int
Adjust(int milliseconds, int diff)
{
    if (milliseconds != NC_INFINITY) {
	milliseconds -= diff;
	if (milliseconds < 0)
	    milliseconds = 0;
    }
    return milliseconds;
}

#define BUTTON_MASK (FROM_LEFT_1ST_BUTTON_PRESSED | \
                     FROM_LEFT_2ND_BUTTON_PRESSED | \
                     FROM_LEFT_3RD_BUTTON_PRESSED | \
                     FROM_LEFT_4TH_BUTTON_PRESSED | \
                     RIGHTMOST_BUTTON_PRESSED)

static mmask_t
decode_mouse(SCREEN *sp, int mask)
{
    mmask_t result = 0;

    (void) sp;
    assert(sp && console_initialized);

    if (mask & FROM_LEFT_1ST_BUTTON_PRESSED)
	result |= BUTTON1_PRESSED;
    if (mask & FROM_LEFT_2ND_BUTTON_PRESSED)
	result |= BUTTON2_PRESSED;
    if (mask & FROM_LEFT_3RD_BUTTON_PRESSED)
	result |= BUTTON3_PRESSED;
    if (mask & FROM_LEFT_4TH_BUTTON_PRESSED)
	result |= BUTTON4_PRESSED;

    if (mask & RIGHTMOST_BUTTON_PRESSED) {
	switch (WINCONSOLE.numButtons) {
	case 1:
	    result |= BUTTON1_PRESSED;
	    break;
	case 2:
	    result |= BUTTON2_PRESSED;
	    break;
	case 3:
	    result |= BUTTON3_PRESSED;
	    break;
	case 4:
	    result |= BUTTON4_PRESSED;
	    break;
	}
    }

    return result;
}

static int
_nc_console_twait(
		     SCREEN *sp,
		     HANDLE fd,
		     int mode,
		     int milliseconds,
		     int *timeleft
		     EVENTLIST_2nd(_nc_eventlist * evl))
{
    INPUT_RECORD inp_rec;
    BOOL b;
    DWORD nRead = 0, rc = (DWORD) (-1);
    int code = 0;
    FILETIME fstart;
    FILETIME fend;
    int diff;
    bool isImmed = (milliseconds == 0);

#ifdef NCURSES_WGETCH_EVENTS
    (void) evl;			/* TODO: implement wgetch-events */
#endif

#define CONSUME() read_keycode(fd,&inp_rec,1,&nRead)

    assert(sp);

    TR(TRACE_IEVENT, ("start twait: %d milliseconds, mode: %d",
		      milliseconds, mode));

    if (milliseconds < 0)
	milliseconds = NC_INFINITY;

    memset(&inp_rec, 0, sizeof(inp_rec));

    while (true) {
	GetSystemTimeAsFileTime(&fstart);
	rc = WaitForSingleObject(fd, (DWORD) milliseconds);
	GetSystemTimeAsFileTime(&fend);
	diff = (int) tdiff(fstart, fend);
	milliseconds = Adjust(milliseconds, diff);

	if (!isImmed && milliseconds <= 0)
	    break;

	if (rc == WAIT_OBJECT_0) {
	    if (mode) {
		b = GetNumberOfConsoleInputEvents(fd, &nRead);
		if (b && nRead > 0) {
		    b = PeekConsoleInput(fd, &inp_rec, 1, &nRead);
		    if (b && nRead > 0) {
			switch (inp_rec.EventType) {
			case KEY_EVENT:
			    if (mode & TW_INPUT) {
				WORD vk = inp_rec.Event.KeyEvent.wVirtualKeyCode;
				WORD ch = inp_rec.Event.KeyEventChar;

				if (inp_rec.Event.KeyEvent.bKeyDown) {
				    if (0 == ch) {
					int nKey = MapKey(vk);
					if (nKey < 0) {
					    CONSUME();
					    continue;
					}
				    }
				    code = TW_INPUT;
				    goto end;
				} else {
				    CONSUME();
				}
			    }
			    continue;
			case MOUSE_EVENT:
			    if (decode_mouse(sp,
					     (inp_rec.Event.MouseEvent.dwButtonState
					      & BUTTON_MASK)) == 0) {
				CONSUME();
			    } else if (mode & TW_MOUSE) {
				code = TW_MOUSE;
				goto end;
			    }
			    continue;
			    /* e.g., FOCUS_EVENT */
			default:
			    CONSUME();
			    _nc_console_selectActiveHandle();
			    continue;
			}
		    }
		}
	    }
	    continue;
	} else {
	    if (rc != WAIT_TIMEOUT) {
		code = -1;
		break;
	    } else {
		code = 0;
		break;
	    }
	}
    }
  end:

    TR(TRACE_IEVENT, ("end twait: returned %d (%d), remaining time %d msec",
		      code, errno, milliseconds));

    if (timeleft)
	*timeleft = milliseconds;

    return code;
}

static int
wcon_twait(TERMINAL_CONTROL_BLOCK * TCB,
	   int mode,
	   int milliseconds,
	   int *timeleft
	   EVENTLIST_2nd(_nc_eventlist * evl))
{
    SCREEN *sp;
    int code = 0;

    if (validateConsoleHandle()) {
	SetSP();

	code = _nc_console_twait(sp,
				 WINCONSOLE.inp,
				 mode,
				 milliseconds,
				 timeleft EVENTLIST_2nd(evl));
    }
    return code;
}

static int
wcon_read(TERMINAL_CONTROL_BLOCK * TCB, int *buf)
{
    SCREEN *sp;
    int n = -1;

    T((T_CALLED("win32con::wcon_read(%p)"), TCB));

    assert(buf);
    if (validateConsoleHandle()) {
	SetSP();

	n = _nc_console_read(sp, WINCONSOLE.inp, buf);
    }
    returnCode(n);
}

static int
wcon_nap(TERMINAL_CONTROL_BLOCK * TCB GCC_UNUSED, int ms)
{
    T((T_CALLED("win32con::wcon_nap(%p, %d)"), TCB, ms));
    Sleep((DWORD) ms);
    returnCode(OK);
}

static int
wcon_cursorSet(TERMINAL_CONTROL_BLOCK * TCB GCC_UNUSED, int mode)
{
    int res = -1;

    T((T_CALLED("win32con:wcon_cursorSet(%d)"), mode));
    if (validateConsoleHandle()) {
	CONSOLE_CURSOR_INFO this_CI = WINCONSOLE.save_CI;
	switch (mode) {
	case 0:
	    this_CI.bVisible = FALSE;
	    break;
	case 1:
	    break;
	case 2:
	    this_CI.dwSize = 100;
	    break;
	}
	SetConsoleCursorInfo(WINCONSOLE.hdl, &this_CI);
    }
    returnCode(res);
}

static bool
wcon_kyExist(TERMINAL_CONTROL_BLOCK * TCB GCC_UNUSED, int keycode)
{
    WORD nKey;
    void *res;
    bool found = FALSE;
    LONG key = GenMap(0, (WORD) keycode);

    T((T_CALLED("win32con::wcon_kyExist(%d)"), keycode));
    res = bsearch(&key,
		  WINCONSOLE.rmap,
		  (size_t) (N_INI + FKEYS),
		  sizeof(keylist[0]),
		  rkeycompare);
    if (res) {
	key = *((LONG *) res);
	nKey = LOWORD(key);
	if (!(nKey & 0x8000))
	    found = TRUE;
    }
    returnCode(found);
}

static int
wcon_kpad(TERMINAL_CONTROL_BLOCK * TCB, int flag GCC_UNUSED)
{
    SCREEN *sp;
    int code = ERR;

    T((T_CALLED("win32con::wcon_kpad(%p, %d)"), TCB, flag));

    if (validateConsoleHandle()) {
	SetSP();

	if (sp) {
	    code = OK;
	}
    }
    returnCode(code);
}

static int
wcon_keyok(TERMINAL_CONTROL_BLOCK * TCB,
	   int keycode,
	   int flag)
{
    int code = ERR;
    SCREEN *sp;
    WORD nKey;
    WORD vKey;
    void *res;
    LONG key = GenMap(0, (WORD) keycode);

    T((T_CALLED("win32con::wcon_keyok(%p, %d, %d)"), TCB, keycode, flag));

    if (validateConsoleHandle()) {
	SetSP();

	if (sp) {
	    res = bsearch(&key,
			  WINCONSOLE.rmap,
			  (size_t) (N_INI + FKEYS),
			  sizeof(keylist[0]),
			  rkeycompare);
	    if (res) {
		key = *((LONG *) res);
		vKey = HIWORD(key);
		nKey = (LOWORD(key)) & 0x7fff;
		if (!flag)
		    nKey |= 0x8000;
		*(LONG *) res = GenMap(vKey, nKey);
	    }
	}
    }
    returnCode(code);
}

NCURSES_EXPORT_VAR (TERM_DRIVER) _nc_WIN_DRIVER = {
    FALSE,
	wcon_name,		/* Name          */
	wcon_CanHandle,		/* CanHandle     */
	wcon_init,		/* init          */
	wcon_release,		/* release       */
	wcon_size,		/* size          */
	wcon_sgmode,		/* sgmode        */
	wcon_conattr,		/* conattr       */
	wcon_mvcur,		/* hwcur         */
	wcon_mode,		/* mode          */
	wcon_rescol,		/* rescol        */
	wcon_rescolors,		/* rescolors     */
	wcon_setcolor,		/* color         */
	wcon_dobeepflash,	/* DoBeepFlash   */
	wcon_initpair,		/* initpair      */
	wcon_initcolor,		/* initcolor     */
	wcon_do_color,		/* docolor       */
	wcon_initmouse,		/* initmouse     */
	wcon_testmouse,		/* testmouse     */
	wcon_setfilter,		/* setfilter     */
	wcon_hwlabel,		/* hwlabel       */
	wcon_hwlabelOnOff,	/* hwlabelOnOff  */
	wcon_doupdate,		/* update        */
	wcon_defaultcolors,	/* defaultcolors */
	wcon_print,		/* print         */
	wcon_size,		/* getsize       */
	wcon_setsize,		/* setsize       */
	wcon_initacs,		/* initacs       */
	wcon_screen_init,	/* scinit        */
	wcon_wrap,		/* scexit        */
	wcon_twait,		/* twait         */
	wcon_read,		/* read          */
	wcon_nap,		/* nap           */
	wcon_kpad,		/* kpad          */
	wcon_keyok,		/* kyOk          */
	wcon_kyExist,		/* kyExist       */
	wcon_cursorSet		/* cursorSet     */
};

/* --------------------------------------------------------- */

static HANDLE
get_handle(int fd)
{
    intptr_t value = _get_osfhandle(fd);
    return (HANDLE) value;
}

/*   Borrowed from ansicon project.
     Check whether or not an I/O handle is associated with
     a Windows console.
*/
static BOOL
IsConsoleHandle(HANDLE hdl)
{
    DWORD dwFlag = 0;
    BOOL result;

    if (!GetConsoleMode(hdl, &dwFlag)) {
	result = (int) WriteConsoleA(hdl, NULL, 0, &dwFlag, NULL);
    } else {
	result = (int) (dwFlag & ENABLE_PROCESSED_OUTPUT);
    }
    return result;
}

/*   Our replacement for the systems _isatty to include also
     a test for mintty. This is called from the NC_ISATTY macro
     defined in curses.priv.h
 */
NCURSES_EXPORT(int)
_nc_mingw_isatty(int fd)
{
    int result = 0;

    if (SysISATTY(fd)) {
	result = 1;
    } else {
#if WINVER >= 0x0600
	result = _nc_console_checkmintty(fd, NULL);
#endif
    }
    return result;
}

/*   This is used when running in terminfo mode to discover,
     whether or not the "terminal" is actually a Windows
     Console. It is the responsibility of the console to deal
     with the terminal escape sequences that are sent by
     terminfo.
 */
NCURSES_EXPORT(int)
_nc_console_test(int fd)
{
    HANDLE hdl = get_handle(fd);
    int code = 0;

    T((T_CALLED("win32con::_nc_console_test(%d)"), fd));

    code = (int) IsConsoleHandle(hdl);

    returnCode(code);
}

#define TC_PROLOGUE(fd) \
    SCREEN *sp;                                               \
    TERMINAL *term = NULL;                                    \
    int code = ERR;                                           \
    if (_nc_screen_chain == NULL)                             \
        return 0;                                             \
    for (each_screen(sp)) {                                   \
        if (sp->_term && (sp->_term->Filedes == fd)) {        \
            term = sp->_term;                                 \
            break;                                            \
        }                                                     \
    }                                                         \
    assert(term != NULL)

NCURSES_EXPORT(int)
_nc_mingw_tcsetattr(
		       int fd,
		       int optional_action GCC_UNUSED,
		       const struct termios *arg)
{
    TC_PROLOGUE(fd);

    if (_nc_console_test(fd)) {
	DWORD dwFlag = 0;
	HANDLE ofd = get_handle(fd);
	if (ofd != INVALID_HANDLE_VALUE) {
	    if (arg) {
		if (arg->c_lflag & ICANON)
		    dwFlag |= ENABLE_LINE_INPUT;
		else
		    dwFlag = dwFlag & (DWORD) (~ENABLE_LINE_INPUT);

		if (arg->c_lflag & ECHO)
		    dwFlag = dwFlag | ENABLE_ECHO_INPUT;
		else
		    dwFlag = dwFlag & (DWORD) (~ENABLE_ECHO_INPUT);

		if (arg->c_iflag & BRKINT)
		    dwFlag |= ENABLE_PROCESSED_INPUT;
		else
		    dwFlag = dwFlag & (DWORD) (~ENABLE_PROCESSED_INPUT);
	    }
	    dwFlag |= ENABLE_MOUSE_INPUT;
	    SetConsoleMode(ofd, dwFlag);
	    code = OK;
	}
    }
    if (arg)
	term->Nttyb = *arg;

    return code;
}

NCURSES_EXPORT(int)
_nc_mingw_tcgetattr(int fd, struct termios *arg)
{
    TC_PROLOGUE(fd);

    if (_nc_console_test(fd)) {
	if (arg)
	    *arg = term->Nttyb;
    }
    return code;
}

NCURSES_EXPORT(int)
_nc_mingw_tcflush(int fd, int queue)
{
    int code = ERR;

    if (_nc_console_test(fd)) {
	if (queue == TCIFLUSH) {
	    code = (FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE))
		    ? OK
		    : (int) GetLastError());
	}
    }
    return code;
}

NCURSES_EXPORT(int)
_nc_mingw_testmouse(
		       SCREEN *sp,
		       HANDLE fd,
		       int delay
		       EVENTLIST_2nd(_nc_eventlist * evl))
{
    int rc = 0;

    assert(sp);

    if (sp->_drv_mouse_head < sp->_drv_mouse_tail) {
	rc = TW_MOUSE;
    } else {
	rc = _nc_console_twait(sp,
			       fd,
			       TWAIT_MASK,
			       delay,
			       (int *) 0
			       EVENTLIST_2nd(evl));
    }
    return rc;
}

static bool
InitConsole(void)
{
    {
	/* initialize once, or not at all */
	if (!console_initialized) {
	    int i;
	    DWORD num_buttons;
	    WORD a;
	    BOOL buffered = TRUE;
	    BOOL b;

	    START_TRACE();

	    WINCONSOLE.map = (LPDWORD) malloc(sizeof(DWORD) * MAPSIZE);
	    WINCONSOLE.rmap = (LPDWORD) malloc(sizeof(DWORD) * MAPSIZE);
	    WINCONSOLE.ansi_map = (LPDWORD) malloc(sizeof(DWORD) * MAPSIZE);

	    for (i = 0; i < (N_INI + FKEYS); i++) {
		if (i < N_INI) {
		    WINCONSOLE.rmap[i] = WINCONSOLE.map[i] =
			(DWORD) keylist[i];
		    WINCONSOLE.ansi_map[i] = (DWORD) ansi_keys[i];
		} else {
		    WINCONSOLE.rmap[i] = WINCONSOLE.map[i] =
			(DWORD) GenMap((VK_F1 + (i - N_INI)),
				       (KEY_F(1) + (i - N_INI)));
		    WINCONSOLE.ansi_map[i] =
			(DWORD) GenMap((VK_F1 + (i - N_INI)),
				       (';' + (i - N_INI)));
		}
	    }
	    qsort(WINCONSOLE.ansi_map,
		  (size_t) (MAPSIZE),
		  sizeof(keylist[0]),
		  keycompare);
	    qsort(WINCONSOLE.map,
		  (size_t) (MAPSIZE),
		  sizeof(keylist[0]),
		  keycompare);
	    qsort(WINCONSOLE.rmap,
		  (size_t) (MAPSIZE),
		  sizeof(keylist[0]),
		  rkeycompare);

	    if (GetNumberOfConsoleMouseButtons(&num_buttons)) {
		WINCONSOLE.numButtons = (int) num_buttons;
	    } else {
		WINCONSOLE.numButtons = 1;
	    }

	    a = _nc_console_MapColor(true, COLOR_WHITE) |
		_nc_console_MapColor(false, COLOR_BLACK);
	    for (i = 0; i < CON_NUMPAIRS; i++)
		WINCONSOLE.pairs[i] = a;

	    {
		b = AllocConsole();

		if (!b)
		    b = AttachConsole(ATTACH_PARENT_PROCESS);

		WINCONSOLE.inp = GetDirectHandle("CONIN$", FILE_SHARE_READ);
		WINCONSOLE.out = GetDirectHandle("CONOUT$", FILE_SHARE_WRITE);

		if (getenv("NCGDB") || getenv("NCURSES_CONSOLE2")) {
		    T(("... will not buffer console"));
		    buffered = FALSE;
		    WINCONSOLE.hdl = WINCONSOLE.out;
		} else {
		    T(("... creating console buffer"));
		    WINCONSOLE.hdl = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
							       FILE_SHARE_READ | FILE_SHARE_WRITE,
							       NULL,
							       CONSOLE_TEXTMODE_BUFFER,
							       NULL);
		}

		if (WINCONSOLE.hdl != INVALID_HANDLE_VALUE) {
		    WINCONSOLE.buffered = buffered;
		    _nc_console_get_SBI();
		    WINCONSOLE.save_SBI = WINCONSOLE.SBI;
		    if (!buffered) {
			save_original_screen();
			_nc_console_set_scrollback(FALSE, &WINCONSOLE.SBI);
		    }
		    GetConsoleCursorInfo(WINCONSOLE.hdl, &WINCONSOLE.save_CI);
		    T(("... initial cursor is %svisible, %d%%",
		       (WINCONSOLE.save_CI.bVisible ? "" : "not-"),
		       (int) WINCONSOLE.save_CI.dwSize));
		}
	    }

	    console_initialized = TRUE;
	}
    }
    return (WINCONSOLE.hdl != INVALID_HANDLE_VALUE);
}
