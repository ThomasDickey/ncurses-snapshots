/*
 * Dump control definitions and variables
 */

/* capability output formats */
#define F_TERMINFO	0	/* use terminfo names */
#define F_VARIABLE	1	/* use C variable names */
#define F_TERMCAP	2	/* termcap names, no capability conversion */
#define F_TCONVERT	3	/* termcap names, with capability conversion */
#define F_TCONVERR	4	/* as T_CONVERT, no skip of untranslatables */
#define F_LITERAL	5	/* like F_TERMINFO, but no smart defaults */

/* capability sort modes */
#define S_DEFAULT	0	/* sort by terminfo name (implicit) */
#define S_NOSORT	1	/* don't sort */
#define S_TERMINFO	2	/* sort by terminfo names (explicit) */
#define S_VARIABLE	3	/* sort by C variable names */
#define S_TERMCAP	4	/* sort by termcap names */

extern void dump_init(int mode, int sort, int width, int trace);
extern void dump_entry(TERMTYPE *cur_term, int (*pred)(int type, int index));
extern void compare_entry(void (*hook)(int type, int index, char *name));
extern char *expand(unsigned char *str);

#define FAIL	-1
