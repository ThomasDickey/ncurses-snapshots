
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
 *	captoinfo.c --- conversion between termcap and terminfo formats
 *
 *	The captoinfo() code was swiped from Ross Ridge's mytinfo package,
 *	adapted to fit ncurses by Eric S. Raymond <esr@snark.thyrsus.com>.
 *
 *	There is just one entry point:
 *
 *	char *captoinfo(n, s, parametrized)
 *
 *	Convert value s for termcap string capabilitynamed n into terminfo
 *	format.  
 *
 *	Besides all the standard termcap escapes, this translator understands
 *	the following extended escapes:
 *
 *	used by GNU Emacs termcap libraries
 *		%a[+*-/=][cp]x	GNU arithmetic. 
 *		%m	  	xor the first two parameters by 0177
 *		%b	  	backup to previous parameter
 *		%f	  	skip this parameter
 *
 *	used by the University of Waterloo (MFCF) termcap libraries
 *		%-x	 subtract parameter FROM char x and output it as a char
 *		%ax	 add the character x to parameter
 *
 *	If #define WATERLOO is on, also enable these translations:
 *
 *		%sx	 subtract parameter FROM the character x
 *
 *	By default, this Waterloo translations are not compiled in, because
 *	the Waterloo %s conflicts with the way terminfo uses %s in strings for
 *	function programming.
 *
 * 	Note the two definitions of %a: the GNU definition is translated if the
 *	characters after the 'a' are valid for it, otherwise the UW definition
 *	is translated.
 */

#include "curses.priv.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unctrl.h>
#include "tic.h"

#ifndef MAX_PUSHED
/* maximum # of parameters that can be pushed onto the stack */
#define MAX_PUSHED 16
#endif

#ifndef MAX_ENTRY
/* maximum characters in a translated capability */
#define MAX_ENTRY	80
#endif

static int stack[MAX_PUSHED];	/* the stack */
static int stackptr;		/* the next empty place on the stack */
static int onstack;		/* the top of stack */
static int seenm;		/* seen a %m */
static int seenn;		/* seen a %n */
static int seenr;		/* seen a %r */
static int param;		/* current parameter */
static char *dp;		/* pointer to end of the converted string */

static void push(void)
/* push onstack on to the stack */
{
    if (stackptr > MAX_PUSHED)
	_nc_warning("string too complex to convert");
    else
	stack[stackptr++] = onstack;
}

static void pop(void)
/* pop the top of the stack into onstack */
{
    if (stackptr == 0) 
	if (onstack == 0)
	    _nc_warning("I'm confused");
	else
	    onstack = 0;
    else
	onstack = stack[--stackptr];
    param++;
}

static int cvtchar(register char *sp)
/* convert a character to a terminfo push */
{
    unsigned char c = 0;
    int len;

    switch(*sp) {
    case '\\':
	switch(*++sp) {
	case '\'':
	case '$':
	case '\\':
	case '%':
	    c = *sp;
	    len = 2;
	    break;
	case '\0':
	    c = '\\';
	    len = 1;
	    break;
	case '0':
	case '1':
	case '2':
	case '3':
	    len = 1;
	    while (isdigit(*sp))
	    {
		c = 8 * c + (*sp++ - '0');
		len++;
	    }
	    break;
	default:
	    c = *sp;
	    len = 2;
	    break;
	}
	break;
    case '^':
	c = (*++sp & 0x1f);
	len = 2;
	break;
    default:
	c = *sp;
	len = 1;
    }
    if (isgraph(c) && c != ',' && c != '\'' && c != '\\' && c != ':') {
	*dp++ = '%'; *dp++ = '\''; *dp++ = c; *dp++ = '\'';
    } else {
	*dp++ = '%'; *dp++ = '{';
	if (c > 99)
	    *dp++ = c / 100 + '0';
	if (c > 9)
	    *dp++ = ((int)(c / 10)) % 10 + '0';
	*dp++ = c % 10 + '0';
	*dp++ = '}';
    }
    return len;
}

