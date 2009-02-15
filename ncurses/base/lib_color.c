/****************************************************************************
 * Copyright (c) 1998-2008,2009 Free Software Foundation, Inc.              *
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

/* lib_color.c
 *
 * Handles color emulation of SYS V curses
 */

#include <curses.priv.h>
#include <tic.h>

MODULE_ID("$Id: lib_color.c,v 1.87.1.3 2009/02/14 20:50:52 tom Exp $")

#define DATA(r,g,b) {r,g,b, 0,0,0, 0}

#define TYPE_CALLOC(type,elts) typeCalloc(type, (unsigned)(elts))

#define MAX_PALETTE	8

#define OkColorHi(n)	(((n) < COLORS) && ((n) < maxcolors))
#define InPalette(n)	((n) >= 0 && (n) < MAX_PALETTE)

/*
 * Given a RGB range of 0..1000, we'll normally set the individual values
 * to about 2/3 of the maximum, leaving full-range for bold/bright colors.
 */
#define RGB_ON  680
#define RGB_OFF 0
/* *INDENT-OFF* */
static const color_t cga_palette[] =
{
    /*  R               G               B */
    DATA(RGB_OFF,	RGB_OFF,	RGB_OFF),	/* COLOR_BLACK */
    DATA(RGB_ON,	RGB_OFF,	RGB_OFF),	/* COLOR_RED */
    DATA(RGB_OFF,	RGB_ON,		RGB_OFF),	/* COLOR_GREEN */
    DATA(RGB_ON,	RGB_ON,		RGB_OFF),	/* COLOR_YELLOW */
    DATA(RGB_OFF,	RGB_OFF,	RGB_ON),	/* COLOR_BLUE */
    DATA(RGB_ON,	RGB_OFF,	RGB_ON),	/* COLOR_MAGENTA */
    DATA(RGB_OFF,	RGB_ON,		RGB_ON),	/* COLOR_CYAN */
    DATA(RGB_ON,	RGB_ON,		RGB_ON),	/* COLOR_WHITE */
};

static const color_t hls_palette[] =
{
    /*  	H       L       S */
    DATA(	0,	0,	0),		/* COLOR_BLACK */
    DATA(	120,	50,	100),		/* COLOR_RED */
    DATA(	240,	50,	100),		/* COLOR_GREEN */
    DATA(	180,	50,	100),		/* COLOR_YELLOW */
    DATA(	330,	50,	100),		/* COLOR_BLUE */
    DATA(	60,	50,	100),		/* COLOR_MAGENTA */
    DATA(	300,	50,	100),		/* COLOR_CYAN */
    DATA(	0,	50,	100),		/* COLOR_WHITE */
};

NCURSES_EXPORT_VAR(const color_t*) _nc_cga_palette = cga_palette;
NCURSES_EXPORT_VAR(const color_t*) _nc_hls_palette = hls_palette;

/* *INDENT-ON* */

/*
 * Ensure that we use color pairs only when colors have been started, and also
 * that the index is within the limits of the table which we allocated.
 */
#define ValidPair(pair) \
    ((SP_PARM != 0) && (pair >= 0) && (pair < SP_PARM->_pair_limit) && SP_PARM->_coloron)

#if NCURSES_EXT_FUNCS
/*
 * These are called from _nc_do_color(), which in turn is called from
 * vidattr - so we have to assume that sp may be null.
 */
static int
default_fg(SCREEN *sp)
{
    return (sp != 0) ? sp->_default_fg : COLOR_WHITE;
}

static int
default_bg(SCREEN *sp)
{
    return sp != 0 ? sp->_default_bg : COLOR_BLACK;
}
#else
#define default_fg(sp) COLOR_WHITE
#define default_bg(sp) COLOR_BLACK
#endif

static void
set_background_color(SCREEN *sp, int bg, int (*outc) (SCREEN *, int))
{
    CallDriver_3(sp, color, FALSE, bg, outc);
}

static void
set_foreground_color(SCREEN *sp, int fg, int (*outc) (SCREEN *, int))
{
    CallDriver_3(sp, color, TRUE, fg, outc);
}

static void
init_color_table(SCREEN *sp)
{
    const color_t *tp = InfoOf(sp).defaultPalette;
    int n;

    assert(tp != 0);

    for (n = 0; n < COLORS; n++) {
	if (InPalette(n)) {
	    sp->_color_table[n] = tp[n];
	} else {
	    sp->_color_table[n] = tp[n % MAX_PALETTE];
	    if (tp == _nc_hls_palette) {
		sp->_color_table[n].green = 100;
	    } else {
		if (sp->_color_table[n].red)
		    sp->_color_table[n].red = 1000;
		if (sp->_color_table[n].green)
		    sp->_color_table[n].green = 1000;
		if (sp->_color_table[n].blue)
		    sp->_color_table[n].blue = 1000;
	    }
	}
    }
}

