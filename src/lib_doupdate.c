
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992, 1993, 1994                 *
*                          by Zeyd M. Ben-Halim                            *
*                          zmbenhal@netcom.com                             *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, not removed   *
*        from header files, and is reproduced in any documentation         *
*        accompanying it or the applications linked with it.               *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/


/*-----------------------------------------------------------------
 *
 *	lib_doupdate.c
 *
 *	The routine doupdate() and its dependents
 *
 *-----------------------------------------------------------------*/
 
#include <stdlib.h>
#include <sys/time.h>
#ifdef SYS_SELECT
#include <sys/select.h>
#endif
#include <string.h>
#include "curses.priv.h"
#include "terminfo.h"

/*
 * Enable checking to see if doupdate and friends are tracking the true
 * cursor position correctly.  NOTE: this is a debugging hack which will
 * work ONLY on ANSI-compatible terminals!
 */
/* #define POSITION_DEBUG */

static void ClrUpdate( WINDOW *scr );
static void TransformLine( int lineno );
static void NoIDcTransformLine( int lineno );
static void IDcTransformLine( int lineno );
static void ClearScreen( void );
static void InsStr( chtype *line, int count );
static void DelChar( int count );

#ifdef POSITION_DEBUG
/****************************************************************************
 *
 * Debugging code.  Only works on ANSI-standard terminals.
 *
 ****************************************************************************/

void position_check(int expected_y, int expected_x, char *legend)
/* check to see if the real cursor position matches the virtual */
{
    static char  buf[9];
    int y, x;

    memset(buf, '\0', sizeof(buf));
    (void) write(1, "\033[6n", 4);	/* only works on ANSI-compatibles */
    (void) read(0, (void *)buf, 8);
    _tracef("probe returned %s", visbuf(buf));

    /* try to interpret as a position report */
    if (sscanf(buf, "\033[%d;%dR", &y, &x) != 2)
	_tracef("position probe failed in %s", legend);
    else if (y - 1 != expected_y || x - 1 != expected_x)
	_tracef("position seen (%d, %d) doesn't match expected one (%d, %d) in %s",
		y-1, x-1, expected_y, expected_x, legend);
    else
	_tracef("position matches OK in %s", legend);
}
#endif /* POSITION_DEBUG */

/****************************************************************************
 *
 * Optimized update code
 *
 ****************************************************************************/

static inline void PutAttrChar(chtype ch)
{
	TR(TRACE_CHARPUT, ("PutAttrChar(%s, %s) at (%d, %d)",
			  _tracechar(ch & A_CHARTEXT),
			  _traceattr((ch & (chtype)A_ATTRIBUTES)),
			   SP->_cursrow, SP->_curscol));
	if (curscr->_attrs != (ch & (chtype)A_ATTRIBUTES)) {
		curscr->_attrs = ch & (chtype)A_ATTRIBUTES;
		vidputs(curscr->_attrs, _outch);
	}
	putc(ch & A_CHARTEXT, SP->_ofp);
}

static int LRCORNER = FALSE;

static inline void PutChar(chtype ch)
{
	if (LRCORNER == TRUE) {
		if (SP->_curscol == screen_columns-2) {
			PutAttrChar(newscr->_line[screen_lines-1].text[screen_columns-2]);
			SP->_curscol++;
#ifdef POSITION_DEBUG
			position_check(SP->_cursrow,SP->_curscol,"PutChar");
#endif /* POSITION_DEBUG */
			return;
		} else if (SP->_curscol == screen_columns-1) {
		int i = screen_lines;
		int j = screen_columns -1;
			if (cursor_left)
				putp(cursor_left);
			else
				mvcur(-1, -1, i-1, j);
			if (enter_insert_mode && exit_insert_mode) {
				putp(enter_insert_mode);
				PutAttrChar(newscr->_line[i-1].text[j]);
				putp(exit_insert_mode);
			} else if (insert_character) {
				putp(insert_character);
				PutAttrChar(newscr->_line[i-1].text[j]);
			}
			return;
		}
	}
	PutAttrChar(ch);
	SP->_curscol++; 
	if (SP->_curscol >= screen_columns) {
		if (auto_right_margin) {	 
			SP->_curscol = 0;	   
			SP->_cursrow++;
		} else {
		 	SP->_curscol--;
		}
	}
#ifdef POSITION_DEBUG
	position_check(SP->_cursrow, SP->_curscol, "PutChar");
#endif /* POSITION_DEBUG */
}	

