/******************************************************************************
 * Copyright 1998 by Thomas E. Dickey <dickey@clark.net>                      *
 * All Rights Reserved.                                                       *
 *                                                                            *
 * Permission to use, copy, modify, and distribute this software and its      *
 * documentation for any purpose and without fee is hereby granted, provided  *
 * that the above copyright notice appear in all copies and that both that    *
 * copyright notice and this permission notice appear in supporting           *
 * documentation, and that the name of the above listed copyright holder(s)   *
 * not be used in advertising or publicity pertaining to distribution of the  *
 * software without specific, written prior permission.                       *
 *                                                                            *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD   *
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND  *
 * FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE  *
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES          *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN      *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR *
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                *
 ******************************************************************************/

#include <curses.priv.h>

#include <ctype.h>
#include <tic.h>

MODULE_ID("$Id: comp_expand.c,v 1.3 1998/01/03 19:41:29 tom Exp $")

static int trailing_spaces(const char *src)
{
	while (*src == ' ')
		src++;
	return *src == 0;
}

/* this deals with differences over whether 0x7f and 0x80..0x9f are controls */
#define CHAR_OF(s) (*(unsigned const char *)(s))
#define REALCTL(s) (CHAR_OF(s) < 127 && iscntrl(CHAR_OF(s)))
#define REALPRINT(s) (CHAR_OF(s) < 127 && isprint(CHAR_OF(s)))

char *_nc_tic_expand(const char *srcp, bool tic_format)
{
static char *	buffer;
static size_t	length;

int		bufp;
const char	*ptr, *str = VALID_STRING(srcp) ? srcp : "";
bool		islong = (strlen(str) > 3);
size_t		need = (2 + strlen(str)) * 4;
int		ch;

	if (buffer == 0) {
		buffer = malloc(length = need);
	} else if (need > length) {
		buffer = realloc(buffer, length = need);
	}

    	bufp = 0;
    	ptr = str;
    	while ((ch = (*str & 0xff)) != 0) {
		if (ch == '%' && REALPRINT(str+1)) {
	    		buffer[bufp++] = *str++;
	    		buffer[bufp++] = *str;
		}
		else if (ch == 128) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = '0';
		}
		else if (ch == '\033') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'E';
		}
		else if (ch == '\\' && tic_format && (str == srcp || str[-1] != '^')) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = '\\';
		}
		else if (ch == ' ' && tic_format && (str == srcp || trailing_spaces(str))) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 's';
		}
		else if ((ch == ',' || ch == ':' || ch == '^') && tic_format) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = ch;
		}
		else if (REALPRINT(str) && (ch != ',' && ch != ':' && !(ch == '!' && !tic_format) && ch != '^'))
		    	buffer[bufp++] = ch;
#if 0		/* FIXME: this would be more readable (in fact the whole 'islong' logic should be removed) */
		else if (ch == '\b') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'b';
		}
		else if (ch == '\f') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'f';
		}
		else if (ch == '\t' && islong) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 't';
		}
#endif
		else if (ch == '\r' && (islong || (strlen(srcp) > 2 && str[1] == '\0'))) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'r';
		}
		else if (ch == '\n' && islong) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'n';
		}
#define UnCtl(c) ((c) + '@')
		else if (REALCTL(str) && ch != '\\' && (!islong || isdigit(str[1])))
		{
			(void) sprintf(&buffer[bufp], "^%c", UnCtl(ch));
			bufp += 2;
		}
		else
		{
			(void) sprintf(&buffer[bufp], "\\%03o", ch);
			bufp += 4;
		}

		str++;
    	}

    	buffer[bufp] = '\0';
    	return(buffer);
}
