
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
 *	curses.priv.h
 *
 *	Header file for curses library objects which are private to
 *	the library.
 *
 */

#include "version.h"

#ifndef __GNUC__
#define inline
#endif

#ifndef NOACTION
#include <unistd.h>
typedef struct sigaction sigaction_t;
#else
#include "SigAction.h"
#endif

#include "curses.h"

#ifndef NOTERMIOS
#include <termios.h>
#ifndef TERMIOS
#define TERMIOS 1
#endif
#else
#include <sgtty.h>
#include <sys/ioctl.h>
#endif

#define min(a,b)	((a) > (b)  ?  (b)  :  (a))
#define max(a,b)	((a) < (b)  ?  (b)  :  (a))

#define FG(n)	((n) & 0x0f)
#define BG(n)	(((n) & 0xf0) >> 4)

#define BLANK        (' '|A_NORMAL)

#define CHANGED     -1

#ifdef TRACE
#define T(a)	if (_tracing & TRACE_CALLS) _tracef a 
#define TR(n, a)	if (_tracing & (n)) _tracef a 
extern int _tracing;
extern char *visbuf(const char *);
#else	
#define T(a)
#define TR(n, a)
#endif

extern int setupscreen(int, int);
extern void get_screensize(void);
extern void curses_signal_handler(bool);
extern int _outch(char);
extern void init_acs(void);
extern WINDOW *makenew(int, int, int, int);
extern int timed_wait(int fd, int wait, int *timeleft);
extern int scrolln(int, int, int, int);
extern void scroll_optimize(void);
extern void scroll_window(WINDOW *, int, int, int);
extern void wchangesync(WINDOW *win);
extern void outstr(char *str);

struct try {
        struct try      *child;     /* ptr to child.  NULL if none          */
        struct try      *sibling;   /* ptr to sibling.  NULL if none        */
        unsigned char    ch;        /* character at this node               */
        unsigned short   value;     /* code of string so far.  0 if none.   */
};
  
/*
 * Structure for soft labels.
 */
  
typedef struct {
	char dirty;			/* all labels have changed */
	char hidden;			/* soft lables are hidden */
	WINDOW *win;
 	struct slk_ent {
 	    char text[9];		/* text for the label */
 	    char form_text[9];		/* formatted text (left/center/...) */
 	    int x;			/* x coordinate of this field */
 	    char dirty;			/* this label has changed */
 	    char visible;		/* field is visible */
	} ent[8];
} SLK;

#define FIFO_SIZE	32

struct screen {
       	int		_ifd;	    	/* input file ptr for screen        */
   	FILE		*_ofp;	    	/* output file ptr for screen       */
   	int		_checkfd;	/* filedesc for typeahead check     */ 
	struct term	*_term;	    	/* terminal type information        */
	short		_lines;		/* screen lines			    */
	short		_columns;	/* screen columns		    */
	WINDOW		*_curscr;   	/* current screen                   */
	WINDOW		*_newscr;	/* virtual screen to be updated to  */
	WINDOW		*_stdscr;	/* screen's full-window context     */
	struct try  	*_keytry;   	/* "Try" for use with keypad mode   */
	unsigned int	_fifo[FIFO_SIZE]; 	/* input pushback buffer    */
	signed char	_fifohead, 	/* head of fifo queue               */
			_fifotail, 	/* tail of fifo queue               */
			_fifopeek;	/* where to peek for next char      */
	bool		_endwin;	/* are we out of window mode?       */
	chtype		_current_attr;	/* terminal attribute current set   */
	bool		_coloron;	/* is color enabled?                */
	int		_cursor;	/* visibility of the cursor         */
	int         	_cursrow;   	/* physical cursor row              */
	int         	_curscol;   	/* physical cursor column           */
	bool		_nl;	    	/* True if NL -> CR/NL is on        */
	bool		_raw;	    	/* True if in raw mode              */
	int		_cbreak;    	/* 1 if in cbreak mode              */
                       		    	/* > 1 if in halfdelay mode         */
	bool		_echo;	    	/* True if echo on                  */
	bool		_use_meta;      /* use the meta key?		    */
 	SLK		*_slk;	    	/* ptr to soft key struct / NULL    */
	int		_baudrate;	/* used to compute padding	    */
};

/*
 * We don't want to use the lines or columns capabilities internally,
 * because if the application is running multiple screens under
 * X windows, it's quite possible they could all have type xterm
 * but have different sizes!  So...
 */
#define screen_lines	SP->_lines
#define screen_columns	SP->_columns

extern struct screen	*SP;

extern int _slk_format;			/* format specified in slk_init() */

#define MAXCOLUMNS    135
#define MAXLINES      66
#define UNINITIALISED ((struct try * ) -1)