static void getparm(int parm, int n)
/* push n copies of param on the terminfo stack if not already there */
{
	if (seenr) {
		if (parm == 1)
			parm = 2;
		else if (parm == 2)
			parm = 1;
		}
	if (onstack == parm) {
		if (n > 1) {
			_nc_warning("string may not be optimal");
			*dp++ = '%'; *dp++ = 'P'; *dp++ = 'a';
			while(n--) {
				*dp++ = '%'; *dp++ = 'g'; *dp++ = 'a';
			}
		}
		return;
	}
	if (onstack != 0)
		push();

	onstack = parm;
	
	while(n--) {		/* %p0 */
		*dp++ = '%'; *dp++ = 'p'; *dp++ = '0' + parm;
	}

	if (seenn && parm < 3) {	/* %{96}%^ */
		*dp++ = '%'; *dp++ = '{'; *dp++ = '9'; *dp++ = '6'; *dp++ = '}';
		*dp++ = '%'; *dp++ = '^';
	}
	
	if (seenm && parm < 3) {	/* %{127}%^ */
		*dp++ = '%'; *dp++ = '{'; *dp++ = '1'; *dp++ = '2'; *dp++ = '7';
		*dp++ = '}'; *dp++ = '%'; *dp++ = '^';
	}
}

char *_nc_captoinfo(
/* convert a termcap string to terminfo format */
register char *cap,	/* relevant terminfo capability index */
register char *s,	/* string value of the capability */
bool parametrized)	/* do % translations? */	
{
    static char line[MAX_ENTRY];
    char *capstart;

    stackptr = 0;
    onstack = 0;
    seenm = 0;
    seenn = 0;
    seenr = 0;
    param = 1;

    dp = line;

    /* skip the initial padding */
    capstart = (char *)NULL;
    if (isdigit(*s))
	for (capstart = s; ; s++)
	    if (!(isdigit(*s) || *s == '*' || *s == '.'))
		break;

    while(*s != '\0') {
	switch(*s) {
	case '%':
	    s++;
	    if (!parametrized) {
		*dp++ = '%';
		break;
	    }
	    switch(*s++) {
	    case '%': *dp++ = '%'; break;
	    case 'r':
		if (seenr++ == 1) {
		    _nc_warning("saw %%r twice");
		}
		break;
	    case 'm':
		if (seenm++ == 1) {
		    _nc_warning("saw %%m twice");
		}
		break;
	    case 'n':
		if (seenn++ == 1) {
		    _nc_warning("saw %%n twice");
		}
		break;
	    case 'i': *dp++ = '%'; *dp++ = 'i'; break;
	    case '6': 
	    case 'B':
		getparm(param, 2);
		/* %{6}%*%+ */
		*dp++ = '%'; *dp++ = '{'; *dp++ = '6';
		*dp++ = '}'; *dp++ = '%'; *dp++ = '*';
		*dp++ = '%'; *dp++ = '+';
		break;
	    case '8':
	    case 'D':
		getparm(param, 2);
		/* %{2}%*%- */
		*dp++ = '%'; *dp++ = '{'; *dp++ = '2';
		*dp++ = '}'; *dp++ = '%'; *dp++ = '*';
		*dp++ = '%'; *dp++ = '-';
		break;
	    case '>':
		getparm(param, 2);
		/* %?%{x}%>%t%{y}%+%; */
		*dp++ = '%'; *dp++ = '?'; 
		s += cvtchar(s);
		*dp++ = '%'; *dp++ = '>';
		*dp++ = '%'; *dp++ = 't';
		s += cvtchar(s);
		*dp++ = '%'; *dp++ = '+';
		*dp++ = '%'; *dp++ = ';';
		break;
	    case 'a':
		if ((*s == '=' || *s == '+' || *s == '-'
		     || *s == '*' || *s == '/')
		    && (s[1] == 'p' || s[1] == 'c')
		    && s[2] != '\0') {
		    int l;
		    l = 2;
		    if (*s != '=')
			getparm(param, 1);
		    if (s[1] == 'p') {
			getparm(param + s[2] - '@', 1);
			if (param != onstack) {
			    pop();
			    param--;
			}
			l++;
		    } else
			l += cvtchar(s + 2);
		    switch(*s) {
		    case '+':
			*dp++ = '%'; *dp++ = '+';
			break;
		    case '-':
			*dp++ = '%'; *dp++ = '-';
			break;
		    case '*':
			*dp++ = '%'; *dp++ = '*';
			break;
		    case '/':
			*dp++ = '%'; *dp++ = '/';
			break;
		    case '=':
			if (seenr)
			    if (param == 1)
				onstack = 2;
			    else if (param == 2)
				onstack = 1;
			    else
				onstack = param;
			else
			    onstack = param;
			break;
		    }
		    s += l;
		    break;
		}
		getparm(param, 1);
		s += cvtchar(s);
		*dp++ = '%'; *dp++ = '+';
		break;
	    case '+':
		getparm(param, 1);
		s += cvtchar(s);
		*dp++ = '%'; *dp++ = '+';
		*dp++ = '%'; *dp++ = 'c';
		pop();
		break;
	    case 's':
#ifdef WATERLOO
		s += cvtchar(s);
		getparm(param, 1);
		*dp++ = '%'; *dp++ = '-';
#else
		getparm(param, 1);
		*dp++ = '%'; *dp++ = 's';
		pop();
#endif /* WATERLOO */
		break;
	    case '-':
		s += cvtchar(s);
		getparm(param, 1);
		*dp++ = '%'; *dp++ = '-';
		*dp++ = '%'; *dp++ = 'c';
		pop();
		break;
	    case '.':
		getparm(param, 1);
		*dp++ = '%'; *dp++ = 'c';
		pop();
		break;
	    case '2':
		getparm(param, 1);
		*dp++ = '%'; /* *dp++ = '0'; */
		*dp++ = '2'; *dp++ = 'd';
		pop();
		break;
	    case '3':
		getparm(param, 1);
		*dp++ = '%'; /* *dp++ = '0'; */
		*dp++ = '3'; *dp++ = 'd';
		pop();
		break;
	    case 'd':
		getparm(param, 1);
		*dp++ = '%'; *dp++ = 'd';
		pop();
		break;
	    case 'f':
		param++;
		break;
	    case 'b':
		param--;
		break;
	    case '\\':
		*dp++ = '%';
		*dp++ = '\\';
		break;
	    default:
		*dp++ = '%';
		s--;
		_nc_warning("'%s' unknown %% code %s",
			cap, _tracechar(*s));
		break;
	    }
	    break;
#ifdef REVISIBILIZE
	case '\\':
	    *dp++ = *s++; *dp++ = *s++; break;
	case '\n':
	    *dp++ = '\\'; *dp++ = 'n'; s++; break;
	case '\t':
	    *dp++ = '\\'; *dp++ = 't'; s++; break;
	case '\r':
	    *dp++ = '\\'; *dp++ = 'r'; s++; break;
	case '\200':
	    *dp++ = '\\'; *dp++ = '0'; s++; break;
	case '\f':
	    *dp++ = '\\'; *dp++ = 'f'; s++; break;
	case '\b':
	    *dp++ = '\\'; *dp++ = 'b'; s++; break;
	case ' ':
	    *dp++ = '\\'; *dp++ = 's'; s++; break;
	case '^':
	    *dp++ = '\\'; *dp++ = '^'; s++; break;
	case ':':
	    *dp++ = '\\'; *dp++ = ':'; s++; break;
	case ',':
	    *dp++ = '\\'; *dp++ = ','; s++; break;
	default:
	    if (*s == '\033') {
		*dp++ = '\\';
		*dp++ = 'E';
		s++;
	    } else if (*s > 0 && *s < 32) {
		*dp++ = '^';
		*dp++ = *s + '@';
		s++;
	    } else if (*s <= 0 || *s >= 127) {
		*dp++ = '\\';
		*dp++ = ((*s & 0300) >> 6) + '0';
		*dp++ = ((*s & 0070) >> 3) + '0';
		*dp++ = (*s & 0007) + '0';
		s++;
	    } else
		*dp++ = *s++;
	    break;
#else
	default:
	    *dp++ = *s++;
	    break;
#endif
	}
    }

    /*
     * Now, if we stripped off some leading padding, add it at the end
     * of the string as mandatory padding.
     */
    if (capstart)
    {
	*dp++ = '$';
	*dp++ = '<';
	for (s = capstart; ; s++)
	    if (isdigit(*s) || *s == '*' || *s == '.')
		*dp++ = *s;
	    else
		break;
	*dp++ = '/';
	*dp++ = '>';
    }

    *dp = '\0';
    return(line);
}

