
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
 *	comp_scan.c --- Lexical scanner for terminfo compiler.
 *
 *	reset_input()
 *	get_token()
 *	panic_mode()
 *	int syntax;
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tic.h"

extern _tracechar(unsigned char c);	/* avoid including curses.h */

/*
 * Maximum length of string capability we'll accept before raising an error.
 * Yes, there is a real capability in /etc/termcap this long, an "is".
 */
#define MAXCAPLEN	600

#define iswhite(ch)	(ch == ' '  ||  ch == '\t')

int	syntax;			/* termcap or terminfo? */
int	curr_line;		/* current line # in input */
long	curr_file_pos;		/* file offset of current line */
long	comment_start;		/* start of comment range before name */
long	comment_end;		/* end of comment range before name */

static int first_column;	/* See 'next_char()' below */
static char separator = ',';	/* capability separator */
static FILE *yyin;		/* scanner's input file descriptor */
static int pushtype;		/* type of pushback token */

static void reset_to(char);
static char next_char(void);
static char trans_string(char *);

/*
 *	int
 *	get_token()
 *
 *	Scans the input for the next token, storing the specifics in the
 *	global structure 'curr_token' and returning one of the following:
 *
 *		NAMES		A line beginning in column 1.  'name'
 *				will be set to point to everything up to but
 *				not including the first separator on the line.
 *		BOOLEAN		An entry consisting of a name followed by
 *				a separator.  'name' will be set to point to
 *				the name of the capability.
 *		NUMBER		An entry of the form
 *					name#digits,
 *				'name' will be set to point to the capability
 *				name and 'valnumber' to the number given.
 *		STRING		An entry of the form
 *					name=characters,
 *				'name' is set to the capability name and
 *				'valstring' to the string of characters, with
 *				input translations done.
 *		CANCEL		An entry of the form
 *					name@,
 *				'name' is set to the capability name and
 *				'valnumber' to -1.
 *		EOF		The end of the file has been reached.
 *
 *	A `separator' is either a comma or a semicolon, depending on whether
 *	we are in termcap or terminfo mode.
 *
 */

