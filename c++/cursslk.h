// * this is for making emacs happy: -*-Mode: C++;-*-
/*----------------------------------------------------------------------------+
|            The ncurses SLK C++ binding is Copyright (C) 1997                |
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

// $Id: cursslk.h,v 1.1 1997/09/07 22:22:48 juergen Exp $

#ifndef _CURSSLK_H
#define _CURSSLK_H

#include <cursesw.h>

class Soft_Label_Key_Set {
public:
  // This inner class represents the attributes of a Soft Label Key (SLK)
  class Soft_Label_Key {
    friend class Soft_Label_Key_Set;
  public:
    typedef enum { Left=0, Center=1, Right=2 } Justification;

  private:
    char *label;           // The Text of the Label
    Justification format;  // The Justification
    int num;               // The number of the Label

    Soft_Label_Key() : label((char*)0),format(Left),num(-1) {
    }
    
    virtual ~Soft_Label_Key() {
      delete[] label;
    };

  public:
    // Set the text of the Label
    void operator=(char *text);
    
    // Set the Justification of the Label
    inline void operator=(Justification just) {
      format = just;
    }
	 
    // Retrieve the text of the label
    inline char* operator()(void) const {
      return label; 
    }
  };
  
public:
  typedef enum {
    None                = -1,
    Three_Two_Three     = 0,
    Four_Four           = 1,
    PC_Style            = 2,
    PC_Style_With_Index = 3
  } Label_Layout;

private:
  static long count;               // Number of Key Sets
  static Label_Layout  format;     // Layout of the Key Sets
  static int  num_labels;          // Number Of Labels in Key Sets
  bool b_attrInit;                 // Are attributes initialized
  
  Soft_Label_Key *slk_array;       // The array of SLK's

  // Init the Key Set
  void init();

  // Activate or Deactivate Label# i, Label counting starts with 1!
  void activate_label(int i, bool bf=TRUE);

  // Activate of Deactivate all Labels
  void activate_labels(bool bf);

protected:
  inline void Error (const char* msg) const THROWS(NCursesException) {
    THROW(new NCursesException (msg));
  }

  // Remove SLK's from screen
  void clear() { 
    if (ERR==::slk_clear())
      Error("slk_clear");
  }    

  // Restore them
  void restore() { 
    if (ERR==::slk_restore())
      Error("slk_restore");
  }

public:

  // Construct a Key Set, use the most comfortable layout as default.
  // You must create a Soft_Label_Key_Set before you create any object of
  // the NCursesWindow, NCursesPanel or derived classes. (Actually before
  // ::initscr() is called).
  Soft_Label_Key_Set(Label_Layout fmt);

  // This constructor assumes, that you already constructed a Key Set
  // with a layout by the constructor above. This layout will be reused.
  Soft_Label_Key_Set();

  virtual ~Soft_Label_Key_Set();

  // Get Label# i. Label counting starts with 1!
  Soft_Label_Key& operator[](int i);

  // Retrieve number of Labels
  inline int labels() const { return num_labels; }          

  // Refresh the SLK portion of the screen
  inline void refresh() { 
    if (ERR==::slk_refresh())
      Error("slk_refresh");
  }

  // Mark the SLK portion of the screen for refresh, defer actual refresh
  // until next update call.
  inline void noutrefresh() { 
    if (ERR==::slk_noutrefresh())
      Error("slk_noutrefresh");
  }

  // Mark the whole SLK portion of the screen as modified
  inline void touch() { 
    if (ERR==::slk_touch())
      Error("slk_touch");
  }

  // Activate Label# i
  inline void show(int i) {
    activate_label(i,FALSE);
    activate_label(i,TRUE);
  }

  // Hide Label# i
  inline void hide(int i) {
    activate_label(i,FALSE);
  }

  // Show all Labels
  inline void show() {
    activate_labels(FALSE);
    activate_labels(TRUE);
  }

  // Hide all Labels
  inline void hide() {
    activate_labels(FALSE);
  }

  inline void attron(attr_t attrs) {
    if (ERR==::slk_attron(attrs))
      Error("slk_attron");
  }

  inline void attroff(attr_t attrs) {
    if (ERR==::slk_attroff(attrs))
      Error("slk_attroff");
  }

  inline void attrset(attr_t attrs) {
    if (ERR==::slk_attrset(attrs))
      Error("slk_attrset");
  }

  inline attr_t attr() const {
    return ::slk_attr();
  }
};

#endif // _CURSSLK_H