static inline void GoTo(int row, int col)
{
	TR(TRACE_MOVE, ("Goto(%d, %d) from (%d, %d)",
			row, col, SP->_cursrow, SP->_curscol));
#ifdef POSITION_DEBUG
	position_check(SP->_cursrow, SP->_curscol, "GoTo");
#endif /* POSITION_DEBUG */
	mvcur(SP->_cursrow, SP->_curscol, row, col); 
	SP->_cursrow = row; 
	SP->_curscol = col; 
}

int _outch(char ch)
{
	if (SP != NULL)
		putc(ch, SP->_ofp);
	else
		putc(ch, stdout);
	return OK;
}

int doupdate(void)
{
int	i;
	
	T(("doupdate() called"));

#ifdef TRACE
	if (_tracing & TRACE_UPDATE)
	{
	    if (curscr->_clear)
		_tracef("curscr is clear");
	    else
		_tracedump("curscr", curscr);
	    _tracedump("newscr", newscr);
	}
#endif /* TRACE */

	curses_signal_handler(FALSE);

	if (SP->_endwin == TRUE) {
		T(("coming back from shell mode"));
		reset_prog_mode();
		if (enter_ca_mode)
			putp(enter_ca_mode);
		newscr->_clear = TRUE;
		SP->_endwin = FALSE;
	}

	/* check for pending input */
	{
	fd_set fdset;
	struct timeval timeout = {0,0};

		FD_ZERO(&fdset);
		FD_SET(SP->_checkfd, &fdset);
		if (select(SP->_checkfd+1, &fdset, NULL, NULL, &timeout) != 0) {
			fflush(SP->_ofp);
			return OK;
		}
	}

	if (curscr->_clear) {		/* force refresh ? */
		T(("clearing and updating curscr"));
		ClrUpdate(curscr);		/* yes, clear all & update */
		curscr->_clear = FALSE;	/* reset flag */
	} else {
		if (newscr->_clear) {
			T(("clearing and updating newscr"));
			ClrUpdate(newscr);
			newscr->_clear = FALSE;
		} else {
		        scroll_optimize();

			T(("Transforming lines"));
			for (i = 0; i < min(screen_lines, newscr->_maxy + 1); i++) {
				if(newscr->_line[i].firstchar != _NOCHANGE)
					TransformLine(i);
			}
		}
	}
	T(("marking screen as updated"));
	for (i = 0; i <= newscr->_maxy; i++) {
		newscr->_line[i].firstchar = _NOCHANGE;
		newscr->_line[i].lastchar = _NOCHANGE;
		newscr->_line[i].oldindex = i;
	}
	for (i = 0; i <= curscr->_maxy; i++) {
		curscr->_line[i].oldindex = i;
	}

	curscr->_curx = newscr->_curx;
	curscr->_cury = newscr->_cury;

	GoTo(curscr->_cury, curscr->_curx);
	
	/* perhaps we should turn attributes off here */

	if (curscr->_attrs != A_NORMAL)
		vidattr(curscr->_attrs = A_NORMAL);

	fflush(SP->_ofp);

	curses_signal_handler(TRUE);

	return OK;
}

/*
**	ClrUpdate(scr)
**
**	Update by clearing and redrawing the entire screen.
**
*/