int
get_token()
{
long		number;
int		type;
int		ch;
static char	buffer[1024];
char		*ptr;
int		dot_flag = FALSE;
long		token_start;

	if (pushtype != NO_PUSHBACK)
	{
	    int retval = pushtype;

	    DEBUG(3, ("pushed-back token: `%s', class %d",
		      curr_token.tk_name, pushtype));

	    pushtype = NO_PUSHBACK;

	    /* currtok wasn't altered by push_token() */
	    return(retval);
	}

	if (feof(yyin))
	    return(EOF);

	token_start = ftell(yyin);
	while ((ch = next_char()) == '\n'  ||  iswhite(ch))
	    continue;

	/* we may be looking at a termcap-style continuation */
	if (ch == '\\')
	    while ((ch = next_char()) == '\n'  ||  iswhite(ch))
		continue;
	
	if (ch == EOF)
	    type = EOF;
	else {
	    /* if this is a termcap entry, skip a leading separator */
	    if (separator == ':' && ch == ':')
		ch = next_char();

	    if (ch == '.') {
			dot_flag = TRUE;
			DEBUG(8, ("dot-flag set"));

			while ((ch = next_char())=='.' || iswhite(ch))
			    continue;
	    }

	    /* have to make some punctuation chars legal for terminfo */
	    if (!isalnum(ch) && !strchr("@%&*!#", ch)) {
		 	warning("Illegal character - %s", _tracechar(ch));
		 	panic_mode(separator);
	    }

	    ptr = buffer;
	    *(ptr++) = ch;

	    if (first_column) {
			comment_start = token_start;
			comment_end = curr_file_pos;

	    		/*
			 * This is hairy.  We're parsing the first line of the
			 * entry, trying to figure out if it uses termcap or
			 * terminfo syntax.  We use the following rules:
			 * 
			 * 1. If the last character before the newline is
			 *    is `\' or '$', it's a multiline termcap entry
			 *    (the $ case copes with some broken entries in
			 *    the SVr4 termcap file).
			 *
			 * 2. If the last character before the newline is
			 *    a `:', it's a single-line termcap entry.
			 *
			 * 3. If the last character before the newline is
			 *    a ',', it's a terminfo entry.
			 *
			 * 4. Anything else is an error.
			 *
			 */
			while ((ch = next_char()) != '\n' && ch != EOF)
			    *(ptr++) = ch;
			--ptr;

			if (isspace(*ptr))
			    ptr--;

			if (ch == EOF)
			    err_abort("Premature EOF");
			else if (ptr[0] == '\\' || ptr[0] == '$')
			{
			    syntax = SYN_TERMCAP;
			    reset_to(separator = ':');
			    --ptr;
			}
			else if (ptr[0] == ':')
			{
			    syntax = SYN_TERMCAP;
			    reset_to(separator = ':');
			}
			else if (ptr[0] == ',')
			{
			    syntax = SYN_TERMINFO;
			    separator = ',';
			}
			else
			    err_abort("Can't determine the entry format");
			ptr[0] = '\0';

			ptr = buffer;

			curr_token.tk_name = buffer;
			type = NAMES;
	    } else {
			ch = next_char();
			/* we must allow ';' to catch k; */
			while (isalnum(ch) || ch == ';') {
			    	*(ptr++) = ch;
			    	ch = next_char();
			}

			*ptr++ = '\0';
			switch (ch) {
			case ',':
			case ':':
				if (ch != separator)
					err_abort("Separator inconsistent with syntax");
				curr_token.tk_name = buffer;
				type = BOOLEAN;
				break;
			case '@':
				if (next_char() != separator)
					warning("Missing separator");
				curr_token.tk_name = buffer;
				type = CANCEL;
				break;

		    	case '#':
				number = 0;
				while (isdigit(ch = next_char()))
					number = number * 10 + ch - '0';
				if (ch != separator)
					warning("Missing separator");
				curr_token.tk_name = buffer;
				curr_token.tk_valnumber = number;
				type = NUMBER;
				break;
		    
			case '=':
				ch = trans_string(ptr);
				if (ch != separator)
					warning("Missing separator");
				curr_token.tk_name = buffer;
				curr_token.tk_valstring = ptr;
				type = STRING;
				break;

			default:
				/* just to get rid of the compiler warning */
				type = UNDEF;
				warning("Illegal character - %s",
					_tracechar(ch));
			}
		} /* end else (first_column == FALSE) */
	} /* end else (ch != EOF) */

	if (dot_flag == TRUE)
	    DEBUG(8, ("Commented out "));

	if (_tracing & 0x80)
	{
	    fprintf(stderr, "Token: ");
	    switch (type)
	    {
		case BOOLEAN:
		    fprintf(stderr, "Boolean; name='%s'\n",
                                                          curr_token.tk_name);
		    break;
		
		case NUMBER:
		    fprintf(stderr, "Number;  name='%s', value=%d\n",
				curr_token.tk_name, curr_token.tk_valnumber);
		    break;
		
		case STRING:
		    fprintf(stderr, "String;  name='%s', value='%s'\n",
				curr_token.tk_name, visbuf(curr_token.tk_valstring));
		    break;
		
		case CANCEL:
		    fprintf(stderr, "Cancel; name='%s'\n",
                                                          curr_token.tk_name);
		    break;
		
		case NAMES:

		    fprintf(stderr, "Names; value='%s'\n",
                                                          curr_token.tk_name);
		    break;

		case EOF:
		    fprintf(stderr, "End of file\n");
		    break;

		default:
		    warning("Bad token type");
	    }
	}

	if (dot_flag == TRUE)		/* if commented out, use the next one */
	    type = get_token();

	DEBUG(3, ("token: `%s', class %d", curr_token.tk_name, type));

	return(type);
}


/*
 *	push_token()
 *
 *	Push a token of given type so that it will be reread by the next
 *	get_token() call.
 */

void push_token(int class)
{
    /*
     * This implementation is kind of bogus, it will fail if we ever do
     * more than one pushback at a time between get_token() calls.  It
     * relies on the fact that curr_tok is static storage that nothing 
     * but get_token() touches.
     */
    pushtype = class;

    DEBUG(3, ("pushing token: `%s', class %d",
	      curr_token.tk_name, pushtype));
}

/*
 * 	char *next_char()
 *
 *	Returns the next character in the input stream.  Comments and leading
 *	white space are stripped.
 *
 *	The global state variable 'firstcolumn' is set TRUE if the character
 * 	returned is from the first column of the input line.
 *
 *	The global variable curr_line is incremented for each new line.
 *	The global variable curr_file_pos is set to the file offset of the
 *	beginning of each line.
 */
#define LEXBUFSIZ	1024

static int	curr_column = -1;
static char	line[LEXBUFSIZ];

