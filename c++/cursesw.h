// * This makes emacs happy -*-Mode: C++;-*-
#ifndef _CURSESW_H
#define _CURSESW_H

#pragma interface

#include <config.h>

#include <stdio.h>
#include <stdarg.h>
#include <builtin.h>
#ifndef BSD_NET2
#include <values.h>
#endif
#include <strstream.h>
extern "C" {
#include   <curses.h>
}

/* SCO 3.2v4 curses.h includes term.h, which defines lines as a macro.
   Undefine it here, because NCursesWindow uses lines as a method.  */
#undef lines

// "Convert" macros to inlines, if needed.
#ifdef addch
inline int (addch)(chtype ch)  { return addch(ch); }
#undef addch
#endif
#ifdef addstr
/* The (char*) cast is to hack around missing const's */
inline int (addstr)(const char * str)  { return addstr((char*)str); }
#undef addstr
#endif
#ifdef attron
inline int (attron)(chtype at) { return attron(at); }
#undef attron
#endif
#ifdef attroff
inline int (attroff)(chtype at) { return attroff(at); }
#undef attroff
#endif
#ifdef attrset
inline chtype (attrset)(chtype at) { return attrset(at); }
#undef attrset
#endif
#ifdef border
inline int (border)(chtype ls, chtype rs, chtype ts, chtype bs, chtype tl, chtype tr, chtype bl, chtype br)
{ return border(ls,rs,ts,bs,tl,tr,bl,br); }
#undef border
#endif
#ifdef box
inline int (box)(WINDOW *win, int v, int h) { return box(win, v, h); }
#undef box
#endif
#ifdef clear
inline int (clear)()  { return clear(); }
#undef clear
#endif
#ifdef clearok
inline int (clearok)(WINDOW* win, int bf)  { return clearok(win, bf); }
#undef clearok
#else
extern "C" int clearok(WINDOW*, bool);
#endif
#ifdef clrtobot
inline int (clrtobot)()  { return clrtobot(); }
#undef clrtobot
#endif
#ifdef clrtoeol
inline int (clrtoeol)()  { return clrtoeol(); }
#undef clrtoeol
#endif
#ifdef delch
inline int (delch)()  { return delch(); }
#undef delch
#endif
#ifdef deleteln
inline int (deleteln)()  { return deleteln(); }
#undef deleteln
#endif
#ifdef erase
inline int (erase)()  { return erase(); }
#undef erase
#endif
#ifdef flushok
inline int (flushok)(WINDOW* _win, int _bf)  { return flushok(_win, _bf); }
#undef flushok
#else
#define _no_flushok
#endif
#ifdef getch
inline int (getch)()  { return getch(); }
#undef getch
#endif
#ifdef getstr
inline int (getstr)(char *_str)  { return getstr(_str); }
#undef getstr
#endif
#ifdef getyx
inline void (getyx)(WINDOW* win, int& y, int& x) { getyx(win, y, x); }
#undef getyx
#endif
#ifdef getbegyx
inline void (getbegyx)(WINDOW* win, int& y, int& x) { getbegyx(win, y, x); }
#undef getbegyx
#endif
#ifdef getmaxyx
inline void (getmaxyx)(WINDOW* win, int& y, int& x) { getmaxyx(win, y, x); }
#undef getmaxyx
#endif
#ifdef hline
inline int (hline)(chtype ch, int n) { return hline(ch, n); }
#undef hline
#endif
#ifdef inch
inline int (inch)()  { return inch(); }
#undef inch
#endif
#ifdef insch
inline int (insch)(char c)  { return insch(c); }
#undef insch
#endif
#ifdef insertln
inline int (insertln)()  { return insertln(); }
#undef insertln
#endif
#ifdef leaveok
inline int (leaveok)(WINDOW* win, int bf)  { return leaveok(win, bf); }
#undef leaveok
#else
extern "C" int leaveok(WINDOW* win, bool bf);
#endif
#ifdef move
inline int (move)(int x, int y)  { return move(x, y); }
#undef move
#endif
#ifdef refresh
inline int (rfresh)()  { return refresh(); }
#undef refresh
#endif
#ifdef scrl
inline int (scrl)(int l) { return scrl(l); }
#undef scrl
#endif
#ifdef scroll
inline int (scroll)(WINDOW *win) { return scroll(win); }
#undef scroll
#endif
#ifdef scrollok
inline int (scrollok)(WINDOW* win, int bf)  { return scrollok(win, bf); }
#undef scrollok
#else
#ifndef hpux
extern "C" int scrollok(WINDOW*, bool);
#else
extern "C" int scrollok(WINDOW*, char);
#endif
#endif
#ifdef setscrreg
inline int (setscrreg)(int t, int b) { return setscrreg(t, b); }
#undef setscrreg
#endif
#ifdef standend
inline chtype (standend)()  { return standend(); }
#undef standend
#endif
#ifdef standout
inline chtype (standout)()  { return standout(); }
#undef standout
#endif
#ifdef subpad
inline WINDOW *(subpad)(WINDOW *p, int l, int c, int y, int x) 
{ return derwin(p,l,c,y,x); }
#undef subpad
#endif
#ifdef timeout
inline int (timeout)(int delay) { return timeout(delay); }
#undef timeout
#endif
#ifdef touchline
inline int (touchline)(WINDOW *win, int s, int c)
{ return touchline(win,s,c); }
#undef touchline
#endif
#ifdef touchwin
inline int (touchwin)(WINDOW *win) { return touchwin(win); }
#undef touchwin
#endif
#ifdef untouchwin
inline int (untouchwin)(WINDOW *win) { return untouchwin(win); }
#undef untouchwin
#endif
#ifdef vline
inline int (vline)(chtype ch, int n) { return vline(ch, n); }
#undef vline
#endif
#ifdef waddstr
inline int (waddstr)(WINDOW *win, char *str) { return waddstr(win, str); }
#undef waddstr
#endif
#ifdef waddchstr
inline int (waddchstr)(WINDOW *win, chtype *at) { return waddchstr(win, at); }
#undef waddchstr
#endif
#ifdef wstandend
inline int (wstandend)(WINDOW *win)  { return wstandend(win); }
#undef wstandend
#endif
#ifdef wstandout
inline int (wstandout)(WINDOW *win)  { return wstandout(win); }
#undef wstandout
#endif
#ifdef wattroff
inline int (wattroff)(WINDOW *win, int att) { return wattroff(win, att); }
#undef wattroff
#endif
#ifdef wattrset
inline int (wattrset)(WINDOW *win, int att) { return wattrset(win, att); }
#undef wattrset
#endif
#ifdef winch
inline int (winch)(WINDOW* win) { return winch(win); }
#undef winch
#endif