static void ClrUpdate(WINDOW *scr)
{
int	i = 0, j = 0;
int	lastNonBlank;
int	inspace;

	T(("ClrUpdate(%x) called", scr));
	if (back_color_erase) {
		T(("back_color_erase, turning attributes off"));
		vidattr(A_NORMAL);
	}
	ClearScreen();

	T(("updating screen from scratch"));
	for (i = 0; i < min(screen_lines, scr->_maxy + 1); i++) {
		GoTo(i, 0);
		LRCORNER = FALSE;
		lastNonBlank = scr->_maxx;
		
		while (scr->_line[i].text[lastNonBlank] == BLANK && lastNonBlank > 0)
			lastNonBlank--;

		/* check if we are at the lr corner */
		if (i == screen_lines-1)
			if ((auto_right_margin) && !(eat_newline_glitch) &&
			    (lastNonBlank == screen_columns-1) && !(scr->_scroll)) 
			{
				T(("Lower-right corner needs special handling"));
			    LRCORNER = TRUE;
			}

		inspace = 0;
		memset(SP->_curscr->_line[i].text,
		       BLANK,
		       sizeof(chtype) * lastNonBlank);
		for (j = 0; j <= min(lastNonBlank, screen_columns); j++) {
			if ((scr->_line[i].text[j]) == BLANK) {
				inspace++;
				continue;
			} else if (inspace) {
				mvcur(i, j - inspace, i, j);
				inspace = 0;
			}
			PutChar(scr->_line[i].text[j]);
		}
	}


	if (scr != curscr) {
		for (i = 0; i < screen_lines ; i++)
			for (j = 0; j < screen_columns; j++)
				curscr->_line[i].text[j] = scr->_line[i].text[j];
	}
}

/*
**	TransformLine(lineno)
**
**	Call either IDcTransformLine or NoIDcTransformLine to do the
**	update, depending upon availability of insert/delete character.
*/

static void TransformLine(int lineno)
{

	T(("TransformLine(%d) called",lineno));

	if ( (insert_character  ||  (enter_insert_mode  &&  exit_insert_mode))
		 &&  delete_character)
		IDcTransformLine(lineno);
	else
		NoIDcTransformLine(lineno);
}



/*
**	NoIDcTransformLine(lineno)
**
**	Transform the given line in curscr to the one in newscr, without
**	using Insert/Delete Character.
**
**		firstChar = position of first different character in line
**		lastChar = position of last different character in line
**
**		overwrite all characters between firstChar and lastChar.
**
*/

static void NoIDcTransformLine(int lineno)
{
int	firstChar, lastChar;
chtype	*newLine = newscr->_line[lineno].text;
chtype	*oldLine = curscr->_line[lineno].text;
int	k;
int	attrchanged = 0;
	
	T(("NoIDcTransformLine(%d) called", lineno));

	firstChar = 0;
	while (firstChar < screen_columns - 1 &&  newLine[firstChar] == oldLine[firstChar]) {
		if(ceol_standout_glitch) {
			if((newLine[firstChar] & (chtype)A_ATTRIBUTES) != (oldLine[firstChar] & (chtype)A_ATTRIBUTES))
			attrchanged = 1;
		}			
		firstChar++;
	}

	T(("first char at %d is %x", firstChar, newLine[firstChar]));
	if (firstChar > screen_columns)
		return;

	if(ceol_standout_glitch && attrchanged) {
		firstChar = 0;
		lastChar = screen_columns - 1;
		GoTo(lineno, firstChar);
		if(clr_eol)
			putp(clr_eol);		
	} else {
		lastChar = screen_columns - 1;
		while (lastChar > firstChar  &&  newLine[lastChar] == oldLine[lastChar])
			lastChar--;
		GoTo(lineno, firstChar);
	}			

	/* check if we are at the lr corner */
	if (lineno == screen_lines-1)
		if ((auto_right_margin) && !(eat_newline_glitch) &&
		    (lastChar == screen_columns-1) && !(curscr->_scroll)) 
		{
			T(("Lower-right corner needs special handling"));
		    LRCORNER = TRUE;
		}

	T(("updating chars %d to %d", firstChar, lastChar));
	for (k = firstChar; k <= lastChar; k++) {
		PutChar(newLine[k]);
		oldLine[k] = newLine[k];
	}
}

/*
**	IDcTransformLine(lineno)
**
**	Transform the given line in curscr to the one in newscr, using
**	Insert/Delete Character.
**
**		firstChar = position of first different character in line
**		oLastChar = position of last different character in old line
**		nLastChar = position of last different character in new line
**
**		move to firstChar
**		overwrite chars up to min(oLastChar, nLastChar)
**		if oLastChar < nLastChar
**			insert newLine[oLastChar+1..nLastChar]
**		else
**			delete oLastChar - nLastChar spaces
*/

