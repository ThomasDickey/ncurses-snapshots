
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/

/* lib_color.c 
 *  
 * Handles color emulation of SYS V curses
 *
 */

#include <stdlib.h>
#include "curses.priv.h"
#include "term.h"

int COLOR_PAIRS;
int COLORS;
unsigned char *color_pairs;

typedef struct
{
    short red, green, blue;
}
color_t;
static color_t	*color_table;

static const color_t cga_palette[] =
{
    /*  R	G	B */
	{0,	0,	0},	/* COLOR_BLACK */
	{1000,	0,	0},	/* COLOR_RED */
	{0,	1000,	0},	/* COLOR_GREEN */
	{1000,	1000,	0},	/* COLOR_YELLOW */
	{0,	0,	1000},	/* COLOR_BLUE */
	{1000,	0,	1000},	/* COLOR_MAGENTA */
	{0,	1000,	1000},	/* COLOR_CYAN */
	{1000,	1000,	1000},	/* COLOR_WHITE */
};
static const color_t hls_palette[] =
{
    /*  H	L	S */
	{0,	0,	0},	/* COLOR_BLACK */
	{120,	50,	100},	/* COLOR_RED */
	{240,	50,	100},	/* COLOR_GREEN */
	{180,	50,	100},	/* COLOR_YELLOW */
	{330,	50,	100},	/* COLOR_BLUE */
	{60,	50,	100},	/* COLOR_MAGENTA */
	{300,	50,	100},	/* COLOR_CYAN */
	{0,	50,	100},	/* COLOR_WHITE */
};

int start_color(void)
{
	T(("start_color() called."));

	if (orig_pair != NULL)
	{
		TPUTS_TRACE("orig_pair");
		putp(orig_pair);
	}
	else return ERR;
	if (max_pairs != -1)
		COLOR_PAIRS = max_pairs;
	else return ERR;
	color_pairs = calloc(sizeof(char), max_pairs);
	if (max_colors != -1)
		COLORS = max_colors;
	else
		return ERR;
	SP->_coloron = 1;

	if (can_change)
	{
	    color_table = malloc(sizeof(color_t) * COLORS);
	    if (hue_lightness_saturation)
		memcpy(color_table, hls_palette, sizeof(color_t) * COLORS);
	    else
		memcpy(color_table, cga_palette, sizeof(color_t) * COLORS);
	}

	T(("started color: COLORS = %d, COLOR_PAIRS = %d", COLORS, COLOR_PAIRS));

	return OK;
}

static void rgb2hls(short r, short g, short b, short *h, short *l, short *s)
/* convert RGB to HLS system */
{
    int min, max, t;

    if ((min = g < r ? g : r) > b) min = b;
    if ((max = g > r ? g : r) < b) max = b;

    /* calculate lightness */
    *l = (min + max) / 20;

    if (min == max)
    {				/* black, white and all shades of gray */
	*h = 0;
	*s = 0;
	return;
    }

    /* calculate saturation */
    if (*l < 50)
	*s = ((max - min) * 100) / (max + min);
    else *s = ((max - min) * 100) / (2000 - max - min);

    /* calculate hue */
    if (r == max)
	t = 120 + ((g - b) * 60) / (max - min);
    else
	if (g == max)
	    t = 240 + ((b - r) * 60) / (max - min);
	else
	    t = 360 + ((r - g) * 60) / (max - min);

    *h = t % 360;
}

int init_pair(short pair, short f, short b)
{
	T(("init_pair( %d, %d, %d )", pair, f, b));

	if ((pair < 1) || (pair >= COLOR_PAIRS))
		return ERR;
	if ((f  < 0) || (f >= COLORS) || (b < 0) || (b >= COLORS))
		return ERR;

	/* still to do:
	   if pair was initialized before a screen update is performed
	   replacing original pair colors with the new ones
	*/

	color_pairs[pair] = ( (f & 0x0f) | (b & 0x0f) << 4 );

	return OK;
}

int init_color(short color, short r, short g, short b)
{
	if (initialize_color != NULL) {
		if (color < 0 || color >= COLORS)
			return ERR;
		if (hue_lightness_saturation == TRUE)
			if (r < 0 || r > 360 || g < 0 || g > 100 || b < 0 || b > 100)
				return ERR;	
		if (hue_lightness_saturation == FALSE)
			if (r < 0 || r > 1000 || g < 0 ||  g > 1000 || b < 0 || b > 1000)
				return ERR;
				
		if (hue_lightness_saturation)
		    rgb2hls(r, g, b,
			      &color_table[color].red,
			      &color_table[color].green,
			      &color_table[color].blue);
		else
		{
			color_table[color].red = r;
			color_table[color].green = g;
			color_table[color].blue = b;
		}

		TPUTS_TRACE("initialize_color");
		putp(tparm(initialize_color, color, r, g, b));
		return OK;
	}
	
	return ERR;
}

bool can_change_color(void)
{
	return can_change;
}

int has_colors(void)
{
	return ((orig_pair != NULL) && (max_colors != -1) && (max_pairs != -1)
		&& 
		(((set_foreground != NULL) && (set_background != NULL)) ||
		((set_a_foreground != NULL) && (set_a_background != NULL)))
		);
}

int color_content(short color, short *r, short *g, short *b)
{
    if (color < 0 || color > COLORS)
	return ERR;

    *r = color_table[color].red;
    *g = color_table[color].green;
    *b = color_table[color].blue;
    return OK;
}

int pair_content(short pair, short *f, short *b)
{

	if ((pair < 1) || (pair > COLOR_PAIRS))
		return ERR;
	*f = color_pairs[pair] & 0x0f;
	*b = color_pairs[pair] & 0xf0;
	*b >>= 4;
	return OK;
}