/*
 * Reset the color pair, e.g., to whatever color pair 0 is.
 */
static bool
reset_color_pair(SCREEN *sp)
{
    return CallDriver(sp, rescol);
}

/*
 * Reset color pairs and definitions.  Actually we do both more to accommodate
 * badly-written terminal descriptions than for the relatively rare case where
 * someone has changed the color definitions.
 */
NCURSES_EXPORT(bool)
NC_SNAME(_nc_reset_colors) (SCREEN *sp)
{
    int result = FALSE;

    T((T_CALLED("_nc_reset_colors(%p)"), sp));
    if (sp->_color_defs > 0)
	sp->_color_defs = -(sp->_color_defs);
    if (reset_color_pair(sp))
	result = TRUE;

    result = CallDriver(sp, rescolors);
    returnBool(result);
}

NCURSES_EXPORT(bool)
_nc_reset_colors(void)
{
    return NC_SNAME(_nc_reset_colors) (CURRENT_SCREEN);
}

NCURSES_EXPORT(int)
NCURSES_SP_NAME(start_color) (NCURSES_SP_DCL)
{
    int result = ERR;
    int maxpairs = 0, maxcolors = 0;

    T((T_CALLED("start_color(%p)"), SP_PARM));

    if (SP_PARM == 0) {
	result = ERR;
    } else if (SP_PARM->_coloron) {
	result = OK;
    } else {
	maxpairs = InfoOf(SP_PARM).maxpairs;
	maxcolors = InfoOf(SP_PARM).maxcolors;
	if (reset_color_pair(SP_PARM) != TRUE) {
	    set_foreground_color(SP_PARM, default_fg(SP_PARM), NC_SNAME(_nc_outch));
	    set_background_color(SP_PARM, default_bg(SP_PARM), NC_SNAME(_nc_outch));
	}

	if (maxpairs > 0 && maxcolors > 0) {
	    SP_PARM->_pair_limit = maxpairs;

#if NCURSES_EXT_FUNCS
	    /*
	     * If using default colors, allocate extra space in table to
	     * allow for default-color as a component of a color-pair.
	     */
	    SP_PARM->_pair_limit += (1 + (2 * maxcolors));
#endif
	    SP_PARM->_pair_count = maxpairs;
	    SP_PARM->_color_count = maxcolors;
#if !USE_REENTRANT
	    COLOR_PAIRS = maxpairs;
	    COLORS = maxcolors;
#endif

	    SP_PARM->_color_pairs = TYPE_CALLOC(colorpair_t, SP_PARM->_pair_limit);
	    if (SP_PARM->_color_pairs != 0) {
		SP_PARM->_color_table = TYPE_CALLOC(color_t, maxcolors);
		if (SP_PARM->_color_table != 0) {
		    SP_PARM->_color_pairs[0] = PAIR_OF(default_fg(SP_PARM),
						       default_bg(SP_PARM));
		    init_color_table(SP_PARM);

		    T(("started color: COLORS = %d, COLOR_PAIRS = %d",
		       COLORS, COLOR_PAIRS));

		    SP_PARM->_coloron = 1;
		    result = OK;
		} else if (SP_PARM->_color_pairs != 0) {
		    FreeAndNull(SP_PARM->_color_pairs);
		}
	    }
	} else {
	    result = OK;
	}
    }
    returnCode(result);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
start_color(void)
{
    return NC_SNAME(start_color) (CURRENT_SCREEN);
}
#endif

/* This function was originally written by Daniel Weaver <danw@znyx.com> */
static void
rgb2hls(short r, short g, short b, short *h, short *l, short *s)
/* convert RGB to HLS system */
{
    short min, max, t;

    if ((min = g < r ? g : r) > b)
	min = b;
    if ((max = g > r ? g : r) < b)
	max = b;

    /* calculate lightness */
    *l = (min + max) / 20;

    if (min == max) {		/* black, white and all shades of gray */
	*h = 0;
	*s = 0;
	return;
    }

    /* calculate saturation */
    if (*l < 50)
	*s = ((max - min) * 100) / (max + min);
    else
	*s = ((max - min) * 100) / (2000 - max - min);

    /* calculate hue */
    if (r == max)
	t = 120 + ((g - b) * 60) / (max - min);
    else if (g == max)
	t = 240 + ((b - r) * 60) / (max - min);
    else
	t = 360 + ((r - g) * 60) / (max - min);

    *h = t % 360;
}

/*
 * Extension (1997/1/18) - Allow negative f/b values to set default color
 * values.
 */
NCURSES_EXPORT(int)
NCURSES_SP_NAME(init_pair) (NCURSES_SP_DCLx short pair, short f, short b)
{
    colorpair_t result;
    colorpair_t previous;
    int maxcolors;

    T((T_CALLED("init_pair(%p,%d,%d,%d)"), SP_PARM, pair, f, b));

    if (!ValidPair(pair))
	returnCode(ERR);

    maxcolors = InfoOf(SP_PARM).maxcolors;

    previous = SP_PARM->_color_pairs[pair];
#if NCURSES_EXT_FUNCS
    if (SP_PARM->_default_color) {
	bool isDefault = FALSE;
	bool wasDefault = FALSE;
	int default_pairs = SP_PARM->_default_pairs;

	/*
	 * Map caller's color number, e.g., -1, 0, 1, .., 7, etc., into
	 * internal unsigned values which we will store in the _color_pairs[]
	 * table.
	 */
	if (isDefaultColor(f)) {
	    f = COLOR_DEFAULT;
	    isDefault = TRUE;
	} else if (!OkColorHi(f)) {
	    returnCode(ERR);
	}

	if (isDefaultColor(b)) {
	    b = COLOR_DEFAULT;
	    isDefault = TRUE;
	} else if (!OkColorHi(b)) {
	    returnCode(ERR);
	}

	/*
	 * Check if the table entry that we are going to init/update used
	 * default colors.
	 */
	if ((FORE_OF(previous) == COLOR_DEFAULT)
	    || (BACK_OF(previous) == COLOR_DEFAULT))
	    wasDefault = TRUE;

	/*
	 * Keep track of the number of entries in the color pair table which
	 * used a default color.
	 */
	if (isDefault && !wasDefault) {
	    ++default_pairs;
	} else if (wasDefault && !isDefault) {
	    --default_pairs;
	}

	/*
	 * As an extension, ncurses allows the pair number to exceed the
	 * terminal's color_pairs value for pairs using a default color.
	 *
	 * Note that updating a pair which used a default color with one
	 * that does not will decrement the count - and possibly interfere
	 * with sequentially adding new pairs.
	 */
	if (pair > (SP_PARM->_pair_count + default_pairs)) {
	    returnCode(ERR);
	}
	SP_PARM->_default_pairs = default_pairs;
    } else
#endif
    {
	if ((f < 0) || !OkColorHi(f)
	    || (b < 0) || !OkColorHi(b)
	    || (pair < 1))
	    returnCode(ERR);
    }

    /*
     * When a pair's content is changed, replace its colors (if pair was
     * initialized before a screen update is performed replacing original
     * pair colors with the new ones).
     */
    result = PAIR_OF(f, b);
    if (previous != 0
	&& previous != result) {
	int y, x;

	for (y = 0; y <= SP_PARM->_curscr->_maxy; y++) {
	    struct ldat *ptr = &(SP_PARM->_curscr->_line[y]);
	    bool changed = FALSE;
	    for (x = 0; x <= SP_PARM->_curscr->_maxx; x++) {
		if (GetPair(ptr->text[x]) == pair) {
		    /* Set the old cell to zero to ensure it will be
		       updated on the next doupdate() */
		    SetChar(ptr->text[x], 0, 0);
		    CHANGED_CELL(ptr, x);
		    changed = TRUE;
		}
	    }
	    if (changed)
		NC_SNAME(_nc_make_oldhash) (SP_PARM, y);
	}
    }

    SP_PARM->_color_pairs[pair] = result;
    if (GET_SCREEN_PAIR(SP_PARM) == pair)
	SET_SCREEN_PAIR(SP_PARM, (chtype) (~0));	/* force attribute update */

    CallDriver_3(SP_PARM, initpair, pair, f, b);

    returnCode(OK);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
init_pair(short pair, short f, short b)
{
    return NC_SNAME(init_pair) (CURRENT_SCREEN, pair, f, b);
}
#endif

#define okRGB(n) ((n) >= 0 && (n) <= 1000)

NCURSES_EXPORT(int)
NCURSES_SP_NAME(init_color) (NCURSES_SP_DCLx
			     short color, short r, short g, short b)
{
    int result = ERR;
    int maxcolors;

    T((T_CALLED("init_color(%p,%d,%d,%d,%d)"), SP_PARM, color, r, g, b));

    if (SP_PARM == 0)
	returnCode(result);

    maxcolors = InfoOf(SP_PARM).maxcolors;

    if (InfoOf(SP_PARM).initcolor
	&& SP_PARM->_coloron
	&& (color >= 0 && OkColorHi(color))
	&& (okRGB(r) && okRGB(g) && okRGB(b))) {

	SP_PARM->_color_table[color].init = 1;
	SP_PARM->_color_table[color].r = r;
	SP_PARM->_color_table[color].g = g;
	SP_PARM->_color_table[color].b = b;

	if (InfoOf(SP_PARM).defaultPalette == _nc_hls_palette) {
	    rgb2hls(r, g, b,
		    &SP_PARM->_color_table[color].red,
		    &SP_PARM->_color_table[color].green,
		    &SP_PARM->_color_table[color].blue);
	} else {
	    SP_PARM->_color_table[color].red = r;
	    SP_PARM->_color_table[color].green = g;
	    SP_PARM->_color_table[color].blue = b;
	}

	CallDriver_4(SP_PARM, initcolor, color, r, g, b);
	SP_PARM->_color_defs = max(color + 1, SP_PARM->_color_defs);

	result = OK;
    }
    returnCode(result);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
init_color(short color, short r, short g, short b)
{
    return NC_SNAME(init_color) (CURRENT_SCREEN, color, r, g, b);
}
#endif

NCURSES_EXPORT(bool)
NCURSES_SP_NAME(can_change_color) (NCURSES_SP_DCL)
{
    T((T_CALLED("can_change_color(%p)"), SP_PARM));
    returnCode((InfoOf(SP_PARM).canchange != 0) ? TRUE : FALSE);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(bool)
can_change_color(void)
{
    return NC_SNAME(can_change_color) (CURRENT_SCREEN);
}
#endif

NCURSES_EXPORT(bool)
NCURSES_SP_NAME(has_colors) (NCURSES_SP_DCL)
{

    T((T_CALLED("has_colors()")));
    returnCode(InfoOf(SP_PARM).hascolor);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(bool)
has_colors(void)
{
    return NC_SNAME(has_colors) (CURRENT_SCREEN);
}
#endif

NCURSES_EXPORT(int)
NCURSES_SP_NAME(color_content) (NCURSES_SP_DCLx
				short color, short *r, short *g, short *b)
{
    int result = ERR;
    int maxcolors;

    T((T_CALLED("color_content(%p,%d,%p,%p,%p)"), SP_PARM, color, r, g, b));

    if (SP_PARM == 0)
	returnCode(result);

    maxcolors = InfoOf(SP_PARM).maxcolors;

    if (color < 0 || !OkColorHi(color) || !SP_PARM->_coloron) {
	result = ERR;
    } else {
	NCURSES_COLOR_T c_r = SP_PARM->_color_table[color].red;
	NCURSES_COLOR_T c_g = SP_PARM->_color_table[color].green;
	NCURSES_COLOR_T c_b = SP_PARM->_color_table[color].blue;

	if (r)
	    *r = c_r;
	if (g)
	    *g = c_g;
	if (b)
	    *b = c_b;

	TR(TRACE_ATTRS, ("...color_content(%d,%d,%d,%d)",
			 color, c_r, c_g, c_b));
	result = OK;
    }
    returnCode(result);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
color_content(short color, short *r, short *g, short *b)
{
    return NC_SNAME(color_content) (CURRENT_SCREEN, color, r, g, b);
}
#endif

NCURSES_EXPORT(int)
NCURSES_SP_NAME(pair_content) (NCURSES_SP_DCLx
			       short pair, short *f, short *b)
{
    int result;

    T((T_CALLED("pair_content(%p,%d,%p,%p)"), SP_PARM, pair, f, b));

    if (!ValidPair(pair)) {
	result = ERR;
    } else {
	NCURSES_COLOR_T fg = FORE_OF(SP_PARM->_color_pairs[pair]);
	NCURSES_COLOR_T bg = BACK_OF(SP_PARM->_color_pairs[pair]);

#if NCURSES_EXT_FUNCS
	if (fg == COLOR_DEFAULT)
	    fg = -1;
	if (bg == COLOR_DEFAULT)
	    bg = -1;
#endif

	if (f)
	    *f = fg;
	if (b)
	    *b = bg;

	TR(TRACE_ATTRS, ("...pair_content(%p,%d,%d,%d)", SP_PARM, pair, fg, bg));
	result = OK;
    }
    returnCode(result);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(int)
pair_content(short pair, short *f, short *b)
{
    return NC_SNAME(pair_content) (CURRENT_SCREEN, pair, f, b);
}
#endif

NCURSES_EXPORT(void)
NCURSES_SP_NAME(_nc_do_color) (NCURSES_SP_DCLx
			       short old_pair,
			       short pair,
			       bool reverse,
			       int (*outc) (NCURSES_SP_DCLx int _ch))
{
    CallDriver_4(SP_PARM, docolor, old_pair, pair, reverse, outc);
}

#if NCURSES_SP_FUNCS
NCURSES_EXPORT(void)
_nc_do_color(short old_pair, short pair, bool reverse, int (*outc) (int))
{
    SetSafeOutcWrapper(outc);
    NC_SNAME(_nc_do_color) (CURRENT_SCREEN, old_pair, pair, reverse, _nc_outc_wrapper);
}
#endif