#ifdef mvwaddch
inline int (mvwaddch)(WINDOW *win, int y, int x, chtype ch)
{ return mvwaddch(win, y, x, ch); }
#undef mvwaddch
#endif
#ifdef mvwaddchnstr
inline int (mvwaddchnstr)(WINDOW *win, int y, int x, chtype *str, int n)
{ return mvwaddchnstr(win, y, x, str, n); }
#undef mvwaddchnstr
#endif
#ifdef mvwaddchstr
inline int (mvwaddchstr)(WINDOW *win, int y, int x, chtype *str)
{ return mvwaddchstr(win, y, x, str); }
#undef mvwaddchstr
#endif
#ifdef mvwaddnstr
inline int (mvwaddnstr)(WINDOW *win, int y, int x, const char *str, int n)
{ return mvwaddnstr(win, y, x, (char*)str, n); }
#undef mvwaddnstr
#endif
#ifdef mvwaddstr
inline int (mvwaddstr)(WINDOW *win, int y, int x, const char * str)
{ return mvwaddstr(win, y, x, (char*)str); }
#undef mvwaddstr
#endif
#ifdef mvwdelch
inline int (mvwdelch)(WINDOW *win, int y, int x)
{ return mvwdelch(win, y, x); }
#undef mvwdelch
#endif
#ifdef mvwgetch
inline int (mvwgetch)(WINDOW *win, int y, int x) { return mvwgetch(win, y, x);}
#undef mvwgetch
#endif
#ifdef mvwgetstr
inline int (mvwgetstr)(WINDOW *win, int y, int x, char *str)
{return mvwgetstr(win,y,x, str);}
#undef mvwgetstr
#endif
#ifdef mvwinch
inline int (mvwinch)(WINDOW *win, int y, int x) { return mvwinch(win, y, x);}
#undef mvwinch
#endif
#ifdef mvwinsch
inline int (mvwinsch)(WINDOW *win, int y, int x, char c)
{ return mvwinsch(win, y, x, c); }
#undef mvwinsch
#endif

