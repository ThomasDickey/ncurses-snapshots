
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
 *	term_entry.h -- interface to entry-manipulation code
 */

#ifndef _TERM_ENTRY_H
#define _TERM_ENTRY_H

#define MAX_USES	32

typedef struct entry {
	TERMTYPE	tterm;
	int		nuses;
	char		*uses[MAX_USES];
	long		cstart, cend;
	struct entry	*next;
	struct entry	*last;
}
ENTRY;

extern ENTRY	*_nc_head, *_nc_tail;
#define for_entry_list(qp)	for (qp = _nc_head; qp; qp = qp->next)

#define MAX_LINE	132

/* alloc_entry.c: elementary allocation code */
extern void _nc_init_entry(TERMTYPE *tp);
extern char *_nc_save_str(char *string);
extern void _nc_merge_entry(TERMTYPE *to, TERMTYPE *from);
extern void _nc_wrap_entry(ENTRY *ep);

/* parse_entry.c: entry-parsing code */
extern int _nc_parse_entry(ENTRY *, int, bool);

/* write_entry.c: writing an entry to the file system */
extern void _nc_write_entry(TERMTYPE *tp);

/* comp_parse.c: entry list handling */
extern void _nc_read_entry_source(FILE *fp, char *, int, bool);
extern bool _nc_entry_match(char *, char *);
extern int _nc_resolve_uses(void);
extern void _nc_free_entries(void);

#endif /* _TERM_ENTRY_H */

/* term_entry.h ends here */
