/*
 *	lib_slk.c
 *	Soft key routines.
 */

#include "curses.priv.h"
#include <string.h>
#include <stdlib.h>

#define MAX_SKEY	8	/* count of soft keys */
#define MAX_SKEY_LEN	8	/* max length of soft key text */

/*
 * We'd like to move these into the screen context structure, but cannot,
 * because slk_init() is called before initscr()/newterm().
 */
static int _slk_format;			/* format specified in slk_init() */
static chtype _slk_attr = A_STANDOUT;	/* soft label attribute */

/*
 * Fetch the label text.
 */

char *
slk_label(int n)
{
	T(("slk_label(%d)", n));

	if (SP->_slk == NULL || n < 1 || n > MAX_SKEY)
		return NULL;
	return(SP->_slk->ent[n-1].text);
}

/*
 * Write the soft labels to the soft-key window.
 */

static void
slk_intern_refresh(SLK *slk)
{
int i;
	for (i = 0; i < MAX_SKEY; i++) {
		if (slk->dirty || slk->ent[i].dirty) {
			if (slk->ent[i].visible) {
				wmove(slk->win,0,slk->ent[i].x);
				wattrset(slk->win,_slk_attr);
				waddstr(slk->win,slk->ent[i].form_text);
				wattrset(slk->win,A_NORMAL);
			}
			slk->ent[i].dirty = FALSE;
		}
	}
	slk->dirty = FALSE;
}

/*
 * Refresh the soft labels.
 */

int
slk_noutrefresh(void)
{
	T(("slk_noutrefresh()"));
	
	if (SP->_slk == NULL)
		return(ERR);
	if (SP->_slk->hidden)
		return(OK);
	slk_intern_refresh(SP->_slk);
	return(wnoutrefresh(SP->_slk->win));
}

/*
 * Refresh the soft labels.
 */

int
slk_refresh(void)
{
	T(("slk_refresh()"));
	
	if (SP->_slk == NULL)
		return(ERR);
	if (SP->_slk->hidden)
		return(OK);
	slk_intern_refresh(SP->_slk);
	return(wrefresh(SP->_slk->win));
}

/*
 * Restore the soft labels on the screen.
 */

int
slk_restore(void)
{
	T(("slk_restore()"));
	
	if (SP->_slk == NULL)
		return(ERR);
	SP->_slk->hidden = FALSE;
	SP->_slk->dirty = TRUE;
	return slk_refresh();
}

/*
 * Set soft label text.
 */

int
slk_set(int i, char *const astr, int format)
{
SLK *slk = SP->_slk;
int len;
char *str = astr;

	T(("slk_set(%d, \"%s\", %d)", i, str, format));

	if (slk == NULL || i < 1 || i > MAX_SKEY || format < 0 || format > 2)
		return(ERR);
	if (str == NULL)
		str = "";
	--i;
	(void) strncpy(slk->ent[i].text, str, MAX_SKEY);
	memset(slk->ent[i].form_text,' ', MAX_SKEY);
	slk->ent[i].text[MAX_SKEY_LEN] = 0;
	slk->ent[i].form_text[MAX_SKEY_LEN] = 0;
	len = strlen(slk->ent[i].text);
	switch(format) {
	case 0: /* left-justified */
		memcpy(slk->ent[i].form_text,
		       slk->ent[i].text,
		       len);
		break;
	case 1: /* centered */
		memcpy(slk->ent[i].form_text+(MAX_SKEY_LEN-len)/2,
		       slk->ent[i].text,
		       len);
		break;
	case 2: /* right-justified */
		memcpy(slk->ent[i].form_text+MAX_SKEY_LEN-len,
		       slk->ent[i].text,
		       len);
		break;
	}
	slk->ent[i].dirty = TRUE;
	return(OK);
}

/*
 * Force the code to believe that the soft keys have been changed.
 */

int
slk_touch(void)
{
	T(("slk_touch()"));
	
	if (SP->_slk == NULL)
		return(ERR);
	SP->_slk->dirty = TRUE;
	return(OK);
}

/*
 * Remove soft labels from the screen.
 */

int
slk_clear(void)
{
	T(("slk_clear()"));
	
	if (SP->_slk == NULL)
		return(ERR);
	SP->_slk->hidden = TRUE;
	werase(SP->_slk->win);
	return wrefresh(SP->_slk->win);
}

/*
 * Initialize soft labels.
 * Called from newterm()
 */

static int
slk_initialize(WINDOW *stwin, int cols)
{
SLK *slk;
int i, maxlab, x;

	T(("slk_initialize()"));

	if ((SP->_slk = slk = (SLK*) calloc(1,sizeof(SLK))) == NULL)
		return(OK);
	maxlab = (cols+1)/9;
	for (i = 0; i < MAX_SKEY; i++) {
		memset(slk->ent[i].form_text,' ',MAX_SKEY_LEN);
		slk->ent[i].visible = i < maxlab;
	}
	if (_slk_format == 1) {	/* 4-4 */
		int gap = cols - (MAX_SKEY * MAX_SKEY_LEN) - 6;

		if (gap < 1)
			gap = 1;
		for (i = x = 0; i < MAX_SKEY; i++) {
			slk->ent[i].x = x;
		x += MAX_SKEY_LEN;
		x += (i == 3) ? gap : 1;
		}
	}
	else {			/* 0 -> 3-2-3 */
		int gap = (cols - (MAX_SKEY * MAX_SKEY_LEN) - 5) / 2;

		if (gap < 1)
			gap = 1;
		for (i = x = 0; i < MAX_SKEY; i++) {
			slk->ent[i].x = x;
			x += MAX_SKEY_LEN;
			x += (i == 2 || i == 4) ? gap : 1;
		}
	}
	slk->dirty = TRUE;
	if ((slk->win = stwin) == NULL)
	{
		free(slk);
		return(ERR);
	}

	return(OK);
}

/*
 * Initialize soft labels.  Called by the user before initscr().
 */

int
slk_init(int format)
{
	if (format < 0 || format > 1)
		return(ERR);
	_slk_format = format;
	ripoffline(-1, slk_initialize);
	return(OK);
}

/* Functions to manipulate the soft-label attribute */

int 
slk_attrset(attr_t attr)
{
    _slk_attr = attr;
    return(OK);
}

int 
slk_attron(attr_t attr)
{
    _slk_attr |= attr;
    return(OK);
}

int 
slk_attroff(attr_t attr)
{
    _slk_attr &=~ attr;
    return(OK);
}

