// * this is for making emacs happy: -*-Mode: C++;-*-
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

#include "cursesapp.h"
#include "internal.h"

MODULE_ID("$Id: cursesmain.cc,v 1.1 1997/09/07 22:22:56 juergen Exp $")

/* This is the default implementation of main() for a NCursesApplication.
 * You only have to instantiate a static NCursesApplication object in your
 * main application source file and link this module with your application.
 */ 
int main(int argc, char* argv[])
{
  NCursesApplication* A = NCursesApplication::getApplication();
  if (!A)
    return(1);
  A->handleArgs(argc,argv);
  return((*A)());
  ::endwin();
}
