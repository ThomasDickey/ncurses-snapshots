

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


/*
 *	captoinfo.c --- conversion between termcap and terminfo formats
 *
 *	This code was swiped from Ross Ridge's mytinfo package,
 *	adapted to fit ncurses by Eric S. Raymond <esr@snark.thyrsus.com>.
 *
 *	There is just one entry point:
 *
 *	char *captoinfo(n, s) -- convert value s for termcap string capability
 *		named n into terminfo format
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
 *		%sx	 subtract parameter FROM the character x
 *
 * 	Note the two definitions of %a: the GNU definition is translated if the
 *	characters after the 'a' are valid for it, otherwise the UW definition
 *	is translated.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
		fprintf(stderr, "warning: string too complex to convert\n");
	else
		stack[stackptr++] = onstack;
}

static void pop(void)
/* pop the top of the stack into onstack */
{
	if (stackptr == 0) 
		if (onstack == 0)
			fprintf(stderr, "warning: I'm confused\n");
		else
			onstack = 0;
	else
		onstack = stack[--stackptr];
	param++;
}

static int cvtchar(register char *sp)
/* convert a character to a terminfo push */
{
char c;
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
			if (sp[1] == '0' && sp[2] == '0') {
				c = '\0';
				len = 4;
			} else {
				c = '\200';	/* '\0' ???? */
				len = 2;
			}
			break;
		default:
			c = *sp;
			len = 2;
			break;
		}
		break;
	default:
		c = *sp;
		len = 1;
	}
	c &= 0177;
	if (isgraph(c) && c != ',' && c != '\'' && c != '\\' && c != ':') {
		*dp++ = '%'; *dp++ = '\''; *dp++ = c; *dp++ = '\'';
	} else {
		*dp++ = '%'; *dp++ = '{';
		if (c > 99)
			*dp++ = c / 100 + '0';
		if (c > 9)
			*dp++ = (c / 10) % 10 + '0';
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
			fprintf(stderr, "warning: string may not be optimal");
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

char *captoinfo(
/* convert a termcap string to terminfo format */
register char *cap,	/* relevant terminfo capability index */
register char *s)	/* string value of the capability */
{
	static char line[MAX_ENTRY];
	int nocode = 0;

	stackptr = 0;
	onstack = 0;
	seenm = 0;
	seenn = 0;
	seenr = 0;
	param = 1;

	dp = line;

	while(*s != '\0') {
	switch(*s) {
	case '%':
		s++;
		if (nocode) {
			*dp++ = '%';
			break;
		}
		switch(*s++) {
			case '%': *dp++ = '%'; break;
			case 'r':
				if (seenr++ == 1) {
					fprintf(stderr, "warning: saw %%r twice\n");
				}
				break;
			case 'm':
				if (seenm++ == 1) {
					fprintf(stderr, "warning: saw %%m twice\n");
				}
				break;
			case 'n':
				if (seenn++ == 1) {
					fprintf(stderr, "warning: saw %%n twice\n");
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
		s += cvtchar(s);
		getparm(param, 1);
		*dp++ = '%'; *dp++ = '-';
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
		*dp++ = '%'; *dp++ = '0';
		*dp++ = '2'; *dp++ = 'd';
		pop();
		break;
		case '3':
		getparm(param, 1);
		*dp++ = '%'; *dp++ = '0';
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
		default:
		*dp++ = '%';
		s--;
		fprintf(stderr, "warning: '%s' unknown %% code %c",
			cap, *s);
		if (*s >= 0 && *s < 32)
			fprintf(stderr, "^%c\n", *s + '@');
		else if (*s < 0 || *s >= 127)
			fprintf(stderr, "\\%03o\n", *s & 0377);
		else
			fprintf(stderr, "%c\n", *s);
		break;
		}
		break;
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
	}
	}
	*dp = '\0';
	return(line);
}

/* captoinfo.c ends here */
