
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
 *	tic.h - Global variables and structures for the terminfo
 *			compiler.
 *
 */

#ifndef __TIC_H
#define __TIC_H

#include <curses.h>	/* for the _tracef() prototype, ERR/OK, bool defs */

#define DEBUG(n, a)	if (_nc_tracing & (1 << (n - 1))) _tracef a 
extern int _nc_tracing;
extern void _nc_tracef(char *, ...);
extern char *_nc_visbuf(const char *);

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

extern	struct token	_nc_curr_token;

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

struct alias
{
    char	*from;
    char	*to;
};


extern struct name_table_entry	*_nc_info_hash_table[];
extern struct name_table_entry	*_nc_cap_hash_table[];
extern struct alias _nc_alias_table[];

extern struct name_table_entry	*_nc_get_table(bool);

#define NOTFOUND	((struct name_table_entry *) 0)

/* out-of-band values for representing absent capabilities */
#define ABSENT_BOOLEAN		-1
#define ABSENT_NUMERIC		-1
#define ABSENT_STRING		(char *)0

/* out-of-band values for representing cancels */
#define CANCELLED_BOOLEAN	-2
#define CANCELLED_NUMERIC	-2
#define CANCELLED_STRING	(char *)-1

/* termcap entries longer than this may break old binaries */
#define MAX_TERMCAP_LENGTH	1023

/* this is a documented limitation of terminfo */
#define MAX_TERMINFO_LENGTH	4096

/* comp_hash.c: name lookup */
extern void _nc_make_hash_table(struct name_table_entry *,
			    struct name_table_entry **);
struct name_table_entry	*_nc_find_entry(char *,
				    struct name_table_entry **);
struct name_table_entry *_nc_find_type_entry(char *,
					 int,
					 struct name_table_entry *);

/* comp_scan.c: lexical analysis */
extern int  _nc_get_token(void);
extern void _nc_push_token(int);
extern void _nc_reset_input(FILE *, char *);
extern void _nc_panic_mode(char);
extern int _nc_curr_line;
extern long _nc_curr_file_pos;
extern long _nc_comment_start, _nc_comment_end;
extern bool _nc_syntax;
#define SYN_TERMINFO	0
#define SYN_TERMCAP	1

/* comp_error.c: warning & abort messages */
extern void _nc_set_source(const char *name);
extern void _nc_set_type(const char *name);
extern void _nc_syserr_abort(const char *,...);
extern void _nc_err_abort(const char *,...);
extern void _nc_warning(const char *,...);

/* captoinfo.c: capability conversion */
extern char *_nc_captoinfo(char *, char *, bool);
extern char *_nc_infotocap(char *, char *, bool);

/* comp_main.c: compiler main */
extern char	*_nc_progname;

#endif /* __TIC_H */
