
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

/*
 *	vidputs(newmode, outc)
 *
 *	newmode is taken to be the logical 'or' of the symbols in curses.h
 *	representing graphic renditions.  The teminal is set to be in all of
 *	the given modes, if possible.
 *
 *	if the new attribute is normal
 *		if exit-alt-char-set exists
 *			emit it
 *		emit exit-attribute-mode
 *	else if set-attributes exists
 *		use it to set exactly what you want
 *	else
 *		if exit-attribute-mode exists
 *			turn off everything
 *		else
 *			turn off those which can be turned off and aren't in
 *			newmode.
 *		turn on each mode which should be on and isn't, one by one
 *
 *	NOTE that this algorithm won't achieve the desired mix of attributes
 *	in some cases, but those are probably just those cases in which it is
 *	actually impossible, anyway, so...
 *
 *	Special nasty kluge for Intel boxes: Currently, the A_PCCHARSET
 *	highlight is identified with A_PROTECT (this is to avoid taking
 *	up attr_t bits above 9, which the XSI Curses standard uses for
 *	internationalization attributes).  In case this ever ceases to 
 *	be true, explicit A_PCCHARSET code has been left in place which
 *	will only be compiled if A_PCCHARSET != A_PROTECT.
 *
 */

#include "curses.priv.h"
#include <string.h>
#include "term.h"

static void do_color(int pair, int  (*outc)(int))
{
short fg, bg;

	if ( pair == 0 ) {
		TPUTS_TRACE("orig_pair");
		tputs(orig_pair, 1, outc);
	} else {
		pair_content(pair, &fg, &bg);

		T(("setting colors: pair = %d, fg = %d, bg = %d\n", pair, fg, bg));

		if (set_a_foreground) {
		    TPUTS_TRACE("set_a_foreground");
		    tputs(tparm(set_a_foreground, fg), 1, outc);
		} else {
		    TPUTS_TRACE("set_foreground");
		    tputs(tparm(set_foreground, fg), 1, outc);
		}
		if (set_a_background) {
		    TPUTS_TRACE("set_a_background");
		    tputs(tparm(set_a_background, bg), 1, outc);
		} else {
		    TPUTS_TRACE("set_background");
		    tputs(tparm(set_background, bg), 1, outc);
		}
	}
}

#define previous_attr SP->_current_attr

