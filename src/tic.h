
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
 *	tic.h - Global variables and structures for the terminfo
 *			compiler.
 *
 */

#include <stdio.h>
#ifndef NONPOSIX
#include <unistd.h>
#else
#include <libc.h>
#endif

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define SINGLE			/* only one terminal (actually none) */

char	*destination;		/* destination directory for object files */

char	curr_line;		/* current line # in input */
long	curr_file_pos;		/* file offset of current line */

int	debug_level;		/* level of debugging output */

#define DEBUG(level, fmt, a1) \
		if (debug_level >= level)\
		    fprintf(stderr, fmt, a1);

#define DEBUG0(level, str) \
		if (debug_level >= level)\
		    fprintf(stderr, str);

	/*
	 *	These are the types of tokens returned by the scanner.
	 *	The first three are also used in the hash table of capability
	 *	names.  The scanner returns one of these values after loading
	 *	the specifics into the global structure curr_token.
	 *
	 */

#define BOOLEAN 0		/* Boolean capability */
#define NUMBER 1		/* Numeric capability */
#define STRING 2		/* String-valued capability */
#define CANCEL 3		/* Capability to be cancelled in following tc's */
#define NAMES  4		/* The names for a terminal type */
#define UNDEF  5		/* Undefined */

	/*
	 *	The global structure in which the specific parts of a
	 *	scanned token are returned.
	 *
	 */

struct token
{
	char	*tk_name;		/* name of capability */
	int	tk_valnumber;	/* value of capability (if a number) */
	char	*tk_valstring;	/* value of capability (if a string) */
};

struct token	curr_token;

	/*
	 *	The file comp_captab.c contains an array of these structures,
	 *	one per possible capability.  These are then made into a hash
	 *	table array of the same structures for use by the parser.
	 *
	 */

struct name_table_entry
{
	struct name_table_entry *nte_link;
	char	*nte_name;	/* name to hash on */
	int	nte_type;	/* BOOLEAN, NUMBER or STRING */
	short	nte_index;	/* index of associated variable in its array */
};

extern struct name_table_entry	cap_table[];
extern struct name_table_entry	*cap_hash_table[];

#define NOTFOUND	((struct name_table_entry *) 0)
	/*
	 *	Function types
	 *
	 */

struct name_table_entry	*find_entry(char *); /* look up entry in hash table */

extern void set_source(const char *name);
extern void syserr_abort(const char *,...);
extern void err_abort(const char *,...);
extern void warning(const char *,...);
extern void panic_mode(char);
extern int  get_token(void);
extern void reset_input(void);
extern void make_hash_table(void);
extern void compile(void);

extern void make_hash_table(void);

extern char *captoinfo(char *name, char *valstr);
extern int dodump;

#define SYN_TERMINFO	0
#define SYN_TERMCAP	1
extern int syntax;

extern char	*progname;
