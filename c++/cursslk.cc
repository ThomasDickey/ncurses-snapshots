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

#include "cursslk.h"
#include "cursesapp.h"
#include "internal.h"

MODULE_ID("$Id: cursslk.cc,v 1.1 1997/09/07 22:22:56 juergen Exp $")

void Soft_Label_Key_Set::Soft_Label_Key::operator=(char *text)  {
  delete[] label;
  label = new char[1 + ::strlen(text)];
  ::strcpy(label,text);
}

long Soft_Label_Key_Set::count      = 0L;
int  Soft_Label_Key_Set::num_labels = 0;

Soft_Label_Key_Set::Label_Layout
  Soft_Label_Key_Set::format = None;

void Soft_Label_Key_Set::init() {
  slk_array = new Soft_Label_Key[num_labels];
  for(int i=0; i < num_labels; i++) {
    slk_array[i].num = i+1;
  }
  b_attrInit = FALSE;  
}

Soft_Label_Key_Set::Soft_Label_Key_Set() {
  if (format==None)
    Error("No default SLK layout");
  init();
}

Soft_Label_Key_Set::Soft_Label_Key_Set(Soft_Label_Key_Set::Label_Layout fmt) {
  if (fmt==None)
    Error("Invalid SLK Layout");
  if (count++==0) {
    format = fmt;
    if (ERR == ::slk_init((int)fmt))
      Error("slk_init");
    num_labels = (fmt>=PC_Style?12:8);
  }
  else if (fmt!=format)
    Error("All SLKs must have same layout");
  init();
}

Soft_Label_Key_Set::~Soft_Label_Key_Set() {
  if (!::isendwin())
    clear();
  delete[] slk_array;
  count--;
}

Soft_Label_Key_Set::Soft_Label_Key& Soft_Label_Key_Set::operator[](int i) {
  if (i<1 || i>num_labels)
    Error("Invalid Label index");
  return slk_array[i-1];
}

void Soft_Label_Key_Set::activate_label(int i, bool bf) {
  if (!b_attrInit) {
    NCursesApplication* A = NCursesApplication::getApplication();
    if (A) attrset(A->labels());
    b_attrInit = TRUE;
  }
  Soft_Label_Key& K = (*this)[i];
  if (ERR==::slk_set(K.num,bf?K.label:"",K.format))
    Error("slk_set");
  noutrefresh();
}

void Soft_Label_Key_Set::activate_labels(bool bf) {
  if (!b_attrInit) {
    NCursesApplication* A = NCursesApplication::getApplication();
    if (A) attrset(A->labels());
    b_attrInit = TRUE;
  }
  for(int i=1; i <= num_labels; i++) {
    Soft_Label_Key& K = (*this)[i];
    if (ERR==::slk_set(K.num,bf?K.label:"",K.format))
      Error("slk_set");
  }
  if (bf)
    restore();
  else
    clear();
  noutrefresh();
}