static void IDcTransformLine(int lineno)
{
int	firstChar, oLastChar, nLastChar;
chtype	*newLine = newscr->_line[lineno].text;
chtype	*oldLine = curscr->_line[lineno].text;
int	k, n;
int	attrchanged = 0;
	
	T(("IDcTransformLine(%d) called", lineno));

	if(ceol_standout_glitch && clr_eol) {
		firstChar = 0;
		while(firstChar < screen_columns) {
			if((newLine[firstChar] & (chtype)A_ATTRIBUTES) != (oldLine[firstChar] & (chtype)A_ATTRIBUTES))
				attrchanged = 1;
			firstChar++;			
		}
	}
	
	firstChar = 0;
	
	if (attrchanged) {
		GoTo(lineno, firstChar);
		putp(clr_eol);		
		for( k = 0 ; k <= (screen_columns-1) ; k++ )
			PutChar(newLine[k]);
	} else {
		while (firstChar < screen_columns  &&
				newLine[firstChar] == oldLine[firstChar])
			firstChar++;
		
		if (firstChar >= screen_columns)
			return;

		oLastChar = screen_columns - 1;
		while (oLastChar > firstChar  &&  oldLine[oLastChar] == BLANK)
			oLastChar--;
	
		nLastChar = screen_columns - 1;
		while (nLastChar > firstChar  &&  newLine[nLastChar] == BLANK)
			nLastChar--;

		if((nLastChar == firstChar) && clr_eol) {
			GoTo(lineno, firstChar);
			putp(clr_eol);
			if(newLine[firstChar] != BLANK )
				PutChar(newLine[firstChar]);
		} else if( newLine[nLastChar] != oldLine[oLastChar] ) {
			n = max( nLastChar , oLastChar );

			GoTo(lineno, firstChar);

			for( k=firstChar ; k <= n ; k++ )
				PutChar(newLine[k]);
		} else {
			while (newLine[nLastChar] == oldLine[oLastChar]) {
				nLastChar--;
				oLastChar--;
			}
	
			n = min(oLastChar, nLastChar);

			GoTo(lineno, firstChar);
	
			for (k=firstChar; k <= n; k++)
				PutChar(newLine[k]);

			if (oLastChar < nLastChar)
				InsStr(&newLine[k], nLastChar - oLastChar);

			else if (oLastChar > nLastChar )
				DelChar(oLastChar - nLastChar);
		}
	}
	for (k = firstChar; k < screen_columns; k++)
		oldLine[k] = newLine[k];
}

/*
**	ClearScreen()
**
**	Clear the physical screen and put cursor at home
**
*/

static void ClearScreen()
{

	T(("ClearScreen() called"));

	if (clear_screen) {
		putp(clear_screen);
		SP->_cursrow = SP->_curscol = 0;
#ifdef POSITION_DEBUG
		position_check(SP->_cursrow, SP->_curscol, "ClearScreen");
#endif /* POSITION_DEBUG */
	} else if (clr_eos) {
		SP->_cursrow = SP->_curscol = -1;
		GoTo(0,0);

		putp(clr_eos);
	} else if (clr_eol) {
		SP->_cursrow = SP->_curscol = -1;

		while (SP->_cursrow < screen_lines) {
			GoTo(SP->_cursrow, 0);
			putp(clr_eol);
		}
		GoTo(0,0);
	}
	T(("screen cleared"));
}


/*
**	InsStr(line, count)
**
**	Insert the count characters pointed to by line.
**
*/

static void InsStr(chtype *line, int count)
{
	T(("InsStr(%x,%d) called", line, count));

	if (enter_insert_mode  &&  exit_insert_mode) {
		putp(enter_insert_mode);
		while (count) {
			PutChar(*line);
			line++;
			count--;
		}
		putp(exit_insert_mode);
	} else if (parm_ich) {
		tputs(tparm(parm_ich, count), count, _outch);
		while (count) {
			PutChar(*line);
			line++;
			count--;
		}
	} else {
		while (count) {
			putp(insert_character);
			PutChar(*line);
			line++;
			count--;
		}
	}
}

/*
**	DelChar(count)
**
**	Delete count characters at current position
**
*/

static void DelChar(int count)
{
	T(("DelChar(%d) called", count));

	if (parm_dch) {
		tputs(tparm(parm_dch, count), count, _outch);
	} else {
		while (count--)
			putp(delete_character);
	}
}

/*
**	outstr(char *str)
**
**	Emit a string without waiting for update.
*/

void outstr(char *str)
{
    (void) fputs(str, stdout);
    (void) fflush(stdout);
}