/*
 * Here are the capabilities infotocap assumes it can translate to:
 *
 *     %%       output `%'
 *     %d       output value as in printf %d
 *     %2       output value as in printf %2d
 *     %3       output value as in printf %3d
 *     %.       output value as in printf %c
 *     %+c      add character c to value, then do %.
 *     %>xy     if value > x then add y, no output
 *     %r       reverse order of two parameters, no output
 *     %i       increment by one, no output
 *     %n       exclusive-or all parameters with 0140 (Datamedia 2500)
 *     %B       BCD (16*(value/10)) + (value%10), no output
 *     %D       Reverse coding (value - 2*(value%16)), no output (Delta Data).
 *     %m       exclusive-or all parameters with 0177 (not in 4.4BSD)
 */

static	char	buffer[256];
static	char	*bufptr;

char *_nc_infotocap(
/* convert a terminfo string to termcap format */
register char *cap,	/* relevant termcap capability index */
register char *str,	/* string value of the capability */
bool parametrized)	/* do % translations? */
{
    int	seenone = 0, seentwo = 0, saw_m = 0, saw_n = 0;
    char *padding, ch1 = 0, ch2 = 0;

    /*
     * Yes, this code is rather ad-hoc.  It passes the only important
     * test, which is that it correctly back-translates a terminfo version
     * of the historical termcap database -- esr.
     */
    memset(buffer, '\0', sizeof(buffer));
    bufptr = buffer;

    /* we may have to move some trailing mandatory padding up front */
    padding = str + strlen(str) - 1;
    if (*padding == '>' && *--padding == '/')
    {
	--padding;
	while (isdigit(*padding) || *padding == '.' || *padding == '*')
	    padding--;
	if (*padding == '<' && *--padding == '$')
	    *padding = '\0';
	padding += 2;

	while (isdigit(*padding) || *padding == '.' || *padding == '*')
	    *bufptr++ = *padding++;
    }

    for (; *str; str++)
    {
	int	c1, c2;
	char	*cp;

	if (str[0] == '\\' && (str[1] == '^' || str[1] == ','))
	{
	    *(bufptr++) = *++str;
	}
	else if (str[0] == '$' && str[1] == '<')	/* discard padding */
	{
	    str += 2;
	    while (isdigit(*str) || *str == '.' || *str == '*' || *str == '/' || *str == '>')
		str++;
	    --str;
	}	
	else if (*str != '%' || !parametrized)
	    *(bufptr++) = *str;
	else if (sscanf(str, "%%?%%{%d}%%>%%t%%{%d}%%+%%;", &c1,&c2) == 2)
	{
	    str = strchr(str, ';');
	    (void) sprintf(bufptr, "%%>");
	    (void) strcat(bufptr, unctrl(c1));
	    (void) strcat(bufptr, unctrl(c2));
	    bufptr += strlen(bufptr);
	}
	else if (sscanf(str, "%%?%%{%d}%%>%%t%%'%c'%%+%%;", &c1,&ch2) == 2)
	{
	    str = strchr(str, ';');
	    (void) sprintf(bufptr, "%%>%s%c", unctrl(c1), ch2);
	    bufptr += strlen(bufptr);
	}
	else if (sscanf(str, "%%?%%'%c'%%>%%t%%{%d}%%+%%;", &ch1,&c2) == 2)
	{
	    str = strchr(str, ';');
	    (void) sprintf(bufptr, "%%>%c%c", ch1, c2);
	    bufptr += strlen(bufptr);
	}
	else if (sscanf(str, "%%?%%'%c'%%>%%t%%'%c'%%+%%;", &ch1, &ch2) == 2)
	{
	    str = strchr(str, ';');
	    (void) sprintf(bufptr, "%%>%c%c", ch1, ch2);
	    bufptr += strlen(bufptr);
	}
	else if (strncmp(str, "%{6}%*%+", 8) == 0)
	{
	    str += 7;
	    (void) sprintf(bufptr, "%%B");
	    bufptr += strlen(bufptr);
	}
	else if ((sscanf(str, "%%{%d}%%+%%c", &c1) == 1
		  || sscanf(str, "%%'%c'%%+%%c", &ch1) == 1)
		 && (cp = strchr(str, '+')))
	{
	    str = cp + 2;
	    *bufptr++ = '%';
	    *bufptr++ = '+';

	    if (ch1)
		c1 = ch1;
	    if (is7bits(c1) && isprint(c1))
		*bufptr++ = c1;
	    else
	    {
		if (c1 == (c1 & 0x1f)) /* iscntrl() returns T on 255 */
		    (void) strcat(bufptr, unctrl(c1));
		else
		    (void) sprintf(bufptr, "\\%03o", c1);
		bufptr += strlen(bufptr);
	    }
	}
	else if (strncmp(str, "%{2}%*%-", 8) == 0)
	{
	    str += 7;
	    (void) sprintf(bufptr, "%%D");
	    bufptr += strlen(bufptr);
	}
	else if (strncmp(str, "%{96}%^", 7) == 0)
	{
	    str += 6;
	    if (saw_m++ == 0)
	    {
		(void) sprintf(bufptr, "%%n");
		bufptr += strlen(bufptr);
	    }
	}
	else if (strncmp(str, "%{127}%^", 8) == 0)
	{
	    str += 7;
	    if (saw_n++ == 0)
	    {
		(void) sprintf(bufptr, "%%m");
		bufptr += strlen(bufptr);
	    }
	}
	else
	{
	    str++;
	    switch (*str) {
	    case '%':
		*(bufptr++) = '%';
		break;

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		*(bufptr++) = '%';
		while (isdigit(*str))
		    *bufptr++ = *str++;
		if (*str == 'd')
		    str++;
		else
		    _nc_warning("numeric prefix is missing trailing d");
		--str;
		break;

	    case 'd':
		*(bufptr++) = '%';
		*(bufptr++) = 'd';
		break;

	    case 'c':
		*(bufptr++) = '%';
		*(bufptr++) = '.';
		break;

	    /*
	     * %s isn't in termcap, but it's convenient to pass it through
	     * so we can represent things like terminfo pfkey strings in
	     * termcap notation.
	     */
	    case 's':
		*(bufptr++) = '%';
		*(bufptr++) = 's';
		break;

	    case 'p':
		str++;
		if (*str == '1')
		    seenone = 1;
		else if (*str == '2')
		{
		    if (!seenone && !seentwo)
		    {
			*(bufptr++) = '%';
			*(bufptr++) = 'r';
			seentwo++;
		    }
		}
		else if (*str >= '3')
		    return((char *)NULL);
		break;

	    case 'i':
		*(bufptr++) = '%';
		*(bufptr++) = 'i';
		break;

	    default:
		return((char *)NULL);

	    } /* endswitch (*str) */
	} /* endelse (*str == '%') */
	
	if (*str == '\0')
	    break;

    } /* endwhile (*str) */

    *bufptr = '\0';
    return(buffer);

}

#ifdef MAIN
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

int curr_line;

int main(int argc, char *argv[])
{
    int c, tc = FALSE;

    while ((c = getopt(argc, argv, "c")) != EOF)
	switch (c)
	{
	case 'c':
	    tc = TRUE;
	    break;
	}

    curr_line = 0;
    for (;;)
    {
	char	buf[BUFSIZ];

	++curr_line;
	if (fgets(buf, sizeof(buf), stdin) == (char *)NULL)
	    break;
	buf[strlen(buf) - 1] = '\0';
	_nc_set_source(buf);

	if (tc)
	{
	    char	*cp = infotocap("to termcap", buf, 1);

	    if (cp)
		(void) fputs(cp, stdout);
	}
	else
	    (void) fputs(captoinfo("to terminfo", buf, 1), stdout);
	(void) putchar('\n');
    }
    return(0);
}
#endif /* MAIN */

/* captoinfo.c ends here */