static char
next_char(void)
{
	char		*rtn_value;

	if (curr_column < 0 || curr_column >= LEXBUFSIZ || line[curr_column] == '\0')
	{
	    do {
		curr_file_pos = ftell(yyin);

		if ((rtn_value = fgets(line, LEXBUFSIZ, yyin)) != NULL)
		    curr_line++;
	    } while
		(rtn_value != NULL && line[0] == '#');

	    if (rtn_value == NULL)
		return (EOF);

	    curr_column = 0;
	    while (iswhite(line[curr_column]))
		curr_column++;
	}

	if (curr_column == 0  &&  line[0] != '\n')
	    first_column = TRUE;
	else
	    first_column = FALSE;
	
	return (line[curr_column++]);
}

static void reset_to(char ch)
/* roll back the buffer pointer to just after the first instance of ch */
{
    char	*backto = strchr(line, ch);

    if (backto)
	curr_column = (backto - line + 1);

}

static
void backspace(void)
{
	curr_column--;

	if (curr_column < 0)
	    syserr_abort("Backspaced off beginning of line");
}


/*
 *	reset_input()
 *
 *	Resets the input-reading routines.  Used on initialization,
 *	or after a seek has been done.
 */

void reset_input(FILE *fp)
{
	curr_column = -1;
	pushtype = NO_PUSHBACK;
	yyin = fp;
	curr_file_pos = 0L;
	curr_line = 0;
}


/*
 *	char
 *	trans_string(ptr)
 *
 *	Reads characters using next_char() until encountering a separator, nl,
 *	or end-of-file.  The returned value is the character which caused
 *	reading to stop.  The following translations are done on the input:
 *
 *		^X  goes to  ctrl-X (i.e. X & 037)
 *		{\E,\n,\r,\b,\t,\f}  go to
 *			{ESCAPE,newline,carriage-return,backspace,tab,formfeed}
 *		{\^,\\}  go to  {carat,backslash}
 *		\ddd (for ddd = up to three octal digits)  goes to the character ddd
 *
 *		\e == \E
 *		\0 == \200
 *
 */

static char
trans_string(ptr)
char	*ptr;
{
int	count = 0;
int	number;
int	i;
char	ch, last_ch = '\0';

	while ((ch = next_char()) != separator  &&  ch != EOF) {
	    if (ch == '^' && last_ch != '%') {
		ch = next_char();
		if (ch == EOF)
		    err_abort("Premature EOF");

		if (! isprint(ch)) {
		    warning("Illegal ^ character - %s", _tracechar(ch));
		}
		*(ptr++) = ch & 037;
	    }
	    else if (ch == '\\') {
		ch = next_char();
		if (ch == EOF)
		    err_abort("Premature EOF");
		
		if (ch >= '0'  &&  ch <= '7') {
		    number = ch - '0';
		    for (i=0; i < 2; i++) {
			ch = next_char();
			if (ch == EOF)
			    err_abort("Premature EOF");
			
			if (ch < '0'  ||  ch > '7') {
			    backspace();
			    break;
			}

			number = number * 8 + ch - '0';
		    }

		    if (number == 0)
			number = 0200;
		    *(ptr++) = (char) number;
		} else {
		    switch (ch) {
			case 'E':
			case 'e':	*(ptr++) = '\033';	break;
			
			case 'l':
			case 'n':	*(ptr++) = '\n';	break;
			
			case 'r':	*(ptr++) = '\r';	break;
			
			case 'b':	*(ptr++) = '\010';	break;

			case 's':	*(ptr++) = ' ';		break;
			
			case 'f':	*(ptr++) = '\014';	break;
			
			case 't':	*(ptr++) = '\t';	break;
			
			case '\\':	*(ptr++) = '\\';	break;
			
			case '^': 	*(ptr++) = '^';		break;

			case ',':	*(ptr++) = '\\';
					*(ptr++) = ',';		break;

			case ':':	*(ptr++) = ':';		break;

			case '\n':
			    continue;

			default:
			    warning("Illegal character %s in \\ sequence",
				    _tracechar(ch));
			    *(ptr++) = ch;
		    } /* endswitch (ch) */
		} /* endelse (ch < '0' ||  ch > '7') */
	    } /* end else if (ch == '\\') */
	    else {
		*(ptr++) = ch;
	    }
	    
	    count ++;

	    last_ch = ch;

	    if (count > MAXCAPLEN)
		warning("Very long string found.  Missing separator?");
	} /* end while */

	*ptr = '\0';

	return(ch);
}

/*
 * Panic mode error recovery - skip everything until a "ch" is found.
 */
void panic_mode(char ch)
{
	int c;

	for (;;) {
		c = next_char();
		if (c == ch)
			return;
		if (c == EOF)
			return;
	}
}
