#!/bin/sh
################################################################################
# Copyright 1995 by Thomas E. Dickey <dickey@clark.net>                        #
# All Rights Reserved.                                                         #
#                                                                              #
# Permission to use, copy, modify, and distribute this software and its        #
# documentation for any purpose and without fee is hereby granted, provided    #
# that the above copyright notice appear in all copies and that both that      #
# copyright notice and this permission notice appear in supporting             #
# documentation, and that the name of the above listed copyright holder(s) not #
# be used in advertising or publicity pertaining to distribution of the        #
# software without specific, written prior permission. THE ABOVE LISTED        #
# COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,    #
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT #
# SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY SPECIAL,        #
# INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM   #
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE   #
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR    #
# PERFORMANCE OF THIS SOFTWARE.                                                #
################################################################################
#
# MKcurses.sh
#
### Generate curses.h from curses.h.in.  Insert the proper type for bool,
### also the current version stamp (get these from arguments).
version=$1
builtin_bool=$2
type_of_bool=$3

trap 'rm -f conftest.tmp; exit 0' 0 1 2 15

sed \
	-e "s/#\\(define\\)[ 	][ 	]*\\(CXX_BUILTIN_BOOL\\).*/#\\1 \\2 $builtin_bool/" \
	-e "s/#\\(define\\)[ 	][ 	]*\\(CXX_TYPE_OF_BOOL\\).*/#\\1 \\2 $type_of_bool/" \
	-e "s/@VERSION@/$version/" \
	$4/curses.h.in >conftest.tmp

if cmp -s curses.h conftest.tmp 2>/dev/null; then
	echo include/curses.h is unchanged
	rm -f conftest.tmp
else
	echo updating include/curses.h
	rm -f curses.h~
	if test -f curses.h; then mv curses.h curses.h~; fi
	mv conftest.tmp curses.h
fi