int vidputs(attr_t newmode, int  (*outc)(int))
{
chtype	turn_off = (~newmode & previous_attr) & (chtype)(~A_COLOR);
chtype	turn_on  = (newmode & ~previous_attr) & (chtype)(~A_COLOR);

	T(("vidputs(%lx) called %s", newmode, _traceattr(newmode)));
	T(("previous attribute was %s", _traceattr(previous_attr)));

	if (newmode == previous_attr)
		return OK;
	if (newmode == A_NORMAL) {
		if((previous_attr & A_ALTCHARSET) && exit_alt_charset_mode) {
			TPUTS_TRACE("exit_alt_charset_mode");
 			tputs(exit_alt_charset_mode, 1, outc);
 			previous_attr &= ~A_ALTCHARSET;
 		}
		if (previous_attr & A_COLOR) {
			TPUTS_TRACE("orig_pair");
 			tputs(orig_pair, 1, outc);
		}
 		if (previous_attr) {
			TPUTS_TRACE("exit_attribute_mode");
 			tputs(exit_attribute_mode, 1, outc);
		}
 	
	} else if (set_attributes) {
		if (turn_on || turn_off) {
			TPUTS_TRACE("set_attributes");
	    		tputs(tparm(set_attributes,
				(newmode & A_STANDOUT) != 0,
				(newmode & A_UNDERLINE) != 0,
				(newmode & A_REVERSE) != 0,
				(newmode & A_BLINK) != 0,
				(newmode & A_DIM) != 0,
				(newmode & A_BOLD) != 0,
				(newmode & A_INVIS) != 0,
				(newmode & A_PROTECT) != 0,
				(newmode & A_ALTCHARSET) != 0), 1, outc);
#if defined(A_PCCHARSET) && (A_PCCHARSET != A_PROTECT)
			/*
			 * exit_pc_charset_mode is a defined SVr4 terminfo
			 * A_PCCHARSET is an ncurses invention to invoke it
			 */
			if ((turn_off & A_PCCHARSET)  &&  exit_pc_charset_mode) {
				TPUTS_TRACE("exit_pc_charset_mode");
				tputs(exit_pc_charset_mode, 1, outc);
			}
			/*
			 * enter_pc_charset_mode is a defined SVr4 terminfo
			 * A_PCCHARSET is an ncurses invention to invoke it
			 */
			if ((newmode & A_PCCHARSET)  &&  enter_pc_charset_mode) {
				TPUTS_TRACE("enter_pc_charset_mode");
				tputs(enter_pc_charset_mode, 1, outc);
			}
#endif /* A_PCCHARSET */
			/*
			 * Setting attributes in this way tends to unset the
			 * ones (such as color) that weren't specified.
			 */
			turn_off |= A_COLOR;
		}
	} else {

		T(("turning %s off", _traceattr(turn_off)));

		if ((turn_off & A_ALTCHARSET) && exit_alt_charset_mode) {
			TPUTS_TRACE("exit_alt_charset_mode");
			tputs(exit_alt_charset_mode, 1, outc);
			turn_off &= ~A_ALTCHARSET;
		}

		if ((turn_off & A_UNDERLINE)  &&  exit_underline_mode) {
			TPUTS_TRACE("exit_underline_mode");
			tputs(exit_underline_mode, 1, outc);
			turn_off &= ~A_UNDERLINE;
		}

		if ((turn_off & A_STANDOUT)  &&  exit_standout_mode) {
			TPUTS_TRACE("exit_standout_mode");
			tputs(exit_standout_mode, 1, outc);
			turn_off &= ~A_STANDOUT;
		}

		if (turn_off && exit_attribute_mode) {
			TPUTS_TRACE("exit_attribute_mode");
			tputs(exit_attribute_mode, 1, outc);
			turn_on  |= (newmode & (chtype)(~A_COLOR));
			turn_off |= A_COLOR;
		}

#if defined(A_PCCHARSET) && (A_PCCHARSET != A_PROTECT)
		/*
		 * exit_pc_charset_mode is a defined SVr4 terminfo
		 * A_PCCHARSET is an ncurses invention to invoke it
		 */
		if ((turn_off & A_PCCHARSET)  &&  exit_pc_charset_mode) {
			TPUTS_TRACE("exit_pc_charset_mode");
			tputs(exit_pc_charset_mode, 1, outc);
		}
#endif /* A_PCCHARSET */

		T(("turning %s on", _traceattr(turn_on)));

		if ((turn_on & A_ALTCHARSET) && enter_alt_charset_mode) {
			TPUTS_TRACE("enter_alt_charset_mode");
			tputs(enter_alt_charset_mode, 1, outc);
		}

		if ((turn_on & A_BLINK)  &&  enter_blink_mode) {
			TPUTS_TRACE("enter_blink_mode");
			tputs(enter_blink_mode, 1, outc);
		}

		if ((turn_on & A_BOLD)  &&  enter_bold_mode) {
			TPUTS_TRACE("enter_bold_mode");
			tputs(enter_bold_mode, 1, outc);
		}

		if ((turn_on & A_DIM)  &&  enter_dim_mode) {
			TPUTS_TRACE("enter_dim_mode");
			tputs(enter_dim_mode, 1, outc);
		}

		if ((turn_on & A_REVERSE)  &&  enter_reverse_mode) {
			TPUTS_TRACE("enter_reverse_mode");
			tputs(enter_reverse_mode, 1, outc);
		}

		if ((turn_on & A_STANDOUT)  &&  enter_standout_mode) {
			TPUTS_TRACE("enter_standout_mode");
			tputs(enter_standout_mode, 1, outc);
		}

		if ((turn_on & A_PROTECT)  &&  enter_protected_mode) {
			TPUTS_TRACE("enter_protected_mode");
			tputs(enter_protected_mode, 1, outc);
		}

		if ((turn_on & A_INVIS)  &&  enter_secure_mode) {
			TPUTS_TRACE("enter_secure_mode");
			tputs(enter_secure_mode, 1, outc);
		}

		if ((turn_on & A_UNDERLINE)  &&  enter_underline_mode) {
			TPUTS_TRACE("enter_underline_mode");
			tputs(enter_underline_mode, 1, outc);
		}

		if ((turn_on & A_HORIZONTAL)  &&  enter_horizontal_hl_mode) {
			TPUTS_TRACE("enter_horizontal_hl_mode");
			tputs(enter_horizontal_hl_mode, 1, outc);
		}

		if ((turn_on & A_LEFT)  &&  enter_left_hl_mode) {
			TPUTS_TRACE("enter_left_hl_mode");
			tputs(enter_left_hl_mode, 1, outc);
		}

		if ((turn_on & A_LOW)  &&  enter_low_hl_mode) {
			TPUTS_TRACE("enter_low_hl_mode");
			tputs(enter_low_hl_mode, 1, outc);
		}

		if ((turn_on & A_RIGHT)  &&  enter_right_hl_mode) {
			TPUTS_TRACE("enter_right_hl_mode");
			tputs(enter_right_hl_mode, 1, outc);
		}

		if ((turn_on & A_TOP)  &&  enter_top_hl_mode) {
			TPUTS_TRACE("enter_top_hl_mode");
			tputs(enter_top_hl_mode, 1, outc);
		}

		if ((turn_on & A_VERTICAL)  &&  enter_vertical_hl_mode) {
			TPUTS_TRACE("enter_vertical_hl_mode");
			tputs(enter_vertical_hl_mode, 1, outc);
		}

#if defined(A_PCCHARSET) && (A_PCCHARSET != A_PROTECT)
		/*
		 * enter_pc_charset_mode is a defined SVr4 terminfo
		 * A_PCCHARSET is an ncurses invention to invoke it
		 */
		if ((turn_on & A_PCCHARSET)  &&  enter_pc_charset_mode) {
			TPUTS_TRACE("enter_pc_charset_mode");
			tputs(enter_pc_charset_mode, 1, outc);
		}
#endif /* A_PCCHARSET */

	}

	if (SP->_coloron) {
	int pair = PAIR_NUMBER(newmode);
	int current_pair = PAIR_NUMBER(previous_attr);

   		T(("old pair = %d -- new pair = %d", current_pair, pair));
   		if (pair != current_pair || turn_off) {
			do_color(pair, outc);
		}
   	}

	previous_attr = newmode;

	T(("vidputs finished"));
	return OK;
}

int vidattr(attr_t newmode)
{

	T(("vidattr(%lx) called", newmode));

	return(vidputs(newmode, _nc_outch));
}

attr_t termattrs(void)
{
	int attrs = A_NORMAL;

	if (enter_alt_charset_mode)
		attrs |= A_ALTCHARSET;

	if (enter_blink_mode)
		attrs |= A_BLINK;

	if (enter_bold_mode)
		attrs |= A_BOLD;

	if (enter_dim_mode)
		attrs |= A_DIM;

	if (enter_reverse_mode)
		attrs |= A_REVERSE;

	if (enter_standout_mode)
		attrs |= A_STANDOUT;

	if (enter_protected_mode)
		attrs |= A_PROTECT;

	if (enter_secure_mode)
		attrs |= A_INVIS;

	if (enter_underline_mode)
		attrs |= A_UNDERLINE;

	if (SP->_coloron)
		attrs |= A_COLOR;

	return(attrs);
}