#ifdef mvaddch
inline int (mvaddch)(int y, int x, chtype ch)
{ return mvaddch(y, x, ch); }
#undef mvaddch
#endif
#ifdef mvaddnstr
inline int (mvaddnstr)(int y, int x, const char *str, int n)
{ return mvaddnstr(y, x, (char*)str, n); }
#undef mvaddnstr
#endif
#ifdef mvaddstr
inline int (mvaddstr)(int y, int x, const char * str)
{ return mvaddstr(y, x, (char*)str); }
#undef mvaddstr
#endif
#ifdef mvdelch
inline int (mvdelch)(int y, int x) { return mvdelch(y, x);}
#undef mvdelch
#endif
#ifdef mvgetch
inline int (mvgetch)(int y, int x) { return mvgetch(y, x);}
#undef mvgetch
#endif
#ifdef mvgetstr
inline int (mvgetstr)(int y, int x, char *str) {return mvgetstr(y, x, str);}
#undef mvgetstr
#endif
#ifdef mvinch
inline int (mvinch)(int y, int x) { return mvinch(y, x);}
#undef mvinch
#endif
#ifdef mvinsch
inline int (mvinsch)(int y, int x, char c)
{ return mvinsch(y, x, c); }
#undef mvinsch
#endif

#ifdef napms
inline void (napms)(unsigned long x) { napms(x); }
#undef napms
#endif
#ifdef fixterm
inline int (fixterm)(void) { return fixterm(); }
#undef fixterm
#endif
#ifdef resetterm
inline int (resetterm)(void) { return resetterm(); }
#undef resetterm
#endif
#ifdef saveterm
inline int (saveterm)(void) { return saveterm(); }
#undef saveterm
#endif
#ifdef crmode
inline int (crmode)(void) { return crmode(); }
#undef crmode
#endif
#ifdef nocrmode
inline int (nocrmode)(void) { return nocrmode(); }
#undef nocrmode
#endif

/*
 *
 * C++ class for windows.
 *
 *
 */

class NCursesWindow
{
  private:
    void           init(); 
  protected:
    static int     count;            // count of all active windows:
                                     //   We rely on the c++ promise that
                                     //   all otherwise uninitialized
                                     //   static class vars are set to 0

    WINDOW *       w;                // the curses WINDOW

    int            alloced;          // true if we own the WINDOW

    NCursesWindow* par;              // parent, if subwindow
    NCursesWindow* subwins;          // head of subwindows list
    NCursesWindow* sib;              // next subwindow of parent

    void           kill_subwindows(); // disable all subwindows

  public:
    NCursesWindow(WINDOW* &window);  // useful only for stdscr

    NCursesWindow(int lines,         // number of lines
 		  int cols,          // number of columns
	 	  int begin_y,       // line origin
		  int begin_x);      // col origin

    NCursesWindow(NCursesWindow& par,// parent window
		  int lines,         // number of lines
		  int cols,          // number of columns
		  int by,            // absolute or relative
		  int bx,            //   origins:
		  char absrel = 'a');// if `a', by & bx are
                                     // absolute screen pos,
                                     // else if `r', they are
                                     // relative to par origin
    virtual ~NCursesWindow();

