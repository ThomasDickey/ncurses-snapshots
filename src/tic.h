
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
 *	tic.h - Global variables and structures for the terminfo
 *			compiler.
 *
 */

#include <stdio.h>

#ifndef TRUE
typedef char	bool;
#define TRUE	1
#define FALSE	0
#endif

#ifndef OK
#define OK	0
#define ERR	-1
#endif

#define DEBUG(n, a)	if (_tracing & (1 << (n - 1))) _tracef a 
extern int _tracing;
extern void _tracef(char *, ...);
extern char *visbuf(const char *);

/*
 * These are the types of tokens returned by the scanner.  The first
 * three are also used in the hash table of capability names.  The scanner
 * returns one of these values after loading the specifics into the global
 * structure curr_token.
 */

#define BOOLEAN 0		/* Boolean capability */
#define NUMBER 1		/* Numeric capability */
#define STRING 2		/* String-valued capability */
#define CANCEL 3		/* Capability to be cancelled in following tc's */
#define NAMES  4		/* The names for a terminal type */
#define UNDEF  5		/* Undefined */

#define NO_PUSHBACK	-1	/* used in pushtype to indicate no pushback */

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

extern struct name_table_entry	info_table[];
extern struct name_table_entry	*info_hash_table[];
extern struct name_table_entry	cap_table[];
extern struct name_table_entry	*cap_hash_table[];

#define NOTFOUND	((struct name_table_entry *) 0)

/* out-of-band values for representing absent capabilities */
#define ABSENT_BOOLEAN		-1
#define ABSENT_NUMERIC		-1
#define ABSENT_STRING		(char *)0

/* out-of-band values for representing cancels */
#define CANCELLED_BOOLEAN	-2
#define CANCELLED_NUMERIC	-2
#define CANCELLED_STRING	(char *)-1

/* comp_hash.c: name lookup */
extern void make_hash_table(struct name_table_entry *,
			    struct name_table_entry **);
struct name_table_entry	*find_entry(char *,
				    struct name_table_entry **);
struct name_table_entry *find_type_entry(char *,
					 int,
					 struct name_table_entry *);

/* comp_scan.c: lexical analysis */
extern int  get_token(void);
extern void push_token(int);
extern void reset_input(FILE *);
extern void panic_mode(char);
extern int curr_line;
extern long curr_file_pos;
extern long comment_start, comment_end;
extern int syntax;
#define SYN_TERMINFO	0
#define SYN_TERMCAP	1

/* comp_error.c: warning & abort messages */
extern void set_source(const char *name);
extern void set_type(const char *name);
extern void syserr_abort(const char *,...);
extern void err_abort(const char *,...);
extern void warning(const char *,...);

/* captoinfo.c: capability conversion */
extern char *captoinfo(char *, char *, int);
extern char *infotocap(char *, char *, int);

/* comp_main.c: compiler main */
extern char	*progname;
