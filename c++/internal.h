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

// $Id: internal.h,v 1.2 1997/09/07 22:22:48 juergen Exp $

#ifndef _CPLUS_INTERNAL_H
#define _CPLUS_INTERNAL_H 1

#ifdef USE_RCS_IDS
#define MODULE_ID(id) static const char Ident[] = id;
#else
#define MODULE_ID(id) /*nothing*/
#endif

#define CTRL(x) ((x) & 0x1f)

#endif
