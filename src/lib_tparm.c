
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
 *	tparm.c
 *
 */

#include <string.h>
#include <term.h>
#include "curses.priv.h"

/*
 *	char *
 *	tparm(string, parms)
 *
 *	Substitute the given parameters into the given string by the following
 *	rules (taken from terminfo(5)):
 *
 *	     Cursor addressing and other strings  requiring  parame-
 *	ters in the terminal are described by a parameterized string
 *	capability, with like escapes %x in  it.   For  example,  to
 *	address  the  cursor, the cup capability is given, using two
 *	parameters: the row and column to  address  to.   (Rows  and
 *	columns  are  numbered  from  zero and refer to the physical
 *	screen visible to the user, not to any  unseen  memory.)  If
 *	the terminal has memory relative cursor addressing, that can
 *	be indicated by
 *	
 *	     The parameter mechanism uses  a  stack  and  special  %
 *	codes  to manipulate it.  Typically a sequence will push one
 *	of the parameters onto the stack and then print it  in  some
 *	format.  Often more complex operations are necessary.
 *	
 *	     The % encodings have the following meanings:
 *	
 *	     %%        outputs `%'
 *	     %d        print pop() like %d in printf()
 *	     %2d       print pop() like %2d in printf()
 *	     %02d      print pop() like %02d in printf()
 *	     %3d       print pop() like %3d in printf()
 *	     %03d      print pop() like %03d in printf()
 *	     %c        print pop() like %c in printf()
 *	     %s        print pop() like %s in printf()
 *	
 *	     %p[1-9]   push ith parm
 *	     %P[a-z]   set variable [a-z] to pop()
 *	     %g[a-z]   get variable [a-z] and push it
 *	     %'c'      push char constant c
 *	     %{nn}     push integer constant nn
 *	
 *	     %+ %- %* %/ %m
 *	               arithmetic (%m is mod): push(pop() op pop())
 *	     %& %| %^  bit operations: push(pop() op pop())
 *	     %= %> %<  logical operations: push(pop() op pop())
 *	     %A %O     logical and & or operations for conditionals
 *	     %! %~     unary operations push(op pop())
 *	     %i        add 1 to first two parms (for ANSI terminals)
 *	
 *	     %? expr %t thenpart %e elsepart %;
 *	               if-then-else, %e elsepart is optional.
 *	               else-if's are possible ala Algol 68:
 *	               %? c1 %t b1 %e c2 %t b2 %e c3 %t b3 %e c4 %t b4 %e b5 %;
 *	
 *	For those of the above operators which are binary and not commutative,
 *	the stack works in the usual way, with
 *			%gx %gy %m
 *	resulting in x mod y, not the reverse.
 */

#define STACKSIZE	20

typedef union {
	unsigned int	num;
	char	       *str;
} stack_frame;

static  stack_frame	stack[STACKSIZE];
static	int	stack_ptr;
static	char	buffer[256];
static	int	param[9];
static	char	*bufptr;
static	int	variable[26];

static inline void npush(int x)
{
	if (stack_ptr < STACKSIZE) {
		stack[stack_ptr].num = x;
        stack_ptr++;
    }
}

static inline int npop(void)
{
	return   (stack_ptr > 0  ?  stack[--stack_ptr].num  :  0);
}

static inline char *spop(void)
{
	return   (stack_ptr > 0  ?  stack[--stack_ptr].str  :  0);
}

