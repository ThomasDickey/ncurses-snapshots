// * This makes emacs happy -*-Mode: C++;-*-
/*----------------------------------------------------------------------------+
|         The ncurses Application C++ binding is Copyright (C) 1997           |
|             by Juergen Pfeifer <Juergen.Pfeifer@T-Online.de>                |
|                          All Rights Reserved.                               |
|                                                                             |
| Permission to use, copy, modify, and distribute this software and its       |
| documentation for any purpose and without fee is hereby granted, provided   |
| that the above copyright notice appear in all copies and that both that     |
| copyright notice and this permission notice appear in supporting            |
| documentation, and that the name of the above listed copyright holder(s)    |
| not be used in advertising or publicity pertaining to distribution of the   |
| software without specific, written prior permission.                        |
|                                                                             |
| THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO |
| THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT- |
| NESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR  |
| ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER    |
| RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CON-   |
| TRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION |
| WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                               |
+----------------------------------------------------------------------------*/

// $Id: cursesapp.h,v 1.2 1997/10/13 22:56:21 tom Exp $

#ifndef _CURSESAPP_H
#define _CURSESAPP_H

#include <cursslk.h>

class NCursesApplication {
public:
  typedef struct _slk_link {          // This structure is used to maintain
    struct _slk_link* prev;           // a stack of SLKs
    Soft_Label_Key_Set* SLKs;
  } SLK_Link;
private:
  static int rinit(NCursesWindow& w); // Internal Init function for title
  static NCursesApplication* theApp;  // Global ref. to the application

  static SLK_Link* slk_stack;

protected:
  static NCursesWindow* titleWindow;  // The Title Window (if any)

  bool b_Colors;                      // Is this a color application?
  NCursesWindow* Root_Window;         // This is the stdscr equiv.

  // Initialization of attributes;
  // Rewrite this in your derived class if you prefer other settings
  virtual void init(bool bColors);

  // The number of lines for the title window. Default is no title window
  // You may rewrite this in your derived class
  virtual int titlesize() const {
    return 0;
  }

  // This method is called to put something into the title window initially
  // You may rewrite this in your derived class
  virtual void title() {
  }

  // The layout used for the Soft Label Keys. Default is to have no SLKs.
  // You may rewrite this in your derived class
  virtual Soft_Label_Key_Set::Label_Layout useSLKs() const {
    return Soft_Label_Key_Set::None;
  }

  // This method is called to initialize the SLKs. Default is nothing.
  // You may rewrite this in your derived class
  virtual void init_labels(Soft_Label_Key_Set& S) const {
  }

  // Your derived class must implement this method. The return value must
  // be the exit value of your application.
  virtual int run() = 0;


  // The constructor is protected, so you may use it in your derived
  // class constructor. The argument tells whether or not you want colors.
  NCursesApplication(bool wantColors = FALSE);

public:
  virtual ~NCursesApplication();

  // Get a pointer to the current application object
  static NCursesApplication* getApplication() {
    return theApp;
  }

  // This method runs the application and returns its exit value
  int operator()(void);

  // Process the commandline arguments. The default implementation simply
  // ignores them. Your derived class may rewrite this.
  virtual void handleArgs(int argc, char* argv[]) {
  }

  // Does this application use colors?
  inline bool useColors() const {
    return b_Colors;
  }

  // Push the Key Set S onto the SLK Stack. S then becomes the current set
  // of Soft Labelled Keys.
  void push(Soft_Label_Key_Set& S);

  // Throw away the current set of SLKs and make the previous one the
  // new current set.
  bool pop();

  // Retrieve the current set of Soft Labelled Keys.
  Soft_Label_Key_Set* top() const;

  // Attributes to use for menu and forms foregrounds
  virtual chtype foregrounds() const {
    return b_Colors ? COLOR_PAIR(1) : A_BOLD;
  }

  // Attributes to use for menu and forms backgrounds
  virtual chtype backgrounds() const {
    return b_Colors ? COLOR_PAIR(2) : A_NORMAL;
  }

  // Attributes to use for inactive (menu) elements
  virtual chtype inactives() const {
    return b_Colors ? (COLOR_PAIR(3)|A_DIM) : A_DIM;
  }

  // Attributes to use for (form) labels and SLKs
  virtual chtype labels() const {
    return b_Colors ? COLOR_PAIR(4) : A_NORMAL;
  }

  // Attributes to use for form backgrounds
  virtual chtype dialog_backgrounds() const {
    return b_Colors ? COLOR_PAIR(4) : A_NORMAL;
  }

  // Attributes to use as default for (form) window backgrounds
  virtual chtype window_backgrounds() const {
    return b_Colors ? COLOR_PAIR(5) : A_NORMAL;
  }

  // Attributes to use for the title window
  virtual chtype screen_titles() const {
    return b_Colors ? COLOR_PAIR(6) : A_BOLD;
  }

};
 
#endif // _CURSESAPP_H