    // terminal status
    int            lines() const { return LINES; }
                            // number of lines on terminal, *not* window
    int            cols() const { return COLS; }
                            // number of cols  on terminal, *not* window
    virtual int    colors() const { return 1; }
                            // number of available colors

    // window status
    int            height() const { return maxy() - begy() + 1; }
                             // number of lines in this window
    int            width() const { return maxx() - begx() + 1; }
                             // number of cols in this window
    int            begx() const { return w->_begx; }
                             // smallest x coord in window
    int            begy() const { return w->_begy; }
                             // smallest y coord in window
    int            maxx() const { return w->_maxx; }
                             // largest  x coord in window
    int            maxy() const { return w->_maxy; }
                             // largest  x coord in window
    virtual short  foreground() const { return 1; }
                             // actual foreground color
    virtual short  background() const { return 1; }
                             // actual background color
    virtual short  getcolor() const { return 0x11; }
                             // actual color pair number
    virtual int    setpalette(short fore, short back) const { return 0; }
                             // set color palette entry
    virtual int    setcolor(short pair) { return 0; }
                             // set actually used palette entry

    // window positioning
    int            move(int y, int x) { return ::wmove(w, y, x); }

    // coordinate positioning
    void           getyx(int& y, int& x) { ::getyx(w, y, x); }
    int            mvcur(int sy, int ey, int sx, int ex)
                       { return ::mvcur(sy, ey, sx, ex); }

    // input
    int            getch() { return ::wgetch(w); }
    int            getch(int y, int x)
                       { return (::wmove(w, y, x)==ERR) ? ERR : ::wgetch(w); }
    int            getstr(char* str) { return ::wgetstr(w, str); }
    int            getstr(int y, int x, char* str)
                       { return (::wmove(w, y, x)==ERR) ? ERR 
                                                        : ::wgetstr(w, str); }
    int            scanw(const char*, ...)
#if __GNUG__ >= 2
	           __attribute__ ((format (scanf, 2, 3)));
#else
                   ;
#endif
    int            scanw(int, int, const char*, ...)
#if __GNUG__ >= 2
	           __attribute__ ((format (scanf, 4, 5)));
#else
                   ;
#endif

    // output
    int            addch(const chtype ch) { return ::waddch(w, ch); }
    int            addch(int y, int x, chtype ch)
                       { return (::wmove(w, y, x)==ERR) ? ERR 
                                                        : ::waddch(w, ch); }
    int            addstr(const char* str) { return ::waddstr(w, (char*)str); }
    int            addstr(int y, int x, const char * str)
                       { return (::wmove(w, y, x)==ERR) ? ERR
                                                  : ::waddstr(w, (char*)str); }
    int            printw(const char* fmt, ...)
#if __GNUG__ >= 2
	           __attribute__ ((format (printf, 2, 3)));
#else
                   ;
#endif
    int            printw(int y, int x, const char * fmt, ...)
#if __GNUG__ >= 2
	           __attribute__ ((format (printf, 4, 5)));
#else
                   ;
#endif
    int            inch() { return ::winch(w); }
    int            inch(int y, int x)
                       { return (::wmove(w, y, x)==ERR) ? ERR : ::winch(w); }
    int            insch(chtype ch) { return ::winsch(w, ch); }
    int            insch(int y, int x, chtype ch)
                       { return (::wmove(w, y, x)==ERR) ? ERR 
                                                        : ::winsch(w, ch); }
    int            insertln() { return ::winsertln(w); }
    int            attron(chtype at) { return ::wattron(w, at); }
    int            attroff(chtype at) { return ::wattroff(w, at); }
    int            attrset(chtype at) { return ::wattrset(w, at); }

    // borders
    int            box(char vert, char  hor) { return ::box(w, vert, hor); }
    int            box() { return ::box(w, ACS_VLINE, ACS_HLINE); }

