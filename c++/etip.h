// * This makes emacs happy -*-Mode: C++;-*-
/*----------------------------------------------------------------------------+
|       The ncurses menu and forms C++ binding is Copyright (C) 1997          |
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

// $Id: etip.h,v 1.8 1997/09/13 10:59:03 juergen Exp $

#ifndef _ETIP_H
#define _ETIP_H

#include <ncurses_cfg.h>

#ifdef __GNUG__
#  if HAVE_TYPEINFO
#    include <typeinfo>
#  endif
#endif

#if defined(__GNUG__)
#  if HAVE_BUILTIN_H
#    define exception builtin_exception
#    include <builtin.h>
#    undef exception
#  endif
#elif defined (__SUNPRO_CC)
#  include <generic.h>
#  include <string.h>
#else
#  include <string.h>
#endif

extern "C" {
#if HAVE_VALUES_H
#  include <values.h>
#endif

#include <assert.h>
#include <eti.h>
#include <errno.h>
}

// Forward Declarations
class NCursesPanel;
class NCursesMenu;
class NCursesForm;

class NCursesException
{
public:
  int errorno;
  const char *message;

  NCursesException (const char* msg, int err)
    : message(msg), errorno (err)
    {};

  NCursesException (const char* msg)
    : message(msg), errorno (E_SYSTEM_ERROR)
    {};

  virtual const char *classname() const {
    return "NCursesWindow";
  }
};

class NCursesPanelException : public NCursesException
{
public:
  const NCursesPanel* p;

  NCursesPanelException (const char *msg, int err) : 
    NCursesException (msg, err),
    p ((NCursesPanel*)0)
    {};

  NCursesPanelException (const NCursesPanel* panel,
			 const char *msg,
			 int err) : 
    NCursesException (msg, err),
    p (panel)
    {};

  NCursesPanelException (int err) : 
    NCursesException ("panel library error", err),
    p ((NCursesPanel*)0)
    {};

  NCursesPanelException (const NCursesPanel* panel,
			 int err) : 
    NCursesException ("panel library error", err),
    p (panel)
    {};

  virtual const char *classname() const {
    return "NCursesPanel";
  }

};

class NCursesMenuException : public NCursesException
{
public:
  const NCursesMenu* m;

  NCursesMenuException (const char *msg, int err) : 
    NCursesException (msg, err),
    m ((NCursesMenu *)0)
    {};

  NCursesMenuException (const NCursesMenu* menu,
			const char *msg,
			int err) : 
    NCursesException (msg, err),
    m (menu)
    {};

  NCursesMenuException (int err) : 
    NCursesException ("menu library error", err),
    m ((NCursesMenu *)0)
    {};

  NCursesMenuException (const NCursesMenu* menu,
			int err) : 
    NCursesException ("menu library error", err),
    m (menu)
    {};

  virtual const char *classname() const {
    return "NCursesMenu";
  }

};

class NCursesFormException : public NCursesException
{
public:
  const NCursesForm* f;

  NCursesFormException (const char *msg, int err) : 
    NCursesException (msg, err),
    f ((NCursesForm*)0)
    {};

  NCursesFormException (const NCursesForm* form,
			const char *msg,
			int err) : 
    NCursesException (msg, err),
    f (form)
    {};

  NCursesFormException (int err) : 
    NCursesException ("form library error", err),
    f ((NCursesForm*)0)
    {};

  NCursesFormException (const NCursesForm* form,
			int err) : 
    NCursesException ("form library error", err),
    f (form)
    {};

  virtual const char *classname() const {
    return "NCursesForm";
  }

};

#if !(defined(__GNUG__)||defined(__SUNPRO_CC))
#  include <iostream.h>
   extern "C" void exit(int);
#endif

inline void THROW(const NCursesException *e) {
#if defined(__GNUG__)
  (*lib_error_handler)(e?e->classname():"",e?e->message:"");
#elif defined(__SUNPRO_CC)
  genericerror(1, ((e != 0) ? (char *)(e->message) : ""));
#else
  if (e)
    cerr << e->message << endl;
  exit(0);
#endif     
}

#define THROWS(s)

#endif // _ETIP_H