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

#include "system.h"

#include "curses.priv.h"
#include "term.h"	/* ena_acs, acs_chars */
#include <string.h>

#define ASCII(c)	((chtype)(c) & A_CHARTEXT)
#define ALTCHAR(c)	((chtype)(c) & A_CHARTEXT) | A_ALTCHARSET
#ifdef A_PCCHARSET
#define PCCHAR(c)	((chtype)(c) & A_CHARTEXT) | A_PCCHARSET
#endif

chtype acs_map[128];

void init_acs(void)
{
	T(("initializing ACS map"));

#ifdef UNIX
	/*
	 * Initializations for a UNIX-like multi-terminal environment.  Use
	 * ASCII chars and count on the terminfo description to do better.
	 */
	ACS_ULCORNER = ASCII('+');	/* should be upper left corner */
	ACS_LLCORNER = ASCII('+');	/* should be lower left corner */
	ACS_URCORNER = ASCII('+');	/* should be upper right corner */
	ACS_LRCORNER = ASCII('+');	/* should be lower right corner */
	ACS_RTEE     = ASCII('+');	/* should be tee pointing left */
	ACS_LTEE     = ASCII('+');	/* should be tee pointing right */
	ACS_BTEE     = ASCII('+');	/* should be tee pointing up */
	ACS_TTEE     = ASCII('+');	/* should be tee pointing down */
	ACS_HLINE    = ASCII('-');	/* should be horizontal line */
	ACS_VLINE    = ASCII('|');	/* should be vertical line */
	ACS_PLUS     = ASCII('+');	/* should be large plus or crossover */
	ACS_S1       = ASCII('~');	/* should be scan line 1 */
	ACS_S9       = ASCII('_');	/* should be scan line 9 */
	ACS_DIAMOND  = ASCII('+');	/* should be diamond */
	ACS_CKBOARD  = ASCII(':');	/* should be checker board (stipple) */
	ACS_DEGREE   = ASCII('\'');	/* should be degree symbol */
	ACS_PLMINUS  = ASCII('#');	/* should be plus/minus */
	ACS_BULLET   = ASCII('o');	/* should be bullet */
	ACS_LARROW   = ASCII('<');	/* should be arrow pointing left */
	ACS_RARROW   = ASCII('>');	/* should be arrow pointing right */
	ACS_DARROW   = ASCII('v');	/* should be arrow pointing down */
	ACS_UARROW   = ASCII('^');	/* should be arrow pointing up */
	ACS_BOARD    = ASCII('#');	/* should be board of squares */
	ACS_LANTERN  = ASCII('#');	/* should be lantern symbol */
	ACS_BLOCK    = ASCII('#');	/* should be solid square block */
#endif /* UNIX */

#ifdef __i386__
	/*
	 * Character map initialization for Intel boxes
	 */
#ifdef enter_pc_charset_mode
	if (enter_pc_charset_mode)
#endif /* enter_pc_charset_mode */
	{
	/*
	 * IBM high-half and literal control characters to use if we have them
	 * available.  We'd like to load these from a terminfo entry, but there
	 * is no way to represent enclosure with both ALTCHARSET and PCCHARSET
	 * in terminfo's acsc capability.  We'd also like to use ROM chars 
	 * 26 and 27 for the left and right-pointing arrows, but typical
	 * UNIX console drivers (e.g., Linux) won't let us do it -- 27 is
	 * ESC, which gets treated as a control-sequence leader and swallowed.
	 */
	ACS_ULCORNER = ALTCHAR('Z');	/* IBM upper left corner */
	ACS_LLCORNER = ALTCHAR('@');	/* IBM lower left corner */
	ACS_URCORNER = ALTCHAR('?');	/* IBM upper right corner */
	ACS_LRCORNER = ALTCHAR('Y');	/* IBM lower right corner */
	ACS_LTEE     = ALTCHAR('C');	/* IBM tee pointing right */
	ACS_RTEE     = ALTCHAR('4');	/* IBM tee pointing left */
	ACS_BTEE     = ALTCHAR('A');	/* IBM tee pointing up */
	ACS_TTEE     = ALTCHAR('B');	/* IBM tee pointing down */
	ACS_HLINE    = ALTCHAR('D');	/* IBM horizontal line */
	ACS_VLINE    = ALTCHAR('3');	/* IBM vertical line */
	ACS_PLUS     = ALTCHAR('E');	/* IBM large plus or crossover */
	ACS_S1       = ASCII('~');	/* should be scan line 1 */
	ACS_S9       = ASCII('_');	/* should be scan line 9 */
#ifdef A_PCCHARSET
	ACS_DIAMOND  = PCCHAR(4);	/* IBM diamond */
#endif /* A_PCCCHARSET */
	ACS_CKBOARD  = ALTCHAR('2');	/* IBM checker board (stipple) */
	ACS_DEGREE   = ALTCHAR('x');	/* IBM degree symbol */
	ACS_PLMINUS  = ALTCHAR('q');	/* IBM plus/minus */
	ACS_BULLET   = ALTCHAR('~');	/* IBM bullet (actually small block) */
#ifdef A_PCCHARSET
	ACS_LARROW   = PCCHAR(17);	/* IBM arrow pointing left */
	ACS_RARROW   = PCCHAR(16);	/* IBM arrow pointing right */
	ACS_DARROW   = PCCHAR(25);	/* IBM arrow pointing down */
	ACS_UARROW   = PCCHAR(24);	/* IBM arrow pointing up */
#endif /* A_PCCCHARSET */
	ACS_BOARD    = ALTCHAR('0');	/* IBM board of squares */
	ACS_LANTERN  = ASCII('#');	/* should be lantern symbol */
	ACS_BLOCK    = ALTCHAR('[');	/* IBM solid square block */
	}
#endif /* __i386__ */

#ifdef ena_acs
	if (ena_acs != NULL)
	{
		TPUTS_TRACE("ena_acs");
		putp(ena_acs);
	}
#endif /* ena_acs */

#ifdef acs_chars
	if (acs_chars != NULL) {
	    int i = 0;
	    int length = strlen(acs_chars);
	    
		while (i < length) 
			switch (acs_chars[i]) {
			case 'l':case 'm':case 'k':case 'j':
			case 'u':case 't':case 'v':case 'w':
			case 'q':case 'x':case 'n':case 'o':
			case 's':case '`':case 'a':case 'f':
			case 'g':case '~':case ',':case '+':
			case '.':case '-':case 'h':case 'I':
			case '0': 
				acs_map[(unsigned int)acs_chars[i]] = 
					ALTCHAR(acs_chars[++i]);
			default:
				i++;
				break;
			}
	}
#ifdef TRACE
	else {
		T(("acsc not defined, using default mapping"));
	}
#endif /* TRACE */
#endif /* acs_char */
}