    // lines and boxes
    int            hline(int y, int x, chtype ch, int len)
                       { return (::wmove(w, y, x)==ERR) ? ERR:
                                               ::whline(w, ch, len); }
    int            hline(int y, int x, int len)
                       { return (::wmove(w, y, x)==ERR) ? ERR:
                                               ::whline(w, ACS_HLINE, len); }
    int            hline(chtype ch, int len) { return ::whline(w, ch, len); }
    int            hline(int len) { return ::whline(w, ACS_HLINE, len); }
    int            vline(int y, int x, chtype ch, int len)
                       { return (::wmove(w, y, x)==ERR) ? ERR:
                                                ::wvline(w, ch, len); }
    int            vline(int y, int x, int len)
                       { return (::wmove(w, y, x)==ERR) ? ERR:
                                                ::wvline(w, ACS_VLINE, len); }
    int            vline(chtype ch, int len) { return ::wvline(w, ch, len); }
    int            vline(int len) { return ::wvline(w, ACS_VLINE, len); }

    // erasure
    int            erase() { return ::werase(w); }
    int            clear() { return ::wclear(w); }
    int            clearok(int bf) { return ::clearok(w, bf); }
    int            clrtobot() { return ::wclrtobot(w); }
    int            clrtoeol() { return ::wclrtoeol(w); }
    int            delch() { return ::wdelch(w); }
    int            delch(int y, int x)
                       { return (::wmove(w, y, x)==ERR) ? ERR : ::wdelch(w); }
    int            deleteln() { return ::wdeleteln(w); }

    // screen control
    int            scroll() { return ::scroll(w); }
    int            scrollok(int bf) { return ::scrollok(w, bf); }
    int            idlok(int bf) { return ::idlok(w, bf); }
    int            touchwin() { return ::touchwin(w); }
    int            refresh() { return ::wrefresh(w); }
    int            leaveok(int bf) { return ::leaveok(w, bf); }
    int            noutrefresh() { return ::wnoutrefresh(w); }
    int            doupdate() { return ::doupdate(); }
#ifndef _no_flushok
    int            flushok(int bf) { return ::flushok(w, bf); }
#endif
    int            keypad(int bf) { return ::keypad(w, bf); }
    int            standout() { return ::wstandout(w); }
    int            standend() { return ::wstandend(w); }

    // multiple window control
    int            overlay(NCursesWindow &win)
                       { return ::overlay(w, win.w); }
    int            overwrite(NCursesWindow &win)
                       { return ::overwrite(w, win.w); }


    // traversal support
    NCursesWindow*  child() { return subwins; }
    NCursesWindow*  sibling() { return sib; }
    NCursesWindow*  parent() { return par; }
};


class NCursesColorWindow : public NCursesWindow {
  private:
    void           colorInit(void);
    short          getcolor(int getback) const;

  protected:
    static int     colorInitialized;

  public:
    NCursesColorWindow(WINDOW* &window);  // useful only for stdscr

    NCursesColorWindow(int lines,         // number of lines
		       int cols,          // number of columns
		       int begin_y,       // line origin
		       int begin_x);      // col origin

    NCursesColorWindow(NCursesWindow& par,// parent window
		       int lines,         // number of lines
		       int cols,          // number of columns
		       int by,            // absolute or relative
		       int bx,            //   origins:
		       char absrel = 'a');// if `a', by & bx are
                                          // absolute screen pos,
                                          // else if `r', they are
                                          // relative to par origin

    // terminal status
    int            colors() const { return has_colors() ? COLORS : 1; }
                            // number of available colors

    // window status
    short          foreground() const { return getcolor(0); }
                             // actual foreground color
    short          background() const { return getcolor(1); }
                             // actual background color
    short          getcolor() const { return PAIR_NUMBER(w->_attrs); }
                             // actual color pair number
    static int     setpalette(short fore, short back, short pair);
    int            setpalette(short fore, short back) const {
                      return setpalette(fore, back, PAIR_NUMBER(w->_attrs)); }
                             // set color palette entry
    int            setcolor(short pair);
                             // set actually used palette entry

};

#endif // _CURSESW_H
