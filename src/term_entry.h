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

extern ENTRY	*head, *tail;
#define for_entry_list(qp)	for (qp = head; qp; qp = qp->next)

#define MAX_LINE	132

/* alloc_entry.c: elementary allocation code */
extern void init_entry(TERMTYPE *tp);
extern char *save_str(char *string);
extern void merge_entry(TERMTYPE *to, TERMTYPE *from);
extern void wrap_entry(ENTRY *ep);

/* parse_entry.c: entry-parsing code */
extern int parse_entry(ENTRY *, int);

/* write_entry.c: writing an entry to the file system */
extern void write_entry(TERMTYPE *tp);

/* comp_parse.c: entry list handling */
void read_entry_source(FILE *fp, int);
int resolve_uses(void);
extern void free_entries(void);

#endif /* _TERM_ENTRY_H */

/* term_entry.h ends here */