char *tparm(const char *string, ...)
{
va_list	ap;
char	len;
int	number;
int	level;
int	x, y;
int	i;
register char	*cp;

	if (string == NULL)
		return NULL;

	/*
	 * Find the highest parameter-number referred to in the format string.
	 * Use this value to limit the number of arguments copied from the
	 * variable-length argument list.
	 */
	for (cp = (char *)string, number = 0; *cp != '\0'; cp++) {
		if (cp[0] == '%' && cp[1] != '\0') {
			switch (cp[1]) {
			case '%':
				cp++;
				break;
			case 'i':
				if (number < 2)
					number = 2;
				break;
			case 'p':
				cp++;
				if (cp[1] >= '1' && cp[1] <= '9') {
					int c = cp[1] - '0';
					if (c > number)
						number = c;
				}
				break;
			}
		}
	}

	va_start(ap, string);
	for (i = 0; i < number; i++) {
		/*
		 * FIXME: potential loss here if sizeof(int) != sizeof(char *).
		 * A few caps (such as plab_norm) have string-valued parms.
		 */
		param[i] = va_arg(ap, int);
	}

#ifdef TRACE
	if (_tracing & TRACE_CALLS) {
		*(cp = buffer) = '\0';
		for (i = 0; i < number; i++) {
			(void)sprintf(cp, ", %d", param[i]);
			cp += strlen(cp);
		}
		_tracef("tparm(\"%s\"%s) called", visbuf(string), buffer);
 	}
#endif /* TRACE */

	stack_ptr = 0;
	bufptr = buffer;

	while (*string) {
	    if (*string != '%')
			*(bufptr++) = *string;
	    else {
			string++;
			switch (*string) {
		    default:
				break;
		    case '%':
				*(bufptr++) = '%';
				break;

		    case 'd':
				sprintf(bufptr, "%d", npop());
				bufptr += strlen(bufptr);
				break;

		    case '0':
				string++;
				len = *string;
				if ((len == '2'  ||  len == '3')  &&  *++string == 'd') {
				    if (len == '2')
							sprintf(bufptr, "%02d", npop());
						else
							sprintf(bufptr, "%03d", npop());
			    
				    bufptr += strlen(bufptr);
				}
				break;

		    case '2':
				string++;
				if (*string == 'd') {
				    sprintf(bufptr, "%2d", npop());
				    bufptr += strlen(bufptr);
				}
				break;

		    case '3':
				string++;
				if (*string == 'd') {
				    sprintf(bufptr, "%3d", npop());
				    bufptr += strlen(bufptr);
				}
				break;

		    case 'c':
				*(bufptr++) = (char) npop();
				break;

		    case 's':
				strcpy(bufptr, spop());
				bufptr += strlen(bufptr);
				break;

		    case 'p':
				string++;
				if (*string >= '1'  &&  *string <= '9')
				    npush(param[*string - '1']);
				break;

		    case 'P':
				string++;
				if (*string >= 'a'  &&  *string <= 'z')
				    variable[*string - 'a'] = npop();
				break;

		    case 'g':
				string++;
				if (*string >= 'a'  &&  *string <= 'z')
				    npush(variable[*string - 'a']);
				break;

		    case '\'':
				string++;
				npush(*string);
				string++;
				break;

		    case '{':
				number = 0;
				string++;
				while (*string >= '0'  &&  *string <= '9') {
				    number = number * 10 + *string - '0';
				    string++;
				}
				npush(number);
				break;

		    case '+':
				npush(npop() + npop());
				break;

		    case '-':
				y = npop();
				x = npop();
				npush(x - y);
				break;

		    case '*':
				npush(npop() * npop());
				break;

		    case '/':
				y = npop();
				x = npop();
				npush(x / y);
				break;

		    case 'm':
				y = npop();
				x = npop();
				npush(x % y);
				break;

		    case 'A':
				npush(npop() && npop());
				break;

		    case 'O':
				npush(npop() || npop());
				break;
	
		    case '&':
				npush(npop() & npop());
				break;

		    case '|':
				npush(npop() | npop());
				break;
	
		    case '^':
				npush(npop() ^ npop());
				break;

		    case '=':
				y = npop();
				x = npop();
				npush(x == y);
				break;

		    case '<':
				y = npop();
				x = npop();
				npush(x < y);
				break;

		    case '>':
				y = npop();
				x = npop();
				npush(x > y);
				break;

		    case '!':
				npush(! npop());
				break;

		    case '~':
				npush(~ npop());
				break;

		    case 'i':
				param[0]++;
				param[1]++;
				break;

		    case '?':
				break;

		    case 't':
				x = npop();
				if (x) {
				    /* do nothing; keep executing */
				} else {
				    /* scan forward for %e or %; at level zero */
					string++;
					level = 0;
					while (*string) {
					    if (*string == '%') {
							string++;
							if (*string == '?')
							    level++;
							else if (*string == ';') {
							  if (level > 0)
									level--;
							  else
									break;
							}
							else if (*string == 'e'  && level == 0)
							  break;
					    }

					    if (*string)
						string++;
					}
				}
				break;

		    case 'e':
				/* scan forward for a %; at level zero */
			    string++;
			    level = 0;
			    while (*string) {
					if (*string == '%') {
					  string++;
					  if (*string == '?')
							level++;
					  else if (*string == ';') {
							if (level > 0)
							    level--;
							else
						    	break;
					 	}
					}

					if (*string)
					    string++;
			  }
				break;

		    case ';':
				break;

			} /* endswitch (*string) */
	  } /* endelse (*string == '%') */

	  if (*string == '\0')
			break;
	    
	  string++;
	} /* endwhile (*string) */

	*bufptr = '\0';
	return(buffer);
}

