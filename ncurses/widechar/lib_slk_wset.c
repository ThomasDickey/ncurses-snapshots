/****************************************************************************
 * Copyright (c) 2003 Free Software Foundation, Inc.                        *
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
 *  Author: Thomas E. Dickey, 2003                                          *
 ****************************************************************************/

/*
 *	lib_slk_wset.c
 *      Set soft label text.
 */
#include <curses.priv.h>
#include <wctype.h>

MODULE_ID("$Id: lib_slk_wset.c,v 1.2 2003/03/29 23:08:16 tom Exp $")

NCURSES_EXPORT(int)
slk_wset(int i, const wchar_t * astr, int format)
{
    static wchar_t empty[] =
    {L'\0'};
    int result = ERR;
    SLK *slk = SP->_slk;
    int offset;
    size_t arglen;
    const wchar_t *p;

    T((T_CALLED("slk_wset(%d, \"%s\", %d)"), i, _nc_viswbuf(astr), format));

    if (astr == 0)
	astr = empty;
    arglen = wcslen(astr);
    while (iswspace(*astr)) {
	--arglen;
	++astr;			/* skip over leading spaces  */
    }
    p = astr;
    while (iswprint(*p))
	p++;			/* The first non-print stops */

    arglen = (size_t) (p - astr);

    if (slk != NULL &&
	i >= 1 &&
	i <= slk->labcnt &&
	format >= 0 &&
	format <= 2) {
	char *new_text = _nc_doalloc(0, MB_LEN_MAX * arglen);
	size_t n;
	size_t need;
	size_t used = 0;
	int mycols;
	mbstate_t state;

	--i;			/* Adjust numbering of labels */

	/*
	 * Reduce the actual number of columns to fit in the label field.
	 */
	while (arglen != 0 && wcswidth(astr, arglen) > slk->maxlen) {
	    --arglen;
	}
	mycols = wcswidth(astr, arglen);

	/*
	 * translate the wide-character string to multibyte form.
	 *
	 * FIXME: is that really a null-terminated string?  If not, this will
	 * not work.
	 */
	memset(&state, 0, sizeof(state));
	for (n = 0; n < arglen; ++n) {
	    used += wcrtomb(new_text + used, astr[n], &state);
	}
	new_text[used] = '\0';

	need = used + slk->maxlen + 1;
	if ((slk->ent[i].ent_text = _nc_doalloc(slk->ent[i].ent_text, need))
	    == 0)
	    returnCode(ERR);
	if ((slk->ent[i].form_text = _nc_doalloc(slk->ent[i].form_text,
	    need)) == 0)
	    returnCode(ERR);

	(void) strcpy(slk->ent[i].ent_text, new_text);
	memset(slk->ent[i].form_text, ' ', (unsigned) slk->maxlen);

	switch (format) {
	default:
	case 0:		/* left-justified */
	    offset = 0;
	    break;
	case 1:		/* centered */
	    offset = (slk->maxlen - mycols) / 2;
	    break;
	case 2:		/* right-justified */
	    offset = slk->maxlen - mycols;
	    break;
	}
	if (offset != 0)
	    memset(slk->ent[i].form_text, ' ', offset);
	strcpy(slk->ent[i].form_text + offset,
	       slk->ent[i].ent_text);
	if (format != 2 && mycols < slk->maxlen) {
	    sprintf(slk->ent[i].form_text + offset + used,
		    "%*s",
		    slk->maxlen - mycols,
		    " ");
	}
	slk->ent[i].dirty = TRUE;
	result = OK;
    }
    returnCode(result);
}
